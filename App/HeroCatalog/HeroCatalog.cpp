#include "HeroCatalog.h"
#include <Windows.h>
#include <bcrypt.h>
#include <algorithm>
#include <charconv>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include "../../deps/Naraka/json.hpp"

namespace {
namespace fs = std::filesystem;
using Json = nlohmann::ordered_json;
using Row = std::map<std::string, std::string>;
constexpr size_t kMaxFileBytes = 64 * 1024 * 1024;
constexpr size_t kMaxRows = 200000;
constexpr const char* kBattle = "HeroConfig_HeroBattleConfig.tsv";
constexpr const char* kFashion = "HeroFashion_HeroFashion.tsv";
constexpr const char* kTalent = "HeroTalent_HeroTalent.tsv";
constexpr const char* kMapping = "PlayableMappingConfig_MappingConfig.tsv";
constexpr const char* kStates = "AutoAnimatorStates_MaxStateMachineConf.tsv";
constexpr const char* kTalentDetails = "TalentConfig_TalentSkill.tsv";

std::wstring wide(const std::string& text) {
    if (text.empty()) return {};
    int n = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.data(), (int)text.size(), nullptr, 0);
    if (!n) throw std::runtime_error("Invalid UTF-8 input");
    std::wstring out(n, L'\0');
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.data(), (int)text.size(), out.data(), n);
    return out;
}

std::string sha256(const std::string& bytes) {
    BCRYPT_ALG_HANDLE algorithm = nullptr;
    if (BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0) < 0)
        throw std::runtime_error("Cannot initialize SHA-256");
    unsigned char hash[32] = {};
    auto status = BCryptHash(algorithm, nullptr, 0, (PUCHAR)bytes.data(), (ULONG)bytes.size(), hash, sizeof(hash));
    BCryptCloseAlgorithmProvider(algorithm, 0);
    if (status < 0) throw std::runtime_error("SHA-256 failed");
    std::ostringstream out;
    out << std::hex << std::setfill('0');
    for (unsigned char c : hash) out << std::setw(2) << (unsigned)c;
    return out.str();
}

std::string readFile(const fs::path& path) {
    std::ifstream in(path, std::ios::binary | std::ios::ate);
    if (!in) throw std::runtime_error("Cannot open input: " + path.u8string());
    const auto size = in.tellg();
    if (size < 0 || (uint64_t)size > kMaxFileBytes) throw std::runtime_error("Invalid input size: " + path.u8string());
    std::string bytes((size_t)size, '\0');
    in.seekg(0);
    if (!bytes.empty() && !in.read(bytes.data(), (std::streamsize)bytes.size()))
        throw std::runtime_error("Input read failed: " + path.u8string());
    // Reject concurrent growth/truncation rather than hashing only a prefix.
    if (in.peek() != std::char_traits<char>::eof()) throw std::runtime_error("Input changed while reading");
    wide(bytes);
    if (bytes.find('\0') != std::string::npos) throw std::runtime_error("NUL in TSV input");
    return bytes;
}

std::vector<std::string> split(const std::string& text) {
    std::vector<std::string> cells;
    size_t start = 0;
    for (;;) {
        auto end = text.find('\t', start);
        cells.push_back(text.substr(start, end == std::string::npos ? end : end - start));
        if (end == std::string::npos) break;
        start = end + 1;
    }
    return cells;
}

struct Table {
    std::string name;
    bool present = false;
    std::vector<Row> rows;
    std::map<std::string, size_t> byId;
    Json source;
    const Row* get(const std::string& id) const {
        auto it = byId.find(id);
        return it == byId.end() ? nullptr : &rows[it->second];
    }
};

