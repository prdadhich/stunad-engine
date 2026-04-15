#pragma once
#include "GrammarProgram.h"
#include "../core/Kernel.h"
#include "../core/Solid.h"
#include "../core/Profile.h"
#include <memory>
#include <unordered_map>
#include <string>
#include <vector>

struct Node {
    Op op;
    bool dirty = true;
    std::shared_ptr<Solid> solidResult;
    std::shared_ptr<Profile> profileResult;
    std::vector<std::string> dependents; // IDs of nodes that use this node as input
};

class GrammarExecutor {
public:
    // Upsert nodes from a program, comparing against existing nodes to determine dirtiness
    void updateGraph(const GrammarProgram& program);
    
    // Evaluates all dirty nodes in topological order and returns the solid from the target (or last) node
    std::shared_ptr<Solid> evaluate(Kernel& kernel, const std::string& targetNodeId = "");
    
private:
    std::unordered_map<std::string, std::shared_ptr<Node>> graph;
    std::vector<std::string> executionOrder; // The sequential order of ops defines our topology for now
    
    void markDirty(const std::string& nodeId);
    bool executeNode(std::shared_ptr<Node> node, Kernel& kernel);
};
