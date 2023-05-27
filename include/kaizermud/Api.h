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
    extern ApiCall<OpResult<>, const std::string&, const std::string&> setString;
    extern ApiCall<OpResult<>, const std::string&> clearString;
    extern ApiCall<std::optional<std::string>, const std::string&> getString;

    OpResult<> defaultSetString(entt::entity ent, const std::string&, const std::string&);
    OpResult<> defaultClearString(entt::entity ent, const std::string&);
    std::optional<std::string> defaultGetString(entt::entity ent, const std::string&);

    // Relations
    extern ApiCall<OpResult<>, const std::string&, entt::entity> setRelation;
    extern ApiCall<OpResult<>, const std::string&> clearRelation;
    extern ApiCall<entt::entity, const std::string&> getRelation;
    extern ApiCall<std::optional<std::reference_wrapper<const std::vector<entt::entity>>>, const std::string&> getReverseRelation;

    OpResult<> defaultClearRelation(entt::entity ent, const std::string& name);
    entt::entity defaultGetRelation(entt::entity ent, const std::string& name);
    std::optional<std::reference_wrapper<const std::vector<entt::entity>>> defaultGetReverseRelation(entt::entity ent, const std::string& name);
}