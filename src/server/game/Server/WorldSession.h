/*
 * Copyright (C) 2011-2019 Project SkyFire <http://www.projectskyfire.org/>
 * Copyright (C) 2008-2019 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2019 MaNGOS <https://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/// \addtogroup u2w
/// @{
/// \file

#ifndef __WORLDSESSION_H
#define __WORLDSESSION_H

#include "Common.h"
#include "SharedDefines.h"
#include "AddonMgr.h"
#include "DatabaseEnv.h"
#include "World.h"
#include "WorldPacket.h"
#include "Cryptography/BigNumber.h"

class CalendarEvent;
class CalendarInvite;
class Creature;
class GameObject;
class InstanceSave;
class Item;
class LoginQueryHolder;
class Object;
class Player;
class InstanceSave;
class Quest;
class CharacterHandler;
class SpellCastTargets;
class Unit;
class Warden;
class WorldPacket;
class WorldSocket;
struct AreaTableEntry;
struct AuctionEntry;
struct DeclinedName;
struct ItemTemplate;
struct LfgJoinResultData;
struct LfgLockStatus;
struct LfgPlayerBoot;
struct LfgProposal;
struct LfgReward;
struct LfgRoleCheck;
struct LfgUpdateData;
struct MovementInfo;

enum AccountDataType
{
    GLOBAL_CONFIG_CACHE             = 0,                    // 0x01 g
    PER_CHARACTER_CONFIG_CACHE      = 1,                    // 0x02 p
    GLOBAL_BINDINGS_CACHE           = 2,                    // 0x04 g
    PER_CHARACTER_BINDINGS_CACHE    = 3,                    // 0x08 p
    GLOBAL_MACROS_CACHE             = 4,                    // 0x10 g
    PER_CHARACTER_MACROS_CACHE      = 5,                    // 0x20 p
    PER_CHARACTER_LAYOUT_CACHE      = 6,                    // 0x40 p
    PER_CHARACTER_CHAT_CACHE        = 7,                    // 0x80 p
};

#define NUM_ACCOUNT_DATA_TYPES        8

#define GLOBAL_CACHE_MASK           0x15
#define PER_CHARACTER_CACHE_MASK    0xEA

struct AccountData
{
    AccountData() : Time(0), Data("") {}

    time_t Time;
    std::string Data;
};

enum PartyOperation
{
    PARTY_OP_INVITE = 0,
    PARTY_OP_UNINVITE = 1,
    PARTY_OP_LEAVE = 2,
    PARTY_OP_SWAP = 4
};

enum PartyResult
{
    ERR_PARTY_RESULT_OK                 = 0,
    ERR_BAD_PLAYER_NAME_S               = 1,
    ERR_TARGET_NOT_IN_GROUP_S           = 2,
    ERR_TARGET_NOT_IN_INSTANCE_S        = 3,
    ERR_GROUP_FULL                      = 4,
    ERR_ALREADY_IN_GROUP_S              = 5,
    ERR_NOT_IN_GROUP                    = 6,
    ERR_NOT_LEADER                      = 7,
    ERR_PLAYER_WRONG_FACTION            = 8,
    ERR_IGNORING_YOU_S                  = 9,
    ERR_LFG_PENDING                     = 12,
    ERR_INVITE_RESTRICTED               = 13,
    ERR_GROUP_SWAP_FAILED               = 14,               // if (PartyOperation == PARTY_OP_SWAP) ERR_GROUP_SWAP_FAILED else ERR_INVITE_IN_COMBAT
    ERR_INVITE_UNKNOWN_REALM            = 15,
    ERR_INVITE_NO_PARTY_SERVER          = 16,
    ERR_INVITE_PARTY_BUSY               = 17,
    ERR_PARTY_TARGET_AMBIGUOUS          = 18,
    ERR_PARTY_LFG_INVITE_RAID_LOCKED    = 19,
    ERR_PARTY_LFG_BOOT_LIMIT            = 20,
    ERR_PARTY_LFG_BOOT_COOLDOWN_S       = 21,
    ERR_PARTY_LFG_BOOT_IN_PROGRESS      = 22,
    ERR_PARTY_LFG_BOOT_TOO_FEW_PLAYERS  = 23,
    ERR_PARTY_LFG_BOOT_NOT_ELIGIBLE_S   = 24,
    ERR_RAID_DISALLOWED_BY_LEVEL        = 25,
    ERR_PARTY_LFG_BOOT_IN_COMBAT        = 26,
    ERR_VOTE_KICK_REASON_NEEDED         = 27,
    ERR_PARTY_LFG_BOOT_DUNGEON_COMPLETE = 28,
    ERR_PARTY_LFG_BOOT_LOOT_ROLLS       = 29,
    ERR_PARTY_LFG_TELEPORT_IN_COMBAT    = 30
};

enum ChatRestrictionType
{
    ERR_CHAT_RESTRICTED = 0,
    ERR_CHAT_THROTTLED  = 1,
    ERR_USER_SQUELCHED  = 2,
    ERR_YELL_RESTRICTED = 3
};

enum CharterTypes
{
    GUILD_CHARTER_TYPE                            = 9,
    ARENA_TEAM_CHARTER_2v2_TYPE                   = 2,
    ARENA_TEAM_CHARTER_3v3_TYPE                   = 3,
    ARENA_TEAM_CHARTER_5v5_TYPE                   = 5
};

//class to deal with packet processing
//allows to determine if next packet is safe to be processed
class PacketFilter
{
public:
    explicit PacketFilter(WorldSession* pSession) : m_pSession(pSession) {}
    virtual ~PacketFilter() {}

    virtual bool Process(WorldPacket* /*packet*/) { return true; }
    virtual bool ProcessLogout() const { return true; }

protected:
    WorldSession* const m_pSession;
};
//process only thread-safe packets in Map::Update()
class MapSessionFilter : public PacketFilter
{
public:
    explicit MapSessionFilter(WorldSession* pSession) : PacketFilter(pSession) {}
    ~MapSessionFilter() {}

    virtual bool Process(WorldPacket* packet);
    //in Map::Update() we do not process player logout!
    virtual bool ProcessLogout() const { return false; }
};

//class used to filer only thread-unsafe packets from queue
//in order to update only be used in World::UpdateSessions()
class WorldSessionFilter : public PacketFilter
{
public:
    explicit WorldSessionFilter(WorldSession* pSession) : PacketFilter(pSession) {}
    ~WorldSessionFilter() {}

    virtual bool Process(WorldPacket* packet);
};

// Proxy structure to contain data passed to callback function,
// only to prevent bloating the parameter list
class CharacterCreateInfo
{
    friend class WorldSession;
    friend class Player;

