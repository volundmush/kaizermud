#pragma once

#include "kaizermud/base.h"
#include "SQLiteCpp/SQLiteCpp.h"

namespace kaizermud::db {
    extern std::unordered_map<std::string, std::shared_ptr<SQLite::Database>> extraDB;

    extern std::vector<std::string> schema;

    extern std::vector<std::function<void(const std::shared_ptr<SQLite::Database>&)>> preLoadFuncs, postLoadFuncs, preSaveFuncs, postSaveFuncs;

    void runQuery(const std::shared_ptr<SQLite::Database>& db, std::string_view query);

    void createSchema(const std::shared_ptr<SQLite::Database>& db);

    void saveToDB(entt::entity ent, const std::shared_ptr<SQLite::Database>& db);

    void loadFromDB(entt::entity ent, const std::shared_ptr<SQLite::Database>& db);

    void loadDatabase(std::string_view path);

    void saveDatabase(std::string_view path);

}