const std::string& cell(const Row& row, const std::string& key) {
    auto it = row.find(key);
    if (it == row.end()) throw std::runtime_error("Missing field: " + key);
    return it->second;
}
int number(const Row& row, const std::string& key) {
    const auto& text = cell(row, key);
    int value = 0;
    auto parsed = std::from_chars(text.data(), text.data() + text.size(), value);
    if (parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size())
        throw std::runtime_error("Invalid Int32 " + key + "=" + text);
    return value;
}
bool boolean(const Row& row, const std::string& key) {
    const int value = number(row, key);
    if (value != 0 && value != 1) throw std::runtime_error("Invalid Boolean " + key);
    return value != 0;
}
Json intArray(const Row& row, const std::string& key) {
    Json value = Json::parse(cell(row, key));
    if (!value.is_array()) throw std::runtime_error("Expected Int32[]: " + key);
    for (const auto& n : value) {
        if (!n.is_number_integer() || (n.is_number_unsigned() && n.get<uint64_t>() > INT32_MAX) ||
            (!n.is_number_unsigned() && (n.get<int64_t>() < INT32_MIN || n.get<int64_t>() > INT32_MAX)))
            throw std::runtime_error("Invalid Int32[] element: " + key);
    }
    return value;
}

Table load(const fs::path& root, const char* name, bool required,
           std::initializer_list<const char*> columns, bool numericId = true) {
    Table out;
    out.name = name;
    fs::path path = root / name;
    out.source = {{"file", path.u8string()}, {"status", "MISSING"}, {"rows", nullptr}, {"sha256", nullptr}};
    if (!fs::exists(path)) {
        if (required) throw std::runtime_error("Required table missing: " + path.u8string());
        return out;
    }
    std::string bytes = readFile(path);
    out.source["sha256"] = sha256(bytes);
    out.source["bytes"] = bytes.size();
    if (bytes.compare(0, 3, "\xEF\xBB\xBF") == 0) bytes.erase(0, 3);
    std::istringstream stream(bytes);
    std::string line;
    if (!std::getline(stream, line)) throw std::runtime_error("Empty file: " + out.name);
    if (!line.empty() && line.back() == '\r') line.pop_back();
    auto header = split(line);
    std::set<std::string> headers;
    for (const auto& h : header)
        if (h.empty() || !headers.insert(h).second) throw std::runtime_error("Invalid/duplicate header: " + out.name);
    for (const char* col : columns)
        if (!headers.count(col)) throw std::runtime_error(out.name + ": missing column " + col);
    if (!headers.count("_ID")) throw std::runtime_error(out.name + ": missing _ID");
    size_t lineNo = 1;
    while (std::getline(stream, line)) {
        ++lineNo;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;
        auto cells = split(line);
        if (cells.size() != header.size())
            throw std::runtime_error(out.name + ": column count mismatch at line " + std::to_string(lineNo));
        if (out.rows.size() >= kMaxRows) throw std::runtime_error(out.name + ": row limit exceeded");
        Row row;
        for (size_t i = 0; i < header.size(); ++i) row.emplace(header[i], std::move(cells[i]));
        const std::string id = cell(row, "_ID");
        if (id.empty()) throw std::runtime_error(out.name + ": empty ID");
        if (numericId) {
            int parsed = number(row, "_ID");
            if (parsed <= 0 || std::to_string(parsed) != id) throw std::runtime_error(out.name + ": noncanonical ID " + id);
            if (headers.count("_KEY") && cell(row, "_KEY") != id)
                throw std::runtime_error(out.name + ": _KEY/_ID mismatch for " + id);
        }
        if (!out.byId.emplace(id, out.rows.size()).second) throw std::runtime_error(out.name + ": duplicate ID " + id);
        out.rows.push_back(std::move(row));
    }
    out.present = true;
    out.source["status"] = out.rows.empty() ? "EMPTY" : "PARSED";
    out.source["rows"] = out.rows.size();
    return out;
}

