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

#include "Common.h"
#include "Language.h"
#include "DatabaseEnv.h"
#include "WorldPacket.h"
#include "Opcodes.h"
#include "Log.h"
#include "Player.h"
#include "GossipDef.h"
#include "World.h"
#include "ObjectMgr.h"
#include "GuildMgr.h"
#include "WorldSession.h"
#include "BigNumber.h"
#include "SHA1.h"
#include "UpdateData.h"
#include "LootMgr.h"
#include "Chat.h"
#include "zlib.h"
#include "ObjectAccessor.h"
#include "Object.h"
#include "BattlegroundMgr.h"
#include "BattlefieldMgr.h"
#include "OutdoorPvP.h"
#include "Pet.h"
#include "SocialMgr.h"
#include "CellImpl.h"
#include "AccountMgr.h"
#include "Vehicle.h"
#include "CreatureAI.h"
#include "DBCEnums.h"
#include "ScriptMgr.h"
#include "MapManager.h"
#include "InstanceScript.h"
#include "GameObjectAI.h"
#include "Group.h"
#include "AccountMgr.h"
#include "Guild.h"

void WorldSession::HandleRepopRequestOpcode(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Recvd CMSG_REPOP_REQUEST Message");

    recvData.read_skip<uint8>();

    if (GetPlayer()->isAlive() || GetPlayer()->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST))
        return;

    if (GetPlayer()->HasAuraType(SPELL_AURA_PREVENT_RESURRECTION))
        return; // silently return, client should display the error by itself

    // the world update order is sessions, players, creatures
    // the netcode runs in parallel with all of these
    // creatures can kill players
    // so if the server is lagging enough the player can
    // release spirit after he's killed but before he is updated
    if (GetPlayer()->getDeathState() == JUST_DIED)
    {
        sLog->outDebug(LOG_FILTER_NETWORKIO, "HandleRepopRequestOpcode: got request after player %s(%d) was killed and before he was updated", GetPlayer()->GetName(), GetPlayer()->GetGUIDLow());
        GetPlayer()->KillPlayer();
    }

    //this is spirit release confirm?
    GetPlayer()->RemovePet(NULL, PET_SAVE_AS_CURRENT, true);
    GetPlayer()->BuildPlayerRepop();
    GetPlayer()->RepopAtGraveyard();
}

void WorldSession::HandleGossipSelectOptionOpcode(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: CMSG_GOSSIP_SELECT_OPTION");

    uint32 gossipListId;
    uint32 menuId;
    uint64 guid;
    std::string code = "";

    recvData >> guid >> menuId >> gossipListId;

    if (_player->PlayerTalkClass->IsGossipOptionCoded(gossipListId))
        recvData >> code;

    Creature* unit = NULL;
    GameObject* go = NULL;
    if (IS_CRE_OR_VEH_GUID(guid))
    {
        unit = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_NONE);
        if (!unit)
        {
            sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: HandleGossipSelectOptionOpcode - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)));
            return;
        }
    }
    else if (IS_GAMEOBJECT_GUID(guid))
    {
        go = _player->GetMap()->GetGameObject(guid);
        if (!go)
        {
            sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: HandleGossipSelectOptionOpcode - GameObject (GUID: %u) not found.", uint32(GUID_LOPART(guid)));
            return;
        }
    }
    else
    {
        sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: HandleGossipSelectOptionOpcode - unsupported GUID type for highguid %u. lowpart %u.", uint32(GUID_HIPART(guid)), uint32(GUID_LOPART(guid)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    if ((unit && unit->GetCreatureTemplate()->ScriptID != unit->LastUsedScriptID) || (go && go->GetGOInfo()->ScriptId != go->LastUsedScriptID))
    {
        sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: HandleGossipSelectOptionOpcode - Script reloaded while in use, ignoring and set new scipt id");
        if (unit)
            unit->LastUsedScriptID = unit->GetCreatureTemplate()->ScriptID;
        if (go)
            go->LastUsedScriptID = go->GetGOInfo()->ScriptId;
        _player->PlayerTalkClass->SendCloseGossip();
        return;
    }
    if (!code.empty())
    {
        if (unit)
        {
            unit->AI()->sGossipSelectCode(_player, menuId, gossipListId, code.c_str());
            if (!sScriptMgr->OnGossipSelectCode(_player, unit, _player->PlayerTalkClass->GetGossipOptionSender(gossipListId), _player->PlayerTalkClass->GetGossipOptionAction(gossipListId), code.c_str()))
                _player->OnGossipSelect(unit, gossipListId, menuId);
        }
        else
        {
            go->AI()->GossipSelectCode(_player, menuId, gossipListId, code.c_str());
            sScriptMgr->OnGossipSelectCode(_player, go, _player->PlayerTalkClass->GetGossipOptionSender(gossipListId), _player->PlayerTalkClass->GetGossipOptionAction(gossipListId), code.c_str());
        }
    }
    else
    {
        if (unit)
        {
            unit->AI()->sGossipSelect(_player, menuId, gossipListId);
            if (!sScriptMgr->OnGossipSelect(_player, unit, _player->PlayerTalkClass->GetGossipOptionSender(gossipListId), _player->PlayerTalkClass->GetGossipOptionAction(gossipListId)))
                _player->OnGossipSelect(unit, gossipListId, menuId);
        }
        else
        {
            go->AI()->GossipSelect(_player, menuId, gossipListId);
            if (!sScriptMgr->OnGossipSelect(_player, go, _player->PlayerTalkClass->GetGossipOptionSender(gossipListId), _player->PlayerTalkClass->GetGossipOptionAction(gossipListId)))
                _player->OnGossipSelect(go, gossipListId, menuId);
        }
    }
}

