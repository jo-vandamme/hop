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

} // namespace hop
