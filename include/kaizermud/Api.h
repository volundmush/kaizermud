#pragma once
#include <functional>
#include "kaizermud/base.h"
#include "kaizermud/Components.h"
#include "kaizermud/Types.h"
#include "kaizermud/CallParameters.h"


namespace kaizer {

    // Handy-ass template for holding onto and executing API calls by specificity.
    template <typename ResultType, typename... Args>
    class ApiCall {
    public:
        using FunctionType = std::function<ResultType(entt::entity, Args...)>;

    private:
        FunctionType baseCase;
        std::map<uint8_t, FunctionType> overrides;

    public:
        ApiCall() = default;
        explicit ApiCall(FunctionType baseCaseFunc) : baseCase(std::move(baseCaseFunc)) {}

        void setOverride(uint8_t objType, FunctionType func) {
            overrides[objType] = std::move(func);
        }

        ResultType execute(entt::entity e, Args... args) {
            auto &info = registry.get<components::ObjectInfo>(e);
            // check by specific type, then by general type...
            for(auto rit = overrides.rbegin(); rit != overrides.rend(); ++rit) {
                if(info.typeFlags.test(rit->first)) {
                    return rit->second(e, args...);
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

    extern ApiCall<OpResult<>, std::string_view> addType;
    OpResult<> defaultAddType(entt::entity ent, std::string_view key);

    extern ApiCall<OpResult<>, std::string_view> removeType;
    OpResult<> defaultRemoveType(entt::entity ent, std::string_view key);

    extern ApiCall<int16_t> getAdminLevel;
    int16_t defaultGetAdminLevel(entt::entity ent);

    // Info
    extern ApiCall<ObjectID> getID;
    ObjectID defaultGetID(entt::entity ent);

    // Integers
    extern ApiCall<OpResult<>, std::string_view, int64_t> setInteger;
    extern ApiCall<OpResult<>, std::string_view> clearInteger;
    extern ApiCall<std::optional<int64_t>, std::string_view> getInteger;

    OpResult<> defaultSetInteger(entt::entity ent, std::string_view, int64_t);
    OpResult<> defaultClearInteger(entt::entity ent, std::string_view);
    std::optional<int64_t> defaultGetInteger(entt::entity ent, std::string_view);

    // Location
    extern ApiCall<void, entt::entity> setLocation;
    void defaultSetLocation(entt::entity ent, entt::entity loc);

    // Relations
    extern ApiCall<OpResult<>, std::string_view, entt::entity> setRelation;
    extern ApiCall<OpResult<>, std::string_view> clearRelation;
    extern ApiCall<entt::entity, std::string_view> getRelation;
    extern ApiCall<std::optional<std::reference_wrapper<const std::vector<entt::entity>>>, std::string_view> getReverseRelation;

    OpResult<> defaultClearRelation(entt::entity ent, std::string_view name);
    entt::entity defaultGetRelation(entt::entity ent, std::string_view name);
    std::optional<std::reference_wrapper<const std::vector<entt::entity>>> defaultGetReverseRelation(entt::entity ent, std::string_view name);



    // Commands
    extern ApiCall<std::vector<std::pair<std::string, Command*>>> getSortedCommands;
    std::vector<std::pair<std::string, Command*>> defaultGetSortedCommands(entt::entity ent);

    extern ApiCall<std::unordered_map<std::string, Command*>> getCommands;
    std::unordered_map<std::string, Command*> defaultGetCommands(entt::entity ent);

    // Communication
    extern ApiCall<OpResult<>, std::string_view> sendText;
    OpResult<> defaultSendText(entt::entity ent, std::string_view text);

    // Movement
    extern ApiCall<OpResult<>, CallParameters&> moveTo;
    OpResult<> defaultMoveTo(entt::entity ent, CallParameters& params);

    extern ApiCall<OpResult<>, const CallParameters&> atPreMove;
    OpResult<> defaultAtPreMove(entt::entity ent, const CallParameters& param);

    extern ApiCall<OpResult<>, const CallParameters&> atPreObjectLeave;
    OpResult<> defaultAtPreObjectLeave(entt::entity ent, const CallParameters& param);

    extern ApiCall<OpResult<>, const CallParameters&> atPreObjectReceive;
    OpResult<> defaultAtPreObjectReceive(entt::entity ent, const CallParameters& param);

    extern ApiCall<void, const CallParameters&> atObjectLeave;
    void defaultAtObjectLeave(entt::entity ent, const CallParameters& param);

    extern ApiCall<void, const CallParameters&> announceMoveFrom;
    void defaultAnnounceMoveFrom(entt::entity ent, const CallParameters& param);

    extern ApiCall<void, const CallParameters&> announceMoveTo;
    void defaultAnnounceMoveTo(entt::entity ent, const CallParameters& param);

    extern ApiCall<void, const CallParameters&> atObjectReceive;
    void defaultAtObjectReceive(entt::entity ent, const CallParameters& param);

    extern ApiCall<void, const CallParameters&> atPostMove;
    void defaultAtPostMove(entt::entity ent, const CallParameters& param);

    extern ApiCall<void> stowCharacter;
    void defaultStowCharacter(entt::entity ent);

    extern ApiCall<OpResult<entt::entity>> getUnstowLocation;
    OpResult<entt::entity> defaultGetUnstowLocation(entt::entity ent);

    extern ApiCall<OpResult<>> unstowCharacter;
    OpResult<> defaultUnstowCharacter(entt::entity ent);

    // Look / Appearance / Identity
    extern ApiCall<std::string, entt::entity> getDisplayName;
    std::string defaultGetDisplayName(entt::entity ent, entt::entity looker);

    extern ApiCall<std::string, entt::entity> getRoomLine;
    std::string defaultGetRoomLine(entt::entity ent, entt::entity looker);

    extern ApiCall<void, entt::entity> atLook;
    void defaultAtLook(entt::entity ent, entt::entity target);

    extern ApiCall<void, entt::entity> atDesc;
    void defaultAtDesc(entt::entity ent, entt::entity looker);

    extern ApiCall<std::string, entt::entity> renderAppearance;
    std::string defaultRenderAppearance(entt::entity ent, entt::entity looker);

    extern ApiCall<std::set<std::string>, entt::entity> getSearchWords;
    std::set<std::string> defaultGetSearchWords(entt::entity ent, entt::entity looker);

    extern ApiCall<bool, std::string_view, entt::entity> checkSearch;
    bool defaultCheckSearch(entt::entity ent, std::string_view term, entt::entity looker);

    extern ApiCall<bool, entt::entity, std::string_view> canDetect;
    bool defaultCanDetect(entt::entity ent, entt::entity looker, std::string_view sense);


}