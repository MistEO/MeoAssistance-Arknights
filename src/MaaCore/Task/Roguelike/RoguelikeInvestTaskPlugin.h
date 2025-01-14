#pragma once
#include "AbstractRoguelikeTaskPlugin.h"

namespace asst
{
class RoguelikeControlTaskPlugin;

// 肉鸽投资插件
class RoguelikeInvestTaskPlugin : public AbstractRoguelikeTaskPlugin
{
public:
    using AbstractRoguelikeTaskPlugin::AbstractRoguelikeTaskPlugin;
    virtual ~RoguelikeInvestTaskPlugin() override = default;
    virtual bool verify(AsstMsg msg, const json::value& details) const override;
    virtual bool load_params([[maybe_unused]] const json::value& params) override;

private:
    virtual bool _run() override;
    std::optional<int> ocr_count(const auto& img, const auto& task_name) const;
    bool is_investment_available(const cv::Mat& image) const;
    bool is_investment_insufficient(const cv::Mat& image) const;
    bool is_investment_error(const cv::Mat& image) const;
    void stop_roguelike() const;

    int m_invest_count = 0;
    int m_maximum = 0;
    bool m_stop_when_full = false; // 存款满了就停止
};
} // namespace asst
