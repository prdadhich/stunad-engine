#include "GrammarBuilder.h"

int GrammarBuilder::push(OpType type, int a, int b, std::vector<double> params) {
    prog.ops.push_back({type, a, b, params});
    return int(prog.ops.size() - 1);
}

int GrammarBuilder::Box(double x, double y, double z) {
    return push(OpType::Box, -1, -1, {x, y, z});
}

int GrammarBuilder::Cylinder(double r, double h) {
    return push(OpType::Cylinder, -1, -1, {r, h});
}

int GrammarBuilder::Sphere(double r) {
    return push(OpType::Sphere, -1, -1, {r});
}
int GrammarBuilder::Cone(double rBottom, double rTop, double height) {
   
    return push(OpType::Cone, -1, -1, {rBottom, rTop, height});
}

int GrammarBuilder::Shell(int solid, double t) {
    return push(OpType::Shell, solid, -1, {t});
}

int GrammarBuilder::Fillet(int solid, double r) {
    return push(OpType::Fillet, solid, -1, {r});
}

int GrammarBuilder::Union(int a, int b) {
    return push(OpType::Union, a, b);
}

int GrammarBuilder::Cut(int a, int b) {
    return push(OpType::Cut, a, b);
}

int GrammarBuilder::Intersect(int a, int b) {
    return push(OpType::Intersect, a, b);
}

int GrammarBuilder::Translate(int solid, double x, double y, double z) {
    return push(OpType::Translate, solid, -1, {x, y, z});
}

int GrammarBuilder::Rotate(int solid, double rx, double ry, double rz) {
    return push(OpType::Rotate, solid, -1, {rx, ry, rz});
}

int GrammarBuilder::Scale(int solid, double s) {
    return push(OpType::Scale, solid, -1, {s});
}

int GrammarBuilder::CircleProfile(double r) {
    return push(OpType::CircleProfile, -1, -1, {r});
}

int GrammarBuilder::RectProfile(double x, double y) {
    return push(OpType::RectProfile, -1, -1, {x, y});
}


// Helper to flatten points into params
int GrammarBuilder::PolygonProfile(const std::vector<std::pair<double, double>>& points) {
    Op op;
    op.type = OpType::PolygonProfile;
    for (const auto& p : points) {
        op.params.push_back(p.first);
        op.params.push_back(p.second);
    }
    int id = static_cast<int>(prog.ops.size());
    prog.ops.push_back(op);
    return id;
}

// Splines use the same flattening logic
int GrammarBuilder::SplineProfile(const std::vector<std::pair<double, double>>& points) {
    Op op = {};
    op.type = OpType::SplineProfile;
    for (const auto& p : points) {
        op.params.push_back(p.first);
        op.params.push_back(p.second);
    }
    int id = static_cast<int>(prog.ops.size());
    prog.ops.push_back(op);
    return id;
}



int GrammarBuilder::TranslateProfile(int p, double x, double y, double z) {
    return push(OpType::ProfileTranslate, p, -1, {x, y, z});
}

int GrammarBuilder::RotateProfile(int p, double angleDeg) {
    return push(OpType::ProfileRotate, p, -1, {angleDeg});
}

int GrammarBuilder::ScaleProfile(int p, double s) {
    return push(OpType::ProfileScale, p, -1, {s});
}

int GrammarBuilder::Loft(const std::vector<int>& profiles) {
    // store profile indices in params as doubles (safe for v0)
    std::vector<double> ids;
    for (int i : profiles) ids.push_back(double(i));
    return push(OpType::Loft, -1, -1, ids);
}




const GrammarProgram& GrammarBuilder::program() const {
    return prog;
}
