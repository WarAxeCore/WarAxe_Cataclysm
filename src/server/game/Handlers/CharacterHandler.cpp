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
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "ArenaTeamMgr.h"
#include "GuildMgr.h"
#include "SystemConfig.h"
#include "World.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "DatabaseEnv.h"
#include "CharacterDatabase.h"
#include "ArenaTeam.h"
#include "Chat.h"
#include "Group.h"
#include "Guild.h"
#include "Language.h"
#include "Log.h"
#include "Opcodes.h"
#include "Player.h"
#include "PlayerDump.h"
#include "SharedDefines.h"
#include "SocialMgr.h"
#include "UpdateMask.h"
#include "Util.h"
#include "ScriptMgr.h"
#include "Battleground.h"
#include "BattlefieldWG.h"
#include "BattlefieldTB.h"
#include "BattlefieldMgr.h"
#include "AccountMgr.h"
#include "LFGMgr.h"

class LoginQueryHolder : public SQLQueryHolder
{
private:
    uint32 m_accountId;
    uint64 m_guid;
public:
    LoginQueryHolder(uint32 accountId, uint64 guid)
        : m_accountId(accountId), m_guid(guid) { }
    uint64 GetGuid() const { return m_guid; }
    uint32 GetAccountId() const { return m_accountId; }
    bool Initialize();
};

bool LoginQueryHolder::Initialize()
{
    SetSize(MAX_PLAYER_LOGIN_QUERY);

    bool res = true;
    uint32 lowGuid = GUID_LOPART(m_guid);

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_SELECT_CHARACTER);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_FROM, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_SELECT_GROUP_MEMBER);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_GROUP, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_SELECT_CHARACTER_INSTANCE);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_BOUND_INSTANCES, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_SELECT_CHARACTER_AURAS);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_AURAS, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_SELECT_CHARACTER_SPELL);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_SPELLS, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_SELECT_CHARACTER_QUEST_STATUS);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_QUEST_STATUS, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_SELECT_CHARACTER_DAILY_QUEST_STATUS);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_DAILY_QUEST_STATUS, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_SELECT_CHARACTER_WEEKLY_QUEST_STATUS);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_WEEKLY_QUEST_STATUS, stmt);

    //stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_SELECT_CHARACTER_MONTHLY_QUEST_STATUS);
    //stmt->setUInt32(0, lowGuid);
    //res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_MONTHLY_QUEST_STATUS, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_SELECT_CHARACTER_SEASONAL_QUEST_STATUS);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_SEASONAL_QUEST_STATUS, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_SELECT_CHARACTER_REPUTATION);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_REPUTATION, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_SELECT_CHARACTER_INVENTORY);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_INVENTORY, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_SELECT_CHARACTER_ACTIONS);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_ACTIONS, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_SELECT_CHARACTER_MAILCOUNT);
    stmt->setUInt32(0, lowGuid);
    stmt->setUInt64(1, uint64(time(NULL)));
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_MAIL_COUNT, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_SELECT_CHARACTER_MAILDATE);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_MAIL_DATE, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_SELECT_CHARACTER_SOCIAL_LIST);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_SOCIAL_LIST, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_SELECT_CHARACTER_HOME_BIND);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_HOMEBIND, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_SELECT_CHARACTER_SPELL_COOLDOWNS);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_SPELL_COOLDOWNS, stmt);

    if (sWorld->getBoolConfig(CONFIG_DECLINED_NAMES_USED))
    {
        stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_SELECT_CHARACTER_DECLINED_NAMES);
        stmt->setUInt32(0, lowGuid);
        res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_DECLINED_NAMES, stmt);
    }

    stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_SELECT_CHARACTER_GUILD);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_GUILD, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_SELECT_CHARACTER_ARENA_INFO);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_ARENA_INFO, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_SELECT_CHARACTER_ACHIEVEMENTS);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_ACHIEVEMENTS, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_SELECT_CHARACTER_CRITERIA_PROGRESS);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_CRITERIA_PROGRESS, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_SELECT_CHARACTER_EQUIPMENT_SETS);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_EQUIPMENT_SETS, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_SELECT_CHARACTER_BG_DATA);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_BG_DATA, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_SELECT_CHARACTER_GLYPHS);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_GLYPHS, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_SELECT_CHARACTER_TALENTS);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_TALENTS, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_SELECT_CHARACTER_ACCOUNT_DATA);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_ACCOUNT_DATA, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_SELECT_CHARACTER_SKILLS);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_SKILLS, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_SELECT_CHARACTER_RANDOM_BG);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_RANDOM_BG, stmt);

	stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_SELECT_CHARACTER_CURRENCY);
	stmt->setUInt64(0, lowGuid);
	res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_CURRENCY, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_SELECT_CHARACTER_BANNED);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_BANNED, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_SELECT_CHARACTER_QUEST_STATUS_REW);
    stmt->setUInt32(0, lowGuid);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_QUEST_STATUS_REW, stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_SELECT_ACCOUNT_INSTANCE_LOCK_TIMES);
    stmt->setUInt32(0, m_accountId);
    res &= SetPreparedQuery(PLAYER_LOGIN_QUERY_LOAD_INSTANCE_LOCK_TIMES, stmt);

    return res;
}

void WorldSession::HandleCharEnum(PreparedQueryResult result)
{
    WorldPacket data(SMSG_CHARACTER_ENUM, 100);                  // we guess size

    uint8 num = 0;

    data << num;

    _allowedCharsToLogin.clear();
    if (result)
    {
        do
        {
            uint32 guidlow = (*result)[0].GetUInt32();

            if (_allowedCharsToLogin.find(guidlow) != _allowedCharsToLogin.end()) // Fixes a glitch when a player had multiple pets in the same slot
                continue;

            sLog->outDetail("Loading char guid %u from account %u.", guidlow, GetAccountId());
            if (Player::BuildEnumData(result, &data))
            {
                _allowedCharsToLogin.insert(guidlow);
                ++num;
            }
        }
        while (result->NextRow());
    }

    data.put<uint8>(0, num);

    SendPacket(&data);
}

void WorldSession::HandleCharEnumOpcode(WorldPacket & /*recvData*/)
{
    // remove expired bans
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_DELETE_EXPIRED_BANS);
    CharacterDatabase.Execute(stmt);

    /// get all the data necessary for loading all characters (along with their pets) on the account

    if (sWorld->getBoolConfig(CONFIG_DECLINED_NAMES_USED))
    {
        stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_SELECT_ENUM_DECLINED_NAME);
        stmt->setUInt8(0, PET_SLOT_DEFAULT);
        stmt->setUInt32(1, GetAccountId());
    }
    else
    {
        stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_SELECT_ENUM);
        stmt->setUInt32(0, GetAccountId());
    }

    stmt->setUInt32(0, GetAccountId());

    _charEnumCallback = CharacterDatabase.AsyncQuery(stmt);
}

void WorldSession::HandleCharCreateOpcode(WorldPacket& recvData)
{
    std::string name;
    uint8 race_, class_;

    recvData >> name;

    recvData >> race_;
    recvData >> class_;

    // extract other data required for player creating
    uint8 gender, skin, face, hairStyle, hairColor, facialHair, outfitId;
    recvData >> gender >> skin >> face;
    recvData >> hairStyle >> hairColor >> facialHair >> outfitId;

    WorldPacket data(SMSG_CHARACTER_CREATE, 1);                  // returned with diff.values in all cases

    if (AccountMgr::IsPlayerAccount(GetSecurity()))
    {
        if (uint32 mask = sWorld->getIntConfig(CONFIG_CHARACTER_CREATING_DISABLED))
        {
            bool disabled = false;

            uint32 team = Player::TeamForRace(race_);
            switch (team)
            {
                case ALLIANCE: disabled = mask & (1 << 0);
                    break;
                case HORDE: disabled = mask & (1 << 1);
                    break;
            }

            if (disabled)
            {
                data << (uint8)CHAR_CREATE_DISABLED;
                SendPacket(&data);
                return;
            }
        }
    }

    ChrClassesEntry const* classEntry = sChrClassesStore.LookupEntry(class_);
    if (!classEntry)
    {
        data << (uint8)CHAR_CREATE_FAILED;
        SendPacket(&data);
        sLog->outError("Class (%u) not found in DBC while creating new char for account (ID: %u): wrong DBC files or cheater?", class_, GetAccountId());
        return;
    }

    ChrRacesEntry const* raceEntry = sChrRacesStore.LookupEntry(race_);
    if (!raceEntry)
    {
        data << (uint8)CHAR_CREATE_FAILED;
        SendPacket(&data);
        sLog->outError("Race (%u) not found in DBC while creating new char for account (ID: %u): wrong DBC files or cheater?", race_, GetAccountId());
        return;
    }

    // prevent character creating Expansion race without Expansion account
    if (raceEntry->expansion > Expansion())
    {
        data << (uint8)CHAR_CREATE_EXPANSION;
        sLog->outError("Expansion %u account:[%d] tried to Create character with expansion %u race (%u)", Expansion(), GetAccountId(), raceEntry->expansion, race_);
        SendPacket(&data);
        return;
    }

    // prevent character creating Expansion class without Expansion account
    if (classEntry->expansion > Expansion())
    {
        data << (uint8)CHAR_CREATE_EXPANSION_CLASS;
        sLog->outError("Expansion %u account:[%d] tried to Create character with expansion %u class (%u)", Expansion(), GetAccountId(), classEntry->expansion, class_);
        SendPacket(&data);
        return;
    }

    if (AccountMgr::IsPlayerAccount(GetSecurity()))
    {
        uint32 raceMaskDisabled = sWorld->getIntConfig(CONFIG_CHARACTER_CREATING_DISABLED_RACEMASK);
        if ((1 << (race_ - 1)) & raceMaskDisabled)
        {
            data << uint8(CHAR_CREATE_DISABLED);
            SendPacket(&data);
            return;
        }

        uint32 classMaskDisabled = sWorld->getIntConfig(CONFIG_CHARACTER_CREATING_DISABLED_CLASSMASK);
        if ((1 << (class_ - 1)) & classMaskDisabled)
        {
            data << uint8(CHAR_CREATE_DISABLED);
            SendPacket(&data);
            return;
        }
    }

    // prevent character creating with invalid name
    if (!normalizePlayerName(name))
    {
        data << (uint8)CHAR_NAME_NO_NAME;
        SendPacket(&data);
        sLog->outError("Account:[%d] but tried to Create character with empty [name] ", GetAccountId());
        return;
    }

    // check name limitations
    uint8 res = ObjectMgr::CheckPlayerName(name, true);
    if (res != CHAR_NAME_SUCCESS)
    {
        data << uint8(res);
        SendPacket(&data);
        return;
    }

    if (AccountMgr::IsPlayerAccount(GetSecurity()) && sObjectMgr->IsReservedName(name))
    {
        data << (uint8)CHAR_NAME_RESERVED;
        SendPacket(&data);
        return;
    }

    // speedup check for heroic class disabled case
    uint32 heroic_free_slots = sWorld->getIntConfig(CONFIG_HEROIC_CHARACTERS_PER_REALM);
    if (heroic_free_slots == 0 && AccountMgr::IsPlayerAccount(GetSecurity()) && class_ == CLASS_DEATH_KNIGHT)
    {
        data << (uint8)CHAR_CREATE_UNIQUE_CLASS_LIMIT;
        SendPacket(&data);
        return;
    }

    // speedup check for heroic class disabled case
    uint32 req_level_for_heroic = sWorld->getIntConfig(CONFIG_CHARACTER_CREATING_MIN_LEVEL_FOR_HEROIC_CHARACTER);
    if (AccountMgr::IsPlayerAccount(GetSecurity()) && class_ == CLASS_DEATH_KNIGHT && req_level_for_heroic > sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
    {
        data << (uint8)CHAR_CREATE_LEVEL_REQUIREMENT;
        SendPacket(&data);
        return;
    }

    delete _charCreateCallback.GetParam();  // Delete existing if any, to make the callback chain reset to stage 0
    _charCreateCallback.Reset();
    _charCreateCallback.SetParam(new CharacterCreateInfo(name, race_, class_, gender, skin, face, hairStyle, hairColor, facialHair, outfitId, recvData));
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_SELECT_CHECK_NAME);
    stmt->setString(0, name);
    _charCreateCallback.SetFutureResult(CharacterDatabase.AsyncQuery(stmt));
}