    protected:
        CharacterCreateInfo(std::string name, uint8 race, uint8 cclass, uint8 gender, uint8 skin, uint8 face, uint8 hairStyle, uint8 hairColor, uint8 facialHair, uint8 outfitId,
        WorldPacket& data) : Name(name), Race(race), Class(cclass), Gender(gender), Skin(skin), Face(face), HairStyle(hairStyle), HairColor(hairColor), FacialHair(facialHair),
        OutfitId(outfitId), Data(data), CharCount(0), Stage(0)
        {}

        /// User specified variables
        std::string Name;
        uint8 Race;
        uint8 Class;
        uint8 Gender;
        uint8 Skin;
        uint8 Face;
        uint8 HairStyle;
        uint8 HairColor;
        uint8 FacialHair;
        uint8 OutfitId;
        WorldPacket Data;

        /// Server side data
        uint8 CharCount;

        /// Internal
        uint8 Stage;        // Stage of the callback chain

    private:
        virtual ~CharacterCreateInfo() {};
};

/// Player session in the World
class WorldSession
{
    public:
        WorldSession(uint32 id, WorldSocket*sock, AccountTypes sec, uint8 expansion, time_t mute_time, LocaleConstant locale, uint32 recruiter, bool isARecruiter);
        ~WorldSession();

        bool PlayerLoading() const { return m_playerLoading; }
        bool PlayerLogout() const { return m_playerLogout; }
        bool PlayerLogoutWithSave() const { return m_playerLogout && m_playerSave; }

        void SizeError(WorldPacket const& packet, uint32 size) const;

        void ReadAddonsInfo(WorldPacket& data);
        void SendAddonsInfo();

        void ReadMovementInfo(WorldPacket& data, MovementInfo* mi);
        void WriteMovementInfo(WorldPacket* data, MovementInfo* mi);

        void SendPacket(WorldPacket const* packet);
        void SendNotification(const char *format, ...) ATTR_PRINTF(2, 3);
        void SendNotification(uint32 string_id, ...);
        void SendPetNameInvalid(uint32 error, const std::string& name, DeclinedName *declinedName);
        void SendPartyResult(PartyOperation operation, const std::string& member, PartyResult res, uint32 val = 0);
        void SendAreaTriggerMessage(const char* Text, ...) ATTR_PRINTF(2, 3);
        void SendSetPhaseShift(std::set<uint32> const& phaseIds, std::set<uint32> const& terrainswaps, std::set<uint32> const& worldmaps);
		void SendPhaseShift_Override(uint32 PhaseShift, uint32 MapID);
		void SendPhaseShift_Override_Maps(uint32 PhaseShift, uint32 MapID, uint32 worldmapId);
        void SendQueryTimeResponse();

        void SendAuthResponse(uint8 code, bool shortForm, uint32 queuePos = 0);
        void SendClientCacheVersion(uint32 version);

        AccountTypes GetSecurity() const { return _security; }
        uint32 GetAccountId() const { return _accountId; }
        Player* GetPlayer() const { return _player; }
        char const* GetPlayerName() const;
        uint32 GetGuidLow() const;
        void SetSecurity(AccountTypes security) { _security = security; }
        std::string const& GetRemoteAddress() { return m_Address; }
        void SetPlayer(Player* player);
        uint8 Expansion() const { return m_expansion; }

        void InitWarden(BigNumber* k, std::string os);

        /// Session in auth.queue currently
        void SetInQueue(bool state) { m_inQueue = state; }

        /// Is the user engaged in a log out process?
        bool isLogingOut() const { return _logoutTime || m_playerLogout; }

        /// Engage the logout process for the user
        void LogoutRequest(time_t requestTime)
        {
            _logoutTime = requestTime;
        }

        /// Is logout cooldown expired?
        bool ShouldLogOut(time_t currTime) const
        {
            return (_logoutTime > 0 && currTime >= _logoutTime + 20);
        }

        void LogoutPlayer(bool Save);
        void KickPlayer();
        void HandleMoveToGraveyard(WorldPacket& recvData);

        void QueuePacket(WorldPacket* new_packet);
        bool Update(uint32 diff, PacketFilter& updater);

        /// Handle the authentication waiting queue (to be completed)
        void SendAuthWaitQue(uint32 position);

        //void SendTestCreatureQueryOpcode(uint32 entry, uint64 guid, uint32 testvalue);
        void SendNameQueryOpcode(uint64 guid);

        void SendTrainerList(uint64 guid);
        void SendTrainerList(uint64 guid, const std::string& strTitle);
        void SendListInventory(uint64 guid);
        void SendShowBank(uint64 guid);
        void SendShowReforge(uint64 guid);
        void SendTabardVendorActivate(uint64 guid);
        void SendSpiritResurrect();
        void SendBindPoint(Creature* npc);

        void SendAttackStop(Unit const* enemy);

        void SendBattleGroundList(uint64 guid, BattlegroundTypeId bgTypeId);

        void SendTradeStatus(TradeStatus status);
        void SendUpdateTrade(bool trader_data = true);
        void SendCancelTrade();

        void SendPetitionQueryOpcode(uint64 petitionguid);

        // Spell
        void HandleClientCastFlags(WorldPacket& recvPacket, uint8 castFlags, SpellCastTargets & targets);

        // Pet
        void SendPetNameQuery(uint64 guid, uint32 petnumber);
        void SendStablePet(uint64 guid);
        void SendStablePetCallback(PreparedQueryResult result, uint64 guid);
        void SendStableResult(uint8 guid);
        bool CheckStableMaster(uint64 guid);

        // Account Data
        AccountData* GetAccountData(AccountDataType type) { return &m_accountData[type]; }
        void SetAccountData(AccountDataType type, time_t tm, std::string data);
        void SendAccountDataTimes(uint32 mask);
        void LoadGlobalAccountData();
        void LoadAccountData(PreparedQueryResult result, uint32 mask);

        void LoadTutorialsData();
        void SendTutorialsData();
        void SaveTutorialsData(SQLTransaction& trans);
        uint32 GetTutorialInt(uint8 index) { return m_Tutorials[index]; }
        void SetTutorialInt(uint8 index, uint32 value)
        {
            if (m_Tutorials[index] != value)
            {
                m_Tutorials[index] = value;
                m_TutorialsChanged = true;
            }
        }
        //used with item_page table
        bool SendItemInfo(uint32 itemid, WorldPacket data);
        //auction
        void SendAuctionHello(uint64 guid, Creature* unit);
        void SendAuctionCommandResult(uint32 auctionId, uint32 Action, uint32 ErrorCode, uint32 bidError = 0);
        void SendAuctionBidderNotification(uint32 location, uint32 auctionId, uint64 bidder, uint32 bidSum, uint32 diff, uint32 item_template);
        void SendAuctionOwnerNotification(AuctionEntry* auction);

