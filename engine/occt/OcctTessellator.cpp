#include "OcctTessellator.h"
#include "OcctSolid.h"

#include <BRepMesh_IncrementalMesh.hxx>
#include <BRep_Tool.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <Poly_Triangulation.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <Poly_Array1OfTriangle.hxx>



inline Vec3 operator+(const Vec3& a, const Vec3& b) {
    return { a.x + b.x, a.y + b.y, a.z + b.z };
}

inline Vec3 operator-(const Vec3& a, const Vec3& b) {
    return { a.x - b.x, a.y - b.y, a.z - b.z };
}

inline Vec3 operator*(const Vec3& v, float s) {
    return { v.x * s, v.y * s, v.z * s };
}

inline Vec3 Cross(const Vec3& a, const Vec3& b) {
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

inline float Length(const Vec3& v) {
    return std::sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

inline Vec3 Normalize(const Vec3& v) {
    float len = Length(v);
    if (len > 0.0f)
        return { v.x / len, v.y / len, v.z / len };
    return { 0, 0, 0 };
}



StunadMesh* OcctTessellator::Tessellate(OcctSolid* solid, float deflection)
{
    // Ensure triangulation exists
    BRepMesh_IncrementalMesh mesher(solid->shape, deflection);

    auto mesh = new StunadMesh();

    for (TopExp_Explorer exp(solid->shape, TopAbs_FACE); exp.More(); exp.Next())
    {
        TopoDS_Face face = TopoDS::Face(exp.Current());
        TopLoc_Location loc;

        Handle(Poly_Triangulation) triangulation =
         BRep_Tool::Triangulation(face, loc);

        if (triangulation.IsNull())
            continue;

        int baseIndex = (int)mesh->vertices.size();
        const bool reversed = (face.Orientation() == TopAbs_REVERSED);
        const int nodeCount = triangulation->NbNodes();
        // vertices
        for (int i = 1; i <= triangulation->NbNodes(); ++i)
        {
            Vertex v;
           // v.position = { x, y, z };
           // v.normal   = { 0.0f, 0.0f, 0.0f }; // will fill later
           // v.uv       = { 0.0f, 0.0f }; 
            gp_Pnt p = triangulation->Node(i).Transformed(loc);
            v.position = { static_cast<float>(p.X()), static_cast<float>(p.Y()), static_cast<float>(p.Z()) };
            v.normal   = { 0.0f, 0.0f, 0.0f }; // will fill later
            v.uv       = { 0.0f, 0.0f }; 
            mesh->vertices.push_back(v);
           // mesh->vertices.push_back({ (float)p.X(), (float)p.Y(), (float)p.Z() });


        }



           //uv

        if (triangulation->HasUVNodes())
        {
           

            for (int i = 1; i <= nodeCount; ++i)
            {
                gp_Pnt2d uv = triangulation->UVNode(i);
                mesh->vertices[baseIndex + i - 1].uv = {
                    static_cast<float>(uv.X()),
                    static_cast<float>(uv.Y())
                };
            }
        }






        // triangles
        const Poly_Array1OfTriangle& triangles = triangulation->Triangles();
        for (int i = triangles.Lower(); i <= triangles.Upper(); ++i)
        {
            int a, b, c;
            triangles(i).Get(a, b, c);
            uint32_t i0 = baseIndex + a - 1;
            uint32_t i1 = baseIndex + b - 1;
            uint32_t i2 = baseIndex + c - 1;

             if (reversed)
             {
                 std::swap(i1, i2);
             }
               


            mesh->triangles.push_back({
                i0,i1,i2
            });

            mesh->indices.push_back(i0);
            mesh->indices.push_back(i1);
            mesh->indices.push_back(i2);


            //face normals
            const Vec3& p0 = mesh->vertices[i0].position;
            const Vec3& p1 = mesh->vertices[i1].position;
            const Vec3& p2 = mesh->vertices[i2].position;

            Vec3 e1 = p1 - p0;
            Vec3 e2 = p2 - p0;
            Vec3 faceNormal = Cross(e1, e2);

            //accumulate
             mesh->vertices[i0].normal =
                mesh->vertices[i0].normal + faceNormal;
            mesh->vertices[i1].normal =
                mesh->vertices[i1].normal + faceNormal;
            mesh->vertices[i2].normal =
                mesh->vertices[i2].normal + faceNormal;




        }
 
   
    }
    for (auto& v : mesh->vertices)
        v.normal = Normalize(v.normal);
    return mesh;
}