void WorldSession::HandleCharCreateCallback(PreparedQueryResult result, CharacterCreateInfo* createInfo)
{
    /** This is a series of callbacks executed consecutively as a result from the database becomes available.
        This is much more efficient than synchronous requests on packet handler, and much less DoS prone.
        It also prevents data syncrhonisation errors.
    */
    switch (_charCreateCallback.GetStage())
    {
        case 0:
        {
            if (result)
            {
                WorldPacket data(SMSG_CHARACTER_CREATE, 1);
                data << uint8(CHAR_CREATE_NAME_IN_USE);
                SendPacket(&data);
                delete createInfo;
                _charCreateCallback.Reset();
                return;
            }

            ASSERT(_charCreateCallback.GetParam() == createInfo);

            PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_GET_SUM_REALMCHARS);
            stmt->setUInt32(0, GetAccountId());

            _charCreateCallback.FreeResult();
            _charCreateCallback.SetFutureResult(LoginDatabase.AsyncQuery(stmt));
            _charCreateCallback.NextStage();
        }
        break;
        case 1:
        {
            uint16 acctCharCount = 0;
            if (result)
            {
                Field* fields = result->Fetch();
                // SELECT SUM(x) is MYSQL_TYPE_NEWDECIMAL - needs to be read as string
                const char* ch = fields[0].GetCString();
                if (ch)
                    acctCharCount = atoi(ch);
            }

            if (acctCharCount >= sWorld->getIntConfig(CONFIG_CHARACTERS_PER_ACCOUNT))
            {
                WorldPacket data(SMSG_CHARACTER_CREATE, 1);
                data << uint8(CHAR_CREATE_ACCOUNT_LIMIT);
                SendPacket(&data);
                delete createInfo;
                _charCreateCallback.Reset();
                return;
            }

            ASSERT(_charCreateCallback.GetParam() == createInfo);

            PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_SELECT_SUM_CHARS);
            stmt->setUInt32(0, GetAccountId());

            _charCreateCallback.FreeResult();
            _charCreateCallback.SetFutureResult(CharacterDatabase.AsyncQuery(stmt));
            _charCreateCallback.NextStage();
        }
        break;
        case 2:
        {
            if (result)
            {
                Field* fields = result->Fetch();
                createInfo->CharCount = fields[0].GetUInt8();

                if (createInfo->CharCount >= sWorld->getIntConfig(CONFIG_CHARACTERS_PER_REALM))
                {
                    WorldPacket data(SMSG_CHARACTER_CREATE, 1);
                    data << uint8(CHAR_CREATE_SERVER_LIMIT);
                    SendPacket(&data);
                    delete createInfo;
                    _charCreateCallback.Reset();
                    return;
                }
            }

            bool allowTwoSideAccounts = !sWorld->IsPvPRealm() || sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_ACCOUNTS) || !AccountMgr::IsPlayerAccount(GetSecurity());
            uint32 skipCinematics = sWorld->getIntConfig(CONFIG_SKIP_CINEMATICS);

            _charCreateCallback.FreeResult();

            if (!allowTwoSideAccounts || skipCinematics == 1 || createInfo->Class == CLASS_DEATH_KNIGHT)
            {
                PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_SELECT_CHARACTER_CREATE_INFO);
                stmt->setUInt32(0, GetAccountId());
                stmt->setUInt32(1, (skipCinematics == 1 || createInfo->Class == CLASS_DEATH_KNIGHT) ? 10 : 1);
                _charCreateCallback.SetFutureResult(CharacterDatabase.AsyncQuery(stmt));
                _charCreateCallback.NextStage();
                return;
            }

            _charCreateCallback.NextStage();
            HandleCharCreateCallback(PreparedQueryResult(NULL), createInfo);   // Will jump to case 3
        }
        break;
        case 3:
        {
            bool haveSameRace = false;
            uint32 heroicReqLevel = sWorld->getIntConfig(CONFIG_CHARACTER_CREATING_MIN_LEVEL_FOR_HEROIC_CHARACTER);
            bool hasHeroicReqLevel = (heroicReqLevel == 0);
            bool allowTwoSideAccounts = !sWorld->IsPvPRealm() || sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_ACCOUNTS) || !AccountMgr::IsPlayerAccount(GetSecurity());
            uint32 skipCinematics = sWorld->getIntConfig(CONFIG_SKIP_CINEMATICS);

            if (result)
            {
                uint32 team = Player::TeamForRace(createInfo->Race);
                uint32 freeHeroicSlots = sWorld->getIntConfig(CONFIG_HEROIC_CHARACTERS_PER_REALM);

                Field* field = result->Fetch();
                uint8 accRace  = field[1].GetUInt8();

                if (AccountMgr::IsPlayerAccount(GetSecurity()) && createInfo->Class == CLASS_DEATH_KNIGHT)
                {
                    uint8 accClass = field[2].GetUInt8();
                    if (accClass == CLASS_DEATH_KNIGHT)
                    {
                        if (freeHeroicSlots > 0)
                            --freeHeroicSlots;

                        if (freeHeroicSlots == 0)
                        {
                            WorldPacket data(SMSG_CHARACTER_CREATE, 1);
                            data << uint8(CHAR_CREATE_UNIQUE_CLASS_LIMIT);
                            SendPacket(&data);
                            delete createInfo;
                            _charCreateCallback.Reset();
                            return;
                        }
                    }

                    if (!hasHeroicReqLevel)
                    {
                        uint8 accLevel = field[0].GetUInt8();
                        if (accLevel >= heroicReqLevel)
                            hasHeroicReqLevel = true;
                    }
                }

                // need to check team only for first character
                // TODO: what to if account already has characters of both races?
                if (!allowTwoSideAccounts)
                {
                    uint32 accTeam = 0;
                    if (accRace > 0)
                        accTeam = Player::TeamForRace(accRace);

                    if (accTeam != team)
                    {
                        WorldPacket data(SMSG_CHARACTER_CREATE, 1);
                        data << uint8(CHAR_CREATE_PVP_TEAMS_VIOLATION);
                        SendPacket(&data);
                        delete createInfo;
                        _charCreateCallback.Reset();
                        return;
                    }
                }

                // search same race for cinematic or same class if need
                // TODO: check if cinematic already shown? (already logged in?; cinematic field)
                while ((skipCinematics == 1 && !haveSameRace) || createInfo->Class == CLASS_DEATH_KNIGHT)
                {
                    if (!result->NextRow())
                        break;

                    field = result->Fetch();
                    accRace = field[1].GetUInt32();

                    if (!haveSameRace)
                        haveSameRace = createInfo->Race == accRace;

                    if (AccountMgr::IsPlayerAccount(GetSecurity()) && createInfo->Class == CLASS_DEATH_KNIGHT)
                    {
                        uint8 acc_class = field[2].GetUInt32();
                        if (acc_class == CLASS_DEATH_KNIGHT)
                        {
                            if (freeHeroicSlots > 0)
                                --freeHeroicSlots;

                            if (freeHeroicSlots == 0)
                            {
                                WorldPacket data(SMSG_CHARACTER_CREATE, 1);
                                data << uint8(CHAR_CREATE_UNIQUE_CLASS_LIMIT);
                                SendPacket(&data);
                                delete createInfo;
                                _charCreateCallback.Reset();
                                return;
                            }
                        }

                        if (!hasHeroicReqLevel)
                        {
                            uint8 acc_level = field[0].GetUInt32();
                            if (acc_level >= heroicReqLevel)
                                hasHeroicReqLevel = true;
                        }
                    }
                }
            }

            if (AccountMgr::IsPlayerAccount(GetSecurity()) && createInfo->Class == CLASS_DEATH_KNIGHT && !hasHeroicReqLevel)
            {
                WorldPacket data(SMSG_CHARACTER_CREATE, 1);
                data << uint8(CHAR_CREATE_LEVEL_REQUIREMENT);
                SendPacket(&data);
                delete createInfo;
                _charCreateCallback.Reset();
                return;
            }

            if (createInfo->Data.rpos() < createInfo->Data.wpos())
                sLog->outDebug(LOG_FILTER_NETWORKIO, "Character creation %s (account %u) has unhandled tail data.", createInfo->Name.c_str(), GetAccountId());

            Player newChar(this);
            newChar.GetMotionMaster()->Initialize();
            if (!newChar.Create(sObjectMgr->GenerateLowGuid(HIGHGUID_PLAYER), createInfo))
            {
                // Player not create (race/class/etc problem?)
                newChar.CleanupsBeforeDelete();

                WorldPacket data(SMSG_CHARACTER_CREATE, 1);
                data << uint8(CHAR_CREATE_ERROR);
                SendPacket(&data);
                delete createInfo;
                _charCreateCallback.Reset();
                return;
            }

            if ((haveSameRace && skipCinematics == 1) || skipCinematics == 2)
                newChar.setCinematic(1);                          // not show intro

            newChar.SetAtLoginFlag(AT_LOGIN_FIRST);               // First login

            // Player created, save it now
            newChar.SaveToDB(true);
            createInfo->CharCount += 1;

            SQLTransaction trans = LoginDatabase.BeginTransaction();

            PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_DELETE_REALMCHARACTERS);
            stmt->setUInt32(0, GetAccountId());
            stmt->setUInt32(1, realmID);
            trans->Append(stmt);

            stmt = LoginDatabase.GetPreparedStatement(LOGIN_ADD_REALMCHARACTERS);
            stmt->setUInt32(0, createInfo->CharCount);
            stmt->setUInt32(1, GetAccountId());
            stmt->setUInt32(2, realmID);
            trans->Append(stmt);

            LoginDatabase.CommitTransaction(trans);

            QueryResult result2 = CharacterDatabase.PQuery("SELECT id FROM character_pet ORDER BY id DESC LIMIT 1");
            uint32 pet_id = 1;
            if (result2)
            {
                Field* fields = result2->Fetch();
                pet_id = fields[0].GetUInt32();
                pet_id += 1;
            }

            if (createInfo->Class == CLASS_WARLOCK)
            {
                CharacterDatabase.PExecute("REPLACE INTO character_pet (`id`, `entry`, `owner`, `modelid`, `CreatedBySpell`, `PetType`, `level`, `exp`, `Reactstate`, `name`, `renamed`, `slot`, `curhealth`, `curmana`, `curhappiness`, `savetime`, `abdata`) VALUES (%u, 416, %u, 4449, 688, 0, 1, 0, 1, 'Imp', 1, 100, 282, 72, 0, 1295721046, '7 2 7 1 7 0 129 3110 1 0 1 0 1 0 6 2 6 1 6 0 ')", pet_id, newChar.GetGUIDLow());
                CharacterDatabase.PExecute("UPDATE characters SET currentPetSlot = '100', petSlotUsed = '3452816845' WHERE guid = %u", newChar.GetGUIDLow());
                newChar.SetTemporaryUnsummonedPetNumber(pet_id);
            }

            if (createInfo->Class == CLASS_HUNTER)
            {
                switch (createInfo->Race)
                {
                    case RACE_HUMAN:
                        CharacterDatabase.PExecute("REPLACE INTO character_pet (`id`, `entry`, `owner`, `modelid`, `CreatedBySpell`, `PetType`, `level`, `exp`, `Reactstate`, `name`, `renamed`, `slot`, `curhealth`, `curmana`, `curhappiness`, `savetime`, `abdata`) VALUES (%u, 42717, %u, 903, 883, 1, 1, 0, 1, 'Wolf', 0, 0, 192, 0, 166500, 1295727347, '7 2 7 1 7 0 129 2649 129 17253 1 0 1 0 6 2 6 1 6 0 ')", pet_id, newChar.GetGUIDLow());
                        break;
                    case RACE_DWARF:
                        CharacterDatabase.PExecute("REPLACE INTO character_pet (`id`, `entry`, `owner`, `modelid`, `CreatedBySpell`, `PetType`, `level`, `exp`, `Reactstate`, `name`, `renamed`, `slot`, `curhealth`, `curmana`, `curhappiness`, `savetime`, `abdata`) VALUES (%u, 42713, %u, 822, 883, 1, 1, 0, 1, 'Bear', 0, 0, 212, 0, 166500, 1295727650, '7 2 7 1 7 0 129 2649 129 16827 1 0 1 0 6 2 6 1 6 0 ')", pet_id, newChar.GetGUIDLow());
                        break;
                    case RACE_ORC:
                        CharacterDatabase.PExecute("REPLACE INTO character_pet (`id`, `entry`, `owner`, `modelid`, `CreatedBySpell`, `PetType`, `level`, `exp`, `Reactstate`, `name`, `renamed`, `slot`, `curhealth`, `curmana`, `curhappiness`, `savetime`, `abdata`) VALUES (%u, 42719, %u, 744, 883, 1, 1, 0, 1, 'Boar', 0, 0, 212, 0, 166500, 1295727175, '7 2 7 1 7 0 129 2649 129 17253 1 0 1 0 6 2 6 1 6 0 ')", pet_id, newChar.GetGUIDLow());
                        break;
                    case RACE_NIGHTELF:
                        CharacterDatabase.PExecute("REPLACE INTO character_pet (`id`, `entry`, `owner`, `modelid`, `CreatedBySpell`, `PetType`, `level`, `exp`, `Reactstate`, `name`, `renamed`, `slot`, `curhealth`, `curmana`, `curhappiness`, `savetime`, `abdata`) VALUES (%u, 42718, %u,  17090, 883, 1, 1, 0, 1, 'Cat', 0, 0, 192, 0, 166500, 1295727501, '7 2 7 1 7 0 129 2649 129 16827 1 0 1 0 6 2 6 1 6 0 ')", pet_id, newChar.GetGUIDLow());
                        break;
                    case RACE_UNDEAD_PLAYER:
                        CharacterDatabase.PExecute("REPLACE INTO character_pet (`id`, `entry`, `owner`, `modelid`, `CreatedBySpell`, `PetType`, `level`, `exp`, `Reactstate`, `name`, `renamed`, `slot`, `curhealth`, `curmana`, `curhappiness`, `savetime`, `abdata`) VALUES (%u, 51107, %u,  368, 883, 1, 1, 0, 1, 'Spider', 0, 0, 202, 0, 166500, 1295727821, '7 2 7 1 7 0 129 2649 129 17253 1 0 1 0 6 2 6 1 6 0 ')", pet_id, newChar.GetGUIDLow());
                        break;
                    case RACE_TAUREN:
                        CharacterDatabase.PExecute("REPLACE INTO character_pet (`id`, `entry`, `owner`, `modelid`, `CreatedBySpell`, `PetType`, `level`, `exp`, `Reactstate`, `name`, `renamed`, `slot`, `curhealth`, `curmana`, `curhappiness`, `savetime`, `abdata`) VALUES (%u, 42720, %u,  29057, 883, 1, 1, 0, 1, 'Tallstrider', 0, 0, 192, 0, 166500, 1295727912, '7 2 7 1 7 0 129 2649 129 16827 1 0 1 0 6 2 6 1 6 0 ')", pet_id, newChar.GetGUIDLow());
                        break;
                    case RACE_TROLL:
                        CharacterDatabase.PExecute("REPLACE INTO character_pet (`id`, `entry`, `owner`, `modelid`, `CreatedBySpell`, `PetType`, `level`, `exp`, `Reactstate`, `name`, `renamed`, `slot`, `curhealth`, `curmana`, `curhappiness`, `savetime`, `abdata`) VALUES (%u, 42721, %u,  23518, 883, 1, 1, 0, 1, 'Raptor', 0, 0, 192, 0, 166500, 1295727987, '7 2 7 1 7 0 129 2649 129 50498 129 16827 1 0 6 2 6 1 6 0 ')", pet_id, newChar.GetGUIDLow());
                        break;
                    case RACE_GOBLIN:
                        CharacterDatabase.PExecute("REPLACE INTO character_pet (`id`, `entry`, `owner`, `modelid`, `CreatedBySpell`, `PetType`, `level`, `exp`, `Reactstate`, `name`, `renamed`, `slot`, `curhealth`, `curmana`, `curhappiness`, `savetime`, `abdata`) VALUES (%u, 42715, %u, 27692, 883, 1, 1, 0, 1, 'Crab', 0, 0, 212, 0, 166500, 1295720595, '7 2 7 1 7 0 129 2649 129 16827 1 0 1 0 6 2 6 1 6 0 ')", pet_id, newChar.GetGUIDLow());
                        break;
                    case RACE_BLOODELF:
                        CharacterDatabase.PExecute("REPLACE INTO character_pet (`id`, `entry`, `owner`, `modelid`, `CreatedBySpell`, `PetType`, `level`, `exp`, `Reactstate`, `name`, `renamed`, `slot`, `curhealth`, `curmana`, `curhappiness`, `savetime`, `abdata`) VALUES (%u, 42710, %u, 23515, 883, 1, 1, 0, 1, 'Dragonhawk', 0, 0, 202, 0, 166500, 1295728068, '7 2 7 1 7 0 129 2649 129 17253 1 0 1 0 6 2 6 1 6 0 ')", pet_id, newChar.GetGUIDLow());
                        break;
                    case RACE_DRAENEI:
                        CharacterDatabase.PExecute("REPLACE INTO character_pet (`id`, `entry`, `owner`, `modelid`, `CreatedBySpell`, `PetType`, `level`, `exp`, `Reactstate`, `name`, `renamed`, `slot`, `curhealth`, `curmana`, `curhappiness`, `savetime`, `abdata`) VALUES (%u, 42712, %u, 29056, 883, 1, 1, 0, 1, 'Moth', 0, 0, 192, 0, 166500, 1295728128, '7 2 7 1 7 0 129 2649 129 49966 1 0 1 0 6 2 6 1 6 0 ')", pet_id, newChar.GetGUIDLow());
                        break;
                    case RACE_WORGEN:
                        CharacterDatabase.PExecute("REPLACE INTO character_pet (`id`, `entry`, `owner`, `modelid`, `CreatedBySpell`, `PetType`, `level`, `exp`, `Reactstate`, `name`, `renamed`, `slot`, `curhealth`, `curmana`, `curhappiness`, `savetime`, `abdata`) VALUES (%u, 42722, %u, 30221, 883, 1, 1, 0, 1, 'Dog', 0, 0, 192, 0, 166500, 1295728219, '7 2 7 1 7 0 129 2649 129 17253 1 0 1 0 6 2 6 1 6 0 ')", pet_id, newChar.GetGUIDLow());
                        break;
                }

                CharacterDatabase.PExecute("UPDATE characters SET currentPetSlot = '0', petSlotUsed = '1' WHERE guid = %u", newChar.GetGUIDLow());
                newChar.SetTemporaryUnsummonedPetNumber(pet_id);
            }

            newChar.CleanupsBeforeDelete();

            WorldPacket data(SMSG_CHARACTER_CREATE, 1);
            data << uint8(CHAR_CREATE_SUCCESS);
            SendPacket(&data);

            std::string IP_str = GetRemoteAddress();
            sLog->outDetail("Account: %d (IP: %s) Create Character:[%s] (GUID: %u)", GetAccountId(), IP_str.c_str(), createInfo->Name.c_str(), newChar.GetGUIDLow());
            sLog->outChar("Account: %d (IP: %s) Create Character:[%s] (GUID: %u)", GetAccountId(), IP_str.c_str(), createInfo->Name.c_str(), newChar.GetGUIDLow());
            sScriptMgr->OnPlayerCreate(&newChar);
            sWorld->AddCharacterNameData(newChar.GetGUIDLow(), std::string(newChar.GetName()), newChar.getGender(), newChar.getRace(), newChar.getClass());

            delete createInfo;
            _charCreateCallback.SetParam(NULL);
            _charCreateCallback.FreeResult();
        }
        break;
    }
}

