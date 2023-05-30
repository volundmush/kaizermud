#pragma once

#include "kaizermud/base.h"

namespace kaizer {
    struct Type {
        [[nodiscard]] virtual std::string_view getKey() const = 0;
        [[nodiscard]] virtual std::string_view getName() const = 0;
        [[nodiscard]] virtual std::vector<std::string> getAspectSlots() const;
        [[nodiscard]] virtual std::vector<std::string> getQuirkSlots() const;
        [[nodiscard]] virtual std::vector<std::string> getStatSlots() const;
        [[nodiscard]] virtual std::vector<std::string> getEquipSlots() const;
        [[nodiscard]] virtual int getSortPriority() const ;
        virtual void onLoad(entt::entity ent) const;
        virtual void onAdd(entt::entity ent) const;
        virtual void onRemove(entt::entity ent) const;
    };

    extern std::unordered_map<std::string, Type*> typeRegistry;

}
