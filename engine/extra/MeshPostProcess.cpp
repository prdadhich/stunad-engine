#include "MeshPostProcess.h"
#include "../core/Mesh.h"
#include <cmath>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <cstring>  


constexpr float POS_SCALE   = 1e5f;
constexpr float NORMAL_SCALE = 1e3f;
constexpr float UV_SCALE    = 1e5f;


inline int Quantize(float v, float scale)
{
    return static_cast<int>(std::round(v * scale));
}

struct VertexKey
{
    int px, py, pz;
    int nx, ny, nz;
    int u, v;
};


struct VertexKeyHash
{
    std::size_t operator()(const VertexKey& k) const
    {
        std::size_t h = 0;
        auto hashCombine = [&](int v)
        {
            h ^= std::hash<int>()(v) + 0x9e3779b9 + (h << 6) + (h >> 2);
        };

        hashCombine(k.px); hashCombine(k.py); hashCombine(k.pz);
        hashCombine(k.nx); hashCombine(k.ny); hashCombine(k.nz);
        hashCombine(k.u);  hashCombine(k.v);

        return h;
    }
};

inline bool operator==(const VertexKey& a, const VertexKey& b)
{
    return memcmp(&a, &b, sizeof(VertexKey)) == 0;
}


void MeshPostProcess::DeduplicateVertices(StunadMesh& mesh)
{
    std::vector<Vertex> newVertices;
    newVertices.reserve(mesh.vertices.size());

    std::vector<uint32_t> newIndices;
    newIndices.reserve(mesh.indices.size());

    std::unordered_map<VertexKey, uint32_t, VertexKeyHash> vertexMap;

    for (uint32_t idx : mesh.indices)
    {
        const Vertex& v = mesh.vertices[idx];

        VertexKey key;
        key.px = Quantize(v.position.x, POS_SCALE);
        key.py = Quantize(v.position.y, POS_SCALE);
        key.pz = Quantize(v.position.z, POS_SCALE);

        key.nx = Quantize(v.normal.x, NORMAL_SCALE);
        key.ny = Quantize(v.normal.y, NORMAL_SCALE);
        key.nz = Quantize(v.normal.z, NORMAL_SCALE);

        key.u  = Quantize(v.uv.u, UV_SCALE);
        key.v  = Quantize(v.uv.v, UV_SCALE);

        auto it = vertexMap.find(key);
        if (it == vertexMap.end())
        {
            uint32_t newIndex = static_cast<uint32_t>(newVertices.size());
            vertexMap[key] = newIndex;
            newVertices.push_back(v);
            newIndices.push_back(newIndex);
        }
        else
        {
            newIndices.push_back(it->second);
        }
    }

    mesh.vertices = std::move(newVertices);
    mesh.indices  = std::move(newIndices);

}