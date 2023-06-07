#include "kaizermud/utils.h"
#include "sodium.h"

namespace kaizer {

    std::random_device randomDevice;
    std::default_random_engine randomEngine(randomDevice());

    std::unordered_set<std::string> stringPool;

    std::string_view intern(const std::string& str) {
        auto pair = stringPool.insert(str);
        return std::string_view(*pair.first);
    }

    std::string_view intern(std::string_view str) {
        return intern(std::string(str));
    }

    OpResult<> hashPassword(std::string_view password) {
        char hashed_password[crypto_pwhash_STRBYTES];

        if(password.empty()) {
            return {false, "Password cannot be empty"};
        }

        if(crypto_pwhash_str(hashed_password, password.data(), password.size(),
                             crypto_pwhash_OPSLIMIT_INTERACTIVE, crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
            return {false, "Failed to hash password"};
        }

        return {true, std::string(hashed_password)};

    }

    OpResult<> checkPassword(std::string_view hash, std::string_view check) {
        if(crypto_pwhash_str_verify(hash.data(), check.data(), check.size()) != 0) {
            return {false, "Passwords do not match"};
        }
        return {true, std::nullopt};
    }

}