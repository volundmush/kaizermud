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

    // Location
    void defaultSetLocation(entt::entity ent, entt::entity loc) {
        auto &locInfo = registry.get_or_emplace<Location>(ent);
        if(registry.valid(locInfo.data)) {
            // remove ent from old location's contents.
            auto &oldLocCon = registry.get_or_emplace<Contents>(locInfo.data);
            auto it = std::find_if(oldLocCon.data.begin(), oldLocCon.data.end(), [ent](auto e) {
                return e == ent;
            });
            if(it != oldLocCon.data.end()) {
                oldLocCon.data.erase(it);
            }
            if(oldLocCon.data.empty()) {
                registry.remove<Contents>(locInfo.data);
            }
        }
        if(registry.valid(loc)) {
            // add ent to new location's contents.
            auto &newLocCon = registry.get_or_emplace<Contents>(loc);
            newLocCon.data.push_back(ent);
        } else {
            registry.remove<Location>(ent);
        }
    }
    ApiCall<void, entt::entity> setLocation(defaultSetLocation);

    // Commands
    std::vector<std::pair<std::string, Command*>> defaultGetSortedCommands(entt::entity ent) {
        auto &objinfo = registry.get<ObjectInfo>(ent);
        auto &scache = sortedCommandCache[objinfo.typeFlags.to_ulong()];
        if(!scache.empty())
            // Hooray, the cache exists, so we'll use it.
            return scache;
        // The cache doesn't exist so we'll have to create it.
        auto cache = getCommands(ent);

        scache.reserve(cache.size());
        for(auto &[key, cmd] : cache) {
            scache.emplace_back(key, cmd);
        }
        std::sort(scache.begin(), scache.end(), [](auto &a, auto &b) {
            return a.second->getPriority() < b.second->getPriority();
        });

        return scache;
    }

    ApiCall<std::vector<std::pair<std::string, Command*>>> getSortedCommands(defaultGetSortedCommands);

    std::unordered_map<std::string, Command*> defaultGetCommands(entt::entity ent) {
        auto &objinfo = registry.get<ObjectInfo>(ent);
        auto &cache = commandCache[objinfo.typeFlags.to_ulong()];
        if (!cache.empty())
            // Hooray, the cache exists, so we'll use it.
            return cache;
        // The cache doesn't exist so we'll have to create it.

        for (auto i = 0; i < objinfo.typeFlags.size(); i++) {
            auto commands = expandedCommandRegistry.find(i);
            if (commands == expandedCommandRegistry.end())
                continue;
            for (auto &[key, cmd]: commands->second) {
                cache[key] = cmd.get();
            }
        }
        return cache;
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
            setLocation(ent, entt::null);
        }

        if(registry.valid(dest)) {
            setLocation(ent, dest);
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
        auto name = registry.try_get<components::Name>(ent);
        if(name) return std::string(name->data);
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
        auto ldesc = registry.try_get<components::LookDescription>(ent);
        if(ldesc) {
            lines.emplace_back(ldesc->data);
        }
        auto con = registry.try_get<components::Contents>(ent);
        if(con) {
            for(auto &e : con->data) {
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