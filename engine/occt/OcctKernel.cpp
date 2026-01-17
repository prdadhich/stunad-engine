#include "OcctKernel.h"
#include "OcctSolid.h"
#include "OcctTessellator.h"
#include "OcctProfile.h"

#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepFilletAPI_MakeFillet.hxx>
#include <BRepOffsetAPI_MakeThickSolid.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopTools_ListOfShape.hxx>

#include <STEPControl_Writer.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Wire.hxx>

#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepOffsetAPI_ThruSections.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepLib.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepBuilderAPI_FindPlane.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Plane.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <GeomAPI_Interpolate.hxx>
#include <TColgp_HArray1OfPnt.hxx>
#include <cmath>
#include <gp_Ax1.hxx>
#include <gp_Trsf.hxx>
#include <Standard_Handle.hxx>
#include <iostream>

Solid* OcctKernel::MakeBox(double x, double y, double z)
{
    TopoDS_Shape box = BRepPrimAPI_MakeBox(x, y, z).Shape();
    return new OcctSolid(box);
}

Solid* OcctKernel::MakeCylinder(double r, double h)
{
    return new OcctSolid(BRepPrimAPI_MakeCylinder(r, h).Shape());
}


Solid* OcctKernel::MakeSphere(double radius) {
    if (radius <= 1e-6) return nullptr;

    // Creates a sphere at the origin (0,0,0)
    BRepPrimAPI_MakeSphere maker(radius);
    maker.Build();

    if (!maker.IsDone()) return nullptr;
    return new OcctSolid(maker.Shape());
}

