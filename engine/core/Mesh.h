#pragma once
#include <vector>

struct Vec3 {
    float x, y, z;
};

struct Triangle {
    uint32_t a, b, c;
};


struct Vec2 {
    float u, v;
};

struct Vertex {
    Vec3 position;
    Vec3 normal;
    Vec2 uv;
};

class StunadMesh {
public:
    std::vector<Vertex> vertices;
    std::vector<Triangle> triangles;
    std::vector<uint32_t> indices;
    std::vector<uint32_t> triangleMaterialIds;
    void Clear() {
        vertices.clear();
        triangles.clear();
        indices.clear();
        triangleMaterialIds.clear();       
    }
    //std::vector<uint32_t> indices;

    // optional later
    //std::vector<uint32_t> faceId; 
};
