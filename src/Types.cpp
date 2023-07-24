#include <kaizermud/Config.h>
#include "kaizermud/Types.h"
#include "kaizermud/Database.h"
#include "kaizermud/Color.h"
#include "boost/algorithm/string.hpp"
#include "kaizermud/Components.h"
#include "fmt/format.h"
#include "spdlog/spdlog.h"

namespace kaizer {

    ObjectID Type::create() {
        auto ref = createRef();
        SQLite::Statement q1(*db, "INSERT INTO objects (ref) VALUES (?)");
        q1.bind(1, ref);
        q1.exec();
        auto id = db->getLastInsertRowid();
        atCreate(id);
        return id;
    }

    ObjectID Type::create(ObjectID id) {
        auto ref = createRef();
        SQLite::Statement q1(*db, "INSERT INTO objects (id, ref) VALUES (?,?)");
        q1.bind(1, id);
        q1.bind(2, ref);
        q1.exec();
        atCreate(id);
        auto rid = db->getLastInsertRowid();
        return rid;
    }

    ObjectID Type::createProto(const std::string& name) {
        auto ref = createRef();
        SQLite::Statement q1(*db, "INSERT INTO prototypes (name, ref) VALUES (?,?)");
        q1.bind(1, name);
        q1.bind(2, ref);
        q1.exec();
        auto id = db->getLastInsertRowid();
        atCreateProto(id);
        return id;
    }

    ObjectID Type::createRef() {
        SQLite::Statement q1(*db, "INSERT INTO refs (class) VALUES (?)");
        q1.bind(1, getTypeName());
        q1.exec();
        auto ref = db->getLastInsertRowid();
        atCreateRef(ref);
        return ref;
    }

    void Type::atCreate(ObjectID id) {

    }

    void Type::atCreateProto(ObjectID proto) {

    }

    void Type::atCreateRef(ObjectID ref) {

    }

    int16_t Type::getAdminLevel() {
        return 0;
    }

    void Type::setRelation(ObjectID id, int relationKind, ObjectID relation) {
        if(relation == -1) {
            SQLite::Statement q1(*db, "DELETE FROM relations WHERE object=? AND relationKind=?");
            q1.bind(1, id);
            q1.bind(2, relationKind);
            q1.exec();
        } else {
            SQLite::Statement q1(*db, "INSERT OR REPLACE INTO relations (object, relationKind, relation) VALUES (?, ?, ?)");
            q1.bind(1, id);
            q1.bind(2, relationKind);
            q1.bind(3, relation);
            q1.exec();
        }
    }

    ObjectID Type::getRelation(ObjectID id, int relationKind) {
        SQLite::Statement q1(*db, "SELECT relation FROM relations WHERE object=? AND relationKind=?");
        q1.bind(1, id);
        q1.bind(2, relationKind);
        if(q1.executeStep()) {
            return q1.getColumn(0).getInt64();
        }
        return -1;
    }

    std::vector<ObjectID> Type::getReverseRelation(ObjectID id, int relationKind) {
        SQLite::Statement q1(*db, "SELECT object FROM relations WHERE relation=? AND relationKind=?");
        q1.bind(1, id);
        q1.bind(2, relationKind);
        std::vector<ObjectID> ret;
        while(q1.executeStep()) {
            ret.push_back(q1.getColumn(0).getInt64());
        }
        return ret;
    }

    ObjectID Type::getRefID(ObjectID id) {
        // we must retrieve the ref ID from the objects table for this object id.
        SQLite::Statement q1(*db, "SELECT ref FROM objects WHERE id=?");
        q1.bind(1, id);
        if(q1.executeStep()) {
            return q1.getColumn(0).getInt64();
        }
        return -1;
    }

    ObjectID Type::getRefIDProto(ObjectID proto) {
        // we must retrieve the ref ID from the objects table for this object id.
        SQLite::Statement q1(*db, "SELECT ref FROM prototypes WHERE id=?");
        q1.bind(1, proto);
        if(q1.executeStep()) {
            return q1.getColumn(0).getInt64();
        }
        return -1;
    }