std::string tsvCell(const std::string& value) {
    std::string out;
    for (char c : value) {
        switch (c) {
        case '\\': out += "\\\\"; break;
        case '\r': out += "\\r"; break;
        case '\n': out += "\\n"; break;
        case '\t': out += "\\t"; break;
        default: out += c;
        }
    }
    return out;
}
std::string valueText(const Json& value) { return value.is_string() ? value.get<std::string>() : value.dump(); }
std::string tsv(const Json& rows, const std::vector<std::string>& columns) {
    std::string out = "\xEF\xBB\xBF";
    auto add = [&](const std::vector<std::string>& cells) {
        for (size_t i = 0; i < cells.size(); ++i) { if (i) out += '\t'; out += tsvCell(cells[i]); }
        out += "\r\n";
    };
    add(columns);
    for (const auto& row : rows) {
        std::vector<std::string> cells;
        for (const auto& key : columns) cells.push_back(row.contains(key) ? valueText(row[key]) : "null");
        add(cells);
    }
    return out;
}

bool placeholder(const std::string& text) { return text.find("占坑") != std::string::npos; }

Json analyze(const fs::path& input) {
    const Table battle = load(input, kBattle, true, {"_HeroTitle", "_HeroName", "_RelateHeroID", "_Deprecated",
        "_HeroMappingID", "_EmoteMappingID", "_StandbyMappingID", "_SoulMapping"});
    const Table fashion = load(input, kFashion, false, {"_HeroName", "_IsMainHero", "_ShopUnlock", "_UIHeroMappingID"});
    const Table talent = load(input, kTalent, false, {"_DefaultTalentSkillID", "_TalentSkillIDList", "_DefaultTalentUniqueID", "_TalentUniqueIDList"});
    const Table mapping = load(input, kMapping, false, {"_MappingName"});
    const Table states = load(input, kStates, false, {"_TagStr", "_LayerIdx"}, false);
    const Table details = load(input, kTalentDetails, false, {"_RelateHero", "_TalentName", "_ComboGraph", "_ComboMappingID", "_Launch", "_Deprecated"});
    Json report = {{"schema_version", 1}, {"source_kind", "FILE_SNAPSHOT"}, {"source_directory", input.u8string()},
        {"live_process_verified", false}, {"current_playable_hero_count", nullptr},
        {"full_game_catalog_verified", false}, {"sources", Json::array()},
        {"warnings", Json::array()}, {"counts", Json::object()}, {"heroes", Json::array()},
        {"hero_action_links", Json::array()}, {"action_states", Json::array()}, {"hero_skill_links", Json::array()}};
    for (const Table* table : {&battle, &fashion, &talent, &mapping, &states, &details}) {
        report["sources"].push_back(table->source);
        if (!table->present) report["warnings"].push_back("缺少可选输入表：" + table->name);
    }
    report["warnings"].push_back("统计仅对应输入文件；尚未验证当前版本、完整缓存覆盖或英雄开放状态。");
    report["warnings"].push_back("主英雄标记不等于已开放；占位名称、变身关系与主英雄标记分别保留。");
    report["warnings"].push_back("动作映射名是配置组名，不等于具体动作；状态目录未关联英雄，未推算 Hash。");
    report["warnings"].push_back("技能名称来自天赋技能表；连招图名称不是动画片段名，技能 Launch 标记不代表英雄当前开放。");
    std::map<int, std::vector<int>> referencedBy;
    for (const auto& row : battle.rows) {
        int id = number(row, "_ID"), related = number(row, "_RelateHeroID");
        if (related < 0) throw std::runtime_error("Negative related hero ID");
        if (related) {
            referencedBy[related].push_back(id);
            if (related == id || !battle.get(std::to_string(related)))
                report["warnings"].push_back("英雄关联异常：" + std::to_string(id) + " -> " + std::to_string(related));
        }
    }
    int markedMain = 0, namedMain = 0, placeholders = 0, relatedForms = 0, missingFashion = 0;
    int missingMappings = 0, namesDiffer = 0;
    for (const auto& row : battle.rows) {
        const auto& id = cell(row, "_ID");
        const Row* f = fashion.get(id);
        const Row* skills = talent.get(id);
        const bool isPlaceholder = placeholder(cell(row, "_HeroTitle")) || (f && placeholder(cell(*f, "_HeroName")));
        const bool isMain = f && boolean(*f, "_IsMainHero");
        const bool isRelatedForm = referencedBy.count(number(row, "_ID")) != 0;
        std::string category = isPlaceholder ? "PLACEHOLDER" :
            (isRelatedForm && f && !isMain ? "RELATED_NON_MAIN_FORM" : (isMain ? "NAMED_MAIN_CONFIG" : "UNCLASSIFIED"));
        markedMain += isMain; namedMain += isMain && !isPlaceholder;
        placeholders += isPlaceholder; relatedForms += category == "RELATED_NON_MAIN_FORM";
        missingFashion += !f;
        Json hero = {{"hero_id", number(row, "_ID")}, {"battle_title", cell(row, "_HeroTitle")},
            {"internal_name", cell(row, "_HeroName")}, {"display_name", f ? Json(cell(*f, "_HeroName")) : Json(nullptr)},
            {"is_main_config", f ? Json(isMain) : Json(nullptr)}, {"placeholder", isPlaceholder},
            {"category", category}, {"related_hero_id", number(row, "_RelateHeroID")},
            {"related_from_hero_ids", isRelatedForm ? Json(referencedBy.at(number(row, "_ID"))) : Json::array()},
            {"deprecated", boolean(row, "_Deprecated")}, {"shop_unlock_config", f ? Json(boolean(*f, "_ShopUnlock")) : Json(nullptr)},
            {"playable_now", nullptr}, {"fashion_status", f ? "FOUND" : (fashion.present ? "MISSING_ROW" : "MISSING_TABLE")},
            {"talent", nullptr}};
        const bool differs = f && cell(row, "_HeroTitle") != cell(*f, "_HeroName");
        hero["names_differ"] = differs;
        namesDiffer += differs;
        if (skills) hero["talent"] = {{"default_skill_id", number(*skills, "_DefaultTalentSkillID")},
            {"skill_ids", intArray(*skills, "_TalentSkillIDList")},
            {"default_ultimate_id", number(*skills, "_DefaultTalentUniqueID")},
            {"ultimate_ids", intArray(*skills, "_TalentUniqueIDList")}};
        report["heroes"].push_back(std::move(hero));
        auto link = [&](const Row& owner, const char* table, const char* field) {
            const int mapId = number(owner, field);
            if (mapId < 0) throw std::runtime_error("Negative mapping ID");
            const Row* target = mapId ? mapping.get(std::to_string(mapId)) : nullptr;
            const char* status = !mapId ? "NO_OVERRIDE" : !mapping.present ? "MISSING_TABLE" : target ? "FOUND" : "MISSING_ID";
            if (mapId && !target) ++missingMappings;
            report["hero_action_links"].push_back({{"hero_id", number(row, "_ID")}, {"source_table", table},
                {"source_field", field}, {"mapping_id", mapId},
                {"mapping_name", target ? Json(cell(*target, "_MappingName")) : Json(nullptr)},
                {"status", status}, {"concrete_action_names", nullptr}, {"action_hash", nullptr}});
        };
        for (const char* field : {"_HeroMappingID", "_EmoteMappingID", "_StandbyMappingID", "_SoulMapping"}) link(row, kBattle, field);
        if (f) for (const char* field : {"_UIHeroMappingID", "_UIHeroEmoteFaceMappingID", "_UIHeroEnEmoteFaceMappingID",
                "_UIHeroKrEmoteFaceMappingID", "_UIHeroJpEmoteFaceMappingID", "_UIHeroThEmoteFaceMappingID"})
            if (f->count(field)) link(*f, kFashion, field);
    }
    // String-key table: the historical exporter truncated/interpreted reference keys.
    // Only the typed _ID and scalar columns are retained. _KEY and motion pointers
    // must not be presented as hashes, clips or verified relationships.
    for (const auto& row : states.rows) report["action_states"].push_back({{"state_name", cell(row, "_ID")},
        {"tag", cell(row, "_TagStr")}, {"layer", number(row, "_LayerIdx")},
        {"hero_id", nullptr}, {"action_hash", nullptr}, {"association_status", "UNRESOLVED"}});
    int unresolvedSkills = 0, unresolvedSkillMappings = 0;
    for (const auto& hero : report["heroes"]) {
        const int heroId = hero["hero_id"].get<int>();
        std::set<int> selected, all;
        if (!hero["talent"].is_null()) {
            const auto& data = hero["talent"];
            selected.insert(data["default_skill_id"].get<int>());
            selected.insert(data["default_ultimate_id"].get<int>());
            for (const char* kind : {"skill_ids", "ultimate_ids"})
                for (const auto& id : data[kind]) selected.insert(id.get<int>());
            selected.erase(0);
            if (!selected.empty() && *selected.begin() < 0) throw std::runtime_error("Negative talent ID");
        }
        all = selected;
        // Retain related alternative/deprecated skills as well as default choices.
        for (const auto& detail : details.rows)
            if (number(detail, "_RelateHero") == heroId) all.insert(number(detail, "_ID"));
        for (int skillId : all) {
            const Row* detail = details.get(std::to_string(skillId));
            bool ownerMatches = detail && number(*detail, "_RelateHero") == heroId;
            const char* status = !details.present ? "MISSING_TABLE" : !detail ? "MISSING_ID" : ownerMatches ? "FOUND" : "OWNER_MISMATCH";
            if (!ownerMatches) ++unresolvedSkills;
            int mapId = detail ? number(*detail, "_ComboMappingID") : 0;
            if (mapId < 0) throw std::runtime_error("Negative skill mapping ID");
            const Row* target = mapId ? mapping.get(std::to_string(mapId)) : nullptr;
            if (mapId && !target) ++unresolvedSkillMappings;
            report["hero_skill_links"].push_back({{"hero_id", heroId}, {"talent_id", skillId},
                {"listed_in_hero_talent", hero["talent"].is_null() ? Json(nullptr) : Json(selected.count(skillId) != 0)},
                {"related_hero_id", detail ? Json(number(*detail, "_RelateHero")) : Json(nullptr)},
                {"skill_name", detail ? Json(cell(*detail, "_TalentName")) : Json(nullptr)},
                {"combo_graph", detail ? Json(cell(*detail, "_ComboGraph")) : Json(nullptr)},
                {"combo_mapping_id", detail ? Json(mapId) : Json(nullptr)},
                {"combo_mapping_name", target ? Json(cell(*target, "_MappingName")) : Json(nullptr)},
                {"mapping_status", !detail ? "UNKNOWN" : !mapId ? "NO_OVERRIDE" : !mapping.present ? "MISSING_TABLE" : target ? "FOUND" : "MISSING_ID"},
                {"launch_config", detail ? Json(boolean(*detail, "_Launch")) : Json(nullptr)},
                {"deprecated", detail ? Json(boolean(*detail, "_Deprecated")) : Json(nullptr)},
                {"status", status}, {"animation_clip_names", nullptr}, {"action_hash", nullptr}});
        }
    }
    report["counts"] = {{"config_records", battle.rows.size()}, {"main_marked_records", fashion.present ? Json(markedMain) : Json(nullptr)},
        {"named_main_records", fashion.present ? Json(namedMain) : Json(nullptr)}, {"placeholder_records", placeholders},
        {"related_non_main_forms", relatedForms}, {"missing_fashion_rows", missingFashion}, {"different_name_rows", namesDiffer},
        {"mapping_links", report["hero_action_links"].size()}, {"unresolved_nonzero_mapping_links", missingMappings},
        {"state_names", states.present ? Json(states.rows.size()) : Json(nullptr)},
        {"hero_skill_links", report["hero_skill_links"].size()}, {"unresolved_skill_records", unresolvedSkills},
        {"unresolved_skill_mapping_links", unresolvedSkillMappings}};
    for (const Table* secondary : {&fashion, &talent}) for (const auto& row : secondary->rows)
        if (!battle.get(cell(row, "_ID"))) report["warnings"].push_back(secondary->name + " 存在主表以外的 ID：" + cell(row, "_ID"));
    if (missingFashion) report["warnings"].push_back("英雄外观表缺失行数：" + std::to_string(missingFashion));
    if (missingMappings) report["warnings"].push_back("未解析的非零动作映射引用：" + std::to_string(missingMappings));
    if (namesDiffer) report["warnings"].push_back("战斗表与外观表名称不同，已同时保留：" + std::to_string(namesDiffer));
    if (unresolvedSkills) report["warnings"].push_back("未确认所属英雄或详情缺失的技能：" + std::to_string(unresolvedSkills));
    if (unresolvedSkillMappings) report["warnings"].push_back("未解析的非零技能映射：" + std::to_string(unresolvedSkillMappings));
    return report;
}

