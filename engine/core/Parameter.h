#pragma once
#include <string>

struct Parameter
{
    std::string name;
    double value;
    double min;
    double max;

    Parameter(std::string n, double v, double mi, double ma)
        : name(n), value(v), min(mi), max(ma) {}
};
