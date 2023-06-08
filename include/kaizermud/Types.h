#pragma once

#include "kaizermud/base.h"

namespace kaizer {
    struct Type {
        [[nodiscard]] virtual std::size_t getID() const = 0;
        [[nodiscard]] virtual std::string_view getName() const = 0;
        [[nodiscard]] virtual std::vector<std::string> getStatSlots() const;
        [[nodiscard]] virtual std::vector<std::string> getEquipSlots() const;
        virtual void onLoad(entt::entity ent) const;
        virtual void onAdd(entt::entity ent) const;
        virtual void onRemove(entt::entity ent) const;
    };

    extern std::vector<std::shared_ptr<Type>> typeRegistry;

    void registerType(const std::shared_ptr<Type>& entry);

}
