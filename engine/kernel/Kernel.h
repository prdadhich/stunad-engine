#pragma once
#include "Solid.h"

class Kernel
{
public:
    virtual ~Kernel() {}

    virtual Solid* MakeBox(double x, double y, double z) = 0;
    virtual Solid* Subtract(Solid* a, Solid* b) = 0;
    virtual Solid* MakeCylinder(double r, double h) = 0;
    virtual Solid* Fillet(Solid* s, double radius) = 0;
    virtual Solid* Shell(Solid* s, double thickness) = 0;
};
