#include "GrammarExecutor.h"
#include <unordered_map>
#include <memory>
#include <cstdio>


std::unique_ptr<Solid> GrammarExecutor::execute(
    const GrammarProgram& program,
    Kernel& kernel
) {
    if (program.ops.empty()) {
        printf("[Executor] Error: Program is empty.\n");
        return nullptr;
        }

    // Use smart pointers to manage lifetimes automatically and prevent leaks.
    // Maps store results by their unique string ID.
    std::unordered_map<std::string, std::unique_ptr<Solid>> solidMap;
    std::unordered_map<std::string, std::unique_ptr<Profile>> profileMap;
    printf("[Executor] Executing %zu operations...\n", program.ops.size());

    for (const auto& op : program.ops) {
        switch (op.type) {
            // -------- 2D PROFILES --------
            case OpType::CircleProfile:
                profileMap[op.id] = std::unique_ptr<Profile>(kernel.MakeCircleProfile(op.params[0]));
                break;

            case OpType::RectProfile:
                profileMap[op.id] = std::unique_ptr<Profile>(kernel.MakeRectProfile(op.params[0], op.params[1]));
                break;

            case OpType::PolygonProfile: {
                std::vector<std::pair<double, double>> points;
                for (size_t j = 0; j + 1 < op.params.size(); j += 2) {
                    points.push_back({op.params[j], op.params[j + 1]});
                }
                profileMap[op.id] = std::unique_ptr<Profile>(kernel.MakePolygonProfile(points));
                break;
            }

            case OpType::SplineProfile: {
                std::vector<std::pair<double, double>> points;
                for (size_t j = 0; j + 1 < op.params.size(); j += 2) {
                    points.push_back({op.params[j], op.params[j + 1]});
                }
                profileMap[op.id] = std::unique_ptr<Profile>(kernel.MakeSplineProfile(points));
                break;
            }

            // -------- PROFILE TRANSFORMS --------
            case OpType::ProfileTranslate:
                if (profileMap.count(op.refA)) {
                    profileMap[op.id] = std::unique_ptr<Profile>(kernel.TranslateProfile(
                        profileMap[op.refA].get(), op.params[0], op.params[1], op.params[2]));
                }
                break;

            case OpType::ProfileRotate:
                if (profileMap.count(op.refA)) {
                    profileMap[op.id] = std::unique_ptr<Profile>(kernel.RotateProfile(
                        profileMap[op.refA].get(), op.params[0]));
                }
                break;

            case OpType::ProfileScale:
                if (profileMap.count(op.refA)) {
                    profileMap[op.id] = std::unique_ptr<Profile>(kernel.ScaleProfile(
                        profileMap[op.refA].get(), op.params[0]));
                }
                break;

            // -------- 3D SOLIDS --------
            case OpType::Loft: {
                std::vector<Profile*> loftProfiles;
                // Resolve string IDs from the refList
                for (const std::string& pId : op.refList) {
                    if (profileMap.count(pId)) {
                        loftProfiles.push_back(profileMap[pId].get());
                    }
                }
                if (!loftProfiles.empty()) {
                    solidMap[op.id] = std::unique_ptr<Solid>(kernel.Loft(loftProfiles));
                }
                else {
                    fprintf(stderr, "[Executor] Loft Error: No valid profiles found for op %s\n", op.id.c_str());
                }
                break;
            }

            case OpType::Box:
                solidMap[op.id] = std::unique_ptr<Solid>(kernel.MakeBox(op.params[0], op.params[1], op.params[2]));
                break;

            case OpType::Cylinder:
                solidMap[op.id] = std::unique_ptr<Solid>(kernel.MakeCylinder(op.params[0], op.params[1]));
                break;

            case OpType::Sphere:
                solidMap[op.id] = std::unique_ptr<Solid>(kernel.MakeSphere(op.params[0]));
                break;

            case OpType::Cone:
                solidMap[op.id] = std::unique_ptr<Solid>(kernel.MakeCone(op.params[0], op.params[1], op.params[2]));
                break;

            // -------- MODIFIERS & BOOLEANS --------
            case OpType::Shell:
                if (solidMap.count(op.refA)) {
                    solidMap[op.id] = std::unique_ptr<Solid>(kernel.Shell(solidMap[op.refA].get(), op.params[0]));
                }
                break;

            case OpType::Fillet:
                if (solidMap.count(op.refA)) {
                    // Logic for adaptive selection rule
                    if (!op.selectionRule.empty()) {
                        // This calls your new adaptive kernel method
                        solidMap[op.id] = std::unique_ptr<Solid>(kernel.FilletByRule(
                            solidMap[op.refA].get(), op.params[0], op.selectionRule));
                    } else {
                        // Standard fillet
                        solidMap[op.id] = std::unique_ptr<Solid>(kernel.Fillet(
                            solidMap[op.refA].get(), op.params[0]));
                    }
                }
                break;

            case OpType::Cut:
                if (solidMap.count(op.refA) && solidMap.count(op.refB)) {
                    solidMap[op.id] = std::unique_ptr<Solid>(kernel.BooleanCut(
                        solidMap[op.refA].get(), solidMap[op.refB].get()));
                }
                break;

            case OpType::Union:
                if (solidMap.count(op.refA) && solidMap.count(op.refB)) {
                    solidMap[op.id] = std::unique_ptr<Solid>(kernel.BooleanUnion(
                        solidMap[op.refA].get(), solidMap[op.refB].get()));
                }
                break;

            case OpType::Translate:
                if (solidMap.count(op.refA)) {
                    solidMap[op.id] = std::unique_ptr<Solid>(kernel.TranslateSolid(
                        solidMap[op.refA].get(), op.params[0], op.params[1], op.params[2]));
                }
                break;

            case OpType::Rotate:
                if (solidMap.count(op.refA)) {
                    solidMap[op.id] = std::unique_ptr<Solid>(kernel.RotateSolid(
                        solidMap[op.refA].get(), op.params[0], op.params[1], op.params[2]));
                }
                break;

            case OpType::Scale:
                if (solidMap.count(op.refA)) {
                    if (op.params.size() >= 3) {
                        // Non-Uniform Scaling (X, Y, Z)
                        solidMap[op.id] = std::unique_ptr<Solid>(kernel.ScaleSolidNonUniform(
                            solidMap[op.refA].get(), op.params[0], op.params[1], op.params[2]));
                    } else {
                        // Uniform Scaling
                        solidMap[op.id] = std::unique_ptr<Solid>(kernel.ScaleSolid(
                            solidMap[op.refA].get(), op.params[0]));
                    }
                }
                break;
                
                        case OpType::Intersect:
                if (solidMap.count(op.refA) && solidMap.count(op.refB)) {
                    solidMap[op.id] = std::unique_ptr<Solid>(kernel.BooleanIntersect(
                        solidMap[op.refA].get(), solidMap[op.refB].get()));
                }
                break;

        
        }

        if (solidMap.count(op.id)) {
            if (solidMap[op.id] == nullptr) {
                fprintf(stderr, "FAILED (Kernel returned null solid)\n");
                return nullptr; // Exit early so we don't crash later
            }
            printf("SUCCESS (Solid)\n");
        } 
        else if (profileMap.count(op.id)) {
            if (profileMap[op.id] == nullptr) {
                fprintf(stderr, "FAILED (Kernel returned null profile)\n");
                return nullptr;
            }
            printf("SUCCESS (Profile)\n");
        } 
        else {
            fprintf(stderr, "FAILED (Operation not handled or missing inputs)\n");
            return nullptr;
        }
    }


    // Return the final result. 
    // Usually, the caller expects a raw pointer they now own.
    if (solidMap.empty()) return nullptr;
    
    // Get the last operation's solid. .get() returns the raw pointer.
    // Note: In a production environment, you might return the unique_ptr 
    // or ensure the Kernel manages the lifetime of this specific returned object.
    return std::move(solidMap[program.ops.back().id]);
}