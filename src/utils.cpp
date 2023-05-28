#include "kaizermud/utils.h"

namespace kaizermud::utils {

    std::unordered_set<std::string> stringPool;

    std::string_view intern(const std::string& str) {
        auto pair = stringPool.insert(str);
        return std::string_view(*pair.first);
    }

    std::string_view intern(std::string_view str) {
        return intern(std::string(str));
    }

}