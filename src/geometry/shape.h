#pragma once

#include "types.h"
#include "math/bbox.h"
#include "math/transform.h"

#include <string>
#include <atomic>
#include <memory>

namespace hop {

enum ShapeType
{
    TRIANGLE_MESH
};

class Ray;
class HitInfo;

class Shape
{
public:
    Shape() : m_num_instances(0) { }
    virtual ~Shape() { }

    virtual const std::string& get_name() const = 0;
    virtual ShapeType get_type() const = 0;
    virtual uint64 get_num_primitives() const = 0;
    virtual bool is_instance() const = 0;
    virtual const BBoxr& get_bbox() const = 0;
    virtual const Vec3r& get_centroid() const = 0;

    // Shape implementations can override this method to
    // return a tighter bounding box by first transforming the primitives
    // and then computing a bounding box
    virtual BBoxr get_bbox(const Transformr& xfm, bool /*compute_tight_bbox*/) const
    {
        return transform_bbox(xfm, get_bbox());
    }

    void set_id(ShapeID id) { m_shape_id = id; }
    ShapeID get_id() const { return m_shape_id; }

    uint32 get_num_instances() const { return m_num_instances; }
    uint32 inc_instance_count() { return ++m_num_instances; }

protected:
    ShapeID m_shape_id;
    std::atomic<uint32> m_num_instances;
};

typedef std::shared_ptr<Shape> ShapePtr;

} // namespace hop
