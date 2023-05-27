#pragma once
#include <functional>
#include "kaizermud/base.h"
#include "kaizermud/Components.h"


namespace kaizermud::api {

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
            for(const auto &type : {info.types.second, info.types.first}) {
                auto it = overrides.find(std::string(type));
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

    // Strings
    extern ApiCall<OpResult<>, std::string_view, std::string_view> setString;
    extern ApiCall<OpResult<>, std::string_view> clearString;
    extern ApiCall<std::optional<std::string_view>, std::string_view> getString;

    OpResult<> defaultSetString(entt::entity ent, std::string_view key, std::string_view value);
    OpResult<> defaultClearString(entt::entity ent, std::string_view key);
    std::optional<std::string_view> defaultGetString(entt::entity ent, std::string_view key);

    // Relations
    extern ApiCall<OpResult<>, std::string_view, entt::entity> setRelation;
    extern ApiCall<OpResult<>, std::string_view> clearRelation;
    extern ApiCall<entt::entity, std::string_view> getRelation;
    extern ApiCall<std::optional<std::reference_wrapper<const std::vector<entt::entity>>>, std::string_view> getReverseRelation;

    OpResult<> defaultClearRelation(entt::entity ent, std::string_view name);
    entt::entity defaultGetRelation(entt::entity ent, std::string_view name);
    std::optional<std::reference_wrapper<const std::vector<entt::entity>>> defaultGetReverseRelation(entt::entity ent, std::string_view name);
}