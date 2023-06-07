#pragma once
#include "kaizermud/base.h"

namespace kaizer {
    class Search {
    public:
        explicit Search(entt::entity ent);
        Search& in(entt::entity container);
        Search& eq(entt::entity equipment);
        Search& sense(std::string_view sense);
        Search& useID(bool useID);
        Search& useSelf(bool useSelf);
        Search& useAll(bool useAll);
        Search& useHere(bool useHere);
        Search& useAsterisk(bool useAsterisk);

        virtual std::vector<entt::entity> find(std::string_view name);

    protected:
        OpResult<entt::entity> _simplecheck(std::string_view name);
        bool detect(entt::entity target);
        entt::entity ent;
        std::string txt;
        std::vector<entt::entity> containers;
        std::vector<entt::entity> equipments;
        std::set<std::string> senses;
        bool allowID{false};
        bool allowSelf{true};
        bool allowAll{true};
        bool allowHere{false};
        bool allowAsterisk{false};

    };
}