void WorldSession::HandleWhoOpcode(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Recvd CMSG_WHO Message");

    time_t now = time(NULL);
    if (now - timeLastWhoCommand < 5)
        return;
    else timeLastWhoCommand = now;

    uint32 matchcount = 0;

    uint32 level_min, level_max, racemask, classmask, zones_count, str_count;
    uint32 zoneids[10];                                     // 10 is client limit
    std::string player_name, guild_name;

    recvData >> level_min;                                 // maximal player level, default 0
    recvData >> level_max;                                 // minimal player level, default 100 (MAX_LEVEL)
    recvData >> player_name;                               // player name, case sensitive...

    recvData >> guild_name;                                // guild name, case sensitive...

    recvData >> racemask;                                  // race mask
    recvData >> classmask;                                 // class mask
    recvData >> zones_count;                               // zones count, client limit = 10 (2.0.10)

    if (zones_count > 10)
        return;                                             // can't be received from real client or broken packet

    for (uint32 i = 0; i < zones_count; ++i)
    {
        uint32 temp;
        recvData >> temp;                                  // zone id, 0 if zone is unknown...
        zoneids[i] = temp;
        sLog->outDebug(LOG_FILTER_NETWORKIO, "Zone %u: %u", i, zoneids[i]);
    }

    recvData >> str_count;                                 // user entered strings count, client limit=4 (checked on 2.0.10)

    if (str_count > 4)
        return;                                             // can't be received from real client or broken packet

    sLog->outDebug(LOG_FILTER_NETWORKIO, "Minlvl %u, maxlvl %u, name %s, guild %s, racemask %u, classmask %u, zones %u, strings %u", level_min, level_max, player_name.c_str(), guild_name.c_str(), racemask, classmask, zones_count, str_count);

    std::wstring str[4];                                    // 4 is client limit
    for (uint32 i = 0; i < str_count; ++i)
    {
        std::string temp;
        recvData >> temp;                                  // user entered string, it used as universal search pattern(guild+player name)?

        if (!Utf8toWStr(temp, str[i]))
            continue;

        wstrToLower(str[i]);

        sLog->outDebug(LOG_FILTER_NETWORKIO, "String %u: %s", i, temp.c_str());
    }

    std::wstring wplayer_name;
    std::wstring wguild_name;
    if (!(Utf8toWStr(player_name, wplayer_name) && Utf8toWStr(guild_name, wguild_name)))
        return;
    wstrToLower(wplayer_name);
    wstrToLower(wguild_name);

    // client send in case not set max level value 100 but SkyFire supports 255 max level,
    // update it to show GMs with characters after 100 level
    if (level_max >= MAX_LEVEL)
        level_max = STRONG_MAX_LEVEL;

    uint32 team = _player->GetTeam();
    uint32 security = GetSecurity();
    bool allowTwoSideWhoList = sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_WHO_LIST);
    uint32 gmLevelInWhoList  = sWorld->getIntConfig(CONFIG_GM_LEVEL_IN_WHO_LIST);
    uint32 displaycount = 0;

    WorldPacket data(SMSG_WHO, 50);                       // guess size
    data << uint32(matchcount);                           // placeholder, count of players matching criteria
    data << uint32(displaycount);                         // placeholder, count of players displayed

    SKYFIRE_READ_GUARD(HashMapHolder<Player>::LockType, *HashMapHolder<Player>::GetLock());
    HashMapHolder<Player>::MapType const& m = sObjectAccessor->GetPlayers();
    for (HashMapHolder<Player>::MapType::const_iterator itr = m.begin(); itr != m.end(); ++itr)
    {
        if (AccountMgr::IsPlayerAccount(security))
        {
            // player can see member of other team only if CONFIG_ALLOW_TWO_SIDE_WHO_LIST
            if (itr->second->GetTeam() != team && !allowTwoSideWhoList)
                continue;

            // player can see MODERATOR, GAME MASTER, ADMINISTRATOR only if CONFIG_GM_IN_WHO_LIST
            if ((itr->second->GetSession()->GetSecurity() > AccountTypes(gmLevelInWhoList)))
                continue;
        }

        //do not process players which are not in world
        if (!(itr->second->IsInWorld()))
            continue;

        // check if target is globally visible for player
        if (!(itr->second->IsVisibleGloballyFor(_player)))
            continue;

        // check if target's level is in level range
        uint8 lvl = itr->second->getLevel();
        if (lvl < level_min || lvl > level_max)
            continue;

        // check if class matches classmask
        uint32 class_ = itr->second->getClass();
        if (!(classmask & (1 << class_)))
            continue;

        // check if race matches racemask
        uint32 race = itr->second->getRace();
        if (!(racemask & (1 << race)))
            continue;

        uint32 pzoneid = itr->second->GetZoneId();
        uint8 gender = itr->second->getGender();

        bool z_show = true;
        for (uint32 i = 0; i < zones_count; ++i)
        {
            if (zoneids[i] == pzoneid)
            {
                z_show = true;
                break;
            }

            z_show = false;
        }
        if (!z_show)
            continue;

        std::string pname = itr->second->GetName();
        std::wstring wpname;
        if (!Utf8toWStr(pname, wpname))
            continue;
        wstrToLower(wpname);

        if (!(wplayer_name.empty() || wpname.find(wplayer_name) != std::wstring::npos))
            continue;

        std::string gname = sGuildMgr->GetGuildNameById(itr->second->GetGuildId());
        std::wstring wgname;
        if (!Utf8toWStr(gname, wgname))
            continue;
        wstrToLower(wgname);

        if (!(wguild_name.empty() || wgname.find(wguild_name) != std::wstring::npos))
            continue;

        std::string aname;
        if (AreaTableEntry const* areaEntry = GetAreaEntryByAreaID(itr->second->GetZoneId()))
            aname = areaEntry->area_name;

        bool s_show = true;
        for (uint32 i = 0; i < str_count; ++i)
        {
            if (!str[i].empty())
            {
                if (wgname.find(str[i]) != std::wstring::npos ||
                    wpname.find(str[i]) != std::wstring::npos ||
                    Utf8FitTo(aname, str[i]))
                {
                    s_show = true;
                    break;
                }
                s_show = false;
            }
        }
        if (!s_show)
            continue;

        // 49 is maximum player count sent to client - can be overridden
        // through config, but is unstable
        if ((matchcount++) >= sWorld->getIntConfig(CONFIG_MAX_WHO))
            continue;

        data << pname;                                    // player name
        data << gname;                                    // guild name
        data << uint32(lvl);                              // player level
        data << uint32(class_);                           // player class
        data << uint32(race);                             // player race
        data << uint8(gender);                            // player gender
        data << uint32(pzoneid);                          // player zone id

        ++displaycount;
    }

    data.put(0, displaycount);                            // insert right count, count displayed
    data.put(4, matchcount);                              // insert right count, count of matches

    SendPacket(&data);
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Send SMSG_WHO Message");
}

void WorldSession::HandleLogoutRequestOpcode(WorldPacket& /*recvData*/)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Recvd CMSG_LOGOUT_REQUEST Message, security - %u", GetSecurity());

    if (uint64 lguid = GetPlayer()->GetLootGUID())
        DoLootRelease(lguid);

    uint8 reason = 0;

    if (GetPlayer()->isInCombat())
        reason = 1;
    else if (GetPlayer()->_movementInfo.HasMovementFlag(MOVEMENTFLAG_JUMPING | MOVEMENTFLAG_FALLING))
        reason = 3;                                         // is jumping or falling
    else if (GetPlayer()->duel || GetPlayer()->HasAura(9454)) // is dueling or frozen by GM via freeze command
        reason = 2;                                         // FIXME - Need the correct value

    if (reason)
    {
        WorldPacket data(SMSG_LOGOUT_RESPONSE, 1+4);
        data << uint8(reason);
        data << uint32(0);
        SendPacket(&data);
        LogoutRequest(0);
        return;
    }

    //instant logout in taverns/cities or on taxi or for admins, gm's, mod's if its enabled in worldserver.conf
    if (GetPlayer()->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING) || GetPlayer()->isInFlight() ||
        GetSecurity() >= AccountTypes(sWorld->getIntConfig(CONFIG_INSTANT_LOGOUT)))
    {
		GetPlayer()->_SaveCurrentPhase();
        WorldPacket data(SMSG_LOGOUT_RESPONSE, 1+4);
        data << uint8(0);
        data << uint32(16777216);
        SendPacket(&data);
        LogoutPlayer(true);
        return;
    }

    // not set flags if player can't free move to prevent lost state at logout cancel
    if (GetPlayer()->CanFreeMove())
    {
        GetPlayer()->SetStandState(UNIT_STAND_STATE_SIT);

        WorldPacket data(SMSG_FORCE_MOVE_ROOT, (8+4));    // guess size
        data.append(GetPlayer()->GetPackGUID());
        data << (uint32)2;
        SendPacket(&data);
        GetPlayer()->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
    }

	GetPlayer()->_SaveCurrentPhase();
    WorldPacket data(SMSG_LOGOUT_RESPONSE, 1+4);
    data << uint8(0);
    data << uint32(0);
    SendPacket(&data);
    LogoutRequest(time(NULL));
}

void WorldSession::HandlePlayerLogoutOpcode(WorldPacket& /*recvData*/)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Recvd CMSG_PLAYER_LOGOUT Message");
}

void WorldSession::HandleLogoutCancelOpcode(WorldPacket& /*recvData*/)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Recvd CMSG_LOGOUT_CANCEL Message");

    LogoutRequest(0);

    WorldPacket data(SMSG_LOGOUT_CANCEL_ACK, 0);
    SendPacket(&data);

    // not remove flags if can't free move - its not set in Logout request code.
    if (GetPlayer()->CanFreeMove())
    {
        //!we can move again
        data.Initialize(SMSG_FORCE_MOVE_UNROOT, 8);       // guess size
        data.append(GetPlayer()->GetPackGUID());
        data << uint32(0);
        SendPacket(&data);

        //! Stand Up
        GetPlayer()->SetStandState(UNIT_STAND_STATE_STAND);

        //! DISABLE_ROTATE
        GetPlayer()->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
    }

    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Sent SMSG_LOGOUT_CANCEL_ACK Message");
}

void WorldSession::HandleTogglePvP(WorldPacket& recvData)
{
    // this opcode can be used in two ways: Either set explicit new status or toggle old status
    if (recvData.size() == 1)
    {
        bool newPvPStatus;
        recvData >> newPvPStatus;
        GetPlayer()->ApplyModFlag(PLAYER_FLAGS, PLAYER_FLAGS_IN_PVP, newPvPStatus);
        GetPlayer()->ApplyModFlag(PLAYER_FLAGS, PLAYER_FLAGS_PVP_TIMER, !newPvPStatus);
    }
    else
    {
        GetPlayer()->ToggleFlag(PLAYER_FLAGS, PLAYER_FLAGS_IN_PVP);
        GetPlayer()->ToggleFlag(PLAYER_FLAGS, PLAYER_FLAGS_PVP_TIMER);
    }

    if (GetPlayer()->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_IN_PVP))
    {
        if (!GetPlayer()->IsPvP() || GetPlayer()->pvpInfo.endTimer != 0)
            GetPlayer()->UpdatePvP(true, true);
    }
    else
    {
        if (!GetPlayer()->pvpInfo.inHostileArea && GetPlayer()->IsPvP())
            GetPlayer()->pvpInfo.endTimer = time(NULL);     // start toggle-off
    }

    //if (OutdoorPvP* pvp = _player->GetOutdoorPvP())
    //    pvp->HandlePlayerActivityChanged(_player);
}

