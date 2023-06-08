#pragma once
#include "kaizermud/Commands.h"

namespace kaizer::base {

    struct BldCmd : Command {
        [[nodiscard]] uint8_t getType() override {return 0;};
    };

}