#include "GrammarExecutor.h"
#include <unordered_map>
#include <memory>
#include <cstdio>
#include <cmath>

bool areOpsEqual(const Op& a, const Op& b) {
    if (a.type != b.type) return false;
    if (a.refA != b.refA) return false;
    if (a.refB != b.refB) return false;
    if (a.refList != b.refList) return false;
    if (a.params.size() != b.params.size()) return false;
    for (size_t i = 0; i < a.params.size(); ++i) {
        if (std::abs(a.params[i] - b.params[i]) > 1e-9) return false;
    }
    return true;
}

void GrammarExecutor::updateGraph(const GrammarProgram& program) {
    executionOrder.clear();
    
    // First pass: upsert nodes and check for dirtiness
    for (const auto& op : program.ops) {
        executionOrder.push_back(op.id);
        
        auto it = graph.find(op.id);
        if (it == graph.end()) {
            auto node = std::make_shared<Node>();
            node->op = op;
            node->dirty = true;
            graph[op.id] = node;
        } else {
            // Check if op changed
            if (!areOpsEqual(it->second->op, op)) {
                it->second->op = op;
                markDirty(op.id); // This recursively marks dependents dirty
            }
        }
    }
    
    // Rebuild dependents list
    for (auto& pair : graph) {
        pair.second->dependents.clear();
    }
    for (const auto& op : program.ops) {
        if (!op.refA.empty() && graph.count(op.refA)) {
            graph[op.refA]->dependents.push_back(op.id);
        }
        if (!op.refB.empty() && graph.count(op.refB)) {
            graph[op.refB]->dependents.push_back(op.id);
        }
        for (const auto& ref : op.refList) {
            if (graph.count(ref)) {
                graph[ref]->dependents.push_back(op.id);
            }
        }
    }
}

void GrammarExecutor::markDirty(const std::string& nodeId) {
    if (!graph.count(nodeId)) return;
    auto node = graph[nodeId];
    if (node->dirty) return; // already dirty
    
    node->dirty = true;
    for (const auto& depId : node->dependents) {
        markDirty(depId);
    }
}

std::shared_ptr<Solid> GrammarExecutor::evaluate(Kernel& kernel, const std::string& targetNodeId) {
    if (executionOrder.empty()) {
        printf("[Executor] Error: Graph is empty.\n");
        return nullptr;
    }

    printf("[Executor] Evaluating Graph (%zu nodes)...\n", executionOrder.size());

    int rebuiltCount = 0;
    int cachedCount = 0;

    for (const std::string& nodeId : executionOrder) {
        auto node = graph[nodeId];
        if (node->dirty) {
            if (!executeNode(node, kernel)) {
                fprintf(stderr, "[Executor] Evaluation failed at node %s\n", nodeId.c_str());
                return nullptr;
            }
            node->dirty = false;
            rebuiltCount++;
        } else {
            cachedCount++;
        }
    }

    printf("[Executor] Completed. Rebuilt: %d, Cached: %d\n", rebuiltCount, cachedCount);

    std::string resolveTarget = targetNodeId.empty() ? executionOrder.back() : targetNodeId;
    if (graph.count(resolveTarget)) {
        return graph[resolveTarget]->solidResult;
    } else {
        fprintf(stderr, "[Executor] Error: Target operation %s not found or didn't produce a Solid.\n", resolveTarget.c_str());
        return nullptr;
    }
}

