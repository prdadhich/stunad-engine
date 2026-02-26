#include "FlowChannelResolver.h"
#include "../strategy/CircularChannelStrategy.h"
#include "../strategy/RectangularChannelStrategy.h"


GrammarProgram FlowChannelResolver::resolve(const FlowChannelIntent& intent) {
    CircularChannelStrategy circular;
    RectangularChannelStrategy rect;

    if (intent.priority == FlowPriority::CompactPacking) {
        return rect.emit(intent);
    }

    return circular.emit(intent);
}
