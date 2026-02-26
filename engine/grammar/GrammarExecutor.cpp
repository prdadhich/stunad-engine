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
                // Extract points (leave first param)
                for (size_t j = 1; j +1 < op.params.size() ; j += 2) {
                    points.push_back({op.params[j], op.params[j+1]});
                }
                // Last parameter is our isClosed flag
                bool isClosed = (op.params[0] > 0.5); 
                profileMap[op.id] = kernel.MakeSplineProfile(points, isClosed);
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


            case OpType::ProfileRotate3D: {
                if (profileMap.count(op.refA)) {
                    double angle = op.params[0];
                    double ax = op.params[1];
                    double ay = op.params[2];
                    double az = op.params[3];

                    profileMap[op.id] = kernel.RotateProfile3D(
                        profileMap[op.refA].get(), 
                        angle, 
                        ax, ay, az
                    );
                    
                    printf("[DEBUG] RotateProfile3D %s: Angle %.1f around axis(%.1f, %.1f, %.1f)\n", 
                            op.id.c_str(), angle, ax, ay, az);
                    }
                    break;
                }

                    // -------- 3D SOLIDS --------
            case OpType::Loft: {
                std::vector<Profile*> loftProfiles;
                
                // 1. Resolve string IDs from the refList
                for (const std::string& pId : op.refList) {
                    if (profileMap.count(pId)) {
                        loftProfiles.push_back(profileMap[pId].get());
                    } else {
                        printf("[DEBUG] Loft: Could not find Profile ID: %s in profileMap\n", pId.c_str());
                        fprintf(stderr, "[Executor] Warning: Loft op %s references missing profile %s\n", 
                                op.id.c_str(), pId.c_str());
                    }
                }

                // 2. Extract the 'ruled' flag (passed as a double in params)
                bool ruled = false; 
                bool makeSolid = false;
                if (!op.params.empty()) {
                    ruled = (op.params[0] > 0.5); // 1.0 = ruled (straight), 0.0 = smooth
                    makeSolid = (op.params[1] > 0.5); // 1 makeSolid 0 surface
                }

                // 3. Final safety check: OCCT Loft requires at least 2 profiles
                if (loftProfiles.size() >= 2) {
                    // Ownership transfer: Kernel returns unique_ptr, we move it into map
                    solidMap[op.id] = kernel.Loft(loftProfiles, ruled,makeSolid);
                }
                else {
                    fprintf(stderr, "[Executor] Loft Error: Op %s requires at least 2 profiles (found %zu)\n", 
                            op.id.c_str(), loftProfiles.size());
                    return nullptr; // Fail early if geometry cannot be built
                }
                break;
            }

            case OpType::Sweep: {
                bool makeSolid = false;
                makeSolid = (op.params[0] > 0.5);
                if (!op.params.empty()) {
                    makeSolid = (op.params[0] > 0.5); // 1 makeSolid 0 surface
                }
                if (profileMap.count(op.refA) && profileMap.count(op.refB)) {
                    // refA = Profile (cross-section), refB = Path (spine)
                    solidMap[op.id] = kernel.Sweep(profileMap[op.refA].get(), profileMap[op.refB].get(),makeSolid);
                }
                else {
                    fprintf(stderr, "[Executor] Sweep Error: Missing profile or path for op %s\n", op.id.c_str());
                    return nullptr;
                }
                break;
            }
            case OpType::Extrude: {
                if (profileMap.count(op.refA)) {
                    double height = op.params[0];
                    bool makeSolid = (op.params.size() > 1) ? (op.params[1] > 0.5) : true;
                    solidMap[op.id] = kernel.Extrude(profileMap[op.refA].get(), height, makeSolid);
                }
                break;
            }
            
            case OpType::ProfileSetPlane:
                if (profileMap.count(op.refA)) {
                    profileMap[op.id] = kernel.SetProfilePlane(
                        profileMap[op.refA].get(),
                        op.params[0], op.params[1], op.params[2], // Origin
                        op.params[3], op.params[4], op.params[5]  // Normal
                    );
                }
                break;


            case OpType::AlignProfileToPath:
                if (profileMap.count(op.refA) && profileMap.count(op.refB)) {
                    profileMap[op.id] = kernel.AlignProfileToPath(
                        profileMap[op.refA].get(), 
                        profileMap[op.refB].get()
                    );
                }
                break;

            case OpType::Revolve: {
                if (profileMap.count(op.refA)) {
                    double angle = op.params[0];
                    bool makeSolid = (op.params.size() > 1) ? (op.params[1] > 0.5) : true;
                    solidMap[op.id] = kernel.Revolve(profileMap[op.refA].get(), angle, makeSolid);
                }
                break;
            }
            case OpType::Thicken: {
                if (solidMap.count(op.refA)) {
                    double thickness = op.params[0];
                    solidMap[op.id] = kernel.Thicken(solidMap[op.refA].get(), thickness);
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

            case OpType::Fillet: {
                if (solidMap.count(op.refA)) {
                    double radius = op.params[0];
                    // Read mode from params[1], default to 0 (All)
                    FilletType mode = FilletType::All; 
                    if (op.params.size() > 1) {
                        mode = static_cast<FilletType>( (int)op.params[1] );
                    }
                    
                    solidMap[op.id] = kernel.FilletEdges(solidMap[op.refA].get(), radius, mode);
                }
                break;
            }

            case OpType::Cut:
                if (solidMap.count(op.refA) && solidMap.count(op.refB)) {
                    solidMap[op.id] = kernel.BooleanCut(solidMap[op.refA].get(), solidMap[op.refB].get());
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


                // Inside the switch(op.type) in GrammarExecutor::execute

            case OpType::Mirror:
                if (solidMap.count(op.refA)) {
                    solidMap[op.id] = kernel.Mirror(solidMap[op.refA].get(), 
                        op.params[0], op.params[1], op.params[2], // Plane Origin
                        op.params[3], op.params[4], op.params[5]  // Plane Normal
                    );
                }
                break;

            case OpType::PatternLinear:
                if (solidMap.count(op.refA)) {
                    solidMap[op.id] = kernel.PatternLinear(solidMap[op.refA].get(), 
                        (int)op.params[0], op.params[1], // count, spacing
                        op.params[2], op.params[3], op.params[4] // direction
                    );
                }
                break;

            case OpType::PatternCircular:
                if (solidMap.count(op.refA)) {
                    solidMap[op.id] = kernel.PatternCircular(solidMap[op.refA].get(), 
                        (int)op.params[0], op.params[1], // count, total angle
                        op.params[2], op.params[3], op.params[4] // rotation axis
                    );
                }
                break;


                            

            case OpType::PatternSpiral:
                if (solidMap.count(op.refA)) {
                    solidMap[op.id] = kernel.PatternSpiral(
                        solidMap[op.refA].get(), 
                        (int)op.params[0], // count
                        op.params[1],      // totalAngle
                        op.params[2],      // totalRise
                        op.params[3], op.params[4], op.params[5] // rotation/rise axis
                    );
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
    std::string lastId = program.ops.back().id;

    if (solidMap.count(lastId)) {
        return std::move(solidMap[lastId]);
    } else {
        fprintf(stderr, "[Executor] Error: Last operation %s did not produce a Solid.\n", lastId.c_str());
        return nullptr;
    }
}