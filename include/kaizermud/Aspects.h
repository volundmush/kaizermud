#pragma once

#include <optional>
#include "kaizermud/base.h"
#include "SQLiteCpp/SQLiteCpp.h"
#include "kaizermud/CallParameters.h"

namespace kaizer {

    struct Aspect {
        [[nodiscard]] virtual std::string_view getSlot() = 0;
        [[nodiscard]] virtual std::string_view getKey() = 0;
        virtual void onRemove(entt::entity ent);
        virtual void onAdd(entt::entity ent);
        virtual void onLoad(entt::entity ent);
        [[nodiscard]] virtual OpResult<> canSet(entt::entity ent);
    };

    extern std::unordered_map<std::string, std::unordered_map<std::string, std::shared_ptr<Aspect>>> aspectRegistry;

    OpResult<> registerAspect(std::shared_ptr<Aspect> entry);

    extern std::unordered_map<std::string, std::shared_ptr<Aspect>> aspectSlotDefaults;


}