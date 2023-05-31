#include "kaizermud/Api.h"
#include "kaizermud/Components.h"
#include "kaizermud/utils.h"

namespace kaizer {
    using namespace kaizer::components;

    // Administration
    void defaultAtCreate(entt::entity ent) {
        // does nothing for now!
    }

    ApiCall<void> atCreate(defaultAtCreate);

    // Info
    ObjectID defaultGetID(entt::entity ent) {
        return registry.get<ObjectInfo>(ent).id;
    }

    ApiCall<ObjectID> getID(defaultGetID);

    Type* defaultGetType(entt::entity ent, std::string_view name) {
        auto &info = registry.get<ObjectInfo>(ent);
        auto it = info.types.find(std::string(name));
        if(it == info.types.end()) {
            return nullptr;
        }
        return it->second;
    }

    ApiCall<Type*, std::string_view> getType(defaultGetType);


    OpResult<> defaultAddType(entt::entity ent, std::string_view key) {
        if(key.empty()) {
            return {false, "Key cannot be empty"};
        }
        auto k = std::string(key);
        auto found = typeRegistry.find(k);
        if(found == typeRegistry.end()) {
            return {false, "Type does not exist"};
        }
        auto t = found->second;
        auto &info = registry.get<ObjectInfo>(ent);
        if(info.types.find(k) != info.types.end()) {
            return {false, "Type already set"};
        }
        info.types[k] = t.get();
        info.doSort();
        t->onAdd(ent);
        return {true, std::nullopt};
    }

    ApiCall<OpResult<>, std::string_view> addType(defaultAddType);


    // Strings
    OpResult<> defaultSetString(entt::entity ent, std::string_view key, std::string_view value) {
        if(key.empty()) {
            return {false, "Key cannot be empty"};
        }
        if(value.empty()) {
            return clearString(ent, key);
        }

        auto &sc = registry.get_or_emplace<Strings>(ent);
        sc.data[std::string(key)] = intern(value);
        return {true, std::nullopt};

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
            auto [removed, whynot] = clearRelation(ent, key);
            if(!removed) {
                return {false, whynot};
            }
        }
        rel.data[key] = {target};
        auto &rev = registry.get_or_emplace<ReverseRelations>(target);
        rev.data[key].push_back(ent);
        return {true, std::nullopt};
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


    OpResult<> defaultSetAspectPointer(entt::entity ent, Aspect* asp) {
        auto &comp = registry.get_or_emplace<Aspects>(ent);

        auto &slotData = comp.data[std::string(asp->getSlot())];
        if(slotData) {
            slotData->onRemove(ent);
        }
        slotData = asp;
        slotData->onAdd(ent);
        return {true, std::nullopt};
    }

    ApiCall<OpResult<>, Aspect*> setAspectPointer(defaultSetAspectPointer);


    // Aspects
    Aspect* defaultGetAspect(entt::entity ent, std::string_view name) {
        auto comp = registry.try_get<Aspects>(ent);
        if(!comp) {
            return nullptr;
        }
        auto it = comp->data.find(std::string(name));
        if(it == comp->data.end()) {
            return nullptr;
        }
        return it->second;
    }

    ApiCall<Aspect*, std::string_view> getAspect(defaultGetAspect);

    std::set<std::string> defaultGetAspectSlots(entt::entity ent) {
        auto &objinfo = registry.get<ObjectInfo>(ent);
        std::set<std::string> out;
        for(auto &type : objinfo.sortedTypes) {
            auto aspectSlots = type->getAspectSlots();
            out.insert(aspectSlots.begin(), aspectSlots.end());
        }
        return out;
    }

    ApiCall<std::set<std::string>> getAspectSlots(defaultGetAspectSlots);

    OpResult<> defaultSetAspect(entt::entity ent, std::string_view name, std::string_view value) {
        auto &objinfo = registry.get<ObjectInfo>(ent);
        auto slots = getAspectSlots(ent);
        auto slotIt = slots.find(std::string(name));
        if(slotIt == slots.end()) {
            return {false, "No such aspect slot"};
        }
        auto s = *slotIt;

        auto aspectsIt = aspectRegistry.find(s);
        if(aspectsIt == aspectRegistry.end()) {
            return {false, "No no aspects found for that slot."};
        }

        auto available = aspectsIt->second;

        auto it = available.find(std::string(value));
        if(it == available.end()) {
            return {false, "No such aspect."};
        }

        return setAspectPointer(ent, it->second.get());
    }

    ApiCall<OpResult<>, std::string_view, std::string_view> setAspect(defaultSetAspect);

    // Commands
    std::vector<std::pair<std::string, Command*>> defaultGetSortedCommands(entt::entity ent) {
        auto &cache = registry.get_or_emplace<CommandCache>(ent);
        if(!cache.sortedCommands.empty())
            // Hooray, the cache exists, so we'll use it.
            return cache.sortedCommands;
        // The cache doesn't exist so we'll have to create it.

        auto commands = getCommands(ent);
        return cache.sortedCommands;
    }

    ApiCall<std::vector<std::pair<std::string, Command*>>> getSortedCommands(defaultGetSortedCommands);

    std::unordered_map<std::string, Command*> defaultGetCommands(entt::entity ent) {
        auto &cache = registry.get_or_emplace<CommandCache>(ent);
        if (!cache.commands.empty())
            // Hooray, the cache exists, so we'll use it.
            return cache.commands;
        // The cache doesn't exist so we'll have to create it.
        auto &objinfo = registry.get<ObjectInfo>(ent);
        auto &out = cache.commands;
        for (auto &type: objinfo.sortedTypes) {
            auto commands = commandRegistry.find(std::string(type->getKey()));
            if (commands == commandRegistry.end())
                continue;
            for (auto &[key, cmd]: commands->second) {
                out[key] = cmd.get();
            }
        }
        cache.sortCommands();
        return out;
    }

    ApiCall<std::unordered_map<std::string, Command*>> getCommands(defaultGetCommands);

    // Communication
    OpResult<> defaultSendText(entt::entity ent, std::string_view text) {
        auto session = registry.try_get<components::SessionHolder>(ent);
        if(!session) {
            return {false, "No session."};
        }
        session->data->sendText("I don't understand that command.\n");
        return {true, std::nullopt};
    }

    ApiCall<OpResult<>, std::string_view> sendText(defaultSendText);

}