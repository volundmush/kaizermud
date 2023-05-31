#pragma once
#include "kaizermud/base.h"
#include "SQLiteCpp/SQLiteCpp.h"


namespace kaizer {

    // Base type for all Stats. This implements the base API.
    class Stat {
    public:
        virtual ~Stat() = default;
        std::string objType, name, saveKey;
        virtual void set(entt::entity ent, double value) = 0;
        virtual void modify(entt::entity ent, double value) = 0;
        [[nodiscard]] virtual double get(entt::entity ent);
        [[nodiscard]] virtual double getBase(entt::entity ent) = 0;

    };

    extern std::unordered_map<std::string, std::unordered_map<std::string, std::shared_ptr<Stat>>> statRegistry;

    OpResult<> registerStat(std::shared_ptr<Stat> entry);


}