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
#include <TopTools_IndexedMapOfShape.hxx>
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
#include < BRepBuilderAPI_MakeSolid.hxx>
#include <BRepOffsetAPI_MakePipe.hxx>
#include <BRepPrimAPI_MakeRevol.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <GCPnts_AbscissaPoint.hxx>
#include <BRepAdaptor_CompCurve.hxx>
#include <memory>
#include <cstdio>

std::unique_ptr<Solid> OcctKernel::MakeBox(double x, double y, double z)
{
    TopoDS_Shape box = BRepPrimAPI_MakeBox(x, y, z).Shape();
    return std::make_unique<OcctSolid>(box);
}

std::unique_ptr<Solid> OcctKernel::MakeCylinder(double r, double h)
{
    return std::make_unique<OcctSolid>(BRepPrimAPI_MakeCylinder(r, h).Shape());
}


std::unique_ptr<Solid> OcctKernel::MakeSphere(double radius) {
    if (radius <= 1e-6) return nullptr;

    // Creates a sphere at the origin (0,0,0)
    BRepPrimAPI_MakeSphere maker(radius);
    maker.Build();

    if (!maker.IsDone()) return nullptr;
    return std::make_unique<OcctSolid>(maker.Shape());
}

std::unique_ptr<Solid>OcctKernel::MakeCone(double radiusBottom, double radiusTop, double height) {
    if (height <= 1e-6) return nullptr;

    // OCCT's MakeCone uses: Ax2 (axis), R1 (bottom), R2 (top), H (height)
    // We use the default axis (origin, Z-direction)
    BRepPrimAPI_MakeCone maker(radiusBottom, radiusTop, height);
    maker.Build();

    if (!maker.IsDone()) return nullptr;
    return std::make_unique<OcctSolid>(maker.Shape());
}




std::unique_ptr<Solid> OcctKernel::Shell(Solid* s, double t)
{
    auto so = (OcctSolid*)s;

    BRepOffsetAPI_MakeThickSolid shell;
    TopTools_ListOfShape faces;  // empty means "shell all faces"

    shell.MakeThickSolidByJoin(so->shape, faces, -t, 1e-3);

    return std::make_unique<OcctSolid>(shell.Shape());
}

