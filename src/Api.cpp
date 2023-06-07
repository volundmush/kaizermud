#include "kaizermud/Api.h"
#include "kaizermud/Components.h"
#include "kaizermud/utils.h"
#include "fmt/format.h"
#include "boost/algorithm/string.hpp"
#include "kaizermud/Color.h"
#include "kaizermud/Config.h"
#include "spdlog/spdlog.h"

namespace kaizer {
    using namespace kaizer::components;

    // Administration
    void defaultAtCreate(entt::entity ent) {
        // does nothing for now!
    }

    ApiCall<void> atCreate(defaultAtCreate);

    int16_t defaultGetAdminLevel(entt::entity ent) {
        auto sess = registry.try_get<SessionHolder>(ent);
        if(sess) {
            return sess->data->getAdminLevel();
        } else {
            return 0;
        }
    }

    ApiCall<int16_t> getAdminLevel(defaultGetAdminLevel);

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


    // Integers
    OpResult<> defaultSetInteger(entt::entity ent, std::string_view key, int64_t value) {
        if(key.empty()) {
            return {false, "Key cannot be empty"};
        }

        auto &sc = registry.get_or_emplace<Integers>(ent);
        sc.data[std::string(key)] = value;
        return {true, std::nullopt};

    }

    ApiCall<OpResult<>, std::string_view, int64_t> setInteger(defaultSetInteger);

    OpResult<> defaultClearInteger(entt::entity ent, std::string_view key) {
        if(key.empty()) {
            return {false, "Key cannot be empty"};
        }
        auto &sc = registry.get_or_emplace<Integers>(ent);
        sc.data.erase(std::string(key));
        return {true, std::nullopt};
    }

    ApiCall<OpResult<>, std::string_view> clearInteger(defaultClearInteger);


    std::optional<int64_t> defaultGetInteger(entt::entity ent, std::string_view key) {
        if(key.empty()) {
            return std::nullopt;
        }
        auto sc = registry.try_get<Integers>(ent);
        if(!sc) {
            return std::nullopt;
        }
        auto it = sc->data.find(std::string(key));
        if(it == sc->data.end()) {
            return std::nullopt;
        }
        return it->second;
    }

    ApiCall<std::optional<int64_t>, std::string_view> getInteger(defaultGetInteger);

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
        auto slotIt = slots.find(boost::to_lower_copy(std::string(name)));
        if(slotIt == slots.end()) {
            return {false, "No such aspect slot"};
        }
        const auto& s = *slotIt;

        auto aspectsIt = aspectRegistry.find(s);
        if(aspectsIt == aspectRegistry.end()) {
            return {false, "No no aspects found for that slot."};
        }

        auto available = aspectsIt->second;

