#include "kaizermud/Components.h"
#include "kaizermud/utils.h"

namespace kaizer::components {

    void ObjectInfo::doSort() {
        sortedTypes.clear();
        sortedTypes.reserve(types.size());
        for (auto& [key, type] : types) {
            sortedTypes.push_back(type);
        }
        std::sort(sortedTypes.begin(), sortedTypes.end(), [](Type* a, Type* b) {
            return a->getSortPriority() < b->getSortPriority();
        });
    }

    OpResult<> Account::setPassword(std::string_view newPassword) {
        auto [res, err] = hashPassword(newPassword);
        if(!res) {
            return {false, err};
        }

        password = err.value();
        return {true, std::nullopt};
    }

    OpResult<> Account::checkPassword(std::string_view check) {
        return ::kaizer::checkPassword(password, check);
    }



}