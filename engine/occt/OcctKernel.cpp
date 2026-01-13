#include "OcctKernel.h"
#include "OcctSolid.h"

#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepFilletAPI_MakeFillet.hxx>
#include <BRepOffsetAPI_MakeThickSolid.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopTools_ListOfShape.hxx>
#include <BRepOffsetAPI_MakeThickSolid.hxx>

Solid* OcctKernel::MakeBox(double x, double y, double z)
{
    TopoDS_Shape box = BRepPrimAPI_MakeBox(x, y, z).Shape();
    return new OcctSolid(box);
}

Solid* OcctKernel::MakeCylinder(double r, double h)
{
    return new OcctSolid(BRepPrimAPI_MakeCylinder(r, h).Shape());
}

Solid* OcctKernel::Subtract(Solid* a, Solid* b)
{
    auto sa = (OcctSolid*)a;
    auto sb = (OcctSolid*)b;

    return new OcctSolid(BRepAlgoAPI_Cut(sa->shape, sb->shape).Shape());
}

Solid* OcctKernel::Fillet(Solid* s, double r)
{
    auto so = (OcctSolid*)s;
    BRepFilletAPI_MakeFillet fillet(so->shape);

    for (TopExp_Explorer ex(so->shape, TopAbs_EDGE); ex.More(); ex.Next())
    {
        fillet.Add(r, TopoDS::Edge(ex.Current()));
    }

    return new OcctSolid(fillet.Shape());
}

Solid* OcctKernel::Shell(Solid* s, double t)
{
    auto so = (OcctSolid*)s;

    BRepOffsetAPI_MakeThickSolid shell;
    TopTools_ListOfShape faces;  // empty means "shell all faces"

    shell.MakeThickSolidByJoin(so->shape, faces, -t, 1e-3);

    return new OcctSolid(shell.Shape());
}