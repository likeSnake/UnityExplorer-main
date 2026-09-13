#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include "../../deps/Naraka/json.hpp"

namespace fs = std::filesystem;
using Json = nlohmann::json;
using ExportFn = int (__cdecl*)(const wchar_t*, const wchar_t*, wchar_t*, size_t);
using VersionFn = unsigned (__cdecl*)();

struct Module {
    HMODULE value = nullptr;
    explicit Module(const fs::path& path) {
        value = LoadLibraryExW(path.c_str(), nullptr, LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
        if (!value) throw std::runtime_error("DLL load failed: " + std::to_string(GetLastError()));
    }
    ~Module() { if (value) FreeLibrary(value); }
};

void fixture(const fs::path& path, const std::string& bytes) {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out.write(bytes.data(), (std::streamsize)bytes.size());
    out.close();
    if (!out) throw std::runtime_error("Cannot write test fixture");
}
std::string textFile(const fs::path& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) throw std::runtime_error("Cannot read output");
    return std::string(std::istreambuf_iterator<char>(in), {});
}
Json jsonFile(const fs::path& path) { return Json::parse(textFile(path)); }

fs::path invoke(ExportFn exportFn, const fs::path& input, const fs::path& output) {
    std::vector<wchar_t> result(32768);
    const int rc = exportFn(input.c_str(), output.c_str(), result.data(), result.size());
    if (rc) {
        std::wcerr << result.data() << L"\n";
        throw std::runtime_error("Export failed: " + std::to_string(rc));
    }
    return fs::path(result.data());
}

