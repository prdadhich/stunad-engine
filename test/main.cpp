#include "occt/OcctKernel.h"
#include "nodes/LampNode.h"
#include "extra/MeshPostProcess.h"
int main()
{
    OcctKernel kernel;

    LampNode lamp;
    lamp.params[0].value = 60;
    lamp.params[1].value = 300;
    lamp.params[2].value = 4;

    Solid* s = lamp.Evaluate(kernel);
    kernel.ExportSTEP(s, "lamp.step");

    StunadMesh* mesh = kernel.Tessellate(s, 0.5f);

    printf("Before Vertices: %zu\n", mesh->vertices.size());
    printf("Indices: %zu\n", mesh->indices.size());
    printf("Triangles: %zu\n", mesh->indices.size() / 3);

    MeshPostProcess::DeduplicateVertices(*mesh);


    printf("After Vertices: %zu\n", mesh->vertices.size());
    printf("Indices: %zu\n", mesh->indices.size());
    printf("Triangles: %zu\n", mesh->indices.size() / 3);

/*
    printf("Vertices: %zu\n", mesh->vertices.size());
    printf("Triangles: %zu\n", mesh->triangles.size());

    
*/

}
