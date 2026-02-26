#include "CircularChannelStrategy.h"
#include "../../grammar/Op.h"
#include <cmath>

bool CircularChannelStrategy::applies(const FlowChannelIntent& intent) const {
    // MVP logic: always applicable for now
    return true;
}

GrammarProgram CircularChannelStrategy::emit(const FlowChannelIntent& intent) const {
    GrammarProgram program;

    // Placeholder analytical logic
    double radius = std::sqrt(intent.flowRate);
    double height = intent.length;

    Op cylinder;
    cylinder.id = "channel";
    cylinder.type = OpType::Cylinder;

    // Convention (document this!):
    // params[0] = radius
    // params[1] = height
    cylinder.params = { radius, height };

    program.ops.push_back(cylinder);
    return program;
}
