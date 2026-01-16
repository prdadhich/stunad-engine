#pragma once
#include "../core/Node.h"

class LampNode : public Node
{
public:
    LampNode()
    {
        params.push_back({"radius", 50, 10, 200});
        params.push_back({"height", 200, 50, 500});
        params.push_back({"thickness", 3, 1, 10});
    }

    Solid* Evaluate(Kernel& kernel) override;
};