void WorldSession::HandleCharDeleteOpcode(WorldPacket& recvData)
{
    uint64 guid;
    recvData >> guid;

    // can't delete loaded character
    if (ObjectAccessor::FindPlayer(guid))
        return;

    uint32 accountId = 0;
    std::string name;

    // is guild leader
    if (sGuildMgr->GetGuildByLeader(guid))
    {
        WorldPacket data(SMSG_CHARACTER_DELETE, 1);
        data << (uint8)CHARACTER_DELETE_FAILED_GUILD_LEADER;
        SendPacket(&data);
        return;
    }

    // is arena team captain
    if (sArenaTeamMgr->GetArenaTeamByCaptain(guid))
    {
        WorldPacket data(SMSG_CHARACTER_DELETE, 1);
        data << (uint8)CHARACTER_DELETE_FAILED_ARENA_CAPTAIN;
        SendPacket(&data);
        return;
    }

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_SELECT_ACCOUNT_NAME_BY_GUID);

    stmt->setUInt32(0, GUID_LOPART(guid));

    PreparedQueryResult result = CharacterDatabase.Query(stmt);
    
    if (result)
    {
        Field *fields = result->Fetch();
        accountId     = fields[0].GetUInt32();
        name          = fields[1].GetString();
    }

    // prevent deleting other players' characters using cheating tools
    if (accountId != GetAccountId())
        return;

    std::string IP_str = GetRemoteAddress();
    sLog->outDetail("Account: %d (IP: %s) Delete Character:[%s] (GUID: %u)", GetAccountId(), IP_str.c_str(), name.c_str(), GUID_LOPART(guid));
    sLog->outChar("Account: %d (IP: %s) Delete Character:[%s] (GUID: %u)", GetAccountId(), IP_str.c_str(), name.c_str(), GUID_LOPART(guid));
    sScriptMgr->OnPlayerDelete(guid);

    if (sLog->IsOutCharDump())                                // optimize GetPlayerDump call
    {
        std::string dump;
        if (PlayerDumpWriter().GetDump(GUID_LOPART(guid), dump))
            sLog->outCharDump(dump.c_str(), GetAccountId(), GUID_LOPART(guid), name.c_str());
    }

    Player::DeleteFromDB(guid, GetAccountId());

    WorldPacket data(SMSG_CHARACTER_DELETE, 1);
    data << (uint8)CHARACTER_DELETE_SUCCESS;
    SendPacket(&data);
}

