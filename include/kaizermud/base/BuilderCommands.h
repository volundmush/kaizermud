#pragma once
#include "kaizermud/Commands.h"

namespace kaizer::base {

    struct BldCmd : Command {
        [[nodiscard]] std::string_view getType() override {return "object";};
    };

}