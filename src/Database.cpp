#include "kaizermud/Database.h"
#include <iostream>

namespace kaizermud::db {
    std::unordered_map<std::string, std::shared_ptr<SQLite::Database>> extraDB;
    std::vector<std::function<void(const std::shared_ptr<SQLite::Database>&)>> preLoadFuncs, postLoadFuncs, preSaveFuncs, postSaveFuncs;

    std::vector<std::string> schema = {
            "CREATE TABLE IF NOT EXISTS objects (\n"
            "    id INTEGER PRIMARY KEY,\n"
            "    mainType TEXT NOT NULL,\n"
            "    subType TEXT NOT NULL\n"
            ");",

            "CREATE TABLE IF NOT EXISTS objectStrings (\n"
            "    id INTEGER PRIMARY KEY,\n"
            "    objectId INTEGER NOT NULL,\n"
            "    key TEXT NOT NULL,\n"
            "    value TEXT NOT NULL,\n"
            "    UNIQUE(objectId, key)\n"
            ");",

            "CREATE TABLE IF NOT EXISTS objectRelations (\n"
            "    id INTEGER PRIMARY KEY,\n"
            "    objectId INTEGER NOT NULL,\n"
            "    relationType TEXT NOT NULL,\n"
            "    relationId INTEGER NOT NULL,\n"
            "    UNIQUE(objectId, relationType, relationId)\n"
            ");",

            "CREATE TABLE IF NOT EXISTS objectStats (\n"
            "    id INTEGER PRIMARY KEY,\n"
            "    objectId INTEGER NOT NULL,\n"
            "    statType TEXT NOT NULL,\n"
            "    value REAL NOT NULL,\n"
            "    UNIQUE(objectId, statType)\n"
            ");",

            "CREATE TABLE IF NOT EXISTS objectEquip (\n"
            "    id INTEGER PRIMARY KEY,\n"
            "    objectId INTEGER NOT NULL,\n"
            "    slot TEXT NOT NULL,\n"
            "    itemId INTEGER NOT NULL,\n"
            "    UNIQUE(objectId, slot)\n"
            ");"

            "CREATE TABLE IF NOT EXISTS objectAspect ("
            "   id INTEGER PRIMARY KEY,"
            "   objectId INTEGER NOT NULL,"
            "   aspectType TEXT NOT NULL,"
            "   aspectKey TEXT NOT NULL,"
            "   UNIQUE(objectId, aspectType)"
            ");"

    };

    void runQuery(const std::shared_ptr<SQLite::Database>& db, std::string_view query) {
        try {
            db->exec(query.data());
        }
        catch (const std::exception& e) {
            std::cerr << "Error executing query: " << e.what() << std::endl;
            std::cerr << "For statement: " << query << std::endl;
            exit(1);
        }
    }

    void createSchema(const std::shared_ptr<SQLite::Database>& db) {
        for (const auto& query: schema) {
            runQuery(db, query);
        }
    }

    void loadDatabase(std::string_view path) {
        auto loadDb = std::make_shared<SQLite::Database>(path.data(), SQLite::OPEN_READONLY);

        for (const auto& func: preLoadFuncs) {
            func(loadDb);
        }

        SQLite::Statement stmt(*loadDb, "SELECT * FROM objects");
        while (stmt.executeStep()) {
            std::optional<ObjectID> id = stmt.getColumn(0).getInt();
            auto mainType = stmt.getColumn(1).getText();
            auto subType = stmt.getColumn(2).getText();
            auto [obj, err] = createEntity(mainType, subType, id);
        }

        // copy this just in case the load process alters it...
        auto copyObjects = entities;

        for(const auto& [id, obj]: copyObjects) {
            //apiobj->loadFromDB(loadDb);
        }

        for (const auto& func: postLoadFuncs) {
            func(loadDb);
        }
    }

    void saveDatabase(std::string_view path) {
        auto saveDb = std::make_shared<SQLite::Database>(path.data(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

        SQLite::Transaction transaction(*saveDb);

        createSchema(saveDb);

        for (const auto& func: preSaveFuncs) {
            func(saveDb);
        }

        for (const auto& [id, ent]: entities) {
            //obj->saveToDB(saveDb);
        }

        for (const auto& func: postSaveFuncs) {
            func(saveDb);
        }

        transaction.commit();
    }
}