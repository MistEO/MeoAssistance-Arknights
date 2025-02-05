#pragma once
#include "InfrastProductionTask.h"

namespace asst
{
class InfrastReceptionTask final : public InfrastProductionTask
{
public:
    using InfrastProductionTask::InfrastProductionTask;
    virtual ~InfrastReceptionTask() override = default;

    virtual size_t max_num_of_opers() const noexcept override { return 2ULL; }

    asst::InfrastReceptionTask& set_clue_sending_config(
        bool prioritize_sending_clue,
        int amount_of_clue_to_send,
        bool send_clue_to_ocr,
        bool only_send_clue_to_ocr,
        std::string send_clue_list) noexcept;

protected:
    virtual bool _run() override;

private:
    virtual int operlist_swipe_times() const noexcept override { return 4; }

    bool close_end_of_clue_exchange();
    bool get_clue();
    bool use_clue();
    bool proc_clue_vacancy();
    bool unlock_clue_exchange();
    bool back_to_reception_main();
    bool send_clue();
    bool shift();

    bool m_prioritize_sending_clue = false; // 是否优先送线索
    int m_amount_of_clue_to_send = 3;       // 送线索的数量
    bool m_send_clue_to_ocr = false;        // 优先送给指定ID
    bool m_only_send_clue_to_ocr = false;   // 只送给指定ID
    std::string m_send_clue_list = "";      // 送线索的ID列表
};
}
