#pragma once
#include <vector>

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
    OpType type;

    int a = -1;   // input A
    int b = -1;   // input B (booleans)

    std::vector<double> params;
};