void WorldSession::HandleWorldLoginOpcode(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Recvd World Login Message");
    uint32 unk;
    uint8 unk1;
    recvData >> unk >> unk1;
}

void WorldSession::HandlePlayerLoginOpcode(WorldPacket& recvData)
{
    if (PlayerLoading() || GetPlayer() != NULL)
    {
        sLog->outError("Player tryes to login again, AccountId = %d", GetAccountId());
        return;
    }

    m_playerLoading = true;
    uint64 playerGuid = 0;

    sLog->outStaticDebug("WORLD: Recvd Player Logon Message");

    recvData >> playerGuid;

    if (!CharCanLogin(GUID_LOPART(playerGuid)))
    {
        sLog->outError("Account (%u) can't login with that character (%u).", GetAccountId(), GUID_LOPART(playerGuid));
        KickPlayer();
        return;
    }

    LoginQueryHolder *holder = new LoginQueryHolder(GetAccountId(), playerGuid);
    if (!holder->Initialize())
    {
        delete holder;                                      // delete all unprocessed queries
        m_playerLoading = false;
        return;
    }

    _charLoginCallback = CharacterDatabase.DelayQueryHolder((SQLQueryHolder*)holder);
}

void WorldSession::HandlePlayerLogin(LoginQueryHolder* holder)
{
    uint64 playerGuid = holder->GetGuid();

    Player* pCurrChar = new Player(this);
     // for send server info and strings (config)
    ChatHandler chH = ChatHandler(pCurrChar);

    // "GetAccountId() == db stored account id" checked in LoadFromDB (prevent login not own character using cheating tools)
    if (!pCurrChar->LoadFromDB(GUID_LOPART(playerGuid), holder))
    {
        SetPlayer(NULL);
        KickPlayer();                                       // disconnect client, player no set to session and it will not deleted or saved at kick
        delete pCurrChar;                                   // delete it manually
        delete holder;                                      // delete all unprocessed queries
        m_playerLoading = false;
        return;
    }

    pCurrChar->GetMotionMaster()->Initialize();
    pCurrChar->SendDungeonDifficulty(false);

    WorldPacket data(SMSG_LOGIN_VERIFY_WORLD, 20);
    data << pCurrChar->GetMapId();
    data << pCurrChar->GetPositionX();
    data << pCurrChar->GetPositionY();
    data << pCurrChar->GetPositionZ();
    data << pCurrChar->GetOrientation();
    SendPacket(&data);

    // load player specific part before send times
    LoadAccountData(holder->GetPreparedResult(PLAYER_LOGIN_QUERY_LOAD_ACCOUNT_DATA), PER_CHARACTER_CACHE_MASK);
    SendAccountDataTimes(PER_CHARACTER_CACHE_MASK);

    data.Initialize(SMSG_FEATURE_SYSTEM_STATUS, 2);         // added in 2.2.0
    data << uint8(2);                                       // unknown value
    data << uint8(0);                                       // enable(1)/disable(0) voice chat interface in client
    SendPacket(&data);

    // Send MOTD
    {
        data.Initialize(SMSG_MOTD, 50);                     // new in 2.0.1
        data << (uint32)0;

        uint32 linecount=0;
        std::string str_motd = sWorld->GetMotd();
        std::string::size_type pos, nextpos;

        pos = 0;
        while ((nextpos= str_motd.find('@', pos)) != std::string::npos)
        {
            if (nextpos != pos)
            {
                data << str_motd.substr(pos, nextpos-pos);
                ++linecount;
            }
            pos = nextpos+1;
        }

        if (pos<str_motd.length())
        {
            data << str_motd.substr(pos);
            ++linecount;
        }

        data.put(0, linecount);

        SendPacket(&data);
        sLog->outStaticDebug("WORLD: Sent motd (SMSG_MOTD)");

        // send server info
        if (sWorld->getIntConfig(CONFIG_ENABLE_SINFO_LOGIN) == 1)
        {
            uint32 PlayersNum = sWorld->GetPlayerCount();
            uint32 MaxPlayersNum = sWorld->GetMaxPlayerCount();
            std::string uptime = secsToTimeString(sWorld->GetUptime());

            chH.PSendSysMessage(_CLIENT_BUILD_REVISION_2);
            chH.PSendSysMessage("Revision Hash: " _HASH);
            chH.PSendSysMessage("Build Date: " _DATE);
            chH.PSendSysMessage(LANGUAGE_CONNECTED_PLAYERS, PlayersNum, MaxPlayersNum);
            chH.PSendSysMessage(LANGUAGE_UPTIME, uptime.c_str());

            sLog->outStaticDebug("WORLD: Sent server info");
        }
    }

    //QueryResult *result = CharacterDatabase.PQuery("SELECT guildid, rank FROM guild_member WHERE guid = '%u'", pCurrChar->GetGUIDLow());
    if (PreparedQueryResult resultGuild = holder->GetPreparedResult(PLAYER_LOGIN_QUERY_LOAD_GUILD))
    {
        Field* fields = resultGuild->Fetch();
        pCurrChar->SetInGuild(fields[0].GetUInt32());
        pCurrChar->SetRank(fields[1].GetUInt8());
        if (Guild* guild = sGuildMgr->GetGuildById(pCurrChar->GetGuildId()))
            pCurrChar->SetUInt32Value(PLAYER_GUILDLEVEL, guild->GetLevel());
    }
    else if (pCurrChar->GetGuildId())                        // clear guild related fields in case wrong data about non existed membership
    {
        pCurrChar->SetInGuild(0);
        pCurrChar->SetRank(0);
        pCurrChar->SetUInt32Value(PLAYER_GUILDLEVEL, 0);
    }

    if (pCurrChar->GetGuildId() != 0)
    {
        if (Guild* guild = sGuildMgr->GetGuildById(pCurrChar->GetGuildId()))
            guild->SendLoginInfo(this);
        else
        {
            // remove wrong guild data
            sLog->outError("Player %s (GUID: %u) marked as member of a guild that does not exist(id: %u), removing guild membership for player.", pCurrChar->GetName(), pCurrChar->GetGUIDLow(), pCurrChar->GetGuildId());
            pCurrChar->SetInGuild(0);
        }
    }

    data.Initialize(SMSG_LEARNED_DANCE_MOVES, 8);
    data << uint64(0);
    SendPacket(&data);

    pCurrChar->SendInitialPacketsBeforeAddToMap();

    //Show cinematic at the first time that player login
    if (!pCurrChar->getCinematic())
    {
        pCurrChar->setCinematic(1);

        if (ChrClassesEntry const* cEntry = sChrClassesStore.LookupEntry(pCurrChar->getClass()))
        {
            if (cEntry->CinematicSequence)
                pCurrChar->SendCinematicStart(cEntry->CinematicSequence);
            else if (ChrRacesEntry const* rEntry = sChrRacesStore.LookupEntry(pCurrChar->getRace()))
                pCurrChar->SendCinematicStart(rEntry->CinematicSequence);

            // send new char string if not empty
            if (!sWorld->GetNewCharString().empty())
                chH.PSendSysMessage("%s", sWorld->GetNewCharString().c_str());
        }
    }

    if (Group* group = pCurrChar->GetGroup())
    {
        if (group->isLFGGroup())
        {
            LfgDungeonSet Dungeons;
            Dungeons.insert(sLFGMgr->GetDungeon(group->GetGUID()));
            sLFGMgr->SetSelectedDungeons(pCurrChar->GetGUID(), Dungeons);
            sLFGMgr->SetState(pCurrChar->GetGUID(), sLFGMgr->GetState(group->GetGUID()));
        }
    }

    if (!pCurrChar->GetMap()->AddPlayerToMap(pCurrChar) || !pCurrChar->CheckInstanceLoginValid())
    {
        AreaTrigger const* at = sObjectMgr->GetGoBackTrigger(pCurrChar->GetMapId());
        if (at)
            pCurrChar->TeleportTo(at->target_mapId, at->target_X, at->target_Y, at->target_Z, pCurrChar->GetOrientation());
        else
            pCurrChar->TeleportTo(pCurrChar->_homebindMapId, pCurrChar->_homebindX, pCurrChar->_homebindY, pCurrChar->_homebindZ, pCurrChar->GetOrientation());
    }

    sObjectAccessor->AddObject(pCurrChar);
    //sLog->outDebug("Player %s added to Map.", pCurrChar->GetName());

    //Send WG timer to player at login
    if (sWorld->getBoolConfig(CONFIG_WINTERGRASP_ENABLE))
    {
        if (Battlefield *bfWG = sBattlefieldMgr->GetBattlefieldByBattleId(1))
        {
            if (bfWG->IsWarTime())
                pCurrChar->SendUpdateWorldState(WGClockWorldState[1], uint32(time(NULL)));
            else // Time to next battle
                pCurrChar->SendUpdateWorldState(WGClockWorldState[1], uint32(bfWG->GetTimer()));
        }
    }

    //Send TB timer to player at login
    if (sWorld->getBoolConfig(CONFIG_TOL_BARAD_ENABLE))
    {
        if (Battlefield * bfTB = sBattlefieldMgr->GetBattlefieldToZoneId(5095))
        {
            if (bfTB->IsWarTime())
                pCurrChar->SendUpdateWorldState(TBClockWorldState[1], uint32(time(NULL)));
            else // Time to next battle
                pCurrChar->SendUpdateWorldState(TBClockWorldState[1], uint32(bfTB->GetTimer()));
        }
    }

    pCurrChar->SendInitialPacketsAfterAddToMap();

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_UPDATE_CHARACTER_ONLINE);

    stmt->setUInt32(0, pCurrChar->GetGUIDLow());

    CharacterDatabase.Execute(stmt);

    stmt = LoginDatabase.GetPreparedStatement(LOGIN_UPDATE_ACCOUNT_ONLINE);

    stmt->setUInt32(0, GetAccountId());

    LoginDatabase.Execute(stmt);

    pCurrChar->SetInGameTime(getMSTime());

    // announce group about member online (must be after add to player list to receive announce to self)
    if (Group* group = pCurrChar->GetGroup())
    {
        //pCurrChar->groupInfo.group->SendInit(this); // useless
        group->SendUpdate();
        group->ResetMaxEnchantingLevel();
    }

    // friend status
    sSocialMgr->SendFriendStatus(pCurrChar, FRIEND_ONLINE, pCurrChar->GetGUIDLow(), true);

    // Place character in world (and load zone) before some object loading
    pCurrChar->LoadCorpse();

    // setting Ghost+speed if dead
    if (pCurrChar->_deathState != ALIVE)
    {
        // not blizz like, we must correctly save and load player instead...
        if (pCurrChar->getRace() == RACE_NIGHTELF)
            pCurrChar->CastSpell(pCurrChar, 20584, true, 0);// auras SPELL_AURA_INCREASE_SPEED(+speed in wisp form), SPELL_AURA_INCREASE_SWIM_SPEED(+swim speed in wisp form), SPELL_AURA_TRANSFORM (to wisp form)
        pCurrChar->CastSpell(pCurrChar, 8326, true, 0);     // auras SPELL_AURA_GHOST, SPELL_AURA_INCREASE_SPEED(why?), SPELL_AURA_INCREASE_SWIM_SPEED(why?)

        pCurrChar->SetMovement(MOVE_WATER_WALK);
    }

    pCurrChar->ContinueTaxiFlight();

    // reset for all pets before pet loading
    if (pCurrChar->HasAtLoginFlag(AT_LOGIN_RESET_PET_TALENTS))
        Pet::resetTalentsForAllPetsOf(pCurrChar);

    // Load pet if any (if player not alive and in taxi flight or another then pet will remember as temporary unsummoned)
    pCurrChar->LoadPet();
    pCurrChar->GetSession()->SendStablePet(0);  // testing atm

    // Set FFA PvP for non GM in non-rest mode
    if (sWorld->IsFFAPvPRealm() && !pCurrChar->isGameMaster() && !pCurrChar->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING))
        pCurrChar->SetByteFlag(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_FFA_PVP);

    if (pCurrChar->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_CONTESTED_PVP))
        pCurrChar->SetContestedPvP();

    // Apply at_login requests
    if (pCurrChar->HasAtLoginFlag(AT_LOGIN_RESET_SPELLS))
    {
        pCurrChar->resetSpells();
        SendNotification(LANGUAGE_RESET_SPELLS);
    }

    if (pCurrChar->HasAtLoginFlag(AT_LOGIN_RESET_TALENTS))
    {
        pCurrChar->ResetTalents(true);
        pCurrChar->SendTalentsInfoData(false);              // original talents send already in to SendInitialPacketsBeforeAddToMap, resend reset state
        SendNotification(LANGUAGE_RESET_TALENTS);
    }

    if (pCurrChar->HasAtLoginFlag(AT_LOGIN_FIRST))
        pCurrChar->RemoveAtLoginFlag(AT_LOGIN_FIRST);

    // show time before shutdown if shutdown planned.
    if (sWorld->IsShuttingDown())
        sWorld->ShutdownMsg(true, pCurrChar);

    if (sWorld->getBoolConfig(CONFIG_ALL_TAXI_PATHS))
        pCurrChar->SetTaxiCheater(true);

    if (pCurrChar->isGameMaster())
        SendNotification(LANGUAGE_GM_ON);

    std::string IP_str = GetRemoteAddress();
    sLog->outChar("Account: %d (IP: %s) Login Character:[%s] (GUID: %u)",
        GetAccountId(), IP_str.c_str(), pCurrChar->GetName(), pCurrChar->GetGUIDLow());

    if (!pCurrChar->IsStandState() && !pCurrChar->HasUnitState(UNIT_STATE_STUNNED))
        pCurrChar->SetStandState(UNIT_STAND_STATE_STAND);

    m_playerLoading = false;

    pCurrChar->CastMasterySpells(pCurrChar);

	if (pCurrChar)
	{
		QueryResult result = CharacterDatabase.PQuery("SELECT phaseMask FROM character_phase WHERE guid = '%u'", GetGuidLow());

		if (result)
		{
			do
			{
				Field* fields = result->Fetch();

				uint32 phaseMaskId = fields[0].GetUInt32();

				if (phaseMaskId > 0)
				{
					pCurrChar->SetPhaseMask(phaseMaskId, true);
				}
				else
				{
					pCurrChar->SetPhaseMask(1, true);
				}
			} while (result->NextRow());
		}
	}

	//Set terrainswaps
	if (pCurrChar->GetMapId() == 654) // Gilneas
	{
		if (pCurrChar->GetQuestStatus(14386) == QUEST_STATUS_REWARDED && pCurrChar->GetQuestStatus(14466) != QUEST_STATUS_REWARDED)
		{
			pCurrChar->GetSession()->SendPhaseShift_Override(0, 655);
		}
		if (pCurrChar->GetQuestStatus(14466) == QUEST_STATUS_REWARDED)
		{
			pCurrChar->GetSession()->SendPhaseShift_Override(0, 656);
		}
	}

    sScriptMgr->OnPlayerLogin(pCurrChar);
    delete holder;
}