void WorldSession::HandleZoneUpdateOpcode(WorldPacket& recvData)
{
    uint32 newZone;
    recvData >> newZone;

    sLog->outDetail("WORLD: Recvd ZONE_UPDATE: %u", newZone);

    // use server size data
    uint32 newzone, newarea;
    GetPlayer()->GetZoneAndAreaId(newzone, newarea);
    GetPlayer()->UpdateZone(newzone, newarea);
    //GetPlayer()->SendInitWorldStates(true, newZone);
}

void WorldSession::HandleSetSelectionOpcode(WorldPacket& recvData)
{
    uint64 guid;
    recvData >> guid;

    _player->SetSelection(guid);
}

void WorldSession::HandleStandStateChangeOpcode(WorldPacket& recvData)
{
    // sLog->outDebug(LOG_FILTER_PACKETIO, "WORLD: Received CMSG_STANDSTATECHANGE"); -- too many spam in log at lags/debug stop
    uint32 animstate;
    recvData >> animstate;

    _player->SetStandState(animstate);
}

void WorldSession::HandleContactListOpcode(WorldPacket& recvData)
{
    uint32 unk;
    recvData >> unk;
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_CONTACT_LIST - Unk: %d", unk);
    _player->GetSocial()->SendSocialList(_player);
}

void WorldSession::HandleAddFriendOpcode(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_ADD_FRIEND");

    std::string friendName = GetSkyFireString(LANGUAGE_FRIEND_IGNORE_UNKNOWN);
    std::string friendNote;

    recvData >> friendName;

    recvData >> friendNote;

    if (!normalizePlayerName(friendName))
        return;

    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: %s asked to add friend : '%s'", GetPlayer()->GetName(), friendName.c_str());

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_SELECT_GUID_RACE_ACC_BY_NAME);

    stmt->setString(0, friendName);

    _addFriendCallback.SetParam(friendNote);
    _addFriendCallback.SetFutureResult(CharacterDatabase.AsyncQuery(stmt));
}

void WorldSession::HandleAddFriendOpcodeCallBack(PreparedQueryResult result, std::string friendNote)
{
    if (!GetPlayer())
        return;

    uint64 friendGuid;
    uint32 friendAccountId;
    uint32 team;
    FriendsResult friendResult;

    friendResult = FRIEND_NOT_FOUND;
    friendGuid = 0;

    if (result)
    {
        Field* fields = result->Fetch();

        friendGuid = MAKE_NEW_GUID(fields[0].GetUInt32(), 0, HIGHGUID_PLAYER);
        team = Player::TeamForRace(fields[1].GetUInt8());
        friendAccountId = fields[2].GetUInt32();

        if (!AccountMgr::IsPlayerAccount(GetSecurity()) || sWorld->getBoolConfig(CONFIG_ALLOW_GM_FRIEND) || AccountMgr::IsPlayerAccount(AccountMgr::GetSecurity(friendAccountId, realmID)))
        {
            if (friendGuid)
            {
                if (friendGuid == GetPlayer()->GetGUID())
                    friendResult = FRIEND_SELF;
                else if (GetPlayer()->GetTeam() != team && !sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_ADD_FRIEND) && AccountMgr::IsPlayerAccount(GetSecurity()))
                    friendResult = FRIEND_ENEMY;
                else if (GetPlayer()->GetSocial()->HasFriend(GUID_LOPART(friendGuid)))
                    friendResult = FRIEND_ALREADY;
                else
                {
                    Player* pFriend = ObjectAccessor::FindPlayer(friendGuid);
                    if (pFriend && pFriend->IsInWorld() && pFriend->IsVisibleGloballyFor(GetPlayer()))
                        friendResult = FRIEND_ADDED_ONLINE;
                    else
                        friendResult = FRIEND_ADDED_OFFLINE;
                    if (!GetPlayer()->GetSocial()->AddToSocialList(GUID_LOPART(friendGuid), false))
                    {
                        friendResult = FRIEND_LIST_FULL;
                        sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: %s's friend list is full.", GetPlayer()->GetName());
                    }
                }
                GetPlayer()->GetSocial()->SetFriendNote(GUID_LOPART(friendGuid), friendNote);
            }
        }
    }

    sSocialMgr->SendFriendStatus(GetPlayer(), friendResult, GUID_LOPART(friendGuid), false);

    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Sent (SMSG_FRIEND_STATUS)");
}

void WorldSession::HandleDelFriendOpcode(WorldPacket& recvData)
{
    uint64 FriendGUID;

    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_DEL_FRIEND");

    recvData >> FriendGUID;

    _player->GetSocial()->RemoveFromSocialList(GUID_LOPART(FriendGUID), false);

    sSocialMgr->SendFriendStatus(GetPlayer(), FRIEND_REMOVED, GUID_LOPART(FriendGUID), false);

    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Sent motd (SMSG_FRIEND_STATUS)");
}

void WorldSession::HandleAddIgnoreOpcode(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_ADD_IGNORE");

    std::string ignoreName = GetSkyFireString(LANGUAGE_FRIEND_IGNORE_UNKNOWN);

    recvData >> ignoreName;

    if (!normalizePlayerName(ignoreName))
        return;

    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: %s asked to Ignore: '%s'",
        GetPlayer()->GetName(), ignoreName.c_str());

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_SELECT_GUID_BY_NAME);

    stmt->setString(0, ignoreName);

    _addIgnoreCallback = CharacterDatabase.AsyncQuery(stmt);
}

void WorldSession::HandleAddIgnoreOpcodeCallBack(PreparedQueryResult result)
{
    if (!GetPlayer())
        return;

    uint64 IgnoreGuid;
    FriendsResult ignoreResult;

    ignoreResult = FRIEND_IGNORE_NOT_FOUND;
    IgnoreGuid = 0;

    if (result)
    {
        IgnoreGuid = MAKE_NEW_GUID((*result)[0].GetUInt32(), 0, HIGHGUID_PLAYER);

        if (IgnoreGuid)
        {
            if (IgnoreGuid == GetPlayer()->GetGUID())              //not add yourself
                ignoreResult = FRIEND_IGNORE_SELF;
            else if (GetPlayer()->GetSocial()->HasIgnore(GUID_LOPART(IgnoreGuid)))
                ignoreResult = FRIEND_IGNORE_ALREADY;
            else
            {
                ignoreResult = FRIEND_IGNORE_ADDED;

                // ignore list full
                if (!GetPlayer()->GetSocial()->AddToSocialList(GUID_LOPART(IgnoreGuid), true))
                    ignoreResult = FRIEND_IGNORE_FULL;
            }
        }
    }

    sSocialMgr->SendFriendStatus(GetPlayer(), ignoreResult, GUID_LOPART(IgnoreGuid), false);

    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Sent (SMSG_FRIEND_STATUS)");
}

void WorldSession::HandleDelIgnoreOpcode(WorldPacket& recvData)
{
    uint64 IgnoreGUID;

    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_DEL_IGNORE");

    recvData >> IgnoreGUID;

    _player->GetSocial()->RemoveFromSocialList(GUID_LOPART(IgnoreGUID), true);

    sSocialMgr->SendFriendStatus(GetPlayer(), FRIEND_IGNORE_REMOVED, GUID_LOPART(IgnoreGUID), false);

    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Sent motd (SMSG_FRIEND_STATUS)");
}

void WorldSession::HandleSetContactNotesOpcode(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "CMSG_SET_CONTACT_NOTES");
    uint64 guid;
    std::string note;
    recvData >> guid >> note;
    _player->GetSocial()->SetFriendNote(GUID_LOPART(guid), note);
}

void WorldSession::HandleBugOpcode(WorldPacket& recvData)
{
    uint32 suggestion, contentlen, typelen;
    std::string content, type;

    recvData >> suggestion >> contentlen >> content;

    recvData >> typelen >> type;

    if (suggestion == 0)
        sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_BUG [Bug Report]");
    else
        sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_BUG [Suggestion]");

    sLog->outDebug(LOG_FILTER_NETWORKIO, "%s", type.c_str());
    sLog->outDebug(LOG_FILTER_NETWORKIO, "%s", content.c_str());

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_INSERT_BUG_REPORT);

    stmt->setString(0, type);
    stmt->setString(1, content);

    CharacterDatabase.Execute(stmt);
}

