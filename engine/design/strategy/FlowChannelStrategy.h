#pragma once
#include "../intent/FlowChannelIntent.h"
#include "../../grammar/GrammarProgram.h"

class FlowChannelStrategy {
public:
    virtual ~FlowChannelStrategy() = default;

    virtual bool applies(const FlowChannelIntent& intent) const = 0;
    virtual GrammarProgram emit(const FlowChannelIntent& intent) const = 0;
};