    void Type::setStringRef(kaizer::ObjectID ref, const std::string &key, const std::string &value) {
        auto lkey = boost::to_lower_copy(key);

        // If value is empty or just whitespace, we are deleting the string from this ref.
        if(value.empty() || std::all_of(value.begin(), value.end(), [](unsigned char c){ return std::isspace(c); })) {
            SQLite::Statement q1(*db, "DELETE FROM refStrings WHERE ref=? AND name=?");
            q1.bind(1, ref);
            q1.bind(2, lkey);
            q1.exec();
            return;
        }

        // If value is not being deleted, then we are setting/replacing it.
        // First we check to see if the string exists in strings and if not, we create it.
        int64_t stringID = -1;
        SQLite::Statement q1(*db, "SELECT id FROM strings WHERE data=?");
        q1.bind(1, value);
        if(q1.executeStep()) {
            stringID = q1.getColumn(0).getInt64();
        } else {
            SQLite::Statement q2(*db, "INSERT INTO strings (data, clean) VALUES (?,?)");
            q2.bind(1, value);
            q2.bind(2, stripAnsi(value));
            q2.exec();
            stringID = db->getLastInsertRowid();
        }

        // now we insert-or-replace into refstrings for this ref.
        SQLite::Statement q3(*db, "INSERT OR REPLACE INTO refStrings (ref, name, string) VALUES (?, ?, ?)");
        q3.bind(1, ref);
        q3.bind(2, lkey);
        q3.bind(3, stringID);
        q3.exec();
    }

    void Type::setString(ObjectID id, const std::string& key, const std::string& value) {
        // First let's get the ref ID from objects for this object ID.
        auto refid = getRefID(id);
        setStringRef(refid, key, value);
    }

    void Type::setStringProto(ObjectID proto, const std::string& key, const std::string& value) {
        // First let's get the ref ID from objects for this object ID.
        auto refid = getRefIDProto(proto);
        setStringRef(refid, key, value);
    }

    std::string Type::getStringRef(kaizer::ObjectID ref, const std::string &key, bool clean) {
        auto lkey = boost::to_lower_copy(key);

        // Now we can get the string from refStrings for this ref.
        SQLite::Statement q1(*db, "SELECT strings.data,strings.clean FROM refStrings INNER JOIN strings ON refStrings.string=strings.id WHERE ref=? AND name=?");
        q1.bind(1, ref);
        q1.bind(2, lkey);
        if(q1.executeStep()) {
            if(clean) {
                return q1.getColumn(1).getText();
            }
            return q1.getColumn(0).getText();
        }
        return "";
    }

    std::string Type::getString(ObjectID id, const std::string& key, bool clean) {
        // First let's get the ref ID from objects for this object ID.
        auto refid = getRefID(id);
        return getStringRef(refid, key, clean);
    }

    std::string Type::getStringProto(kaizer::ObjectID proto, const std::string &key, bool clean) {
        // First let's get the ref ID from objects for this object ID.
        auto refid = getRefIDProto(proto);
        return getStringRef(refid, key, clean);
    }


    void Type::setIntRef(kaizer::ObjectID ref, const std::string &key, int64_t value) {
        // This works almost exactly like setStringRef but we want to target the refInts table, and the
        // value is stored on that table directly.
        auto lkey = boost::to_lower_copy(key);

        if(value == 0) {
            SQLite::Statement q1(*db, "DELETE FROM refInts WHERE ref=? AND name=?");
            q1.bind(1, ref);
            q1.bind(2, lkey);
            q1.exec();
            return;
        }

        // Otherwise, insert or replace into refInts...
        SQLite::Statement q2(*db, "INSERT OR REPLACE INTO refInts (ref, name, value) VALUES (?, ?, ?)");
        q2.bind(1, ref);
        q2.bind(2, lkey);
        q2.bind(3, value);
        q2.exec();
    }

    void Type::setInt(kaizer::ObjectID id, const std::string &key, int64_t value) {
        // First let's get the ref ID from objects for this object ID.
        auto refid = getRefID(id);
        setIntRef(refid, key, value);
    }

    void Type::setIntProto(kaizer::ObjectID proto, const std::string &key, int64_t value) {
        // First let's get the ref ID from objects for this object ID.
        auto refid = getRefIDProto(proto);
        setIntRef(refid, key, value);
    }

    int64_t Type::getIntRef(kaizer::ObjectID ref, const std::string &key) {
        auto lkey = boost::to_lower_copy(key);

        // Now we can get the string from refStrings for this ref.
        SQLite::Statement q1(*db, "SELECT value FROM refInts WHERE ref=? AND name=?");
        q1.bind(1, ref);
        q1.bind(2, lkey);
        if(q1.executeStep()) {
            return q1.getColumn(0).getInt64();
        }
        return 0;
    }

