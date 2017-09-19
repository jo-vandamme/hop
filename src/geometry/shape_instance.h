#pragma once

#include "geometry/shape.h"
#include "geometry/hit_info.h"
#include "geometry/surface_interaction.h"
#include "geometry/shape_manager.h"
#include "math/bbox.h"
#include "math/transform.h"

#include <string>
#include <memory>

namespace hop {

class ShapeInstance : public Shape
{
public:
    ShapeInstance(ShapeID id, const Transformr& xfm);

    const std::string& get_name() const override { return m_name; }
    ShapeType get_type() const override { return m_shape->get_type(); };
    uint32 get_num_primitives() const override { return m_shape->get_num_primitives(); };
    bool is_instance() const override { return true; };
    BBoxr get_bbox() override { return m_bbox; };

    void get_surface_interaction(const HitInfo& hit, SurfaceInteraction* info) override;

    Shape* get_shape() const { return m_shape; }

    const Transformr& get_transform() const { return m_transform; }

private:
    Shape* m_shape;
    ShapeID m_shape_id;
    Transformr m_transform;
    BBoxr m_bbox;
    std::string m_name;
};

typedef std::shared_ptr<ShapeInstance> ShapeInstancePtr;

} // namespace hop
