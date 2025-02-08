#pragma once

#include "Config/AbstractConfig.h"

#include <vector>

#include "Common/AsstBattleDef.h"
#include "Common/AsstTypes.h"
#include "Task/Roguelike/RoguelikeConfig.h"

namespace asst
{
class RoguelikeStageEncounterConfig final : public SingletonHolder<RoguelikeStageEncounterConfig>, public AbstractConfig
{
public:
    virtual ~RoguelikeStageEncounterConfig() override = default;

    enum class ComparisonType
    {
        GreaterThan,
        LessThan,
        Equal,
        None,        // 没有配置
        Unsupported, // 配置错误或其他
    };

    struct Vision
    {
        std::string value;
        ComparisonType type = ComparisonType::None;
    };

    struct ChoiceRequire
    {
        std::string name;
        int choose = -1;
        Vision vision; // 现在只有Vision解析，之后要改成requirements且支持多个条件判断同一个选择
    };

    struct RoguelikeEvent
    {
        std::string name;
        int option_num = 0;
        int default_choose = 0;
        std::vector<ChoiceRequire> choice_require;
    };

    using RoguelikeEventMap = std::unordered_map<std::string, RoguelikeEvent>;

    using RoguelikeEventDB =
        std::unordered_map<std::pair<std::string, int>, RoguelikeEventMap, std::pair_hash<std::string, int>>;

    [[nodiscard]] std::optional<std::reference_wrapper<const RoguelikeEvent>> get_event(
        const std::string& theme,
        RoguelikeMode mode,
        const std::string& event_name,
        const RoguelikeEventDB& m_modified_events = {}) const;

    [[nodiscard]] const std::vector<std::string>& get_event_names(const std::string& theme) const
    {
        return m_event_names.at(theme);
    }

private:
    virtual bool parse(const json::value& json) override;

    static constexpr ComparisonType parse_comparison_type(const std::string& type_str);

    // 从配置文件中读取的数据
    RoguelikeEventDB m_events;
    // 任务名列表，用于传给 OCRerConfig::set_required 作为 OCR 目标
    std::unordered_map<std::string, std::vector<std::string>> m_event_names;

    // 未指定适用模式（即 mode 字段为空）的配置将作为默认配置，以 "(theme, DEFAULT_MODE_PLACEHOLDER)" 为 key
    // 对于具体模式 mode，其专用配置，以 (theme, mode) 为 key，将继承默认配置
    static constexpr int DEFAULT_MODE_PLACEHOLDER = -1;
};

inline static auto& RoguelikeStageEncounter = RoguelikeStageEncounterConfig::get_instance();
}
