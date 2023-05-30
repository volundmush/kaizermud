#pragma once
#include "kaizermud/base.h"


namespace kaizer {

    class Command {
    public:
        std::string objType, subType, cmdName;
        virtual OpResult<> canExecute(entt::entity ent);
        virtual void execute(entt::entity ent, std::string_view input);
        virtual int getPriority(entt::entity ent);
        virtual bool isAvailable(entt::entity ent);
        virtual std::string getHelp();
        virtual bool match(entt::entity ent, std::string_view input);
    protected:
        int priority{0};
    };

    extern std::unordered_map<std::string, std::unordered_map<std::string, Command*>> commandRegistry;
    OpResult<> registerCommand(Command* entry);

}