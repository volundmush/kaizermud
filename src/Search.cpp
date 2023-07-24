#include "kaizermud/Search.h"
#include "boost/algorithm/string.hpp"
#include "kaizermud/Database.h"
#include "kaizermud/Types.h"
#include "kaizermud/Components.h"

namespace kaizer {
    Search::Search(ObjectID id) : id(id) {

    }

    Search& Search::in(ObjectID con) {
        containers.push_back(con);
        return *this;
    }

    Search& Search::eq(ObjectID equipment) {
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

    OpResult<ObjectID> Search::_simplecheck(const std::string& name) {
        if(allowSelf) {
            if(boost::iequals(name, "self") || boost::iequals(name, "me"))
                return {id, "self check"};
        }
        if(allowHere && boost::iequals(name, "here")) {
            auto t = getType(id);
            return {t->getRelation(id, static_cast<int>(RelationKind::Location)), "Location check"};
        }
        // if allowID, check for #ID pattern like #5 or #10 and search entities for it...
        if(allowID) {
            if (name[0] == '#') {
                try {
                    auto num = std::stoll(std::string(name.substr(1)));
                    auto found = entities.find(num);
                    if(found != entities.end())
                    {
                        auto &oinfo = registry.get<components::ObjectInfo>(found->second);
                        return {oinfo.id, "Id search"};
                    }
                    return {-1, "id search"};
                } catch (std::invalid_argument &e) {
                    return {-1, std::nullopt};
                }
            }
        }

        return {-1, std::nullopt};
    }

    bool Search::detect(ObjectID target) {
        auto t = getType(id);
        for(const auto& s : senses) {
            if(t->canDetect(id, target, s))
                return true;
        }
        return false;
    }

    std::vector<ObjectID> Search::find(const std::string& input) {
        auto [res, handled] = _simplecheck(input);
        if(handled) {
            if(res != -1) {
                return {res};
            } else {
                return {};
            }
        }

        // So we need to check if the name is prefixed with <something>.<name>.
        // That might be 5.meat or all.meat for instance. if there's no prefix, we
        // assume it's equal to 1.meat, so to speak.

        std::string prefix, name;
        int64_t num = 1;
        bool allMode = false;

        auto dot = input.find('.');
        if(dot != std::string_view::npos) {
            prefix = input.substr(0, dot);
            name = input.substr(dot+1);
        } else {
            prefix = "1";
            name = input;
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
        std::vector<ObjectID> results;

        for(auto c : equipments) {
            auto ct = getType(c);
            auto eq = ct->getReverseRelation(c, static_cast<int>(RelationKind::Equipment));

            for(auto e : eq) {
                if(e == id) continue;
                if(!senses.empty() && !detect(e)) continue;
                if(aster) {
                    results.push_back(e);
                } else {
                    auto et = getType(e);
                    // In allMode, we want ALL things which match name.
                    // but otherwise, we're looking for the nth instance of name.
                    if(et->checkSearch(e, name, id)) {
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
            auto ct = getType(c);
            auto con = ct->getReverseRelation(c, static_cast<int>(RelationKind::Location));
            if(!con.empty()) {
                for(auto e : con) {
                    if(e == id) continue;
                    if(!senses.empty() && !detect(e)) continue;
                    // in Aster mode, we just grab ALL items found...
                    if(aster) {
                        results.push_back(e);
                    } else {
                        // In allMode, we want ALL things which match name.
                        // but otherwise, we're looking for the nth instance of name.
                        auto et = getType(e);
                        if(et->checkSearch(e, name, id)) {
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