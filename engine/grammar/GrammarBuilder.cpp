#include "GrammarBuilder.h"

std::string GrammarBuilder::push(OpType type, std::string a, std::string b, 
                                 std::vector<double> params, 
                                 std::vector<std::string> refList) { 
    Op op;
    op.type = type;
    op.refA = a;
    op.refB = b;
    op.params = params;
    op.refList = refList;
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

std::string GrammarBuilder::Shell(std::string& solidId, double t) {
    return push(OpType::Shell, solidId, "", {t});
}

std::string GrammarBuilder::Fillet(const std::string& solidId, double radius, FilletType mode) {
    // Usage: builder.Fillet("my_box", 2.0, 1); // 1 = Vertical edges only
    return push(OpType::Fillet, solidId, "", { radius, static_cast<double>(mode) });
}


std::string GrammarBuilder::Union(std::string& a, std::string& b) {
    return push(OpType::Union, a, b);
}

std::string GrammarBuilder::Cut(std::string& baseId, std::string& toolId) {
    return push(OpType::Cut, baseId, toolId);
}

std::string GrammarBuilder::Intersect(std::string& a, std::string& b) {
    return push(OpType::Intersect, a, b);
}

// --- 3D TRANSFORMS ---

std::string GrammarBuilder::Translate(std::string& solid, double x, double y, double z) {
    return push(OpType::Translate, solid, "", {x, y, z});
}

std::string GrammarBuilder::Rotate(std::string& solid, double rx, double ry, double rz) {
    return push(OpType::Rotate, solid, "", {rx, ry, rz});
}

std::string GrammarBuilder::Scale(std::string& solid, double s) {
    return push(OpType::Scale, solid, "", {s});
}

std::string GrammarBuilder::Scale(std::string& solid, double sx, double sy, double sz) {
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

std::string GrammarBuilder::SplineProfile(const std::vector<std::pair<double, double>>& points, bool isClosed) {
    std::vector<double> flattened;
    flattened.reserve(points.size() * 2 + 1);
    flattened.push_back(isClosed ? 1.0 : 0.0); 

    for (const auto& p : points) {
        flattened.push_back(p.first);
        flattened.push_back(p.second);
    }
    
    
    return push(OpType::SplineProfile, "", "", flattened);
}

// --- PROFILE TRANSFORMS ---

std::string GrammarBuilder::TranslateProfile(std::string& p, double x, double y, double z) {
    return push(OpType::ProfileTranslate, p, "", {x, y, z});
}

std::string GrammarBuilder::RotateProfile(std::string& p, double angleDeg) {
    return push(OpType::ProfileRotate, p, "", {angleDeg});
}

std::string GrammarBuilder::ScaleProfile(std::string& p, double s) {
    return push(OpType::ProfileScale, p, "", {s});
}

// --- COMPLEX OPERATIONS ---

std::string GrammarBuilder::Loft(const std::vector<std::string>& profiles, bool ruled, bool makeSolid) {
    // params[0] = ruled flag
    // params[1] = makeSolid flag
    std::vector<double> params = { 
        ruled ? 1.0 : 0.0, 
        makeSolid ? 1.0 : 0.0 
    };

    // We leave refA and refB empty ("") because we are using the 'profiles' 
    // vector (which becomes the op.refList) to store the IDs.
    return push(OpType::Loft, "", "", params, profiles);
}

std::string GrammarBuilder::Extrude(const std::string& profileId, double height, bool makeSolid) {
    // params[0] = height, params[1] = makeSolid (1.0 or 0.0)
    std::vector<double> params = { height, makeSolid ? 1.0 : 0.0 };
    
    // refA is the profile to extrude
    return push(OpType::Extrude, profileId, "", params);
}

std::string GrammarBuilder::Revolve(const std::string& profileId, double angleDeg, bool makeSolid) {
    // params[0] = angle, params[1] = makeSolid
    std::vector<double> params = { angleDeg, makeSolid ? 1.0 : 0.0 };
    
    return push(OpType::Revolve, profileId, "", params);
}

std::string GrammarBuilder::Thicken(const std::string& shapeId, double thickness) {
    // refA is the shape, params[0] is the offset distance
    return push(OpType::Thicken, shapeId, "", { thickness });
}

std::string GrammarBuilder::Sweep(const std::string& profileId, const std::string& pathId, bool makeSolid) {
    // refA = cross section, refB = path
    // params[0] = makeSolid flag
    return push(OpType::Sweep, profileId, pathId, { makeSolid ? 1.0 : 0.0 });
}

std::string GrammarBuilder::RotateProfile3D(const std::string& profileId, double angleDeg, double ax, double ay, double az) {
    // params[0] = angle, params[1-3] = axis vector (x, y, z)
    return push(OpType::ProfileRotate3D, profileId, "", { angleDeg, ax, ay, az });
}


std::string GrammarBuilder::SetProfilePlane(const std::string& profileId, const std::vector<double>& origin, const std::vector<double>& normal) 
{
    // params: [ox, oy, oz, dx, dy, dz]
    std::vector<double> params = { origin[0], origin[1], origin[2], normal[0], normal[1], normal[2] };
    return push(OpType::ProfileSetPlane, profileId, "", params);
}

std::string GrammarBuilder::AlignProfileToPath(const std::string& profileId, const std::string& pathId) {
    return push(OpType::AlignProfileToPath, profileId, pathId, {});
}


std::string GrammarBuilder::Mirror(const std::string& solidId, 
                                   const std::vector<double>& origin, 
                                   const std::vector<double>& normal) {
    // params: [ox, oy, oz, nx, ny, nz]
    std::vector<double> params = { origin[0], origin[1], origin[2], normal[0], normal[1], normal[2] };
    return push(OpType::Mirror, solidId, "", params);
}

std::string GrammarBuilder::PatternLinear(const std::string& solidId, int count, double spacing, 
                                          const std::vector<double>& direction) {
    // params: [count, spacing, dx, dy, dz]
    std::vector<double> params = { (double)count, spacing, direction[0], direction[1], direction[2] };
    return push(OpType::PatternLinear, solidId, "", params);
}

std::string GrammarBuilder::PatternCircular(const std::string& solidId, int count, double totalAngle, 
                                            const std::vector<double>& axis) {
    // params: [count, totalAngle, ax, ay, az]
    std::vector<double> params = { (double)count, totalAngle, axis[0], axis[1], axis[2] };
    return push(OpType::PatternCircular, solidId, "", params);
}


std::string GrammarBuilder::PatternSpiral(const std::string& solidId, int count, double totalAngle, double totalRise, const std::vector<double>& axis) {
    // params: [count, totalAngle, totalRise, ax, ay, az]
    std::vector<double> params = { (double)count, totalAngle, totalRise, axis[0], axis[1], axis[2] };
    return push(OpType::PatternSpiral, solidId, "", params);
}



const GrammarProgram& GrammarBuilder::program() const {
    return prog;
}