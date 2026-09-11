import re
import os
import logging
from typing import List, Tuple, Optional

def detect_encoding(file_path: str) -> str:
    """检测文件编码"""
    encodings = ['utf-8', 'gbk', 'latin-1', 'cp1252', 'iso-8859-1']
    
    for encoding in encodings:
        try:
            with open(file_path, 'r', encoding=encoding) as f:
                f.read()
            return encoding
        except UnicodeDecodeError:
            continue
    
    # 如果常见编码都失败，使用utf-8并忽略错误
    return 'utf-8'

def parse_class_line(line: str) -> Tuple[Optional[str], Optional[str]]:
    """解析包含Class:的行，返回类名和字段信息"""
    class_pattern = r'Class:\s*([^->\n]+)(?:->\s*(.+))?'
    match = re.search(class_pattern, line)
    
    if not match:
        return None, None
    
    class_name = match.group(1).strip()
    field_info = match.group(2).strip() if match.group(2) else None
    
    return class_name, field_info

def extract_offset_from_dump(dump_file: str, encoding: str, target_class: str, target_field: Optional[str]) -> Optional[str]:
    """从dump文件中提取指定类和字段的偏移量"""
    try:
        with open(dump_file, 'r', encoding=encoding) as f:
            content = f.read()
    except Exception as e:
        logging.error(f"读取dump文件失败: {e}")
        return None
    
    # 构建类定义的正则模式
    class_pattern = rf'^//\s*Class:\s*{re.escape(target_class)}\s*$'
    class_match = re.search(class_pattern, content, re.MULTILINE)

    
    if not class_match:
        logging.warning(f"在dump文件中未找到类: {target_class}")
        return None
    
    # 找到类定义的开始位置
    class_start = class_match.start()
    
    # 找到下一个类定义或文件结尾
    next_class_pattern = r'//\s*Class:\s*'
    next_class_match = re.search(next_class_pattern, content[class_start + 1:])
    
    if next_class_match:
        class_end = class_start + 1 + next_class_match.start()
    else:
        class_end = len(content)
    
    # 提取类定义范围内的内容
    class_content = content[class_start:class_end]
    
    # 如果有字段信息，则查找字段偏移量
    if target_field:
        # 构建字段匹配的正则模式（全字匹配）
        field_pattern = r'\b' + re.escape(target_field) + r'\b[^;]*//\s*offset:\s*(0x[0-9a-fA-F]+|\d+)'
        field_match = re.search(field_pattern, class_content)
        
        if field_match:
            return field_match.group(1)
        else:
            # 尝试另一种可能的格式
            alt_pattern = r'\b' + re.escape(target_field) + r'\b.*?offset:\s*(0x[0-9a-fA-F]+|\d+)'
            alt_match = re.search(alt_pattern, class_content)
            if alt_match:
                return alt_match.group(1)
            logging.warning(f"在类 {target_class} 中未找到字段 {target_field} 的偏移量")
            return None
    else:
        # 如果没有字段信息，则查找类的RVA偏移值
        # 找到从当前类开始到"internal"或"public"或"private"之间的内容
        access_modifier_pattern = r'\b(internal|public|private)\b'
        access_match = re.search(access_modifier_pattern, class_content)
        
        if access_match:
            # 提取从类定义开始到访问修饰符之间的内容
            header_content = class_content[:access_match.start()]
            # logging.warning(f"\n头部值 {header_content}")
            # 在头部内容中查找RVA值
            rva_pattern = r'RVA:\s*(0x[0-9a-fA-F]+)'
            rva_match = re.search(rva_pattern, header_content)
            
            if rva_match:
                return rva_match.group(1)
            else:
                logging.warning(f"在类 {target_class} 的头部未找到RVA值")
                return None
        else:
            logging.warning(f"在类 {target_class} 的定义中未找到访问修饰符")
            return None

# ---------- 新增/替换的辅助函数与 process_file ----------

def find_assignment_value_in_line(line: str) -> Optional[str]:
    """
    在行的赋值部分（'=' 到 ';'）查找第一个数值（16 进制或十进制）。
    只查找在注释之前的代码部分（避免从注释里匹配到无关数字）。
    返回匹配到的字符串（例如 '0xC628D78' 或 '12345'），找不到返回 None。
    """
    # 先把注释部分分离（'//'）
    comment_pos = line.find('//')
    code_part = line if comment_pos == -1 else line[:comment_pos]

    # 找第一个形如 "= 0x...;" 或 "= 1234;" 的值
    m = re.search(r'=\s*(0x[0-9A-Fa-f]+|\d+)\s*;', code_part)
    if m:
        return m.group(1)
    return None

def replace_assignment_value_in_line(line: str, new_value: str) -> Tuple[str, Optional[str]]:
    """
    将行中注释之前的第一个赋值数值替换为 new_value。
    返回 (new_line, old_value)；若未找到赋值则返回 (line, None)。
    """
    comment_pos = line.find('//')
    code_part = line if comment_pos == -1 else line[:comment_pos]
    comment_part = '' if comment_pos == -1 else line[comment_pos:]

    m = re.search(r'=\s*(0x[0-9A-Fa-f]+|\d+)\s*;', code_part)
    if not m:
        return line, None

    old_val = m.group(1)
    # 构造新的 code_part（保持 '=' 和 ';' 周围空格）
    start, end = m.span(1)  # span of the value itself within code_part
    # 替换值：使用正则替换更鲁棒
    new_code_part = re.sub(r'=\s*(0x[0-9A-Fa-f]+|\d+)\s*;', f'= {new_value};', code_part, count=1)
    new_line = new_code_part + comment_part
    return new_line, old_val

