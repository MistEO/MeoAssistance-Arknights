#include "InfrastReceptionTask.h"

#include "Utils/Ranges.hpp"

#include "Config/TaskData.h"
#include "Controller/Controller.h"
#include "Task/ProcessTask.h"
#include "Utils/Logger.hpp"
#include "Vision/Infrast/InfrastClueVacancyImageAnalyzer.h"
#include "Vision/Matcher.h"
#include "Vision/MultiMatcher.h"

bool asst::InfrastReceptionTask::_run()
{
    m_all_available_opers.clear();

    swipe_to_the_right_of_main_ui();
    if (!enter_facility()) {
        return false;
    }
    click_bottom_left_tab();

    close_end_of_clue_exchange();

    if (m_prioritize_sending_clue) {
        send_clue();
        get_clue();
        if (need_exit()) {
            return false;
        }
        send_clue();
        use_clue();
    }
    else {
        use_clue();
        get_clue();
        if (need_exit()) {
            return false;
        }
        use_clue();
        send_clue();
    }

    if (need_exit()) {
        return false;
    }
    shift();

    return true;
}

bool asst::InfrastReceptionTask::close_end_of_clue_exchange()
{
    ProcessTask task_temp(*this, { "EndOfClueExchangeBegin" });
    return task_temp.run();
}

// get_clue的taskchain里面隐含一个满线索送走三个的task
// SendClueFlag->SendClues->SelectClue->ClueSelected->ClueGiveTo1/2/3/4/NextPage->*Confirm->SelectClue--->CloseSendClue,
// SelectClue.maxTimes=3
//
// todo: sendclue丢给send_clue接管
// todo: use_clue/send_clue重构后确保至少一个空位
// todo: need_exit到底是干啥的
bool asst::InfrastReceptionTask::get_clue()
{
    ProcessTask task_temp(
        *this,
        { "InfrastClueSelfNew", "InfrastClueFriendNew", "InfrastClueSelfMaybeFull", "ReceptionFlag" });
    return task_temp.set_retry_times(ProcessTask::RetryTimesDefault).run();
}

bool asst::InfrastReceptionTask::use_clue()
{
    LogTraceFunction;
    const static std::string clue_vacancy = "InfrastClueVacancy";
    const static std::vector<std::string> clue_suffix = { "No1", "No2", "No3", "No4", "No5", "No6", "No7" };

    proc_clue_vacancy();
    if (unlock_clue_exchange()) {
        proc_clue_vacancy();
    }

    cv::Mat image = ctrler()->get_image();

    // 所有的空位分析一次，看看还缺哪些线索
    InfrastClueVacancyImageAnalyzer vacancy_analyzer(image);

    vacancy_analyzer.set_to_be_analyzed(clue_suffix);
    vacancy_analyzer.analyze();

    const auto& vacancy = vacancy_analyzer.get_vacancy();
    for (const auto& id : vacancy | views::keys) {
        Log.trace("InfrastReceptionTask | Vacancy", id);
    }

    std::string product;
    if (vacancy.size() == 1) {
        product = vacancy.begin()->first;
    }
    else {
        product = "General";
    }
    Log.trace("InfrastReceptionTask | product", product);
    set_product(product);

    back_to_reception_main();

    return true;
}

bool asst::InfrastReceptionTask::proc_clue_vacancy()
{
    LogTraceFunction;
    const static std::string clue_vacancy = "InfrastClueVacancy";
    const static std::vector<std::string> clue_suffix = { "No1", "No2", "No3", "No4", "No5", "No6", "No7" };

    cv::Mat image = ctrler()->get_image();
    for (const std::string& clue : clue_suffix) {
        if (need_exit()) {
            return false;
        }
        // 先识别线索的空位
        InfrastClueVacancyImageAnalyzer vacancy_analyzer(image);

        vacancy_analyzer.set_to_be_analyzed({ clue });
        if (!vacancy_analyzer.analyze()) {
            continue;
        }
        // 点开线索的空位
        Rect vacancy = vacancy_analyzer.get_vacancy().cbegin()->second;
        ctrler()->click(vacancy);
        int delay = Task.get(clue_vacancy + clue)->post_delay;
        sleep(delay);

        // 识别右边列表中的线索，然后用最底下的那个（一般都是剩余时间最短的）
        // swipe_to_the_bottom_of_clue_list_on_the_right();
        image = ctrler()->get_image();
        MultiMatcher clue_analyzer(image);
        clue_analyzer.set_task_info("InfrastClue");

        auto clue_result_opt = clue_analyzer.analyze();
        if (!clue_result_opt) {
            continue;
        }
        sort_by_horizontal_(*clue_result_opt);
        ctrler()->click(clue_result_opt->back().rect);
        delay = Task.get("InfrastClue")->post_delay;
        sleep(delay);
    }
    return true;
}

