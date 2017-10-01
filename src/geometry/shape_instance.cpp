#include "geometry/shape_instance.h"
#include "geometry/shape_manager.h"
#include "math/bbox.h"
#include "math/transform.h"

#include <string>

namespace hop {

ShapeInstance::ShapeInstance(ShapeID shape_id, const Transformr& xfm, bool compute_tight_bbox)
    : m_shape_id(shape_id), m_transform(xfm)
{
    m_shape = ShapeManager::get<Shape>(m_shape_id);
    m_bbox = m_shape->get_bbox(xfm, compute_tight_bbox);
    m_centroid = m_bbox.get_centroid();
    m_name = m_shape->get_name() + "_inst_" + std::to_string(m_shape->inc_instance_count());
    m_transform_swaps_handedness = xfm.swaps_handedness();
}

} // namespace hop
