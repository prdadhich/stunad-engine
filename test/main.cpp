
#include "grammar/GrammarBuilder.h"
#include "grammar/GrammarExecutor.h"
#include "grammar/GrammarValidator.h"
#include "occt/OcctKernel.h"
#include "extra/MeshPostProcess.h"
#include "extra/GltfExporter.h"
#include "nlohmann/json.hpp"
#include <vector>
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif



using json = nlohmann::json;

OpType mapType(const std::string& name) {
    // Profiles
    if (name == "CircleProfile")   return OpType::CircleProfile;
    if (name == "RectProfile")     return OpType::RectProfile;
    if (name == "PolygonProfile")  return OpType::PolygonProfile;
    if (name == "SplineProfile")   return OpType::SplineProfile;

    // Profile transforms
    if (name == "TranslateProfile") return OpType::ProfileTranslate;
    if (name == "RotateProfile")    return OpType::ProfileRotate;
    if (name == "ScaleProfile")     return OpType::ProfileScale;

    // Solids/Primitives
    if (name == "Loft")            return OpType::Loft;
    if (name == "Box")             return OpType::Box;
    if (name == "Cylinder")        return OpType::Cylinder;
    if (name == "Sphere")          return OpType::Sphere;
    if (name == "Cone")            return OpType::Cone;
    if (name == "Sweep")           return OpType::Sweep;
    if (name == "Extrude")         return OpType::Extrude;
    if (name == "Revolve")         return OpType::Revolve;
    if (name == "Thicken")         return OpType::Thicken;
    if (name == "RotateProfile3D")         return OpType::ProfileRotate3D;
   





    // Booleans
    if (name == "Union")           return OpType::Union;
    if (name == "Cut")             return OpType::Cut;
    if (name == "Intersect")       return OpType::Intersect;

    // Modifiers/Transforms
    if (name == "Shell")           return OpType::Shell;
    if (name == "Fillet")          return OpType::Fillet;
    if (name == "Translate")       return OpType::Translate;
    if (name == "Rotate")          return OpType::Rotate;
    if (name == "Scale")           return OpType::Scale;

    return OpType::Box; // Default fallback
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: stunad_test.exe \"<JSON_STRING>\"" << std::endl;
        return 1;
    }

    try {
        OcctKernel kernel;
        GrammarExecutor executor;
        GrammarProgram program;

        // 1. Parse the JSON sent by Python/Web
        auto data = nlohmann::json::parse(argv[1]);

        // 2. Map JSON ops to your GrammarProgram
        // This replaces the hardcoded g.CircleProfile, etc.
    for (const auto& item : data["ops"]) {
        Op op;
        op.id = item["id"];
        op.type = mapType(item["type"]);
        
        // References
        op.refA = item.value("refA", "");
        op.refB = item.value("refB", "");
        
        // List for Lofts (e.g., ["base", "mid", "top"])
        if (item.contains("refList")) {
            op.refList = item["refList"].get<std::vector<std::string>>();
        }

        // Parameters (e.g., [20.0, 10.0])
        if (item.contains("params")) {
            op.params = item["params"].get<std::vector<double>>();
        }

        
        
        program.ops.push_back(op);
    }

    //validate
    GrammarValidator validator;
    std::string error;

    if (!validator.validate(program, error)) {
        printf(error.c_str());
        return 1;
    }



    // 4. Execute 
    std::shared_ptr<Solid> finalResult = executor.execute(program, kernel);

    if (finalResult) {
        // Use a generic name or one passed from JSON
        kernel.ExportSTEP(finalResult.get(), "models/output.step");
        std::cout << "SUCCESS" << std::endl;

        StunadMesh* mesh = kernel.Tessellate(finalResult.get(),.2);

        // 2. Clean up the mesh (important for web performance)
        MeshPostProcess::DeduplicateVertices(*mesh);

        // 3. Export to glTF (Web-ready)
        // We name it 'output' which will create 'output.gltf'
        if (ExportGLTF(*mesh, "models/output.gltf")) {
            std::cout << "SUCCESS: output.gltf generated" << std::endl;
        }
    }

} catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
}
return 0;


}









































/*
int main() {
    try {
        OcctKernel kernel;
        GrammarBuilder g;
        GrammarExecutor executor;

        std::cout << "[Main] Generating Complex Vase Recipe..." << std::endl;

        // 1. Create the Base (A Circle at Z=0)
        std::string baseCircle = g.CircleProfile(20.0);

        // 2. Create the Mid-Section (A Star/Polygon at Z=40)
        std::vector<std::pair<double, double>> starPoints;
        int numPoints = 10;
        for (int i = 0; i < numPoints; ++i) {
            double angle = i * (2.0 * M_PI / numPoints);
            double r = (i % 2 == 0) ? 35.0 : 15.0; 
            starPoints.push_back({ r * cos(angle), r * sin(angle) });
        }
        
        std::string midStar = g.PolygonProfile(starPoints);
        std::string midStarRaised = g.TranslateProfile(midStar, 0, 0, 40.0);

        // 3. Create the Top (A smaller Circle at Z=80)
        std::string topCircle = g.CircleProfile(15.0);
        std::string topCircleRaised = g.TranslateProfile(topCircle, 0, 0, 80.0);

        // 4. Loft them together
        // Order matters: Bottom -> Middle -> Top
        std::string vaseBody = g.Loft({baseCircle, midStarRaised, topCircleRaised});

        // 5. Shell it (Hollow the top)
        // Thickness of 2.0mm. Note: ensure your Kernel::Shell handles 
        // choosing the 'top' face to remove, or just offsets the whole solid.
        //std::string hollowVase = g.Shell(vaseBody, 2.0);

        // 6. Execute
        std::cout << "[Main] Executing Graph..." << std::endl;
        std::shared_ptr<Solid> finalResult = executor.execute(g.program(), kernel);

        if (finalResult) {
            std::cout << "[Main] Exporting Vase..." << std::endl;
            kernel.ExportSTEP(finalResult.get(), "complex_vase.step");
            std::cout << "[Main] Done! Check complex_vase.step" << std::endl;
        } else {
            std::cerr << "[Main] Execution failed to produce a solid." << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "[Main] Exception: " << e.what() << std::endl;
    }

    return 0;
}



*/










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