        //Item Enchantment
        void SendEnchantmentLog(uint64 Target, uint64 Caster, uint32 ItemID, uint32 SpellID);
        void SendItemEnchantTimeUpdate(uint64 Playerguid, uint64 Itemguid, uint32 slot, uint32 Duration);

        //Taxi
        void SendTaxiStatus(uint64 guid);
        void SendTaxiMenu(Creature* unit);
        void SendDoFlight(uint32 mountDisplayId, uint32 path, uint32 pathNode = 0);
        bool SendLearnNewTaxiNode(Creature* unit);
        void SendDiscoverNewTaxiNode(uint32 nodeid);

        // Guild/Arena Team
        void SendArenaTeamCommandResult(uint32 team_action, const std::string& team, const std::string& player, uint32 error_id);
        void SendNotInArenaTeamPacket(uint8 type);
        void SendPetitionShowList(uint64 guid);

        void BuildPartyMemberStatsChangedPacket(Player* player, WorldPacket* data);

        void DoLootRelease(uint64 lguid);

        // Account mute time
        time_t m_muteTime;

        // Locales
        LocaleConstant GetSessionDbcLocale() const { return m_sessionDbcLocale; }
        LocaleConstant GetSessionDbLocaleIndex() const { return m_sessionDbLocaleIndex; }
        const char *GetSkyFireString(int32 entry) const;

        uint32 GetLatency() const { return m_latency; }
        void SetLatency(uint32 latency) { m_latency = latency; }
        uint32 getDialogStatus(Player* player, Object* questgiver, uint32 defstatus);

        ACE_Atomic_Op<ACE_Thread_Mutex, time_t> m_timeOutTime;
        void UpdateTimeOutTime(uint32 diff)
        {
            if (time_t(diff) > m_timeOutTime.value())
                m_timeOutTime = 0;
            else
                m_timeOutTime -= diff;
        }
        void ResetTimeOutTime()
        {
            m_timeOutTime = sWorld->getIntConfig(CONFIG_SOCKET_TIMEOUTTIME);
        }
        bool IsConnectionIdle() const
        {
            return (m_timeOutTime <= 0 && !m_inQueue);
        }

        // Recruit-A-Friend Handling
        uint32 GetRecruiterId() { return recruiterId; }
        bool IsARecruiter() { return isRecruiter; }

    public:                                                 // opcodes handlers

        void Handle_NULL(WorldPacket& recvPacket);          // not used
        void Handle_EarlyProccess(WorldPacket& recvPacket);// just mark packets processed in WorldSocket::OnRead
        void Handle_ServerSide(WorldPacket& recvPacket);    // sever side only, can't be accepted from client
        void Handle_Deprecated(WorldPacket& recvPacket);    // never used anymore by client

        void HandleCharEnumOpcode(WorldPacket& recvPacket);
        void HandleCharDeleteOpcode(WorldPacket& recvPacket);
        void HandleCharCreateOpcode(WorldPacket& recvPacket);
        void HandleCharCreateCallback(PreparedQueryResult result, CharacterCreateInfo* createInfo);
        void HandlePlayerLoginOpcode(WorldPacket& recvPacket);

        void HandleWorldLoginOpcode(WorldPacket& recvPacket);

        void HandleCharEnum(PreparedQueryResult result);
        void HandlePlayerLogin(LoginQueryHolder * holder);
        void HandleCharFactionOrRaceChange(WorldPacket& recvData);

        // played time
        void HandlePlayedTime(WorldPacket& recvPacket);

        // new
        void HandleMoveUnRootAck(WorldPacket& recvPacket);
        void HandleMoveRootAck(WorldPacket& recvPacket);
        void HandleLookingForGroup(WorldPacket& recvPacket);

        // new inspect
        void HandleInspectOpcode(WorldPacket& recvPacket);

        // new party stats
        void HandleInspectHonorStatsOpcode(WorldPacket& recvPacket);

        void HandleMoveWaterWalkAck(WorldPacket& recvPacket);
        void HandleFeatherFallAck(WorldPacket& recvData);

        void HandleMoveHoverAck(WorldPacket& recvData);

        void HandleMountSpecialAnimOpcode(WorldPacket& recvdata);

        // character view
        void HandleShowingHelmOpcode(WorldPacket& recvData);
        void HandleShowingCloakOpcode(WorldPacket& recvData);

        // repair
        void HandleRepairItemOpcode(WorldPacket& recvPacket);

        // Knockback
        void HandleMoveKnockBackAck(WorldPacket& recvPacket);

        void HandleMoveTeleportAck(WorldPacket& recvPacket);
        void HandleForceSpeedChangeAck(WorldPacket& recvData);

        void HandlePingOpcode(WorldPacket& recvPacket);
        void HandleAuthSessionOpcode(WorldPacket& recvPacket);
        void HandleRepopRequestOpcode(WorldPacket& recvPacket);
        void HandleAutostoreLootItemOpcode(WorldPacket& recvPacket);
        void HandleLootMoneyOpcode(WorldPacket& recvPacket);
        void HandleLootOpcode(WorldPacket& recvPacket);
        void HandleLootReleaseOpcode(WorldPacket& recvPacket);
        void HandleLootMasterGiveOpcode(WorldPacket& recvPacket);
        void HandleWhoOpcode(WorldPacket& recvPacket);
        void HandleLogoutRequestOpcode(WorldPacket& recvPacket);
        void HandlePlayerLogoutOpcode(WorldPacket& recvPacket);
        void HandleLogoutCancelOpcode(WorldPacket& recvPacket);

        // GM Ticket opcodes
        void HandleGMTicketCreateOpcode(WorldPacket& recvPacket);
        void HandleGMTicketUpdateOpcode(WorldPacket& recvPacket);
        void HandleGMTicketDeleteOpcode(WorldPacket& recvPacket);
        void HandleGMTicketGetTicketOpcode(WorldPacket& recvPacket);
        void HandleGMTicketSystemStatusOpcode(WorldPacket& recvPacket);
        void HandleGMSurveySubmit(WorldPacket& recvPacket);
        void HandleReportLag(WorldPacket& recvPacket);
        void HandleGMResponseResolve(WorldPacket& recvPacket);

        void HandleTogglePvP(WorldPacket& recvPacket);