void WorldSession::HandleReclaimCorpseOpcode(WorldPacket &recvData)
{
    sLog->outDetail("WORLD: Received CMSG_RECLAIM_CORPSE");

    uint64 guid;
    recvData >> guid;

    if (GetPlayer()->isAlive())
        return;

    // do not allow corpse reclaim in arena
    if (GetPlayer()->InArena())
        return;

    // body not released yet
    if (!GetPlayer()->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST))
        return;

    Corpse* corpse = GetPlayer()->GetCorpse();

    if (!corpse)
        return;

    // prevent resurrect before 30-sec delay after body release not finished
    if (time_t(corpse->GetGhostTime() + GetPlayer()->GetCorpseReclaimDelay(corpse->GetType() == CORPSE_RESURRECTABLE_PVP)) > time_t(time(NULL)))
        return;

    if (!corpse->IsWithinDistInMap(GetPlayer(), CORPSE_RECLAIM_RADIUS, true))
        return;

    // resurrect
    GetPlayer()->ResurrectPlayer(GetPlayer()->InBattleground() ? 1.0f : 0.5f);

    // spawn bones
    GetPlayer()->SpawnCorpseBones();
}

void WorldSession::HandleResurrectResponseOpcode(WorldPacket& recvData)
{
    sLog->outDetail("WORLD: Received CMSG_RESURRECT_RESPONSE");

    uint64 guid;
    uint8 status;
    recvData >> guid;
    recvData >> status;

    if (GetPlayer()->isAlive())
        return;

    if (status == 0)
    {
        GetPlayer()->ClearResurrectRequestData();           // reject
        return;
    }

    if (!GetPlayer()->IsResurrectRequestedBy(guid))
        return;

    GetPlayer()->ResurectUsingRequestData();
}

void WorldSession::SendAreaTriggerMessage(const char* Text, ...)
{
    va_list ap;
    char szStr [1024];
    szStr[0] = '\0';

    va_start(ap, Text);
    vsnprintf(szStr, 1024, Text, ap);
    va_end(ap);

    uint32 length = strlen(szStr)+1;
    WorldPacket data(SMSG_AREA_TRIGGER_MESSAGE, 4+length);
    data << length;
    data << szStr;
    SendPacket(&data);
}

void WorldSession::HandleAreaTriggerOpcode(WorldPacket& recvData)
{
    uint32 triggerId;
    recvData >> triggerId;

    sLog->outDebug(LOG_FILTER_NETWORKIO, "CMSG_AREATRIGGER. Trigger ID: %u", triggerId);

    Player* player = GetPlayer();
    if (player->isInFlight())
    {
        sLog->outDebug(LOG_FILTER_NETWORKIO, "Player '%s' (GUID: %u) in flight, ignore Area Trigger ID:%u",
            player->GetName(), player->GetGUIDLow(), triggerId);
        return;
    }

    AreaTriggerEntry const* atEntry = sAreaTriggerStore.LookupEntry(triggerId);
    if (!atEntry)
    {
        sLog->outDebug(LOG_FILTER_NETWORKIO, "Player '%s' (GUID: %u) send unknown (by DBC) Area Trigger ID:%u",
            player->GetName(), player->GetGUIDLow(), triggerId);
        return;
    }

    if (player->GetMapId() != atEntry->mapid)
    {
        sLog->outDebug(LOG_FILTER_NETWORKIO, "Player '%s' (GUID: %u) too far (trigger map: %u player map: %u), ignore Area Trigger ID: %u",
            player->GetName(), atEntry->mapid, player->GetMapId(), player->GetGUIDLow(), triggerId);
        return;
    }

    // delta is safe radius
    const float delta = 5.0f;

    if (atEntry->radius > 0)
    {
        // if we have radius check it
        float dist = player->GetDistance(atEntry->x, atEntry->y, atEntry->z);
        if (dist > atEntry->radius + delta)
        {
            sLog->outDebug(LOG_FILTER_NETWORKIO, "Player '%s' (GUID: %u) too far (radius: %f distance: %f), ignore Area Trigger ID: %u",
                player->GetName(), player->GetGUIDLow(), atEntry->radius, dist, triggerId);
            return;
        }
    }
    else
    {
        // we have only extent

        // rotate the players position instead of rotating the whole cube, that way we can make a simplified
        // is-in-cube check and we have to calculate only one point instead of 4

        // 2PI = 360°, keep in mind that ingame orientation is counter-clockwise
        double rotation = 2 * M_PI - atEntry->box_orientation;
        double sinVal = sin(rotation);
        double cosVal = cos(rotation);

        float playerBoxDistX = player->GetPositionX() - atEntry->x;
        float playerBoxDistY = player->GetPositionY() - atEntry->y;

        float rotPlayerX = float(atEntry->x + playerBoxDistX * cosVal - playerBoxDistY*sinVal);
        float rotPlayerY = float(atEntry->y + playerBoxDistY * cosVal + playerBoxDistX*sinVal);

        // box edges are parallel to coordiante axis, so we can treat every dimension independently :D
        float dz = player->GetPositionZ() - atEntry->z;
        float dx = rotPlayerX - atEntry->x;
        float dy = rotPlayerY - atEntry->y;
        if ((fabs(dx) > atEntry->box_x / 2 + delta) ||
            (fabs(dy) > atEntry->box_y / 2 + delta) ||
            (fabs(dz) > atEntry->box_z / 2 + delta))
        {
            sLog->outDebug(LOG_FILTER_NETWORKIO, "Player '%s' (GUID: %u) too far (1/2 box X: %f 1/2 box Y: %f 1/2 box Z: %f rotatedPlayerX: %f rotatedPlayerY: %f dZ:%f), ignore Area Trigger ID: %u",
                player->GetName(), player->GetGUIDLow(), atEntry->box_x/2, atEntry->box_y/2, atEntry->box_z/2, rotPlayerX, rotPlayerY, dz, triggerId);
            return;
        }
    }

    if (player->isDebugAreaTriggers)
        ChatHandler(player).PSendSysMessage(LANGUAGE_DEBUG_AREATRIGGER_REACHED, triggerId);

    if (sScriptMgr->OnAreaTrigger(player, atEntry))
        return;

    uint32 quest_id = sObjectMgr->GetQuestForAreaTrigger(triggerId);
    if (quest_id && GetPlayer()->isAlive() && GetPlayer()->IsActiveQuest(quest_id))
    {
        Quest const* quest = sObjectMgr->GetQuestTemplate(quest_id);
        if (quest)
        {
            if (GetPlayer()->GetQuestStatus(quest_id) == QUEST_STATUS_INCOMPLETE)
                GetPlayer()->AreaExploredOrEventHappens(quest_id);
        }
    }

    if (GetPlayer()->GetMap()->IsDungeon())
    {
        uint32 QuestStartId = sObjectMgr->GetQuestStartForAreaTrigger(triggerId);
        Quest const* quest = sObjectMgr->GetQuestTemplate(QuestStartId);

        if (quest && (!GetPlayer()->IsActiveQuest(quest->GetQuestId())) && (GetPlayer()->GetQuestStatus(quest->GetQuestId()) != QUEST_STATUS_COMPLETE))
        {
            if (GetPlayer()->CanTakeQuest(quest, true))
                GetPlayer()->PlayerTalkClass->SendQuestGiverQuestDetails(quest, GetPlayer()->GetGUID(), true);
        }
    }

    if (sObjectMgr->IsTavernAreaTrigger(triggerId))
    {
        // set resting flag we are in the inn
        player->SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING);
        player->InnEnter(time(NULL), atEntry->mapid, atEntry->x, atEntry->y, atEntry->z);
        player->SetRestType(REST_TYPE_IN_TAVERN);

        if (sWorld->IsFFAPvPRealm())
            player->RemoveByteFlag(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_FFA_PVP);

        return;
    }

    if (Battleground* bg = player->GetBattleground())
        if (bg->GetStatus() == STATUS_IN_PROGRESS)
        {
            bg->HandleAreaTrigger(player, triggerId);
            return;
        }

    if (OutdoorPvP* pvp = player->GetOutdoorPvP())
        if (pvp->HandleAreaTrigger(_player, triggerId))
            return;

    AreaTrigger const* at = sObjectMgr->GetAreaTrigger(triggerId);;
    if (!at)
        return;

    bool teleported = false;
    if (player->GetMapId() != at->target_mapId)
    {
        if (!sMapMgr->CanPlayerEnter(at->target_mapId, player, false))
            return;

        if (Group* group = player->GetGroup())
            if (group->isLFGGroup() && player->GetMap()->IsDungeon())
                teleported = player->TeleportToBGEntryPoint();
    }

    if (!teleported)
        player->TeleportTo(at->target_mapId, at->target_X, at->target_Y, at->target_Z, at->target_Orientation, TELE_TO_NOT_LEAVE_TRANSPORT);
}

