#pragma once
#include "Resolver.h"
#include "../intent/FlowChannelIntent.h"

class FlowChannelResolver : public Resolver<FlowChannelIntent> {
public:
    GrammarProgram resolve(const FlowChannelIntent& intent) override;
};
