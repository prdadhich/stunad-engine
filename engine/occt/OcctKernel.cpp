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
#include <Geom_Line.hxx>     
#include <gp_Lin.hxx>        
#include <Standard_Type.hxx>  
#include <BRepBuilderAPI_GTransform.hxx>
#include <gp_GTrsf.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <gp_Dir.hxx>
#include <BRep_Tool.hxx>
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
#include <ShapeFix_Shape.hxx>

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




void OcctKernel::ExportSTEP(Solid* s, const char* path) {
    if (!s) {
        printf("[OcctKernel] Export Error: Solid is null.\n");
        return;
    }

    OcctSolid* occt = static_cast<OcctSolid*>(s);
    STEPControl_Writer writer;

    // IFSelect_RetDone is 0. 
    // We want to proceed ONLY if status is 0.
    IFSelect_ReturnStatus transferStat = writer.Transfer(occt->shape, STEPControl_AsIs);
    
    if (transferStat != IFSelect_RetDone) {
        printf("[OcctKernel] STEP Transfer failed with code: %d\n", (int)transferStat);
        return;
    }

    printf("[OcctKernel] Shape transferred successfully. Writing to disk...\n");

    IFSelect_ReturnStatus writeStat = writer.Write(path);
    
    if (writeStat == IFSelect_RetDone) {
        printf("[OcctKernel] DONE! File successfully created at: %s\n", path);
    } else {
        printf("[OcctKernel] WRITE FAILED! Code: %d. Check if file is open in another app.\n", (int)writeStat);
    }
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

    // Standard_True, Standard_True ensures a solid, ruled (straight-edged) shape
    BRepOffsetAPI_ThruSections loftMaker(Standard_True, Standard_False);
    
    for (Profile* p : profiles) {
        auto* op = static_cast<OcctProfile*>(p);
        if (!op || op->wire.IsNull()) continue;

        // Keep your original wire healing logic
        BRepLib::BuildCurves3d(op->wire);
        op->wire.Closed(Standard_True);
        
        loftMaker.AddWire(op->wire);
    }

    try {
        loftMaker.Build();
        if (!loftMaker.IsDone() || loftMaker.Shape().IsNull()) {
            printf("[OcctKernel] Loft Build Failed\n");
            return nullptr;
        }

        // --- NEW: THE HEALER PASS ---
        // We take the raw shape out of the maker
        TopoDS_Shape finalShape = loftMaker.Shape();
        
        // We use the healer to fix twisted faces or non-manifold edges 
        // that lofting polygons often creates.
        HealInternal(finalShape); 

        // Now return the stable container
        return new OcctSolid(finalShape);

    } catch (...) {
        printf("[OcctKernel] Critical Exception in Loft\n");
        return nullptr;
    }
}

Solid* OcctKernel::RotateSolid(Solid* s, double rx, double ry, double rz) {
    auto* occSolid = static_cast<OcctSolid*>(s);
    if (!occSolid) return nullptr;
    gp_Trsf trsf;

    // Create rotations for each axis (Euler angles)
    gp_Trsf rotX, rotY, rotZ;
    rotX.SetRotation(gp_Ax1(gp::Origin(), gp::DX()), rx * M_PI / 180.0);
    rotY.SetRotation(gp_Ax1(gp::Origin(), gp::DY()), ry * M_PI / 180.0);
    rotZ.SetRotation(gp_Ax1(gp::Origin(), gp::DZ()), rz * M_PI / 180.0);

    // Combine transformations: Z * Y * X
    trsf = rotZ * (rotY * rotX);

    BRepBuilderAPI_Transform transformer(occSolid->shape, trsf);
    return new OcctSolid(transformer.Shape());
}

Solid* OcctKernel::ScaleSolid(Solid* s, double factor) {
    auto* occSolid = static_cast<OcctSolid*>(s);
    if (!occSolid) return nullptr;
    if (factor <= 0) return new OcctSolid(occSolid->shape);

    gp_Trsf trsf;
    trsf.SetScale(gp::Origin(), factor);

    BRepBuilderAPI_Transform transformer(occSolid->shape, trsf);
    return new OcctSolid(transformer.Shape());
}