void WorldSession::HandleUpdateAccountData(WorldPacket &recvData)
{
    sLog->outDetail("WORLD: Received CMSG_UPDATE_ACCOUNT_DATA");

    uint32 type, timestamp, decompressedSize;
    recvData >> type >> timestamp >> decompressedSize;

    sLog->outDebug(LOG_FILTER_NETWORKIO, "UAD: type %u, time %u, decompressedSize %u", type, timestamp, decompressedSize);

    if (type > NUM_ACCOUNT_DATA_TYPES)
        return;

    if (decompressedSize == 0)                               // erase
    {
        SetAccountData(AccountDataType(type), 0, "");

        WorldPacket data(SMSG_UPDATE_ACCOUNT_DATA_COMPLETE, 4+4);
        data << uint32(type);
        data << uint32(0);
        SendPacket(&data);

        return;
    }

    if (decompressedSize > 0xFFFF)
    {
        recvData.rpos(recvData.wpos());                   // unnneded warning spam in this case
        sLog->outError("UAD: Account data packet too big, size %u", decompressedSize);
        return;
    }

    ByteBuffer dest;
    dest.resize(decompressedSize);

    uLongf realSize = decompressedSize;
    if (uncompress(const_cast<uint8*>(dest.contents()), &realSize, const_cast<uint8*>(recvData.contents() + recvData.rpos()), recvData.size() - recvData.rpos()) != Z_OK)
    {
        recvData.rpos(recvData.wpos());                   // unnneded warning spam in this case
        sLog->outError("UAD: Failed to decompress account data");
        return;
    }

    recvData.rpos(recvData.wpos());                       // uncompress read (recvData.size() - recvData.rpos())

    std::string adata;
    dest >> adata;

    SetAccountData(AccountDataType(type), timestamp, adata);

    WorldPacket data(SMSG_UPDATE_ACCOUNT_DATA_COMPLETE, 4+4);
    data << uint32(type);
    data << uint32(0);
    SendPacket(&data);
}

void WorldSession::HandleRequestAccountData(WorldPacket& recvData)
{
    sLog->outDetail("WORLD: Received CMSG_REQUEST_ACCOUNT_DATA");

    uint32 type;
    recvData >> type;

    sLog->outDebug(LOG_FILTER_NETWORKIO, "RAD: type %u", type);

    if (type > NUM_ACCOUNT_DATA_TYPES)
        return;

    AccountData* adata = GetAccountData(AccountDataType(type));

    uint32 size = adata->Data.size();

    uLongf destSize = compressBound(size);

    ByteBuffer dest;
    dest.resize(destSize);

    if (size && compress(const_cast<uint8*>(dest.contents()), &destSize, (uint8*)adata->Data.c_str(), size) != Z_OK)
    {
        sLog->outDebug(LOG_FILTER_NETWORKIO, "RAD: Failed to compress account data");
        return;
    }

    dest.resize(destSize);

    WorldPacket data(SMSG_UPDATE_ACCOUNT_DATA, 8+4+4+4+destSize);
    data << uint64(_player ? _player->GetGUID() : 0);       // player guid
    data << uint32(type);                                   // type (0-7)
    data << uint32(adata->Time);                            // unix time
    data << uint32(size);                                   // decompressed length
    data.append(dest);                                      // compressed data
    SendPacket(&data);
}

void WorldSession::HandleSetActionButtonOpcode(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_SET_ACTION_BUTTON");
    uint8 button;
    uint32 packetData;
    recvData >> button >> packetData;

    uint32 action = ACTION_BUTTON_ACTION(packetData);
    uint8  type   = ACTION_BUTTON_TYPE(packetData);

    sLog->outDetail("BUTTON: %u ACTION: %u TYPE: %u", button, action, type);
    if (!packetData)
    {
        sLog->outDetail("MISC: Remove action from button %u", button);
        GetPlayer()->removeActionButton(button);
    }
    else
    {
        switch (type)
        {
            case ACTION_BUTTON_MACRO:
            case ACTION_BUTTON_CMACRO:
                sLog->outDetail("MISC: Added Macro %u into button %u", action, button);
                break;
            case ACTION_BUTTON_EQSET:
                sLog->outDetail("MISC: Added EquipmentSet %u into button %u", action, button);
                break;
            case ACTION_BUTTON_SPELL:
                sLog->outDetail("MISC: Added Spell %u into button %u", action, button);
                break;
            case ACTION_BUTTON_ITEM:
                sLog->outDetail("MISC: Added Item %u into button %u", action, button);
                break;
			case ACTION_PETLIST:
				sLog->outDetail("MISC: Added pet list %u into button %u", action, button);
				break;
            default:
                sLog->outError("MISC: Unknown action button type %u for action %u into button %u for player %s (GUID: %u)", type, action, button, _player->GetName(), _player->GetGUIDLow());
                return;
        }
        GetPlayer()->addActionButton(button, action, type);
    }
}

void WorldSession::HandleCompleteCinematic(WorldPacket& /*recvData*/)
{
    sLog->outStaticDebug("WORLD: Player is watching cinema");
}

void WorldSession::HandleNextCinematicCamera(WorldPacket& /*recvData*/)
{
    sLog->outStaticDebug("WORLD: Which movie to play");
}

void WorldSession::HandleMoveTimeSkippedOpcode(WorldPacket& recvData)
{
    /*  WorldSession::Update(getMSTime());*/
    sLog->outStaticDebug("WORLD: Time Lag/Synchronization Resent/Update");

    uint64 guid;
    recvData.readPackGUID(guid);
    recvData.read_skip<uint32>();
    /*
        uint64 guid;
        uint32 time_skipped;
        recvData >> guid;
        recvData >> time_skipped;
        sLog->outDebug(LOG_FILTER_PACKETIO, "WORLD: CMSG_MOVE_TIME_SKIPPED");

        /// TODO
        must be need use in SkyFire
        We substract server Lags to move time (AntiLags)
        for exmaple
        GetPlayer()->ModifyLastMoveTime(-int32(time_skipped));
    */
}

void WorldSession::HandleFeatherFallAck(WorldPacket& recvData)
{
    sLog->outStaticDebug("WORLD: CMSG_MOVE_FEATHER_FALL_ACK");

    // no used
    recvData.rfinish();                       // prevent warnings spam
}

void WorldSession::HandleMoveUnRootAck(WorldPacket& recvData)
{
    // no used
    recvData.rfinish();                       // prevent warnings spam
/*
    uint64 guid;
    recvData >> guid;

    // now can skip not our packet
    if (_player->GetGUID() != guid)
    {
        recvData.rfinish();                   // prevent warnings spam
        return;
    }

    sLog->outDebug(LOG_FILTER_PACKETIO, "WORLD: CMSG_FORCE_MOVE_UNROOT_ACK");

    recvData.read_skip<uint32>();                          // unk

    MovementInfo movementInfo;
    movementInfo.guid = guid;
    ReadMovementInfo(recvData, &movementInfo);
    recvData.read_skip<float>();                           // unk2
*/
}

void WorldSession::HandleMoveRootAck(WorldPacket& recvData)
{
    // no used
    recvData.rfinish();                       // prevent warnings spam
/*
    uint64 guid;
    recvData >> guid;

    // now can skip not our packet
    if (_player->GetGUID() != guid)
    {
        recvData.rfinish();                   // prevent warnings spam
        return;
    }

    sLog->outDebug(LOG_FILTER_PACKETIO, "WORLD: CMSG_FORCE_MOVE_ROOT_ACK");

    recvData.read_skip<uint32>();                          // unk

    MovementInfo movementInfo;
    ReadMovementInfo(recvData, &movementInfo);
*/
}

void WorldSession::HandleSetActionBarToggles(WorldPacket& recvData)
{
    uint8 ActionBar;

    recvData >> ActionBar;

    if (!GetPlayer())                                        // ignore until not logged (check needed because STATUS_AUTHED)
    {
        if (ActionBar != 0)
            sLog->outError("WorldSession::HandleSetActionBarToggles in not logged state with value: %u, ignored", uint32(ActionBar));
        return;
    }

    GetPlayer()->SetByteValue(PLAYER_FIELD_BYTES, 2, ActionBar);
}

void WorldSession::HandlePlayedTime(WorldPacket& recvData)
{
    uint8 unk1;
    recvData >> unk1;                                      // 0 or 1 expected

    WorldPacket data(SMSG_PLAYED_TIME, 4 + 4 + 1);
    data << uint32(_player->GetTotalPlayedTime());
    data << uint32(_player->GetLevelPlayedTime());
    data << uint8(unk1);                                    // 0 - will not show in chat frame
    SendPacket(&data);
}

