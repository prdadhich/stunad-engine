#include "GrammarValidator.h"
#include <cfloat>
#include <cmath>

bool GrammarValidator::validate(
    const GrammarProgram& program,
    std::string& error
) {
    int count = program.ops.size();

    for (int i = 0; i < count; ++i) {
        const Op& op = program.ops[i];

        // Check references
        if (op.a >= i || op.b >= i) {
            error = "Op references future or invalid index at op " + std::to_string(i);
            return false;
        }

        // Parameter sanity
        for (double p : op.params) {
            if (!std::isfinite(p)) {
                error = "Non-finite parameter at op " + std::to_string(i);
                return false;
            }
        }

        // Op-specific rules (example)
        if (op.type == OpType::Shell && op.params[0] <= 0) {
            error = "Shell thickness must be > 0";
            return false;
        }

        if (op.type == OpType::Fillet && op.params[0] <= 0) {
            error = "Fillet radius must be > 0";
            return false;
        }
        if (op.type == OpType::Cut || op.type == OpType::Union || op.type == OpType::Intersect)
        {
            if (op.a == op.b) 
            {
                error = "Boolean operation cannot use the same solid for both inputs";
                return false;
            }
        }

        if (op.type == OpType::PolygonProfile || op.type == OpType::SplineProfile) {
            if (op.params.size() < 6) { // Minimum 3 points (x1,y1, x2,y2, x3,y3)
                error = "Polygon/Spline requires at least 3 points (6 parameters)";
                return false;
            }
            if (op.params.size() % 2 != 0) {
                error = "Polygon/Spline must have an even number of parameters (x,y pairs)";
                return false;
            }
        }


        switch (op.type) {

            case OpType::CircleProfile:
            case OpType::RectProfile:    
            case OpType::PolygonProfile: 
            case OpType::SplineProfile:  
                types.push_back(ValueType::Profile);
                break;

            case OpType::ProfileRotate:
            case OpType::ProfileTranslate:
            case OpType::ProfileScale:
                if (types[op.a] != ValueType::Profile) {
                    error = "Profile transform requires Profile input";
                    return false;
                }
                types.push_back(ValueType::Profile);
                break;

            case OpType::Loft:
                for (double id : op.params) {
                    if (types[int(id)] != ValueType::Profile) {
                        error = "Loft requires Profile inputs";
                        return false;
                    }
                }
                types.push_back(ValueType::Solid);
                break;

            case OpType::Fillet:
            case OpType::Shell:
                if (types[op.a] != ValueType::Solid) {
                    error = "Modifier requires Solid input";
                    return false;
                }
                types.push_back(ValueType::Solid);
                break;

            case OpType::Box:
            case OpType::Cylinder:
                types.push_back(ValueType::Solid);
                break;

            case OpType::Sphere:
                    if (op.params[0] <= 0) {
                        error = "Sphere radius must be > 0";
                        return false;
                    }
                    types.push_back(ValueType::Solid);
                    break;

            case OpType::Cone:
                    if (op.params[2] <= 0) {
                        error = "Cone height must be > 0";
                        return false;
                    }
                    types.push_back(ValueType::Solid);
                    break;

            case OpType::Union:
            case OpType::Cut:
            case OpType::Intersect:
                if (types[op.a] != ValueType::Solid || types[op.b] != ValueType::Solid) {
                    error = "Boolean operation requires two Solid inputs";
                    return false;
                }
                types.push_back(ValueType::Solid);
                break;













        }









    }

    return true;
}
