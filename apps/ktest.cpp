#include <iostream>
#include <string>

#include "kaizermud/game.h"


int main(int argc, char* argv[]) {
    std::cout << "starting program..." << std::endl;
    kaizermud::game::Task task;

    // Load the Lua code
    const char* lua_code = R"(
        counter = 0
        function run()
            while true do
                _G.counter = _G.counter + 1
            end
        end
    )";

    kaizermud::game::LuaCode code(lua_code);
    code.compile();

    if (code.state == kaizermud::game::LuaCode::State::ERROR) {
        std::cerr << "Lua Compilation error: " << code.bytecode << std::endl;
        return 1;
    }

    if(task.load("test_script", code)) {
        std::cout << "Loaded Lua code" << std::endl;
    } else {
        std::cerr << "Error: " << task.err << std::endl;
        return 1;
    }

    // Call task.run() up to 5 times
    int counter = 0;
    bool running = true;
    for (int i = 0; i < 5; ++i) {
        if(!running) break;
        task.run();

        switch (task.state) {
            case kaizermud::game::Task::State::RUNNING:
            case kaizermud::game::Task::State::WAITING:
            case kaizermud::game::Task::State::INTERRUPTED:
                // Get the value of "counter" from the Lua state
                lua_getglobal(task.L, "counter");
                counter = lua_tointeger(task.L, -1);
                lua_pop(task.L, 1);
                std::cout << "Task " << i + 1 << ": Counter = " << counter << std::endl;
                break;
            case kaizermud::game::Task::State::FINISHED:
                std::cout << "Task " << i + 1 << ": Finished" << std::endl;
                running = false;
                break;
            case kaizermud::game::Task::State::ERROR:
                std::cerr << "Task " << i + 1 << ": Error: " << task.err << std::endl;
                running = false;
                break;
            default:
                running = false;
                std::cerr << "Task " << i + 1 << ": Unknown state" << std::endl;
                break;
        }
    }

    return 0;
}