#pragma once

#include "material/material.h"

#include <string>
#include <memory>
#include <unordered_map>

namespace hop {

class MaterialManager
{
public:
    static MaterialManager& instance()
    {
        static MaterialManager mm;
        return mm;
    }

    static MaterialID create(const std::string& material_name)
    {
        return instance().create__(material_name);
    }

    MaterialManager(const MaterialManager&) = delete;
    MaterialManager& operator=(const MaterialManager&) = delete;

private:
    MaterialManager();
    MaterialID create__(const std::string& material_name);

private:
    std::unordered_map<std::string, MaterialID> m_name_to_id;
    std::unordered_map<MaterialID, std::shared_ptr<Material>> m_id_to_mat;
    MaterialID m_next_mat_id = 0;
};

} // namespace hop
