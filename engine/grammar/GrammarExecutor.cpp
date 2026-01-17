#include "GrammarExecutor.h"
#include "../core/Profile.h"
#include <iostream>
#include <vector>

Solid* GrammarExecutor::execute(
    const GrammarProgram& program,
    Kernel& kernel
) {
    if (program.ops.empty()) return nullptr;

    // Pre-allocate vectors to match operation count exactly.
    // This ensures indices (IDs) from GrammarBuilder align perfectly.
    std::vector<Solid*> solids(program.ops.size(), nullptr);
    std::vector<Profile*> profiles(program.ops.size(), nullptr);

    for (size_t i = 0; i < program.ops.size(); ++i) {
        const Op& op = program.ops[i];
        Solid* outSolid = nullptr;
        Profile* outProfile = nullptr;

        switch (op.type) {
            // -------- PROFILES --------
            case OpType::CircleProfile:
                outProfile = kernel.MakeCircleProfile(op.params[0]);
                break;

            case OpType::RectProfile:
                outProfile = kernel.MakeRectProfile(op.params[0], op.params[1]);
                break;


            case OpType::PolygonProfile: {
                std::vector<std::pair<double, double>> points;
                for (size_t j = 0; j + 1 < op.params.size(); j += 2) {
                    points.push_back({op.params[j], op.params[j+1]});
                }
                // Use outProfile instead of profiles[i]
                outProfile = kernel.MakePolygonProfile(points); 
                break;
            }

            case OpType::SplineProfile: {
                std::vector<std::pair<double, double>> points;
                for (size_t j = 0; j + 1 < op.params.size(); j += 2) {
                    points.push_back({op.params[j], op.params[j+1]});
                }
                // FIX: Use 'i' (the loop index)
                outProfile = kernel.MakeSplineProfile(points);
                break;
            }



            case OpType::ProfileRotate:
                if (profiles[op.a]) {
                    outProfile = kernel.RotateProfile(profiles[op.a], op.params[0]);
                }
                break;

            case OpType::ProfileTranslate:
                if (profiles[op.a]) {
                    outProfile = kernel.TranslateProfile(
                        profiles[op.a], 
                        op.params[0], op.params[1], op.params[2]
                    );
                }
                break;

            case OpType::ProfileScale:
                if (profiles[op.a]) {
                    outProfile = kernel.ScaleProfile(profiles[op.a], op.params[0]);
                }
                break;

            // -------- SOLIDS --------
            case OpType::Loft: {
                std::vector<Profile*> loftProfiles;
                for (double id : op.params) {
                    int pIdx = static_cast<int>(id);
                    // Critical check: ensure the profile exists at that index
                    if (pIdx >= 0 && pIdx < profiles.size() && profiles[pIdx]) {
                        loftProfiles.push_back(profiles[pIdx]);
                    }
                }
                outSolid = kernel.Loft(loftProfiles);
                break;
            }

            case OpType::Box:
                outSolid = kernel.MakeBox(op.params[0], op.params[1], op.params[2]);
                break;

            case OpType::Cylinder:
                outSolid = kernel.MakeCylinder(op.params[0], op.params[1]);
                break;

            case OpType::Sphere:
                outSolid = kernel.MakeSphere(op.params[0]);
                break;

            case OpType::Cone:
                // params[0] = r1, params[1] = r2, params[2] = height
                outSolid = kernel.MakeCone(op.params[0], op.params[1], op.params[2]);
                break;

            case OpType::Shell:
                if (solids[op.a]) {
                    outSolid = kernel.Shell(solids[op.a], op.params[0]);
                }
                break;

            case OpType::Fillet:
                if (solids[op.a]) {
                    outSolid = kernel.Fillet(solids[op.a], op.params[0]);
                }
                break;

            case OpType::Cut:
                if (solids[op.a] && solids[op.b]) {
                    outSolid = kernel.BooleanCut(solids[op.a], solids[op.b]);
                }
                break;

           case OpType::Union:
                if (solids[op.a] && solids[op.b]) {
                    outSolid = kernel.BooleanUnion(solids[op.a], solids[op.b]);
                }
                break;

            // -------- TRANSFORMS (SOLIDS) --------
            case OpType::Translate:
                // You may need kernel.TranslateSolid(solids[op.a], x, y, z)
                break;

            default:
                break;
        }

        // Store the results in the indexed slots
        solids[i] = outSolid;
        profiles[i] = outProfile;
    }

    // Return the last valid solid produced by the program
    for (int i = (int)solids.size() - 1; i >= 0; --i) {
        if (solids[i] != nullptr) {
            return solids[i];
        }
    }

    return nullptr;
}