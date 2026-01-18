#include "GltfExporter.h"

#include <tiny_gltf.h>
#include <vector>
#include <cstring>

#include "../core/Mesh.h"

bool ExportGLTF(const StunadMesh& mesh, const std::string& path)
{
    tinygltf::Model model;
    tinygltf::TinyGLTF gltf;

    // ------------------------------------------------------------
    // 1. Create buffer (single buffer, interleaved by sections)
    // ------------------------------------------------------------

    std::vector<unsigned char> buffer;

    auto AppendData = [&](const void* data, size_t size) -> size_t
    {
        size_t offset = buffer.size();
        buffer.resize(offset + size);
        std::memcpy(buffer.data() + offset, data, size);
        return offset;
    };

    // Positions
    std::vector<Vec3> positions;
    positions.reserve(mesh.vertices.size());
    for (const auto& v : mesh.vertices){
          Vec3 gltfPos = 
          {
                 v.position.x,   // X stays X
                 v.position.z,   // OCCT Z → glTF Y
                 -v.position.y    // OCCT Y → -Z
          };

    positions.push_back(gltfPos);

    }
      

    size_t posOffset = AppendData(
        positions.data(),
        positions.size() * sizeof(Vec3)
    );

    // Normals
    std::vector<Vec3> normals;
    normals.reserve(mesh.vertices.size());
    for (const auto& v : mesh.vertices)
    {
          Vec3 gltfNormal = {
            v.normal.x,
            v.normal.z,
            -v.normal.y
        };   
        normals.push_back(gltfNormal);
    }
        

    size_t normOffset = AppendData(
        normals.data(),
        normals.size() * sizeof(Vec3)
    );

    // UVs
    std::vector<Vec2> uvs;
    uvs.reserve(mesh.vertices.size());
    for (const auto& v : mesh.vertices)
        uvs.push_back(v.uv);

    size_t uvOffset = AppendData(
        uvs.data(),
        uvs.size() * sizeof(Vec2)
    );

    // Indices
    size_t idxOffset = AppendData(
        mesh.indices.data(),
        mesh.indices.size() * sizeof(uint32_t)
    );

    // ------------------------------------------------------------
    // 2. Add buffer to model
    // ------------------------------------------------------------

    tinygltf::Buffer gltfBuffer;
    gltfBuffer.data = buffer;
    model.buffers.push_back(gltfBuffer);

    // ------------------------------------------------------------
    // 3. BufferViews
    // ------------------------------------------------------------

    auto AddBufferView = [&](size_t offset, size_t length, int target)
    {
        tinygltf::BufferView view;
        view.buffer = 0;
        view.byteOffset = static_cast<int>(offset);
        view.byteLength = static_cast<int>(length);
        view.target = target;
        model.bufferViews.push_back(view);
        return static_cast<int>(model.bufferViews.size() - 1);
    };

    int posView  = AddBufferView(posOffset,  mesh.vertices.size() * sizeof(Vec3),  TINYGLTF_TARGET_ARRAY_BUFFER);
    int normView = AddBufferView(normOffset, normals.size()       * sizeof(Vec3),  TINYGLTF_TARGET_ARRAY_BUFFER);
    int uvView   = AddBufferView(uvOffset,   uvs.size()           * sizeof(Vec2),  TINYGLTF_TARGET_ARRAY_BUFFER);
    int idxView  = AddBufferView(idxOffset,  mesh.indices.size()  * sizeof(uint32_t), TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER);

    // ------------------------------------------------------------
    // 4. Accessors
    // ------------------------------------------------------------

  auto AddAccessor = [&](int view, int componentType, int count, int type, 
                       const std::vector<double>& minVal = {}, 
                       const std::vector<double>& maxVal = {})
{
    tinygltf::Accessor acc;
    acc.bufferView = view;
    acc.componentType = componentType;
    acc.count = count;
    acc.type = type;
    acc.minValues = minVal; // Added this
    acc.maxValues = maxVal; // Added this
    model.accessors.push_back(acc);
    return static_cast<int>(model.accessors.size() - 1);
};

    std::vector<double> minVals = { 1e30, 1e30, 1e30 };
    std::vector<double> maxVals = { -1e30, -1e30, -1e30 };

    for (const auto& p : positions) {
        minVals[0] = std::min(minVals[0], (double)p.x);
        minVals[1] = std::min(minVals[1], (double)p.y);
        minVals[2] = std::min(minVals[2], (double)p.z);

        maxVals[0] = std::max(maxVals[0], (double)p.x);
        maxVals[1] = std::max(maxVals[1], (double)p.y);
        maxVals[2] = std::max(maxVals[2], (double)p.z);
    }

// Use the new bounds in the accessor call
    int posAcc = AddAccessor(posView, TINYGLTF_COMPONENT_TYPE_FLOAT, mesh.vertices.size(), TINYGLTF_TYPE_VEC3, minVals, maxVals);
    int normAcc = AddAccessor(normView, TINYGLTF_COMPONENT_TYPE_FLOAT,   normals.size(), TINYGLTF_TYPE_VEC3);
    int uvAcc   = AddAccessor(uvView,   TINYGLTF_COMPONENT_TYPE_FLOAT,   uvs.size(),  TINYGLTF_TYPE_VEC2);
    int idxAcc  = AddAccessor(idxView,  TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT, mesh.indices.size(), TINYGLTF_TYPE_SCALAR);

    // ------------------------------------------------------------
    // 5. Mesh
    // ------------------------------------------------------------

    tinygltf::Primitive prim;
    prim.attributes["POSITION"]   = posAcc;
    prim.attributes["NORMAL"]     = normAcc;
    prim.attributes["TEXCOORD_0"] = uvAcc;
    prim.indices = idxAcc;
    prim.mode = TINYGLTF_MODE_TRIANGLES;

    tinygltf::Mesh gltfMesh;
    gltfMesh.primitives.push_back(prim);
    model.meshes.push_back(gltfMesh);

    // ------------------------------------------------------------
    // 6. Node + Scene
    // ------------------------------------------------------------

    tinygltf::Node node;
    node.mesh = 0;
    model.nodes.push_back(node);

    tinygltf::Scene scene;
    scene.nodes.push_back(0);
    model.scenes.push_back(scene);
    model.defaultScene = 0;

    tinygltf::Material material;
    material.name = "DefaultMaterial";
    material.pbrMetallicRoughness.baseColorFactor = { 0.8, 0.8, 0.8, 1.0 }; // Light Gray
    material.pbrMetallicRoughness.metallicFactor = 0.0;
    material.pbrMetallicRoughness.roughnessFactor = 0.5;
    model.materials.push_back(material);

// 2. Link the Primitive to the Material
// Ensure your primitive (the one with the attributes) points to material index 0
    model.meshes[0].primitives[0].material = 0;
    // ------------------------------------------------------------
    // 7. Write file
    // ------------------------------------------------------------

    std::string err, warn;
    bool ok = gltf.WriteGltfSceneToFile(
        &model,
        path,
        true,   // embed buffers
        true,   // embed images
        true,   // pretty print
        false   // write binary (.gltf, not .glb)
    );

    if (!warn.empty())
        printf("glTF warning: %s\n", warn.c_str());
    if (!err.empty())
        printf("glTF error: %s\n", err.c_str());

    return ok;
}