def process_file(input_file: str, dump_file: str, output_file: str):
    """处理主函数（替换写回并保存新文件）"""
    # 检测文件编码
    input_encoding = detect_encoding(input_file)
    dump_encoding = detect_encoding(dump_file)

    logging.info(f"输入文件编码: {input_encoding}")
    logging.info(f"dump文件编码: {dump_encoding}")

    # 读取输入文件全部行（保留换行符以便写回）
    try:
        with open(input_file, 'r', encoding=input_encoding, errors='ignore') as f:
            lines = f.readlines()
    except Exception as e:
        logging.error(f"读取输入文件失败: {e}")
        return

    results = []
    changes = []  # 记录实际替换的项：dict(line_num, class, field, old, new)

    # 为了兼容你原来的行为，逐行处理（保留 line_num 起始 1）
    for idx, raw_line in enumerate(lines):
        line_num = idx + 1
        line = raw_line.strip()
        if not line:
            continue

        class_name, field_info = parse_class_line(line)
        if class_name is None:
            logging.info(f"行 {line_num}: 未找到Class信息")
            continue

        logging.info(f"行 {line_num}: 处理类 '{class_name}', 字段 '{field_info}'")

        # 从dump文件中提取偏移量（保持你原有的提取实现）
        offset = extract_offset_from_dump(dump_file, dump_encoding, class_name, field_info)

        if offset:
            # 记录结果（文本日志）
            if field_info:
                result = f"类: {class_name} -> 字段: {field_info} -> 偏移量: {offset}"
            else:
                result = f"类: {class_name} -> RVA偏移值: {offset}"
            results.append(result)
            logging.info(f"找到偏移量: {offset}")

            # 现在尝试在原输入文件行中替换旧值（如果有）
            # 注意：lines[idx] 包含换行符，我们要保留它
            orig_line_full = lines[idx]
            # 先在原始行中找旧值
            old_val = find_assignment_value_in_line(orig_line_full)
            if old_val:
                # 如果旧值与新 offset 不同（字符串比较 - preserve 0x vs decimal）
                # 标准化比较：小写比较 hex 前缀不变
                if old_val.lower() != offset.lower():
                    new_line, replaced_old = replace_assignment_value_in_line(orig_line_full, offset)
                    if replaced_old:
                        # 保持最后的换行符（如果原行有）
                        newline_ending = '\n' if orig_line_full.endswith('\n') else ''
                        lines[idx] = new_line.rstrip('\n') + newline_ending
                        changes.append({
                            'line_num': line_num,
                            'class': class_name,
                            'field': field_info,
                            'old': replaced_old,
                            'new': offset
                        })
                        logging.info(f"已替换 行 {line_num} 中的值: {replaced_old} -> {offset}")
                    else:
                        logging.warning(f"行 {line_num}: 寻找要替换的旧值失败（意外）")
                else:
                    logging.info(f"行 {line_num}: 现有值与提取到的偏移量相同，无需替换")
            else:
                logging.warning(f"行 {line_num}: 未在该行找到可替换的赋值格式（没有找到 '=' ... ';' 匹配）")

        else:
            # 未找到偏移量
            if field_info:
                result = f"类: {class_name} -> 字段: {field_info} -> 偏移量: 未找到"
            else:
                result = f"类: {class_name} -> RVA偏移值: 未找到"
            results.append(result)

    # 写入结果日志到 output_file（保留你原先的格式）
    try:
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write("偏移量提取结果:\n")
            f.write("=" * 50 + "\n")
            for result in results:
                f.write(result + "\n")
            f.write("=" * 50 + "\n")
            f.write(f"总共处理: {len(results)} 个条目\n\n")

            # 额外写入替换记录
            if changes:
                f.write("已替换的值:\n")
                for c in changes:
                    f.write(f"行 {c['line_num']}: 类 {c['class']}")
                    if c['field']:
                        f.write(f" 字段 {c['field']}")
                    f.write(f" -> {c['old']}  -> {c['new']}\n")
                f.write(f"\n总共替换: {len(changes)} 项\n")
            else:
                f.write("未发生任何替换。\n")

        logging.info(f"结果已写入日志: {output_file}")
    except Exception as e:
        logging.error(f"写入输出文件时出错: {e}")
        return

    # 将修改后的 input 文件保存为新的文件（添加 _updated 后缀）
    base, ext = os.path.splitext(input_file)
    modified_file = base + '_updated' + ext
    try:
        with open(modified_file, 'w', encoding=input_encoding) as f:
            f.writelines(lines)
        logging.info(f"已保存修改后的文件: {modified_file}")
        print(f"处理完成! 结果保存在: {output_file}")
        print(f"修改后的输入文件已保存为: {modified_file}")
    except Exception as e:
        logging.error(f"写入修改后文件时出错: {e}")
        return


def setup_logging():
    """设置日志配置"""
    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s - %(levelname)s - %(message)s',
        datefmt='%Y-%m-%d %H:%M:%S'
    )

def main():
    """主函数"""
    setup_logging()
    
    # 文件路径配置
    input_file =  r"D:\NiXiang\dma\dump\Offset.h"     # 包含Class信息的输入文件
    dump_file = r"D:\NiXiang\dma\dump\dump2026.05.20.cs"        # dump文件
    output_file =  r"D:\NiXiang\dma\dump\offset_log.txt"  # 输出日志文件
    
    # 检查文件是否存在
    if not os.path.exists(input_file):
        print(f"错误: 输入文件 '{input_file}' 不存在")
        return
    
    if not os.path.exists(dump_file):
        print(f"错误: dump文件 '{dump_file}' 不存在")
        return
    
    print("开始处理文件...")
    process_file(input_file, dump_file, output_file)

if __name__ == "__main__":
    main()