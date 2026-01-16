#pragma once
#include "../core/Kernel.h"

class OcctKernel : public Kernel
{
public:
    Solid* MakeBox(double x, double y, double z) override;

    Solid* Subtract(Solid* a, Solid* b) override;
    Solid* MakeCylinder(double r, double h) override;
    Solid* Fillet(Solid* s, double r) override;
    Solid* Shell(Solid* s, double t) override;
    void ExportSTEP(Solid* s, const char* path) override;
    Solid* BooleanCut(Solid* a, Solid* b) override;
    StunadMesh* Tessellate(Solid* solid, float linearDeflection) override;

};
