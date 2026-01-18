#pragma once
#include "GrammarProgram.h"
#include <vector>
#include <string>
class GrammarBuilder {
public:

    //profile
    std::string CircleProfile(double radius);
    std::string RectProfile(double x, double y);


    // profile transforms
    std::string TranslateProfile(std::string profile, double x, double y, double z);
    std::string RotateProfile(std::string profile, double angleDeg);
    std::string ScaleProfile(std::string profile, double s);
    std::string PolygonProfile(const std::vector<std::pair<double, double>>& points);
    std::string SplineProfile(const std::vector<std::pair<double, double>>& points);

    //solid from profile
   std::string Loft(const std::vector<std::string>& profileIds);


    //basic solids
    
    std::string Box(double x, double y, double z);
    std::string Cylinder(double r, double h);
    std::string Sphere(double r);
    std::string Cone(double rBottom, double rTop, double height);
    std::string Shell(std::string solid, double t);
    std::string Fillet(std::string solidId, std::string rule, double radius);

    std::string Union(std::string a, std::string b);
    std::string Cut(std::string baseId, std::string toolId);
    std::string Intersect(std::string a, std::string b);

    std::string Translate(std::string solid, double x, double y, double z);
    std::string Rotate(std::string solid, double rx, double ry, double rz);
    std::string Scale(std::string solid, double s);
    std::string Scale(std::string solid, double sx, double sy, double sz);
    const GrammarProgram& program() const;

private:
    GrammarProgram prog;

    std::string push(OpType type, 
                     std::string a = "", 
                     std::string b = "", 
                     std::vector<double> params = {}, 
                     std::vector<std::string> refList = {},
                     std::string rule = "");
};
