#pragma once
#include "GrammarProgram.h"
#include <vector>
#include <string>
#include "../core/Kernel.h"

class GrammarBuilder {
public:

    //profile
    std::string CircleProfile(double radius);
    std::string RectProfile(double x, double y);


    // profile transforms
    std::string TranslateProfile(std::string& profile, double x, double y, double z);
    std::string RotateProfile(std::string& profile, double angleDeg);
    std::string ScaleProfile(std::string& profile, double s);
    std::string PolygonProfile(const std::vector<std::pair<double, double>>& points);
    std::string SplineProfile(const std::vector<std::pair<double, double>>& points, bool isClosed);
    std::string RotateProfile3D(const std::string& profileId, double angleDeg, double ax, double ay, double az);
    std::string SetProfilePlane(const std::string& profileId, const std::vector<double>& origin, const std::vector<double>& normal);
    std::string AlignProfileToPath(const std::string& profileId, const std::string& pathId);

    //solid from profile
   std::string Loft(const std::vector<std::string>&  profilesId, bool ruled = false, bool makeSolid = false);
   std::string Extrude(const std::string& profileId, double height, bool makeSolid = false);
   std::string Revolve(const std::string& profileId, double angleDeg, bool makeSolid = false);
   std::string Thicken(const std::string& shapeId, double thickness);
   std::string Sweep(const std::string& profileId, const std::string& pathId, bool makeSolid= false);



    std::string Mirror(const std::string& solidId, const std::vector<double>& origin, const std::vector<double>& normal);
    std::string PatternLinear(const std::string& solidId, int count, double spacing, const std::vector<double>& direction);
    std::string PatternCircular(const std::string& solidId, int count, double totalAngle, const std::vector<double>& axis);
    std::string PatternSpiral(const std::string& solidId, int count, double totalAngle, double totalRise, const std::vector<double>& axis);



    //basic solids
    
    std::string Box(double x, double y, double z);
    std::string Cylinder(double r, double h);
    std::string Sphere(double r);
    std::string Cone(double rBottom, double rTop, double height);
    std::string Shell(std::string& solidId, double t);
    std::string Fillet(const std::string& solidId, double radius, FilletType mode = FilletType::All);

    std::string Union(std::string& a, std::string& b);
    std::string Cut(std::string& baseId, std::string& toolId);
    std::string Intersect(std::string& solidIdA, std::string& solidIdB);

    std::string Translate(std::string& solidId, double x, double y, double z);
    std::string Rotate(std::string& solidId, double rx, double ry, double rz);
    std::string Scale(std::string& solidId, double s);
    std::string Scale(std::string& solidId, double sx, double sy, double sz);
    const GrammarProgram& program() const;

private:
    GrammarProgram prog;

    std::string push(OpType type, 
                     std::string a = "", 
                     std::string b = "", 
                     std::vector<double> params = {}, 
                     std::vector<std::string> refList = {});
};
