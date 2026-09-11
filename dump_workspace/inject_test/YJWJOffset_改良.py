"""提取 IL2CPP dump 偏移：优先使用 v7 Offset:: 摘要，兼容旧 Class: 注释。

Python 3.9+，仅使用标准库。原 Offset.h 和 dump 不会被覆盖。
无参数时沿用文末的旧路径配置；也可用 --input/--dump/--output/--updated。
"""
import argparse
import bisect
import codecs
from collections import Counter, defaultdict
from dataclasses import dataclass, field
import logging
import os
from pathlib import Path
import re
from typing import Dict, List, Optional, Tuple


NUMBER = re.compile(r"(?:0[xX][0-9a-fA-F]+|[0-9]+)(?:[uUlL]*)\Z")
WRITABLE_STATUSES = {"FOUND", "CALIBRATED", "VERIFIED", "DERIVED"}
MODULE_KEYS = {"UnityPlayer", "GameAssembly", "NarakaBladepoint", "MatrixAddr"}
NORMAL_KEYS = {
    "Base_WindowBase", "m_GlobalTime", "m_CharacterManager", "m_UserDataManager",
    "m_CharactorSync", "m_EntityManager", "m_BuffManager", "m_TransparentTagHandler",
    "AnimPlayableCrossFadeInFixedTimeNormal", "MemoryShockVirtualVtableSlotOffset",
}
SUPER_KEYS = {k + "2" for k in NORMAL_KEYS if k.startswith(("Base_", "m_"))}
SUPER_KEYS.add("AnimPlayableCrossFadeInFixedTimeSuper")
DECL = re.compile(
    r"\b(?:(?:inline|constexpr|const|static)\s+)*(?:std::)?"
    r"(?P<type>uint64_t|uint32_t|uint16_t|uint8_t|uintptr_t|DWORD)\s+"
    r"(?P<name>[A-Za-z_]\w*)\s*=\s*(?P<value>[^;{}]+?)\s*;"
)
SKIP_CPP = re.compile(r'//[^\r\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'')
SUMMARY_ROW = re.compile(
    r"^// Offset::(?P<key>[A-Za-z_]\w*(?:\.[A-Za-z_]\w*)*)\s*=\s*"
    r"(?P<value>[^;]+);\s*//\s*status=(?P<status>[A-Z_]+)(?:\s+(?P<evidence>.*))?$"
)
FIELD_ROW = re.compile(r"^\s+(.+?)\s+(\S+);\s*//\s*offset:\s*(0[xX][0-9a-fA-F]+|[0-9]+)(?:\s|$)")
MODIFIERS = re.compile(r"^(?:(?:public|private|protected|internal|static|readonly|const)\s+)+")

# 仅用于展示。内部状态码和 dump 协议保持原样，避免改变偏移写入规则。
STATUS_ZH = {
    "FOUND": "已找到", "CALIBRATED": "已校准", "VERIFIED": "已验证",
    "DERIVED": "已推导", "CANDIDATE": "候选，待确认", "NOT_FOUND": "未找到",
    "NOT_LOADED": "未加载", "NOT_DUMPED": "已加载但未选择导出",
    "UNRESOLVED": "未确认", "UNRESOLVED_LAYOUT": "布局未确认",
    "AMBIGUOUS": "存在多个匹配，无法确定", "SCAN_INCOMPLETE": "扫描未完成",
    "SKIPPED_LEGACY": "已跳过旧项", "OUTSIDE_MODULE": "地址超出当前模块",
    "NON_EXECUTABLE": "地址位于不可执行内存", "UNREADABLE": "读取失败",
    "NOT_FIELD_OFFSET": "常量，不是字段偏移", "SPECIAL_STORAGE": "特殊存储布局",
    "CONFIG_CONSTANT": "配置常量", "ABI_LAYOUT": "固定内存布局常量",
    "RUNTIME_VA": "运行时绝对地址", "VARIANT_UNKNOWN": "模块变体未确定",
    "SUMMARY_MISSING": "摘要中缺少此项", "NO_MAPPING": "缺少查找映射",
}
ACTION_ZH = {
    "UPDATED": "已更新", "UNCHANGED": "数值相同，无需修改",
    "RETAINED_UNRESOLVED": "保留原值，尚未确认",
    "RETAINED_RUNTIME": "保留原值，不写入运行时地址",
    "RETAINED_CONSTANT": "保留原常量",
    "RETAINED_OUT_OF_RANGE": "保留原值，数值超出类型范围",
}
SOURCE_ZH = {"summary": "顶部偏移摘要", "Class": "旧格式类与字段匹配"}
VARIANT_ZH = {"normal": "普通版", "super": "Super / SuperIBT 变体", None: "未确定"}

# 覆盖 v7/v7.1 已有的诊断语句。类名、字段名、地址及布局成员名保留原文。
# 不认识的新说明保留原始信息，不能为了翻译而丢失证据。
EVIDENCE_ZH = {
    "module base VA; changes on restart": "模块基址的绝对地址，重新启动后会变化",
    "live camera matrix VA; not a managed field/RVA; depends on camera/native layout": "运行时相机矩阵的绝对地址；不是托管字段偏移或模块相对地址，依赖相机和原生内存布局",
    "other GameAssembly variant; this dump covers only ": "属于另一种 GameAssembly 变体；本次仅导出 ",
    "application unresolved sentinel; not a discovered game RVA": "程序用于表示尚未解析的占位常量，不是查找到的游戏相对地址",
    "singleton instance klass back-reference": "单例实例的类指针回指验证通过",
    "manager static_fields layout validations disagree": "不同管理器的静态字段布局验证结果不一致",
    "static_fields layout lacks a live singleton back-reference": "静态字段布局缺少有效单例实例的类指针回指证据",
    "actual hook RCX type unconfirmed; ActorKitInteractableDevice.agent is PlayerEnvironmentColliderAgent, not EntityKit": "尚未确认钩子入口 RCX 的实际类型；ActorKitInteractableDevice.agent 的类型是 PlayerEnvironmentColliderAgent，不能当作 EntityKit",
    "intended hook method must be confirmed": "需要进一步确认这是否为预期的钩子目标方法",
    "native EXE allocation/search hint; no stable managed semantic": "原生可执行文件的内存分配或搜索提示，缺少稳定的托管语义映射",
    "legacy hook context chain; not interchangeable with ActorKit fields": "旧钩子上下文指针链，不能直接用 ActorKit 字段替代",
    "semantic mapping unavailable; needs current native control-flow evidence": "尚未恢复语义映射，需要当前版本的原生控制流证据",
    "legacy comparison immediate; target state semantics not established": "旧比较指令的立即数，尚未确认对应状态的含义",
    "unused old entry; user allowed skipping; ": "长期未使用的旧项，已按用户允许跳过；",
    "operator overload declaring type/signature not specified": "未指定运算符重载所属的类和完整方法签名",
    "no declaring class/signature or current consumer in Offset.h; velocity methods are ambiguous": "Offset.h 未明确所属类、方法签名或当前调用位置，多个速度相关方法无法唯一匹配",
    "Il2CppArray header(0x20) + unboxed Entry.value; ": "数组头部大小（0x20）加上未装箱 Entry.value 的偏移；",
    "entry array header + Dictionary.Entry.value offset": "条目数组头部加上 Dictionary.Entry.value 的偏移",
    "generic definition has no confirmed instance layout": "开放泛型定义尚无已确认的实例布局",
    "x64 Il2CppArray object header: klass,monitor,bounds,max_length": "64 位数组对象头部，依次包含类指针、同步指针、数组边界和最大长度",
    "TypeInfo slot readback changed or failed": "类型信息指针槽的读回值发生变化或读取失败",
    "multiple same-name classes have live TypeInfo slots": "多个同名类均存在有效类型信息指针槽，无法唯一确定",
    "Dictionary entries type not recovered": "尚未恢复字典条目数组的类型",
    "Dictionary entries is not SZARRAY": "字典条目字段不是一维零下标数组",
    "Dictionary.Entry unboxed value layout unconfirmed": "字典条目未装箱时的 value 字段布局尚未确认",
    "UnityPlayer module unavailable": "未找到可用的 UnityPlayer 模块",
    "no visible game client window for dimensions check": "没有可见游戏窗口，无法校验客户区尺寸",
    "invalid UnityPlayer range": "UnityPlayer 模块地址范围无效",
    "native identity unconfirmed": "尚未确认该原生对象的身份",
    "historical executable pointer has no verified MethodInfo pair": "历史偏移处的可执行指针没有通过方法信息配对验证",
    "historical class byte offset; resolve paired MethodInfo for diagnostic only": "历史类结构字节偏移；仅使用方法信息配对进行诊断",
    "historical class byte offset; intended virtual method not specified": "历史类结构字节偏移，尚未指定预期的虚方法",
    " does not point to module executable code; ": " 没有指向当前模块的可执行代码；",
    "historical probe +": "检查历史偏移 +",
    "module base unavailable": "模块基址不可用",
    "runtime layout calibration failed": "运行时布局校准失败",
    "unknown layout member": "无法识别的布局成员",
    "runtime layout: ": "运行时布局成员：",
    "field count unreadable": "字段数量读取失败",
    "invalid field count": "字段数量无效",
    "method count unreadable": "方法数量读取失败",
    "invalid method count": "方法数量无效",
    " metadata read failed": " 元数据读取失败",
    " is a literal": " 是字面常量",
    " has multiple matching fields": " 存在多个匹配字段",
    " has multiple matching overloads": " 存在多个匹配的方法重载",
    " methodPointer unavailable": " 方法指针不可用",
    "multiple declaring classes match: ": "多个声明类均匹配：",
    "multiple classes: ": "存在多个同名类：",
    "method/signature-not-found: ": "未找到对应方法或签名：",
    "TypeInfo slot-not-found: ": "未找到类型信息指针槽：",
    "class-not-found": "未找到类",
    "field-not-found": "未找到字段",
    "Class: ": "类：",
    "[TypeInfo RVA]": "[类型信息相对地址]",
    "[instance]": "[实例字段]",
    "[static]": "[静态字段]",
}
EVIDENCE_LABEL_ZH = {
    "targetRVA": "目标相对地址", "MethodInfo": "方法信息地址", "rawOffset": "原始偏移",
    "params": "参数数量", "slot": "槽位", "klass": "类指针", "VA": "绝对地址",
    "module": "模块", "client": "客户区尺寸", "chain": "指针链", "candidates": "候选数量",
    "offset": "偏移",
}


def chinese_evidence(evidence: str) -> str:
    text = evidence
    for source, translated in sorted(EVIDENCE_ZH.items(), key=lambda pair: len(pair[0]), reverse=True):
        text = text.replace(source, translated)
    text = re.sub(r"\b(" + "|".join(EVIDENCE_LABEL_ZH) + r")=",
                  lambda m: EVIDENCE_LABEL_ZH[m.group(1)] + "=", text)
    text = text.replace("chain=slot", "指针链=指针槽").replace("指针链=slot", "指针链=指针槽")
    text = text.replace("width@", "宽度@").replace("height@", "高度@")
    return text or "无补充说明"


def number(value: str) -> Optional[int]:
    value = value.strip()
    if not NUMBER.fullmatch(value):
        return None
    value = re.sub(r"[uUlL]+$", "", value)
    return int(value, 16 if value.lower().startswith("0x") else 10)


def detect_encoding(file_path: str) -> str:
    """分块验证编码，保留 BOM；不忽略无法解码的字节。"""
    with open(file_path, "rb") as stream:
        prefix = stream.read(4)
    if prefix.startswith(codecs.BOM_UTF8):
        return "utf-8-sig"
    if prefix.startswith((codecs.BOM_UTF16_LE, codecs.BOM_UTF16_BE)):
        return "utf-16"
    for encoding in ("utf-8", "gbk", "cp1252", "latin-1"):
        try:
            with open(file_path, encoding=encoding) as stream:
                while stream.read(1 << 20):
                    pass
            return encoding
        except UnicodeDecodeError:
            continue
    raise ValueError("无法识别文件编码: " + file_path)


@dataclass
class Declaration:
    key: str
    ctype: str
    old: str
    start: int
    end: int
    line: int
    klass: Optional[str]
    member: Optional[str]


@dataclass
class Result:
    status: str
    value: Optional[int] = None
    evidence: str = ""
    source: str = "summary"


@dataclass
class ClassRecord:
    name: str
    namespace: str = ""
    parent: str = ""
    rva: Optional[int] = None
    fields: Dict[str, List[Result]] = field(default_factory=lambda: defaultdict(list))


def parse_class_line(line: str) -> Tuple[Optional[str], Optional[str]]:
    """旧格式支持 Class: 类->字段，也支持带类型的完整字段声明。"""
    match = re.search(r"Class:\s*([^\s,]+?)(?=\s|->|,|$)(?:\s*->\s*([^\r\n]+))?", line)
    if not match:
        return None, None
    klass, member = match.groups()
    return klass, member.strip() if member else None


def mask_cpp(text: str) -> str:
    return SKIP_CPP.sub(lambda m: re.sub(r"[^\r\n]", " ", m.group()), text)


def parse_declarations(text: str) -> List[Declaration]:
    """恢复匿名嵌套结构路径；只处理数据声明，排除函数、枚举和注释。

    这是 Offset.h 所用声明子集的解析器，不执行宏或 C++ 初始化表达式。
    """
    code = mask_cpp(text)
    blocks, stack = [], []
    boundary = 0
    for token in re.finditer(r"[{};]", code):
        if token.group() == "{":
            prefix = code[boundary:token.start()]
            ns = re.search(r"\bnamespace\s+([\w:]+)\s*$", prefix)
            struct = re.search(r"\bstruct(?:\s+\w+)?\s*$", prefix)
            block = {"start": token.start(), "end": len(code),
                     "kind": "namespace" if ns else "struct" if struct else "other",
                     "name": ns.group(1) if ns else ""}
            blocks.append(block)
            stack.append(block)
        elif token.group() == "}":
            if not stack:
                raise ValueError("Offset.h 存在未配对的右括号")
            block = stack.pop()
            block["end"] = token.start()
            if block["kind"] == "struct":
                tail = re.match(r"\s*([A-Za-z_]\w*)\s*;", code[token.end():])
                block["name"] = tail.group(1) if tail else ""
        boundary = token.end()
    if stack:
        raise ValueError("Offset.h 存在未闭合的结构或函数")

    line_starts = [0] + [m.end() for m in re.finditer("\n", text)]
    declarations, seen = [], set()
    for match in DECL.finditer(code):
        parents = [b for b in blocks if b["start"] < match.start() < b["end"]]
        if any(b["kind"] == "other" or (b["kind"] == "struct" and not b["name"]) for b in parents):
            continue
        namespaces = [b["name"] for b in parents if b["kind"] == "namespace"]
        if namespaces and namespaces != ["Offset"]:
            continue
        key = ".".join([b["name"] for b in parents if b["kind"] == "struct"] + [match["name"]])
        if key in seen:
            raise ValueError("Offset.h 存在重复变量路径: " + key)
        seen.add(key)
        line_end = text.find("\n", match.end())
        tail = text[match.end():line_end if line_end != -1 else len(text)]
        # 一行有多个声明时，末尾注释只属于紧邻的声明。
        klass, member = parse_class_line(tail) if tail.lstrip().startswith("//") else (None, None)
        declarations.append(Declaration(key, match["type"], text[match.start("value"):match.end("value")],
                                        match.start("value"), match.end("value"),
                                        bisect.bisect_right(line_starts, match.start()), klass, member))
    return declarations


def variant_for_key(key: str) -> Optional[str]:
    return "normal" if key in NORMAL_KEYS else "super" if key in SUPER_KEYS else None


class DumpIndex:
    def __init__(self):
        self.summary: Dict[str, Result] = {}
        self.has_summary = False
        self.classes: Dict[str, List[ClassRecord]] = defaultdict(list)
        self.variant: Optional[str] = None
        self.module = "unknown"

    @classmethod
    def load(cls, path: str, wanted_fields=(), encoding=None):
        index = cls()
        current = None
        in_fields = False
        inventory = None
        wanted = set(wanted_fields)
        wanted.update("<" + name + ">k__BackingField" for name in wanted_fields)
        with open(path, encoding=encoding or detect_encoding(path)) as stream:
            for raw in stream:
                line = raw.rstrip("\r\n")
                # v7 的全部输入数据在头部；不为每个变量重读 70+ MiB 的正文。
                if index.has_summary and re.match(r"^// (?:Image \d+:|Class:)", line):
                    break
                if line.startswith("// ===== Offset.h Discovery Summary"):
                    index.has_summary = True
                module = re.match(r"^// (?:Module|Active module):\s*(\S+)", line)
                if module:
                    index.module = module.group(1)
                    name = index.module.lower()
                    if "gameassembly_super" in name:
                        index.variant = "super"
                    elif name == "gameassembly.dll":
                        index.variant = "normal"
                if line.startswith("// Offset::"):
                    index.has_summary = True
                    row = SUMMARY_ROW.fullmatch(line)
                    if not row:
                        raise ValueError("无法解析摘要行: " + line)
                    key, token, status, evidence = row.group("key", "value", "status", "evidence")
                    if key in index.summary:
                        raise ValueError("dump 摘要存在重复变量路径: " + key)
                    value = number(token)
                    if status in WRITABLE_STATUSES and value is None:
                        raise ValueError("摘要状态为成功但数值无效: " + key)
                    index.summary[key] = Result(status, value, evidence or "")
                count = re.match(r"^// Inventory: total=(\d+)\b", line)
                if count:
                    inventory = int(count.group(1))
                if index.has_summary:
                    continue
                # 无摘要的旧 dump：一次扫描建立索引，只存请求涉及的字段名。
                name = re.match(r"^// Class:\s*(.+?)\s*$", line)
                if name:
                    current = ClassRecord(name.group(1))
                    index.classes[current.name].append(current)
                    in_fields = False
                    continue
                if current is None:
                    continue
                ns = re.match(r"^// Namespace:\s*(.*?)\s*$", line)
                if ns:
                    current.namespace = ns.group(1)
                rva = re.match(r"^// RVA:\s*(0x[0-9a-fA-F]+)\s*$", line)
                if rva:
                    current.rva = int(rva.group(1), 16)
                parent = re.match(r"^(?:\w+\s+)*(?:class|struct)\s+\S+\s*:\s*([^\s,]+)", line)
                if parent:
                    current.parent = parent.group(1)
                section = re.match(r"^\s+// (Fields|Methods|Properties)\b", line)
                if section:
                    in_fields = section.group(1) == "Fields"
                row = FIELD_ROW.match(line) if in_fields else None
                if row and (row.group(2) in wanted or not wanted):
                    declaration, member, offset = row.groups()
                    value = number(offset)
                    static = bool(re.search(r"\bstatic\b", declaration))
                    status = "UNRESOLVED_LAYOUT" if "`" in current.name and not static and value < 16 else "FOUND"
                    current.fields[member].append(Result(status, value,
                        "Class: {}.{}->{} {} [{}]".format(current.namespace, current.name,
                            MODIFIERS.sub("", declaration), member, "static" if static else "instance"), "Class"))
        if index.has_summary and (inventory is None or inventory != len(index.summary)):
            raise ValueError("摘要缺少 Inventory 或条数不符，可能是未完成的 dump；不生成更新头文件")
        return index

    def candidates(self, name: str, namespace=None):
        if "." in name and namespace is None:
            namespace, name = name.rsplit(".", 1)
        return [c for c in self.classes.get(name, []) if namespace is None or c.namespace == namespace]

    def field_in_class(self, klass: ClassRecord, member: str, visited=None) -> Optional[Result]:
        visited = set() if visited is None else visited
        if id(klass) in visited or len(visited) >= 32:
            return None
        visited.add(id(klass))
        for name in (member, "<" + member + ">k__BackingField"):
            hits = klass.fields.get(name, [])
            if len(hits) > 1:
                return Result("AMBIGUOUS", evidence="重复字段: " + name, source="Class")
            if hits:
                return hits[0]
        if klass.parent:
            parents = self.candidates(klass.parent, klass.namespace if "." not in klass.parent else None)
            if not parents:
                parents = self.candidates(klass.parent)
            if len(parents) == 1:
                return self.field_in_class(parents[0], member, visited)
        return None

    def legacy(self, klass: str, field_info: Optional[str]) -> Result:
        hits = []
        member = field_info.split()[-1] if field_info else None
        for candidate in self.candidates(klass):
            if member:
                result = self.field_in_class(candidate, member)
            else:
                result = Result("FOUND", candidate.rva, "Class: " + klass + " [TypeInfo RVA]", "Class") if candidate.rva is not None else None
            if result:
                hits.append(result)
        if len(hits) > 1:
            return Result("AMBIGUOUS", evidence="同名类/命名空间存在多个匹配，请明确 Class 注释", source="Class")
        return hits[0] if hits else Result("NOT_FOUND", evidence="未找到 Class/字段或类级 RVA", source="Class")


def extract_offset_from_dump(dump_file: str, encoding: str, target_class: str,
                             target_field: Optional[str]) -> Optional[str]:
    """兼容旧调用；主流程使用完整路径摘要，不逐项调用此函数。"""
    index = DumpIndex.load(dump_file, [target_field.split()[-1]] if target_field else (), encoding)
    result = index.legacy(target_class, target_field)
    return "0x{:X}".format(result.value) if result.status == "FOUND" else None


def find_assignment_value_in_line(line: str) -> Optional[str]:
    match = DECL.search(mask_cpp(line))
    return line[match.start("value"):match.end("value")] if match else None


def replace_assignment_value_in_line(line: str, new_value: str) -> Tuple[str, Optional[str]]:
    match = DECL.search(mask_cpp(line))
    if not match:
        return line, None
    start, end = match.span("value")
    return line[:start] + new_value + line[end:], line[start:end]


def choose_result(declaration: Declaration, index: DumpIndex, variant: Optional[str]) -> Result:
    required = variant_for_key(declaration.key)
    if required and variant != required:
        status = "VARIANT_UNKNOWN" if variant is None else "NOT_LOADED"
        # 保留 v7 本身的 NOT_DUMPED/NOT_LOADED 证据，绝不回退取另一变体类 RVA。
        result = index.summary.get(declaration.key)
        if result and result.status in {"NOT_LOADED", "NOT_DUMPED"}:
            return result
        return Result(status, evidence="变量属于{}，当前变体为{}".format(VARIANT_ZH[required], VARIANT_ZH[variant]))
    if index.has_summary:
        return index.summary.get(declaration.key, Result("SUMMARY_MISSING", evidence="此摘要未列出变量；不以 Class 查找覆盖缺失状态"))
    if declaration.klass:
        return index.legacy(declaration.klass, declaration.member)
    return Result("NO_MAPPING", evidence="旧 dump 无 Offset:: 摘要，声明也没有 Class 注释", source="Class")


def process_file(input_file: str, dump_file: str, output_file: str, *, updated_file=None, variant="auto"):
    """生成 *_updated.h 与逐项报告，保留未确认项原值和完整源码格式。"""
    if variant not in {"auto", "normal", "super"}:
        raise ValueError("variant 必须是 auto、normal 或 super")
    input_path = Path(input_file)
    updated_file = Path(updated_file) if updated_file else input_path.with_name(input_path.stem + "_updated" + input_path.suffix)
    paths = [Path(p).resolve() for p in (input_file, dump_file, output_file, updated_file)]
    if len({os.path.normcase(str(p)) for p in paths}) != len(paths):
        raise ValueError("输入、dump、报告和更新文件必须是不同文件，不能覆盖源文件")
    input_encoding = detect_encoding(input_file)
    with open(input_file, encoding=input_encoding, newline="") as stream:
        text = stream.read()
    declarations = parse_declarations(text)
    if not declarations:
        raise ValueError("没有找到受支持的 Offset 数据声明")
    wanted_fields = [d.member.split()[-1] for d in declarations if d.member]
    index = DumpIndex.load(dump_file, wanted_fields)
    active_variant = index.variant if variant == "auto" else variant
    if index.variant and active_variant != index.variant:
        raise ValueError("--variant 与 dump 模块标记冲突；不能跨变体更新")
    rows, edits = [], []
    for d in declarations:
        result = choose_result(d, index, active_variant)
        new_value = "0x{:X}".format(result.value) if result.value is not None else "-"
        action = "RETAINED_UNRESOLVED"
        reason = result.evidence
        if result.status == "RUNTIME_VA" or d.key in MODULE_KEYS:
            action = "RETAINED_RUNTIME"
        elif result.status in {"ABI_LAYOUT", "CONFIG_CONSTANT"}:
            action = "RETAINED_CONSTANT"
            if number(d.old) is not None and number(d.old) != result.value:
                reason += "; 原常量与报告不同，需要人工核对"
        elif result.status in WRITABLE_STATUSES and result.value is not None:
            bits = {"DWORD": 32, "uint32_t": 32, "uint16_t": 16, "uint8_t": 8}.get(d.ctype, 64)
            if not 0 <= result.value < (1 << bits):
                action = "RETAINED_OUT_OF_RANGE"
                reason += "; 数值超出声明类型范围"
            elif number(d.old) == result.value:
                action = "UNCHANGED"
            else:
                action = "UPDATED"
                edits.append((d.start, d.end, new_value))
        rows.append({"key": d.key, "line": d.line, "old": d.old, "new": new_value,
                     "status": result.status, "action": action, "source": result.source, "evidence": reason})
    updated = text
    for start, end, value in reversed(edits):
        updated = updated[:start] + value + updated[end:]
    # 编码失败必须在写文件前发现，不能静默丢弃中文注释或生成半个文件。
    updated_bytes = updated.encode(input_encoding)
    actions = Counter(row["action"] for row in rows)
    action_summary = "；".join("{}：{} 项".format(ACTION_ZH.get(action, action), count)
                               for action, count in actions.items())
    report = ["偏移量提取结果", "=" * 72,
              "输入文件：" + str(input_file), "转储文件：" + str(dump_file),
              "目标模块：" + (index.module if index.module != "unknown" else "未确定"),
              "模块变体：" + VARIANT_ZH[active_variant],
              "提取方式：" + ("按完整变量路径读取顶部摘要" if index.has_summary else "按类与字段注释匹配旧格式转储"),
              "处理条目：{} 项".format(len(rows)), "处理汇总：" + action_summary,
              "只有已找到、已校准、已验证或已推导的有效数值允许写入。",
              "未确认、另一变体、候选、常量及运行时绝对地址保留输入原值；保留不代表已验证。",
              "符号初始化被成功值替换后，仅代表本次选中变体。", ""]
    for row in rows:
        display = dict(row)
        display["new"] = row["new"] if row["new"] != "-" else "无可用数值"
        display["status"] = STATUS_ZH.get(row["status"], "未知状态（{}）".format(row["status"]))
        display["action"] = ACTION_ZH.get(row["action"], "未知处理结果（{}）".format(row["action"]))
        display["source"] = SOURCE_ZH.get(row["source"], "未知来源（{}）".format(row["source"]))
        display["evidence"] = chinese_evidence(row["evidence"])
        report.append("Offset::{key}（第 {line} 行） 原值：{old} → 提取值：{new} | 状态：{status} | 处理：{action} | 来源：{source}\n  依据：{evidence}".format(**display))
    report.append("\n更新后的头文件：" + str(updated_file))
    report_bytes = ("\n".join(report) + "\n").encode("utf-8")
    updated_file.parent.mkdir(parents=True, exist_ok=True)
    Path(output_file).parent.mkdir(parents=True, exist_ok=True)
    updated_file.write_bytes(updated_bytes)
    Path(output_file).write_bytes(report_bytes)
    logging.info("处理 %d 项，更新 %d 项；报告 %s；更新文件 %s", len(rows), actions["UPDATED"], output_file, updated_file)
    return {"rows": rows, "actions": dict(actions), "variant": active_variant,
            "updated_file": str(updated_file), "output_file": str(output_file)}


def setup_logging():
    logging.basicConfig(level=logging.INFO, format="%(asctime)s - %(levelname)s - %(message)s", datefmt="%H:%M:%S")


def main(argv=None):
    setup_logging()
    # 保留用户原有路径配置；命令行参数可覆盖，无需反复编辑脚本。
    input_file = r"D:\NiXiang\dma\dump\Offset.h"
    dump_file = r"D:\NiXiang\dma\dump\dump2026.05.20.cs"
    output_file = r"D:\NiXiang\dma\dump\offset_log.txt"
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", default=input_file, help="原 Offset.h")
    parser.add_argument("--dump", default=dump_file, help="v7 或旧格式 dump.cs")
    parser.add_argument("--output", default=output_file, help="逐项状态与修改报告")
    parser.add_argument("--updated", help="更新头文件，默认输入文件旁的 *_updated.h")
    parser.add_argument("--variant", choices=("auto", "normal", "super"), default="auto",
                        help="默认读取模块标记；无模块标记的旧 dump 可显式指定变体")
    args = parser.parse_args(argv)
    try:
        process_file(args.input, args.dump, args.output, updated_file=args.updated, variant=args.variant)
    except (OSError, UnicodeError, ValueError) as exc:
        logging.error("提取失败: %s", exc)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
