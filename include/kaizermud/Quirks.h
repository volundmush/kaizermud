#pragma once

#include "kaizermud/base.h"

namespace kaizer {

    class Quirk {
    public:
        [[nodiscard]] virtual std::string_view getSlot() = 0;
        [[nodiscard]] virtual std::string_view getKey() = 0;
        virtual void onRemove(entt::entity ent);
        virtual void onAdd(entt::entity ent);
        virtual void onLoad(entt::entity ent);
        [[nodiscard]] virtual OpResult<> canSet(entt::entity ent);
    };

    extern std::unordered_map<std::string, std::unordered_map<std::string, std::shared_ptr<Quirk>>> quirkRegistry;

    OpResult<> registerQuirk(const std::shared_ptr<Quirk>& entry);


}