        void HandleZoneUpdateOpcode(WorldPacket& recvPacket);
        void HandleSetSelectionOpcode(WorldPacket& recvPacket);
        void HandleStandStateChangeOpcode(WorldPacket& recvPacket);
        void HandleEmoteOpcode(WorldPacket& recvPacket);
        void HandleContactListOpcode(WorldPacket& recvPacket);
        void HandleAddFriendOpcode(WorldPacket& recvPacket);
        void HandleAddFriendOpcodeCallBack(PreparedQueryResult result, std::string friendNote);
        void HandleDelFriendOpcode(WorldPacket& recvPacket);
        void HandleAddIgnoreOpcode(WorldPacket& recvPacket);
        void HandleAddIgnoreOpcodeCallBack(PreparedQueryResult result);
        void HandleDelIgnoreOpcode(WorldPacket& recvPacket);
        void HandleSetContactNotesOpcode(WorldPacket& recvPacket);
        void HandleBugOpcode(WorldPacket& recvPacket);
        void HandleItemNameQueryOpcode(WorldPacket& recvPacket);

        void HandleAreaTriggerOpcode(WorldPacket& recvPacket);

        void HandleSetFactionAtWar(WorldPacket& recvData);
        void HandleSetFactionCheat(WorldPacket& recvData);
        void HandleSetWatchedFactionOpcode(WorldPacket& recvData);
        void HandleSetFactionInactiveOpcode(WorldPacket& recvData);

        void HandleUpdateAccountData(WorldPacket& recvPacket);
        void HandleRequestAccountData(WorldPacket& recvPacket);
        void HandleSetActionButtonOpcode(WorldPacket& recvPacket);

        void HandleGameObjectUseOpcode(WorldPacket& recPacket);
        void HandleGameobjectReportUse(WorldPacket& recvPacket);

        void HandleMeetingStoneInfo(WorldPacket& recPacket);

        void HandleNameQueryOpcode(WorldPacket& recvPacket);

        void HandleQueryTimeOpcode(WorldPacket& recvPacket);

        void HandleCreatureQueryOpcode(WorldPacket& recvPacket);

        void HandleGameObjectQueryOpcode(WorldPacket& recvPacket);

        void HandleMoveWorldportAckOpcode(WorldPacket& recvPacket);
        void HandleMoveWorldportAckOpcode();                // for server-side calls

        void HandleMovementOpcodes(WorldPacket& recvPacket);
        void HandleSetActiveMoverOpcode(WorldPacket& recvData);
        void HandleMoveNotActiveMover(WorldPacket& recvData);
        void HandleDismissControlledVehicle(WorldPacket& recvData);
        void HandleRequestVehicleExit(WorldPacket& recvData);
        void HandleChangeSeatsOnControlledVehicle(WorldPacket& recvData);
        void HandleMoveTimeSkippedOpcode(WorldPacket& recvData);

        void HandleRequestRaidInfoOpcode(WorldPacket& recvData);

        void HandleBattlefieldStatusOpcode(WorldPacket& recvData);
        void HandleBattleMasterHelloOpcode(WorldPacket& recvData);

        void HandleGroupInviteOpcode(WorldPacket& recvPacket);
        //void HandleGroupCancelOpcode(WorldPacket& recvPacket);
        void HandleGroupAcceptOpcode(WorldPacket& recvPacket);
        void HandleGroupDeclineOpcode(WorldPacket& recvPacket);
        void HandleGroupUninviteOpcode(WorldPacket& recvPacket);
        void HandleGroupUninviteGuidOpcode(WorldPacket& recvPacket);
        void HandleGroupSetLeaderOpcode(WorldPacket& recvPacket);
        void HandleGroupDisbandOpcode(WorldPacket& recvPacket);        
        void HandleGroupSetRoles(WorldPacket & recvPacket);
        void HandleOptOutOfLootOpcode(WorldPacket& recvData);
        void HandleLootMethodOpcode(WorldPacket& recvPacket);
        void HandleLootRoll(WorldPacket& recvData);
        void HandleRequestPartyMemberStatsOpcode(WorldPacket& recvData);
        void HandleRaidTargetUpdateOpcode(WorldPacket& recvData);
        void HandleRaidReadyCheckOpcode(WorldPacket& recvData);
        void HandleRaidReadyCheckFinishedOpcode(WorldPacket& recvData);
        void HandleGroupRaidConvertOpcode(WorldPacket& recvData);
        void HandleGroupChangeSubGroupOpcode(WorldPacket& recvData);
        void HandleGroupAssistantLeaderOpcode(WorldPacket& recvData);
        void HandlePartyAssignmentOpcode(WorldPacket& recvData);

        void HandlePetitionBuyOpcode(WorldPacket& recvData);
        void HandlePetitionShowSignOpcode(WorldPacket& recvData);
        void HandlePetitionQueryOpcode(WorldPacket& recvData);
        void HandlePetitionRenameOpcode(WorldPacket& recvData);
        void HandlePetitionSignOpcode(WorldPacket& recvData);
        void HandlePetitionDeclineOpcode(WorldPacket& recvData);
        void HandleOfferPetitionOpcode(WorldPacket& recvData);
        void HandleTurnInPetitionOpcode(WorldPacket& recvData);

        void HandleGuildQueryOpcode(WorldPacket& recvPacket);
        void HandleGuildCreateOpcode(WorldPacket& recvPacket);
        void HandleGuildInviteOpcode(WorldPacket& recvPacket);
        void HandleGuildRemoveOpcode(WorldPacket& recvPacket);
        void HandleGuildAcceptOpcode(WorldPacket& recvPacket);
        void HandleGuildDeclineOpcode(WorldPacket& recvPacket);
        void HandleGuildInfoOpcode(WorldPacket& recvPacket);
        void HandleGuildEventLogQueryOpcode(WorldPacket& recvPacket);
        void HandleGuildRosterOpcode(WorldPacket& recvPacket);
        void HandleGuildPromoteOpcode(WorldPacket& recvPacket);
        void HandleGuildDemoteOpcode(WorldPacket& recvPacket);
        void HandleGuildLeaveOpcode(WorldPacket& recvPacket);
        void HandleGuildDisbandOpcode(WorldPacket& recvPacket);
        void HandleGuildLeaderOpcode(WorldPacket& recvPacket);
        void HandleGuildMOTDOpcode(WorldPacket& recvPacket);
        void HandleGuildNewsUpdateSticky(WorldPacket& recvPacket);
        void HandleGuildQueryNews(WorldPacket& recvPacket);
        void HandleGuildExperienceOpcode(WorldPacket& recvPacket);
        void HandleGuildMaxExperienceOpcode(WorldPacket& recvPacket);
        void HandleGuildRewardsOpcode(WorldPacket& recvPacket);
        void HandleGuildSetNoteOpcode(WorldPacket& recvPacket);
        void HandleGuildRankOpcode(WorldPacket& recvPacket);
        void HandleGuildRanksOpcode(WorldPacket& recvPacket);
        void HandleGuildAddRankOpcode(WorldPacket& recvPacket);
        void HandleGuildDelRankOpcode(WorldPacket& recvPacket);
        void HandleGuildSwitchRankOpcode(WorldPacket& recvPacket);
        void HandleGuildChangeInfoTextOpcode(WorldPacket& recvPacket);
        void HandleSaveGuildEmblemOpcode(WorldPacket& recvPacket);

