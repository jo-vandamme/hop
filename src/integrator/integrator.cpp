#include "integrator/integrator.h"
#include "geometry/world.h"

#include <memory>

namespace hop {

Integrator::Integrator(std::shared_ptr<World> world, float ray_epsilon)
    : m_world(world), m_ray_epsilon(ray_epsilon)
{
}

void Integrator::set_world(std::shared_ptr<World> world)
{
    m_world = world;
}

} // namespace hop
