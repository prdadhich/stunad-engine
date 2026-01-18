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
            case OpType::SplineProfile:
                if (op.type == OpType::PolygonProfile || op.type == OpType::SplineProfile) {
                    if (op.params.size() < 6 || op.params.size() % 2 != 0) {
                        error = "Polygon/Spline " + op.id + " requires even number of params (min 6)";
                        return false;
                    }
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
                for (const std::string& pId : op.refList) {
                    if (typeMap.find(pId) == typeMap.end() || typeMap[pId] != ValueType::Profile) {
                        error = "Loft " + op.id + " requires valid Profile IDs in refList";
                        return false;
                    }
                }
                typeMap[op.id] = ValueType::Solid;
                break;

            case OpType::Fillet:
            case OpType::Shell:
            case OpType::Translate:
            case OpType::Rotate:
                if (typeMap[op.refA] != ValueType::Solid) {
                    error = "Modifier/Transform " + op.id + " requires a Solid input";
                    return false;
                }
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