        void HandleTaxiNodeStatusQueryOpcode(WorldPacket& recvPacket);
        void HandleTaxiQueryAvailableNodes(WorldPacket& recvPacket);
        void HandleActivateTaxiOpcode(WorldPacket& recvPacket);
        void HandleActivateTaxiExpressOpcode(WorldPacket& recvPacket);
        void HandleMoveSplineDoneOpcode(WorldPacket& recvPacket);
        void SendActivateTaxiReply(ActivateTaxiReply reply);

        void HandleTabardVendorActivateOpcode(WorldPacket& recvPacket);
        void HandleBankerActivateOpcode(WorldPacket& recvPacket);
        void HandleBuyBankSlotOpcode(WorldPacket& recvPacket);
        void HandleTrainerListOpcode(WorldPacket& recvPacket);
        void HandleTrainerBuySpellOpcode(WorldPacket& recvPacket);
        void HandlePetitionShowListOpcode(WorldPacket& recvPacket);
        void HandleGossipHelloOpcode(WorldPacket& recvPacket);
        void HandleGossipSelectOptionOpcode(WorldPacket& recvPacket);
        void HandleSpiritHealerActivateOpcode(WorldPacket& recvPacket);
        void HandleNpcTextQueryOpcode(WorldPacket& recvPacket);
        void HandleBinderActivateOpcode(WorldPacket& recvPacket);
        void HandleListStabledPetsOpcode(WorldPacket& recvPacket);
        void HandleStablePet(WorldPacket& recvPacket);
        void HandleStablePetCallback(PreparedQueryResult result);
        void HandleUnstablePet(WorldPacket& recvPacket);
        void HandleUnstablePetCallback(PreparedQueryResult result, uint32 petnumber);
        void HandleBuyStableSlot(WorldPacket& recvPacket);
        void HandleStableRevivePet(WorldPacket& recvPacket);
        void HandleStableSwapPet(WorldPacket& recvPacket);
        void HandleStableSwapPetCallback(PreparedQueryResult result, uint8 petnumber);

        void HandleDuelAcceptedOpcode(WorldPacket& recvPacket);
        void HandleDuelCancelledOpcode(WorldPacket& recvPacket);

        void HandleAcceptTradeOpcode(WorldPacket& recvPacket);
        void HandleBeginTradeOpcode(WorldPacket& recvPacket);
        void HandleBusyTradeOpcode(WorldPacket& recvPacket);
        void HandleCancelTradeOpcode(WorldPacket& recvPacket);
        void HandleClearTradeItemOpcode(WorldPacket& recvPacket);
        void HandleIgnoreTradeOpcode(WorldPacket& recvPacket);
        void HandleInitiateTradeOpcode(WorldPacket& recvPacket);
        void HandleSetTradeGoldOpcode(WorldPacket& recvPacket);
        void HandleSetTradeItemOpcode(WorldPacket& recvPacket);
        void HandleUnacceptTradeOpcode(WorldPacket& recvPacket);

        void HandleAuctionHelloOpcode(WorldPacket& recvPacket);
        void HandleAuctionListItems(WorldPacket& recvData);
        void HandleAuctionListBidderItems(WorldPacket& recvData);
        void HandleAuctionSellItem(WorldPacket& recvData);
        void HandleAuctionRemoveItem(WorldPacket& recvData);
        void HandleAuctionListOwnerItems(WorldPacket& recvData);
        void HandleAuctionPlaceBid(WorldPacket& recvData);
        void HandleAuctionListPendingSales(WorldPacket& recvData);

        void HandleGetMailList(WorldPacket& recvData);
        void HandleSendMail(WorldPacket& recvData);
        void HandleMailTakeMoney(WorldPacket& recvData);
        void HandleMailTakeItem(WorldPacket& recvData);
        void HandleMailMarkAsRead(WorldPacket& recvData);
        void HandleMailReturnToSender(WorldPacket& recvData);
        void HandleMailDelete(WorldPacket& recvData);
        void HandleItemTextQuery(WorldPacket& recvData);
        void HandleMailCreateTextItem(WorldPacket& recvData);
        void HandleQueryNextMailTime(WorldPacket& recvData);
        void HandleCancelChanneling(WorldPacket& recvData);

        void SendItemPageInfo(ItemTemplate* itemProto);
        void HandleSplitItemOpcode(WorldPacket& recvPacket);
        void HandleSwapInvItemOpcode(WorldPacket& recvPacket);
        void HandleDestroyItemOpcode(WorldPacket& recvPacket);
        void HandleAutoEquipItemOpcode(WorldPacket& recvPacket);
        void HandleRequestHotFix(WorldPacket& recvPacket);
        void HandleSellItemOpcode(WorldPacket& recvPacket);
        void HandleBuyItemInSlotOpcode(WorldPacket& recvPacket);
        void HandleBuyItemOpcode(WorldPacket& recvPacket);
        void HandleListInventoryOpcode(WorldPacket& recvPacket);
        void HandleAutoStoreBagItemOpcode(WorldPacket& recvPacket);
        void HandleReadItem(WorldPacket& recvPacket);
        void HandleAutoEquipItemSlotOpcode(WorldPacket& recvPacket);
        void HandleSwapItem(WorldPacket& recvPacket);
        void HandleBuybackItem(WorldPacket& recvPacket);
        void HandleAutoBankItemOpcode(WorldPacket& recvPacket);
        void HandleAutoStoreBankItemOpcode(WorldPacket& recvPacket);
        void HandleWrapItemOpcode(WorldPacket& recvPacket);

        void HandleAttackSwingOpcode(WorldPacket& recvPacket);
        void HandleAttackStopOpcode(WorldPacket& recvPacket);
        void HandleSetSheathedOpcode(WorldPacket& recvPacket);

        void HandleUseItemOpcode(WorldPacket& recvPacket);
        void HandleOpenItemOpcode(WorldPacket& recvPacket);
        void HandleCastSpellOpcode(WorldPacket& recvPacket);
        void HandleCancelCastOpcode(WorldPacket& recvPacket);
        void HandleCancelAuraOpcode(WorldPacket& recvPacket);
        void HandleCancelGrowthAuraOpcode(WorldPacket& recvPacket);
        void HandleCancelAutoRepeatSpellOpcode(WorldPacket& recvPacket);

