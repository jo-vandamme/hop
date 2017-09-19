#pragma once

#include "hop.h"
#include "types.h"
#include "math/vec3.h"
#include "math/bbox.h"

namespace hop { namespace bvh {

class ALIGN(64) Node
{
public:
    Real bbox_data[4 * 3];

    union OffsetUnion
    {
        uint32 right_child; // if this is not a leaf
        uint32 primitive; // we are a bottom-level leaf
        uint32 instance; // we are a top-level leaf
    } offset;

    uint16 num_primitives; // we are a bottom-level leaf, 0 if we are an internal node
    uint8 split_axis;
    uint8 type;

    Node()
        : num_primitives(0), split_axis(0), type(0)
    {
        offset.right_child = 0;
    }

    // uint8 padding[8]
    // TODO test this padding (gives us a total of 64 bytes so consecutive nodes won't straddle cache lines)

    bool is_interior() const;
    bool is_leaf() const;

    void set_type(uint8 type);
    uint8 get_type() const;

    void set_left_bbox(const BBoxr& bbox);
    void set_right_bbox(const BBoxr& bbox);

    BBoxr get_left_bbox() const;
    BBoxr get_right_bbox() const;

    void set_split_axis(uint8 axis);
    uint8 get_split_axis() const;

    void set_instance_index(uint32 index);
    uint32 get_instance_index() const;

    void set_primitives(uint32 prim_offset, uint32 num);
    uint32 get_primitives_offset() const;
    uint32 get_num_primitives() const;

    void offset_child_nodes(uint32 off);
    void set_right_child(uint32 right);
    uint32 get_right_child() const;
};

inline bool Node::is_interior() const
{
    return type == 0;
}

inline bool Node::is_leaf() const
{
    return type != 0;
}

inline void Node::set_type(uint8 t)
{
    type = t;
}

inline uint8 Node::get_type() const
{
    return type;
}

inline void Node::set_left_bbox(const BBoxr& bbox)
{
    for (uint32 i = 0; i < 3; ++i)
    {
        bbox_data[i * 4 + 0] = bbox.pmin[i];
        bbox_data[i * 4 + 2] = bbox.pmax[i];
    }
}

inline void Node::set_right_bbox(const BBoxr& bbox)
{
    for (uint32 i = 0; i < 3; ++i)
    {
        bbox_data[i * 4 + 1] = bbox.pmin[i];
        bbox_data[i * 4 + 3] = bbox.pmax[i];
    }
}

inline BBoxr Node::get_left_bbox() const
{
    BBoxr bbox;
    for (uint32 i = 0; i < 3; ++i)
    {
        bbox.pmin[i] = bbox_data[i * 4 + 0];
        bbox.pmax[i] = bbox_data[i * 4 + 2];
    }
    return bbox;
}

inline BBoxr Node::get_right_bbox() const
{
    BBoxr bbox;
    for (uint32 i = 0; i < 3; ++i)
    {
        bbox.pmin[i] = bbox_data[i * 4 + 1];
        bbox.pmax[i] = bbox_data[i * 4 + 3];
    }
    return bbox;
}

inline void Node::set_split_axis(uint8 axis)
{
    split_axis = axis;
}

inline uint8 Node::get_split_axis() const
{
    return split_axis;
}

inline void Node::set_instance_index(uint32 index)
{
    offset.instance = index;
}

inline uint32 Node::get_instance_index() const
{
    return offset.instance;
}

inline void Node::set_primitives(uint32 prim_offset, uint32 num)
{
    offset.primitive = prim_offset;
    num_primitives = num;
}

inline uint32 Node::get_primitives_offset() const
{
    return offset.primitive;
}

inline uint32 Node::get_num_primitives() const
{
    return num_primitives;
}

inline void Node::offset_child_nodes(uint32 off)
{
    // Ignore leaves
    if (is_leaf())
        return;

    offset.right_child += off;
}

inline void Node::set_right_child(uint32 right)
{
    offset.right_child = right;
}

inline uint32 Node::get_right_child() const
{
    return offset.right_child;
}

} } // namespace hop::bvh
