#pragma once
#include "AbstractRoguelikeTaskPlugin.h"

namespace asst
{

class RoguelikeDeepExplorationSpecialParameters : public AbstractRoguelikeTaskPlugin
{
public:
    using AbstractRoguelikeTaskPlugin::AbstractRoguelikeTaskPlugin;
    virtual ~RoguelikeDeepExplorationSpecialParameters() override = default;
    virtual bool verify(AsstMsg msg, const json::value& details) const override;

protected:
    virtual bool _run() override;

private:
};
}

