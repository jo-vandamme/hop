#include "geometry/shape.h"

namespace hop {

Shape::Shape()
    : m_mask(0), m_num_instances(0)
{
    set_visible(true);
}

Shape::~Shape()
{
}

bool Shape::is_visible() const
{
    return (m_mask & ShapeMask::VISIBLE) != 0;
}

void Shape::set_visible(bool visible)
{
    if (visible)
        m_mask |= ShapeMask::VISIBLE;
    else
        m_mask &= ~ShapeMask::VISIBLE;
}

} // namespace hop
