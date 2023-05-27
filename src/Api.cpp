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
    OpResult<> defaultSetString(entt::entity ent, std::string_view key, std::string_view value) {
        if(key.empty()) {
            return {false, "Key cannot be empty"};
        }
        if(value.empty()) {
            return clearString(ent, key);
        }

        auto &sc = registry.get_or_emplace<Strings>(ent);
        sc.data[std::string(key)] = utils::intern(value);

    }

    ApiCall<OpResult<>, std::string_view, std::string_view> setString(defaultSetString);

    OpResult<> defaultClearString(entt::entity ent, std::string_view key) {
        if(key.empty()) {
            return {false, "Key cannot be empty"};
        }
        auto &sc = registry.get_or_emplace<Strings>(ent);
        sc.data.erase(std::string(key));
        return {true, std::nullopt};
    }

    ApiCall<OpResult<>, std::string_view> clearString(defaultClearString);


    std::optional<std::string_view> defaultGetString(entt::entity ent, std::string_view key) {
        if(key.empty()) {
            return std::nullopt;
        }
        auto sc = registry.try_get<Strings>(ent);
        if(!sc) {
            return std::nullopt;
        }
        auto it = sc->data.find(std::string(key));
        if(it == sc->data.end()) {
            return std::nullopt;
        }
        return it->second;
    }

    ApiCall<std::optional<std::string_view>, std::string_view> getString(defaultGetString);


    // Relations
    OpResult<> defaultSetRelation(entt::entity ent, std::string_view name, entt::entity target) {
        auto &rel = registry.get_or_emplace<Relations>(ent);
        auto key = std::string(name);
        auto exist = rel.data.find(key);
        if(exist != rel.data.end()) {
            auto [removed, whynot] = clearRelation(ent, name);
            if(!removed) {
                return {false, whynot};
            }
        }
        rel.data[key] = {target};
        auto &rev = registry.get_or_emplace<ReverseRelations>(target);
        rev.data[key].push_back(ent);
    }

    ApiCall<OpResult<>, std::string_view, entt::entity> setRelation(defaultSetRelation);

    OpResult<> defaultClearRelation(entt::entity ent, std::string_view name) {
        auto rel = registry.try_get<Relations>(ent);
        if(!rel) {
            return {true, std::nullopt};
        }
        auto key = std::string(name);
        auto it = rel->data.find(key);
        if(it == rel->data.end()) {
            return {true, std::nullopt};
        }
        auto found = it->second;
        rel->data.erase(it);
        auto rev = registry.try_get<ReverseRelations>(it->second);
        if(rev) {
            auto &vec = rev->data[key];
            vec.erase(std::remove(vec.begin(), vec.end(), ent), vec.end());
        }
        return {true, std::nullopt};
    }

    ApiCall<OpResult<>, std::string_view> clearRelation(defaultClearRelation);


    entt::entity defaultGetRelation(entt::entity ent, std::string_view name) {
        auto rel = registry.try_get<Relations>(ent);
        if(!rel) {
            return entt::null;
        }
        auto it = rel->data.find(std::string(name));
        if(it == rel->data.end()) {
            return entt::null;
        }
        return it->second;
    }

    ApiCall<entt::entity, std::string_view> getRelation(defaultGetRelation);

    std::optional<std::reference_wrapper<const std::vector<entt::entity>>> defaultGetReverseRelation(entt::entity ent, std::string_view name) {
        auto rev = registry.try_get<ReverseRelations>(ent);
        if(!rev) {
            return std::nullopt;
        }
        auto it = rev->data.find(std::string(name));
        if(it == rev->data.end()) {
            return std::nullopt;
        }
        return std::cref(it->second);
    }

    ApiCall<std::optional<std::reference_wrapper<const std::vector<entt::entity>>>, std::string_view> getReverseRelation(defaultGetReverseRelation);

}