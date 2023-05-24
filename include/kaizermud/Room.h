#pragma once

#include "kaizermud/Object.h"

namespace kaizermud::game {
    class Room : public Object {
    public:
        [[nodiscard]] std::string_view getType() const override;
    };

}