#include "obj.h"
#include "types.h"
#include "util/string_util.h"
#include "util/file_util.h"
#include "util/log.h"
#include "math/vec2.h"
#include "math/vec3.h"
#include "math/bbox.h"
#include "geometry/shape.h"
#include "geometry/shape_instance.h"
#include "geometry/triangle_mesh.h"
#include "geometry/shape_manager.h"
#include "material/material_manager.h"
#include "except.h"

#include <memory>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

namespace hop { namespace obj {

std::vector<float> parse_floats(const std::vector<std::string>& vec)
{
    std::vector<float> out;
    for (auto& v : vec)
        out.push_back(std::stof(v));
    return out;
}

// Parse face definition. Each face definitions consists of 3 arguments,
// one for each vertex. Each one of the vertex arguments is comprised of
// 1, 2 or 3 args separated by a slash character. The following formats are
// supported:
//     vertex_index
//     vertex_index/uv_index
//     vertex_index//normal_index
//     vertex_index/uv_index/normal_index
//
// Indices start from 1 and may be negative to indicate
// an offset off the end of the vertex/uv list
uint32 parse_index(const std::string& s, size_t size)
{
    int n = 0;
    try {
        n = std::stoi(s) - 1;
    } catch (...) { }
    if (n < 0)
        return uint32(int(size) + n);
    else
        return uint32(n);
}

// Only works with triangular or quad faces and returns an error if a
// face with more than 4 vertices is encountered.
ShapeID load(const char* file)
{
    Log("obj") << INFO << "loading OBJ: " << file;

    std::ifstream file_stream(file);

    if (!file_stream.good())
        throw Error("Can't open OBJ file: " + std::string(file));

    std::vector<std::string> tokens(20);

    std::vector<Vec3r> vertices;
    std::vector<Vec3r> normals;
    std::vector<Vec2r> uvs;
    std::vector<Triangle> triangles;

    MaterialID material_id = 0;

    std::string mesh_name = "default";

    size_t line_num = 0;

    for (std::string line; std::getline(file_stream, line);)
    {
        ++line_num;
        std::stringstream line_stream(line);
        tokens.clear();
        std::string token;
        while (line_stream >> token)
            tokens.push_back(token);

        if (tokens.empty() || tokens[0][0] == '#')
            continue;

        std::string keyword = tokens[0];
        tokens.erase(tokens.begin());

        if (keyword == "usemtl")
        {
            material_id = MaterialManager::create(tokens[0]);
        }
        else if (keyword == "v")
        {
            auto f = parse_floats(tokens);
            vertices.push_back(Vec3r(f[0], f[1], f[2]));
        }
        else if (keyword == "vt")
        {
            auto f = parse_floats(tokens);
            uvs.push_back(Vec2r(f[0], f[1]));
        }
        else if (keyword == "vn")
        {
            auto f = parse_floats(tokens);
            normals.push_back(Vec3r(f[0], f[1], f[2]));
        }
        else if (keyword == "f")
        {
            uint32 face_vert_idx[4] = { 0 };
            uint32 face_norm_idx[4] = { 0 };
            uint32 face_uv_idx[4] = { 0 };

            bool has_uvs = false;
            bool has_normals = false;

            if (tokens.size() > 4)
                throw Error("Expected 3 or 4 arguments at line " + std::to_string(line_num) +
                            " got " + std::to_string(tokens.size()));

            for (size_t i = 0; i < tokens.size(); ++i)
            {
                auto vertex = split_string(tokens[i] + "//", '/');

                face_vert_idx[i] = parse_index(vertex[0], vertices.size());

                if (has_uvs && vertex[1].empty())
                {
                    throw Error("expected uv coordinates at line " + std::to_string(line_num));
                }
                else if (!vertex[1].empty())
                {
                    face_uv_idx[i] = parse_index(vertex[1], uvs.size());
                    has_uvs = true;
                }
                if (has_normals && vertex[2].empty())
                {
                    throw Error("expected normal at line " + std::to_string(line_num));
                }
                else if (!vertex[2].empty())
                {
                    face_norm_idx[i] = parse_index(vertex[2], normals.size());
                    has_normals = true;
                }
            }

            size_t indices_list[2][3] = { { 0, 1, 2 }, { 0, 2, 3 } };
            size_t num_tris = (tokens.size() == 3) ? 1 : 2;
            for (size_t i = 0; i < num_tris; ++i)
            {
                size_t* indices = &indices_list[i][0];
                Triangle tri;

                bool uvs_null = true;
                bool normals_null = true;
                for (size_t j = 0; j < 3; ++j)
                {
                    size_t index = indices[j];
                    tri.vertices[j] = vertices[face_vert_idx[index]];
                    if (has_normals)
                    {
                        tri.normals[j] = normalize(normals[face_norm_idx[index]]);
                        if (tri.normals[j].x != 0.0 || tri.normals[j].y != 0.0 || tri.normals[j].z != 0.0)
                            normals_null = false;
                    }
                    if (has_uvs)
                    {
                        tri.uvs[j] = uvs[face_uv_idx[index]];
                        if (tri.uvs[j].x != 0.0 || tri.uvs[j].y != 0.0)
                            uvs_null = false;
                    }
                }

                if (!has_normals || normals_null)
                {
                    const Vec3r e01 = tri.vertices[1] - tri.vertices[0];
                    const Vec3r e02 = tri.vertices[2] - tri.vertices[0];
                    const Vec3r normal = normalize(cross(e01, e02));
                    tri.normals[0] = tri.normals[1] = tri.normals[2] = normal;
                }

                if (!has_uvs || uvs_null)
                {
                    tri.uvs[0] = Vec2r(0, 0);
                    tri.uvs[1] = Vec2r(1, 0);
                    tri.uvs[2] = Vec2r(1, 1);
                }

                //tri.bbox = BBoxr(tri.vertices[0], tri.vertices[1], tri.vertices[2]);
                //tri.centroid = tri.bbox.get_centroid();
                tri.material_id = material_id;

                triangles.push_back(std::move(tri));
            }
        }
        /*else if (keyword == "g" || keyword == "o")
        {
            if (!triangles.empty())
            {
                std::shared_ptr<TriangleMesh> mesh =
                    std::make_shared<TriangleMesh>(mesh_name, triangles);
                instances.push_back(std::make_shared<ShapeInstance>(mesh, Transformr()));
            }

            mesh_name = tokens[0];
            triangles.clear();
        }*/
    }

    /*if (!triangles.empty())
    {
        std::shared_ptr<TriangleMesh> mesh =
            std::make_shared<TriangleMesh>(mesh_name, triangles);
        instances.push_back(std::make_shared<ShapeInstance>(mesh, Transformr()));
    }*/

    file_stream.close();

    if (triangles.empty())
        return 0;

    const std::string name = remove_extension(get_filename(std::string(file)));
    //std::shared_ptr<TriangleMesh> mesh =
    //    std::make_shared<TriangleMesh>(name, triangles);

    Log("obj") << INFO << "loaded " << triangles.size() << " triangles";

    return ShapeManager::create<TriangleMesh>(name, triangles);
}

} } // namespace hop::obj
