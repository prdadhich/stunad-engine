#include "GrammarBuilder.h"

std::string GrammarBuilder::push(OpType type, std::string a, std::string b, 
                                 std::vector<double> params, 
                                 std::vector<std::string> refList,
                                 std::string rule) { 
    Op op;
    op.type = type;
    op.refA = a;
    op.refB = b;
    op.params = params;
    op.refList = refList;
    op.selectionRule = rule; // <--- Store rule
    op.id = "op_" + std::to_string(prog.ops.size());
    
    prog.ops.push_back(op);
    return op.id;
}
// --- 3D PRIMITIVES ---

std::string GrammarBuilder::Box(double x, double y, double z) {
    return push(OpType::Box, "", "", {x, y, z});
}

std::string GrammarBuilder::Cylinder(double r, double h) {
    return push(OpType::Cylinder, "", "", {r, h});
}

std::string GrammarBuilder::Sphere(double r) {
    return push(OpType::Sphere, "", "", {r});
}

std::string GrammarBuilder::Cone(double rBottom, double rTop, double height) {
    return push(OpType::Cone, "", "", {rBottom, rTop, height});
}

// --- MODIFIERS & BOOLEANS ---

std::string GrammarBuilder::Shell(std::string solid, double t) {
    return push(OpType::Shell, solid, "", {t});
}

std::string GrammarBuilder::Fillet(std::string solidId, std::string rule, double radius) {
    // Correctly passing the rule into the selectionRule parameter of push
    return push(OpType::Fillet, solidId, "", {radius}, {}, rule);
}

std::string GrammarBuilder::Union(std::string a, std::string b) {
    return push(OpType::Union, a, b);
}

std::string GrammarBuilder::Cut(std::string baseId, std::string toolId) {
    return push(OpType::Cut, baseId, toolId);
}

std::string GrammarBuilder::Intersect(std::string a, std::string b) {
    return push(OpType::Intersect, a, b);
}

// --- 3D TRANSFORMS ---

std::string GrammarBuilder::Translate(std::string solid, double x, double y, double z) {
    return push(OpType::Translate, solid, "", {x, y, z});
}

std::string GrammarBuilder::Rotate(std::string solid, double rx, double ry, double rz) {
    return push(OpType::Rotate, solid, "", {rx, ry, rz});
}

std::string GrammarBuilder::Scale(std::string solid, double s) {
    return push(OpType::Scale, solid, "", {s});
}

std::string GrammarBuilder::Scale(std::string solid, double sx, double sy, double sz) {
    return push(OpType::Scale, solid, "", {sx, sy, sz}); // Pass all 3 factors
}
// --- 2D PROFILES ---

std::string GrammarBuilder::CircleProfile(double r) {
    return push(OpType::CircleProfile, "", "", {r});
}

std::string GrammarBuilder::RectProfile(double x, double y) {
    return push(OpType::RectProfile, "", "", {x, y});
}

std::string GrammarBuilder::PolygonProfile(const std::vector<std::pair<double, double>>& points) {
    std::vector<double> flattenedParams;
    flattenedParams.reserve(points.size() * 2);
    for (const auto& p : points) {
        flattenedParams.push_back(p.first);
        flattenedParams.push_back(p.second);
    }
    return push(OpType::PolygonProfile, "", "", flattenedParams);
}

std::string GrammarBuilder::SplineProfile(const std::vector<std::pair<double, double>>& points) {
    std::vector<double> flattened;
    flattened.reserve(points.size() * 2);
    for (const auto& p : points) {
        flattened.push_back(p.first);
        flattened.push_back(p.second);
    }
    return push(OpType::SplineProfile, "", "", flattened);
}

// --- PROFILE TRANSFORMS ---

std::string GrammarBuilder::TranslateProfile(std::string p, double x, double y, double z) {
    return push(OpType::ProfileTranslate, p, "", {x, y, z});
}

std::string GrammarBuilder::RotateProfile(std::string p, double angleDeg) {
    return push(OpType::ProfileRotate, p, "", {angleDeg});
}

std::string GrammarBuilder::ScaleProfile(std::string p, double s) {
    return push(OpType::ProfileScale, p, "", {s});
}

// --- COMPLEX OPERATIONS ---

std::string GrammarBuilder::Loft(const std::vector<std::string>& profileIds) {
    // Uses the refList to store the series of profile IDs for the loft operation
    return push(OpType::Loft, "", "", {}, profileIds);
}

const GrammarProgram& GrammarBuilder::program() const {
    return prog;
}