#pragma once
#include "kaizermud/base.h"


namespace kaizermud::game {

    class CommandHandler;
    class Command {
    public:
        Command(CommandHandler *handler);
        virtual OpResult<> canExecute();
        virtual void execute();
        virtual void parse(const std::string& input);
        virtual int priority();
        virtual bool isAvailable();
        virtual std::string getHelp();
        virtual void reset();
        virtual bool match(const std::string& input);

    protected:
        CommandHandler *handler;
        std::unordered_map<std::string, std::string> args;
    };

    struct CommandEntry {
        std::string objType, cmdName;
        std::function<std::unique_ptr<Command>(CommandHandler*)> ctor;
    };

    extern std::unordered_map<std::string, std::unordered_map<std::string, CommandEntry>> commandRegistry;
    OpResult<> registerCommand(CommandEntry entry);

    class CommandHandler {
    public:

        std::vector<std::unique_ptr<Command>> commands;
        bool execute(const std::string& input);
        void load();
    protected:
        bool loaded;
    };
}