bool GrammarExecutor::executeNode(std::shared_ptr<Node> node, Kernel& kernel) {
    const Op& op = node->op;
    
    // Clear old result
    node->solidResult.reset();
    node->profileResult.reset();
    
    switch (op.type) {
        // -------- 2D PROFILES --------
        case OpType::CircleProfile:
            node->profileResult = std::unique_ptr<Profile>(kernel.MakeCircleProfile(op.params[0]));
            break;

        case OpType::RectProfile:
            node->profileResult = std::unique_ptr<Profile>(kernel.MakeRectProfile(op.params[0], op.params[1]));
            break;

        case OpType::PolygonProfile: {
            std::vector<std::pair<double, double>> points;
            for (size_t j = 0; j + 1 < op.params.size(); j += 2) {
                points.push_back({op.params[j], op.params[j + 1]});
            }
            node->profileResult = std::unique_ptr<Profile>(kernel.MakePolygonProfile(points));
            break;
        }

        case OpType::SplineProfile: {
            std::vector<std::pair<double, double>> points;
            for (size_t j = 1; j +1 < op.params.size() ; j += 2) {
                points.push_back({op.params[j], op.params[j+1]});
            }
            bool isClosed = (op.params[0] > 0.5); 
            node->profileResult = std::unique_ptr<Profile>(kernel.MakeSplineProfile(points, isClosed));
            break;
        }

        // -------- PROFILE TRANSFORMS --------
        case OpType::ProfileTranslate:
            if (graph.count(op.refA) && graph[op.refA]->profileResult) {
                node->profileResult = std::unique_ptr<Profile>(kernel.TranslateProfile(
                    graph[op.refA]->profileResult.get(), op.params[0], op.params[1], op.params[2]));
            }
            break;

        case OpType::ProfileRotate:
            if (graph.count(op.refA) && graph[op.refA]->profileResult) {
                node->profileResult = std::unique_ptr<Profile>(kernel.RotateProfile(
                    graph[op.refA]->profileResult.get(), op.params[0]));
            }
            break;

        case OpType::ProfileScale:
            if (graph.count(op.refA) && graph[op.refA]->profileResult) {
                node->profileResult = std::unique_ptr<Profile>(kernel.ScaleProfile(
                    graph[op.refA]->profileResult.get(), op.params[0]));
            }
            break;

        case OpType::ProfileRotate3D: {
            if (graph.count(op.refA) && graph[op.refA]->profileResult) {
                double angle = op.params[0];
                double ax = op.params[1];
                double ay = op.params[2];
                double az = op.params[3];

                node->profileResult = std::unique_ptr<Profile>(kernel.RotateProfile3D(
                    graph[op.refA]->profileResult.get(), 
                    angle, 
                    ax, ay, az
                ));
            }
            break;
        }

        // -------- 3D SOLIDS --------
        case OpType::Loft: {
            std::vector<Profile*> loftProfiles;
            for (const std::string& pId : op.refList) {
                if (graph.count(pId) && graph[pId]->profileResult) {
                    loftProfiles.push_back(graph[pId]->profileResult.get());
                } else {
                    fprintf(stderr, "[Executor] Warning: Loft op %s references missing profile %s\n", op.id.c_str(), pId.c_str());
                }
            }

            // Safe param extraction to prevent C++ vector out-of-bounds crash
            bool ruled = false; 
            bool makeSolid = false;
            if (op.params.size() > 0) ruled = (op.params[0] > 0.5);
            if (op.params.size() > 1) makeSolid = (op.params[1] > 0.5);

            if (loftProfiles.size() >= 2) {
                node->solidResult = std::unique_ptr<Solid>(kernel.Loft(loftProfiles, ruled, makeSolid));
            } else {
                fprintf(stderr, "[Executor] Loft Error: Op %s requires at least 2 profiles\n", op.id.c_str());
                return false;
            }
            break;
        }

        case OpType::Sweep: {
            bool makeSolid = false;
            if (!op.params.empty()) {
                makeSolid = (op.params[0] > 0.5);
            }
            if (graph.count(op.refA) && graph.count(op.refB) && graph[op.refA]->profileResult && graph[op.refB]->profileResult) {
                node->solidResult = std::unique_ptr<Solid>(kernel.Sweep(graph[op.refA]->profileResult.get(), graph[op.refB]->profileResult.get(), makeSolid));
            } else {
                fprintf(stderr, "[Executor] Sweep Error: Missing profile or path for op %s\n", op.id.c_str());
                return false;
            }
            break;
        }
        
        case OpType::Extrude: {
            if (graph.count(op.refA) && graph[op.refA]->profileResult) {
                double height = op.params[0];
                bool makeSolid = (op.params.size() > 1) ? (op.params[1] > 0.5) : true;
                node->solidResult = std::unique_ptr<Solid>(kernel.Extrude(graph[op.refA]->profileResult.get(), height, makeSolid));
            } else {
                fprintf(stderr, "[Executor] Extrude Error: Missing profile %s\n", op.refA.c_str());
                return false;
            }
            break;
        }
        
        case OpType::ProfileSetPlane:
            if (graph.count(op.refA) && graph[op.refA]->profileResult) {
                node->profileResult = std::unique_ptr<Profile>(kernel.SetProfilePlane(
                    graph[op.refA]->profileResult.get(),
                    op.params[0], op.params[1], op.params[2], // Origin
                    op.params[3], op.params[4], op.params[5]  // Normal
                ));
            }
            break;

        case OpType::AlignProfileToPath:
            if (graph.count(op.refA) && graph.count(op.refB) && graph[op.refA]->profileResult && graph[op.refB]->profileResult) {
                node->profileResult = std::unique_ptr<Profile>(kernel.AlignProfileToPath(
                    graph[op.refA]->profileResult.get(), 
                    graph[op.refB]->profileResult.get()
                ));
            }
            break;

        case OpType::Revolve: {
            if (graph.count(op.refA) && graph[op.refA]->profileResult) {
                double angle = op.params[0];
                bool makeSolid = (op.params.size() > 1) ? (op.params[1] > 0.5) : true;
                node->solidResult = std::unique_ptr<Solid>(kernel.Revolve(graph[op.refA]->profileResult.get(), angle, makeSolid));
            }
            break;
        }
        
        case OpType::Thicken: {
            if (graph.count(op.refA) && graph[op.refA]->solidResult) {
                double thickness = op.params[0];
                node->solidResult = std::unique_ptr<Solid>(kernel.Thicken(graph[op.refA]->solidResult.get(), thickness));
            }
            break;
        }

        case OpType::Box:
            node->solidResult = std::unique_ptr<Solid>(kernel.MakeBox(op.params[0], op.params[1], op.params[2]));
            break;

        case OpType::Cylinder:
            node->solidResult = std::unique_ptr<Solid>(kernel.MakeCylinder(op.params[0], op.params[1]));
            break;

        case OpType::Sphere:
            node->solidResult = std::unique_ptr<Solid>(kernel.MakeSphere(op.params[0]));
            break;

        case OpType::Cone:
            node->solidResult = std::unique_ptr<Solid>(kernel.MakeCone(op.params[0], op.params[1], op.params[2]));
            break;

        // -------- MODIFIERS & BOOLEANS --------
        case OpType::Shell:
            if (graph.count(op.refA) && graph[op.refA]->solidResult) {
                node->solidResult = std::unique_ptr<Solid>(kernel.Shell(graph[op.refA]->solidResult.get(), op.params[0]));
            }
            break;

        case OpType::Fillet: {
            if (graph.count(op.refA) && graph[op.refA]->solidResult) {
                double radius = op.params[0];
                FilletType mode = FilletType::All; 
                if (op.params.size() > 1) {
                    mode = static_cast<FilletType>( (int)op.params[1] );
                }
                node->solidResult = std::unique_ptr<Solid>(kernel.FilletEdges(graph[op.refA]->solidResult.get(), radius, mode));
            }
            break;
        }

        case OpType::Cut:
            if (graph.count(op.refA) && graph.count(op.refB) && graph[op.refA]->solidResult && graph[op.refB]->solidResult) {
                node->solidResult = std::unique_ptr<Solid>(kernel.BooleanCut(graph[op.refA]->solidResult.get(), graph[op.refB]->solidResult.get()));
            }
            break;

        case OpType::Union:
            if (graph.count(op.refA) && graph.count(op.refB) && graph[op.refA]->solidResult && graph[op.refB]->solidResult) {
                node->solidResult = std::unique_ptr<Solid>(kernel.BooleanUnion(graph[op.refA]->solidResult.get(), graph[op.refB]->solidResult.get()));
            }
            break;

        case OpType::Translate:
            if (graph.count(op.refA) && graph[op.refA]->solidResult) {
                node->solidResult = std::unique_ptr<Solid>(kernel.TranslateSolid(graph[op.refA]->solidResult.get(), op.params[0], op.params[1], op.params[2]));
            }
            break;

        case OpType::Rotate:
            if (graph.count(op.refA) && graph[op.refA]->solidResult) {
                node->solidResult = std::unique_ptr<Solid>(kernel.RotateSolid(graph[op.refA]->solidResult.get(), op.params[0], op.params[1], op.params[2]));
            }
            break;

        case OpType::Scale:
            if (graph.count(op.refA) && graph[op.refA]->solidResult) {
                if (op.params.size() >= 3) {
                    node->solidResult = std::unique_ptr<Solid>(kernel.ScaleSolidNonUniform(graph[op.refA]->solidResult.get(), op.params[0], op.params[1], op.params[2]));
                } else {
                    node->solidResult = std::unique_ptr<Solid>(kernel.ScaleSolid(graph[op.refA]->solidResult.get(), op.params[0]));
                }
            }
            break;
            
        case OpType::Intersect:
            if (graph.count(op.refA) && graph.count(op.refB) && graph[op.refA]->solidResult && graph[op.refB]->solidResult) {
                node->solidResult = std::unique_ptr<Solid>(kernel.BooleanIntersect(graph[op.refA]->solidResult.get(), graph[op.refB]->solidResult.get()));
            }
            break;

        case OpType::Mirror:
            if (graph.count(op.refA) && graph[op.refA]->solidResult) {
                node->solidResult = std::unique_ptr<Solid>(kernel.Mirror(graph[op.refA]->solidResult.get(), 
                    op.params[0], op.params[1], op.params[2],
                    op.params[3], op.params[4], op.params[5]
                ));
            }
            break;

        case OpType::PatternLinear:
            if (graph.count(op.refA) && graph[op.refA]->solidResult) {
                node->solidResult = std::unique_ptr<Solid>(kernel.PatternLinear(graph[op.refA]->solidResult.get(), 
                    (int)op.params[0], op.params[1],
                    op.params[2], op.params[3], op.params[4]
                ));
            }
            break;

        case OpType::PatternCircular:
            if (graph.count(op.refA) && graph[op.refA]->solidResult) {
                node->solidResult = std::unique_ptr<Solid>(kernel.PatternCircular(graph[op.refA]->solidResult.get(), 
                    (int)op.params[0], op.params[1],
                    op.params[2], op.params[3], op.params[4]
                ));
            }
            break;

        case OpType::PatternSpiral:
            if (graph.count(op.refA) && graph[op.refA]->solidResult) {
                node->solidResult = std::unique_ptr<Solid>(kernel.PatternSpiral(graph[op.refA]->solidResult.get(), 
                    (int)op.params[0], op.params[1], op.params[2],
                    op.params[3], op.params[4], op.params[5]
                ));
            }
            break;
    }

    if (!node->solidResult && !node->profileResult) {
        return false;
    }

    return true;
}