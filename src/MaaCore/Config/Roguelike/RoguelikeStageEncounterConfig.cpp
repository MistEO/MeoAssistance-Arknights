#include "RoguelikeStageEncounterConfig.h"

#include <meojson/json.hpp>

#include "Utils/Logger.hpp"

bool asst::RoguelikeStageEncounterConfig::parse(const json::value& json)
{
    LogTraceFunction;

    const std::string theme = json.at("theme").as_string();

    // 未指定适用模式（即 mode 字段为空）的配置将作为默认配置，以 (theme, DEFAULT_MODE_PLACEHOLDER) 为 key
    // 对于具体模式 mode，其专用配置，以 (theme, mode) 为 key，将继承默认配置
    const std::pair<std::string, int> default_config_key = std::make_pair(theme, DEFAULT_MODE_PLACEHOLDER);
    std::vector<int> modes = json.get("mode", std::vector<int> {});
    std::unordered_map<std::string, RoguelikeEvent> events;

    std::vector<std::string>& event_names = m_event_names[theme];

    for (const auto& event_json : json.at("stage").as_array()) {
        RoguelikeEvent event;
        event.name = event_json.at("name").as_string();
        if (ranges::find(event_names, event.name) == event_names.end()) {
            event_names.emplace_back(event.name);
        }
        event.option_num = event_json.get("option_num", 0);
        event.default_choose = event_json.get("choose", 0);
        if (auto choice_require_opt = event_json.find("choices");
            choice_require_opt && choice_require_opt->is_array()) {
            for (const auto& choice_json : choice_require_opt->as_array()) {
                ChoiceRequire choice;
                choice.name = choice_json.at("name").as_string();
                choice.choose = choice_json.get("choose", -1);
                if (auto requirements_opt = choice_json.find("requirements");
                    requirements_opt && requirements_opt->is_array()) {
                    for (const auto& requirement_json : requirements_opt->as_array()) {
                        auto name = requirement_json.get("name", "");
                        if (name == "Vision") {
                            choice.vision.value = requirement_json.get("value", "");
                            choice.vision.type = parse_comparison_type(requirement_json.get("type", ""));
                        }
                        else if (name == "Relic") {
                            // not supported
                        }
                    }
                }
                event.choice_require.emplace_back(std::move(choice));
            }
        }
        events[event.name] = std::move(event);
    }

    // 若 modes 为空，则认为当前正在加载默认配置；若 modes 不为空，则认为当前正在加载模式专用配置
    // 注意，理论上默认配置与具体模式的配置都可以被多次加载、覆盖，但我目前没有想到这样做的意义
    if (modes.empty()) {
        // 防止重复加载
        if (m_events.contains(default_config_key)) {
            Log.error(__FUNCTION__, "| Default configuration for theme", theme, "has already exist!");
            return false;
        }
        m_events[default_config_key] = events;
    }
    else {
        for (int mode : modes) {
            const std::pair<std::string, int> mode_config_key = std::make_pair(theme, mode);
            // 防止重复加载
            if (m_events.contains(mode_config_key)) {
                Log.error(__FUNCTION__, "| Configuration for theme", theme, "and mode", mode, "has already exist!");
                return false;
            }
            m_events[mode_config_key] = events;
        }
    }

    return true;
}

std::optional<std::reference_wrapper<const asst::RoguelikeStageEncounterConfig::RoguelikeEvent>>
    asst::RoguelikeStageEncounterConfig::get_event(
        const std::string& theme,
        const RoguelikeMode mode,
        const std::string& event_name,
        const RoguelikeEventDB& m_modified_events) const
{
    const std::pair<std::string, int> mode_config_key = std::make_pair(theme, static_cast<int>(mode));
    const std::pair<std::string, int> default_config_key = std::make_pair(theme, DEFAULT_MODE_PLACEHOLDER);

    if (const auto db_it = m_modified_events.find(mode_config_key); db_it != m_modified_events.end()) {
        if (const auto map_it = db_it->second.find(event_name); map_it != db_it->second.end()) {
            Log.info("__FUNCTION__", "| Retrieved modified mode-specific data.");
            return map_it->second;
        }
    }
    if (const auto db_it = m_modified_events.find(default_config_key); db_it != m_modified_events.end()) {
        if (const auto map_it = db_it->second.find(event_name); map_it != db_it->second.end()) {
            Log.info("__FUNCTION__", "| Retrieved modified default data.");
            return map_it->second;
        }
    }
    if (const auto db_it = m_events.find(mode_config_key); db_it != m_events.end()) {
        if (const auto map_it = db_it->second.find(event_name); map_it != db_it->second.end()) {
            Log.info("__FUNCTION__", "| Retrieved original mode-specific data.");
            return map_it->second;
        }
    }
    if (const auto db_it = m_events.find(default_config_key); db_it != m_events.end()) {
        if (const auto map_it = db_it->second.find(event_name); map_it != db_it->second.end()) {
            Log.info("__FUNCTION__", "| Retrieved original default data.");
            return map_it->second;
        }
    }
    Log.error("__FUNCTION__", "| Failed to retrieve data.");
    return std::nullopt;
}

constexpr asst::RoguelikeStageEncounterConfig::ComparisonType
    asst::RoguelikeStageEncounterConfig::parse_comparison_type(const std::string& type_str)
{
    if (type_str == ">") {
        return ComparisonType::GreaterThan;
    }
    if (type_str == "<") {
        return ComparisonType::LessThan;
    }
    if (type_str == "=") {
        return ComparisonType::Equal;
    }
    return ComparisonType::Unsupported;
}
