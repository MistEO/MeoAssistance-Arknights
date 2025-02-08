#pragma once
#include "AbstractRoguelikeTaskPlugin.h"
#include "Config/Roguelike/RoguelikeStageEncounterConfig.h"
#include "Config/TaskData.h"
#include "Sami/RoguelikeCollapsalParadigmTaskPlugin.h"
#include "Vision/OCRer.h"

namespace asst
{
class RoguelikeStageEncounterTaskPlugin : public AbstractRoguelikeTaskPlugin
{
public:
    using AbstractRoguelikeTaskPlugin::AbstractRoguelikeTaskPlugin;
    using Config = RoguelikeStageEncounterConfig;
    virtual ~RoguelikeStageEncounterTaskPlugin() override = default;

public:
    virtual bool verify(AsstMsg msg, const json::value& details) const override;

    void set_event(
        const std::string& theme,
        RoguelikeMode mode,
        const std::string& event_name,
        int choose,
        int option_num);

protected:
    virtual bool _run() override;
    static bool satisfies_condition(const Config::ChoiceRequire& requirement, int special_val);
    static int process_task(const Config::RoguelikeEvent& event, const int special_val);
    static int hp(const cv::Mat& image);

private:
    // 程序运行过程中临时修改的数据
    Config::RoguelikeEventDB m_modified_events;
};
}
