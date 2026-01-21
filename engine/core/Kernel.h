#pragma once
#include "Solid.h"
#include "Mesh.h"
#include "Profile.h"
#include <string>
#include <cstdio>
#include <memory>



enum class FilletType {
    All = 0 ,
    Vertical = 1, // Edges roughly parallel to Z-axis
    Planar = 2    // Edges on XY-parallel planes
};


class Kernel
{
public:
    virtual ~Kernel() {}

    virtual std::unique_ptr<Solid>  MakeBox(double x, double y, double z) = 0;
    virtual std::unique_ptr<Solid>  MakeCylinder(double r, double h) = 0;
    virtual std::unique_ptr<Solid>  MakeSphere(double radius) = 0 ;
    virtual std::unique_ptr<Solid>  MakeCone(double radiusBottom, double radiusTop, double height) =0 ;


    virtual std::unique_ptr<Solid>  BooleanCut(Solid* a, Solid* b)=0;
    
    
    virtual std::unique_ptr<Solid>  Shell(Solid* s, double thickness) = 0;
    virtual std::unique_ptr<Solid>  BooleanUnion(Solid* a, Solid* b) =0;
    virtual std::unique_ptr<Solid>  Loft(const std::vector<Profile*>& profiles,bool ruled,bool makeSolid) = 0;
    virtual std::unique_ptr<Solid>  Sweep(Profile* profile, Profile* path, bool makeSolid) = 0 ; 
    virtual std::unique_ptr<Solid>  Revolve(Profile* p, double angleDeg, bool makeSolid) = 0;
    virtual std::unique_ptr<Solid>  Thicken(Solid* s, double thickness) = 0;
    virtual std::unique_ptr<Solid>  Extrude(Profile* p, double height, bool makeSolid) = 0;
    virtual void ExportSTEP(Solid* s, const char* path) = 0;
    virtual std::unique_ptr<Solid>  FilletEdges(Solid* s, double radius, FilletType mode) = 0;
    virtual std::unique_ptr<Solid>  TranslateSolid(Solid* s, double dx, double dy, double dz) = 0;
    virtual std::unique_ptr<Solid>  RotateSolid(Solid* s, double rx, double ry, double rz) = 0;
    virtual std::unique_ptr<Solid>  ScaleSolid(Solid* s, double factor)  = 0 ;
    virtual std::unique_ptr<Solid>  BooleanIntersect(Solid* a, Solid* b) = 0;
    virtual std::unique_ptr<Solid>  ScaleSolidNonUniform(Solid* s, double sx, double sy, double sz) = 0;

    
    virtual StunadMesh* Tessellate(Solid* solid, float linearDeflection) = 0;






        // Profiles
    virtual std::unique_ptr<Profile> MakeCircleProfile(double r) = 0;
    virtual std::unique_ptr<Profile> MakeRectProfile(double x, double y) = 0;
    virtual std::unique_ptr<Profile> MakePolygonProfile(const std::vector<std::pair<double, double>>& points)= 0;
    virtual std::unique_ptr<Profile> MakeSplineProfile(const std::vector<std::pair<double, double>>& points,bool isClosed) = 0;
    virtual std::unique_ptr<Profile> RotateProfile3D(Profile* p, double angleDeg, double ax, double ay, double az)  = 0 ;


    virtual std::unique_ptr<Profile> TranslateProfile(Profile* p, double x, double y, double z) = 0;
    virtual std::unique_ptr<Profile> RotateProfile(Profile* p, double angleDeg) = 0;
    virtual std::unique_ptr<Profile> ScaleProfile(Profile* p, double s) = 0;

    
    






   
};
