#include "RectangularChannelStrategy.h"
#include "../../grammar/Op.h"
#include <cmath>

bool RectangularChannelStrategy::applies(const FlowChannelIntent& intent) const {
    // MVP logic: always applicable for now
    return true;
}

GrammarProgram RectangularChannelStrategy::emit(const FlowChannelIntent& intent) const {
    GrammarProgram program;

    // Simple placeholder sizing
    double area = intent.flowRate;   // fake, just for now
    double width = std::sqrt(area);
    double height = width;
    double length = intent.length;

    Op box;
    box.id = "channel";
    box.type = OpType::Box;

    // Convention:
    // params[0] = width
    // params[1] = depth
    // params[2] = height
    box.params = { width, width, length };

    program.ops.push_back(box);
    return program;
}