void WorldSession::HandleSetFactionAtWar(WorldPacket& recvData)
{
    sLog->outStaticDebug("WORLD: Received CMSG_SET_FACTION_ATWAR");

    uint32 repListID;
    uint8  flag;

    recvData >> repListID;
    recvData >> flag;

    GetPlayer()->GetReputationMgr().SetAtWar(repListID, flag);
}

//I think this function is never used :/ I dunno, but i guess this opcode not exists
void WorldSession::HandleSetFactionCheat(WorldPacket & /*recvData*/)
{
    sLog->outError("WORLD SESSION: HandleSetFactionCheat, not expected call, please report.");
    GetPlayer()->GetReputationMgr().SendStates();
}

void WorldSession::HandleMeetingStoneInfo(WorldPacket & /*recvData*/)
{
    sLog->outStaticDebug("WORLD: Received CMSG_MEETING_STONE_INFO");

    //SendLfgUpdate(0, 0, 0);
}

void WorldSession::HandleTutorialFlag(WorldPacket& recvData)
{
    uint32 data;
    recvData >> data;

    uint8 index = uint8(data / 32);
    if (index >= MAX_ACCOUNT_TUTORIAL_VALUES)
        return;

    uint32 value = (data % 32);

    uint32 flag = GetTutorialInt(index);
    flag |= (1 << value);
    SetTutorialInt(index, flag);
}

void WorldSession::HandleTutorialClear(WorldPacket & /*recvData*/)
{
    for (uint8 i = 0; i < MAX_ACCOUNT_TUTORIAL_VALUES; ++i)
        SetTutorialInt(i, 0xFFFFFFFF);
}

void WorldSession::HandleTutorialReset(WorldPacket & /*recvData*/)
{
    for (uint8 i = 0; i < MAX_ACCOUNT_TUTORIAL_VALUES; ++i)
        SetTutorialInt(i, 0x00000000);
}

void WorldSession::HandleSetWatchedFactionOpcode(WorldPacket& recvData)
{
    sLog->outStaticDebug("WORLD: Received CMSG_SET_WATCHED_FACTION");
    uint32 fact;
    recvData >> fact;
    GetPlayer()->SetUInt32Value(PLAYER_FIELD_WATCHED_FACTION_INDEX, fact);
}

void WorldSession::HandleSetFactionInactiveOpcode(WorldPacket& recvData)
{
    sLog->outStaticDebug("WORLD: Received CMSG_SET_FACTION_INACTIVE");
    uint32 replistid;
    uint8 inactive;
    recvData >> replistid >> inactive;

    _player->GetReputationMgr().SetInactive(replistid, inactive);
}

void WorldSession::HandleShowingHelmOpcode(WorldPacket& recvData)
{
    sLog->outStaticDebug("CMSG_SHOWING_HELM for %s", _player->GetName());
    recvData.read_skip<uint8>(); // unknown, bool?
    _player->ToggleFlag(PLAYER_FLAGS, PLAYER_FLAGS_HIDE_HELM);
}

void WorldSession::HandleShowingCloakOpcode(WorldPacket& recvData)
{
    sLog->outStaticDebug("CMSG_SHOWING_CLOAK for %s", _player->GetName());
    recvData.read_skip<uint8>(); // unknown, bool?
    _player->ToggleFlag(PLAYER_FLAGS, PLAYER_FLAGS_HIDE_CLOAK);
}

