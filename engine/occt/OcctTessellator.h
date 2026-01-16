#pragma once
#include "../core/Mesh.h"
#include "OcctSolid.h"


class OcctTessellator {
public:
    static StunadMesh* Tessellate(OcctSolid* solid, float deflection);
};
