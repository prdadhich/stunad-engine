#pragma once

#include "FlowChannelStrategy.h"

class CircularChannelStrategy : public FlowChannelStrategy {
public:
    bool applies(const FlowChannelIntent& intent) const override;
    GrammarProgram emit(const FlowChannelIntent& intent) const override;
};