        void HandleLearnTalentOpcode(WorldPacket& recvPacket);
        void HandleLearnPreviewTalents(WorldPacket& recvPacket);
        void HandleTalentWipeConfirmOpcode(WorldPacket& recvPacket);
        void HandleUnlearnSkillOpcode(WorldPacket& recvPacket);

        void HandleQuestgiverStatusQueryOpcode(WorldPacket& recvPacket);
        void HandleQuestgiverStatusMultipleQuery(WorldPacket& recvPacket);
        void HandleQuestgiverHelloOpcode(WorldPacket& recvPacket);
        void HandleQuestgiverAcceptQuestOpcode(WorldPacket& recvPacket);
        void HandleQuestgiverQueryQuestOpcode(WorldPacket& recvPacket);
        void HandleQuestgiverChooseRewardOpcode(WorldPacket& recvPacket);
        void HandleQuestgiverRequestRewardOpcode(WorldPacket& recvPacket);
        void HandleQuestQueryOpcode(WorldPacket& recvPacket);
        void HandleQuestgiverCancel(WorldPacket& recvData);
        void HandleQuestLogSwapQuest(WorldPacket& recvData);
        void HandleQuestLogRemoveQuest(WorldPacket& recvData);
        void HandleQuestConfirmAccept(WorldPacket& recvData);
        void HandleQuestgiverCompleteQuest(WorldPacket& recvData);
        void HandleQuestgiverQuestAutoLaunch(WorldPacket& recvPacket);
        void HandlePushQuestToParty(WorldPacket& recvPacket);
        void HandleQuestPushResult(WorldPacket& recvPacket);

        bool processChatmessageFurtherAfterSecurityChecks(std::string&, uint32);
        void HandleMessagechatOpcode(WorldPacket& recvPacket);
        void SendPlayerNotFoundNotice(std::string name);
        void SendPlayerAmbiguousNotice(std::string name);
        void SendWrongFactionNotice();
        void SendChatRestrictedNotice(ChatRestrictionType restriction);
        void HandleTextEmoteOpcode(WorldPacket& recvPacket);
        void HandleChatIgnoredOpcode(WorldPacket& recvPacket);

        void HandleReclaimCorpseOpcode(WorldPacket& recvPacket);
        void HandleCorpseQueryOpcode(WorldPacket& recvPacket);
        void HandleCorpseMapPositionQuery(WorldPacket& recvPacket);
        void HandleResurrectResponseOpcode(WorldPacket& recvPacket);
        void HandleSummonResponseOpcode(WorldPacket& recvData);

        void HandleJoinChannel(WorldPacket& recvPacket);
        void HandleLeaveChannel(WorldPacket& recvPacket);
        void HandleChannelList(WorldPacket& recvPacket);
        void HandleChannelPassword(WorldPacket& recvPacket);
        void HandleChannelSetOwner(WorldPacket& recvPacket);
        void HandleChannelOwner(WorldPacket& recvPacket);
        void HandleChannelModerator(WorldPacket& recvPacket);
        void HandleChannelUnmoderator(WorldPacket& recvPacket);
        void HandleChannelMute(WorldPacket& recvPacket);
        void HandleChannelUnmute(WorldPacket& recvPacket);
        void HandleChannelInvite(WorldPacket& recvPacket);
        void HandleChannelKick(WorldPacket& recvPacket);
        void HandleChannelBan(WorldPacket& recvPacket);
        void HandleChannelUnban(WorldPacket& recvPacket);
        void HandleChannelAnnouncements(WorldPacket& recvPacket);
        void HandleChannelModerate(WorldPacket& recvPacket);
        void HandleChannelDeclineInvite(WorldPacket& recvPacket);
        void HandleChannelDisplayListQuery(WorldPacket& recvPacket);
        void HandleGetChannelMemberCount(WorldPacket& recvPacket);
        void HandleSetChannelWatch(WorldPacket& recvPacket);

        void HandleCompleteCinematic(WorldPacket& recvPacket);
        void HandleNextCinematicCamera(WorldPacket& recvPacket);

        void HandlePageQuerySkippedOpcode(WorldPacket& recvPacket);
        void HandlePageTextQueryOpcode(WorldPacket& recvPacket);

        void HandleTutorialFlag (WorldPacket& recvData);
        void HandleTutorialClear(WorldPacket& recvData);
        void HandleTutorialReset(WorldPacket& recvData);

        //Pet
        void HandlePetAction(WorldPacket& recvData);
        void HandlePetStopAttack(WorldPacket& recvData);
        void HandlePetActionHelper(Unit* pet, uint64 guid1, uint32 spellid, uint16 flag, uint64 guid2, float x, float y, float z);
        void HandlePetNameQuery(WorldPacket& recvData);
        void HandlePetSetAction(WorldPacket& recvData);
        void HandlePetAbandon(WorldPacket& recvData);
        void HandlePetRename(WorldPacket& recvData);
        void HandlePetCancelAuraOpcode(WorldPacket& recvPacket);
        void HandlePetSpellAutocastOpcode(WorldPacket& recvPacket);
        void HandlePetCastSpellOpcode(WorldPacket& recvPacket);
        void HandlePetLearnTalent(WorldPacket& recvPacket);
        void HandleLearnPreviewTalentsPet(WorldPacket& recvPacket);

        void HandleSetActionBarToggles(WorldPacket& recvData);

        void HandleCharRenameOpcode(WorldPacket& recvData);
        void HandleChangePlayerNameOpcodeCallBack(PreparedQueryResult result, std::string newName);
        void HandleSetPlayerDeclinedNames(WorldPacket& recvData);

        void HandleTotemDestroyed(WorldPacket& recvData);
        void HandleDismissCritter(WorldPacket& recvData);

        //Battleground
        void HandleBattlemasterHelloOpcode(WorldPacket& recvData);
        void HandleBattlemasterJoinOpcode(WorldPacket& recvData);
        void HandleBattlegroundPlayerPositionsOpcode(WorldPacket& recvData);
        void HandleBattlegroundPortOpcode(WorldPacket& recvData);
        void HandlePVPLogDataOpcode(WorldPacket& recvData);
        void HandleBattleFieldPortOpcode(WorldPacket& recvData);
        void HandleBattlefieldListOpcode(WorldPacket& recvData);
        void HandleLeaveBattlefieldOpcode(WorldPacket& recvData);
        void HandleBattlemasterJoinArena(WorldPacket& recvData);
        void HandleReportPvPAFK(WorldPacket& recvData);

