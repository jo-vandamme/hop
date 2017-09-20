#include "integrators/integrator.h"
#include "geometry/world.h"

#include <memory>

namespace hop {

Integrator::Integrator(std::shared_ptr<World> world)
    : m_world(world)
{
}

void Integrator::set_world(std::shared_ptr<World> world)
{
    m_world = world;
}

} // namespace hop
