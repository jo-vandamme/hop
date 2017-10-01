#include "geometry/triangle_mesh.h"
#include "math/bbox.h"
#include "math/math.h"

#include <string>
#include <utility>
#include <vector>

namespace hop {

TriangleMesh::TriangleMesh(const std::string& name,
                           std::vector<Triangle>& triangles)
    : m_name(name), m_triangles(std::move(triangles))
{
    m_bbox = BBoxr();
    for (auto& tri : m_triangles)
    {
        const BBoxr bbox = tri.get_bbox();
        m_bboxes.push_back(bbox);
        m_bbox.merge(bbox);
    }
    m_centroid = m_bbox.get_centroid();
    m_num_primitives = m_triangles.size();
}

void TriangleMesh::clear_triangles()
{
    // Release the vector and its associated memory
    m_triangles = std::vector<Triangle>();
}

void TriangleMesh::clear_bboxes()
{
    // Release the vector and its associated memory
    m_bboxes = std::vector<BBoxr>();
}

BBoxr TriangleMesh::get_bbox(const Transformr& xfm, bool compute_tight_bbox) const
{
    if (m_triangles.empty() || !compute_tight_bbox)
        return transform_bbox(xfm, m_bbox);

    BBoxr bbox;
    for (auto& tri : m_triangles)
    {
        Vec3r v0 = transform_point(xfm, Vec3r(tri.vertices[0]));
        Vec3r v1 = transform_point(xfm, Vec3r(tri.vertices[1]));
        Vec3r v2 = transform_point(xfm, Vec3r(tri.vertices[2]));
        bbox.merge(BBoxr(v0, v1, v2));
    }
    return bbox;
}

} // namespace hop
