#pragma once
#include <vector>
#include <string>
enum class OpType {


    // Profiles (2D intent)
    CircleProfile,
    RectProfile,
    PolygonProfile, 
    SplineProfile,

    // Profile transforms
    ProfileTranslate,
    ProfileRotate,
    ProfileScale,

    // Solids (3D intent)
    Loft,





    // Primitives
    Box,
    Cylinder,
    Sphere,
    Cone,

    // Booleans
    Union,
    Cut,
    Intersect,

    // Modifiers
    Shell,
    Fillet,

    // Transforms
    Translate,
    Rotate,
    Scale
};

struct Op {
    std::string id;
    OpType type;

    std::string refA;   // input A
    std::string refB;// input B (booleans)
    std::vector<std::string> refList;  //To handle multiple inputs like Loft 
    std::string selectionRule;
    std::vector<double> params;
};
