#pragma once

#include "kaizermud/base.h"
#include "SQLiteCpp/SQLiteCpp.h"

namespace kaizermud::db {
    extern std::unordered_map<std::string, std::shared_ptr<SQLite::Database>> extraDB;

    extern std::vector<std::string> schema;

    extern std::vector<std::function<void(const std::shared_ptr<SQLite::Database>&)>> preLoadFuncs, postLoadFuncs, preSaveFuncs, postSaveFuncs;

    void runQuery(const std::shared_ptr<SQLite::Database>& db, const std::string& query);

    void createSchema(const std::shared_ptr<SQLite::Database>& db);

    void loadDatabase(const std::string& path);

    void saveDatabase(const std::string& path);

}