void WorldSession::HandleInspectOpcode(WorldPacket& recvData)
{
    uint64 guid;
    recvData >> guid;
    sLog->outStaticDebug("Inspected guid is " UI64FMTD, guid);

    _player->SetSelection(guid);

    Player* player = ObjectAccessor::FindPlayer(guid);
    if (!player)                                                // wrong player
        return;

    uint32 talent_points = 0x29;
    WorldPacket data(SMSG_INSPECT_TALENT, 8+4+talent_points);

    data << uint64(player->GetGUID());

    if (sWorld->getBoolConfig(CONFIG_TALENTS_INSPECTING) || _player->isGameMaster())
    {
        player->BuildPlayerTalentsInfoData(&data);
    }
    else
    {
        data << uint32(0);                                  // unspentTalentPoints
        data << uint8(0);                                   // talentGroupCount
        data << uint8(0);                                   // talentGroupIndex
    }

    player->BuildEnchantmentsInfoData(&data);
    if (uint32 guildId = player->GetGuildId())
    {
        if (Guild* guild = sGuildMgr->GetGuildById(guildId))
        {
            data << uint64(guild->GetId());           // guild id
            data << uint32(guild->GetLevel());        // guild level
            data << uint64(player->GetGUID());        // not sure
            data << uint32(guild->GetMembersCount()); // number of members
        }
    }
    SendPacket(&data);
}

void WorldSession::HandleInspectHonorStatsOpcode(WorldPacket& recvData)
{
    uint64 guid;
    recvData >> guid;

    Player* player = ObjectAccessor::FindPlayer(guid);

    if (!player)
    {
        sLog->outError("InspectHonorStats: WTF, player not found...");
        return;
    }

    WorldPacket data(SMSG_INSPECT_HONOR_STATS, 8+1+4*4);
    data << uint64(player->GetGUID());
    data << uint8(player->GetCurrency(CURRENCY_TYPE_HONOR_POINTS));
    data << uint32(player->GetUInt32Value(PLAYER_FIELD_KILLS));
    data << uint8(0);
    data << uint32(player->GetUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS));
    data << uint32(0);
    data << uint32(0);
    SendPacket(&data);
}

void WorldSession::HandleWorldTeleportOpcode(WorldPacket& recvData)
{
    // write in client console: worldport 469 452 6454 2536 180 or /console worldport 469 452 6454 2536 180
    // Received opcode CMSG_WORLD_TELEPORT
    // Time is ***, map=469, x=452.000000, y=6454.000000, z=2536.000000, orient=3.141593

    uint32 time;
    uint32 mapid;
    float PositionX;
    float PositionY;
    float PositionZ;
    float Orientation;

    recvData >> time;                                      // time in m.sec.
    recvData >> mapid;
    recvData >> PositionX;
    recvData >> PositionY;
    recvData >> PositionZ;
    recvData >> Orientation;                               // o (3.141593 = 180 degrees)

    //sLog->outDebug("Received opcode CMSG_WORLD_TELEPORT");
    if (GetPlayer()->isInFlight())
    {
        sLog->outDebug(LOG_FILTER_NETWORKIO, "Player '%s' (GUID: %u) in flight, ignore worldport command.", GetPlayer()->GetName(), GetPlayer()->GetGUIDLow());
        return;
    }

    sLog->outStaticDebug("Time %u sec, map=%u, x=%f, y=%f, z=%f, orient=%f", time/1000, mapid, PositionX, PositionY, PositionZ, Orientation);

    if (AccountMgr::IsAdminAccount(GetSecurity()))
        GetPlayer()->TeleportTo(mapid, PositionX, PositionY, PositionZ, Orientation);
    else
        SendNotification(LANGUAGE_YOU_NOT_HAVE_PERMISSION);
    sLog->outDebug(LOG_FILTER_NETWORKIO, "Received worldport command from player %s", GetPlayer()->GetName());
}

void WorldSession::HandleWhoisOpcode(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "Received opcode CMSG_WHOIS");
    std::string charname;
    recvData >> charname;

    if (!AccountMgr::IsAdminAccount(GetSecurity()))
    {
        SendNotification(LANGUAGE_YOU_NOT_HAVE_PERMISSION);
        return;
    }

    if (charname.empty() || !normalizePlayerName (charname))
    {
        SendNotification(LANGUAGE_NEED_CHARACTER_NAME);
        return;
    }

    Player* player = sObjectAccessor->FindPlayerByName(charname.c_str());

    if (!player)
    {
        SendNotification(LANGUAGE_PLAYER_NOT_EXIST_OR_OFFLINE, charname.c_str());
        return;
    }

    uint32 accid = player->GetSession()->GetAccountId();

    PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SELECT_ACCOUNT_WHOIS);

    stmt->setUInt32(0, accid);

    PreparedQueryResult result = LoginDatabase.Query(stmt);

    if (!result)
    {
        SendNotification(LANGUAGE_ACCOUNT_FOR_PLAYER_NOT_FOUND, charname.c_str());
        return;
    }

    Field* fields = result->Fetch();
    std::string acc = fields[0].GetString();
    if (acc.empty())
        acc = "Unknown";
    std::string email = fields[1].GetString();
    if (email.empty())
        email = "Unknown";
    std::string lastip = fields[2].GetString();
    if (lastip.empty())
        lastip = "Unknown";

    std::string msg = charname + "'s " + "account is " + acc + ", e-mail: " + email + ", last ip: " + lastip;

    WorldPacket data(SMSG_WHOIS, msg.size()+1);
    data << msg;
    _player->GetSession()->SendPacket(&data);

    sLog->outDebug(LOG_FILTER_NETWORKIO, "Received whois command from player %s for character %s", GetPlayer()->GetName(), charname.c_str());
}

void WorldSession::HandleComplainOpcode(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: CMSG_COMPLAIN");

    uint8 spam_type;                                        // 0 - mail, 1 - chat
    uint64 spammer_guid;
    uint32 unk1 = 0;
    uint32 unk2 = 0;
    uint32 unk3 = 0;
    uint32 unk4 = 0;
    std::string description = "";
    recvData >> spam_type;                                 // unk 0x01 const, may be spam type (mail/chat)
    recvData >> spammer_guid;                              // player guid
    switch (spam_type)
    {
        case 0:
            recvData >> unk1;                              // const 0
            recvData >> unk2;                              // probably mail id
            recvData >> unk3;                              // const 0
            break;
        case 1:
            recvData >> unk1;                              // probably language
            recvData >> unk2;                              // message type?
            recvData >> unk3;                              // probably channel id
            recvData >> unk4;                              // unk random value
            recvData >> description;                       // spam description string (messagetype, channel name, player name, message)
            break;
    }

    // NOTE: all chat messages from this spammer automatically ignored by spam reporter until logout in case chat spam.
    // if it's mail spam - ALL mails from this spammer automatically removed by client

    // Complaint Received message
    WorldPacket data(SMSG_COMPLAIN_RESULT, 1);
    data << uint8(0);
    SendPacket(&data);

    sLog->outDebug(LOG_FILTER_NETWORKIO, "REPORT SPAM: type %u, guid %u, unk1 %u, unk2 %u, unk3 %u, unk4 %u, message %s", spam_type, GUID_LOPART(spammer_guid), unk1, unk2, unk3, unk4, description.c_str());
}

void WorldSession::HandleRealmSplitOpcode(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "CMSG_REALM_SPLIT");

    uint32 unk;
    std::string split_date = "01/01/01";
    recvData >> unk;

    WorldPacket data(SMSG_REALM_SPLIT, 4+4+split_date.size()+1);
    data << unk;
    data << uint32(0x00000000);                             // realm split state
    // split states:
    // 0x0 realm normal
    // 0x1 realm split
    // 0x2 realm split pending
    data << split_date;
    SendPacket(&data);
    //sLog->outDebug("response sent %u", unk);
}

void WorldSession::HandleFarSightOpcode(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: CMSG_FAR_SIGHT");

    uint8 apply;
    recvData >> apply;

    switch (apply)
    {
        case 0:
            sLog->outDebug(LOG_FILTER_NETWORKIO, "Player %u set vision to self", _player->GetGUIDLow());
            _player->SetSeer(_player);
            break;
        case 1:
            sLog->outDebug(LOG_FILTER_NETWORKIO, "Added FarSight " UI64FMTD " to player %u", _player->GetUInt64Value(PLAYER_FARSIGHT), _player->GetGUIDLow());
            if (WorldObject* target = _player->GetViewpoint())
                _player->SetSeer(target);
            else
                sLog->outError("Player %s requests non-existing seer " UI64FMTD, _player->GetName(), _player->GetUInt64Value(PLAYER_FARSIGHT));
            break;
        default:
            sLog->outDebug(LOG_FILTER_NETWORKIO, "Unhandled mode in CMSG_FAR_SIGHT: %u", apply);
            return;
    }

    GetPlayer()->UpdateVisibilityForPlayer();
}

