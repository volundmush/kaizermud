#pragma once
#include "kaizermud/Object.h"

namespace kaizermud::game {
    class Character : public Object {
    public:
        [[nodiscard]] std::string_view getType() const override;
    };

}