        void HandleWardenDataOpcode(WorldPacket& recvData);
        void HandleWorldTeleportOpcode(WorldPacket& recvData);
        void HandleMinimapPingOpcode(WorldPacket& recvData);
        void HandleRandomRollOpcode(WorldPacket& recvData);
        void HandleFarSightOpcode(WorldPacket& recvData);
        void HandleSetDungeonDifficultyOpcode(WorldPacket& recvData);
        void HandleSetRaidDifficultyOpcode(WorldPacket& recvData);
        void HandleMoveSetCanFlyAckOpcode(WorldPacket& recvData);
        void HandleSetTitleOpcode(WorldPacket& recvData);
        void HandleRealmSplitOpcode(WorldPacket& recvData);
        void HandleTimeSyncResp(WorldPacket& recvData);
        void HandleWhoisOpcode(WorldPacket& recvData);
        void HandleResetInstancesOpcode(WorldPacket& recvData);
        void HandleHearthAndResurrect(WorldPacket& recvData);
        void HandleGuildPartyStateUpdate(WorldPacket& recvData);
        //void HandleUITimeRequest(WorldPacket& recvData); // NYI
        void HandleInstanceLockResponse(WorldPacket& recvPacket);

        // Battlefield
        void SendBfInvitePlayerToWar(uint32 BattleId, uint32 ZoneId, uint64 time);
        void SendBfInvitePlayerToQueue(uint32 BattleId);
        void SendBfQueueInviteResponse(uint32 BattleId, uint32 ZoneId);
        void SendBfEntered(uint32 BattleId);
        void SendBfLeaveMessage(uint32 BattleId);
        void HandleBfQueueInviteResponse(WorldPacket& recvData);
        void HandleBfJoinQueue(WorldPacket& recvData);
        void HandleBfEntryInviteResponse(WorldPacket& recvData);
        void HandleBfExitRequest(WorldPacket& recvData);

        // Looking for Dungeon/Raid
        void HandleLfgSetCommentOpcode(WorldPacket& recvData);
        void HandleLfgPlayerLockInfoRequestOpcode(WorldPacket& recvData);
        void HandleLfgPartyLockInfoRequestOpcode(WorldPacket& recvData);
        void HandleLfgJoinOpcode(WorldPacket& recvData);
        void HandleLfgLeaveOpcode(WorldPacket& /*recvData*/);
        void HandleLfgSetRolesOpcode(WorldPacket& recvData);
        void HandleLfgProposalResultOpcode(WorldPacket& recvData);
        void HandleLfgSetBootVoteOpcode(WorldPacket& recvData);
        void HandleLfgTeleportOpcode(WorldPacket& recvData);
        void HandleLfrSearchOpcode(WorldPacket& recvData);
        void HandleLfrLeaveOpcode(WorldPacket& recvData);

        void SendLfgUpdatePlayer(const LfgUpdateData& updateData);
        void SendLfgUpdateParty(const LfgUpdateData& updateData);
        void SendLfgRoleChosen(uint64 guid, uint8 roles);
        void SendLfgRoleCheckUpdate(const LfgRoleCheck* pRoleCheck);
        void SendLfgUpdateSearch(bool update);
        void SendLfgJoinResult(const LfgJoinResultData& joinData);
        void SendLfgQueueStatus(uint32 dungeon, int32 waitTime, int32 avgWaitTime, int32 waitTimeTanks, int32 waitTimeHealer, int32 waitTimeDps, uint32 queuedTime, uint8 tanks, uint8 healers, uint8 dps);
        void SendLfgPlayerReward(uint32 rdungeonEntry, uint32 sdungeonEntry, uint8 done, const LfgReward* reward, const Quest *qRew);
        void SendLfgBootPlayer(const LfgPlayerBoot* pBoot);
        void SendLfgUpdateProposal(uint32 proposalId, const LfgProposal *pProp);
        void SendLfgDisabled();
        void SendLfgOfferContinue(uint32 dungeonEntry);
        void SendLfgTeleportError(uint8 err);

        // Arena Team
        void HandleArenaTeamCreate(WorldPacket& recv_packet);
        void HandleInspectArenaTeamsOpcode(WorldPacket& recvData);
        void HandleArenaTeamQueryOpcode(WorldPacket& recvData);
        void HandleArenaTeamRosterOpcode(WorldPacket& recvData);
        void HandleArenaTeamInviteOpcode(WorldPacket& recvData);
        void HandleArenaTeamAcceptOpcode(WorldPacket& recvData);
        void HandleArenaTeamDeclineOpcode(WorldPacket& recvData);
        void HandleArenaTeamLeaveOpcode(WorldPacket& recvData);
        void HandleArenaTeamRemoveOpcode(WorldPacket& recvData);
        void HandleArenaTeamDisbandOpcode(WorldPacket& recvData);
        void HandleArenaTeamLeaderOpcode(WorldPacket& recvData);

        void HandleAreaSpiritHealerQueryOpcode(WorldPacket& recvData);
        void HandleAreaSpiritHealerQueueOpcode(WorldPacket& recvData);
        void HandleCancelMountAuraOpcode(WorldPacket& recvData);
        void HandleSelfResOpcode(WorldPacket& recvData);
        void HandleComplainOpcode(WorldPacket& recvData);
        void HandleRequestPetInfoOpcode(WorldPacket& recvData);

        // Socket gem
        void HandleSocketOpcode(WorldPacket& recvData);

        void HandleCancelTempEnchantmentOpcode(WorldPacket& recvData);

        void HandleItemRefundInfoRequest(WorldPacket& recvData);
        void HandleItemRefund(WorldPacket& recvData);

        void HandleChannelVoiceOnOpcode(WorldPacket& recvData);
        void HandleVoiceSessionEnableOpcode(WorldPacket& recvData);
        void HandleSetActiveVoiceChannel(WorldPacket& recvData);
        void HandleSetTaxiBenchmarkOpcode(WorldPacket& recvData);

        // Guild Bank
        void HandleGuildPermissions(WorldPacket& recvData);
        void HandleGuildBankMoneyWithdrawn(WorldPacket& recvData);
        void HandleGuildBankerActivate(WorldPacket& recvData);
        void HandleGuildBankQueryTab(WorldPacket& recvData);
        void HandleGuildBankLogQuery(WorldPacket& recvData);
        void HandleGuildBankDepositMoney(WorldPacket& recvData);
        void HandleGuildBankWithdrawMoney(WorldPacket& recvData);
        void HandleGuildBankSwapItems(WorldPacket& recvData);

