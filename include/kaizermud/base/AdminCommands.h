#pragma once

#include "kaizermud/Commands.h"

namespace kaizer::base {


    struct AdmCmd : Command {
        [[nodiscard]] std::string_view getType() override {return "object";};
    };

}