Solid* OcctKernel::MakeCone(double radiusBottom, double radiusTop, double height) {
    if (height <= 1e-6) return nullptr;

    // OCCT's MakeCone uses: Ax2 (axis), R1 (bottom), R2 (top), H (height)
    // We use the default axis (origin, Z-direction)
    BRepPrimAPI_MakeCone maker(radiusBottom, radiusTop, height);
    maker.Build();

    if (!maker.IsDone()) return nullptr;
    return new OcctSolid(maker.Shape());
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
Solid* OcctKernel::BooleanUnion(Solid* a, Solid* b) {
    if (!a || !b) return a ? a : b;

    auto* sa = static_cast<OcctSolid*>(a);
    auto* sb = static_cast<OcctSolid*>(b);

    // BRepAlgoAPI_Fuse performs the boolean OR operation
    BRepAlgoAPI_Fuse fuse(sa->shape, sb->shape);
    fuse.Build();

    if (!fuse.IsDone()) {
        std::cerr << "OCCT Union Error: Fuse failed." << std::endl;
        return new OcctSolid(sa->shape); // Return original if fail
    }

    return new OcctSolid(fuse.Shape());
}




void OcctKernel::ExportSTEP(Solid* s, const char* path)
{
    OcctSolid* occt = (OcctSolid*)s;

    STEPControl_Writer writer;
    writer.Transfer(occt->shape, STEPControl_AsIs);
    writer.Write(path);
}

Solid* OcctKernel::BooleanCut(Solid* a, Solid* b) 
{
    auto sa = (OcctSolid*)a;
    auto sb = (OcctSolid*)b;
    TopoDS_Shape result = BRepAlgoAPI_Cut(sa->shape, sb->shape);

    return new OcctSolid(result);
}


Solid* OcctKernel::Loft(const std::vector<Profile*>& profiles) {
    if (profiles.size() < 2) return nullptr;

    // Parameter 1: isSolid = True
    // Parameter 2: isRuled = True (Essential for Polygons)
    BRepOffsetAPI_ThruSections loftMaker(Standard_True, Standard_True);
    
    for (Profile* p : profiles) {
        auto* op = static_cast<OcctProfile*>(p);
        if (!op || op->wire.IsNull()) continue;

        // --- HEALING STEP ---
        // Ensure the wire is topologically clean and flagged as closed
        BRepLib::BuildCurves3d(op->wire);
        op->wire.Closed(Standard_True);
        
        loftMaker.AddWire(op->wire);
    }

    try {
        loftMaker.Build();
        if (!loftMaker.IsDone() || loftMaker.Shape().IsNull()) {
            std::cerr << "OCCT Loft Build Failed" << std::endl;
            return nullptr;
        }
        return new OcctSolid(loftMaker.Shape());
    } catch (...) {
        return nullptr;
    }
}


StunadMesh* OcctKernel::Tessellate(Solid* solid, float deflection)
{
    return OcctTessellator::Tessellate((OcctSolid*)solid, deflection);
}


//profiles

Profile* OcctKernel::MakeCircleProfile(double r) {
    auto* op = new OcctProfile(r);
    gp_Circ circ(gp_Ax2(gp::Origin(), gp::DZ()), r);
    op->wire = BRepBuilderAPI_MakeWire(BRepBuilderAPI_MakeEdge(circ));
    return op;
}

Profile* OcctKernel::MakeRectProfile(double w, double h) {
    auto* op = new OcctProfile(w, h);
    double hw = w / 2.0;
    double hh = h / 2.0;
    gp_Pnt p1(-hw, -hh, 0), p2(hw, -hh, 0), p3(hw, hh, 0), p4(-hw, hh, 0);
    
    op->wire = BRepBuilderAPI_MakeWire(
        BRepBuilderAPI_MakeEdge(p1, p2),
        BRepBuilderAPI_MakeEdge(p2, p3),
        BRepBuilderAPI_MakeEdge(p3, p4),
        BRepBuilderAPI_MakeEdge(p4, p1)
    );
    return op;
}


Profile* OcctKernel::MakePolygonProfile(const std::vector<std::pair<double, double>>& points) {
    if (points.size() < 3) return nullptr;

    auto* op = new OcctProfile(ProfileKind::Polygon);
    BRepBuilderAPI_MakePolygon polyMaker;
    
    for (const auto& pt : points) {
        polyMaker.Add(gp_Pnt(pt.first, pt.second, 0.0));
    }
    
    // 1. Force closure
    polyMaker.Close();

    if (polyMaker.IsDone()) {
        op->wire = polyMaker.Wire();
        
        // 2. Critical: Heal the topology so OCCT treats the 36 edges as 1 loop
        BRepLib::BuildCurves3d(op->wire);
        
        // 3. Precision: Close any tiny floating point gaps between vertices
        BRepLib::Precision(1e-6); 
    }

    return op;
}

Profile* OcctKernel::MakeSplineProfile(const std::vector<std::pair<double, double>>& points) {
    // 1. Create the profile object with the generic constructor
    auto* op = new OcctProfile(ProfileKind::Spline);

    // 2. Prepare OCCT point array (OCCT arrays are 1-based)
    int n = static_cast<int>(points.size());
    Handle(TColgp_HArray1OfPnt) occtPoints = new TColgp_HArray1OfPnt(1, n);

    for (int i = 0; i < n; ++i) {
        occtPoints->SetValue(i + 1, gp_Pnt(points[i].first, points[i].second, 0.0));
    }

    // 3. Interpolate a periodic (closed) B-Spline
    // Standard_True makes the curve close smoothly back to the start
    GeomAPI_Interpolate interpolator(occtPoints, Standard_True, 1e-6);
    interpolator.Perform();

    if (!interpolator.IsDone()) {
        delete op;
        return nullptr;
    }

    // 4. Convert the Geometry to Topology (Wire)
    TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(interpolator.Curve());
    op->wire = BRepBuilderAPI_MakeWire(edge);
    if (!op->wire.IsNull()) {
    op->wire.Closed(Standard_True);
    }

    return op;
}



Profile* OcctKernel::RotateProfile(Profile* p, double angleDeg) {
    auto* src = static_cast<OcctProfile*>(p);
    auto* out = new OcctProfile(*src);

    gp_Trsf trsf;
    // Rotate around the current Z-axis position of the profile
    gp_Ax1 axis(gp_Pnt(out->x, out->y, out->z), gp::DZ());
    trsf.SetRotation(axis, angleDeg * M_PI / 180.0);

    BRepBuilderAPI_Transform xf(out->wire, trsf);
    out->wire = TopoDS::Wire(xf.Shape());

    out->rotDeg += angleDeg;
    return out;
}

Profile* OcctKernel::ScaleProfile(Profile* p, double s) {
    auto* src = static_cast<OcctProfile*>(p);
    auto* out = new OcctProfile(*src);

    // 1. Update the metadata
    if (out->kind == ProfileKind::Circle) {
        out->radius *= s;
    } else if (out->kind == ProfileKind::Rect) {
        out->width *= s;
        out->height *= s;
    }

    // 2. Transform the geometry (Choice B)
    gp_Trsf trsf;
    // Scale centered at the profile's current position (x, y, z)
    trsf.SetScale(gp_Pnt(out->x, out->y, out->z), s);
    
    BRepBuilderAPI_Transform xf(out->wire, trsf);
    out->wire = TopoDS::Wire(xf.Shape());

    return out;
}



Profile* OcctKernel::TranslateProfile(Profile* p, double x, double y, double z) {
    auto* src = (OcctProfile*)p;
    
    // Create a transformation matrix
    gp_Trsf trsf;
    trsf.SetTranslation(gp_Vec(x, y, z));
    
    // Apply transformation to the wire
    BRepBuilderAPI_Transform transform(src->wire, trsf);
    
    // Create new profile and store the transformed wire
    auto* result = new OcctProfile(*src); // Use your copy constructor
    result->wire = TopoDS::Wire(transform.Shape());
    result->wire.Closed(Standard_True);
    
    return result;
}






