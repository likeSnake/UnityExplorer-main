"""行为回归：python tools/tests/test_yjwj_offset_extractor.py"""
import codecs
import importlib.util
from pathlib import Path
import sys
import tempfile
import unittest


SCRIPT = Path(__file__).resolve().parents[2] / "dump_workspace/inject_test/YJWJOffset_new.py"
spec = importlib.util.spec_from_file_location("offset_extractor", SCRIPT)
extractor = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = extractor
spec.loader.exec_module(extractor)


def summary(rows, module="GameAssembly_Super.dll"):
    return ("// ===== Offset.h Discovery Summary (v7) =====\n"
            "// Active module: " + module + " base=0x10000 variant=Super/SuperIBT\n" +
            "\n".join("// Offset::{} = {}; // status={} {}".format(*r) for r in rows) +
            "\n// Inventory: total={} resolved=0\n// Image 0: Test.dll - 0\n".format(len(rows)))


def klass(name, fields="", namespace="", header="", parent=""):
    return ("// Class: " + name + " \n// Namespace: " + namespace + "\n" + header +
            "public class " + name + (" : " + parent if parent else "") +
            "\n{\n\t// Fields\n" + fields + "}\n")


class ExtractorTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.folder = Path(self.temp.name)
        self.input = self.folder / "Offset.h"
        self.dump = self.folder / "dump.cs"
        self.report = self.folder / "result.txt"
        self.updated = self.folder / "Offset_updated.h"

    def run_extract(self, header, dump, encoding="utf-8", variant="auto"):
        original = header.encode(encoding)
        self.input.write_bytes(original)
        self.dump.write_bytes(dump.encode("utf-8"))
        result = extractor.process_file(str(self.input), str(self.dump), str(self.report), variant=variant)
        self.assertEqual(self.input.read_bytes(), original)
        self.assertEqual(self.dump.read_bytes(), dump.encode("utf-8"))
        return result, self.updated.read_bytes().decode(encoding)

    def test_anonymous_nested_paths_and_non_declarations(self):
        header = '''namespace Offset {
          inline uint64_t InitActorKitRefer = MemoryShockUnresolvedRva;
          // struct { DWORD bogus = 1; } Bogus;
          struct { DWORD Status = 0; struct { DWORD Status = 2; } Inner; } A;
          struct { DWORD Status = 3; } B;
          enum E { Choice = 1 };
          void f() { static const uint64_t local = 4; B.Status = 1; }
          const char* text = "{ uint64_t fake = 9; }";
        }'''
        self.assertEqual([d.key for d in extractor.parse_declarations(header)],
                         ["InitActorKitRefer", "A.Status", "A.Inner.Status", "B.Status"])

    def test_memory_offsets_without_class_and_symbolic_initializers(self):
        header = ("namespace Offset {\n"
                  "inline uint64_t InitActorKitRefer = MemoryShockUnresolvedRva; // 保留中文\n"
                  "inline constexpr uint32_t Il2CppClassMethodCountOffset = 0x110;\n"
                  "inline uint64_t CrossFadeInFixedTime = OldMethod;\n}")
        data = summary([("InitActorKitRefer", "0x376ABF8", "FOUND", "TypeInfo RVA"),
                        ("Il2CppClassMethodCountOffset", "0x120", "CALIBRATED", "layout"),
                        ("CrossFadeInFixedTime", "0x797D760", "FOUND", "method")])
        result, updated = self.run_extract(header, data)
        self.assertEqual(result["actions"], {"UPDATED": 3})
        self.assertIn("= 0x376ABF8; // 保留中文", updated)
        self.assertIn("= 0x120;", updated)
        self.assertIn("= 0x797D760;", updated)

    def test_full_paths_prevent_same_member_collisions(self):
        result, updated = self.run_extract(
            "namespace Offset { struct { DWORD Status = 1; } A; struct { DWORD Status = 2; } B; }",
            summary([("A.Status", "0x60", "FOUND", "A"), ("B.Status", "0x34", "FOUND", "B")]))
        self.assertEqual(result["actions"]["UPDATED"], 2)
        self.assertIn("Status = 0x60; } A", updated)
        self.assertIn("Status = 0x34; } B", updated)

    def test_status_gate_preserves_unsafe_values_and_constants(self):
        statuses = ["NOT_FOUND", "NOT_LOADED", "NOT_DUMPED", "UNRESOLVED", "UNRESOLVED_LAYOUT",
                    "AMBIGUOUS", "CANDIDATE", "SCAN_INCOMPLETE", "SKIPPED_LEGACY", "OUTSIDE_MODULE",
                    "NON_EXECUTABLE", "UNREADABLE", "CONFIG_CONSTANT", "ABI_LAYOUT", "RUNTIME_VA", "FUTURE_STATUS"]
        header = "namespace Offset {\n" + "\n".join("DWORD item{} = 1;".format(i) for i in range(len(statuses))) + "\n}"
        data = summary([("item" + str(i), "0x999", status, "not safe to apply") for i, status in enumerate(statuses)])
        result, updated = self.run_extract(header, data)
        self.assertEqual(header, updated)
        self.assertTrue(all(r["action"].startswith("RETAINED_") for r in result["rows"]))

    def test_success_statuses_and_zero_static_offset(self):
        statuses = ["FOUND", "CALIBRATED", "VERIFIED", "DERIVED"]
        header = "namespace Offset {\n" + "\n".join("DWORD item{} = 1;".format(i) for i in range(4)) + "\n}"
        result, updated = self.run_extract(header, summary([("item" + str(i), "0x0", s, "static/layout") for i, s in enumerate(statuses)]))
        self.assertEqual(result["actions"]["UPDATED"], 4)
        self.assertEqual(updated.count("= 0x0;"), 4)

    def test_equal_numbers_preserve_spelling_and_comments(self):
        header = "namespace Offset { DWORD Value  =  216; // Class: Old->bad 0x555\r\n}"
        result, updated = self.run_extract(header, summary([("Value", "0xD8", "FOUND", "Class: Actual->x")]))
        self.assertEqual(updated, header)
        self.assertEqual(result["rows"][0]["action"], "UNCHANGED")

    def test_summary_is_authoritative_over_class_even_on_failure(self):
        header = "namespace Offset { DWORD Value = 1; // Class: ActorModel->objectMsg\n}"
        body = klass("ActorModel", "\tpublic Object objectMsg; // offset:0xD8\n")
        for status in ("NOT_FOUND", "CANDIDATE"):
            with self.subTest(status=status):
                result, updated = self.run_extract(header, summary([("Value", "0xE8", status, "")]) + body)
                self.assertEqual(updated, header)
                self.assertEqual(result["rows"][0]["status"], status)

    def test_summary_missing_entry_cannot_fall_back_to_old_class(self):
        header = "namespace Offset { DWORD Value = 1; // Class: ActorModel->objectMsg\n}"
        body = klass("ActorModel", "\tpublic Object objectMsg; // offset:0xD8\n")
        result, updated = self.run_extract(header, summary([]) + body)
        self.assertEqual(updated, header)
        self.assertEqual(result["rows"][0]["status"], "SUMMARY_MISSING")

    def test_unknown_variant_prevents_legacy_normal_super_mix(self):
        header = "namespace Offset {\ninline uint64_t m_GlobalTime = 1; // Class: GameBaseObject\ninline uint64_t m_GlobalTime2 = 2; // Class: GameBaseObject\n}"
        body = klass("GameBaseObject", header="// RVA: 0x100\n")
        result, updated = self.run_extract(header, body)
        self.assertEqual(updated, header)
        self.assertTrue(all(r["status"] == "VARIANT_UNKNOWN" for r in result["rows"]))
        result, updated = self.run_extract(header, body, variant="normal")
        self.assertIn("m_GlobalTime = 0x100;", updated)
        self.assertIn("m_GlobalTime2 = 2;", updated)

    def test_variant_conflict_and_duplicate_summary_abort_before_write(self):
        self.input.write_text("namespace Offset { DWORD Value = 1; }", encoding="utf-8")
        self.dump.write_text(summary([("Value", "0x10", "FOUND", "")]), encoding="utf-8")
        with self.assertRaisesRegex(ValueError, "variant"):
            extractor.process_file(str(self.input), str(self.dump), str(self.report), variant="normal")
        self.assertFalse(self.updated.exists())
        duplicate = summary([("Value", "0x10", "FOUND", ""), ("Value", "0x20", "FOUND", "")])
        self.dump.write_text(duplicate, encoding="utf-8")
        with self.assertRaisesRegex(ValueError, "重复"):
            extractor.process_file(str(self.input), str(self.dump), str(self.report))
        self.assertFalse(self.updated.exists())

    def test_truncated_or_invalid_summary_aborts(self):
        self.input.write_text("namespace Offset { DWORD Value = 1; }", encoding="utf-8")
        cases = [summary([("Value", "0x10", "FOUND", "")]).replace("total=1", "total=2"),
                 "// ===== Offset.h Discovery Summary (v7) =====\n",
                 summary([("Value", "NOT_FOUND", "FOUND", "")])]
        for data in cases:
            with self.subTest(data=data):
                self.dump.write_text(data, encoding="utf-8")
                with self.assertRaises(ValueError):
                    extractor.process_file(str(self.input), str(self.dump), str(self.report))
                self.assertFalse(self.updated.exists())

    def test_legacy_backing_field_parent_namespace_and_ambiguous_classes(self):
        body = (klass("Base", "\tprivate Int32 <Value>k__BackingField; // offset:0x28\n", "Good") +
                klass("Child", namespace="Good", parent="Base") +
                klass("Child", "\tprivate Int32 Value; // offset:0x60\n", "Other"))
        header = "namespace Offset {\nDWORD Good = 1; // Class: Good.Child->Int32 Value\nDWORD Ambiguous = 2; // Class: Child->Value\n}"
        result, updated = self.run_extract(header, body)
        self.assertIn("Good = 0x28;", updated)
        self.assertIn("Ambiguous = 2;", updated)
        self.assertEqual(result["rows"][1]["status"], "AMBIGUOUS")

    def test_legacy_duplicate_names_match_unique_member_and_case(self):
        body = (klass("UserData", "\tpublic Boolean success; // offset:0x10\n") +
                klass("UserData", "\tpublic Object battleData; // offset:0x38\n"))
        header = "namespace Offset {\nDWORD Data = 1; // Class: UserData->Object battleData\nDWORD WrongCase = 2; // Class: UserData->BattleData\n}"
        result, updated = self.run_extract(header, body)
        self.assertIn("Data = 0x38;", updated)
        self.assertEqual(result["rows"][1]["status"], "NOT_FOUND")

    def test_legacy_method_rva_is_not_a_typeinfo_rva(self):
        body = klass("NoSlot") + "\t// Methods\n\t// RVA: 0x999 VA: 0x10999\n\tpublic Void Run() { }\n"
        result, updated = self.run_extract("namespace Offset { DWORD Value = 1; // Class: NoSlot\n}", body)
        self.assertEqual(result["rows"][0]["status"], "NOT_FOUND")
        self.assertIn("Value = 1", updated)

    def test_legacy_generic_zero_layout_and_literal(self):
        body = klass("List`1", "\tpublic Object _items; // offset:0x0\n\tpublic static Int32 Zero; // offset:0x0\n\tpublic const Int32 Literal; // size:0x4\n")
        header = "namespace Offset {\nDWORD Items = 16; // Class: List`1->_items\nDWORD Static = 1; // Class: List`1->Zero\nDWORD Constant = 8; // Class: List`1->Literal\n}"
        result, updated = self.run_extract(header, body)
        self.assertEqual([r["status"] for r in result["rows"]], ["UNRESOLVED_LAYOUT", "FOUND", "NOT_FOUND"])
        self.assertIn("Static = 0x0", updated)
        self.assertIn("Items = 16", updated)

    def test_bom_crlf_gbk_and_no_final_newline_are_preserved(self):
        header = "namespace Offset {\r\n\tDWORD Value  =  1; // 中文 123\r\n}"
        expected = header.replace("=  1;", "=  0x20;")
        for encoding in ("utf-8", "utf-8-sig", "gbk", "utf-16"):
            with self.subTest(encoding=encoding):
                self.run_extract(header, summary([("Value", "0x20", "FOUND", "")]), encoding)
                self.assertEqual(self.updated.read_bytes(), expected.encode(encoding))

    def test_out_of_range_and_module_va_are_not_applied(self):
        header = "namespace Offset { DWORD TooWide = 1; inline uint64_t GameAssembly = 0; }"
        result, updated = self.run_extract(header, summary([("TooWide", "0x100000000", "FOUND", ""),
                                                            ("GameAssembly", "0x123456780000", "FOUND", "module")]))
        self.assertEqual(updated, header)
        self.assertEqual(result["rows"][0]["action"], "RETAINED_OUT_OF_RANGE")
        self.assertEqual(result["rows"][1]["action"], "RETAINED_RUNTIME")

    def test_output_collision_and_duplicate_header_keys_are_rejected(self):
        self.input.write_text("namespace Offset { DWORD Value = 1; }", encoding="utf-8")
        self.dump.write_text(summary([]), encoding="utf-8")
        with self.assertRaises(ValueError):
            extractor.process_file(str(self.input), str(self.dump), str(self.input))
        with self.assertRaisesRegex(ValueError, "重复"):
            extractor.parse_declarations("namespace Offset { DWORD Value = 1; DWORD Value = 2; }")


if __name__ == "__main__":
    unittest.main(verbosity=2)
