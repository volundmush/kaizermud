#pragma once
#include "kaizermud/Commands.h"

namespace kaizer::base {

    struct BldCmd : Command {
        [[nodiscard]] std::set<uint8_t> getCmdMask() override {return {0};};
    };

}