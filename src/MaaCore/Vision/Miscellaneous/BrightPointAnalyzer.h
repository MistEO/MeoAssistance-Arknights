#pragma once

#include "Vision/VisionHelper.h"

namespace asst
{
class BrightPointAnalyzer : public VisionHelper
{
public:
    using Result = Point;
    using ResultsVec = std::vector<Result>;
    using ResultsVecOpt = std::optional<ResultsVec>;

    using VisionHelper::VisionHelper;
    virtual ~BrightPointAnalyzer() override = default;

    ResultsVecOpt analyze();

    // FIXME: 老接口太难重构了，先弄个这玩意兼容下，后续慢慢全删掉
    const auto& get_result() const noexcept { return m_result; }

    void set_rgb_mode(const bool rgb_mode) { m_rgb_mode = rgb_mode; }

    void set_lb(const double rgb_lb) { m_lb = rgb_lb; }

    void set_ub(const double rgb_ub) { m_ub = rgb_ub; }

    void set_rgb_lb(const std::vector<int>& rgb_lb)
    {
        if (rgb_lb.size() == 3) {
            m_rgb_lb = cv::Scalar(rgb_lb[0], rgb_lb[1], rgb_lb[2]);
        }
    }

    void set_rgb_ub(const std::vector<int>& rgb_ub)
    {
        if (rgb_ub.size() == 3) {
            m_rgb_ub = cv::Scalar(rgb_ub[0], rgb_ub[1], rgb_ub[2]);
        }
    }

private:
    bool m_rgb_mode = false;
    double m_lb = 200;
    double m_ub = 255;
    cv::Scalar m_rgb_lb = { 0, 0, 0 };
    cv::Scalar m_rgb_ub = { 0, 0, 0 };
    ResultsVec m_result;
};
}
