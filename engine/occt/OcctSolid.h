#pragma once
#include "../kernel/Solid.h"
#include <TopoDS_Shape.hxx>

class OcctSolid : public Solid
{
public:
    TopoDS_Shape shape;
    OcctSolid(const TopoDS_Shape& s) : shape(s) {}
};