void WorldSession::HandleSetTitleOpcode(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "CMSG_SET_TITLE");

    int32 title;
    recvData >> title;

    // -1 at none
    if (title > 0 && title < MAX_TITLE_INDEX)
    {
       if (!GetPlayer()->HasTitle(title))
            return;
    }
    else
        title = 0;

    GetPlayer()->SetUInt32Value(PLAYER_CHOSEN_TITLE, title);
}

void WorldSession::HandleTimeSyncResp(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "CMSG_TIME_SYNC_RESP");

    uint32 counter, clientTicks;
    recvData >> counter >> clientTicks;

    if (counter != _player->m_timeSyncCounter - 1)
        sLog->outDebug(LOG_FILTER_NETWORKIO, "Wrong time sync counter from player %s (cheater?)", _player->GetName());

    sLog->outDebug(LOG_FILTER_NETWORKIO, "Time sync received: counter %u, client ticks %u, time since last sync %u", counter, clientTicks, clientTicks - _player->m_timeSyncClient);

    uint32 ourTicks = clientTicks + (getMSTime() - _player->m_timeSyncServer);

    // diff should be small
    sLog->outDebug(LOG_FILTER_NETWORKIO, "Our ticks: %u, diff %u, latency %u", ourTicks, ourTicks - clientTicks, GetLatency());

    _player->m_timeSyncClient = clientTicks;
}

void WorldSession::HandleResetInstancesOpcode(WorldPacket& /*recvData*/)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: CMSG_RESET_INSTANCES");

	if (Group* group = _player->GetGroup())
	{
		if (group->IsLeader(_player->GetGUID()))
			group->ResetInstances(INSTANCE_RESET_ALL, false, _player);
	}
    else
        _player->ResetInstances(INSTANCE_RESET_ALL, false);
}

void WorldSession::HandleSetDungeonDifficultyOpcode(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "MSG_SET_DUNGEON_DIFFICULTY");

    uint32 mode;
    recvData >> mode;

    if (mode >= MAX_DUNGEON_DIFFICULTY)
    {
        sLog->outError("WorldSession::HandleSetDungeonDifficultyOpcode: player %d sent an invalid instance mode %d!", _player->GetGUIDLow(), mode);
        return;
    }

    if (Difficulty(mode) == _player->GetDungeonDifficulty())
        return;

    // cannot reset while in an instance
    Map* map = _player->FindMap();
    if (map && map->IsDungeon())
    {
        sLog->outError("WorldSession::HandleSetDungeonDifficultyOpcode: player (Name: %s, GUID: %u) tried to reset the instance while player is inside!", _player->GetName(), _player->GetGUIDLow());
        return;
    }

    Group* group = _player->GetGroup();
    if (group)
    {
        if (group->IsLeader(_player->GetGUID()))
        {
            for (GroupReference* itr = group->GetFirstMember(); itr != NULL; itr = itr->next())
            {
                Player* pGroupGuy = itr->getSource();
                if (!pGroupGuy)
                    continue;

                if (!pGroupGuy->IsInMap(pGroupGuy))
                    return;

                if (pGroupGuy->GetMap()->IsNonRaidDungeon())
                {
                    sLog->outError("WorldSession::HandleSetDungeonDifficultyOpcode: player %d tried to reset the instance while group member (Name: %s, GUID: %u) is inside!", _player->GetGUIDLow(), pGroupGuy->GetName(), pGroupGuy->GetGUIDLow());
                    return;
                }
            }
            // the difficulty is set even if the instances can't be reset
            //_player->SendDungeonDifficulty(true);
            group->ResetInstances(INSTANCE_RESET_CHANGE_DIFFICULTY, false, _player);
            group->SetDungeonDifficulty(Difficulty(mode));
        }
    }
    else
    {
        _player->ResetInstances(INSTANCE_RESET_CHANGE_DIFFICULTY, false);
        _player->SetDungeonDifficulty(Difficulty(mode));
    }
}

void WorldSession::HandleSetRaidDifficultyOpcode(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "MSG_SET_RAID_DIFFICULTY");

    uint32 mode;
    recvData >> mode;

    if (mode >= MAX_RAID_DIFFICULTY)
    {
        sLog->outError("WorldSession::HandleSetRaidDifficultyOpcode: player %d sent an invalid instance mode %d!", _player->GetGUIDLow(), mode);
        return;
    }

    // cannot reset while in an instance
    Map* map = _player->FindMap();
    if (map && map->IsDungeon())
    {
        sLog->outError("WorldSession::HandleSetRaidDifficultyOpcode: player %d tried to reset the instance while inside!", _player->GetGUIDLow());
        return;
    }

    if (Difficulty(mode) == _player->GetRaidDifficulty())
        return;

    Group* group = _player->GetGroup();
    if (group)
    {
        if (group->IsLeader(_player->GetGUID()))
        {
            for (GroupReference* itr = group->GetFirstMember(); itr != NULL; itr = itr->next())
            {
                Player* pGroupGuy = itr->getSource();
                if (!pGroupGuy)
                    continue;

                if (!pGroupGuy->IsInMap(pGroupGuy))
                    return;

                if (pGroupGuy->GetMap()->IsRaid())
                {
                    sLog->outError("WorldSession::HandleSetRaidDifficultyOpcode: player %d tried to reset the instance while inside!", _player->GetGUIDLow());
                    return;
                }
            }
            // the difficulty is set even if the instances can't be reset
            //_player->SendDungeonDifficulty(true);
            group->ResetInstances(INSTANCE_RESET_CHANGE_DIFFICULTY, true, _player);
            group->SetRaidDifficulty(Difficulty(mode));
        }
    }
    else
    {
        _player->ResetInstances(INSTANCE_RESET_CHANGE_DIFFICULTY, true);
        _player->SetRaidDifficulty(Difficulty(mode));
    }
}

void WorldSession::HandleCancelMountAuraOpcode(WorldPacket& /*recvData*/)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: CMSG_CANCEL_MOUNT_AURA");

    //If player is not mounted, so go out :)
    if (!_player->IsMounted())                              // not blizz like; no any messages on blizz
    {
        ChatHandler(this).SendSysMessage(LANGUAGE_CHARACTER_NON_MOUNTED);
        return;
    }

    if (_player->isInFlight())                               // not blizz like; no any messages on blizz
    {
        ChatHandler(this).SendSysMessage(LANGUAGE_YOU_IN_FLIGHT);
        return;
    }

    _player->Dismount();
    _player->RemoveAurasByType(SPELL_AURA_MOUNTED);
}

void WorldSession::HandleMoveSetCanFlyAckOpcode(WorldPacket& recvData)
{
    // fly mode on/off
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: CMSG_MOVE_SET_CAN_FLY_ACK");

    uint64 guid;                                            // guid - unused
    recvData.readPackGUID(guid);

    recvData.read_skip<uint32>();                          // unk

    MovementInfo movementInfo;
    movementInfo.guid = guid;
    ReadMovementInfo(recvData, &movementInfo);

    recvData.read_skip<float>();                           // unk2

    _player->_mover->_movementInfo.flags = movementInfo.GetMovementFlags();
}

void WorldSession::HandleRequestPetInfoOpcode(WorldPacket& /*recvData */)
{
    /*
        sLog->outDebug(LOG_FILTER_PACKETIO, "WORLD: CMSG_REQUEST_PET_INFO");
        recvData.hexlike();
    */
}

void WorldSession::HandleSetTaxiBenchmarkOpcode(WorldPacket& recvData)
{
    uint8 mode;
    recvData >> mode;

    sLog->outDebug(LOG_FILTER_NETWORKIO, "Client used \"/timetest %d\" command", mode);
}

void WorldSession::HandleQueryInspectAchievements(WorldPacket& recvData)
{
    uint64 guid;
    recvData.readPackGUID(guid);

    Player* player = ObjectAccessor::FindPlayer(guid);
    if (!player)
        return;

    player->GetAchievementMgr().SendRespondInspectAchievements(_player);
}

void WorldSession::HandleGuildPartyStateUpdate(WorldPacket& /*recvData*/)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: CMSG_GUILD_UPDATE_PARTY_STATE");

    // TODO: implement

    if (Group* group = GetPlayer()->GetGroup())
        group->SendGuildGroupStateUpdate(group->IsGuildGroup(GetPlayer()->GetGuildId()));
}

void WorldSession::HandleWorldStateUITimerUpdate(WorldPacket& /*recvData*/)
{
    // empty opcode
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: CMSG_WORLD_STATE_UI_TIMER_UPDATE");

    WorldPacket data(SMSG_WORLD_STATE_UI_TIMER_UPDATE, 4);
    data << uint32(time(NULL));
    SendPacket(&data);
}