void WorldSession::HandleCharRenameOpcode(WorldPacket& recvData)
{
    uint64 guid;
    std::string newName;

    recvData >> guid;
    recvData >> newName;

    // prevent character rename to invalid name
    if (!normalizePlayerName(newName))
    {
        WorldPacket data(SMSG_CHARACTER_RENAME, 1);
        data << uint8(CHAR_NAME_NO_NAME);
        SendPacket(&data);
        return;
    }

    uint8 res = ObjectMgr::CheckPlayerName(newName, true);
    if (res != CHAR_NAME_SUCCESS)
    {
        WorldPacket data(SMSG_CHARACTER_RENAME, 1);
        data << uint8(res);
        SendPacket(&data);
        return;
    }

    // check name limitations
    if (AccountMgr::IsPlayerAccount(GetSecurity()) && sObjectMgr->IsReservedName(newName))
    {
        WorldPacket data(SMSG_CHARACTER_RENAME, 1);
        data << uint8(CHAR_NAME_RESERVED);
        SendPacket(&data);
        return;
    }

    // make sure that the character belongs to the current account, that rename at login is enabled
    // and that there is no character with the desired new name
    _charRenameCallback.SetParam(newName);

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_SELECT_FREE_NAME);

    stmt->setUInt32(0, GUID_LOPART(guid));
    stmt->setUInt32(1, GetAccountId());
    stmt->setUInt16(2, AT_LOGIN_RENAME);
    stmt->setUInt16(3, AT_LOGIN_RENAME);
    stmt->setString(4, newName);

    _charRenameCallback.SetFutureResult(CharacterDatabase.AsyncQuery(stmt));
}

void WorldSession::HandleChangePlayerNameOpcodeCallBack(PreparedQueryResult result, std::string newName)
{
    if (!result)
    {
        WorldPacket data(SMSG_CHARACTER_RENAME, 1);
        data << uint8(CHAR_CREATE_ERROR);
        SendPacket(&data);
        return;
    }

    uint32 guidLow = result->Fetch()[0].GetUInt32();
    uint64 guid = MAKE_NEW_GUID(guidLow, 0, HIGHGUID_PLAYER);
    std::string oldname = result->Fetch()[1].GetString();

    CharacterDatabase.PExecute("UPDATE characters set name = '%s', at_login = at_login & ~ %u WHERE guid ='%u'", newName.c_str(), uint32(AT_LOGIN_RENAME), guidLow);
    CharacterDatabase.PExecute("DELETE FROM character_declinedname WHERE guid ='%u'", guidLow);

    sLog->outChar("Account: %d (IP: %s) Character:[%s] (guid:%u) Changed name to: %s", GetAccountId(), GetRemoteAddress().c_str(), oldname.c_str(), guidLow, newName.c_str());

    WorldPacket data(SMSG_CHARACTER_RENAME, 1+8+(newName.size()+1));
    data << uint8(RESPONSE_SUCCESS);
    data << uint64(guid);
    data << newName;
    SendPacket(&data);

    sWorld->UpdateCharacterNameData(guidLow, newName);
}

void WorldSession::HandleSetPlayerDeclinedNames(WorldPacket& recvData)
{
    uint64 guid;

    recvData >> guid;

    // not accept declined names for unsupported languages
    std::string name;
    if (!sObjectMgr->GetPlayerNameByGUID(guid, name))
    {
        WorldPacket data(SMSG_SET_PLAYER_DECLINED_NAMES_RESULT, 4+8);
        data << uint32(1);
        data << uint64(guid);
        SendPacket(&data);
        return;
    }

    std::wstring wname;
    if (!Utf8toWStr(name, wname))
    {
        WorldPacket data(SMSG_SET_PLAYER_DECLINED_NAMES_RESULT, 4+8);
        data << uint32(1);
        data << uint64(guid);
        SendPacket(&data);
        return;
    }

    if (!isCyrillicCharacter(wname[0]))                      // name already stored as only single alphabet using
    {
        WorldPacket data(SMSG_SET_PLAYER_DECLINED_NAMES_RESULT, 4+8);
        data << uint32(1);
        data << uint64(guid);
        SendPacket(&data);
        return;
    }

    std::string name2;
    DeclinedName declinedname;

    recvData >> name2;

    if (name2 != name)                                       // character have different name
    {
        WorldPacket data(SMSG_SET_PLAYER_DECLINED_NAMES_RESULT, 4+8);
        data << uint32(1);
        data << uint64(guid);
        SendPacket(&data);
        return;
    }

    for (int i = 0; i < MAX_DECLINED_NAME_CASES; ++i)
    {
        recvData >> declinedname.name[i];
        if (!normalizePlayerName(declinedname.name[i]))
        {
            WorldPacket data(SMSG_SET_PLAYER_DECLINED_NAMES_RESULT, 4+8);
            data << uint32(1);
            data << uint64(guid);
            SendPacket(&data);
            return;
        }
    }

    if (!ObjectMgr::CheckDeclinedNames(wname, declinedname))
    {
        WorldPacket data(SMSG_SET_PLAYER_DECLINED_NAMES_RESULT, 4+8);
        data << uint32(1);
        data << uint64(guid);
        SendPacket(&data);
        return;
    }

    for (int i = 0; i < MAX_DECLINED_NAME_CASES; ++i)
        CharacterDatabase.EscapeString(declinedname.name[i]);

    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    trans->PAppend("DELETE FROM character_declinedname WHERE guid = '%u'", GUID_LOPART(guid));
    trans->PAppend("INSERT INTO character_declinedname (guid, genitive, dative, accusative, instrumental, prepositional) VALUES ('%u', '%s', '%s', '%s', '%s', '%s')",
        GUID_LOPART(guid), declinedname.name[0].c_str(), declinedname.name[1].c_str(), declinedname.name[2].c_str(), declinedname.name[3].c_str(), declinedname.name[4].c_str());
    CharacterDatabase.CommitTransaction(trans);

    WorldPacket data(SMSG_SET_PLAYER_DECLINED_NAMES_RESULT, 4+8);
    data << uint32(0);                                      // OK
    data << uint64(guid);
    SendPacket(&data);
}

void WorldSession::HandleAlterAppearance(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "CMSG_ALTER_APPEARANCE");

    uint32 Hair, Color, FacialHair, SkinColor;
    recvData >> Hair >> Color >> FacialHair >> SkinColor;

    BarberShopStyleEntry const* bs_hair = sBarberShopStyleStore.LookupEntry(Hair);

    if (!bs_hair || bs_hair->type != 0 || bs_hair->race != _player->getRace() || bs_hair->gender != _player->getGender())
        return;

    BarberShopStyleEntry const* bs_facialHair = sBarberShopStyleStore.LookupEntry(FacialHair);

    if (!bs_facialHair || bs_facialHair->type != 2 || bs_facialHair->race != _player->getRace() || bs_facialHair->gender != _player->getGender())
        return;

    BarberShopStyleEntry const* bs_skinColor = sBarberShopStyleStore.LookupEntry(SkinColor);

    if (bs_skinColor && (bs_skinColor->type != 3 || bs_skinColor->race != _player->getRace() || bs_skinColor->gender != _player->getGender()))
        return;

    uint64 Cost = _player->GetBarberShopCost(bs_hair->hair_id, Color, bs_facialHair->hair_id, bs_skinColor);

    // 0 - ok
    // 1, 3 - not enough money
    // 2 - you have to seat on barber chair
    if (!_player->HasEnoughMoney(Cost))
    {
        WorldPacket data(SMSG_BARBER_SHOP_RESULT, 4);
        data << uint32(1);                                  // no money
        SendPacket(&data);
        return;
    }
    else
    {
        WorldPacket data(SMSG_BARBER_SHOP_RESULT, 4);
        data << uint32(0);                                  // ok
        SendPacket(&data);
    }

    _player->ModifyMoney(-int32(Cost));                     // it isn't free
    _player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_AT_BARBER, Cost);

    _player->SetByteValue(PLAYER_BYTES, 2, uint8(bs_hair->hair_id));
    _player->SetByteValue(PLAYER_BYTES, 3, uint8(Color));
    _player->SetByteValue(PLAYER_BYTES_2, 0, uint8(bs_facialHair->hair_id));
    if (bs_skinColor)
        _player->SetByteValue(PLAYER_BYTES, 0, uint8(bs_skinColor->hair_id));

    _player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_VISIT_BARBER_SHOP, 1);

    _player->SetStandState(0);                              // stand up
}

void WorldSession::HandleRemoveGlyph(WorldPacket& recvData)
{
    uint32 slot;
    recvData >> slot;

    if (slot >= MAX_GLYPH_SLOT_INDEX)
    {
        sLog->outDebug(LOG_FILTER_NETWORKIO, "Client sent wrong glyph slot number in opcode CMSG_REMOVE_GLYPH %u", slot);
        return;
    }

    if (uint32 glyph = _player->GetGlyph(_player->GetActiveSpec(), slot))
    {
        if (GlyphPropertiesEntry const *gp = sGlyphPropertiesStore.LookupEntry(glyph))
        {
            _player->RemoveAurasDueToSpell(gp->SpellId);
            _player->SetGlyph(slot, 0);
            _player->SendTalentsInfoData(false);
        }
    }
}

void WorldSession::HandleCharCustomize(WorldPacket& recvData)
{
    uint64 guid;
    std::string newName;

    recvData >> guid;
    recvData >> newName;

    uint8 gender, skin, face, hairStyle, hairColor, facialHair;
    recvData >> gender >> skin >> hairColor >> hairStyle >> facialHair >> face;

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_SELECT_CHARACTER_AT_LOGIN);

    stmt->setUInt32(0, GUID_LOPART(guid));

    PreparedQueryResult result = CharacterDatabase.Query(stmt);
    
    if (!result)
    {
        WorldPacket data(SMSG_CHARACTER_CUSTOMIZE, 1);
        data << uint8(CHAR_CREATE_ERROR);
        SendPacket(&data);
        return;
    }

    Field *fields = result->Fetch();
    uint32 at_loginFlags = fields[0].GetUInt16();

    if (!(at_loginFlags & AT_LOGIN_CUSTOMIZE))
    {
        WorldPacket data(SMSG_CHARACTER_CUSTOMIZE, 1);
        data << uint8(CHAR_CREATE_ERROR);
        SendPacket(&data);
        return;
    }

    // prevent character rename to invalid name
    if (!normalizePlayerName(newName))
    {
        WorldPacket data(SMSG_CHARACTER_CUSTOMIZE, 1);
        data << uint8(CHAR_NAME_NO_NAME);
        SendPacket(&data);
        return;
    }

    uint8 res = ObjectMgr::CheckPlayerName(newName, true);
    if (res != CHAR_NAME_SUCCESS)
    {
        WorldPacket data(SMSG_CHARACTER_CUSTOMIZE, 1);
        data << uint8(res);
        SendPacket(&data);
        return;
    }

    // check name limitations
    if (AccountMgr::IsPlayerAccount(GetSecurity()) && sObjectMgr->IsReservedName(newName))
    {
        WorldPacket data(SMSG_CHARACTER_CUSTOMIZE, 1);
        data << uint8(CHAR_NAME_RESERVED);
        SendPacket(&data);
        return;
    }

    // character with this name already exist
    if (uint64 newguid = sObjectMgr->GetPlayerGUIDByName(newName))
    {
        if (newguid != guid)
        {
            WorldPacket data(SMSG_CHARACTER_CUSTOMIZE, 1);
            data << uint8(CHAR_CREATE_NAME_IN_USE);
            SendPacket(&data);
            return;
        }
    }

    stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_SELECT_CHARACTER_NAME);
    stmt->setUInt32(0, GUID_LOPART(guid));
    result = CharacterDatabase.Query(stmt);

    if (result)
    {
        std::string oldname = result->Fetch()[0].GetString();
        sLog->outChar("Account: %d (IP: %s), Character[%s] (guid:%u) Customized to: %s", GetAccountId(), GetRemoteAddress().c_str(), oldname.c_str(), GUID_LOPART(guid), newName.c_str());
    }
    Player::Customize(guid, gender, skin, face, hairStyle, hairColor, facialHair);

    stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_UPDATE_CHARACTER_NAME_AT_LOGIN);

    stmt->setString(0, newName);
    stmt->setUInt16(1, uint16(AT_LOGIN_CUSTOMIZE));
    stmt->setUInt32(2, GUID_LOPART(guid));

    CharacterDatabase.Execute(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_DELETE_DECLINED_NAME);

    stmt->setUInt32(0, GUID_LOPART(guid));

    CharacterDatabase.Execute(stmt);

    sWorld->UpdateCharacterNameData(GUID_LOPART(guid), newName, gender);

    WorldPacket data(SMSG_CHARACTER_CUSTOMIZE, 1+8+(newName.size()+1)+6);
    data << uint8(RESPONSE_SUCCESS);
    data << uint64(guid);
    data << newName;
    data << uint8(gender);
    data << uint8(skin);
    data << uint8(face);
    data << uint8(hairStyle);
    data << uint8(hairColor);
    data << uint8(facialHair);
    SendPacket(&data);
}

