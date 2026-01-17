#pragma once
#include "Solid.h"
#include "Mesh.h"
#include "Profile.h"

class Kernel
{
public:
    virtual ~Kernel() {}

    virtual Solid* MakeBox(double x, double y, double z) = 0;
    virtual Solid* MakeCylinder(double r, double h) = 0;
    virtual Solid* MakeSphere(double radius) = 0 ;
    virtual Solid* MakeCone(double radiusBottom, double radiusTop, double height) =0 ;


    virtual Solid* BooleanCut(Solid* a, Solid* b)=0;
    
    virtual Solid* Fillet(Solid* s, double radius) = 0;
    virtual Solid* Shell(Solid* s, double thickness) = 0;
    virtual Solid* BooleanUnion(Solid* a, Solid* b) =0;
    virtual Solid* Loft(const std::vector<Profile*>& profiles) = 0;
    virtual void ExportSTEP(Solid* s, const char* path) = 0;

    
    virtual StunadMesh* Tessellate(Solid* solid, float linearDeflection) = 0;






        // Profiles
    virtual Profile* MakeCircleProfile(double r) = 0;
    virtual Profile* MakeRectProfile(double x, double y) = 0;
    virtual Profile* MakePolygonProfile(const std::vector<std::pair<double, double>>& points)= 0;
    virtual Profile* MakeSplineProfile(const std::vector<std::pair<double, double>>& points) = 0;


    virtual Profile* TranslateProfile(Profile* p, double x, double y, double z) = 0;
    virtual Profile* RotateProfile(Profile* p, double angleDeg) = 0;
    virtual Profile* ScaleProfile(Profile* p, double s) = 0;

    
    






   
};
