#include "kaizermud/Database.h"
#include "kaizermud/Components.h"
#include "kaizermud/Prototypes.h"
#include "kaizermud/Types.h"
#include "kaizermud/utils.h"
#include <iostream>
#include "spdlog/spdlog.h"
#include "kaizermud/game.h"
#include "kaizermud/LoadMessages.h"
#include "kaizermud/Config.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <regex>
#include <exception>

namespace kaizer {
    std::shared_ptr<SQLite::Database> db;
    std::vector<std::function<void()>> loadFuncs;

    std::vector<std::string> schema = {
            "CREATE TABLE IF NOT EXISTS refs ("
            "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "    class TEXT NOT NULL"
            ");",

            "CREATE TABLE IF NOT EXISTS strings ("
            "   id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "   data TEXT NOT NULL UNIQUE,"
            "   clean TEXT NOT NULL COLLATE NOCASE"
            ");",

            "CREATE TABLE IF NOT EXISTS refStrings ("
            "   id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "   ref INTEGER NOT NULL,"
            "   name TEXT NOT NULL COLLATE NOCASE,"
            "   string INTEGER NOT NULL,"
            "   FOREIGN KEY(ref) REFERENCES refs(id) ON UPDATE CASCADE ON DELETE CASCADE,"
            "   FOREIGN KEY(string) REFERENCES strings(id) ON UPDATE CASCADE ON DELETE RESTRICT,"
            "   UNIQUE(ref, name)"
            ");",

            "CREATE TABLE IF NOT EXISTS refInts ("
            "   id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "   ref INTEGER NOT NULL,"
            "   name TEXT NOT NULL COLLATE NOCASE,"
            "   value INTEGER NOT NULL DEFAULT 0,"
            "   FOREIGN KEY(ref) REFERENCES refs(id) ON UPDATE CASCADE ON DELETE CASCADE,"
            "   UNIQUE(ref, name)"
            ");",

            "CREATE TABLE IF NOT EXISTS refReals ("
            "   id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "   ref INTEGER NOT NULL,"
            "   name TEXT NOT NULL COLLATE NOCASE,"
            "   value REAL NOT NULL DEFAULT 0,"
            "   FOREIGN KEY(ref) REFERENCES refs(id) ON UPDATE CASCADE ON DELETE CASCADE,"
            "   UNIQUE(ref, name)"
            ");",

            "CREATE TABLE IF NOT EXISTS refStats ("
            "   id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "   ref INTEGER NOT NULL,"
            "   name TEXT NOT NULL COLLATE NOCASE,"
            "   value REAL NOT NULL DEFAULT 0,"
            "   FOREIGN KEY(ref) REFERENCES refs(id) ON UPDATE CASCADE ON DELETE CASCADE,"
            "   UNIQUE(ref, name)"
            ");",

            "CREATE TABLE IF NOT EXISTS objects ("
            "   id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "   ref INTEGER NOT NULL,"
            "   created INTEGER NOT NULL DEFAULT (strftime('%s','now')),"
            "   FOREIGN KEY(ref) REFERENCES refs(id) ON UPDATE CASCADE ON DELETE RESTRICT"
            ");",

            "CREATE TABLE IF NOT EXISTS relations ("
            "   object INTEGER NOT NULL,"
            "   relationKind INTEGER NOT NULL,"
            "   relation INTEGER NOT NULL,"
            "   relationData TEXT NOT NULL DEFAULT '',"
            "   PRIMARY KEY(object, relationKind),"
            "   FOREIGN KEY(object) REFERENCES objects(id) ON UPDATE CASCADE ON DELETE CASCADE,"
            "   FOREIGN KEY(relation) REFERENCES objects(id) ON UPDATE CASCADE ON DELETE CASCADE"
            ");",

            "CREATE TABLE IF NOT EXISTS prototypes ("
            "   id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "   name TEXT NOT NULL UNIQUE COLLATE NOCASE,"
            "   ref INTEGER NOT NULL,"
            "   FOREIGN KEY(ref) REFERENCES refs(id) ON UPDATE CASCADE ON DELETE RESTRICT"
            ");",

            "CREATE TABLE IF NOT EXISTS accounts ("
            "   id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "   username TEXT NOT NULL UNIQUE COLLATE NOCASE,"
            "   password TEXT NOT NULL DEFAULT '',"
            "   email TEXT NOT NULL DEFAULT '',"
            "   created INTEGER NOT NULL DEFAULT (strftime('%s','now')),"
            "   lastLogin INTEGER NOT NULL DEFAULT (strftime('%s','now')),"
            "   lastLogout INTEGER NOT NULL DEFAULT (strftime('%s','now')),"
            "   lastPasswordChanged INTEGER NOT NULL DEFAULT (strftime('%s','now')),"
            "   totalPlayTime REAL NOT NULL DEFAULT 0,"
            "   totalLoginTime REAL NOT NULL DEFAULT 0,"
            "   disabledReason TEXT NOT NULL DEFAULT '',"
            "   disabledUntil INTEGER NOT NULL DEFAULT 0,"
            "   adminLevel INTEGER NOT NULL DEFAULT 0"
            ");",

            "CREATE TABLE IF NOT EXISTS playerCharacters ("
            "   character INTEGER NOT NULL PRIMARY KEY,"
            "   account INTEGER NOT NULL,"
            "   lastLogin INTEGER NOT NULL DEFAULT (strftime('%s','now')),"
            "   lastLogout INTEGER NOT NULL DEFAULT (strftime('%s','now')),"
            "   totalPlayTime REAL NOT NULL DEFAULT 0,"
            "   FOREIGN KEY(account) REFERENCES accounts(id) ON UPDATE CASCADE ON DELETE CASCADE,"
            "   FOREIGN KEY(character) REFERENCES objects(id) ON UPDATE CASCADE ON DELETE CASCADE,"
            "   UNIQUE(account, character)"
            ");",

            "DROP VIEW IF EXISTS exits;",

            "CREATE VIEW exits as SELECT o.id as exitID,o.ref as ref, r1.relation AS location,r2.relation AS destination,IFNULL(i.value,0) as direction FROM relations as r1 LEFT JOIN objects AS o ON o.id=r1.object LEFT JOIN relations as r2 ON r1.object=r2.object AND r2.relationKind=2 LEFT JOIN refs as r3 ON o.ref=r3.id LEFT JOIN refInts AS i on r3.id=i.ref AND i.name='direction' WHERE r1.relationKind=1 AND r3.class='exit';"

    };