    int64_t Type::getInt(kaizer::ObjectID id, const std::string &key) {
        // First let's get the ref ID from objects for this object ID.
        auto refid = getRefID(id);
        return getIntRef(refid, key);
    }

    int64_t Type::getIntProto(kaizer::ObjectID proto, const std::string &key) {
        // First let's get the ref ID from objects for this object ID.
        auto refid = getRefIDProto(proto);
        return getIntRef(refid, key);
    }

    void Type::setRealRef(ObjectID ref, const std::string& key, double value) {
        // This works almost exactly like setStringRef but we want to target the refInts table, and the
        // value is stored on that table directly.
        auto lkey = boost::to_lower_copy(key);

        if(value == 0) {
            SQLite::Statement q1(*db, "DELETE FROM refReals WHERE ref=? AND name=?");
            q1.bind(1, ref);
            q1.bind(2, lkey);
            q1.exec();
            return;
        }

        // Otherwise, insert or replace into refInts...
        SQLite::Statement q2(*db, "INSERT OR REPLACE INTO refReals (ref, name, value) VALUES (?, ?, ?)");
        q2.bind(1, ref);
        q2.bind(2, lkey);
        q2.bind(3, value);
        q2.exec();
    }

    void Type::setReal(ObjectID id, const std::string& key, double value) {
        // First let's get the ref ID from objects for this object ID.
        auto refid = getRefID(id);
        setRealRef(refid, key, value);
    }

    void Type::setRealProto(ObjectID proto, const std::string& key, double value) {
        // First let's get the ref ID from objects for this object ID.
        auto refid = getRefIDProto(proto);
        setRealRef(refid, key, value);
    }

    double Type::getRealRef(ObjectID ref, const std::string& key) {
        auto lkey = boost::to_lower_copy(key);

        // Now we can get the string from refStrings for this ref.
        SQLite::Statement q1(*db, "SELECT value FROM refReals WHERE ref=? AND name=?");
        q1.bind(1, ref);
        q1.bind(2, lkey);
        if(q1.executeStep()) {
            return q1.getColumn(0).getDouble();
        }
        return 0;
    }

    double Type::getReal(ObjectID id, const std::string& key) {
        // First let's get the ref ID from objects for this object ID.
        auto refid = getRefID(id);
        return getRealRef(refid, key);
    }

    double Type::getRealProto(ObjectID proto, const std::string& key) {
        // First let's get the ref ID from objects for this object ID.
        auto refid = getRefIDProto(proto);
        return getRealRef(refid, key);
    }

    void Type::setStatRef(kaizer::ObjectID ref, const std::string &key, double value) {
        // This works almost exactly like setStringRef but we want to target the refInts table, and the
        // value is stored on that table directly.
        auto lkey = boost::to_lower_copy(key);

        if(value == 0) {
            SQLite::Statement q1(*db, "DELETE FROM refStats WHERE ref=? AND name=?");
            q1.bind(1, ref);
            q1.bind(2, lkey);
            q1.exec();
            return;
        }

        // Otherwise, insert or replace into refInts...
        SQLite::Statement q2(*db, "INSERT OR REPLACE INTO refStats (ref, name, value) VALUES (?, ?, ?)");
        q2.bind(1, ref);
        q2.bind(2, lkey);
        q2.bind(3, value);
        q2.exec();
    }

    void Type::setStat(ObjectID id, const std::string &key, double value) {
        // First let's get the ref ID from objects for this object ID.
        auto refid = getRefID(id);
        setStatRef(refid, key, value);
    }

    void Type::setStatProto(ObjectID proto, const std::string &key, double value) {
        // First let's get the ref ID from objects for this object ID.
        auto refid = getRefIDProto(proto);
        setStatRef(refid, key, value);
    }

    double Type::getStatRef(ObjectID ref, const std::string &key) {
        auto lkey = boost::to_lower_copy(key);

        // Now we can get the string from refStrings for this ref.
        SQLite::Statement q1(*db, "SELECT value FROM refStats WHERE ref=? AND name=?");
        q1.bind(1, ref);
        q1.bind(2, lkey);
        if(q1.executeStep()) {
            return q1.getColumn(0).getDouble();
        }
        return 0;
    }

    double Type::getStat(ObjectID id, const std::string &key) {
        // First let's get the ref ID from objects for this object ID.
        auto refid = getRefID(id);
        return getStatRef(refid, key);
    }