        auto it = available.find(boost::to_lower_copy(std::string(value)));
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
            auto commands = expandedCommandRegistry.find(std::string(type->getKey()));
            if (commands == expandedCommandRegistry.end())
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
        if(text.ends_with("\n")) {
            session->data->sendText(text);
        } else {
            auto out = std::string(text);
            session->data->sendText(out + "\n");
        }
        return {true, std::nullopt};
    }

    ApiCall<OpResult<>, std::string_view> sendText(defaultSendText);

    // Movement
    OpResult<> defaultMoveTo(entt::entity ent, CallParameters& param) {
        auto loc = getRelation(ent, "location");
        if(registry.valid(loc)) param.setEntity("origin", loc);
        param.setEntity("mover", ent);

        auto [canMove, reas] = atPreMove(ent, param);
        if(!canMove) {
            return {false, reas};
        }


        if(registry.valid(loc)) {
            auto [canLeave, reason] = atPreObjectLeave(loc, param);
            if(!canLeave) {
                return {false, reason};
            }
        }

        auto dest = param.getEntity("destination");

        if(registry.valid(dest)) {
            if (dest == ent) {
                return {false, "You cannot move into yourself! The universe might implode!"};
            }
            auto d = dest;
            while(true) {
                auto l = getRelation(d, "location");
                if(!registry.valid(l)) break;
                if(l == ent) {
                    return {false, "That would cause circular references! The universe might implode!"};
                }
                d = l;
            }


            auto [canEnter, reason] = atPreObjectReceive(dest, param);
            if(!canEnter) {
                return {false, reason};
            }
        }

        auto quiet = param.getBool("quiet");

        if(registry.valid(loc)) {
            atObjectLeave(loc, param);
            if(!quiet) announceMoveFrom(ent, param);
            clearRelation(ent, "location");
        }

        if(registry.valid(dest)) {
            setRelation(ent, "location", dest);
            atObjectReceive(dest, param);
            if(!quiet) announceMoveTo(ent, param);
        }

        atPostMove(ent, param);
        return {true, std::nullopt};
    }
    ApiCall<OpResult<>, CallParameters&> moveTo(defaultMoveTo);


    OpResult<> defaultAtPreMove(entt::entity ent, const CallParameters& param) {
        return {true, std::nullopt};
    }
    ApiCall<OpResult<>, const CallParameters&> atPreMove(defaultAtPreMove);

    OpResult<> defaultAtPreObjectLeave(entt::entity ent, const CallParameters& param) {
        return {true, std::nullopt};
    }
    ApiCall<OpResult<>, const CallParameters&> atPreObjectLeave(defaultAtPreObjectLeave);

    OpResult<> defaultAtPreObjectReceive(entt::entity ent, const CallParameters& param) {
        return {true, std::nullopt};
    }
    ApiCall<OpResult<>, const CallParameters&> atPreObjectReceive(defaultAtPreObjectReceive);

    void defaultAtObjectLeave(entt::entity ent, const CallParameters& param) {

    }
    ApiCall<void, const CallParameters&> atObjectLeave(defaultAtObjectLeave);

    void defaultAnnounceMoveFrom(entt::entity ent, const CallParameters& param) {

    }
    ApiCall<void, const CallParameters&> announceMoveFrom(defaultAnnounceMoveFrom);

    void defaultAnnounceMoveTo(entt::entity ent, const CallParameters& param) {

    }
    ApiCall<void, const CallParameters&> announceMoveTo(defaultAnnounceMoveTo);

    void defaultAtObjectReceive(entt::entity ent, const CallParameters& param) {

    }
    ApiCall<void, const CallParameters&> atObjectReceive(defaultAtObjectReceive);

    void defaultAtPostMove(entt::entity ent, const CallParameters& param) {
        auto dest = param.getEntity("destination");
        if(registry.valid(dest)) {
            atLook(ent, dest);
        }
    }
    ApiCall<void, const CallParameters&> atPostMove(defaultAtPostMove);

    void defaultStowCharacter(entt::entity ent) {
        auto loc = getRelation(ent, "location");
        if(!registry.valid(loc)) return;
        auto &saved = registry.get<components::SavedLocations>(ent);
        saved.data["logout"] = getID(loc);
        CallParameters params;
        params.setEntity("mover", ent);
        params.setEntity("destination", entt::null);
        params.setString("moveType", "system");
        params.setBool("force", true);
        auto &pen = registry.get_or_emplace<components::PendingMove>(ent);
        pen.params = params;
        pen.reportTo = ent;
    }
    ApiCall<void> stowCharacter(defaultStowCharacter);

    OpResult<entt::entity> defaultGetUnstowLocation(entt::entity ent) {
        std::vector<std::pair<std::string, ObjectID>> toCheck;
        auto saved = registry.try_get<components::SavedLocations>(ent);
        if(saved) {
            auto logout = saved->data.find("logout");
            if(logout != saved->data.end()) {
                toCheck.emplace_back("logout", logout->second);
            }
        }
        toCheck.emplace_back("startingRoom", config::startingRoom);

        for(auto [name, id] : toCheck) {
            auto found = entities.find(id);
            if(found == entities.end()) {
                spdlog::error("Could not find {} location {} for unstow of Entity {}", name, id, getID(ent));
                continue;
            }
            if(!getType(found->second, "room")) {
                spdlog::error("Location {} for unstow of Entity {} is not a room", id, getID(ent));
                continue;
            }
            return {found->second, name};
        }
        return {entt::null, "nowhere"};
    }
    ApiCall<OpResult<entt::entity>> getUnstowLocation(defaultGetUnstowLocation);

    OpResult<> defaultUnstowCharacter(entt::entity ent) {
        auto [destination, destType] = getUnstowLocation(ent);
        if(!registry.valid(destination)) {
            spdlog::error("Could not unstow Entity {} to {}", getID(ent), destType.value());
            return {false, "Could not unstow character"};
        }

        CallParameters params;
        params.setEntity("mover", ent);
        params.setEntity("destination", destination);
        params.setString("moveType", "system");
        params.setBool("force", true);
        auto &pen = registry.get_or_emplace<components::PendingMove>(ent);
        pen.params = params;
        pen.reportTo = ent;
        return {true, std::nullopt};
    }
    ApiCall<OpResult<>> unstowCharacter(defaultUnstowCharacter);

    // Look / Appearance
    std::string defaultGetDisplayName(entt::entity ent, entt::entity looker) {
        auto name = getString(ent, "name");
        if(name.has_value()) return std::string(name.value());
        return fmt::format("Unnamed Object {}", getID(ent));
    }
    ApiCall<std::string, entt::entity> getDisplayName(defaultGetDisplayName);

    std::string defaultGetRoomLine(entt::entity ent, entt::entity looker) {
        return getDisplayName(ent, looker);
    }
    ApiCall<std::string, entt::entity> getRoomLine(defaultGetRoomLine);

    void defaultAtLook(entt::entity ent, entt::entity target) {
        if(!registry.valid(target)) {
            sendText(ent, "You don't see that here.");
            return;
        }

        if(registry.any_of<components::SessionHolder>(ent)) {
            // Rendering a complicated appearance can be expensive.
            // We will only do it if ent has a listening session.
            sendText(ent, renderAppearance(target, ent));
        }

        atDesc(target, ent);

    }
    ApiCall<void, entt::entity> atLook(defaultAtLook);

    void defaultAtDesc(entt::entity ent, entt::entity looker) {

    }
    ApiCall<void, entt::entity> atDesc(defaultAtDesc);

    std::string defaultRenderAppearance(entt::entity ent, entt::entity looker) {
        std::vector<std::string> lines;
        lines.emplace_back(getDisplayName(ent, looker));
        auto desc = getString(ent, "look_description");
        if(desc.has_value()) {
            lines.emplace_back(desc.value());
        }
        auto rev = getReverseRelation(ent, "location");
        if(rev.has_value()) {
            for(auto e : rev.value().get()) {
                if(e == looker) continue;
                if(lines.empty()) {
                    lines.emplace_back("Contents:");
                }
                lines.emplace_back(fmt::format("  {}", getDisplayName(e, looker)));
            }
        }
        return boost::algorithm::join(lines, "\n");
    }
    ApiCall<std::string, entt::entity> renderAppearance(defaultRenderAppearance);

    // Search
    std::set<std::string> defaultGetSearchWords(entt::entity ent, entt::entity looker) {
        auto name = renderAnsi(getDisplayName(ent, looker), ColorType::NoColor);
        std::set<std::string> words;
        boost::split(words, name, boost::algorithm::is_space());
        return words;
    }
    ApiCall<std::set<std::string>, entt::entity> getSearchWords(defaultGetSearchWords);

    bool defaultCheckSearch(entt::entity ent, std::string_view term, entt::entity looker) {
        auto w = getSearchWords(ent, looker);
        std::vector<std::string> words;
        // Sort w into words by string length, smallest first and longest last.
        std::transform(w.begin(), w.end(), std::back_inserter(words), [](const std::string& s) { return s; });
        std::sort(words.begin(), words.end(), [](const std::string& a, const std::string& b) { return a.size() < b.size(); });

        for(auto& word : words) {
            if(boost::istarts_with(word, term)) return true;
        }
        return false;
    }
    ApiCall<bool, std::string_view, entt::entity> checkSearch(defaultCheckSearch);

    bool defaultCanDetect(entt::entity ent, entt::entity looker, std::string_view sense) {
        return true;
    }
    ApiCall<bool, entt::entity, std::string_view> canDetect(defaultCanDetect);



}