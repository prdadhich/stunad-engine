#pragma once
#include "../core/Param.h"
#include "../kernel/Kernel.h"

class Pedestal
{
public:
    Param<double> radius{50};
    Param<double> height{200};

    Pedestal(Kernel* k) : kernel(k) {}

    Solid* Build()
    {
        auto outer = kernel->MakeCylinder(radius.Get(), height.Get());
        auto shelled = kernel->Shell(outer, 4);
        return kernel->Fillet(shelled, 6);
    }


private:
    Kernel* kernel;
};
