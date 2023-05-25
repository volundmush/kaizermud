#pragma once
#include "kaizermud/base.h"


namespace kaizermud::game {

    class CommandHandler;
    class Command {
    public:
        Command(CommandHandler *handler);
        virtual OpResult canExecute();
        virtual void execute();
        virtual void parse(std::string_view input);
        virtual int priority();
        virtual bool isAvailable();
        virtual std::string getHelp();
        virtual void reset();
        virtual bool match(std::string_view input);

    protected:
        CommandHandler *handler;
        std::unordered_map<std::string, std::string> args;
    };

    struct CommandEntry {
        std::string objType, cmdName;
        std::function<std::unique_ptr<Command>(CommandHandler*)> ctor;
    };

    extern std::unordered_map<std::string, std::unordered_map<std::string, CommandEntry>> commandRegistry;
    OpResult registerCommand(CommandEntry entry);

    class CommandHandler {
    public:
        CommandHandler(std::shared_ptr<Object> obj);
        std::shared_ptr<Object> obj;
        std::vector<std::unique_ptr<Command>> commands;
        bool execute(std::string_view input);
        void load();
    protected:
        bool loaded;
    };
}