void WorldSession::HandleReadyForAccountDataTimes(WorldPacket& /*recvData*/)
{
    // empty opcode
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: CMSG_READY_FOR_ACCOUNT_DATA_TIMES");

    SendAccountDataTimes(GLOBAL_CACHE_MASK);
}

void WorldSession::SendSetPhaseShift(std::set<uint32> const& phaseIds, std::set<uint32> const& terrainswaps, std::set<uint32> const& worldmaps)
{
    uint64 guid = _player->GetGUID();

    // Note: As the actual values are sent as uint16, just remember to not set any of the 4 over 65535 (uint16 maximum), even if the DB table allows it.
    // Note 2:     // In sniffs these phase ID's are seen like this:
  /*| PhaseMask | PhaseID |
    |         1 |     169 |
    |         2 |     170 |
    |         4 |     171 |
    |         8 |     172 |
    |        16 |     173 |
    |        32 |     174 |
    |        64 |     175 |
    |       128 |     176 |
    |       256 |     177 | */

    bool hasPhases = phaseIds.empty() ? false : true;

    uint32 phaseFlags         = hasPhases ? 0x10 : 0x8;  // In sniffs, if phasesCount is 0 (player in default / all - map phase), 0x8. Specific: 0x10.

    uint32 phasesCount        = hasPhases ? phaseIds.size() : 0;
    uint32 activeSwapsCount   = terrainswaps.empty() ? 0 : terrainswaps.size();
    uint32 worldMapAreas      = worldmaps.empty() ? 0 : worldmaps.size();
    uint32 inactiveSwapsCount = 0; // Not implemented yet. These and Active swaps "shift" on off based on player zoning (in / out).

    WorldPacket data(SMSG_SET_PHASE_SHIFT, 1 + 8 + 4 + 4 + 4 + 4 + phaseIds.size() * 2 + 2 + terrainswaps.size() * 2 + 2 + worldmaps.size() * 2 + 2);

    // Phase flags - Flags 0x8 and 0x10 are related to areatriggers and affected areas.
    data << uint32(phaseFlags);     // Flags.

    // Phases the player is in - Phase.dbc IDs.
    data << uint32(phasesCount * 2);
    if (hasPhases)    
        for (std::set<uint32>::const_iterator itr = phaseIds.begin(); itr != phaseIds.end(); ++itr)
            data << uint16(*itr);

    // Map (M) display control - WorldMapArea.dbc IDs.
    data << uint32(worldMapAreas * 2);
    if (worldMapAreas > 0)
        for (std::set<uint32>::const_iterator itr = worldmaps.begin(); itr != worldmaps.end(); ++itr)
            data << uint16(*itr);

    // Active terrain swaps.
    data << uint32(activeSwapsCount * 2);
    if (activeSwapsCount > 0)
        for (std::set<uint32>::const_iterator itr = terrainswaps.begin(); itr != terrainswaps.end(); ++itr)
            data << uint16(*itr);

    // Inactive terrain swaps.
    data << uint32(inactiveSwapsCount);
    // if (inactiveSwapsCount > 0)
        // for (uint8 i = 0; i < inactiveSwapsCount; ++i)
            // data << uint16(0);

	data.WriteBit(4);
	data.WriteBit(6);
	data.WriteBit(1);
	data.WriteBit(7);
	data.WriteBit(2);
	data.WriteBit(0);
	data.WriteBit(5);
	data.WriteBit(3);

    data.FlushBits();

	data.WriteByteSeq(0);
	data.WriteByteSeq(4);
	data.WriteByteSeq(7);
	data.WriteByteSeq(6);
	data.WriteByteSeq(3);
	data.WriteByteSeq(5);
	data.WriteByteSeq(1);
	data.WriteByteSeq(2);

    SendPacket(&data);
}

void WorldSession::SendPhaseShift_Override(uint32 PhaseShift, uint32 MapID)
{
	if (!_player)
		return;

	WorldPacket data(SMSG_SET_PHASE_SHIFT, 4);
	data << uint64(_player->GetGUID());
	data << uint32(0);          // Count of bytes - Array1 - Unused
	data << uint32(0);          // Count of bytes - Array2 - TerrainSwap, unused.

	data << uint32(2);          // Count of bytes - Array3 - Phases
	data << uint16(PhaseShift);

	if (MapID)
	{
		data << uint32(2);          // Count of bytes - Array4 - TerrainSwap
		data << uint16(MapID);
	}
	else
		data << uint32(0);

	data << uint32(2); // Count of Bytes - worldmapID?
	data << uint16(MapID);

	if (!PhaseShift)
		data << uint32(0x08);
	else
		data << uint32(0);          // Flags (seem to be from Phase.dbc, not really sure)
	SendPacket(&data);
}

void WorldSession::SendPhaseShift_Override_Maps(uint32 PhaseShift, uint32 MapID, uint32 worldMapId)
{
	if (!_player)
		return;

	WorldPacket data(SMSG_SET_PHASE_SHIFT, 4);
	data << uint64(_player->GetGUID());
	data << uint32(0);          // Count of bytes - Array1 - Unused
	data << uint32(0);          // Count of bytes - Array2 - TerrainSwap, unused.

	data << uint32(2);          // Count of bytes - Array3 - Phases
	data << uint16(PhaseShift);

	if (MapID)
	{
		data << uint32(2);          // Count of bytes - Array4 - TerrainSwap
		data << uint16(MapID);
	}
	else
		data << uint32(0);

	if (worldMapId > 0)
	{
		data << uint32(2); // Count of Bytes - worldmapID?
		data << uint16(worldMapId);
	}

	if (!PhaseShift)
		data << uint32(0x08);
	else
		data << uint32(0);          // Flags (seem to be from Phase.dbc, not really sure)
	SendPacket(&data);
}

//Battlefield and Battleground
void WorldSession::HandleAreaSpiritHealerQueryOpcode(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: CMSG_AREA_SPIRIT_HEALER_QUERY");
    Battleground* bg = _player->GetBattleground();

    uint64 guid;
    recvData >> guid;

    Creature* unit = GetPlayer()->GetMap()->GetCreature(guid);
    if (!unit)
        return;

    if (!unit->isSpiritService())                            // it's not spirit service
        return;

    if (bg)
        sBattlegroundMgr->SendAreaSpiritHealerQueryOpcode(_player, bg, guid);

    if (Battlefield* bf = sBattlefieldMgr->GetBattlefieldToZoneId(_player->GetZoneId()))
        bf->SendAreaSpiritHealerQueryOpcode(_player, guid);
}

void WorldSession::HandleAreaSpiritHealerQueueOpcode(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: CMSG_AREA_SPIRIT_HEALER_QUEUE");
    Battleground* bg = _player->GetBattleground();

    uint64 guid;
    recvData >> guid;

    Creature* unit = GetPlayer()->GetMap()->GetCreature(guid);
    if (!unit)
        return;

    if (!unit->isSpiritService())                            // it's not spirit service
        return;

    if (bg)
        bg->AddPlayerToResurrectQueue(guid, _player->GetGUID());

    if (Battlefield* bf = sBattlefieldMgr->GetBattlefieldToZoneId(_player->GetZoneId()))
        bf->AddPlayerToResurrectQueue(guid, _player->GetGUID());
}

void WorldSession::HandleHearthAndResurrect(WorldPacket& /*recvData*/)
{
    if (_player->isInFlight())
        return;

    if (Battlefield* bf = sBattlefieldMgr->GetBattlefieldToZoneId(_player->GetZoneId()))
    {
        // bf->PlayerAskToLeave(_player); FIXME
        return;
    }

    AreaTableEntry const *atEntry = GetAreaEntryByAreaID(_player->GetAreaId());
    if (!atEntry || !(atEntry->flags & AREA_FLAG_CAN_HEARTH_AND_RES))
        return;

    _player->BuildPlayerRepop();
    _player->ResurrectPlayer(100);
    _player->TeleportTo(_player->_homebindMapId, _player->_homebindX, _player->_homebindY, _player->_homebindZ, _player->GetOrientation());
}

void WorldSession::HandleInstanceLockResponse(WorldPacket& recvPacket)
{
    uint8 accept;
    recvPacket >> accept;

    if (!_player->HasPendingBind())
    {
        sLog->outDetail("InstanceLockResponse: Player %s (guid %u) tried to bind himself/teleport to graveyard without a pending bind!", _player->GetName(), _player->GetGUIDLow());
        return;
    }

    if (accept)
        _player->BindToInstance();
    else
        _player->RepopAtGraveyard();

    _player->SetPendingBind(0, 0);
}