    void runQuery(std::string_view query) {
        try {
            db->exec(query.data());
        }
        catch (const std::exception& e) {
            std::cerr << "Error executing query: " << e.what() << std::endl;
            std::cerr << "For statement: " << query << std::endl;
            exit(1);
        }
    }

    void createSchema() {
        SQLite::Transaction transaction(*db);
        for (const auto& query: schema) {
            runQuery(query);
        }
        transaction.commit();
    }

    void readyDatabase() {
        namespace fs = std::filesystem;
        auto path = fs::current_path() / config::dbFolder;
        if (!fs::exists(path)) {
            fs::create_directory(path);
        }
        path /= (config::dbName + "." + config::dbExt);
        db = std::make_shared<SQLite::Database>(path, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        SQLite::Statement fk(*db, "PRAGMA foreign_keys = ON;");
        fk.exec();

        createSchema();

        for(auto f : loadFuncs) {
            f();
        }
    }

    Type* getType(const std::string& name) {
        auto found = typeRegistry.find(name);
        if (found == typeRegistry.end()) {
            // SERIOUS ERROR! We don't have a type registered for this name!
            // Runtime exception!
            spdlog::error("Invalid type {}", name);
            // Throw an exception!
            throw std::runtime_error("Invalid type " + name);
        }
        return found->second.get();
    }

    Type* getType(ObjectID id) {
        // If ObjectID exists, retrieves the appropriate Type and returns it.
        // We must query objects and refs to get the class name...
        SQLite::Statement q(*db, "SELECT refs.class FROM objects INNER JOIN refs ON objects.ref = refs.id WHERE objects.id = ?");
        q.bind(1, id);
        if (q.executeStep()) {
            auto className = q.getColumn(0).getText();
            return getType(className);
        } else {
            // This should NOT HAPPEN.
            throw std::runtime_error("Invalid object ID in getType: " + std::to_string(id));
        }
    }

    Type* getType(entt::entity ent) {
        auto objinfo = registry.try_get<components::ObjectInfo>(ent);
        if(objinfo) return objinfo->type;
        throw std::runtime_error("Invalid entity in getType");
    }

    entt::entity getEntity(ObjectID id) {
        auto found = entities.find(id);
        if (found != entities.end()) {
            return found->second;
        }
        // Let's query for the entity... we want the object id, ref id, and class name.
        SQLite::Statement q(*db, "SELECT refs.id, refs.class FROM objects INNER JOIN refs ON objects.ref = refs.id WHERE objects.id = ?");
        q.bind(1, id);
        if (q.executeStep()) {
            auto refID = q.getColumn(0).getInt64();
            auto className = q.getColumn(1).getText();

            // Now let's make sure we have a valid type pointer...
            auto type = getType(className);

            auto entity = registry.create();
            auto &objinfo = registry.emplace<components::ObjectInfo>(entity);
            objinfo.id = id;
            objinfo.ref = refID;
            objinfo.type = type;
            entities[id] = entity;
            return entity;
        } else {
            // this is a serious error, we tried to get an ID that didn't exist!
            throw std::runtime_error("Tried to get entity for object " + std::to_string(id) + " but it doesn't exist!");
        }
    }
}