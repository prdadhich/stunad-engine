#pragma once

#include "FlowChannelStrategy.h"

class RectangularChannelStrategy : public FlowChannelStrategy {
public:
    bool applies(const FlowChannelIntent& intent) const override;
    GrammarProgram emit(const FlowChannelIntent& intent) const override;
};
