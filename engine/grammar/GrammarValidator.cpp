#include "GrammarValidator.h"
#include <unordered_map>
#include <cmath>

bool GrammarValidator::validate(
    const GrammarProgram& program,
    std::string& error
) {
    // Tracks the type of each ID (Solid or Profile) as we iterate
    std::unordered_map<std::string, ValueType> typeMap;

    for (const auto& op : program.ops) {
        // 1. Basic ID Validation
        if (op.id.empty()) {
            error = "Operation ID cannot be empty";
            return false;
        }
        if (typeMap.count(op.id)) {
            error = "Duplicate ID detected: " + op.id;
            return false;
        }

        // 2. Reference Validation (Check if refA and refB actually exist)
        if (!op.refA.empty() && typeMap.find(op.refA) == typeMap.end()) {
            error = "Op " + op.id + " references undefined ID: " + op.refA;
            return false;
        }
        if (!op.refB.empty() && typeMap.find(op.refB) == typeMap.end()) {
            error = "Op " + op.id + " references undefined ID: " + op.refB;
            return false;
        }

        // 3. Parameter Sanity (Check for NaN or Infinity)
        for (double p : op.params) {
            if (!std::isfinite(p)) {
                error = "Non-finite parameter in op: " + op.id;
                return false;
            }
        }

        // 4. Type Checking & Logic Rules
        switch (op.type) {
            // --- 2D Profiles ---
            case OpType::CircleProfile:
            case OpType::RectProfile:
            case OpType::PolygonProfile:
                if (op.type == OpType::PolygonProfile) {
                    if (op.params.size() < 6 || op.params.size() % 2 != 0) {
                        error = "Polygon " + op.id + " requires even number of params (min 6)";
                        return false;
                    }
                }
                typeMap[op.id] = ValueType::Profile;
                break;


            case OpType::SplineProfile:
                // If you include the isClosed flag, the count should be ODD (Points * 2 + 1)
                if (op.params.size() < 7 || op.params.size() % 2 == 0) {
                    error = "Spline " + op.id + " requires points and a closure flag (odd number of params)";
                    return false;
                }
                typeMap[op.id] = ValueType::Profile;
                break;

            case OpType::ProfileRotate:
            case OpType::ProfileTranslate:
            case OpType::ProfileScale:
                if (typeMap[op.refA] != ValueType::Profile) {
                    error = "Transform " + op.id + " requires a Profile input";
                    return false;
                }
                typeMap[op.id] = ValueType::Profile;
                break;

            // --- 3D Solids ---
            case OpType::Box:
            case OpType::Cylinder:
            case OpType::Sphere:
            case OpType::Cone:
                typeMap[op.id] = ValueType::Solid;
                break;

            case OpType::Loft:
                if (op.refList.empty()) {
                    error = "Loft " + op.id + " requires at least one profile in refList";
                    return false;
                }
                if (op.params.size() < 2) {
                    error = op.id + " requires 2 parameters (ruled, makeSolid).";
                    return false;
                }
                for (const auto& ref : op.refList) {
                        if (typeMap[ref] != ValueType::Profile) {
                            error = "Loft requires Profiles";
                            return false;
                        }
                    }
                typeMap[op.id] = ValueType::Solid; // Even if open, we treat it as solid intent
                break;


            case OpType::Extrude:
            case OpType::Revolve:
                // Params: [0]=height/angle, [1]=makeSolid (optional)
                if (op.params.empty()) {
                    error = op.id + " requires at least 1 parameter.";
                    return false;
                }
                typeMap[op.id] = ValueType::Solid; 
                break;


            case OpType::ProfileRotate3D:
                if (op.refA.empty()) {
                    error = "RotateProfile3D " + op.id + " requires an input profile (refA).";
                    return false;
                }
                // We expect [angle, axisX, axisY, axisZ]
                if (op.params.size() < 4) {
                    error = "RotateProfile3D " + op.id + " requires 4 parameters: [angle, ax, ay, az].";
                    return false;
                }
                typeMap[op.id] = ValueType::Profile;
                break;


            case OpType::Thicken:
            // refA = the shell to thicken, params[0] = thickness
                if (typeMap[op.refA] != ValueType::Solid) { // Even surfaces are stored in solidMap
                    error = op.id + " requires a shape to thicken.";
                    return false;
                }
                if (op.params.empty()) {
                    error = op.id + " requires a thickness value.";
                    return false;
                }
                typeMap[op.id] = ValueType::Solid;
                break;

            
            case OpType::Shell:
            case OpType::Translate:
            case OpType::Rotate:
                if (typeMap[op.refA] != ValueType::Solid) {
                    error = "Modifier/Transform " + op.id + " requires a Solid input";
                    return false;
                }
                typeMap[op.id] = ValueType::Solid;
                break;
            case OpType::Sweep:
                // Sweep logic: refA = Profile (Shape), refB = Profile (Path)
                if (op.refA.empty() || op.refB.empty()) {
                    error = "Sweep " + op.id + " requires both a profile (refA) and a path (refB).";
                    return false;
                }
                if (typeMap[op.refA] != ValueType::Profile || typeMap[op.refB] != ValueType::Profile) {
                    error = "Sweep " + op.id + " requires two Profile inputs.";
                    return false;
                }
                // Params: [0] = makeSolid flag (1.0 for solid, 0.0 for surface)
                if (op.params.empty()) {
                    error = "Sweep " + op.id + " requires a makeSolid parameter.";
                    return false;
                }
                typeMap[op.id] = ValueType::Solid; 
                break;



            case OpType::Fillet:
                if (typeMap[op.refA] != ValueType::Solid) {
                    error = "Fillet requires a solid input";
                    return false;
                }
                if (op.params.size() < 1) {
                    error = "Fillet requires a radius";
                    return false;
                }
                // Result remains a Solid
                typeMap[op.id] = ValueType::Solid;
                break;

            case OpType::Scale:
                if (typeMap[op.refA] != ValueType::Solid) {
                    error = "Scale " + op.id + " requires a Solid input";
                    return false;
                }
                if (op.params.size() != 1 && op.params.size() != 3) {
                    error = "Scale " + op.id + " requires either 1 or 3 parameters";
                    return false;
                }
                typeMap[op.id] = ValueType::Solid;
                break;

            case OpType::Union:
            case OpType::Cut:
            case OpType::Intersect:
                if (typeMap[op.refA] != ValueType::Solid || typeMap[op.refB] != ValueType::Solid) {
                    error = "Boolean " + op.id + " requires two Solid inputs";
                    return false;
                }
                if (op.refA == op.refB) {
                    error = "Boolean " + op.id + " cannot use the same solid twice";
                    return false;
                }
                typeMap[op.id] = ValueType::Solid;
                break;

            default:
                error = "Unknown OpType in validator";
                return false;
        }
    }

    return true;
}