    double Type::getStatProto(ObjectID proto, const std::string &key) {
        // First let's get the ref ID from objects for this object ID.
        auto refid = getRefIDProto(proto);
        return getStatRef(refid, key);
    }


    std::string Type::getName(ObjectID id) {
        auto name = getString(id, "name");
        if(name.empty()) {
            return fmt::format("Unnamed {} {}", getTypeName(), id);
        }
        return name;
    }

    void Type::sendText(kaizer::ObjectID id, const std::string &text) {
        // For this we will need to check the entities table for the ObjectID,
        // and check for a SessionHolder component...
        auto found = entities.find(id);
        if(found != entities.end()) {
            auto sess = registry.try_get<components::SessionHolder>(found->second);
            if(sess) {
                if(text.ends_with("\n"))
                    sess->data->sendText(text);
                else
                    sess->data->sendText(text + "\n");
            }
        }
    }

    OpResult<> Type::moveTo(ObjectID id, CallParameters &param) {
        auto loc = getRelation(id, static_cast<int>(RelationKind::Location));
        if(loc != -1) param.setEntity("origin", loc);
        param.setEntity("mover", id);

        auto [canMove, reas] = atPreMove(id, param);
        if(!canMove) {
            return {false, reas};
        }

        Type* locType = nullptr;

        if(loc != -1) {
            locType = getType(loc);
            auto [canLeave, reason] = locType->atPreObjectLeave(loc, param);
            if(!canLeave) {
                return {false, reason};
            }
        }

        auto dest = param.getEntity("destination");

        if(dest != -1) {
            if (dest == id) {
                return {false, "You cannot move into yourself! The universe might implode!"};
            }
            auto d = dest;
            while(true) {
                auto l = getRelation(d, static_cast<int>(RelationKind::Location));
                if(l == -1) break;
                if(l == id) {
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

        if(loc != -1) {
            locType->atObjectLeave(loc, param);
            if(!quiet) announceMoveFrom(id, param);
            setRelation(id, static_cast<int>(RelationKind::Location), -1);
        }

        if(dest != -1) {
            setRelation(id, static_cast<int>(RelationKind::Location), dest);
            auto destType = getType(dest);
            destType->atObjectReceive(dest, param);
            if(!quiet) announceMoveTo(id, param);
        }

        atPostMove(id, param);
        return {true, std::nullopt};
    }

    OpResult<> Type::atPreMove(kaizer::ObjectID id, kaizer::CallParameters &param) {
        return {true, std::nullopt};
    }

    OpResult<> Type::atPreObjectLeave(kaizer::ObjectID id, kaizer::CallParameters &param) {
        return {true, std::nullopt};
    }

    OpResult<> Type::atPreObjectReceive(kaizer::ObjectID id, kaizer::CallParameters &param) {
        return {true, std::nullopt};
    }

    void Type::atObjectLeave(kaizer::ObjectID id, kaizer::CallParameters &param) {
    }

    void Type::atObjectReceive(kaizer::ObjectID id, kaizer::CallParameters &param) {
    }

    void Type::atPostMove(kaizer::ObjectID id, kaizer::CallParameters &param) {
        auto dest = param.getEntity("destination");
        if(dest != -1) {
            atLook(id, dest);
        }
    }

    void Type::announceMoveFrom(kaizer::ObjectID id, kaizer::CallParameters &param) {
    }

    void Type::announceMoveTo(kaizer::ObjectID id, kaizer::CallParameters &param) {
    }

    void Type::stow(ObjectID id) {
        auto loc = getRelation(id, static_cast<int>(RelationKind::Location));
        if(loc == -1) return;
        setRelation(id, static_cast<int>(RelationKind::Stow), loc);

        auto ent = getEntity(id);

        CallParameters params;
        params.setEntity("mover", id);
        params.setEntity("destination", -1);
        params.setString("moveType", "system");
        params.setBool("force", true);
        auto &pen = registry.get_or_emplace<components::PendingMove>(ent);
        pen.params = params;
        pen.reportTo = id;

    }

    OpResult<ObjectID> Type::getUnstowLocation(kaizer::ObjectID id) {
        auto saved = getRelation(id, static_cast<int>(RelationKind::Stow));
        if(saved != -1) {
            return {saved, "logout"};
        }
        return {config::startingRoom, "startingRoom"};
    }

    OpResult<> Type::unstow(ObjectID id) {
        auto [destination, destType] = getUnstowLocation(id);
        if(destination == -1) {
            spdlog::error("Could not unstow Entity {} to {}", id, destType.value());
            return {false, "Could not unstow character"};
        }

        auto ent = getEntity(id);
        CallParameters params;
        params.setEntity("mover", id);
        params.setEntity("destination", destination);
        params.setString("moveType", "system");
        params.setBool("force", true);
        auto &pen = registry.get_or_emplace<components::PendingMove>(ent);
        pen.params = params;
        pen.reportTo = id;
        return {true, std::nullopt};
    }

    std::string Type::getDisplayName(kaizer::ObjectID id, kaizer::ObjectID looker) {
        return getName(id);
    }

    std::string Type::getRoomLine(kaizer::ObjectID id, kaizer::ObjectID looker) {
        return getDisplayName(id, looker);
    }

    std::string Type::renderAppearance(kaizer::ObjectID id, kaizer::ObjectID looker) {
        std::vector<std::string> lines;
        lines.emplace_back(getDisplayName(id, looker));
        auto ldesc = getString(id, "lookDescription");
        if(!ldesc.empty()) {
            lines.emplace_back(ldesc);
        }
        auto con = getReverseRelation(id, static_cast<int>(RelationKind::Location));
        if(!con.empty()) {
            for(auto &e : con) {
                if(e == looker) continue;
                if(lines.empty()) {
                    lines.emplace_back("Contents:");
                }
                auto t = getType(e);
                lines.emplace_back(fmt::format("  {}", t->getRoomLine(e, looker)));
            }
        }
        return boost::algorithm::join(lines, "\n");
    }

    void Type::atLook(kaizer::ObjectID id, kaizer::ObjectID target) {
        if(target == -1) {
            sendText(id, "You don't see that here.");
            return;
        }
        auto targetType = getType(target);
        auto found = entities.find(id);
        if(found != entities.end()) {
            if(registry.any_of<components::SessionHolder>(found->second)) {
                // Rendering a complicated appearance can be expensive.
                // We will only do it if ent has a listening session.
                sendText(id, targetType->renderAppearance(target, id));
            }
        }

        targetType->atDesc(target, id);
    }

    void Type::atDesc(kaizer::ObjectID id, kaizer::ObjectID looker) {

    }

    std::set<std::string> Type::getSearchWords(kaizer::ObjectID id, kaizer::ObjectID looker) {
        auto name = stripAnsi(getDisplayName(id, looker));
        std::set<std::string> words;
        boost::split(words, name, boost::algorithm::is_space());
        return words;
    }

    bool Type::checkSearch(kaizer::ObjectID id, const std::string &term, kaizer::ObjectID looker) {
        auto w = getSearchWords(id, looker);
        std::vector<std::string> words;
        // Sort w into words by string length, smallest first and longest last.
        std::transform(w.begin(), w.end(), std::back_inserter(words), [](const std::string& s) { return s; });
        std::sort(words.begin(), words.end(), [](const std::string& a, const std::string& b) { return a.size() < b.size(); });

        for(auto& word : words) {
            if(boost::istarts_with(word, term)) return true;
        }
        return false;
    }

    bool Type::canDetect(kaizer::ObjectID id, kaizer::ObjectID target, const std::string &sense) {
        return true;
    }

    const std::unordered_map<std::string, Command*>& Type::getCommands() {
        if(commandsCached) return commands;
        commandsCached = true;
        for(auto i = 0; i < cmdMask.size(); i++) {
            auto com = expandedCommandRegistry.find(i);
            if (com == expandedCommandRegistry.end())
                continue;
            for (auto &[key, cmd]: com->second) {
                commands[key] = cmd.get();
            }
        }
        return commands;
    }

    const std::vector<std::pair<std::string, Command*>>& Type::getSortedCommands() {
        if(sortedCommandsCached) return sortedCommands;

        sortedCommandsCached = true;
        sortedCommands.reserve(commands.size());
        for(auto& [key, cmd] : getCommands()) {
            sortedCommands.emplace_back(key, cmd);
        }

        std::sort(sortedCommands.begin(), sortedCommands.end(), [](const auto& a, const auto& b) {
            return a.second->getPriority() < b.second->getPriority();
        });

        return sortedCommands;
    }

    // This is the global type registry.
    std::unordered_map<std::string , std::shared_ptr<Type>> typeRegistry;



    void registerType(const std::shared_ptr<Type>& entry) {
        typeRegistry[entry->getTypeName()] = entry;
    }

}