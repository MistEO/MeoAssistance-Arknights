#pragma once
#include "AbstractRoguelikeTaskPlugin.h"

namespace asst
{

class RoguelikeIterateDeepExplorationPlugin : public AbstractRoguelikeTaskPlugin
{
public:
    using AbstractRoguelikeTaskPlugin::AbstractRoguelikeTaskPlugin;
    virtual ~RoguelikeIterateDeepExplorationPlugin() override = default;
    virtual bool verify(AsstMsg msg, const json::value& details) const override;
    virtual bool load_params(const json::value& params) override;

protected:
    virtual bool _run() override;

private:
    int m_current_difficulty = -1;
    int deep_exploration_count;
};
}
