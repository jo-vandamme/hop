#pragma once

#include "types.h"
#include "geometry/shape.h"

#include <memory>
#include <utility>
#include <unordered_map>

namespace hop {

class ShapeManager
{
public:
    static ShapeManager& instance()
    {
        static ShapeManager sm;
        return sm;
    }

    template <typename ShapeType, typename ...Args>
    static ShapeID create(Args&&... args)
    {
        return instance().create__<ShapeType>(std::forward<Args>(args)...);
    }

    template <typename ShapeType>
    static ShapeType* get(ShapeID id)
    {
        return instance().get__<ShapeType>(id);
    }

    ShapeManager(const ShapeManager&) = delete;
    ShapeManager& operator=(const ShapeManager&) = delete;

private:
    ShapeManager() = default;

    template <typename ShapeType, typename ...Args>
    ShapeID create__(Args&&... args)
    {
        ShapeID id = ++m_next_shapeid;
        std::shared_ptr<ShapeType> shape =
            std::make_shared<ShapeType>(std::forward<Args>(args)...);
        shape->set_id(id);

        m_shapes[id] = std::dynamic_pointer_cast<Shape>(shape);

        return id;
    }

    template <typename ShapeType>
    ShapeType* get__(ShapeID id)
    {
        std::shared_ptr<ShapeType> out = std::dynamic_pointer_cast<ShapeType>(m_shapes[id]);
        return out.get();
    }

private:
    std::unordered_map<ShapeID, std::shared_ptr<Shape>> m_shapes;
    uint32 m_next_shapeid = 0;
};

} // namespace hop
