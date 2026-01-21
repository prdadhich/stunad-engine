#pragma once
#include "../core/Kernel.h"
#include <string>
#include <TopoDS_Shape.hxx>
#include <cstdio>
#include <memory>




class OcctKernel : public Kernel
{
public:
    std::unique_ptr<Solid>  MakeBox(double x, double y, double z) override;

    
    std::unique_ptr<Solid>  MakeCylinder(double r, double h) override;
    std::unique_ptr<Solid>  MakeSphere(double radius) override;
    std::unique_ptr<Solid>  MakeCone(double radiusBottom, double radiusTop, double height) override;




   
    std::unique_ptr<Solid>  Shell(Solid* s, double t) override;
    void ExportSTEP(Solid* s, const char* path) override;
    std::unique_ptr<Solid>  BooleanCut(Solid* a, Solid* b) override;
    std::unique_ptr<Solid>  BooleanUnion(Solid* a, Solid* b) override;
    std::unique_ptr<Solid>  Loft(const std::vector<Profile*>& profiles,bool ruled,bool makeSolid) override;
    std::unique_ptr<Solid>  Sweep(Profile* profile, Profile* path, bool makeSolid) override;
    std::unique_ptr<Solid>  Revolve(Profile* p, double angleDeg, bool makeSolid) override;
    std::unique_ptr<Solid>  Thicken(Solid* s, double thickness) override;
    std::unique_ptr<Solid>  Extrude(Profile* p, double height, bool makeSolid) override;
    std::unique_ptr<Solid>  FilletEdges(Solid* s, double radius, FilletType mode) override;
    std::unique_ptr<Solid>  TranslateSolid(Solid* s, double dx, double dy, double dz) override;

    //postprocess
    StunadMesh* Tessellate(Solid* solid, float linearDeflection) override;


    //transform solid
    std::unique_ptr<Solid>  RotateSolid(Solid* s, double rx, double ry, double rz) override;
    std::unique_ptr<Solid>  ScaleSolid(Solid* s, double factor) override;
    std::unique_ptr<Solid>  BooleanIntersect(Solid* a, Solid* b) override;
    std::unique_ptr<Solid>  ScaleSolidNonUniform(Solid* s, double sx, double sy, double sz) override;



   //profile create
    std::unique_ptr<Profile> MakeCircleProfile(double r) override;
    std::unique_ptr<Profile> MakeRectProfile(double x, double y) override;
    std::unique_ptr<Profile> MakePolygonProfile(const std::vector<std::pair<double, double>>& points) override;
    std::unique_ptr<Profile> MakeSplineProfile(const std::vector<std::pair<double, double>>& points,bool isClosed) override;
    std::unique_ptr<Profile> RotateProfile3D(Profile* p, double angleDeg, double ax, double ay, double az) override;

    //profile transforms
    std::unique_ptr<Profile> RotateProfile(Profile* p, double angleDeg) override;
    std::unique_ptr<Profile> TranslateProfile(Profile* p, double x,double y , double z) override;
    std::unique_ptr<Profile> ScaleProfile(Profile* p, double s) override;

private:
    bool HealInternal(TopoDS_Shape& shape);


};
