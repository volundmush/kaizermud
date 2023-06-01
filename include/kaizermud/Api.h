#pragma once
#include <functional>
#include "kaizermud/base.h"
#include "kaizermud/Components.h"
#include "kaizermud/Types.h"


namespace kaizer {

    // Handy-ass template for holding onto and executing API calls by specificity.
    template <typename ResultType, typename... Args>
    class ApiCall {
    public:
        using FunctionType = std::function<ResultType(entt::entity, Args...)>;

    private:
        FunctionType baseCase;
        std::unordered_map<std::string, FunctionType> overrides;

    public:
        ApiCall() = default;
        explicit ApiCall(FunctionType baseCaseFunc) : baseCase(std::move(baseCaseFunc)) {}

        void setOverride(std::string objType, FunctionType func) {
            overrides[std::move(objType)] = std::move(func);
        }

        ResultType execute(entt::entity e, Args... args) {
            auto &info = registry.get<components::ObjectInfo>(e);
            // check by specific type, then by general type...
            for(const auto &type : info.sortedTypes) {
                auto it = overrides.find(std::string(type->getKey()));
                if(it != overrides.end()) {
                    return it->second(e, args...);
                }
            }
            return baseCase(e, args...);
        }

        ResultType operator()(entt::entity e, Args... args) {
            return execute(e, std::forward<Args>(args)...);
        }
    };

    // Administration
    extern ApiCall<void> atCreate;
    void defaultAtCreate(entt::entity ent);

    extern ApiCall<Type*, std::string_view> getType;
    Type* defaultGetType(entt::entity ent, std::string_view name);

    extern ApiCall<OpResult<>, std::string_view> addType;
    OpResult<> defaultAddType(entt::entity ent, std::string_view key);

    extern ApiCall<OpResult<>, std::string_view> removeType;
    OpResult<> defaultRemoveType(entt::entity ent, std::string_view key);

    extern ApiCall<int16_t> getAdminLevel;
    int16_t defaultGetAdminLevel(entt::entity ent);

    // Info
    extern ApiCall<ObjectID> getID;
    ObjectID defaultGetID(entt::entity ent);

    // Strings
    extern ApiCall<OpResult<>, std::string_view, std::string_view> setString;
    extern ApiCall<OpResult<>, std::string_view> clearString;
    extern ApiCall<std::optional<std::string_view>, std::string_view> getString;

    OpResult<> defaultSetString(entt::entity ent, std::string_view, std::string_view);
    OpResult<> defaultClearString(entt::entity ent, std::string_view);
    std::optional<std::string_view> defaultGetString(entt::entity ent, std::string_view);

    // Relations
    extern ApiCall<OpResult<>, std::string_view, entt::entity> setRelation;
    extern ApiCall<OpResult<>, std::string_view> clearRelation;
    extern ApiCall<entt::entity, std::string_view> getRelation;
    extern ApiCall<std::optional<std::reference_wrapper<const std::vector<entt::entity>>>, std::string_view> getReverseRelation;

    OpResult<> defaultClearRelation(entt::entity ent, std::string_view name);
    entt::entity defaultGetRelation(entt::entity ent, std::string_view name);
    std::optional<std::reference_wrapper<const std::vector<entt::entity>>> defaultGetReverseRelation(entt::entity ent, std::string_view name);

    // Aspects
    extern ApiCall<OpResult<>, std::string_view, std::string_view> setAspect;
    extern ApiCall<Aspect*, std::string_view> getAspect;
    Aspect* defaultGetAspect(entt::entity ent, std::string_view name);

    extern ApiCall<OpResult<>, Aspect*> setAspectPointer;
    OpResult<> defaultSetAspectPointer(entt::entity ent, Aspect* asp);

    OpResult<> defaultSetAspect(entt::entity ent, std::string_view name, std::string_view value);

    extern ApiCall<std::set<std::string>> getAspectSlots;
    std::set<std::string> defaultGetAspectSlots(entt::entity ent);

    // Commands
    extern ApiCall<std::vector<std::pair<std::string, Command*>>> getSortedCommands;
    std::vector<std::pair<std::string, Command*>> defaultGetSortedCommands(entt::entity ent);

    extern ApiCall<std::unordered_map<std::string, Command*>> getCommands;
    std::unordered_map<std::string, Command*> defaultGetCommands(entt::entity ent);

    // Communication
    extern ApiCall<OpResult<>, std::string_view> sendText;
    OpResult<> defaultSendText(entt::entity ent, std::string_view text);

}