void WorldSession::HandleEquipmentSetSave(WorldPacket &recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "CMSG_EQUIPMENT_SET_SAVE");

    uint64 setGuid;
    recvData.readPackGUID(setGuid);

    uint32 index;
    recvData >> index;
    if (index >= MAX_EQUIPMENT_SET_INDEX)                    // client set slots amount
        return;

    std::string name;
    recvData >> name;

    std::string iconName;
    recvData >> iconName;

    EquipmentSet eqSet;

    eqSet.Guid      = setGuid;
    eqSet.Name      = name;
    eqSet.IconName  = iconName;
    eqSet.state     = EQUIPMENT_SET_NEW;

    for (uint32 i = 0; i < EQUIPMENT_SLOT_END; ++i)
    {
        uint64 itemGuid;
        recvData.readPackGUID(itemGuid);

        Item *item = _player->GetItemByPos(INVENTORY_SLOT_BAG_0, i);

        if (!item && itemGuid)                               // cheating check 1
            return;

        if (item && item->GetGUID() != itemGuid)             // cheating check 2
            return;

        eqSet.Items[i] = GUID_LOPART(itemGuid);
    }

    _player->SetEquipmentSet(index, eqSet);
}

void WorldSession::HandleEquipmentSetDelete(WorldPacket &recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "CMSG_EQUIPMENT_SET_DELETE");

    uint64 setGuid;
    recvData.readPackGUID(setGuid);

    _player->DeleteEquipmentSet(setGuid);
}

void WorldSession::HandleEquipmentSetUse(WorldPacket &recvData)
{
    if (_player->isInCombat())
        return;

    sLog->outDebug(LOG_FILTER_NETWORKIO, "CMSG_EQUIPMENT_SET_USE");
    recvData.hexlike();

    for (uint32 i = 0; i < EQUIPMENT_SLOT_END; ++i)
    {
        uint64 itemGuid;
        recvData.readPackGUID(itemGuid);

        uint8 srcbag, srcslot;
        recvData >> srcbag >> srcslot;

        sLog->outDebug(LOG_FILTER_PLAYER_ITEMS, "Item " UI64FMTD ": srcbag %u, srcslot %u", itemGuid, srcbag, srcslot);

        Item *item = _player->GetItemByGuid(itemGuid);

        uint16 dstpos = i | (INVENTORY_SLOT_BAG_0 << 8);

        if (!item)
        {
            Item *uItem = _player->GetItemByPos(INVENTORY_SLOT_BAG_0, i);
            if (!uItem)
                continue;

            ItemPosCountVec sDest;
            InventoryResult msg = _player->CanStoreItem(NULL_BAG, NULL_SLOT, sDest, uItem, false);
            if (msg == EQUIP_ERR_OK)
            {
                _player->RemoveItem(INVENTORY_SLOT_BAG_0, i, true);
                _player->StoreItem(sDest, uItem, true);
            }
            else
                _player->SendEquipError(msg, uItem, NULL);

            continue;
        }

        if (item->GetPos() == dstpos)
            continue;

        _player->SwapItem(item->GetPos(), dstpos);
    }

    WorldPacket data(SMSG_EQUIPMENT_SET_USE_RESULT, 1);
    data << uint8(0);                                       // 4 - equipment swap failed - inventory is full
    SendPacket(&data);
}

