#pragma once
#include "GrammarProgram.h"

class GrammarBuilder {
public:

    //profile
    int CircleProfile(double radius);
    int RectProfile(double x, double y);


    // profile transforms
    int TranslateProfile(int profile, double x, double y, double z);
    int RotateProfile(int profile, double angleDeg);
    int ScaleProfile(int profile, double s);
    int PolygonProfile(const std::vector<std::pair<double, double>>& points);
    int SplineProfile(const std::vector<std::pair<double, double>>& points);

    //solid from profile
    int Loft(const std::vector<int>& profiles);


    //basic solids
    
    int Box(double x, double y, double z);
    int Cylinder(double r, double h);
    int Sphere(double r);
    int Cone(double rBottom, double rTop, double height);
    int Shell(int solid, double t);
    int Fillet(int solid, double r);

    int Union(int a, int b);
    int Cut(int a, int b);
    int Intersect(int a, int b);

    int Translate(int solid, double x, double y, double z);
    int Rotate(int solid, double rx, double ry, double rz);
    int Scale(int solid, double s);

    const GrammarProgram& program() const;

private:
    GrammarProgram prog;

    int push(OpType type,
             int a = -1,
             int b = -1,
             std::vector<double> params = {});
};