std::unique_ptr<Solid> OcctKernel::BooleanUnion(Solid* a, Solid* b) {
    if (!a && !b) return nullptr;
    if (!a) return std::make_unique<OcctSolid>(static_cast<OcctSolid*>(b)->shape);
    if (!b) return std::make_unique<OcctSolid>(static_cast<OcctSolid*>(a)->shape);

    auto* sa = static_cast<OcctSolid*>(a);
    auto* sb = static_cast<OcctSolid*>(b);

    // BRepAlgoAPI_Fuse performs the boolean OR operation
    BRepAlgoAPI_Fuse fuse(sa->shape, sb->shape);
    fuse.Build();

    if (!fuse.IsDone()) {
        std::cerr << "OCCT Union Error: Fuse failed." << std::endl;
        return std::make_unique<OcctSolid>(sa->shape); // Return original if fail
    }

    TopoDS_Shape result = fuse.Shape();
    HealInternal(result);
    return std::make_unique<OcctSolid>(fuse.Shape());
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

std::unique_ptr<Solid> OcctKernel::BooleanCut(Solid* a, Solid* b) {
    if (!a || !b) return a ? std::make_unique<OcctSolid>(static_cast<OcctSolid*>(a)->shape) : nullptr;

    auto* sa = static_cast<OcctSolid*>(a);
    auto* sb = static_cast<OcctSolid*>(b);

    // Verify both are solids. If one is a shell (open profile loft), Booleans will likely fail.
    if (sa->shape.ShapeType() != TopAbs_SOLID || sb->shape.ShapeType() != TopAbs_SOLID) {
        printf("[Kernel] Warning: BooleanCut requires manifold solids. Input might be an open shell.\n");
    }

    BRepAlgoAPI_Cut cutter(sa->shape, sb->shape);
    cutter.Build();

    if (!cutter.IsDone()) return std::make_unique<OcctSolid>(sa->shape);
    return std::make_unique<OcctSolid>(cutter.Shape());
}

std::unique_ptr<Solid> OcctKernel::Loft(const std::vector<Profile*>& profiles, bool ruled, bool makeSolid) {
    if (profiles.size() < 2) return nullptr;

    // Use the makeSolid flag to initialize the loft maker.
    // Standard_True in the first param attempts to build a solid volume immediately.
    BRepOffsetAPI_ThruSections loftMaker(makeSolid, ruled);
    
    bool allClosed = true;

    for (Profile* p : profiles) {
        auto* op = static_cast<OcctProfile*>(p);
        if (!op || op->wire.IsNull()) continue;

        // 1. Ensure 3D curves exist for the wire (Critical for Splines)
        BRepLib::BuildCurves3d(op->wire);
        
        // 2. Update global closure status
        if (!op->wire.Closed()) {
            allClosed = false;
        }
        
        loftMaker.AddWire(op->wire);
    }

    try {
        loftMaker.Build();
        if (!loftMaker.IsDone() || loftMaker.Shape().IsNull()) {
            printf("[OcctKernel] Loft Build Failed\n");
            return nullptr;
        }

        TopoDS_Shape finalShape = loftMaker.Shape();
        
        // 3. THE HEALER PASS
        // Heal before attempting to make a solid to fix twisted faces.
        HealInternal(finalShape); 

        // 4. Force Solidification if intent is Solid but output is a Shell
        // This happens if OCCT builds the faces but doesn't "sew" them into a volume automatically.
        if (makeSolid && allClosed && finalShape.ShapeType() == TopAbs_SHELL) {
            BRepBuilderAPI_MakeSolid solidMaker(TopoDS::Shell(finalShape));
            if (solidMaker.IsDone()) {
                finalShape = solidMaker.Solid();
                // Optional: Heal again if solidification introduced new issues
                HealInternal(finalShape); 
            } else {
                printf("[OcctKernel] Warning: Loft was closed but could not be made into a Solid.\n");
            }
        }

        return std::make_unique<OcctSolid>(finalShape);

    } catch (...) {
        printf("[OcctKernel] Critical Exception in Loft\n");
        return nullptr;
    }
}



std::unique_ptr<Solid> OcctKernel::Sweep(Profile* profile, Profile* path, bool makeSolid) {
    if (!profile || !path) return nullptr;

    auto* occProfile = static_cast<OcctProfile*>(profile);
    auto* occPath = static_cast<OcctProfile*>(path);

    // Ensure 3D curves are built for the path spline
    BRepLib::BuildCurves3d(occPath->wire);

    TopoDS_Shape sectionShape = occProfile->wire;
    if (makeSolid) {
        BRepBuilderAPI_MakeFace faceMaker(occProfile->wire);
        if (faceMaker.IsDone()) sectionShape = faceMaker.Face();
    }

    // Generate the sweep
    BRepOffsetAPI_MakePipe pipeMaker(occPath->wire, sectionShape);
    pipeMaker.Build();

    if (!pipeMaker.IsDone()) return nullptr;
    TopoDS_Shape result = pipeMaker.Shape();

    // Force Solidification if it's still a shell
    if (makeSolid && result.ShapeType() != TopAbs_SOLID) {
        BRepBuilderAPI_MakeSolid solidMaker;
        for (TopExp_Explorer exp(result, TopAbs_SHELL); exp.More(); exp.Next()) {
            solidMaker.Add(TopoDS::Shell(exp.Current()));
        }
        if (solidMaker.IsDone()) result = solidMaker.Solid();
    }

    HealInternal(result);
    return std::make_unique<OcctSolid>(result);
}


std::unique_ptr<Solid> OcctKernel::Revolve(Profile* p, double angleDeg, bool makeSolid) {
    if (!p) return nullptr;
    auto* occP = static_cast<OcctProfile*>(p);

    TopoDS_Shape profileToRevolve = occP->wire;

    // If solid is requested, promote the Wire to a Face
    if (makeSolid) {
        BRepBuilderAPI_MakeFace faceMaker(occP->wire);
        if (faceMaker.IsDone()) {
            profileToRevolve = faceMaker.Face();
        }
    }

    // Revolve directly around the Z-axis
    // The user/AI is now responsible for ensuring the profile is "standing up" via RotateProfile3D
    gp_Ax1 axis(gp::Origin(), gp::DZ());
    BRepPrimAPI_MakeRevol revol(profileToRevolve, axis, angleDeg * M_PI / 180.0);
    revol.Build();

    if (!revol.IsDone()) return nullptr;
    TopoDS_Shape result = revol.Shape();

    // Solidify if necessary
    if (makeSolid && result.ShapeType() != TopAbs_SOLID) {
        BRepBuilderAPI_MakeSolid solidMaker;
        for (TopExp_Explorer exp(result, TopAbs_SHELL); exp.More(); exp.Next()) {
            solidMaker.Add(TopoDS::Shell(exp.Current()));
        }
        if (solidMaker.IsDone()) result = solidMaker.Solid();
    }

    HealInternal(result);
    return std::make_unique<OcctSolid>(result);
}

std::unique_ptr<Solid> OcctKernel::Extrude(Profile* p, double height, bool makeSolid) {
    if (!p) return nullptr;
    auto* occP = static_cast<OcctProfile*>(p);

    // Ensure the wire is valid and closed
    if (occP->wire.IsNull()) return nullptr;

    TopoDS_Shape profileToExtrude = occP->wire;

    // MANDATORY: For a solid result, we must extrude a FACE
    if (makeSolid && occP->wire.Closed()) {
        BRepBuilderAPI_MakeFace faceMaker(occP->wire);
        if (faceMaker.IsDone()) {
            profileToExtrude = faceMaker.Face();
        }
    }

    gp_Vec vec(0.0, 0.0, height);
    BRepPrimAPI_MakePrism prism(profileToExtrude, vec);
    prism.Build();

    if (!prism.IsDone()) return nullptr;

    TopoDS_Shape result = prism.Shape();
    
    // Sometimes for small heights, OCCT creates a Shell even from a Face.
    // This forces it into a Solid object.
    if (makeSolid && result.ShapeType() != TopAbs_SOLID) {
        BRepBuilderAPI_MakeSolid solidMaker;
        for (TopExp_Explorer exp(result, TopAbs_SHELL); exp.More(); exp.Next()) {
            solidMaker.Add(TopoDS::Shell(exp.Current()));
        }
        if (solidMaker.IsDone()) result = solidMaker.Solid();
    }

    HealInternal(result);
    return std::make_unique<OcctSolid>(result);
}


std::unique_ptr<Solid> OcctKernel::Thicken(Solid* s, double thickness) {
    if (!s) return nullptr;
    auto* occS = static_cast<OcctSolid*>(s);
    
    // Safety check: Thicken is usually performed on Shells/Surfaces.
    // If it's already a solid, this will create an "offset" solid.
    
    Standard_Real tol = 1e-3;
    BRepOffsetAPI_MakeThickSolid offsetMaker;
    TopTools_ListOfShape closingFaces; // Empty list means thicken the whole shape
    
    try {
        // -thickness creates an "outward" or "inward" offset depending on normals
        offsetMaker.MakeThickSolidByJoin(occS->shape, closingFaces, thickness, tol);
        offsetMaker.Build();

        if (offsetMaker.IsDone()) {
            TopoDS_Shape result = offsetMaker.Shape();
            HealInternal(result);
            return std::make_unique<OcctSolid>(result);
        }
    } catch (...) {
        printf("[OcctKernel] Thicken failed for op.\n");
    }
    
    return nullptr;
}







std::unique_ptr<Solid> OcctKernel::RotateSolid(Solid* s, double rx, double ry, double rz) {
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
    return std::make_unique<OcctSolid>(transformer.Shape());
}

std::unique_ptr<Solid> OcctKernel::ScaleSolid(Solid* s, double factor) {
    auto* occSolid = static_cast<OcctSolid*>(s);
    if (!occSolid) return nullptr;
    if (factor <= 0) return std::make_unique<OcctSolid>(occSolid->shape);

    gp_Trsf trsf;
    trsf.SetScale(gp::Origin(), factor);

    BRepBuilderAPI_Transform transformer(occSolid->shape, trsf);
    return std::make_unique<OcctSolid>(transformer.Shape());
}

std::unique_ptr<Solid> OcctKernel::BooleanIntersect(Solid* a, Solid* b) {
    if (!a || !b) return nullptr;

    auto* sa = static_cast<OcctSolid*>(a);
    auto* sb = static_cast<OcctSolid*>(b);

    BRepAlgoAPI_Common common(sa->shape, sb->shape);
    common.Build();

    if (!common.IsDone()) {
        return std::make_unique<OcctSolid>(sa->shape); 
    }

    return std::make_unique<OcctSolid>(common.Shape());
}


std::unique_ptr<Solid> OcctKernel::ScaleSolidNonUniform(Solid* s, double sx, double sy, double sz) {
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
        return std::make_unique<OcctSolid>(occSolid->shape);
    }

    return std::make_unique<OcctSolid>(transformer.Shape());
}
















