#include "material/material_manager.h"
#include "material/material.h"
#include "util/log.h"

#include <string>
#include <unordered_map>

namespace hop {

MaterialManager::MaterialManager()
{
    m_name_to_id["default"] = 0;
    m_id_to_mat[0] = std::make_shared<Material>("default");
}

MaterialID MaterialManager::create__(const std::string& material_name)
{
    auto it = m_name_to_id.find(material_name);
    if (it != m_name_to_id.end())
        return it->second;

    MaterialID id = ++m_next_mat_id;
    m_name_to_id[material_name] = id;
    m_id_to_mat[id] = std::make_shared<Material>(material_name);

    Log("material") << DEBUG << "created material " << material_name << " with id " << id;

    return id;
}

} // namespace hop