        void HandleGuildBankUpdateTab(WorldPacket& recvData);
        void HandleGuildBankBuyTab(WorldPacket& recvData);
        void HandleQueryGuildBankTabText(WorldPacket& recvData);
        void HandleSetGuildBankTabText(WorldPacket& recvData);
        void HandleGuildQueryTradeSkill(WorldPacket& recvData);

        // Refer-a-Friend
        void HandleGrantLevel(WorldPacket& recvData);
        void HandleAcceptGrantLevel(WorldPacket& recvData);

        // Calendar
        void HandleCalendarGetCalendar(WorldPacket& recvData);
        void HandleCalendarGetEvent(WorldPacket& recvData);
        void HandleCalendarGuildFilter(WorldPacket& recvData);
        void HandleCalendarArenaTeam(WorldPacket& recvData);
        void HandleCalendarAddEvent(WorldPacket& recvData);
        void HandleCalendarUpdateEvent(WorldPacket& recvData);
        void HandleCalendarRemoveEvent(WorldPacket& recvData);
        void HandleCalendarCopyEvent(WorldPacket& recvData);
        void HandleCalendarEventInvite(WorldPacket& recvData);
        void HandleCalendarEventRsvp(WorldPacket& recvData);
        void HandleCalendarEventRemoveInvite(WorldPacket& recvData);
        void HandleCalendarEventStatus(WorldPacket& recvData);
        void HandleCalendarEventModeratorStatus(WorldPacket& recvData);
        void HandleCalendarComplain(WorldPacket& recvData);
        void HandleCalendarGetNumPending(WorldPacket& recvData);
        void HandleCalendarEventSignup(WorldPacket& recvData);

        void SendCalendarEvent(CalendarEvent const& calendarEvent, CalendarSendEventType sendEventType);
        void SendCalendarEventInvite(CalendarInvite const& invite, bool pending);
        void SendCalendarEventInviteAlert(CalendarEvent const& calendarEvent, CalendarInvite const& calendarInvite);
        void SendCalendarEventInviteRemove(CalendarInvite const& invite, uint32 flags);
        void SendCalendarEventInviteRemoveAlert(CalendarEvent const& calendarEvent, CalendarInviteStatus status);
        void SendCalendarEventRemovedAlert(CalendarEvent const& calendarEvent);
        void SendCalendarEventUpdateAlert(CalendarEvent const& calendarEvent, CalendarSendEventType sendEventType);
        void SendCalendarEventStatus(CalendarEvent const& calendarEvent, CalendarInvite const& invite);
        void SendCalendarEventModeratorStatusAlert(CalendarInvite const& invite);
        void SendCalendarCommandResult(CalendarError err, char const* param = NULL);
        void SendCalendarRaidLockout(InstanceSave const* save, bool add);
        void SendCalendarRaidLockoutUpdated(InstanceSave const* save);
        void SendCalendarClearActionPending();

        void HandleSpellClick(WorldPacket& recvData);
        void HandleMirrorImageDataRequest(WorldPacket& recvData);
        void HandleAlterAppearance(WorldPacket& recvData);
        void HandleRemoveGlyph(WorldPacket& recvData);
        void HandleCharCustomize(WorldPacket& recvData);
        void HandleQueryInspectAchievements(WorldPacket& recvData);
        void HandleEquipmentSetSave(WorldPacket& recvData);
        void HandleEquipmentSetDelete(WorldPacket& recvData);
        void HandleEquipmentSetUse(WorldPacket& recvData);
        void HandleWorldStateUITimerUpdate(WorldPacket& recvData);
        void HandleReadyForAccountDataTimes(WorldPacket& recvData);
        void HandleQueryQuestsCompleted(WorldPacket& recvData);
        void HandleQuestPOIQuery(WorldPacket& recvData);
        void HandleEjectPassenger(WorldPacket& data);
        void HandleEnterPlayerVehicle(WorldPacket& data);
        void HandleUpdateProjectilePosition(WorldPacket& recvPacket);
        void HandleReforgeItem(WorldPacket& recvData);

    private:
        void InitializeQueryCallbackParameters();
        void ProcessQueryCallbacks();

        PreparedQueryResultFuture _charEnumCallback;
        PreparedQueryResultFuture _addIgnoreCallback;
        PreparedQueryResultFuture _stablePetCallback;
        QueryCallback<PreparedQueryResult, std::string> _charRenameCallback;
        QueryCallback<PreparedQueryResult, std::string> _addFriendCallback;
        QueryCallback<PreparedQueryResult, uint32> _unstablePetCallback;
        QueryCallback<PreparedQueryResult, uint32> _stableSwapCallback;
        QueryCallback<PreparedQueryResult, uint64> _sendStabledPetCallback;
        QueryCallback<PreparedQueryResult, CharacterCreateInfo*, true> _charCreateCallback;
        QueryResultHolderFuture _charLoginCallback;

    private:
        // private trade methods
        void moveItems(Item* myItems[], Item* hisItems[]);

        // logging helper
        void LogUnexpectedOpcode(WorldPacket* packet, const char* status, const char *reason);
        void LogUnprocessedTail(WorldPacket* packet);

        // EnumData helpers
        bool CharCanLogin(uint32 lowGUID)
        {
            return _allowedCharsToLogin.find(lowGUID) != _allowedCharsToLogin.end();
        }

        // this stores the GUIDs of the characters who can login
        // characters who failed on Player::BuildEnumData shouldn't login
        std::set<uint32> _allowedCharsToLogin;

        uint32 m_GUIDLow;     // set logged in or recently logout player (while m_playerRecentlyLogout set)
        Player* _player;
        WorldSocket* m_Socket;
        std::string m_Address;

        AccountTypes _security;
        uint32 _accountId;
        uint8 m_expansion;

        typedef std::list<AddonInfo> AddonsList;

        // Warden
        Warden* _warden;                                    // Remains NULL if Warden system is not enabled by config

        time_t _logoutTime;
        bool m_inQueue;                                     // session wait in auth.queue
        bool m_playerLoading;                               // code processed in LoginPlayer
        bool m_playerLogout;                                // code processed in LogoutPlayer
        bool m_playerRecentlyLogout;
        bool m_playerSave;
        LocaleConstant m_sessionDbcLocale;
        LocaleConstant m_sessionDbLocaleIndex;
        uint32 m_latency;
        AccountData m_accountData[NUM_ACCOUNT_DATA_TYPES];
        uint32 m_Tutorials[MAX_ACCOUNT_TUTORIAL_VALUES];
        bool   m_TutorialsChanged;
        AddonsList m_addonsList;
        uint32 recruiterId;
        bool isRecruiter;
        ACE_Based::LockedQueue<WorldPacket*, ACE_Thread_Mutex> _recvQueue;
        time_t timeLastWhoCommand;
};
#endif
/// @}