void WorldSession::HandleCharFactionOrRaceChange(WorldPacket& recvData)
{
    // TODO: Move queries to prepared statements
    uint64 guid;
    std::string newname;
    uint8 gender, skin, face, hairStyle, hairColor, facialHair, race;
    recvData >> guid;
    recvData >> newname;
    recvData >> gender >> skin >> hairColor >> hairStyle >> facialHair >> face >> race;

    uint32 lowGuid = GUID_LOPART(guid);

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_SELECT_CHARACTER_CLASS_LVL_AT_LOGIN);

    stmt->setUInt32(0, lowGuid);

    PreparedQueryResult result = CharacterDatabase.Query(stmt);
    
    if (!result)
    {
        WorldPacket data(SMSG_CHARACTER_FACTION_CHANGE, 1);
        data << uint8(CHAR_CREATE_ERROR);
        SendPacket(&data);
        return;
    }

    Field *fields = result->Fetch();
    uint32 playerClass = fields[0].GetUInt8();
    uint32 level = fields[1].GetUInt8();
    uint32 at_loginFlags = fields[2].GetUInt16();
    uint32 used_loginFlag = ((recvData.GetOpcode() == CMSG_CHARACTER_RACE_CHANGE) ? AT_LOGIN_CHANGE_RACE : AT_LOGIN_CHANGE_FACTION);

    if (!sObjectMgr->GetPlayerInfo(race, playerClass))
    {
        WorldPacket data(SMSG_CHARACTER_FACTION_CHANGE, 1);
        data << uint8(CHAR_CREATE_ERROR);
        SendPacket(&data);
        return;
    }

    if (!(at_loginFlags & used_loginFlag))
    {
        WorldPacket data(SMSG_CHARACTER_FACTION_CHANGE, 1);
        data << uint8(CHAR_CREATE_ERROR);
        SendPacket(&data);
        return;
    }

    if (AccountMgr::IsPlayerAccount(GetSecurity()))
    {
        uint32 raceMaskDisabled = sWorld->getIntConfig(CONFIG_CHARACTER_CREATING_DISABLED_RACEMASK);
        if ((1 << (race - 1)) & raceMaskDisabled)
        {
            WorldPacket data(SMSG_CHARACTER_FACTION_CHANGE, 1);
            data << uint8(CHAR_CREATE_ERROR);
            SendPacket(&data);
            return;
        }
    }

    // prevent character rename to invalid name
    if (!normalizePlayerName(newname))
    {
        WorldPacket data(SMSG_CHARACTER_FACTION_CHANGE, 1);
        data << uint8(CHAR_NAME_NO_NAME);
        SendPacket(&data);
        return;
    }

    uint8 res = ObjectMgr::CheckPlayerName(newname, true);
    if (res != CHAR_NAME_SUCCESS)
    {
        WorldPacket data(SMSG_CHARACTER_FACTION_CHANGE, 1);
        data << uint8(res);
        SendPacket(&data);
        return;
    }

    // check name limitations
    if (AccountMgr::IsPlayerAccount(GetSecurity()) && sObjectMgr->IsReservedName(newname))
    {
        WorldPacket data(SMSG_CHARACTER_FACTION_CHANGE, 1);
        data << uint8(CHAR_NAME_RESERVED);
        SendPacket (&data);
        return;
    }

    // character with this name already exist
    if (uint64 newguid = sObjectMgr->GetPlayerGUIDByName(newname))
    {
        if (newguid != guid)
        {
            WorldPacket data(SMSG_CHARACTER_FACTION_CHANGE, 1);
            data << uint8(CHAR_CREATE_NAME_IN_USE);
            SendPacket(&data);
            return;
        }
    }

    CharacterDatabase.EscapeString(newname);
    Player::Customize(guid, gender, skin, face, hairStyle, hairColor, facialHair);
    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    trans->PAppend("UPDATE `characters` SET name='%s', race='%u', at_login=at_login & ~ %u WHERE guid='%u'", newname.c_str(), race, used_loginFlag, lowGuid);
    trans->PAppend("DELETE FROM character_declinedname WHERE guid ='%u'", lowGuid);
    sWorld->UpdateCharacterNameData(GUID_LOPART(guid), newname, gender, race);

    BattlegroundTeamId team = BG_TEAM_ALLIANCE;

    // Search each faction is targeted
    switch (race)
    {
        case RACE_ORC:
        case RACE_TAUREN:
        case RACE_UNDEAD_PLAYER:
        case RACE_TROLL:
        case RACE_BLOODELF:
        case RACE_GOBLIN:
            team = BG_TEAM_HORDE;
            break;
        default:
            break;
    }

    // Switch Languages
    // delete all languages first
    trans->PAppend("DELETE FROM `character_skills` WHERE `skill` IN (98, 113, 759, 111, 313, 109, 115, 315, 673, 137, 791, 792) AND `guid`='%u'", lowGuid);

    // now add them back
    if (team == BG_TEAM_ALLIANCE)
    {
        trans->PAppend("INSERT INTO `character_skills` (guid, skill, value, max) VALUES (%u, 98, 300, 300)", lowGuid);
        switch (race)
        {
        case RACE_DWARF:
            trans->PAppend("INSERT INTO `character_skills` (guid, skill, value, max) VALUES (%u, 111, 300, 300)", lowGuid);
            break;
        case RACE_DRAENEI:
            trans->PAppend("INSERT INTO `character_skills` (guid, skill, value, max) VALUES (%u, 759, 300, 300)", lowGuid);
            break;
        case RACE_GNOME:
            trans->PAppend("INSERT INTO `character_skills` (guid, skill, value, max) VALUES (%u, 313, 300, 300)", lowGuid);
            break;
        case RACE_NIGHTELF:
            trans->PAppend("INSERT INTO `character_skills` (guid, skill, value, max) VALUES (%u, 113, 300, 300)", lowGuid);
            break;
        case RACE_WORGEN:
            trans->PAppend("INSERT INTO `character_skills` (guid, skill, value, max) VALUES (%u, 791, 300, 300)", lowGuid);
            break;
        }
    }
    else if (team == BG_TEAM_HORDE)
    {
        trans->PAppend("INSERT INTO `character_skills` (guid, skill, value, max) VALUES (%u, 109, 300, 300)", lowGuid);
        switch (race)
        {
        case RACE_UNDEAD_PLAYER:
            trans->PAppend("INSERT INTO `character_skills` (guid, skill, value, max) VALUES (%u, 673, 300, 300)", lowGuid);
            break;
        case RACE_TAUREN:
            trans->PAppend("INSERT INTO `character_skills` (guid, skill, value, max) VALUES (%u, 115, 300, 300)", lowGuid);
            break;
        case RACE_TROLL:
            trans->PAppend("INSERT INTO `character_skills` (guid, skill, value, max) VALUES (%u, 315, 300, 300)", lowGuid);
            break;
        case RACE_BLOODELF:
            trans->PAppend("INSERT INTO `character_skills` (guid, skill, value, max) VALUES (%u, 137, 300, 300)", lowGuid);
            break;
        case RACE_GOBLIN:
            trans->PAppend("INSERT INTO `character_skills` (guid, skill, value, max) VALUES (%u, 792, 300, 300)", lowGuid);
            break;
        }
    }

    if (recvData.GetOpcode() == CMSG_CHARACTER_FACTION_CHANGE)
    {
        // Delete all Flypaths
        trans->PAppend("UPDATE `characters` SET taxi_path = '' WHERE guid ='%u'", lowGuid);

        if (level > 7)
        {
            // Update Taxi path
            // this doesn't seem to be 100% blizzlike... but it can't really be helped.
            std::ostringstream taximaskstream;
            uint32 numFullTaximasks = level / 7;
            if (numFullTaximasks > 11)
                numFullTaximasks = 11;
            if (team == BG_TEAM_ALLIANCE)
            {
                if (playerClass != CLASS_DEATH_KNIGHT)
                {
                    for (uint8 i = 0; i < numFullTaximasks; ++i)
                        taximaskstream << uint32(sAllianceTaxiNodesMask[i]) << ' ';
                }
                else
                {
                    for (uint8 i = 0; i < numFullTaximasks; ++i)
                        taximaskstream << uint32(sAllianceTaxiNodesMask[i] | sDeathKnightTaxiNodesMask[i]) << ' ';
                }
            }
            else
            {
                if (playerClass != CLASS_DEATH_KNIGHT)
                {
                    for (uint8 i = 0; i < numFullTaximasks; ++i)
                        taximaskstream << uint32(sHordeTaxiNodesMask[i]) << ' ';
                }
                else
                {
                    for (uint8 i = 0; i < numFullTaximasks; ++i)
                        taximaskstream << uint32(sHordeTaxiNodesMask[i] | sDeathKnightTaxiNodesMask[i]) << ' ';
                }
            }

            uint32 numEmptyTaximasks = 11 - numFullTaximasks;
            for (uint8 i = 0; i < numEmptyTaximasks; ++i)
                taximaskstream << "0 ";
            taximaskstream << '0';
            std::string taximask = taximaskstream.str();
            trans->PAppend("UPDATE `characters` SET `taximask`= '%s' WHERE `guid` = '%u'", taximask.c_str(), lowGuid);
        }

        // Delete all current quests
        trans->PAppend("DELETE FROM `character_queststatus` WHERE guid ='%u'", GUID_LOPART(guid));

        // Delete record of the faction old completed quests
        {
            std::ostringstream quests;
            ObjectMgr::QuestMap const& qTemplates = sObjectMgr->GetQuestTemplates();
            for (ObjectMgr::QuestMap::const_iterator iter = qTemplates.begin(); iter != qTemplates.end(); ++iter)
            {
                Quest *qinfo = iter->second;
                uint32 requiredRaces = qinfo->GetRequiredRaces();
                if (team == BG_TEAM_ALLIANCE)
                {
                    if (requiredRaces & RACEMASK_ALLIANCE)
                    {
                        quests << uint32(qinfo->GetQuestId());
                        quests << ',';
                    }
                }
                else // if (team == BG_TEAM_HORDE)
                {
                    if (requiredRaces & RACEMASK_HORDE)
                    {
                        quests << uint32(qinfo->GetQuestId());
                        quests << ',';
                    }
                }
            }

            std::string questsStr = quests.str();
            questsStr = questsStr.substr(0, questsStr.length() - 1);

            if (!questsStr.empty())
                trans->PAppend("DELETE FROM `character_queststatus_rewarded` WHERE guid='%u' AND quest IN (%s)", lowGuid, questsStr.c_str());
        }

        if (!sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_GUILD))
        {
            // Reset guild
            PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_SELECT_GUILD_ID);

            stmt->setUInt32(0, lowGuid);

            PreparedQueryResult result = CharacterDatabase.Query(stmt);
            if (result)
                if (Guild* guild = sGuildMgr->GetGuildById((result->Fetch()[0]).GetUInt32()))
                    guild->DeleteMember(MAKE_NEW_GUID(lowGuid, 0, HIGHGUID_PLAYER));
        }

        if (!sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_ADD_FRIEND))
        {
            // Delete Friend List
            trans->PAppend("DELETE FROM `character_social` WHERE `guid`= '%u'", lowGuid);
            trans->PAppend("DELETE FROM `character_social` WHERE `friend`= '%u'", lowGuid);
        }

        // Leave Arena Teams
        Player::LeaveAllArenaTeams(guid);

        // Reset homebind and position
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_DELETE_PLAYER_HOMEBIND);
        stmt->setUInt32(0, lowGuid);
        trans->Append(stmt);

        stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_INSERT_PLAYER_HOMEBIND);
        stmt->setUInt32(0, lowGuid);
        if (team == BG_TEAM_ALLIANCE)
        {
            stmt->setUInt16(1, 0);
            stmt->setUInt16(2, 1519);
            stmt->setFloat (3, -8867.68f);
            stmt->setFloat (4, 673.373f);
            stmt->setFloat (5, 97.9034f);
            Player::SavePositionInDB(0, -8867.68f, 673.373f, 97.9034f, 0.0f, 1519, lowGuid);
        }
        else
        {
            stmt->setUInt16(1, 1);
            stmt->setUInt16(2, 1637);
            stmt->setFloat (3, 1633.33f);
            stmt->setFloat (4, -4439.11f);
            stmt->setFloat (5, 15.7588f);
            Player::SavePositionInDB(1, 1633.33f, -4439.11f, 15.7588f, 0.0f, 1637, lowGuid);
        }
        trans->Append(stmt);

        // Achievement conversion
        for (std::map<uint32, uint32>::const_iterator it = sObjectMgr->FactionChange_Achievements.begin(); it != sObjectMgr->FactionChange_Achievements.end(); ++it)
        {
            uint32 achiev_alliance = it->first;
            uint32 achiev_horde = it->second;
            trans->PAppend("DELETE FROM `character_achievement` WHERE `achievement`=%u AND `guid`=%u",
                            team == BG_TEAM_ALLIANCE ? achiev_alliance : achiev_horde, lowGuid);
            trans->PAppend("UPDATE `character_achievement` SET achievement = '%u' where achievement = '%u' AND guid = '%u'",
                team == BG_TEAM_ALLIANCE ? achiev_alliance : achiev_horde, team == BG_TEAM_ALLIANCE ? achiev_horde : achiev_alliance, lowGuid);
        }

        // Item conversion
        for (std::map<uint32, uint32>::const_iterator it = sObjectMgr->FactionChange_Items.begin(); it != sObjectMgr->FactionChange_Items.end(); ++it)
        {
            uint32 item_alliance = it->first;
            uint32 item_horde = it->second;
            trans->PAppend("UPDATE `item_instance` ii, `character_inventory` ci SET ii.itemEntry = '%u' WHERE ii.itemEntry = '%u' AND ci.guid = '%u' AND ci.item = ii.guid",
                team == BG_TEAM_ALLIANCE ? item_alliance : item_horde, team == BG_TEAM_ALLIANCE ? item_horde : item_alliance, guid);
        }

        // Spell conversion
        for (std::map<uint32, uint32>::const_iterator it = sObjectMgr->FactionChange_Spells.begin(); it != sObjectMgr->FactionChange_Spells.end(); ++it)
        {
            uint32 spell_alliance = it->first;
            uint32 spell_horde = it->second;
            trans->PAppend("DELETE FROM `character_spell` WHERE `spell`=%u AND `guid`=%u",
                            team == BG_TEAM_ALLIANCE ? spell_alliance : spell_horde, lowGuid);
            trans->PAppend("UPDATE `character_spell` SET spell = '%u' where spell = '%u' AND guid = '%u'",
                team == BG_TEAM_ALLIANCE ? spell_alliance : spell_horde, team == BG_TEAM_ALLIANCE ? spell_horde : spell_alliance, lowGuid);
        }

        // Reputation conversion
        for (std::map<uint32, uint32>::const_iterator it = sObjectMgr->FactionChange_Reputation.begin(); it != sObjectMgr->FactionChange_Reputation.end(); ++it)
        {
            uint32 reputation_alliance = it->first;
            uint32 reputation_horde = it->second;
            trans->PAppend("DELETE FROM character_reputation WHERE faction = '%u' AND guid = '%u'",
                team == BG_TEAM_ALLIANCE ? reputation_alliance : reputation_horde, lowGuid);
            trans->PAppend("UPDATE `character_reputation` SET faction = '%u' where faction = '%u' AND guid = '%u'",
                team == BG_TEAM_ALLIANCE ? reputation_alliance : reputation_horde, team == BG_TEAM_ALLIANCE ? reputation_horde : reputation_alliance, lowGuid);
        }
    }

    CharacterDatabase.CommitTransaction(trans);

    std::string IP_str = GetRemoteAddress();
    sLog->outDebug(LOG_FILTER_UNITS, "Account: %d (IP: %s), Character guid: %u Change Race/Faction to: %s", GetAccountId(), IP_str.c_str(), lowGuid, newname.c_str());

    WorldPacket data(SMSG_CHARACTER_FACTION_CHANGE, 1 + 8 + (newname.size() + 1) + 1 + 1 + 1 + 1 + 1 + 1 + 1);
    data << uint8(RESPONSE_SUCCESS);
    data << uint64(guid);
    data << newname;
    data << uint8(gender);
    data << uint8(skin);
    data << uint8(face);
    data << uint8(hairStyle);
    data << uint8(hairColor);
    data << uint8(facialHair);
    data << uint8(race);
    SendPacket(&data);
}
