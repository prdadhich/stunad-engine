#pragma once
#include <vector>
#include <memory>
#include "Solid.h"
#include "Parameter.h"

class Kernel;

class Node
{
public:
    std::vector<Parameter> params;

    virtual std::unique_ptr<Solid> Evaluate(Kernel& kernel) = 0;
    virtual ~Node() {}
};
