#pragma once

#include "types.h"
#include "math/bbox.h"
#include "geometry/surface_interaction.h"

#include <string>
#include <atomic>
#include <memory>

namespace hop {

using ShapeID = uint32;

enum ShapeType
{
    TRIANGLE_MESH
};

enum ShapeMask
{
    VISIBLE = 1 << 0
};

class Ray;
class HitInfo;

class Shape
{
public:
    Shape();
    virtual ~Shape();

    virtual const std::string& get_name() const = 0;
    virtual ShapeType get_type() const = 0;
    virtual uint32 get_num_primitives() const = 0;
    virtual bool is_instance() const = 0;
    virtual const BBoxr& get_bbox() const = 0;
    virtual const Vec3r& get_centroid() const = 0;

    bool is_visible() const;
    void set_visible(bool visible);

    void set_id(ShapeID id) { m_shape_id = id; }
    ShapeID get_id() const { return m_shape_id; }

    uint32 get_num_instances() const { return m_num_instances; }
    uint32 inc_instance_count() { return ++m_num_instances; }

    virtual void get_surface_interaction(const HitInfo& hit, SurfaceInteraction* info) = 0;

protected:
    uint8 m_mask;
    ShapeID m_shape_id;
    std::atomic<uint32> m_num_instances;
};

typedef std::shared_ptr<Shape> ShapePtr;

} // namespace hop
