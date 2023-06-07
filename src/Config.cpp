#include "kaizermud/Config.h"

namespace kaizer::config {
    // Save Options
    std::string dbFolder = "db";
    std::string dbName = "game";
    std::string dbExt = "sqlite3";
    double saveInterval = 60.0 * 20.0; // 20 minutes

    // Game Options
    ObjectID startingRoom = 1;
}