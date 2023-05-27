#include "kaizermud/Api.h"
#include "kaizermud/Components.h"
#include "kaizermud/utils.h"

namespace kaizermud::api {
    using namespace kaizermud::components;

    // Administration
    void defaultAtCreate(entt::entity ent) {
        // does nothing for now!
    }

    ApiCall<void> atCreate(defaultAtCreate);


    // Strings
    OpResult<> defaultSetString(entt::entity ent, const std::string& key, const std::string& value) {
        if(key.empty()) {
            return {false, "Key cannot be empty"};
        }
        if(value.empty()) {
            return clearString(ent, key);
        }

        auto &sc = registry.get_or_emplace<Strings>(ent);
        sc.data[key] = value;
        return {true, std::nullopt};

    }

    ApiCall<OpResult<>, const std::string&, const std::string&> setString(defaultSetString);

    OpResult<> defaultClearString(entt::entity ent, const std::string& key) {
        if(key.empty()) {
            return {false, "Key cannot be empty"};
        }
        auto &sc = registry.get_or_emplace<Strings>(ent);
        sc.data.erase(key);
        return {true, std::nullopt};
    }

    ApiCall<OpResult<>, const std::string&> clearString(defaultClearString);


    std::optional<std::string> defaultGetString(entt::entity ent, const std::string& key) {
        if(key.empty()) {
            return std::nullopt;
        }
        auto sc = registry.try_get<Strings>(ent);
        if(!sc) {
            return std::nullopt;
        }
        auto it = sc->data.find(key);
        if(it == sc->data.end()) {
            return std::nullopt;
        }
        return it->second;
    }

    ApiCall<std::optional<std::string>, const std::string&> getString(defaultGetString);


    // Relations
    OpResult<> defaultSetRelation(entt::entity ent, const std::string& name, entt::entity target) {
        auto &rel = registry.get_or_emplace<Relations>(ent);
        auto exist = rel.data.find(name);
        if(exist != rel.data.end()) {
            auto [removed, whynot] = clearRelation(ent, name);
            if(!removed) {
                return {false, whynot};
            }
        }
        rel.data[name] = {target};
        auto &rev = registry.get_or_emplace<ReverseRelations>(target);
        rev.data[name].push_back(ent);
        return {true, std::nullopt};
    }

    ApiCall<OpResult<>, const std::string&, entt::entity> setRelation(defaultSetRelation);

    OpResult<> defaultClearRelation(entt::entity ent, const std::string& name) {
        auto rel = registry.try_get<Relations>(ent);
        if(!rel) {
            return {true, std::nullopt};
        }
        auto it = rel->data.find(name);
        if(it == rel->data.end()) {
            return {true, std::nullopt};
        }
        auto found = it->second;
        rel->data.erase(it);
        auto rev = registry.try_get<ReverseRelations>(it->second);
        if(rev) {
            auto &vec = rev->data[name];
            vec.erase(std::remove(vec.begin(), vec.end(), ent), vec.end());
        }
        return {true, std::nullopt};
    }

    ApiCall<OpResult<>, const std::string&> clearRelation(defaultClearRelation);


    entt::entity defaultGetRelation(entt::entity ent, const std::string& name) {
        auto rel = registry.try_get<Relations>(ent);
        if(!rel) {
            return entt::null;
        }
        auto it = rel->data.find(name);
        if(it == rel->data.end()) {
            return entt::null;
        }
        return it->second;
    }

    ApiCall<entt::entity, const std::string&> getRelation(defaultGetRelation);

    std::optional<std::reference_wrapper<const std::vector<entt::entity>>> defaultGetReverseRelation(entt::entity ent, const std::string& name) {
        auto rev = registry.try_get<ReverseRelations>(ent);
        if(!rev) {
            return std::nullopt;
        }
        auto it = rev->data.find(name);
        if(it == rev->data.end()) {
            return std::nullopt;
        }
        return std::cref(it->second);
    }

    ApiCall<std::optional<std::reference_wrapper<const std::vector<entt::entity>>>, const std::string&> getReverseRelation(defaultGetReverseRelation);

}