bool asst::InfrastReceptionTask::unlock_clue_exchange()
{
    ProcessTask task(*this, { "UnlockClues" });
    task.set_retry_times(2);
    return task.run();
}

bool asst::InfrastReceptionTask::back_to_reception_main()
{
    ProcessTask(*this, { "EndOfClueExchange" }).set_retry_times(0).run();
    return ProcessTask(*this, { "BackToReceptionMain" }).run();
}

// todo: 接管sendclue，起点在CloseCluePage->SendClueFlag
// todo: m_prioritize_sending_clue送之前把线索都弹出来
// todo: m_amount_of_clue_to_send定量
// todo: m_send_clue_to_ocr定向, m_only_send_clue_to_ocr跳过fallback
// todo: parse m_send_clue_list
// todo: 没匹配上callback通知用户
bool asst::InfrastReceptionTask::send_clue()
{
    LogTraceFunction;
    ProcessTask task(*this, { "SendClues" });
    return task.run();
}

bool asst::InfrastReceptionTask::shift()
{
    LogTraceFunction;

    if (m_is_custom && current_room_config().skip) {
        Log.info("skip this room");
        return true;
    }

    const auto image = ctrler()->get_image();
    Matcher add_analyzer(image);

    const auto raw_task_ptr = Task.get("InfrastAddOperator" + facility_name() + m_work_mode_name);
    switch (raw_task_ptr->algorithm) {
    case AlgorithmType::JustReturn:
        if (raw_task_ptr->action == ProcessTaskAction::ClickRect) {
            ctrler()->click(raw_task_ptr->specific_rect);
        }
        break;
    case AlgorithmType::MatchTemplate: {
        add_analyzer.set_task_info(raw_task_ptr);

        if (!add_analyzer.analyze()) {
            return true;
        }
        ctrler()->click(add_analyzer.get_result().rect);
    } break;
    default:
        break;
    }
    sleep(raw_task_ptr->post_delay);

    for (int i = 0; i <= OperSelectRetryTimes; ++i) {
        if (need_exit()) {
            return false;
        }

        if (is_use_custom_opers()) {
            bool name_select_ret = swipe_and_select_custom_opers();
            if (name_select_ret) {
                break;
            }
            else {
                swipe_to_the_left_of_operlist();
                continue;
            }
        }

        click_clear_button();

        if (!opers_detect_with_swipe()) {
            return false;
        }
        swipe_to_the_left_of_operlist();

        optimal_calc();
        bool ret = opers_choose();
        if (!ret) {
            m_all_available_opers.clear();
            swipe_to_the_left_of_operlist();
            continue;
        }
        break;
    }
    click_confirm_button();
    return true;
}

asst::InfrastReceptionTask& asst::InfrastReceptionTask::set_clue_sending_config(
    bool prioritize_sending_clue,
    int amount_of_clue_to_send,
    bool send_clue_to_ocr,
    bool only_send_clue_to_ocr,
    std::string send_clue_list) noexcept
{
    m_prioritize_sending_clue = prioritize_sending_clue;
    m_amount_of_clue_to_send = amount_of_clue_to_send;
    m_send_clue_to_ocr = send_clue_to_ocr;
    m_only_send_clue_to_ocr = only_send_clue_to_ocr;
    m_send_clue_list = send_clue_list;
    return *this;
}
