#pragma once
#include "../kernel/Kernel.h"

class OcctKernel : public Kernel
{
public:
    Solid* MakeBox(double x, double y, double z) override;

    Solid* Subtract(Solid* a, Solid* b) override;
    Solid* MakeCylinder(double r, double h) override;
    Solid* Fillet(Solid* s, double r);
    Solid* Shell(Solid* s, double t);
};
