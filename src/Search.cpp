#include "kaizermud/Search.h"
#include "kaizermud/Api.h"
#include "boost/algorithm/string.hpp"

namespace kaizer {
    Search::Search(entt::entity ent) : ent(ent) {

    }

    Search& Search::in(entt::entity container) {
        containers.push_back(container);
        return *this;
    }

    Search& Search::eq(entt::entity equipment) {
        equipments.push_back(equipment);
        return *this;
    }

    Search& Search::sense(std::string_view sense) {
        senses.insert(std::string(sense));
        return *this;
    }

    Search& Search::useID(bool useID) {
        this->allowID = useID;
        return *this;
    }

    Search& Search::useSelf(bool useSelf) {
        this->allowSelf = useSelf;
        return *this;
    }

    Search& Search::useAll(bool useAll) {
        this->allowAll = useAll;
        return *this;
    }

    Search& Search::useHere(bool useHere) {
        this->allowHere = useHere;
        return *this;
    }

    Search& Search::useAsterisk(bool useAsterisk) {
        this->allowAsterisk = useAsterisk;
        return *this;
    }

    OpResult<entt::entity> Search::_simplecheck(std::string_view name) {
        if(allowSelf) {
            if(boost::iequals(name, "self") || boost::iequals(name, "me"))
                return {ent, "self check"};
        }
        if(allowHere && boost::iequals(name, "here")) {
            return {getRelation(ent, "location"), "Location check"};
        }
        // if allowID, check for #ID pattern like #5 or #10 and search entities for it...
        if(allowID) {
            if (name[0] == '#') {
                try {
                    auto id = std::stoll(std::string(name.substr(1)));
                    auto found = entities.find(id);
                    if(found != entities.end())
                        return {found->second, "Id search"};
                    return {entt::null, "id search"};
                } catch (std::invalid_argument &e) {
                    return {entt::null, std::nullopt};
                }
            }
        }

        return {entt::null, std::nullopt};
    }

    bool Search::detect(entt::entity target) {
        for(const auto& s : senses) {
            if(canDetect(ent, target, s))
                return true;
        }
        return false;
    }

    std::vector<entt::entity> Search::find(std::string_view name) {
        auto [res, handled] = _simplecheck(name);
        if(handled) {
            if(registry.valid(res)) {
                return {res};
            } else {
                return {};
            }
        }

        // So we need to check if the name is prefixed with <something>.<name>.
        // That might be 5.meat or all.meat for instance. if there's no prefix, we
        // assume it's equal to 1.meat, so to speak.

        std::string_view prefix;
        int64_t num = 1;
        bool allMode = false;

        auto dot = name.find('.');
        if(dot != std::string_view::npos) {
            prefix = name.substr(0, dot);
            name = name.substr(dot+1);
        } else {
            prefix = "1";
        }

        // Now check to make sure that the prefix is either a number or the string "all".
        if(!boost::iequals(prefix, "all")) {
            try {
                num = std::stoll(std::string(prefix));
                if(num < 1) return {};
            } catch (std::invalid_argument &e) {
                return {};
            }
        } else {
            // switch to AllMode...
            allMode = true && allowAll;
        }

        std::size_t count = 0;
        bool aster = false;
        if(name == "*") {
            if(!allowAsterisk) return {};
            aster = true;
        }
        std::vector<entt::entity> results;

        for(auto c : equipments) {
            auto eq = registry.try_get<components::Equipment>(c);
            if(!eq) continue;
            for(auto [slot, e] : eq->data) {
                if(e == ent) continue;
                if(!senses.empty() && !detect(e)) continue;
                if(aster) {
                    results.push_back(e);
                } else {
                    // In allMode, we want ALL things which match name.
                    // but otherwise, we're looking for the nth instance of name.
                    if(checkSearch(e, name, ent)) {
                        if(allMode) {
                            results.push_back(e);
                        } else {
                            count++;
                            if(count == num) {
                                results.push_back(e);
                                return results;
                            }
                        }
                    }
                }
            }
        }

        for(auto c : containers) {
            auto rev = getReverseRelation(c, "location");
            if(rev.has_value()) {
                for(auto e : rev.value().get()) {
                    if(e == ent) continue;
                    if(!senses.empty() && !detect(e)) continue;
                    // in Aster mode, we just grab ALL items found...
                    if(aster) {
                        results.push_back(e);
                    } else {
                        // In allMode, we want ALL things which match name.
                        // but otherwise, we're looking for the nth instance of name.
                        if(checkSearch(e, name, ent)) {
                            if(allMode) {
                                results.push_back(e);
                            } else {
                                count++;
                                if(count == num) {
                                    results.push_back(e);
                                    return results;
                                }
                            }
                        }
                    }
                }
            }
        }


        return results;
    }


}