void writeNew(const fs::path& path, const std::string& text) {
    HANDLE file = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) throw std::runtime_error("Cannot create output: " + path.u8string());
    DWORD written = 0;
    bool ok = text.size() <= MAXDWORD && WriteFile(file, text.data(), (DWORD)text.size(), &written, nullptr) && written == text.size();
    if (ok) ok = FlushFileBuffers(file) != FALSE;
    if (!CloseHandle(file)) ok = false;
    if (!ok) throw std::runtime_error("Output write failed: " + path.u8string());
}

fs::path exportFiles(const fs::path& input, const fs::path& root) {
    // Parse all inputs before creating a run directory: bad input cannot leave
    // apparently successful products from this invocation.
    Json report = analyze(input);
    SYSTEMTIME now = {};
    GetLocalTime(&now);
    wchar_t stamp[96] = {};
    swprintf_s(stamp, L"catalog_%04u%02u%02u_%02u%02u%02u_%03u_%lu", now.wYear, now.wMonth, now.wDay,
        now.wHour, now.wMinute, now.wSecond, now.wMilliseconds, GetCurrentProcessId());
    fs::create_directories(root);
    fs::path output;
    for (unsigned suffix = 0; suffix < 100; ++suffix) {
        output = root / (std::wstring(stamp) + L"_" + std::to_wstring(suffix));
        if (fs::create_directory(output)) break;
        if (suffix == 99) throw std::runtime_error("Cannot allocate a unique output directory");
    }
    wchar_t localTime[48] = {};
    swprintf_s(localTime, L"%04u-%02u-%02uT%02u:%02u:%02u.%03u", now.wYear, now.wMonth, now.wDay,
        now.wHour, now.wMinute, now.wSecond, now.wMilliseconds);
    report["exported_at_local"] = fs::path(localTime).u8string();
    Json products = Json::array();
    auto emit = [&](const char* filename, const std::string& text) {
        writeNew(output / filename, text);
        products.push_back({{"file", filename}, {"bytes", text.size()}, {"sha256", sha256(text)}});
    };
    try {
        emit("heroes.json", report.dump(2) + "\n");
        emit("heroes.tsv", tsv(report["heroes"], {"hero_id", "battle_title", "display_name", "internal_name", "category",
            "is_main_config", "placeholder", "related_hero_id", "related_from_hero_ids", "deprecated", "playable_now", "names_differ"}));
        emit("hero_action_links.tsv", tsv(report["hero_action_links"], {"hero_id", "source_table", "source_field", "mapping_id", "mapping_name", "status"}));
        emit("hero_skills.tsv", tsv(report["hero_skill_links"], {"hero_id", "talent_id", "skill_name", "combo_graph", "combo_mapping_id", "combo_mapping_name",
            "status", "mapping_status", "listed_in_hero_talent", "related_hero_id", "launch_config", "deprecated"}));
        emit("action_states.tsv", tsv(report["action_states"], {"state_name", "tag", "layer", "hero_id", "action_hash", "association_status"}));
        const auto& counts = report["counts"];
        std::string header = "// ===== 英雄配置目录（文件快照，非对局玩家）=====\r\n";
        header += "// 数据来源：" + tsvCell(input.u8string()) + "\r\n";
        header += "// 配置记录：" + counts["config_records"].dump() + "；主英雄标记：" + counts["main_marked_records"].dump() +
            "；非占位主英雄记录：" + counts["named_main_records"].dump() + "\r\n";
        header += "// 关联非主形态：" + counts["related_non_main_forms"].dump() + "；占位记录：" + counts["placeholder_records"].dump() + "\r\n";
        header += "// 当前可选英雄总数：未验证；全部动作覆盖：未验证；详细数据见同目录 heroes.json\r\n";
        for (const auto& hero : report["heroes"]) header += "// Hero ID=" + hero["hero_id"].dump() + " Name=" +
            tsvCell(hero["battle_title"].get<std::string>()) + " Category=" + hero["category"].get<std::string>() + "\r\n";
        header += "// ============================================\r\n";
        emit("dump_header.cs", "\xEF\xBB\xBF" + header);
        std::string log = "英雄目录导出完成。\r\n输入为已有文件快照，未连接游戏进程。\r\n";
        log += "配置记录数：" + counts["config_records"].dump() + "\r\n动作状态名：" + counts["state_names"].dump() + "\r\n";
        for (const auto& warning : report["warnings"]) log += "提示：" + warning.get<std::string>() + "\r\n";
        emit("offset_log.txt", "\xEF\xBB\xBF" + log);
        Json manifest = {{"schema_version", 1}, {"status", "COMPLETE_WITH_LIMITATIONS"}, {"source_kind", "FILE_SNAPSHOT"},
            {"live_process_verified", false}, {"exported_at_local", report["exported_at_local"]},
            {"counts", report["counts"]}, {"sources", report["sources"]}, {"products", products}};
        // A manifest is written only after all product writes succeed.
        writeNew(output / "manifest.json", manifest.dump(2) + "\n");
    } catch (...) {
        try { writeNew(output / "FAILED.txt", "本次导出未完整完成，不可作为成功产物使用。\r\n"); } catch (...) {}
        throw;
    }
    return output;
}
} // namespace

