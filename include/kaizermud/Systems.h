#pragma once

#include <boost/asio/awaitable.hpp>
#include "kaizermud/base.h"

namespace kaizer {

        class System {
        public:
            virtual std::string_view getName() = 0;
            virtual int getPriority() = 0;
            virtual boost::asio::awaitable<bool> shouldRun(double deltaTime);
            virtual boost::asio::awaitable<void> run(double deltaTime);
        };

        extern std::vector<std::shared_ptr<System>> sortedSystems;
        extern std::unordered_map<std::string, std::shared_ptr<System>> systemRegistry;
        void registerSystem(std::shared_ptr<System> system);
        void sortSystems();

        class ProcessConnections : public System {
        public:
            std::string_view getName() override {return "ProcessConnections";};
            int getPriority() override {return -10000;};
            boost::asio::awaitable<void> run(double deltaTime) override;
        };

        class ProcessSessions : public System {
        public:
            std::string_view getName() override {return "ProcessSessions";};
            int getPriority() override {return -9000;};
            boost::asio::awaitable<void> run(double deltaTime) override;
        };

        class ProcessOutput : public System {
        public:
            std::string_view getName() override {return "ProcessOutput";};
            int getPriority() override {return 10000;};
            boost::asio::awaitable<void> run(double deltaTime) override;
        };

        class ProcessCommands : public System {
        public:
            std::string_view getName() override {return "ProcessCommands";};
            int getPriority() override {return 1000;};
            boost::asio::awaitable<void> run(double deltaTime) override;
            virtual bool checkHooks(entt::entity ent, std::unordered_map<std::string, std::string>& input);
            virtual bool checkCommands(entt::entity ent, std::unordered_map<std::string, std::string>& input);
            virtual void handleNotFound(entt::entity ent, std::unordered_map<std::string, std::string>& input);
            virtual void handleBadMatch(entt::entity ent, std::unordered_map<std::string, std::string>& input);
        };

        class ProcessMovement : public System {
        public:
            std::string_view getName() override {return "ProcessMovement";};
            int getPriority() override {return 8000;};
            boost::asio::awaitable<void> run(double deltaTime) override;
        };

        class ProcessLook : public System {
        public:
            std::string_view getName() override {return "ProcessLook";};
            int getPriority() override {return 8500;};
            boost::asio::awaitable<void> run(double deltaTime) override;
        };

        void registerBaseSystems();

}