
#include "grammar/GrammarBuilder.h"
#include "grammar/GrammarExecutor.h"
#include "grammar/GrammarValidator.h"
#include "occt/OcctKernel.h"
#include <vector>
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif



int main() {
    GrammarBuilder g;

    // 1. Bottom: Large 10-pointed Star
    std::vector<std::pair<double, double>> starPoints;
    for (int i = 0; i < 10; ++i) {
        double angle = i * (36.0 * M_PI / 180.0);
        double r = (i % 2 == 0) ? 20.0 : 10.0;
        starPoints.push_back({ r * cos(angle), r * sin(angle) });
    }
    int bottom = g.PolygonProfile(starPoints);

    // 2. Middle: Hexagon, Moved up 30, and Rotated 30 degrees
    std::vector<std::pair<double, double>> hexPoints;
    for (int i = 0; i < 6; ++i) {
        double angle = i * (60.0 * M_PI / 180.0);
        hexPoints.push_back({ 12.0 * cos(angle), 12.0 * sin(angle) });
    }
    int mid = g.PolygonProfile(hexPoints);
    int midTrans = g.TranslateProfile(mid, 0, 0, 30);
    int midRot = g.RotateProfile(midTrans, 30.0);

    // 3. Top: Circle, Moved up 60
    int top = g.CircleProfile(8.0);
    int topTrans = g.TranslateProfile(top, 0, 0, 60);

    // 4. Create the Solid Loft
    int vaseSolid = g.Loft({bottom, midRot, topTrans});

    // 5. Hollow it out (Shelling)
    // Thickness of 2.0 units. 
    // Note: If this fails, try a smaller thickness like 0.5
   // int finalVase = g.Shell(vaseSolid, 0.5);

    // --- Execution ---
    GrammarExecutor exec;
    OcctKernel kernel;
    Solid* result = exec.execute(g.program(), kernel);

    if (result) {
        kernel.ExportSTEP(result, "hollow_vase.step");
        printf("COMPLETE SUCCESS: Hollowed complex vase created.\n");
    } else {
        printf("SHELL FAILURE: The loft was fine, but the hollowing failed.\n");
    }

    return 0;
}















/*

int main() {
    OcctKernel kernel;

    // 1. Generate a Star Shape
    std::vector<std::pair<double, double>> starPoints;
    int numPoints = 10;
    for (int i = 0; i < numPoints; ++i) {
        double angle = i * (2.0 * M_PI / numPoints);
        // Alternate between radius 15 and radius 7
        double r = (i % 2 == 0) ? 15.0 : 7.0;
        starPoints.push_back({ r * cos(angle), r * sin(angle) });
    }

    Profile* p1 = kernel.MakePolygonProfile(starPoints);
    Profile* p2 = kernel.TranslateProfile(p1, 0, 0, 40); // Move up 40
    // Optional: Scale the top to make it look like a star-pyramid
    Profile* p2_scaled = kernel.ScaleProfile(p2, 0.5); 

    std::vector<Profile*> profiles = { p1, p2_scaled };
    Solid* result = kernel.Loft(profiles);

    if (result) {
        kernel.ExportSTEP(result, "star_loft.step");
        printf("SUCCESS: Star loft created.\n");
    } else {
        printf("FAILURE: Star loft failed.\n");
    }

    return 0;
}




*/











/*
int main() {
    GrammarBuilder g;

 

   std::vector<std::pair<double, double>> squarePoints = {
        {-10.0, -10.0},
        { 10.0, -10.0},
        { 10.0,  10.0},
        {-10.0,  10.0}
    };

    int c2 = g.PolygonProfile(squarePoints);
    
    // 2. Translate it UP 50 units
    // (Moving it only in Z is the "safest" way to test a loft)
    int t2 = g.TranslateProfile(c2, 0, 0, 50); 

    // 3. Perform the Loft
    g.Loft({c2, t2});
   

    GrammarExecutor exec;
    OcctKernel kernel;
    GrammarValidator validator;
    std::string error;

    if (!validator.validate(g.program(), error)) {
        printf(error.c_str());
        return 1;
    }
    Solid* result = exec.execute(g.program(), kernel);
    if (!result) {
            printf("Error in result");
            return 1;
        }

    kernel.ExportSTEP(result, "lamp.step");
    // TODO: tessellate + export
    return 0;
}
*/

























/** 
#include "occt/OcctKernel.h"
#include "nodes/LampNode.h"
#include "extra/MeshPostProcess.h"
#include "extra/GltfExporter.h"
int main()
{
    OcctKernel kernel;

    LampNode lamp;
    lamp.params[0].value = 60;
    lamp.params[1].value = 300;
    lamp.params[2].value = 4;

    Solid* s = lamp.Evaluate(kernel);
    kernel.ExportSTEP(s, "lamp.step");

    StunadMesh* mesh = kernel.Tessellate(s, 0.005f);

    printf("Before Vertices: %zu\n", mesh->vertices.size());
    printf("Indices: %zu\n", mesh->indices.size());
    printf("Triangles: %zu\n", mesh->indices.size() / 3);

    //MeshPostProcess::DeduplicateVertices(*mesh);


    printf("After Vertices: %zu\n", mesh->vertices.size());
    printf("Indices: %zu\n", mesh->indices.size());
    printf("Triangles: %zu\n", mesh->indices.size() / 3);



    bool ok = ExportGLTF(*mesh, "model.gltf");



    
    if (!ok)
    {
        printf("ExportGLTF FAILED\n");
    }
    else
    {
        printf("ExportGLTF SUCCEEDED\n");
    }
*/
/*
    printf("Vertices: %zu\n", mesh->vertices.size());
    printf("Triangles: %zu\n", mesh->triangles.size());

    
*/

//}