unsigned __cdecl HeroCatalogVersion() noexcept { return 1; }

int __cdecl ExportHeroCatalogW(const wchar_t* configDirectory, const wchar_t* outputRoot,
                               wchar_t* result, size_t resultCapacity) noexcept {
    if (!result || resultCapacity < 32768) return 3;
    result[0] = 0;
    if (!configDirectory || !*configDirectory || !outputRoot || !*outputRoot) return 1;
    try {
        fs::path input = fs::absolute(configDirectory).lexically_normal();
        fs::path root = fs::absolute(outputRoot).lexically_normal();
        if (!fs::is_directory(input)) throw std::runtime_error("Input directory does not exist");
        const auto output = exportFiles(input, root).wstring();
        if (output.size() >= resultCapacity) throw std::runtime_error("Output path is too long");
        wcscpy_s(result, resultCapacity, output.c_str());
        return 0;
    } catch (const std::exception& e) {
        try {
            auto message = L"导出失败：" + wide(e.what());
            wcsncpy_s(result, resultCapacity, message.c_str(), _TRUNCATE);
        } catch (...) { wcscpy_s(result, resultCapacity, L"导出失败，请检查输入与输出路径。"); }
        return 2;
    } catch (...) {
        wcscpy_s(result, resultCapacity, L"导出失败：未知异常。");
        return 2;
    }
}
