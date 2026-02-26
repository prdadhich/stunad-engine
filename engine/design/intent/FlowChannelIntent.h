#pragma once

enum class FlowPriority {
    MinPressureLoss,
    CompactPacking,
    MaxSurfaceArea
};

struct FlowChannelIntent {
    double flowRate;          // m^3/s or kg/s (you decide, but be consistent)
    double maxPressureDrop;   // Pa or bar
    double length;            // channel length
    FlowPriority priority;
};
