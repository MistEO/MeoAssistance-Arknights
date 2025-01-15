#include "RoguelikeDeepExplorationSpecialParameters.h"

#include "Config/TaskData.h"
#include "Controller/Controller.h"
#include "Task/ProcessTask.h"
#include "Utils/Logger.hpp"

bool load_params([[maybe_unused]] const json::value& params)
{
    // 不显式声明mac/arm64编译会出错
    return true;
}

bool asst::RoguelikeDeepExplorationSpecialParameters::verify(AsstMsg msg, const json::value& details) const
{
    if (msg != AsstMsg::SubTaskStart || details.get("subtask", std::string()) != "ProcessTask") {
        return false;
    }

    if (!RoguelikeConfig::is_valid_theme(m_config->get_theme())) {
        Log.error("Roguelike name doesn't exist!");
        return false;
    }

    const std::string roguelike_name = m_config->get_theme() + "@";
    const auto& mode = m_config->get_mode();
    const std::string& task = details.get("details", "task", "");
    std::string_view task_view = task;
    if (task_view.starts_with(roguelike_name)) {
        task_view.remove_prefix(roguelike_name.length());
    }
    if (task_view == "Roguelike@StartExplore" && mode == RoguelikeMode::Exploration) {
        return true;
    }
    else {
        return false;
    }
}

bool asst::RoguelikeDeepExplorationSpecialParameters::_run()
{
    LogTraceFunction;

    /// todo: 深入调查目标识别+策略适配
    /// 傀影：
    /// 十万火急        6战斗(pragmatic)
    /// 逢生            --
    /// 经营           消耗100源石锭
    /// 火力覆盖        抓10个狙/术
    /// 地面推进        抓10个重/近
    /// 路难寻          --
    /// 奇遇记          --
    /// 剑走偏锋        生吃30，跳过初始招募，触发队伍为空警告
    /// 迷茫新世界       --
    /// 白手起家        --
    /// 夺宝奇兵        3精英(aggressive)
    /// 娱乐至上        6幕间余兴
    ///
    /// 水月：
    /// 感知流逝        --
    /// 畸变加剧        --
    /// 肾上腺素飙升    -- o:不买东西
    /// 躁狂           3钥匙
    /// 泰拉的晨曦      7骰子
    /// 议会使者        -- o:抓近战位
    /// 女皇之声        -- o:抓术
    /// 加冕           通关  国王套可怎么适配啊.png
    /// 全副武装        8战斗 o:抓重
    /// 妙手回春        4紧急 o:抓...奶？没准不改了
    /// 出其不意        抓3特
    /// 旁敲侧击        抓3辅
    ///
    /// 萨米：         初始选择追加调查装备，考虑默认选择LUD-99X选后不管/小车，据说适配了
    /// 观察者的视野    --
    /// 吹号者的视野    --
    /// 营商者的视野    100源石锭
    /// 勘察者的视野    7抗干扰 o:拐树篱
    /// 天真者的视野    3失与得
    /// 护卫者的视野    --
    /// 环游者的视野    6战斗
    /// 受困者的视野    --
    /// 观光者的视野    4紧急
    /// 复仇者的视野    --
    /// 愚痴者的视野    --
    /// 异化者的视野    --
    /// 先知者的视野    --
    ///
    /// 萨卡兹：
    /// 还没开呢.jpg

    /// task: 偏向特定节点 偏向特定抓取 成为生吃高手
    ///

    return true;
}