Solid* OcctKernel::BooleanIntersect(Solid* a, Solid* b) {
    if (!a || !b) return nullptr;

    auto* sa = static_cast<OcctSolid*>(a);
    auto* sb = static_cast<OcctSolid*>(b);

    BRepAlgoAPI_Common common(sa->shape, sb->shape);
    common.Build();

    if (!common.IsDone()) {
        return new OcctSolid(sa->shape); 
    }

    return new OcctSolid(common.Shape());
}


Solid* OcctKernel::ScaleSolidNonUniform(Solid* s, double sx, double sy, double sz) {
    auto* occSolid = static_cast<OcctSolid*>(s);
    
    if (!occSolid || occSolid->shape.IsNull()) {
        return nullptr;
    }
    // gp_GTrsf is required for non-uniform scaling
    gp_GTrsf gtrsf;
    gtrsf.SetVectorialPart(gp_Mat(sx, 0, 0, 
                                  0, sy, 0, 
                                  0, 0, sz));

   BRepBuilderAPI_GTransform transformer(occSolid->shape, gtrsf);
    
    if (!transformer.IsDone()) {
        return new OcctSolid(occSolid->shape);
    }

    return new OcctSolid(transformer.Shape());
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



Solid* OcctKernel::FilletByRule(Solid* s, double radius, const std::string& rule) {
    auto* occSolid = static_cast<OcctSolid*>(s);
    BRepFilletAPI_MakeFillet filler(occSolid->shape);
    
    bool foundAny = false;
    TopExp_Explorer ex(occSolid->shape, TopAbs_EDGE);
    
    while (ex.More()) {
        TopoDS_Edge E = TopoDS::Edge(ex.Current());
        
        bool shouldFillet = false;

        // --- RULE ENGINE ---
        if (rule == "all_edges") {
            shouldFillet = true;
        } 
        else if (rule == "vertical_edges") {
            // Logic: Check if edge direction is parallel to Z-axis
            Standard_Real first, last;
            Handle(Geom_Curve) curve = BRep_Tool::Curve(E, first, last);
            if (!curve.IsNull() && curve->IsKind(STANDARD_TYPE(Geom_Line))) {
                gp_Dir dir = Handle(Geom_Line)::DownCast(curve)->Lin().Direction();
                if (dir.IsParallel(gp::DZ(), 0.1)) shouldFillet = true;
            }
        }
        else if (rule == "horizontal_edges") {
             Standard_Real first, last;
             Handle(Geom_Curve) curve = BRep_Tool::Curve(E, first, last);
             if (!curve.IsNull() && curve->IsKind(STANDARD_TYPE(Geom_Line))) {
                gp_Dir dir = Handle(Geom_Line)::DownCast(curve)->Lin().Direction();
                if (std::abs(dir.Z()) < 0.1) shouldFillet = true;
             }
        }

        if (shouldFillet) {
            filler.Add(radius, E);
            foundAny = true;
        }
        ex.Next();
    }

    if (!foundAny) return new OcctSolid(occSolid->shape);

    filler.Build();
    if (filler.IsDone()) {
        return new OcctSolid(filler.Shape());
    }
    return new OcctSolid(occSolid->shape);
}


Solid* OcctKernel::TranslateSolid(Solid* s, double dx, double dy, double dz) {
    auto* occSolid = static_cast<OcctSolid*>(s);
    
    gp_Trsf trsf;
    trsf.SetTranslation(gp_Vec(dx, dy, dz));
    
    BRepBuilderAPI_Transform transformer(occSolid->shape, trsf);
    if (transformer.IsDone()) {
        return new OcctSolid(transformer.Shape());
    }
    
    // Fallback: return original if transform fails
    return new OcctSolid(occSolid->shape);
}


bool OcctKernel::HealInternal(TopoDS_Shape& shape) {
    BRepCheck_Analyzer analyzer(shape);
    if (analyzer.IsValid()) return true;

    // printf is okay here since we use it for kernel logging
    printf("[OcctKernel] Healing invalid geometry...\n");

    ShapeFix_Shape fixer(shape);
    fixer.Init(shape);
    fixer.Perform();
    
    TopoDS_Shape fixedShape = fixer.Shape();
    
    // Check again
    BRepCheck_Analyzer postAnalyzer(fixedShape);
    if (postAnalyzer.IsValid()) {
        shape = fixedShape;
        return true;
    }
    
    return false; // Even the healer couldn't save it
}




