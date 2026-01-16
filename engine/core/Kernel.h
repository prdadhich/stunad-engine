#pragma once
#include "Solid.h"
#include "Mesh.h"
class Kernel
{
public:
    virtual ~Kernel() {}

    virtual Solid* MakeBox(double x, double y, double z) = 0;
    virtual Solid* Subtract(Solid* a, Solid* b) = 0;
    virtual Solid* MakeCylinder(double r, double h) = 0;
    virtual Solid* BooleanCut(Solid* a, Solid* b)=0;
    virtual Solid* Fillet(Solid* s, double radius) = 0;
    virtual Solid* Shell(Solid* s, double thickness) = 0;
    virtual void ExportSTEP(Solid* s, const char* path) = 0;

    
    virtual StunadMesh* Tessellate(Solid* solid, float linearDeflection) = 0;
   
};
