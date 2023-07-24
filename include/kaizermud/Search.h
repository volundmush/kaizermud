#pragma once
#include "kaizermud/base.h"

namespace kaizer {
    class Search {
    public:
        explicit Search(ObjectID id);
        Search& in(ObjectID id);
        Search& eq(ObjectID id);
        Search& sense(std::string_view sense);
        Search& useID(bool useID);
        Search& useSelf(bool useSelf);
        Search& useAll(bool useAll);
        Search& useHere(bool useHere);
        Search& useAsterisk(bool useAsterisk);

        virtual std::vector<ObjectID> find(const std::string& input);

    protected:
        OpResult<ObjectID> _simplecheck(const std::string& name);
        bool detect(ObjectID target);
        ObjectID id;
        std::string txt;
        std::vector<ObjectID> containers;
        std::vector<ObjectID> equipments;
        std::set<std::string> senses;
        bool allowID{false};
        bool allowSelf{true};
        bool allowAll{true};
        bool allowHere{false};
        bool allowAsterisk{false};

    };
}