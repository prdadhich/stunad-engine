#pragma once
#include "../core/Kernel.h"

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

    //postprocess
    StunadMesh* Tessellate(Solid* solid, float linearDeflection) override;

   //profile create
    Profile* MakeCircleProfile(double r) override;
    Profile* MakeRectProfile(double x, double y) override;
    Profile* MakePolygonProfile(const std::vector<std::pair<double, double>>& points) override;
    Profile* MakeSplineProfile(const std::vector<std::pair<double, double>>& points) override;

    //profile transforms
    Profile* RotateProfile(Profile* p, double angleDeg) override;
    Profile* TranslateProfile(Profile* p, double x,double y , double z) override;
    Profile* ScaleProfile(Profile* p, double s) override;



};
