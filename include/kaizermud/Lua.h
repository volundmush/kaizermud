#pragma once
#include "kaizermud/base.h"
#include <set>
#include <optional>
#include <string>
#include <map>
#include <unordered_map>
#include <memory>
#include <chrono>
#include <boost/asio/experimental/channel.hpp>
#include "kaizermud/thermite.h"
#include "kaizermud/ClientConnection.h"
#include "lualib.h"
#include "luacode.h"
#include "LuaBridge/LuaBridge.h"

namespace kaizermud::game {
    extern lua_CompileOptions compile_options;

    struct LuaCode {
        explicit LuaCode(const std::string &code) :code(code) {};
        ~LuaCode() {
            if (bytecode) free(bytecode);
        };

        enum class State : uint8_t {
            UNCOMPILED,
            COMPILED,
            ERROR
        };

        std::string code{};
        char *bytecode{nullptr};
        size_t length{0};
        State state{State::UNCOMPILED};

        void compile();
    };
}