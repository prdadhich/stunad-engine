#pragma once

#include <string>

// Forward declaration (no heavy includes here)
class StunadMesh;

/**
 * Export a mesh to glTF 2.0 (.gltf + .bin)
 *
 * @param mesh   Mesh to export (read-only)
 * @param path   Output path WITHOUT extension
 *               Example: "models/cube"
 *
 * This will create:
 *   cube.gltf
 *   cube.bin
 */
bool ExportGLTF(const StunadMesh& mesh, const std::string& path);
