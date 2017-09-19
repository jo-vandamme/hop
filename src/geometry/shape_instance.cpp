#include "geometry/shape_instance.h"
#include "geometry/shape_manager.h"
#include "geometry/hit_info.h"
#include "geometry/surface_interaction.h"
#include "math/bbox.h"
#include "math/transform.h"

#include <string>
#include <memory>

namespace hop {

ShapeInstance::ShapeInstance(ShapeID shape_id, const Transformr& xfm)
    : m_shape_id(shape_id), m_transform(xfm)
{
    m_shape = ShapeManager::get<Shape>(m_shape_id);
    m_bbox = transform_bbox(xfm, m_shape->get_bbox());
    m_name = m_shape->get_name() + "_inst_" + std::to_string(m_shape->inc_instance_count());
}

void ShapeInstance::get_surface_interaction(const HitInfo& hit, SurfaceInteraction* info)
{
    m_shape->get_surface_interaction(hit, info);
    info->normal = transform_normal(m_transform, info->normal);
}

} // namespace hop