int selfTest(ExportFn exportFn, const fs::path& root) {
    const auto run = root / (L"selftest_" + std::to_wstring(GetCurrentProcessId()) + L"_" + std::to_wstring(GetTickCount64()));
    const auto input = run / L"input", outputs = run / L"outputs";
    if (!fs::create_directories(input)) throw std::runtime_error("Test directory collision");
    const std::string battleHeader = "_KEY\t_ID\t_HeroTitle\t_HeroName\t_RelateHeroID\t_Deprecated\t_HeroMappingID\t_EmoteMappingID\t_StandbyMappingID\t_SoulMapping\n";
    const std::string battleRows = "100\t100\t测试女侠\theroine\t101\t0\t0\t200\t201\t200\n"
        "101\t101\t变身\tform\t0\t0\t0\t200\t200\t200\n"
        "102\t102\t【占坑】\tfuture\t0\t0\t0\t200\t200\t200\n";
    const auto battlePath = input / L"HeroConfig_HeroBattleConfig.tsv";
    const auto fashionPath = input / L"HeroFashion_HeroFashion.tsv";
    const std::string fashionHeader = "_KEY\t_ID\t_HeroName\t_IsMainHero\t_ShopUnlock\t_UIHeroMappingID\n";
    const std::string fashionRows = "100\t100\t测试女侠\t1\t1\t200\n101\t101\t形态别名\t0\t0\t0\n102\t102\t【占坑】\t1\t1\t200\n";
    fixture(battlePath, "\xEF\xBB\xBF" + battleHeader + battleRows);
    fixture(fashionPath, fashionHeader + fashionRows);
    fixture(input / L"PlayableMappingConfig_MappingConfig.tsv", "_KEY\t_ID\t_MappingName\n200\t200\thero_emote_group\n");
    fixture(input / L"AutoAnimatorStates_MaxStateMachineConf.tsv", "_KEY\t_ID\t_TagStr\t_LayerIdx\t_MotionClipsList\n99999999\tidle\tstandby\t0\t[1234567]\n");
    int passed = 0, failed = 0;
    auto check = [&](bool ok, const char* description) {
        std::cout << (ok ? "PASS " : "FAIL ") << description << '\n';
        ok ? ++passed : ++failed;
    };
    const auto firstPath = invoke(exportFn, input, outputs);
    Json first = jsonFile(firstPath / L"heroes.json");
    check(first["counts"]["config_records"] == 3, "count config rows");
    check(first["counts"]["main_marked_records"] == 2 && first["counts"]["named_main_records"] == 1, "placeholder separated from main flag");
    check(first["counts"]["related_non_main_forms"] == 1, "related form requires explicit relation and non-main flag");
    check(first["heroes"][0]["battle_title"] == "测试女侠", "UTF-8 and BOM preserved");
    check(first["heroes"][1]["names_differ"] == true, "different source names retained");
    check(first["current_playable_hero_count"].is_null() && first["heroes"][0]["playable_now"].is_null(), "availability stays unknown");
    check(first["counts"]["unresolved_nonzero_mapping_links"] == 1, "missing nonzero mapping reported");
    check(first["hero_action_links"][0]["status"] == "NO_OVERRIDE", "zero mapping is distinct from missing");
    check(first["hero_action_links"][1]["mapping_name"] == "hero_emote_group" && first["hero_action_links"][1]["concrete_action_names"].is_null(), "mapping name not labeled as action");
    check(first["action_states"][0]["state_name"] == "idle" && first["action_states"][0]["action_hash"].is_null(), "unsafe source key not treated as hash");
    check(first["sources"][2]["status"] == "MISSING" && first["heroes"][0]["talent"].is_null(), "optional missing table explicit");
    check(first["sources"][0]["sha256"].get<std::string>().size() == 64, "source digest recorded");
    check(jsonFile(firstPath / L"manifest.json")["products"].size() == 7, "manifest covers all seven products");
    check(textFile(firstPath / L"dump_header.cs").find("// Hero ID=100") != std::string::npos, "comment header generated");
    const auto secondPath = invoke(exportFn, input, outputs);
    check(secondPath != firstPath, "repeat invocation never overwrites output");
    std::vector<wchar_t> result(32768);
    auto reject = [&]() { return exportFn(input.c_str(), outputs.c_str(), result.data(), result.size()) == 2; };
    size_t goodRuns = (size_t)std::distance(fs::directory_iterator(outputs), fs::directory_iterator{});
    fixture(battlePath, battleHeader + battleRows + "100\t100\t重复\tx\t0\t0\t0\t0\t0\t0\n");
    check(reject(), "duplicate IDs rejected");
    fixture(battlePath, battleHeader + "9\t8\tx\tx\t0\t0\t0\t0\t0\t0\n");
    check(reject(), "key ID mismatch rejected");
    fixture(battlePath, battleHeader + "100\t100\tx\n");
    check(reject(), "ragged TSV rejected");
    fixture(battlePath, battleHeader + "100\t100\t" + std::string(1, '\xFF') + "\tx\t0\t0\t0\t0\t0\t0\n");
    check(reject(), "invalid UTF-8 rejected");
    fixture(battlePath, battleHeader + battleRows);
    fixture(fashionPath, fashionHeader + "100\t100\tx\t2\t1\t200\n");
    check(reject(), "invalid Boolean rejected");
    check((size_t)std::distance(fs::directory_iterator(outputs), fs::directory_iterator{}) == goodRuns, "invalid inputs create no successful output");
    check(exportFn(input.c_str(), outputs.c_str(), result.data(), 1) == 3, "insufficient result buffer rejected before export");
    fixture(fashionPath, fashionHeader);
    auto empty = jsonFile(invoke(exportFn, input, outputs) / L"heroes.json");
    check(empty["sources"][1]["status"] == "EMPTY" && empty["counts"]["missing_fashion_rows"] == 3 && empty["heroes"][0]["is_main_config"].is_null(), "empty table distinct from missing or false");
    fixture(fashionPath, fashionHeader + fashionRows);
    fixture(input / L"HeroTalent_HeroTalent.tsv", "_ID\t_DefaultTalentSkillID\t_TalentSkillIDList\t_DefaultTalentUniqueID\t_TalentUniqueIDList\n100\t10\t[11,12]\t20\t[21]\n");
    auto talent = jsonFile(invoke(exportFn, input, outputs) / L"heroes.json");
    check(talent["heroes"][0]["talent"]["skill_ids"] == Json::array({11,12}), "typed skill array exported");
    const std::string detailsHeader = "_ID\t_RelateHero\t_TalentName\t_ComboGraph\t_ComboMappingID\t_Launch\t_Deprecated\n";
    fixture(input / L"TalentConfig_TalentSkill.tsv", detailsHeader + "10\t100\t庇护\thero_graph\t200\t1\t0\n11\t101\t异主技能\tform_graph\t200\t1\t0\n");
    auto detailed = jsonFile(invoke(exportFn, input, outputs) / L"heroes.json");
    check(detailed["hero_skill_links"][0]["skill_name"] == "庇护" && detailed["hero_skill_links"][0]["combo_mapping_name"] == "hero_emote_group", "skill name and combo mapping linked by ID");
    check(detailed["hero_skill_links"][1]["status"] == "OWNER_MISMATCH", "skill ownership mismatch preserved");
    check(detailed["hero_skill_links"][2]["status"] == "MISSING_ID", "missing skill ID reported");
    fixture(input / L"HeroTalent_HeroTalent.tsv", "_ID\t_DefaultTalentSkillID\t_TalentSkillIDList\t_DefaultTalentUniqueID\t_TalentUniqueIDList\n100\t10\t[\"pointer\"]\t20\t[21]\n");
    check(reject(), "wrong array element type rejected");
    fixture(input / L"HeroTalent_HeroTalent.tsv", "_ID\t_DefaultTalentSkillID\t_TalentSkillIDList\t_DefaultTalentUniqueID\t_TalentUniqueIDList\n100\t10\t[18446744073709551615]\t20\t[21]\n");
    check(reject(), "unsigned array overflow rejected");
    Json summary = {{"passed", passed}, {"failed", failed}, {"fixture_directory", run.u8string()}, {"dll_loaded_in_host", true}};
    fixture(run / L"test_result.json", summary.dump(2));
    std::cout << summary.dump(2) << '\n';
    return failed ? 1 : 0;
}

int wmain(int argc, wchar_t** argv) {
    SetConsoleOutputCP(CP_UTF8);
    if (argc != 4) {
        std::wcerr << L"Usage: HeroCatalogHost.exe <DLL> <config-directory> <output-root>\n"
            L"       HeroCatalogHost.exe <DLL> --self-test <test-output-root>\n";
        return 2;
    }
    try {
        Module module(fs::absolute(argv[1]));
        auto version = reinterpret_cast<VersionFn>(GetProcAddress(module.value, "HeroCatalogVersion"));
        auto exportFn = reinterpret_cast<ExportFn>(GetProcAddress(module.value, "ExportHeroCatalogW"));
        if (!version || !exportFn || version() != 1) throw std::runtime_error("Unsupported DLL API version");
        if (std::wstring(argv[2]) == L"--self-test") return selfTest(exportFn, fs::absolute(argv[3]));
        auto output = invoke(exportFn, fs::absolute(argv[2]), fs::absolute(argv[3]));
        std::cout << "OUTPUT=" << output.u8string() << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "ERROR: " << error.what() << '\n';
        return 1;
    }
}
