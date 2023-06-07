#pragma once
#include "kaizermud/base.h"

namespace kaizer::config {
    // Save Options
    extern std::string dbFolder;
    extern std::string dbName;
    extern std::string dbExt;

    extern double saveInterval;


    // Game Options
    extern ObjectID startingRoom;

}