StunadMesh* OcctKernel::Tessellate(Solid* solid, float deflection)
{
    return OcctTessellator::Tessellate((OcctSolid*)solid, deflection);
}


//profiles

std::unique_ptr<Profile> OcctKernel::MakeCircleProfile(double r) {
    auto op = std::make_unique<OcctProfile>(r);
    gp_Circ circ(gp_Ax2(gp::Origin(), gp::DZ()), r);
    op->wire = BRepBuilderAPI_MakeWire(BRepBuilderAPI_MakeEdge(circ));
    return op;
}

std::unique_ptr<Profile> OcctKernel::MakeRectProfile(double w, double h) {
    auto op = std::make_unique<OcctProfile>(w, h);
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


std::unique_ptr<Profile> OcctKernel::MakePolygonProfile(const std::vector<std::pair<double, double>>& points) {
    if (points.size() < 3) return nullptr;

    auto op = std::make_unique<OcctProfile>(ProfileKind::Polygon);
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

std::unique_ptr<Profile> OcctKernel::MakeSplineProfile(const std::vector<std::pair<double, double>>& points, bool isClosed) {
    auto op = std::make_unique<OcctProfile>(ProfileKind::Spline);
    int n = static_cast<int>(points.size());
    if (n < 2) return nullptr;

    Handle(TColgp_HArray1OfPnt) occtPoints = new TColgp_HArray1OfPnt(1, n);
    for (int i = 0; i < n; ++i) {
        occtPoints->SetValue(i + 1, gp_Pnt(points[i].first, points[i].second, 0.0));
    }

    // Standard_True makes it periodic (smoothly closed)
    GeomAPI_Interpolate interpolator(occtPoints, isClosed, 1e-6);
    interpolator.Perform();

    if (!interpolator.IsDone()) return nullptr;

    TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(interpolator.Curve());
    op->wire = BRepBuilderAPI_MakeWire(edge);
    
    if (isClosed) {
        op->wire.Closed(Standard_True);
        // Robustness: Try to make a face. Lofts between faces are more stable.
        BRepBuilderAPI_MakeFace faceMaker(op->wire);
        if (faceMaker.IsDone()) { /* Face generated internally */ }
    }

    return op;
}



std::unique_ptr<Profile> OcctKernel::RotateProfile(Profile* p, double angleDeg) {
    auto* src = static_cast<OcctProfile*>(p);
    auto out = std::make_unique<OcctProfile>(*src);

    gp_Trsf trsf;
    // Rotate around the current Z-axis position of the profile
    gp_Ax1 axis(gp_Pnt(out->x, out->y, out->z), gp::DZ());
    trsf.SetRotation(axis, angleDeg * M_PI / 180.0);

    BRepBuilderAPI_Transform xf(out->wire, trsf);
    out->wire = TopoDS::Wire(xf.Shape());

    out->rotDeg += angleDeg;
    return out;
}

std::unique_ptr<Profile> OcctKernel::ScaleProfile(Profile* p, double s) {
    auto* src = static_cast<OcctProfile*>(p);
    auto out = std::make_unique<OcctProfile>(*src);

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



std::unique_ptr<Profile> OcctKernel::TranslateProfile(Profile* p, double x, double y, double z) {
    auto* src = static_cast<OcctProfile*>(p);
    
    gp_Trsf trsf;
    trsf.SetTranslation(gp_Vec(x, y, z));
    
    BRepBuilderAPI_Transform transform(src->wire, trsf);
    
    auto result = std::make_unique<OcctProfile>(*src); 
    result->wire = TopoDS::Wire(transform.Shape());
    
    // Crucial: Update the logical center of the profile
    result->x += x;
    result->y += y;
    result->z += z;
    
    return result;
}



std::unique_ptr<Solid> OcctKernel::FilletEdges(Solid* s, double radius, FilletType mode) {
    if (!s) return nullptr;
    auto* occSolid = static_cast<OcctSolid*>(s);
    
    if (radius < 1e-6) return std::make_unique<OcctSolid>(occSolid->shape);

    BRepFilletAPI_MakeFillet filler(occSolid->shape);
    
    // FIX 1: Use a Map to prevent adding the same edge twice
    TopTools_IndexedMapOfShape edgeMap;
    TopExp::MapShapes(occSolid->shape, TopAbs_EDGE, edgeMap);
    
    bool foundAny = false;

    for (int i = 1; i <= edgeMap.Extent(); ++i) {
        TopoDS_Edge E = TopoDS::Edge(edgeMap(i));
        bool shouldFillet = false;

        if (mode == FilletType::All) { // All
            shouldFillet = true;
        } 
        else {
            Standard_Real first, last;
            Handle(Geom_Curve) curve = BRep_Tool::Curve(E, first, last);
            if (!curve.IsNull()) {
                gp_Pnt pStart = curve->Value(first);
                gp_Pnt pEnd = curve->Value(last);
                
                double dz = std::abs(pStart.Z() - pEnd.Z());
                double dxy = std::sqrt(std::pow(pStart.X() - pEnd.X(), 2) + 
                                       std::pow(pStart.Y() - pEnd.Y(), 2));

                // Mode 1: Vertical Ribs
                // Check: dz is significant AND dxy is effectively zero (or much smaller than dz)
                if (mode == FilletType::Vertical && dz > 1e-3 && dxy < 1e-3) {
                    shouldFillet = true;
                } 
                // Mode 2: Planar Caps
                // Check: Start and End points are on the same Z plane
                else if (mode == FilletType::Planar && dz < 1e-4) {
                    shouldFillet = true;
                }
            }
        }

        if (shouldFillet) {
            try {
                filler.Add(radius, E);
                foundAny = true;
            } catch (...) {
                // Individual edge might fail if radius is too large for its length
                continue; 
            }
        }
    }

    if (!foundAny) return std::make_unique<OcctSolid>(occSolid->shape);

    try {
        filler.Build();
        if (filler.IsDone()) {
            TopoDS_Shape result = filler.Shape();
            HealInternal(result); 
            return std::make_unique<OcctSolid>(result);
        } else {
            printf("[OcctKernel] Fillet failed: check radius size against edge lengths.\n");
        }
    } catch (...) {
        printf("[OcctKernel] Fillet Build Critical Failure\n");
    }

    return std::make_unique<OcctSolid>(occSolid->shape);
}
std::unique_ptr<Solid> OcctKernel::TranslateSolid(Solid* s, double dx, double dy, double dz) {
    auto* occSolid = static_cast<OcctSolid*>(s);
    
    gp_Trsf trsf;
    trsf.SetTranslation(gp_Vec(dx, dy, dz));
    
    BRepBuilderAPI_Transform transformer(occSolid->shape, trsf);
    if (transformer.IsDone()) {
        return std::make_unique<OcctSolid>(transformer.Shape());
    }
    
    // Fallback: return original if transform fails
    return std::make_unique<OcctSolid>(occSolid->shape);
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




std::unique_ptr<Profile> OcctKernel::RotateProfile3D(Profile* p, double angleDeg, double ax, double ay, double az) {
    auto* src = static_cast<OcctProfile*>(p);
    if (!src || src->wire.IsNull()) return nullptr;

    // 1. Create the rotation axis (origin + direction vector)
    // We rotate around the profile's current logical center
    gp_Pnt center(src->x, src->y, src->z);
    gp_Dir direction(ax, ay, az);
    gp_Ax1 rotationAxis(center, direction);

    // 2. Define the transformation
    gp_Trsf trsf;
    trsf.SetRotation(rotationAxis, angleDeg * M_PI / 180.0);

    // 3. Apply transformation to a copy of the wire
    BRepBuilderAPI_Transform transformer(src->wire, trsf);
    
    auto result = std::make_unique<OcctProfile>(*src);
    result->wire = TopoDS::Wire(transformer.Shape());

    // Note: Since this is a 3D rotation, the 2D 'rotDeg' metadata 
    // might become ambiguous, but the wire itself is now correctly oriented.
    return result;
}






std::unique_ptr<Profile> OcctKernel::SetProfilePlane(Profile* p, double originX, double originY, double originZ, double dirX, double dirY, double dirZ) 
{
    auto* src = static_cast<OcctProfile*>(p);
    if (!src || src->wire.IsNull()) return nullptr;

    // 1. Define the target coordinate system (Origin + Normal direction)
    gp_Pnt origin(originX, originY, originZ);
    gp_Dir normal(dirX, dirY, dirZ);
    gp_Ax2 targetPlane(origin, normal);

    // 2. Create the transformation from the default XY plane to the target plane
    gp_Trsf trsf;
    trsf.SetTransformation(targetPlane, gp::XOY()); 

    // 3. Apply the transform
    BRepBuilderAPI_Transform transformer(src->wire, trsf);
    
    auto result = std::make_unique<OcctProfile>(*src);
    result->wire = TopoDS::Wire(transformer.Shape());

    // Update logical center to match the new origin
    result->x = originX;
    result->y = originY;
    result->z = originZ;

    return result;
}




std::unique_ptr<Profile> OcctKernel::AlignProfileToPath(Profile* profile, Profile* path) {
    auto* occProfile = static_cast<OcctProfile*>(profile);
    auto* occPath = static_cast<OcctProfile*>(path);

    if (!occProfile || !occPath || occPath->wire.IsNull()) return nullptr;

    // 1. Use CompCurve instead of BRepAdaptor_Curve to handle TopoDS_Wire
    BRepAdaptor_CompCurve adaptor(occPath->wire);
    
    // 2. Get the start point (P) and tangent (V) at the first parameter
    gp_Pnt startPoint;
    gp_Vec tangent;
    
    // FirstParameter() and LastParameter() work the same way for CompCurve
    adaptor.D1(adaptor.FirstParameter(), startPoint, tangent);

    // 3. Mathematical Safety
    if (tangent.SquareMagnitude() < 1e-9) {
        tangent = gp_Vec(0, 0, 1); 
    }

    // 4. Reuse your SetProfilePlane logic
    return SetProfilePlane(profile, 
                           startPoint.X(), startPoint.Y(), startPoint.Z(), 
                           tangent.X(), tangent.Y(), tangent.Z());
}



// --- Mirroring ---
std::unique_ptr<Solid> OcctKernel::Mirror(Solid* s, double ox, double oy, double oz, double dx, double dy, double dz) {
    auto* occS = static_cast<OcctSolid*>(s);
    gp_Ax2 mirrorPlane(gp_Pnt(ox, oy, oz), gp_Dir(dx, dy, dz));
    gp_Trsf trsf;
    trsf.SetMirror(mirrorPlane);
    
    BRepBuilderAPI_Transform transformer(occS->shape, trsf, Standard_True); // True = Copy
    return std::make_unique<OcctSolid>(transformer.Shape());
}

// --- Patterning ---
std::unique_ptr<Solid> OcctKernel::PatternLinear(Solid* s, int count, double spacing, double dx, double dy, double dz) {
    auto* occS = static_cast<OcctSolid*>(s);
    TopoDS_Shape result = occS->shape;
    gp_Vec step(gp_Dir(dx, dy, dz));
    step *= spacing;

    for (int i = 1; i < count; ++i) {
        gp_Trsf trsf;
        trsf.SetTranslation(step * i);
        BRepBuilderAPI_Transform transformer(occS->shape, trsf, Standard_True);
        BRepAlgoAPI_Fuse fuse(result, transformer.Shape());
        fuse.Build();
        if (fuse.IsDone()) result = fuse.Shape();
    }
    return std::make_unique<OcctSolid>(result);
}

std::unique_ptr<Solid> OcctKernel::PatternCircular(Solid* s, int count, double angleDeg, double ax, double ay, double az) {
    auto* occS = static_cast<OcctSolid*>(s);
    TopoDS_Shape result = occS->shape;
    gp_Ax1 axis(gp::Origin(), gp_Dir(ax, ay, az));

    for (int i = 1; i < count; ++i) {
        gp_Trsf trsf;
        trsf.SetRotation(axis, (angleDeg * M_PI / 180.0) * i);
        BRepBuilderAPI_Transform transformer(occS->shape, trsf, Standard_True);
        BRepAlgoAPI_Fuse fuse(result, transformer.Shape());
        fuse.Build();
        if (fuse.IsDone()) result = fuse.Shape();
    }
    return std::make_unique<OcctSolid>(result);
}



std::unique_ptr<Solid> OcctKernel::PatternSpiral(Solid* s, int count, double totalAngle, double totalRise, double ax, double ay, double az) {
    auto* occS = static_cast<OcctSolid*>(s);
    TopoDS_Shape result = occS->shape;
    
    gp_Ax1 axis(gp::Origin(), gp_Dir(ax, ay, az));
    double angleStep = (totalAngle * M_PI / 180.0) / (count - 1);
    double riseStep = totalRise / (count - 1);
    gp_Vec moveVec(gp_Dir(ax, ay, az));

    for (int i = 1; i < count; ++i) {
        gp_Trsf rot, trans;
        rot.SetRotation(axis, angleStep * i);
        trans.SetTranslation(moveVec * (riseStep * i));
        
        gp_Trsf finalTrsf = trans * rot;
        
        BRepBuilderAPI_Transform transformer(occS->shape, finalTrsf, Standard_True);
        BRepAlgoAPI_Fuse fuse(result, transformer.Shape());
        fuse.Build();
        if (fuse.IsDone()) result = fuse.Shape();
    }
    return std::make_unique<OcctSolid>(result);
}