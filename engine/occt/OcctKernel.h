#pragma once
#include "../core/Kernel.h"
#include <string>
#include <TopoDS_Shape.hxx>
class OcctKernel : public Kernel
{
public:
    Solid* MakeBox(double x, double y, double z) override;

    
    Solid* MakeCylinder(double r, double h) override;
    Solid* MakeSphere(double radius) override;
    Solid* MakeCone(double radiusBottom, double radiusTop, double height) override;




    Solid* Fillet(Solid* s, double r) override;
    Solid* Shell(Solid* s, double t) override;
    void ExportSTEP(Solid* s, const char* path) override;
    Solid* BooleanCut(Solid* a, Solid* b) override;
    Solid* BooleanUnion(Solid* a, Solid* b) override;
    Solid* Loft(const std::vector<Profile*>& profiles) override;
    Solid* FilletByRule(Solid* s, double radius, const std::string& rule) override;
    Solid* TranslateSolid(Solid* s, double dx, double dy, double dz) override;

    //postprocess
    StunadMesh* Tessellate(Solid* solid, float linearDeflection) override;


    //transform solid
    Solid* RotateSolid(Solid* s, double rx, double ry, double rz) override;
    Solid* ScaleSolid(Solid* s, double factor) override;
    Solid* BooleanIntersect(Solid* a, Solid* b) override;
    Solid* ScaleSolidNonUniform(Solid* s, double sx, double sy, double sz) override;



   //profile create
    Profile* MakeCircleProfile(double r) override;
    Profile* MakeRectProfile(double x, double y) override;
    Profile* MakePolygonProfile(const std::vector<std::pair<double, double>>& points) override;
    Profile* MakeSplineProfile(const std::vector<std::pair<double, double>>& points) override;

    //profile transforms
    Profile* RotateProfile(Profile* p, double angleDeg) override;
    Profile* TranslateProfile(Profile* p, double x,double y , double z) override;
    Profile* ScaleProfile(Profile* p, double s) override;

private:
    bool HealInternal(TopoDS_Shape& shape);


};
