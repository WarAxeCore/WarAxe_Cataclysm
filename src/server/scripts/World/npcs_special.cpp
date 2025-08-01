/*
 * Copyright (C) 2011-2019 Project SkyFire <http://www.projectskyfire.org/>
 * Copyright (C) 2008-2019 TrinityCore <http://www.trinitycore.org/>
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

/* ScriptData
SFName: Npcs_Special
SF%Complete: 100
SFComment: To be used for special NPCs that are located globally.
SFCategory: NPCs
EndScriptData
*/

/* ContentData
npc_air_force_bots       80%    support for misc (invisible) guard bots in areas where player allowed to fly. Summon guards after a preset time if tagged by spell
npc_lunaclaw_spirit      80%    support for quests 6001/6002 (Body and Heart)
npc_chicken_cluck       100%    support for quest 3861 (Cluck!)
npc_dancing_flames      100%    midsummer event NPC
npc_guardian            100%    guardianAI used to prevent players from accessing off-limits areas. Not in use by SD2
npc_garments_of_quests   80%    NPC's related to all Garments of-quests 5621, 5624, 5625, 5648, 565
npc_injured_patient     100%    patients for triage-quests (6622 and 6624)
npc_doctor              100%    Gustaf Vanhowzen and Gregory Victor, quest 6622 and 6624 (Triage)
npc_kingdom_of_dalaran_quests   Misc NPC's gossip option related to quests 12791, 12794 and 12796
npc_mount_vendor        100%    Regular mount vendors all over the world. Display gossip if player doesn't meet the requirements to buy
npc_rogue_trainer        80%    Scripted trainers, so they are able to offer item 17126 for class quest 6681
npc_sayge               100%    Darkmoon event fortune teller, buff player based on answers given
npc_snake_trap_serpents  80%    AI for snakes that summoned by Snake Trap
npc_shadowfiend         100%   restore 5% of owner's mana when shadowfiend die from damage
npc_locksmith            75%    list of keys needs to be confirmed
npc_firework            100%    NPC's summoned by rockets and rocket clusters, for making them cast visual
EndContentData */

#include "ScriptPCH.h"
#include "ScriptedEscortAI.h"
#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "World.h"

/*########
# npc_air_force_bots
#########*/

enum SpawnType
{
    SPAWNTYPE_TRIPWIRE_ROOFTOP,                             // no warning, summon Creature at smaller range
    SPAWNTYPE_ALARMBOT,                                     // cast guards mark and summon npc - if player shows up with that buff duration < 5 seconds attack
};

struct SpawnAssociation
{
    uint32 thisCreatureEntry;
    uint32 spawnedCreatureEntry;
    SpawnType spawnType;
};

enum AirforceBot
{
    SPELL_GUARDS_MARK               = 38067,
    AURA_DURATION_TIME_LEFT         = 5000
};

float const RANGE_TRIPWIRE          = 15.0f;
float const RANGE_GUARDS_MARK       = 50.0f;

SpawnAssociation spawnAssociations[] =
{
    {2614,  15241, SPAWNTYPE_ALARMBOT},                    //Air Force Alarm Bot (Alliance)
    {2615,  15242, SPAWNTYPE_ALARMBOT},                    //Air Force Alarm Bot (Horde)
    {21974, 21976, SPAWNTYPE_ALARMBOT},                    //Air Force Alarm Bot (Area 52)
    {21993, 15242, SPAWNTYPE_ALARMBOT},                    //Air Force Guard Post (Horde - Bat Rider)
    {21996, 15241, SPAWNTYPE_ALARMBOT},                    //Air Force Guard Post (Alliance - Gryphon)
    {21997, 21976, SPAWNTYPE_ALARMBOT},                    //Air Force Guard Post (Goblin - Area 52 - Zeppelin)
    {21999, 15241, SPAWNTYPE_TRIPWIRE_ROOFTOP},            //Air Force Trip Wire - Rooftop (Alliance)
    {22001, 15242, SPAWNTYPE_TRIPWIRE_ROOFTOP},            //Air Force Trip Wire - Rooftop (Horde)
    {22002, 15242, SPAWNTYPE_TRIPWIRE_ROOFTOP},            //Air Force Trip Wire - Ground (Horde)
    {22003, 15241, SPAWNTYPE_TRIPWIRE_ROOFTOP},            //Air Force Trip Wire - Ground (Alliance)
    {22063, 21976, SPAWNTYPE_TRIPWIRE_ROOFTOP},            //Air Force Trip Wire - Rooftop (Goblin - Area 52)
    {22065, 22064, SPAWNTYPE_ALARMBOT},                    //Air Force Guard Post (Ethereal - Stormspire)
    {22066, 22067, SPAWNTYPE_ALARMBOT},                    //Air Force Guard Post (Scryer - Dragonhawk)
    {22068, 22064, SPAWNTYPE_TRIPWIRE_ROOFTOP},            //Air Force Trip Wire - Rooftop (Ethereal - Stormspire)
    {22069, 22064, SPAWNTYPE_ALARMBOT},                    //Air Force Alarm Bot (Stormspire)
    {22070, 22067, SPAWNTYPE_TRIPWIRE_ROOFTOP},            //Air Force Trip Wire - Rooftop (Scryer)
    {22071, 22067, SPAWNTYPE_ALARMBOT},                    //Air Force Alarm Bot (Scryer)
    {22078, 22077, SPAWNTYPE_ALARMBOT},                    //Air Force Alarm Bot (Aldor)
    {22079, 22077, SPAWNTYPE_ALARMBOT},                    //Air Force Guard Post (Aldor - Gryphon)
    {22080, 22077, SPAWNTYPE_TRIPWIRE_ROOFTOP},            //Air Force Trip Wire - Rooftop (Aldor)
    {22086, 22085, SPAWNTYPE_ALARMBOT},                    //Air Force Alarm Bot (Sporeggar)
    {22087, 22085, SPAWNTYPE_ALARMBOT},                    //Air Force Guard Post (Sporeggar - Spore Bat)
    {22088, 22085, SPAWNTYPE_TRIPWIRE_ROOFTOP},            //Air Force Trip Wire - Rooftop (Sporeggar)
    {22090, 22089, SPAWNTYPE_ALARMBOT},                    //Air Force Guard Post (Toshley's Station - Flying Machine)
    {22124, 22122, SPAWNTYPE_ALARMBOT},                    //Air Force Alarm Bot (Cenarion)
    {22125, 22122, SPAWNTYPE_ALARMBOT},                    //Air Force Guard Post (Cenarion - Stormcrow)
    {22126, 22122, SPAWNTYPE_ALARMBOT}                      //Air Force Trip Wire - Rooftop (Cenarion Expedition)
};

class npc_air_force_bots : public CreatureScript
{
public:
    npc_air_force_bots() : CreatureScript("npc_air_force_bots") { }

    struct npc_air_force_botsAI : public ScriptedAI
    {
        npc_air_force_botsAI(Creature* creature) : ScriptedAI(creature)
        {
            SpawnAssoc = NULL;
            SpawnedGUID = 0;

            // find the correct spawnhandling
            static uint32 entryCount = sizeof(spawnAssociations) / sizeof(SpawnAssociation);

            for (uint8 i = 0; i < entryCount; ++i)
            {
                if (spawnAssociations[i].thisCreatureEntry == creature->GetEntry())
                {
                    SpawnAssoc = &spawnAssociations[i];
                    break;
                }
            }

            if (!SpawnAssoc)
                sLog->outErrorDb("TCSR: Creature template entry %u has ScriptName npc_air_force_bots, but it's not handled by that script", creature->GetEntry());
            else
            {
                CreatureTemplate const* spawnedTemplate = sObjectMgr->GetCreatureTemplate(SpawnAssoc->spawnedCreatureEntry);

                if (!spawnedTemplate)
                {
                    sLog->outErrorDb("TCSR: Creature template entry %u does not exist in DB, which is required by npc_air_force_bots", SpawnAssoc->spawnedCreatureEntry);
                    SpawnAssoc = NULL;
                    return;
                }
            }
        }

        SpawnAssociation* SpawnAssoc;
        uint64 SpawnedGUID;

        void Reset() {}

        Creature* SummonGuard()
        {
            Creature* summoned = me->SummonCreature(SpawnAssoc->spawnedCreatureEntry, 0.0f, 0.0f, 0.0f, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 300000);

            if (summoned)
                SpawnedGUID = summoned->GetGUID();
            else
            {
                sLog->outErrorDb("TCSR: npc_air_force_bots: wasn't able to spawn Creature %u", SpawnAssoc->spawnedCreatureEntry);
                SpawnAssoc = NULL;
            }

            return summoned;
        }

        Creature* GetSummonedGuard()
        {
            Creature* creature = Unit::GetCreature(*me, SpawnedGUID);

            if (creature && creature->isAlive())
                return creature;

            return NULL;
        }

        void MoveInLineOfSight(Unit* who)
        {
            if (!SpawnAssoc)
                return;

            if (me->IsValidAttackTarget(who))
            {
                Player* playerTarget = who->ToPlayer();

                // airforce guards only spawn for players
                if (!playerTarget)
                    return;

                Creature* lastSpawnedGuard = SpawnedGUID == 0 ? NULL : GetSummonedGuard();

                // prevent calling Unit::GetUnit at next MoveInLineOfSight call - speedup
                if (!lastSpawnedGuard)
                    SpawnedGUID = 0;

                switch (SpawnAssoc->spawnType)
                {
                    case SPAWNTYPE_ALARMBOT:
                    {
                        if (!who->IsWithinDistInMap(me, RANGE_GUARDS_MARK))
                            return;

                        Aura* markAura = who->GetAura(SPELL_GUARDS_MARK);
                        if (markAura)
                        {
                            // the target wasn't able to move out of our range within 25 seconds
                            if (!lastSpawnedGuard)
                            {
                                lastSpawnedGuard = SummonGuard();

                                if (!lastSpawnedGuard)
                                    return;
                            }

                            if (markAura->GetDuration() < AURA_DURATION_TIME_LEFT)
                                if (!lastSpawnedGuard->getVictim())
                                    lastSpawnedGuard->AI()->AttackStart(who);
                        }
                        else
                        {
                            if (!lastSpawnedGuard)
                                lastSpawnedGuard = SummonGuard();

                            if (!lastSpawnedGuard)
                                return;

                            lastSpawnedGuard->CastSpell(who, SPELL_GUARDS_MARK, true);
                        }
                        break;
                    }
                    case SPAWNTYPE_TRIPWIRE_ROOFTOP:
                    {
                        if (!who->IsWithinDistInMap(me, RANGE_TRIPWIRE))
                            return;

                        if (!lastSpawnedGuard)
                            lastSpawnedGuard = SummonGuard();

                        if (!lastSpawnedGuard)
                            return;

                        // ROOFTOP only triggers if the player is on the ground
                        if (!playerTarget->IsFlying() && !lastSpawnedGuard->getVictim())
                            lastSpawnedGuard->AI()->AttackStart(who);

                        break;
                    }
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_air_force_botsAI(creature);
    }
};

/*######
## npc_lunaclaw_spirit
######*/

enum Lunaclaw
{
    QUEST_BODY_HEART_A      = 6001,
    QUEST_BODY_HEART_H      = 6002,

    TEXT_ID_DEFAULT         = 4714,
    TEXT_ID_PROGRESS        = 4715
};

#define GOSSIP_ITEM_GRANT   "You have thought well, spirit. I ask you to grant me the strength of your body and the strength of your heart."

class npc_lunaclaw_spirit : public CreatureScript
{
public:
    npc_lunaclaw_spirit() : CreatureScript("npc_lunaclaw_spirit") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (player->GetQuestStatus(QUEST_BODY_HEART_A) == QUEST_STATUS_INCOMPLETE || player->GetQuestStatus(QUEST_BODY_HEART_H) == QUEST_STATUS_INCOMPLETE)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_GRANT, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

        player->SEND_GOSSIP_MENU(TEXT_ID_DEFAULT, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        if (action == GOSSIP_ACTION_INFO_DEF + 1)
        {
            player->SEND_GOSSIP_MENU(TEXT_ID_PROGRESS, creature->GetGUID());
            player->AreaExploredOrEventHappens(player->GetTeam() == ALLIANCE ? QUEST_BODY_HEART_A : QUEST_BODY_HEART_H);
        }
        return true;
    }
};

/*########
# npc_chicken_cluck
#########*/

enum Cluck
{
    EMOTE_HELLO         = -1070004,
    EMOTE_CLUCK_TEXT    = -1070006,

    QUEST_CLUCK         = 3861,
    FACTION_FRIENDLY    = 35,
    FACTION_CHICKEN     = 31
};

class npc_chicken_cluck : public CreatureScript
{
public:
    npc_chicken_cluck() : CreatureScript("npc_chicken_cluck") { }

    struct npc_chicken_cluckAI : public ScriptedAI
    {
        npc_chicken_cluckAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 ResetFlagTimer;

        void Reset()
        {
            ResetFlagTimer = 120000;
            me->setFaction(FACTION_CHICKEN);
            me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
        }

        void EnterCombat(Unit* /*who*/) {}

        void UpdateAI(uint32 const diff)
        {
            // Reset flags after a certain time has passed so that the next player has to start the 'event' again
            if (me->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER))
            {
                if (ResetFlagTimer <= diff)
                {
                    EnterEvadeMode();
                    return;
                }
                else
                    ResetFlagTimer -= diff;
            }

            if (UpdateVictim())
                DoMeleeAttackIfReady();
        }

        void ReceiveEmote(Player* player, uint32 emote)
        {
            switch (emote)
            {
                case TEXT_EMOTE_CHICKEN:
                    if (player->GetQuestStatus(QUEST_CLUCK) == QUEST_STATUS_NONE && rand() % 30 == 1)
                    {
                        me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                        me->setFaction(FACTION_FRIENDLY);
                        DoScriptText(EMOTE_HELLO, me);
                    }
                    break;
                case TEXT_EMOTE_CHEER:
                    if (player->GetQuestStatus(QUEST_CLUCK) == QUEST_STATUS_COMPLETE)
                    {
                        me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                        me->setFaction(FACTION_FRIENDLY);
                        DoScriptText(EMOTE_CLUCK_TEXT, me);
                    }
                    break;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_chicken_cluckAI(creature);
    }

    bool OnQuestAccept(Player* /*player*/, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == QUEST_CLUCK)
            CAST_AI(npc_chicken_cluck::npc_chicken_cluckAI, creature->AI())->Reset();

        return true;
    }

    bool OnQuestComplete(Player* /*player*/, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == QUEST_CLUCK)
            CAST_AI(npc_chicken_cluck::npc_chicken_cluckAI, creature->AI())->Reset();

        return true;
    }
};

/*######
## npc_dancing_flames
######*/

enum Dancingflames
{
    SPELL_BRAZIER       = 45423,
    SPELL_SEDUCTION     = 47057,
    SPELL_FIERY_AURA    = 45427
};

class npc_dancing_flames : public CreatureScript
{
public:
    npc_dancing_flames() : CreatureScript("npc_dancing_flames") { }

    struct npc_dancing_flamesAI : public ScriptedAI
    {
        npc_dancing_flamesAI(Creature* creature) : ScriptedAI(creature) {}

        bool Active;
        uint32 CanIteract;

        void Reset()
        {
            Active = true;
            CanIteract = 3500;
            DoCast(me, SPELL_BRAZIER, true);
            DoCast(me, SPELL_FIERY_AURA, false);
            float x, y, z;
            me->GetPosition(x, y, z);
            me->Relocate(x, y, z + 0.94f);
            me->SetLevitate(true);
            me->HandleEmoteCommand(EMOTE_ONESHOT_DANCE);
            WorldPacket data;                       //send update position to client
            me->BuildHeartBeatMsg(&data);
            me->SendMessageToSet(&data, true);
        }

        void UpdateAI(uint32 const diff)
        {
            if (!Active)
            {
                if (CanIteract <= diff)
                {
                    Active = true;
                    CanIteract = 3500;
                    me->HandleEmoteCommand(EMOTE_ONESHOT_DANCE);
                }
                else
                    CanIteract -= diff;
            }
        }

        void EnterCombat(Unit* /*who*/) {}

        void ReceiveEmote(Player* player, uint32 emote)
        {
            if (me->IsWithinLOS(player->GetPositionX(), player->GetPositionY(), player->GetPositionZ()) && me->IsWithinDistInMap(player, 30.0f))
            {
                me->SetInFront(player);
                Active = false;

                WorldPacket data;
                me->BuildHeartBeatMsg(&data);
                me->SendMessageToSet(&data, true);
                switch (emote)
                {
                    case TEXT_EMOTE_KISS:
                        me->HandleEmoteCommand(EMOTE_ONESHOT_SHY);
                        break;
                    case TEXT_EMOTE_WAVE:
                        me->HandleEmoteCommand(EMOTE_ONESHOT_WAVE);
                        break;
                    case TEXT_EMOTE_BOW:
                        me->HandleEmoteCommand(EMOTE_ONESHOT_BOW);
                        break;
                    case TEXT_EMOTE_JOKE:
                        me->HandleEmoteCommand(EMOTE_ONESHOT_LAUGH);
                        break;
                    case TEXT_EMOTE_DANCE:
                        if (!player->HasAura(SPELL_SEDUCTION))
                            DoCast(player, SPELL_SEDUCTION, true);
                        break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_dancing_flamesAI(creature);
    }
};

/*######
## Triage quest (Signed for 9623)
######*/

enum Triage
{
    SAY_DOC1            = -1000201,
    SAY_DOC2            = -1000202,
    SAY_DOC3            = -1000203,

    DOCTOR_ALLIANCE     = 12939,
    DOCTOR_HORDE        = 12920,

    ALLIANCE_COORDS     = 7,
    HORDE_COORDS        = 6
};

struct Location
{
    float x, y, z, o;
};

static Location AllianceCoords[]=
{
    {-3757.38f, -4533.05f, 14.16f, 3.62f},                     // Top-far-right bunk as seen from entrance
    {-3754.36f, -4539.13f, 14.16f, 5.13f},                     // Top-far-left bunk
    {-3749.54f, -4540.25f, 14.28f, 3.34f},                     // Far-right bunk
    {-3742.10f, -4536.85f, 14.28f, 3.64f},                     // Right bunk near entrance
    {-3755.89f, -4529.07f, 14.05f, 0.57f},                     // Far-left bunk
    {-3749.51f, -4527.08f, 14.07f, 5.26f},                     // Mid-left bunk
    {-3746.37f, -4525.35f, 14.16f, 5.22f},                     // Left bunk near entrance
};

//alliance run to where
#define A_RUNTOX -3742.96f
#define A_RUNTOY -4531.52f
#define A_RUNTOZ 11.91f

static Location HordeCoords[]=
{
    {-1013.75f, -3492.59f, 62.62f, 4.34f},                     // Left, Behind
    {-1017.72f, -3490.92f, 62.62f, 4.34f},                     // Right, Behind
    {-1015.77f, -3497.15f, 62.82f, 4.34f},                     // Left, Mid
    {-1019.51f, -3495.49f, 62.82f, 4.34f},                     // Right, Mid
    {-1017.25f, -3500.85f, 62.98f, 4.34f},                     // Left, front
    {-1020.95f, -3499.21f, 62.98f, 4.34f}                       // Right, Front
};

//horde run to where
#define H_RUNTOX -1016.44f
#define H_RUNTOY -3508.48f
#define H_RUNTOZ 62.96f

uint32 const AllianceSoldierId[3] =
{
    12938,                                                  // 12938 Injured Alliance Soldier
    12936,                                                  // 12936 Badly injured Alliance Soldier
    12937                                                   // 12937 Critically injured Alliance Soldier
};

uint32 const HordeSoldierId[3] =
{
    12923,                                                  //12923 Injured Soldier
    12924,                                                  //12924 Badly injured Soldier
    12925                                                   //12925 Critically injured Soldier
};

/*######
## npc_doctor (handles both Gustaf Vanhowzen and Gregory Victor)
######*/

class npc_doctor : public CreatureScript
{
public:
    npc_doctor() : CreatureScript("npc_doctor") {}

    struct npc_doctorAI : public ScriptedAI
    {
        npc_doctorAI(Creature* creature) : ScriptedAI(creature) {}

        uint64 PlayerGUID;

        uint32 SummonPatientTimer;
        uint32 SummonPatientCount;
        uint32 PatientDiedCount;
        uint32 PatientSavedCount;

        bool Event;

        std::list<uint64> Patients;
        std::vector<Location*> Coordinates;

        void Reset()
        {
            PlayerGUID = 0;

            SummonPatientTimer = 10000;
            SummonPatientCount = 0;
            PatientDiedCount = 0;
            PatientSavedCount = 0;

            Patients.clear();
            Coordinates.clear();

            Event = false;

            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        }

        void BeginEvent(Player* player)
        {
            PlayerGUID = player->GetGUID();

            SummonPatientTimer = 10000;
            SummonPatientCount = 0;
            PatientDiedCount = 0;
            PatientSavedCount = 0;

            switch (me->GetEntry())
            {
                case DOCTOR_ALLIANCE:
                    for (uint8 i = 0; i < ALLIANCE_COORDS; ++i)
                        Coordinates.push_back(&AllianceCoords[i]);
                    break;
                case DOCTOR_HORDE:
                    for (uint8 i = 0; i < HORDE_COORDS; ++i)
                        Coordinates.push_back(&HordeCoords[i]);
                    break;
            }

            Event = true;
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        }

        void PatientDied(Location* point)
        {
            Player* player = Unit::GetPlayer(*me, PlayerGUID);
            if (player && ((player->GetQuestStatus(6624) == QUEST_STATUS_INCOMPLETE) || (player->GetQuestStatus(6622) == QUEST_STATUS_INCOMPLETE)))
            {
                ++PatientDiedCount;

                if (PatientDiedCount > 5 && Event)
                {
                    if (player->GetQuestStatus(6624) == QUEST_STATUS_INCOMPLETE)
                        player->FailQuest(6624);
                    else if (player->GetQuestStatus(6622) == QUEST_STATUS_INCOMPLETE)
                        player->FailQuest(6622);

                    Reset();
                    return;
                }

                Coordinates.push_back(point);
            }
            else
                // If no player or player abandon quest in progress
                Reset();
        }

        void PatientSaved(Creature* /*soldier*/, Player* player, Location* point)
        {
            if (player && PlayerGUID == player->GetGUID())
            {
                if ((player->GetQuestStatus(6624) == QUEST_STATUS_INCOMPLETE) || (player->GetQuestStatus(6622) == QUEST_STATUS_INCOMPLETE))
                {
                    ++PatientSavedCount;

                    if (PatientSavedCount == 15)
                    {
                        if (!Patients.empty())
                        {
                            std::list<uint64>::const_iterator itr;
                            for (itr = Patients.begin(); itr != Patients.end(); ++itr)
                            {
                                if (Creature* patient = Unit::GetCreature(*me, *itr))
                                    patient->setDeathState(JUST_DIED);
                            }
                        }

                        if (player->GetQuestStatus(6624) == QUEST_STATUS_INCOMPLETE)
                            player->AreaExploredOrEventHappens(6624);
                        else if (player->GetQuestStatus(6622) == QUEST_STATUS_INCOMPLETE)
                            player->AreaExploredOrEventHappens(6622);

                        Reset();
                        return;
                    }

                    Coordinates.push_back(point);
                }
            }
        }

        void UpdateAI(uint32 const diff);

        void EnterCombat(Unit* /*who*/) {}
    };

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
    {
        if ((quest->GetQuestId() == 6624) || (quest->GetQuestId() == 6622))
            CAST_AI(npc_doctor::npc_doctorAI, creature->AI())->BeginEvent(player);

        return true;
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_doctorAI(creature);
    }
};

/*#####
## npc_injured_patient (handles all the patients, no matter Horde or Alliance)
#####*/

class npc_injured_patient : public CreatureScript
{
public:
    npc_injured_patient() : CreatureScript("npc_injured_patient") { }

    struct npc_injured_patientAI : public ScriptedAI
    {
        npc_injured_patientAI(Creature* creature) : ScriptedAI(creature) {}

        uint64 DoctorGUID;
        Location* Coord;

        void Reset()
        {
            DoctorGUID = 0;
            Coord = NULL;

            //no select
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

            //no regen health
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);

            //to make them lay with face down
            me->SetUInt32Value(UNIT_FIELD_BYTES_1, UNIT_STAND_STATE_DEAD);

            uint32 mobId = me->GetEntry();

            switch (mobId)
            {                                                   //lower max health
                case 12923:
                case 12938:                                     //Injured Soldier
                    me->SetHealth(me->CountPctFromMaxHealth(75));
                    break;
                case 12924:
                case 12936:                                     //Badly injured Soldier
                    me->SetHealth(me->CountPctFromMaxHealth(50));
                    break;
                case 12925:
                case 12937:                                     //Critically injured Soldier
                    me->SetHealth(me->CountPctFromMaxHealth(25));
                    break;
            }
        }

        void EnterCombat(Unit* /*who*/) {}

        void SpellHit(Unit* caster, SpellInfo const* spell)
        {
            if (caster->GetTypeId() == TYPEID_PLAYER && me->isAlive() && spell->Id == 20804)
            {
                if ((CAST_PLR(caster)->GetQuestStatus(6624) == QUEST_STATUS_INCOMPLETE) || (CAST_PLR(caster)->GetQuestStatus(6622) == QUEST_STATUS_INCOMPLETE))
                    if (DoctorGUID)
                        if (Creature* doctor = Unit::GetCreature(*me, DoctorGUID))
                            CAST_AI(npc_doctor::npc_doctorAI, doctor->AI())->PatientSaved(me, CAST_PLR(caster), Coord);

                //make not selectable
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

                //regen health
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);

                //stand up
                me->SetUInt32Value(UNIT_FIELD_BYTES_1, UNIT_STAND_STATE_STAND);

                DoScriptText(RAND(SAY_DOC1, SAY_DOC2, SAY_DOC3), me);

                uint32 mobId = me->GetEntry();
                me->SetWalk(false);

                switch (mobId)
                {
                    case 12923:
                    case 12924:
                    case 12925:
                        me->GetMotionMaster()->MovePoint(0, H_RUNTOX, H_RUNTOY, H_RUNTOZ);
                        break;
                    case 12936:
                    case 12937:
                    case 12938:
                        me->GetMotionMaster()->MovePoint(0, A_RUNTOX, A_RUNTOY, A_RUNTOZ);
                        break;
                }
            }
        }

        void UpdateAI(uint32 const /*diff*/)
        {
            //lower HP on every world tick makes it a useful counter, not officlone though
            if (me->isAlive() && me->GetHealth() > 6)
                me->ModifyHealth(-5);

            if (me->isAlive() && me->GetHealth() <= 6)
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->setDeathState(JUST_DIED);
                me->SetFlag(UNIT_DYNAMIC_FLAGS, 32);

                if (DoctorGUID)
                    if (Creature* doctor = Unit::GetCreature(*me, DoctorGUID))
                        CAST_AI(npc_doctor::npc_doctorAI, doctor->AI())->PatientDied(Coord);
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_injured_patientAI(creature);
    }
};

void npc_doctor::npc_doctorAI::UpdateAI(uint32 const diff)
{
    if (Event && SummonPatientCount >= 20)
    {
        Reset();
        return;
    }

    if (Event)
    {
        if (SummonPatientTimer <= diff)
        {
            if (Coordinates.empty())
                return;

            std::vector<Location*>::iterator itr = Coordinates.begin() + rand() % Coordinates.size();
            uint32 patientEntry = 0;

            switch (me->GetEntry())
            {
                case DOCTOR_ALLIANCE:
                    patientEntry = AllianceSoldierId[rand() % 3];
                    break;
                case DOCTOR_HORDE:
                    patientEntry = HordeSoldierId[rand() % 3];
                    break;
                default:
                    sLog->outError("TSCR: Invalid entry for Triage doctor. Please check your database");
                    return;
            }

            if (Location* point = *itr)
            {
                if (Creature* Patient = me->SummonCreature(patientEntry, point->x, point->y, point->z, point->o, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000))
                {
                    //303, this flag appear to be required for client side item->spell to work (TARGET_SINGLE_FRIEND)
                    Patient->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE);

                    Patients.push_back(Patient->GetGUID());
                    CAST_AI(npc_injured_patient::npc_injured_patientAI, Patient->AI())->DoctorGUID = me->GetGUID();
                    CAST_AI(npc_injured_patient::npc_injured_patientAI, Patient->AI())->Coord = point;

                    Coordinates.erase(itr);
                }
            }
            SummonPatientTimer = 10000;
            ++SummonPatientCount;
        }
        else
            SummonPatientTimer -= diff;
    }
}

/*######
## npc_garments_of_quests
######*/

//TODO: get text for each NPC

enum Garments
{
    SPELL_LESSER_HEAL_R2    = 2052,
    SPELL_FORTITUDE_R1      = 1243,

    QUEST_MOON              = 5621,
    QUEST_LIGHT_1           = 5624,
    QUEST_LIGHT_2           = 5625,
    QUEST_SPIRIT            = 5648,
    QUEST_DARKNESS          = 5650,

    ENTRY_SHAYA             = 12429,
    ENTRY_ROBERTS           = 12423,
    ENTRY_DOLF              = 12427,
    ENTRY_KORJA             = 12430,
    ENTRY_DG_KEL            = 12428,

    //used by 12429, 12423, 12427, 12430, 12428, but signed for 12429
    SAY_COMMON_HEALED       = -1000164,
    SAY_DG_KEL_THANKS       = -1000165,
    SAY_DG_KEL_GOODBYE      = -1000166,
    SAY_ROBERTS_THANKS      = -1000167,
    SAY_ROBERTS_GOODBYE     = -1000168,
    SAY_KORJA_THANKS        = -1000169,
    SAY_KORJA_GOODBYE       = -1000170,
    SAY_DOLF_THANKS         = -1000171,
    SAY_DOLF_GOODBYE        = -1000172,
    SAY_SHAYA_THANKS        = -1000173,
    SAY_SHAYA_GOODBYE       = -1000174, //signed for 21469
};

class npc_garments_of_quests : public CreatureScript
{
public:
    npc_garments_of_quests() : CreatureScript("npc_garments_of_quests") { }

    struct npc_garments_of_questsAI : public npc_escortAI
    {
        npc_garments_of_questsAI(Creature* creature) : npc_escortAI(creature) { Reset(); }

        uint64 CasterGUID;

        bool IsHealed;
        bool CanRun;

        uint32 RunAwayTimer;

        void Reset()
        {
            CasterGUID = 0;

            IsHealed = false;
            CanRun = false;

            RunAwayTimer = 5000;

            me->SetStandState(UNIT_STAND_STATE_KNEEL);
            //expect database to have RegenHealth=0
            me->SetHealth(me->CountPctFromMaxHealth(70));
        }

        void EnterCombat(Unit* /*who*/) {}

        void SpellHit(Unit* caster, SpellInfo const* Spell)
        {
            if (Spell->Id == SPELL_LESSER_HEAL_R2 || Spell->Id == SPELL_FORTITUDE_R1)
            {
                //not while in combat
                if (me->isInCombat())
                    return;

                //nothing to be done now
                if (IsHealed && CanRun)
                    return;

                if (Player* player = caster->ToPlayer())
                {
                    switch (me->GetEntry())
                    {
                        case ENTRY_SHAYA:
                            if (player->GetQuestStatus(QUEST_MOON) == QUEST_STATUS_INCOMPLETE)
                            {
                                if (IsHealed && !CanRun && Spell->Id == SPELL_FORTITUDE_R1)
                                {
                                    DoScriptText(SAY_SHAYA_THANKS, me, caster);
                                    CanRun = true;
                                }
                                else if (!IsHealed && Spell->Id == SPELL_LESSER_HEAL_R2)
                                {
                                    CasterGUID = caster->GetGUID();
                                    me->SetStandState(UNIT_STAND_STATE_STAND);
                                    DoScriptText(SAY_COMMON_HEALED, me, caster);
                                    IsHealed = true;
                                }
                            }
                            break;
                        case ENTRY_ROBERTS:
                            if (player->GetQuestStatus(QUEST_LIGHT_1) == QUEST_STATUS_INCOMPLETE)
                            {
                                if (IsHealed && !CanRun && Spell->Id == SPELL_FORTITUDE_R1)
                                {
                                    DoScriptText(SAY_ROBERTS_THANKS, me, caster);
                                    CanRun = true;
                                }
                                else if (!IsHealed && Spell->Id == SPELL_LESSER_HEAL_R2)
                                {
                                    CasterGUID = caster->GetGUID();
                                    me->SetStandState(UNIT_STAND_STATE_STAND);
                                    DoScriptText(SAY_COMMON_HEALED, me, caster);
                                    IsHealed = true;
                                }
                            }
                            break;
                        case ENTRY_DOLF:
                            if (player->GetQuestStatus(QUEST_LIGHT_2) == QUEST_STATUS_INCOMPLETE)
                            {
                                if (IsHealed && !CanRun && Spell->Id == SPELL_FORTITUDE_R1)
                                {
                                    DoScriptText(SAY_DOLF_THANKS, me, caster);
                                    CanRun = true;
                                }
                                else if (!IsHealed && Spell->Id == SPELL_LESSER_HEAL_R2)
                                {
                                    CasterGUID = caster->GetGUID();
                                    me->SetStandState(UNIT_STAND_STATE_STAND);
                                    DoScriptText(SAY_COMMON_HEALED, me, caster);
                                    IsHealed = true;
                                }
                            }
                            break;
                        case ENTRY_KORJA:
                            if (player->GetQuestStatus(QUEST_SPIRIT) == QUEST_STATUS_INCOMPLETE)
                            {
                                if (IsHealed && !CanRun && Spell->Id == SPELL_FORTITUDE_R1)
                                {
                                    DoScriptText(SAY_KORJA_THANKS, me, caster);
                                    CanRun = true;
                                }
                                else if (!IsHealed && Spell->Id == SPELL_LESSER_HEAL_R2)
                                {
                                    CasterGUID = caster->GetGUID();
                                    me->SetStandState(UNIT_STAND_STATE_STAND);
                                    DoScriptText(SAY_COMMON_HEALED, me, caster);
                                    IsHealed = true;
                                }
                            }
                            break;
                        case ENTRY_DG_KEL:
                            if (player->GetQuestStatus(QUEST_DARKNESS) == QUEST_STATUS_INCOMPLETE)
                            {
                                if (IsHealed && !CanRun && Spell->Id == SPELL_FORTITUDE_R1)
                                {
                                    DoScriptText(SAY_DG_KEL_THANKS, me, caster);
                                    CanRun = true;
                                }
                                else if (!IsHealed && Spell->Id == SPELL_LESSER_HEAL_R2)
                                {
                                    CasterGUID = caster->GetGUID();
                                    me->SetStandState(UNIT_STAND_STATE_STAND);
                                    DoScriptText(SAY_COMMON_HEALED, me, caster);
                                    IsHealed = true;
                                }
                            }
                            break;
                    }

                    //give quest credit, not expect any special quest objectives
                    if (CanRun)
                        player->TalkedToCreature(me->GetEntry(), me->GetGUID());
                }
            }
        }

        void WaypointReached(uint32 /*point*/)
        {
        }

        void UpdateAI(uint32 const diff)
        {
            if (CanRun && !me->isInCombat())
            {
                if (RunAwayTimer <= diff)
                {
                    if (Unit* unit = Unit::GetUnit(*me, CasterGUID))
                    {
                        switch (me->GetEntry())
                        {
                            case ENTRY_SHAYA:
                                DoScriptText(SAY_SHAYA_GOODBYE, me, unit);
                                break;
                            case ENTRY_ROBERTS:
                                DoScriptText(SAY_ROBERTS_GOODBYE, me, unit);
                                break;
                            case ENTRY_DOLF:
                                DoScriptText(SAY_DOLF_GOODBYE, me, unit);
                                break;
                            case ENTRY_KORJA:
                                DoScriptText(SAY_KORJA_GOODBYE, me, unit);
                                break;
                            case ENTRY_DG_KEL:
                                DoScriptText(SAY_DG_KEL_GOODBYE, me, unit);
                                break;
                        }

                        Start(false, true, true);
                    }
                    else
                        EnterEvadeMode();                       //something went wrong

                    RunAwayTimer = 30000;
                }
                else
                    RunAwayTimer -= diff;
            }

            npc_escortAI::UpdateAI(diff);
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_garments_of_questsAI(creature);
    }
};

/*######
## npc_guardian
######*/

enum GuardianSpells
{
    SPELL_DEATHTOUCH       = 5
};

class npc_guardian : public CreatureScript
{
public:
    npc_guardian() : CreatureScript("npc_guardian") { }

    struct npc_guardianAI : public ScriptedAI
    {
        npc_guardianAI(Creature* creature) : ScriptedAI(creature) {}

        void Reset()
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        }

        void EnterCombat(Unit* /*who*/)
        {
        }

        void UpdateAI(uint32 const /*diff*/)
        {
            if (!UpdateVictim())
                return;

            if (me->isAttackReady())
            {
                DoCast(me->getVictim(), SPELL_DEATHTOUCH, true);
                me->resetAttackTimer();
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_guardianAI(creature);
    }
};

/*######
## npc_kingdom_of_dalaran_quests
######*/

enum KingdomDalaran
{
    SPELL_TELEPORT_DALARAN  = 53360,
    ITEM_KT_SIGNET          = 39740,
    QUEST_MAGICAL_KINGDOM_A = 12794,
    QUEST_MAGICAL_KINGDOM_H = 12791,
    QUEST_MAGICAL_KINGDOM_N = 12796
};

#define GOSSIP_ITEM_TELEPORT_TO "I am ready to be teleported to Dalaran."

class npc_kingdom_of_dalaran_quests : public CreatureScript
{
public:
    npc_kingdom_of_dalaran_quests() : CreatureScript("npc_kingdom_of_dalaran_quests") { }
    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (creature->isQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        if (player->HasItemCount(ITEM_KT_SIGNET, 1) && (!player->GetQuestRewardStatus(QUEST_MAGICAL_KINGDOM_A) ||
            !player->GetQuestRewardStatus(QUEST_MAGICAL_KINGDOM_H) || !player->GetQuestRewardStatus(QUEST_MAGICAL_KINGDOM_N)))
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_TELEPORT_TO, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* /*creature*/, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        if (action == GOSSIP_ACTION_INFO_DEF + 1)
        {
            player->CLOSE_GOSSIP_MENU();
            player->CastSpell(player, SPELL_TELEPORT_DALARAN, false);
        }
        return true;
    }
};

/*######
## npc_mount_vendor
######*/

class npc_mount_vendor : public CreatureScript
{
public:
    npc_mount_vendor() : CreatureScript("npc_mount_vendor") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (creature->isQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        bool canBuy = false;
        uint32 vendor = creature->GetEntry();
        uint8 race = player->getRace();

        switch (vendor)
        {
            case 384:                                           //Katie Hunter
            case 1460:                                          //Unger Statforth
            case 2357:                                          //Merideth Carlson
            case 4885:                                          //Gregor MacVince
                if (player->GetReputationRank(72) != REP_EXALTED && race != RACE_HUMAN)
                    player->SEND_GOSSIP_MENU(5855, creature->GetGUID());
                else canBuy = true;
                break;
            case 1261:                                          //Veron Amberstill
                if (player->GetReputationRank(47) != REP_EXALTED && race != RACE_DWARF)
                    player->SEND_GOSSIP_MENU(5856, creature->GetGUID());
                else canBuy = true;
                break;
            case 3362:                                          //Ogunaro Wolfrunner
                if (player->GetReputationRank(76) != REP_EXALTED && race != RACE_ORC)
                    player->SEND_GOSSIP_MENU(5841, creature->GetGUID());
                else canBuy = true;
                break;
            case 3685:                                          //Harb Clawhoof
                if (player->GetReputationRank(81) != REP_EXALTED && race != RACE_TAUREN)
                    player->SEND_GOSSIP_MENU(5843, creature->GetGUID());
                else canBuy = true;
                break;
            case 4730:                                          //Lelanai
                if (player->GetReputationRank(69) != REP_EXALTED && race != RACE_NIGHTELF)
                    player->SEND_GOSSIP_MENU(5844, creature->GetGUID());
                else canBuy = true;
                break;
            case 4731:                                          //Zachariah Post
                if (player->GetReputationRank(68) != REP_EXALTED && race != RACE_UNDEAD_PLAYER)
                    player->SEND_GOSSIP_MENU(5840, creature->GetGUID());
                else canBuy = true;
                break;
            case 7952:                                          //Zjolnir
                if (player->GetReputationRank(530) != REP_EXALTED && race != RACE_TROLL)
                    player->SEND_GOSSIP_MENU(5842, creature->GetGUID());
                else canBuy = true;
                break;
            case 7955:                                          //Milli Featherwhistle
                if (player->GetReputationRank(54) != REP_EXALTED && race != RACE_GNOME)
                    player->SEND_GOSSIP_MENU(5857, creature->GetGUID());
                else canBuy = true;
                break;
            case 16264:                                         //Winaestra
                if (player->GetReputationRank(911) != REP_EXALTED && race != RACE_BLOODELF)
                    player->SEND_GOSSIP_MENU(10305, creature->GetGUID());
                else canBuy = true;
                break;
            case 17584:                                         //Torallius the Pack Handler
                if (player->GetReputationRank(930) != REP_EXALTED && race != RACE_DRAENEI)
                    player->SEND_GOSSIP_MENU(10239, creature->GetGUID());
                else canBuy = true;
                break;
        }

        if (canBuy)
        {
            if (creature->isVendor())
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, GOSSIP_TEXT_BROWSE_GOODS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);
            player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
        }
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        if (action == GOSSIP_ACTION_TRADE)
            player->GetSession()->SendListInventory(creature->GetGUID());

        return true;
    }
};

/*######
## npc_rogue_trainer
######*/

#define GOSSIP_HELLO_ROGUE1 "I wish to unlearn my talents"
#define GOSSIP_HELLO_ROGUE2 "<Take the letter>"
#define GOSSIP_HELLO_ROGUE3 "Purchase a Dual Talent Specialization."

class npc_rogue_trainer : public CreatureScript
{
public:
    npc_rogue_trainer() : CreatureScript("npc_rogue_trainer") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (creature->isQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        if (creature->isTrainer())
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER, GOSSIP_TEXT_TRAIN, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRAIN);

        if (creature->isCanTrainingAndResetTalentsOf(player))
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER, GOSSIP_HELLO_ROGUE1, GOSSIP_SENDER_MAIN, GOSSIP_OPTION_UNLEARNTALENTS);

        if (player->GetSpecsCount() == 1 && creature->isCanTrainingAndResetTalentsOf(player) && player->getLevel() >= sWorld->getIntConfig(CONFIG_MIN_DUALSPEC_LEVEL))
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER, GOSSIP_HELLO_ROGUE3, GOSSIP_SENDER_MAIN, GOSSIP_OPTION_LEARNDUALSPEC);

        if (player->getClass() == CLASS_ROGUE && player->getLevel() >= 24 && !player->HasItemCount(17126, 1) && !player->GetQuestRewardStatus(6681))
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_HELLO_ROGUE2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            player->SEND_GOSSIP_MENU(5996, creature->GetGUID());
        } else
            player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());

        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        switch (action)
        {
            case GOSSIP_ACTION_INFO_DEF + 1:
                player->CLOSE_GOSSIP_MENU();
                player->CastSpell(player, 21100, false);
                break;
            case GOSSIP_ACTION_TRAIN:
                player->GetSession()->SendTrainerList(creature->GetGUID());
                break;
            case GOSSIP_OPTION_UNLEARNTALENTS:
                player->CLOSE_GOSSIP_MENU();
                player->SendTalentWipeConfirm(creature->GetGUID());
                break;
            case GOSSIP_OPTION_LEARNDUALSPEC:
                if (player->GetSpecsCount() == 1 && !(player->getLevel() < sWorld->getIntConfig(CONFIG_MIN_DUALSPEC_LEVEL)))
                {
                    if (!player->HasEnoughMoney(100000))
                    {
                        player->SendBuyError(BUY_ERR_NOT_ENOUGHT_MONEY, 0, 0, 0);
                        player->PlayerTalkClass->SendCloseGossip();
                        break;
                    }
                    else
                    {
                        player->ModifyMoney(-100000);

                        // Cast spells that teach dual spec
                        // Both are also ImplicitTarget self and must be cast by player
                        player->CastSpell(player, 63680, true, NULL, NULL, player->GetGUID());
                        player->CastSpell(player, 63624, true, NULL, NULL, player->GetGUID());

                        // Should show another Gossip text with "Congratulations..."
                        player->PlayerTalkClass->SendCloseGossip();
                    }
                }
                break;
        }
        return true;
    }
};

/*######
## npc_sayge
######*/

enum Sayge
{
    SPELL_DMG       = 23768,    // damage
    SPELL_RES       = 23769,    // Resistance
    SPELL_ARM       = 23767,    // armor
    SPELL_SPI       = 23738,    // spirit
    SPELL_INT       = 23766,    // intellect
    SPELL_STM       = 23737,    // stamina
    SPELL_STR       = 23735,    // strength
    SPELL_AGI       = 23736,    // agility
    SPELL_FORTUNE   = 23765     // faire fortune
};

#define GOSSIP_HELLO_SAYGE          "Yes"
#define GOSSIP_SENDACTION_SAYGE1    "Slay the Man"
#define GOSSIP_SENDACTION_SAYGE2    "Turn him over to liege"
#define GOSSIP_SENDACTION_SAYGE3    "Confiscate the corn"
#define GOSSIP_SENDACTION_SAYGE4    "Let him go and have the corn"
#define GOSSIP_SENDACTION_SAYGE5    "Execute your friend painfully"
#define GOSSIP_SENDACTION_SAYGE6    "Execute your friend painlessly"
#define GOSSIP_SENDACTION_SAYGE7    "Let your friend go"
#define GOSSIP_SENDACTION_SAYGE8    "Confront the diplomat"
#define GOSSIP_SENDACTION_SAYGE9    "Show not so quiet defiance"
#define GOSSIP_SENDACTION_SAYGE10   "Remain quiet"
#define GOSSIP_SENDACTION_SAYGE11   "Speak against your brother openly"
#define GOSSIP_SENDACTION_SAYGE12   "Help your brother in"
#define GOSSIP_SENDACTION_SAYGE13   "Keep your brother out without letting him know"
#define GOSSIP_SENDACTION_SAYGE14   "Take credit, keep gold"
#define GOSSIP_SENDACTION_SAYGE15   "Take credit, share the gold"
#define GOSSIP_SENDACTION_SAYGE16   "Let the knight take credit"
#define GOSSIP_SENDACTION_SAYGE17   "Thanks"

class npc_sayge : public CreatureScript
{
public:
    npc_sayge() : CreatureScript("npc_sayge") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (creature->isQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        if (player->HasSpellCooldown(SPELL_INT) ||
            player->HasSpellCooldown(SPELL_ARM) ||
            player->HasSpellCooldown(SPELL_DMG) ||
            player->HasSpellCooldown(SPELL_RES) ||
            player->HasSpellCooldown(SPELL_STR) ||
            player->HasSpellCooldown(SPELL_AGI) ||
            player->HasSpellCooldown(SPELL_STM) ||
            player->HasSpellCooldown(SPELL_SPI))
            player->SEND_GOSSIP_MENU(7393, creature->GetGUID());
        else
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_HELLO_SAYGE, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            player->SEND_GOSSIP_MENU(7339, creature->GetGUID());
        }

        return true;
    }

    void SendAction(Player* player, Creature* creature, uint32 action)
    {
        switch (action)
        {
            case GOSSIP_ACTION_INFO_DEF + 1:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE1,            GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE2,            GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE3,            GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE4,            GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);
                player->SEND_GOSSIP_MENU(7340, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF + 2:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE5,            GOSSIP_SENDER_MAIN + 1, GOSSIP_ACTION_INFO_DEF);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE6,            GOSSIP_SENDER_MAIN + 2, GOSSIP_ACTION_INFO_DEF);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE7,            GOSSIP_SENDER_MAIN + 3, GOSSIP_ACTION_INFO_DEF);
                player->SEND_GOSSIP_MENU(7341, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF + 3:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE8,            GOSSIP_SENDER_MAIN + 4, GOSSIP_ACTION_INFO_DEF);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE9,            GOSSIP_SENDER_MAIN + 5, GOSSIP_ACTION_INFO_DEF);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE10,           GOSSIP_SENDER_MAIN + 2, GOSSIP_ACTION_INFO_DEF);
                player->SEND_GOSSIP_MENU(7361, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF + 4:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE11,           GOSSIP_SENDER_MAIN + 6, GOSSIP_ACTION_INFO_DEF);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE12,           GOSSIP_SENDER_MAIN + 7, GOSSIP_ACTION_INFO_DEF);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE13,           GOSSIP_SENDER_MAIN + 8, GOSSIP_ACTION_INFO_DEF);
                player->SEND_GOSSIP_MENU(7362, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF + 5:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE14,           GOSSIP_SENDER_MAIN + 5, GOSSIP_ACTION_INFO_DEF);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE15,           GOSSIP_SENDER_MAIN + 4, GOSSIP_ACTION_INFO_DEF);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE16,           GOSSIP_SENDER_MAIN + 3, GOSSIP_ACTION_INFO_DEF);
                player->SEND_GOSSIP_MENU(7363, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE17,           GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 6);
                player->SEND_GOSSIP_MENU(7364, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF + 6:
                creature->CastSpell(player, SPELL_FORTUNE, false);
                player->SEND_GOSSIP_MENU(7365, creature->GetGUID());
                break;
        }
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        switch (sender)
        {
            case GOSSIP_SENDER_MAIN:
                SendAction(player, creature, action);
                break;
            case GOSSIP_SENDER_MAIN + 1:
                creature->CastSpell(player, SPELL_DMG, false);
                player->AddSpellCooldown(SPELL_DMG, 0, time(NULL) + 7200);
                SendAction(player, creature, action);
                break;
            case GOSSIP_SENDER_MAIN + 2:
                creature->CastSpell(player, SPELL_RES, false);
                player->AddSpellCooldown(SPELL_RES, 0, time(NULL) + 7200);
                SendAction(player, creature, action);
                break;
            case GOSSIP_SENDER_MAIN + 3:
                creature->CastSpell(player, SPELL_ARM, false);
                player->AddSpellCooldown(SPELL_ARM, 0, time(NULL) + 7200);
                SendAction(player, creature, action);
                break;
            case GOSSIP_SENDER_MAIN + 4:
                creature->CastSpell(player, SPELL_SPI, false);
                player->AddSpellCooldown(SPELL_SPI, 0, time(NULL) + 7200);
                SendAction(player, creature, action);
                break;
            case GOSSIP_SENDER_MAIN + 5:
                creature->CastSpell(player, SPELL_INT, false);
                player->AddSpellCooldown(SPELL_INT, 0, time(NULL) + 7200);
                SendAction(player, creature, action);
                break;
            case GOSSIP_SENDER_MAIN + 6:
                creature->CastSpell(player, SPELL_STM, false);
                player->AddSpellCooldown(SPELL_STM, 0, time(NULL) + 7200);
                SendAction(player, creature, action);
                break;
            case GOSSIP_SENDER_MAIN + 7:
                creature->CastSpell(player, SPELL_STR, false);
                player->AddSpellCooldown(SPELL_STR, 0, time(NULL) + 7200);
                SendAction(player, creature, action);
                break;
            case GOSSIP_SENDER_MAIN + 8:
                creature->CastSpell(player, SPELL_AGI, false);
                player->AddSpellCooldown(SPELL_AGI, 0, time(NULL) + 7200);
                SendAction(player, creature, action);
                break;
        }
        return true;
    }
};

// Steam Tonk

enum Tonk
{
    SPELL_TONK_MINE_DETONATE  = 25099
};

class npc_steam_tonk : public CreatureScript
{
public:
    npc_steam_tonk() : CreatureScript("npc_steam_tonk") { }

    struct npc_steam_tonkAI : public ScriptedAI
    {
        npc_steam_tonkAI(Creature* creature) : ScriptedAI(creature) {}

        void Reset() {}
        void EnterCombat(Unit* /*who*/) {}

        void OnPossess(bool apply)
        {
            if (apply)
            {
                // Initialize the action bar without the melee attack command
                me->InitCharmInfo();
                me->GetCharmInfo()->InitEmptyActionBar(false);

                me->SetReactState(REACT_PASSIVE);
            }
            else
                me->SetReactState(REACT_AGGRESSIVE);
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_steam_tonkAI(creature);
    }
};

class npc_tonk_mine : public CreatureScript
{
public:
    npc_tonk_mine() : CreatureScript("npc_tonk_mine") { }

    struct npc_tonk_mineAI : public ScriptedAI
    {
        npc_tonk_mineAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetReactState(REACT_PASSIVE);
        }

        uint32 ExplosionTimer;

        void Reset()
        {
            ExplosionTimer = 3000;
        }

        void EnterCombat(Unit* /*who*/) {}
        void AttackStart(Unit* /*who*/) {}
        void MoveInLineOfSight(Unit* /*who*/) {}

        void UpdateAI(uint32 const diff)
        {
            if (ExplosionTimer <= diff)
            {
                DoCast(me, SPELL_TONK_MINE_DETONATE, true);
                me->setDeathState(DEAD); // unsummon it
            }
            else
                ExplosionTimer -= diff;
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_tonk_mineAI(creature);
    }
};

/*####
## npc_brewfest_reveler
####*/

class npc_brewfest_reveler : public CreatureScript
{
public:
    npc_brewfest_reveler() : CreatureScript("npc_brewfest_reveler") { }

    struct npc_brewfest_revelerAI : public ScriptedAI
    {
        npc_brewfest_revelerAI(Creature* creature) : ScriptedAI(creature) {}
        void ReceiveEmote(Player* player, uint32 emote)
        {
            if (!IsHolidayActive(HOLIDAY_BREWFEST))
                return;

            if (emote == TEXT_EMOTE_DANCE)
                me->CastSpell(player, 41586, false);
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_brewfest_revelerAI(creature);
    }
};

/*####
## npc_winter_reveler
####*/

enum WinterReveler
{
    SPELL_MISTLETOE_DEBUFF       = 26218,
    SPELL_CREATE_MISTLETOE       = 26206,
    SPELL_CREATE_HOLLY           = 26207,
    SPELL_CREATE_SNOWFLAKES      = 45036,
};

class npc_winter_reveler : public CreatureScript
{
    public:
        npc_winter_reveler() : CreatureScript("npc_winter_reveler") { }

        struct npc_winter_revelerAI : public ScriptedAI
        {
            npc_winter_revelerAI(Creature* creature) : ScriptedAI(creature) {}

            void ReceiveEmote(Player* player, uint32 emote)
            {
                if (player->HasAura(SPELL_MISTLETOE_DEBUFF))
                    return;

                if (!IsHolidayActive(HOLIDAY_FEAST_OF_WINTER_VEIL))
                    return;

                if (emote == TEXT_EMOTE_KISS)
                {
                    uint32 spellId = RAND<uint32>(SPELL_CREATE_MISTLETOE, SPELL_CREATE_HOLLY, SPELL_CREATE_SNOWFLAKES);
                    me->CastSpell(player, spellId, false);
                    me->CastSpell(player, SPELL_MISTLETOE_DEBUFF, false);
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_winter_revelerAI(creature);
        }
};

/*####
## npc_snake_trap_serpents
####*/
enum TrapSpells
{
    SPELL_MIND_NUMBING_POISON    = 25810,   //Viper
    SPELL_DEADLY_POISON          = 34655,   //Venomous Snake
    SPELL_CRIPPLING_POISON       = 30981,   //Viper
    NPC_VIPER                    = 19921
};

#define VENOMOUS_SNAKE_TIMER   1500
#define VIPER_TIMER            3000

class npc_snake_trap : public CreatureScript
{
public:
    npc_snake_trap() : CreatureScript("npc_snake_trap_serpents") { }

    struct npc_snake_trap_serpentsAI : public ScriptedAI
    {
        npc_snake_trap_serpentsAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 SpellTimer;
        bool IsViper;

        void EnterCombat(Unit* /*who*/) {}

        void Reset()
        {
            SpellTimer = 0;

            CreatureTemplate const* Info = me->GetCreatureTemplate();

            IsViper = Info->Entry == NPC_VIPER ? true : false;

            me->SetMaxHealth(uint32(107 * (me->getLevel() - 40) * 0.025f));
            //Add delta to make them not all hit the same time
            uint32 delta = (rand() % 7) * 100;
            me->SetStatFloatValue(UNIT_FIELD_BASEATTACKTIME, float(Info->baseattacktime + delta));
            me->SetStatFloatValue(UNIT_FIELD_RANGED_ATTACK_POWER , float(Info->attackpower));

            // Start attacking attacker of owner on first ai update after spawn - move in line of sight may choose better target
            if (!me->getVictim() && me->isSummon())
                if (Unit* Owner = me->ToTempSummon()->GetSummoner())
                    if (Owner->getAttackerForHelper())
                        AttackStart(Owner->getAttackerForHelper());
        }

        //Redefined for random target selection:
        void MoveInLineOfSight(Unit* who)
        {
            if (!me->getVictim() && me->canCreatureAttack(who))
            {
                if (me->GetDistanceZ(who) > CREATURE_Z_ATTACK_RANGE)
                    return;

                float attackRadius = me->GetAttackDistance(who);
                if (me->IsWithinDistInMap(who, attackRadius) && me->IsWithinLOSInMap(who))
                {
                    if (!(rand() % 5))
                    {
                        me->setAttackTimer(BASE_ATTACK, (rand() % 10) * 100);
                        SpellTimer = (rand() % 10) * 100;
                        AttackStart(who);
                    }
                }
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            if (me->getVictim()->HasBreakableByDamageCrowdControlAura(me))
            {
                me->InterruptNonMeleeSpells(false);
                return;
            }

            if (SpellTimer <= diff)
            {
                if (IsViper) //Viper
                {
                    if (urand(0, 2) == 0) //33% chance to cast
                    {
                        uint32 spell;
                        if (urand(0, 1) == 0)
                            spell = SPELL_MIND_NUMBING_POISON;
                        else
                            spell = SPELL_CRIPPLING_POISON;

                        DoCast(me->getVictim(), spell);
                    }

                    SpellTimer = VIPER_TIMER;
                }
                else //Venomous Snake
                {
                    if (urand(0, 2) == 0) //33% chance to cast
                        DoCast(me->getVictim(), SPELL_DEADLY_POISON);
                    SpellTimer = VENOMOUS_SNAKE_TIMER + (rand() % 5) * 100;
                }
            }
            else
                SpellTimer -= diff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_snake_trap_serpentsAI(creature);
    }
};

// Mob Mojo
#define SAY_RANDOM_MOJO0    "Now that's what I call froggy-style!"
#define SAY_RANDOM_MOJO1    "Your lily pad or mine?"
#define SAY_RANDOM_MOJO2    "This won't take long, did it?"
#define SAY_RANDOM_MOJO3    "I thought you'd never ask!"
#define SAY_RANDOM_MOJO4    "I promise not to give you warts..."
#define SAY_RANDOM_MOJO5    "Feelin' a little froggy, are ya?"
#define SAY_RANDOM_MOJO6a   "Listen, "
#define SAY_RANDOM_MOJO6b   ", I know of a little swamp not too far from here...."
#define SAY_RANDOM_MOJO7    "There's just never enough Mojo to go around..."

class mob_mojo : public CreatureScript
{
public:
    mob_mojo() : CreatureScript("mob_mojo") { }

    struct mob_mojoAI : public ScriptedAI
    {
        mob_mojoAI(Creature* creature) : ScriptedAI(creature) {Reset();}

        uint32 Hearts;
        uint64 VictimGUID;

        void Reset()
        {
            VictimGUID = 0;
            Hearts = 15000;
            if (Unit* own = me->GetOwner())
                me->GetMotionMaster()->MoveFollow(own, 0, 0);
        }

        void EnterCombat(Unit* /*who*/) {}

        void UpdateAI(uint32 const diff)
        {
            if (me->HasAura(20372))
            {
                if (Hearts <= diff)
                {
                    me->RemoveAurasDueToSpell(20372);
                    Hearts = 15000;
                }
                Hearts -= diff;
            }
        }

        void ReceiveEmote(Player* player, uint32 emote)
        {
            me->HandleEmoteCommand(emote);
            Unit* own = me->GetOwner();
            if (!own || own->GetTypeId() != TYPEID_PLAYER || CAST_PLR(own)->GetTeam() != player->GetTeam())
                return;
            if (emote == TEXT_EMOTE_KISS)
            {
                std::string whisp = "";
                switch (rand() % 8)
                {
                    case 0:
                        whisp.append(SAY_RANDOM_MOJO0);
                        break;
                    case 1:
                        whisp.append(SAY_RANDOM_MOJO1);
                        break;
                    case 2:
                        whisp.append(SAY_RANDOM_MOJO2);
                        break;
                    case 3:
                        whisp.append(SAY_RANDOM_MOJO3);
                        break;
                    case 4:
                        whisp.append(SAY_RANDOM_MOJO4);
                        break;
                    case 5:
                        whisp.append(SAY_RANDOM_MOJO5);
                        break;
                    case 6:
                        whisp.append(SAY_RANDOM_MOJO6a);
                        whisp.append(player->GetName());
                        whisp.append(SAY_RANDOM_MOJO6b);
                        break;
                    case 7:
                        whisp.append(SAY_RANDOM_MOJO7);
                        break;
                }

                me->MonsterWhisper(whisp.c_str(), player->GetGUID());
                if (VictimGUID)
                    if (Player* victim = Unit::GetPlayer(*me, VictimGUID))
                        victim->RemoveAura(43906);//remove polymorph frog thing
                me->AddAura(43906, player);//add polymorph frog thing
                VictimGUID = player->GetGUID();
                DoCast(me, 20372, true);//tag.hearts
                me->GetMotionMaster()->MoveFollow(player, 0, 0);
                Hearts = 15000;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_mojoAI(creature);
    }
};

// Mirror Image
class npc_mirror_image : public CreatureScript
{
public:
    npc_mirror_image() : CreatureScript("npc_mirror_image") { }

    struct npc_mirror_imageAI : CasterAI
    {
        npc_mirror_imageAI(Creature* creature) : CasterAI(creature) {}

        void InitializeAI()
        {
            CasterAI::InitializeAI();
            Unit* owner = me->GetOwner();
            if (!owner)
                return;
            // Inherit Master's Threat List (not yet implemented)
            owner->CastSpell((Unit*)NULL, 58838, true);
            // here mirror image casts on summoner spell (not present in client dbc) 49866
            // here should be auras (not present in client dbc): 35657, 35658, 35659, 35660 selfcasted by mirror images (stats related?)
            // Clone Me!
            owner->CastSpell(me, 45204, false);
        }

        // Do not reload Creature templates on evade mode enter - prevent visual lost
        void EnterEvadeMode()
        {
            if (me->IsInEvadeMode() || !me->isAlive())
                return;

            Unit* owner = me->GetCharmerOrOwner();

            me->CombatStop(true);
            if (owner && !me->HasUnitState(UNIT_STATE_FOLLOW))
            {
                me->GetMotionMaster()->Clear(false);
                me->GetMotionMaster()->MoveFollow(owner, PET_FOLLOW_DIST, me->GetFollowAngle(), MOTION_SLOT_ACTIVE);
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_mirror_imageAI(creature);
    }
};

// ebon gargoyle
class npc_ebon_gargoyle : public CreatureScript
{
public:
    npc_ebon_gargoyle() : CreatureScript("npc_ebon_gargoyle") { }

    struct npc_ebon_gargoyleAI : CasterAI
    {
        npc_ebon_gargoyleAI(Creature* creature) : CasterAI(creature) {}

        uint32 DespawnTimer;

        void InitializeAI()
        {
            CasterAI::InitializeAI();
            uint64 ownerGuid = me->GetOwnerGUID();
            if (!ownerGuid)
                return;
            // Not needed to be despawned now
            DespawnTimer = 0;
            // Find victim of Summon Gargoyle spell
            std::list<Unit*> targets;
            SkyFire::AnyUnfriendlyUnitInObjectRangeCheck u_check(me, me, 30);
            SkyFire::UnitListSearcher<SkyFire::AnyUnfriendlyUnitInObjectRangeCheck> searcher(me, targets, u_check);
            me->VisitNearbyObject(30, searcher);
            for (std::list<Unit*>::const_iterator iter = targets.begin(); iter != targets.end(); ++iter)
                if ((*iter)->GetAura(49206, ownerGuid))
                {
                    me->Attack((*iter), false);
                    break;
                }
        }

        void JustDied(Unit* /*killer*/)
        {
            // Stop Feeding Gargoyle when it dies
            if (Unit* owner = me->GetOwner())
                owner->RemoveAurasDueToSpell(50514);
        }

        // Fly away when dismissed
        void SpellHit(Unit* source, SpellInfo const* spell)
        {
            if (spell->Id != 50515 || !me->isAlive())
                return;

            Unit* owner = me->GetOwner();

            if (!owner || owner != source)
                return;

            // Stop Fighting
            me->ApplyModFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE, true);
            // Sanctuary
            me->CastSpell(me, 54661, true);
            me->SetReactState(REACT_PASSIVE);

            // Fly Away
            me->AddUnitMovementFlag(MOVEMENTFLAG_CAN_FLY|MOVEMENTFLAG_ASCENDING|MOVEMENTFLAG_FLYING);
            me->SetSpeed(MOVE_FLIGHT, 0.75f, true);
            me->SetSpeed(MOVE_RUN, 0.75f, true);
            float x = me->GetPositionX() + 20 * cos(me->GetOrientation());
            float y = me->GetPositionY() + 20 * sin(me->GetOrientation());
            float z = me->GetPositionZ() + 40;
            me->GetMotionMaster()->Clear(false);
            me->GetMotionMaster()->MovePoint(0, x, y, z);

            // Despawn as soon as possible
            DespawnTimer = 4 * IN_MILLISECONDS;
        }

        void UpdateAI(const uint32 diff)
        {
            if (DespawnTimer > 0)
            {
                if (DespawnTimer > diff)
                    DespawnTimer -= diff;
                else
                    me->DespawnOrUnsummon();
                return;
            }
            CasterAI::UpdateAI(diff);
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_ebon_gargoyleAI(creature);
    }
};

// Light Well
class npc_lightwell : public CreatureScript
{
public:
    npc_lightwell() : CreatureScript("npc_lightwell") { }

    struct npc_lightwellAI : public PassiveAI
    {
        npc_lightwellAI(Creature* creature) : PassiveAI(creature) {}

        void Reset()
        {
            DoCast(me, 59907, false); // Spell for Lightwell Charges
        }

        void EnterEvadeMode()
        {
            if (!me->isAlive())
                return;

            me->DeleteThreatList();
            me->CombatStop(true);
            me->ResetPlayerDamageReq();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_lightwellAI(creature);
    }
};

// Training dummy
enum eTrainingDummy
{
    NPC_ADVANCED_TARGET_DUMMY                  = 2674,
    NPC_TARGET_DUMMY                           = 2673
};

class npc_training_dummy : public CreatureScript
{
public:
    npc_training_dummy() : CreatureScript("npc_training_dummy") { }

    struct npc_training_dummyAI : Scripted_NoMovementAI
    {
        npc_training_dummyAI(Creature* creature) : Scripted_NoMovementAI(creature)
        {
            Entry = creature->GetEntry();
        }

        uint32 Entry;
        uint32 ResetTimer;
        uint32 DespawnTimer;

        void Reset()
        {
            me->SetControlled(true, UNIT_STATE_STUNNED);//disable rotate
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);//imune to knock aways like blast wave

            ResetTimer = 5000;
            DespawnTimer = 15000;
        }

        void EnterEvadeMode()
        {
            if (!_EnterEvadeMode())
                return;

            Reset();
        }

        void DamageTaken(Unit* /*doneBy*/, uint32& damage)
        {
            ResetTimer = 5000;
            damage = 0;
        }

        void EnterCombat(Unit* /*who*/)
        {
            if (Entry != NPC_ADVANCED_TARGET_DUMMY && Entry != NPC_TARGET_DUMMY)
                return;
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            if (!me->HasUnitState(UNIT_STATE_STUNNED))
                me->SetControlled(true, UNIT_STATE_STUNNED);//disable rotate

            if (Entry != NPC_ADVANCED_TARGET_DUMMY && Entry != NPC_TARGET_DUMMY)
            {
                if (ResetTimer <= diff)
                {
                    EnterEvadeMode();
                    ResetTimer = 5000;
                }
                else
                    ResetTimer -= diff;
                return;
            }
            else
            {
                if (DespawnTimer <= diff)
                    me->DespawnOrUnsummon();
                else
                    DespawnTimer -= diff;
            }
        }
        void MoveInLineOfSight(Unit* /*who*/){return;}
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_training_dummyAI(creature);
    }
};

//Sentinel Hill Gnolls
//Should only take damage if it's from a player
// 42407, 124, 452, 501, 
class npc_gnoll_sh : public CreatureScript
{
public:
	npc_gnoll_sh() : CreatureScript("npc_gnoll_sh") { }

	struct npc_gnoll_shAI : public ScriptedAI
	{
		npc_gnoll_shAI(Creature* creature) : ScriptedAI(creature) {}

		void DamageTaken(Unit* done_by, uint32& damage)
		{
			if (done_by && done_by->GetTypeId() != TYPEID_PLAYER) // If damage dealt is by someone other than a player no damage is dealt.
			{
				damage = 0;
			}
		}

		void UpdateAI(uint32 const /*diff*/)
		{
			if (!UpdateVictim())
				return;

			DoMeleeAttackIfReady();
		}
	};

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_gnoll_shAI(creature);
	}
};

/*######
# npc_shadowfiend
######*/

enum Shadowfiend
{
    MANA_LEECH                       = 28305,
    GLYPH_OF_SHADOWFIEND_MANA        = 58227,
    GLYPH_OF_SHADOWFIEND             = 58228
};

class npc_shadowfiend : public CreatureScript
{
public:
    npc_shadowfiend() : CreatureScript("npc_shadowfiend") { }

    struct npc_shadowfiendAI : public ScriptedAI
    {
        npc_shadowfiendAI(Creature* creature) : ScriptedAI(creature) {}

        void Reset()
        {
            if (me->isSummon())
                if (Unit* owner = me->ToTempSummon()->GetSummoner())
                    if (Unit* pet = owner->GetGuardianPet())
                        pet->CastSpell(pet, MANA_LEECH, true);
        }

        void DamageTaken(Unit* /*killer*/, uint32& damage)
        {
            if (me->isSummon())
                if (Unit* owner = me->ToTempSummon()->GetSummoner())
                    if (owner->HasAura(GLYPH_OF_SHADOWFIEND) && damage >= me->GetHealth())
                        owner->CastSpell(owner, GLYPH_OF_SHADOWFIEND_MANA, true);
        }

        void UpdateAI(uint32 const /*diff*/)
        {
            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_shadowfiendAI(creature);
    }
};

/*######
# npc_wormhole
######*/

#define GOSSIP_ENGINEERING1   "Borean Tundra."
#define GOSSIP_ENGINEERING2   "Howling Fjord."
#define GOSSIP_ENGINEERING3   "Sholazar Basin."
#define GOSSIP_ENGINEERING4   "Icecrown."
#define GOSSIP_ENGINEERING5   "Storm Peaks."

enum Wormhole
{
    SPELL_HOWLING_FJORD         = 67838,
    SPELL_SHOLAZAR_BASIN        = 67835,
    SPELL_ICECROWN              = 67836,
    SPELL_STORM_PEAKS           = 67837,

    TEXT_WORMHOLE               = 907
};

class npc_wormhole : public CreatureScript
{
public:
    npc_wormhole() : CreatureScript("npc_wormhole") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (creature->isSummon())
        {
            if (player == creature->ToTempSummon()->GetSummoner())
            {
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ENGINEERING1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ENGINEERING2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ENGINEERING3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ENGINEERING4, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ENGINEERING5, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);

                player->PlayerTalkClass->SendGossipMenu(TEXT_WORMHOLE, creature->GetGUID());
            }
        }
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* /*creature*/, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        bool roll = urand(0, 1);

        switch (action)
        {
            case GOSSIP_ACTION_INFO_DEF + 1: //Borean Tundra
                player->CLOSE_GOSSIP_MENU();
                if (roll) //At the moment we don't have chance on spell_target_position table so we hack this
                    player->TeleportTo(571, 4305.505859f, 5450.839844f, 63.005806f, 0.627286f);
                else
                    player->TeleportTo(571, 3201.936279f, 5630.123535f, 133.658798f, 3.855272f);
                break;
            case GOSSIP_ACTION_INFO_DEF + 2: //Howling Fjord
                player->CLOSE_GOSSIP_MENU();
                player->CastSpell(player, SPELL_HOWLING_FJORD, true);
                break;
            case GOSSIP_ACTION_INFO_DEF + 3: //Sholazar Basin
                player->CLOSE_GOSSIP_MENU();
                player->CastSpell(player, SPELL_SHOLAZAR_BASIN, true);
                break;
            case GOSSIP_ACTION_INFO_DEF + 4: //Icecrown
                player->CLOSE_GOSSIP_MENU();
                player->CastSpell(player, SPELL_ICECROWN, true);
                break;
            case GOSSIP_ACTION_INFO_DEF + 5: //Storm peaks
                player->CLOSE_GOSSIP_MENU();
                player->CastSpell(player, SPELL_STORM_PEAKS, true);
                break;
        }
        return true;
    }
};

/*######
## npc_pet_trainer
######*/

enum PetTrainer
{
    TEXT_ISHUNTER               = 5838,
    TEXT_NOTHUNTER              = 5839,
    TEXT_PETINFO                = 13474,
    TEXT_CONFIRM                = 7722
};

#define GOSSIP_PET1             "How do I train my pet?"
#define GOSSIP_PET2             "I wish to untrain my pet."
#define GOSSIP_PET_CONFIRM      "Yes, please do."

class npc_pet_trainer : public CreatureScript
{
public:
    npc_pet_trainer() : CreatureScript("npc_pet_trainer") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (creature->isQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        if (player->getClass() == CLASS_HUNTER)
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_PET1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            if (player->GetPet() && player->GetPet()->getPetType() == HUNTER_PET)
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_PET2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);

            player->PlayerTalkClass->SendGossipMenu(TEXT_ISHUNTER, creature->GetGUID());
            return true;
        }
        player->PlayerTalkClass->SendGossipMenu(TEXT_NOTHUNTER, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        switch (action)
        {
            case GOSSIP_ACTION_INFO_DEF + 1:
                player->PlayerTalkClass->SendGossipMenu(TEXT_PETINFO, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF + 2:
                {
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_PET_CONFIRM, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
                    player->PlayerTalkClass->SendGossipMenu(TEXT_CONFIRM, creature->GetGUID());
                }
                break;
            case GOSSIP_ACTION_INFO_DEF + 3:
                {
                    player->ResetPetTalents();
                    player->CLOSE_GOSSIP_MENU();
                }
                break;
        }
        return true;
    }
};

/*######
## npc_locksmith
######*/

enum LockSmith
{
    QUEST_HOW_TO_BRAKE_IN_TO_THE_ARCATRAZ = 10704,
    QUEST_DARK_IRON_LEGACY                = 3802,
    QUEST_THE_KEY_TO_SCHOLOMANCE_A        = 5505,
    QUEST_THE_KEY_TO_SCHOLOMANCE_H        = 5511,
    QUEST_HOTTER_THAN_HELL_A              = 10758,
    QUEST_HOTTER_THAN_HELL_H              = 10764,
    QUEST_RETURN_TO_KHAGDAR               = 9837,
    QUEST_CONTAINMENT                     = 13159,

    ITEM_ARCATRAZ_KEY                     = 31084,
    ITEM_SHADOWFORGE_KEY                  = 11000,
    ITEM_SKELETON_KEY                     = 13704,
    ITEM_SHATTERED_HALLS_KEY              = 28395,
    ITEM_THE_MASTERS_KEY                  = 24490,
    ITEM_VIOLET_HOLD_KEY                  = 42482,

    SPELL_ARCATRAZ_KEY                    = 54881,
    SPELL_SHADOWFORGE_KEY                 = 54882,
    SPELL_SKELETON_KEY                    = 54883,
    SPELL_SHATTERED_HALLS_KEY             = 54884,
    SPELL_THE_MASTERS_KEY                 = 54885,
    SPELL_VIOLET_HOLD_KEY                 = 67253
};

#define GOSSIP_LOST_ARCATRAZ_KEY         "I've lost my key to the Arcatraz."
#define GOSSIP_LOST_SHADOWFORGE_KEY      "I've lost my key to the Blackrock Depths."
#define GOSSIP_LOST_SKELETON_KEY         "I've lost my key to the Scholomance."
#define GOSSIP_LOST_SHATTERED_HALLS_KEY  "I've lost my key to the Shattered Halls."
#define GOSSIP_LOST_THE_MASTERS_KEY      "I've lost my key to the Karazhan."
#define GOSSIP_LOST_VIOLET_HOLD_KEY      "I've lost my key to the Violet Hold."

class npc_locksmith : public CreatureScript
{
public:
    npc_locksmith() : CreatureScript("npc_locksmith") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        // Arcatraz Key
        if (player->GetQuestRewardStatus(QUEST_HOW_TO_BRAKE_IN_TO_THE_ARCATRAZ) && !player->HasItemCount(ITEM_ARCATRAZ_KEY, 1, true))
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_ARCATRAZ_KEY, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

        // Shadowforge Key
        if (player->GetQuestRewardStatus(QUEST_DARK_IRON_LEGACY) && !player->HasItemCount(ITEM_SHADOWFORGE_KEY, 1, true))
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_SHADOWFORGE_KEY, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);

        // Skeleton Key
        if ((player->GetQuestRewardStatus(QUEST_THE_KEY_TO_SCHOLOMANCE_A) || player->GetQuestRewardStatus(QUEST_THE_KEY_TO_SCHOLOMANCE_H)) &&
            !player->HasItemCount(ITEM_SKELETON_KEY, 1, true))
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_SKELETON_KEY, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);

        // Shatered Halls Key
        if ((player->GetQuestRewardStatus(QUEST_HOTTER_THAN_HELL_A) || player->GetQuestRewardStatus(QUEST_HOTTER_THAN_HELL_H)) &&
            !player->HasItemCount(ITEM_SHATTERED_HALLS_KEY, 1, true))
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_SHATTERED_HALLS_KEY, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);

        // Master's Key
        if (player->GetQuestRewardStatus(QUEST_RETURN_TO_KHAGDAR) && !player->HasItemCount(ITEM_THE_MASTERS_KEY, 1, true))
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_THE_MASTERS_KEY, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);

        // Violet Hold Key
        if (player->GetQuestRewardStatus(QUEST_CONTAINMENT) && !player->HasItemCount(ITEM_VIOLET_HOLD_KEY, 1, true))
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_VIOLET_HOLD_KEY, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 6);

        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());

        return true;
    }

    bool OnGossipSelect(Player* player, Creature* /*creature*/, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        switch (action)
        {
            case GOSSIP_ACTION_INFO_DEF + 1:
                player->CLOSE_GOSSIP_MENU();
                player->CastSpell(player, SPELL_ARCATRAZ_KEY, false);
                break;
            case GOSSIP_ACTION_INFO_DEF + 2:
                player->CLOSE_GOSSIP_MENU();
                player->CastSpell(player, SPELL_SHADOWFORGE_KEY, false);
                break;
            case GOSSIP_ACTION_INFO_DEF + 3:
                player->CLOSE_GOSSIP_MENU();
                player->CastSpell(player, SPELL_SKELETON_KEY, false);
                break;
            case GOSSIP_ACTION_INFO_DEF + 4:
                player->CLOSE_GOSSIP_MENU();
                player->CastSpell(player, SPELL_SHATTERED_HALLS_KEY, false);
                break;
            case GOSSIP_ACTION_INFO_DEF + 5:
                player->CLOSE_GOSSIP_MENU();
                player->CastSpell(player, SPELL_THE_MASTERS_KEY, false);
                break;
            case GOSSIP_ACTION_INFO_DEF + 6:
                player->CLOSE_GOSSIP_MENU();
                player->CastSpell(player, SPELL_VIOLET_HOLD_KEY, false);
                break;
        }
        return true;
    }
};

/*######
## npc_tabard_vendor
######*/

enum TabardVendor
{
    QUEST_TRUE_MASTERS_OF_LIGHT = 9737,
    QUEST_THE_UNWRITTEN_PROPHECY = 9762,
    QUEST_INTO_THE_BREACH = 10259,
    QUEST_BATTLE_OF_THE_CRIMSON_WATCH = 10781,
    QUEST_SHARDS_OF_AHUNE = 11972,

    ACHIEVEMENT_EXPLORE_NORTHREND = 45,
    ACHIEVEMENT_TWENTYFIVE_TABARDS = 1021,
    ACHIEVEMENT_THE_LOREMASTER_A = 1681,
    ACHIEVEMENT_THE_LOREMASTER_H = 1682,

    ITEM_TABARD_OF_THE_HAND = 24344,
    ITEM_TABARD_OF_THE_BLOOD_KNIGHT = 25549,
    ITEM_TABARD_OF_THE_PROTECTOR = 28788,
    ITEM_OFFERING_OF_THE_SHATAR = 31408,
    ITEM_GREEN_TROPHY_TABARD_OF_THE_ILLIDARI = 31404,
    ITEM_PURPLE_TROPHY_TABARD_OF_THE_ILLIDARI = 31405,
    ITEM_TABARD_OF_THE_SUMMER_SKIES = 35279,
    ITEM_TABARD_OF_THE_SUMMER_FLAMES = 35280,
    ITEM_TABARD_OF_THE_ACHIEVER = 40643,
    ITEM_LOREMASTERS_COLORS = 43300,
    ITEM_TABARD_OF_THE_EXPLORER = 43348,

    SPELL_TABARD_OF_THE_BLOOD_KNIGHT = 54974,
    SPELL_TABARD_OF_THE_HAND = 54976,
    SPELL_GREEN_TROPHY_TABARD_OF_THE_ILLIDARI = 54977,
    SPELL_PURPLE_TROPHY_TABARD_OF_THE_ILLIDARI = 54982,
    SPELL_TABARD_OF_THE_ACHIEVER = 55006,
    SPELL_TABARD_OF_THE_PROTECTOR = 55008,
    SPELL_LOREMASTERS_COLORS = 58194,
    SPELL_TABARD_OF_THE_EXPLORER = 58224,
    SPELL_TABARD_OF_SUMMER_SKIES = 62768,
    SPELL_TABARD_OF_SUMMER_FLAMES = 62769
};

#define GOSSIP_LOST_TABARD_OF_BLOOD_KNIGHT "I've lost my Tabard of Blood Knight."
#define GOSSIP_LOST_TABARD_OF_THE_HAND "I've lost my Tabard of the Hand."
#define GOSSIP_LOST_TABARD_OF_THE_PROTECTOR "I've lost my Tabard of the Protector."
#define GOSSIP_LOST_GREEN_TROPHY_TABARD_OF_THE_ILLIDARI "I've lost my Green Trophy Tabard of the Illidari."
#define GOSSIP_LOST_PURPLE_TROPHY_TABARD_OF_THE_ILLIDARI "I've lost my Purple Trophy Tabard of the Illidari."
#define GOSSIP_LOST_TABARD_OF_SUMMER_SKIES "I've lost my Tabard of Summer Skies."
#define GOSSIP_LOST_TABARD_OF_SUMMER_FLAMES "I've lost my Tabard of Summer Flames."
#define GOSSIP_LOST_LOREMASTERS_COLORS "I've lost my Loremaster's Colors."
#define GOSSIP_LOST_TABARD_OF_THE_EXPLORER "I've lost my Tabard of the Explorer."
#define GOSSIP_LOST_TABARD_OF_THE_ACHIEVER "I've lost my Tabard of the Achiever."

class npc_tabard_vendor : public CreatureScript
{
public:
    npc_tabard_vendor() : CreatureScript("npc_tabard_vendor") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        bool lostBloodKnight = false;
        bool lostHand = false;
        bool lostProtector = false;
        bool lostIllidari = false;
        bool lostSummer = false;

        //Tabard of the Blood Knight
        if (player->GetQuestRewardStatus(QUEST_TRUE_MASTERS_OF_LIGHT) && !player->HasItemCount(ITEM_TABARD_OF_THE_BLOOD_KNIGHT, 1, true))
            lostBloodKnight = true;

        //Tabard of the Hand
        if (player->GetQuestRewardStatus(QUEST_THE_UNWRITTEN_PROPHECY) && !player->HasItemCount(ITEM_TABARD_OF_THE_HAND, 1, true))
            lostHand = true;

        //Tabard of the Protector
        if (player->GetQuestRewardStatus(QUEST_INTO_THE_BREACH) && !player->HasItemCount(ITEM_TABARD_OF_THE_PROTECTOR, 1, true))
            lostProtector = true;

        //Green Trophy Tabard of the Illidari
        //Purple Trophy Tabard of the Illidari
        if (player->GetQuestRewardStatus(QUEST_BATTLE_OF_THE_CRIMSON_WATCH) &&
            (!player->HasItemCount(ITEM_GREEN_TROPHY_TABARD_OF_THE_ILLIDARI, 1, true) &&
            !player->HasItemCount(ITEM_PURPLE_TROPHY_TABARD_OF_THE_ILLIDARI, 1, true) &&
            !player->HasItemCount(ITEM_OFFERING_OF_THE_SHATAR, 1, true)))
            lostIllidari = true;

        //Tabard of Summer Skies
        //Tabard of Summer Flames
        if (player->GetQuestRewardStatus(QUEST_SHARDS_OF_AHUNE) &&
            !player->HasItemCount(ITEM_TABARD_OF_THE_SUMMER_SKIES, 1, true) &&
            !player->HasItemCount(ITEM_TABARD_OF_THE_SUMMER_FLAMES, 1, true))
            lostSummer = true;

        if (lostBloodKnight || lostHand || lostProtector || lostIllidari || lostSummer)
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, GOSSIP_TEXT_BROWSE_GOODS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);

            if (lostBloodKnight)
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_TABARD_OF_BLOOD_KNIGHT, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

            if (lostHand)
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_TABARD_OF_THE_HAND, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);

            if (lostProtector)
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_TABARD_OF_THE_PROTECTOR, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);

            if (lostIllidari)
            {
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_GREEN_TROPHY_TABARD_OF_THE_ILLIDARI, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_PURPLE_TROPHY_TABARD_OF_THE_ILLIDARI, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);
            }

            if (lostSummer)
            {
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_TABARD_OF_SUMMER_SKIES, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 6);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_TABARD_OF_SUMMER_FLAMES, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 7);
            }

            player->SEND_GOSSIP_MENU(13583, creature->GetGUID());
        }
        else
            player->GetSession()->SendListInventory(creature->GetGUID());

        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        switch (action)
        {
            case GOSSIP_ACTION_TRADE:
                player->GetSession()->SendListInventory(creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF + 1:
                player->CLOSE_GOSSIP_MENU();
                player->CastSpell(player, SPELL_TABARD_OF_THE_BLOOD_KNIGHT, false);
                break;
            case GOSSIP_ACTION_INFO_DEF + 2:
                player->CLOSE_GOSSIP_MENU();
                player->CastSpell(player, SPELL_TABARD_OF_THE_HAND, false);
                break;
            case GOSSIP_ACTION_INFO_DEF + 3:
                player->CLOSE_GOSSIP_MENU();
                player->CastSpell(player, SPELL_TABARD_OF_THE_PROTECTOR, false);
                break;
            case GOSSIP_ACTION_INFO_DEF + 4:
                player->CLOSE_GOSSIP_MENU();
                player->CastSpell(player, SPELL_GREEN_TROPHY_TABARD_OF_THE_ILLIDARI, false);
                break;
            case GOSSIP_ACTION_INFO_DEF + 5:
                player->CLOSE_GOSSIP_MENU();
                player->CastSpell(player, SPELL_PURPLE_TROPHY_TABARD_OF_THE_ILLIDARI, false);
                break;
            case GOSSIP_ACTION_INFO_DEF + 6:
                player->CLOSE_GOSSIP_MENU();
                player->CastSpell(player, SPELL_TABARD_OF_SUMMER_SKIES, false);
                break;
            case GOSSIP_ACTION_INFO_DEF + 7:
                player->CLOSE_GOSSIP_MENU();
                player->CastSpell(player, SPELL_TABARD_OF_SUMMER_FLAMES, false);
                break;
        }
        return true;
    }
};

/*######
## npc_experience
######*/

#define EXP_COST                100000 //10 00 00 copper (10golds)
#define GOSSIP_TEXT_EXP         14736
#define GOSSIP_XP_OFF           "I no longer wish to gain experience."
#define GOSSIP_XP_ON            "I wish to start gaining experience again."

class npc_experience : public CreatureScript
{
public:
    npc_experience() : CreatureScript("npc_experience") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_XP_OFF, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_XP_ON, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
        player->PlayerTalkClass->SendGossipMenu(GOSSIP_TEXT_EXP, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* /*creature*/, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        bool noXPGain = player->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_NO_XP_GAIN);
        bool doSwitch = false;

        switch (action)
        {
            case GOSSIP_ACTION_INFO_DEF + 1://xp off
                {
                    if (!noXPGain)//does gain xp
                        doSwitch = true;//switch to don't gain xp
                }
                break;
            case GOSSIP_ACTION_INFO_DEF + 2://xp on
                {
                    if (noXPGain)//doesn't gain xp
                        doSwitch = true;//switch to gain xp
                }
                break;
        }
        if (doSwitch)
        {
            if (!player->HasEnoughMoney(EXP_COST))
                player->SendBuyError(BUY_ERR_NOT_ENOUGHT_MONEY, 0, 0, 0);
            else if (noXPGain)
            {
                player->ModifyMoney(-EXP_COST);
                player->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_NO_XP_GAIN);
            }
            else if (!noXPGain)
            {
                player->ModifyMoney(-EXP_COST);
                player->SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_NO_XP_GAIN);
            }
        }
        player->PlayerTalkClass->SendCloseGossip();
        return true;
    }
};

enum Fireworks
{
    NPC_OMEN                = 15467,
    NPC_MINION_OF_OMEN      = 15466,
    NPC_FIREWORK_BLUE       = 15879,
    NPC_FIREWORK_GREEN      = 15880,
    NPC_FIREWORK_PURPLE     = 15881,
    NPC_FIREWORK_RED        = 15882,
    NPC_FIREWORK_YELLOW     = 15883,
    NPC_FIREWORK_WHITE      = 15884,
    NPC_FIREWORK_BIG_BLUE   = 15885,
    NPC_FIREWORK_BIG_GREEN  = 15886,
    NPC_FIREWORK_BIG_PURPLE = 15887,
    NPC_FIREWORK_BIG_RED    = 15888,
    NPC_FIREWORK_BIG_YELLOW = 15889,
    NPC_FIREWORK_BIG_WHITE  = 15890,

    NPC_CLUSTER_BLUE        = 15872,
    NPC_CLUSTER_RED         = 15873,
    NPC_CLUSTER_GREEN       = 15874,
    NPC_CLUSTER_PURPLE      = 15875,
    NPC_CLUSTER_WHITE       = 15876,
    NPC_CLUSTER_YELLOW      = 15877,
    NPC_CLUSTER_BIG_BLUE    = 15911,
    NPC_CLUSTER_BIG_GREEN   = 15912,
    NPC_CLUSTER_BIG_PURPLE  = 15913,
    NPC_CLUSTER_BIG_RED     = 15914,
    NPC_CLUSTER_BIG_WHITE   = 15915,
    NPC_CLUSTER_BIG_YELLOW  = 15916,
    NPC_CLUSTER_ELUNE       = 15918,

    GO_FIREWORK_LAUNCHER_1  = 180771,
    GO_FIREWORK_LAUNCHER_2  = 180868,
    GO_FIREWORK_LAUNCHER_3  = 180850,
    GO_CLUSTER_LAUNCHER_1   = 180772,
    GO_CLUSTER_LAUNCHER_2   = 180859,
    GO_CLUSTER_LAUNCHER_3   = 180869,
    GO_CLUSTER_LAUNCHER_4   = 180874,

    SPELL_ROCKET_BLUE       = 26344,
    SPELL_ROCKET_GREEN      = 26345,
    SPELL_ROCKET_PURPLE     = 26346,
    SPELL_ROCKET_RED        = 26347,
    SPELL_ROCKET_WHITE      = 26348,
    SPELL_ROCKET_YELLOW     = 26349,
    SPELL_ROCKET_BIG_BLUE   = 26351,
    SPELL_ROCKET_BIG_GREEN  = 26352,
    SPELL_ROCKET_BIG_PURPLE = 26353,
    SPELL_ROCKET_BIG_RED    = 26354,
    SPELL_ROCKET_BIG_WHITE  = 26355,
    SPELL_ROCKET_BIG_YELLOW = 26356,
    SPELL_LUNAR_FORTUNE     = 26522,

    ANIM_GO_LAUNCH_FIREWORK = 3,
    ZONE_MOONGLADE          = 493,
};

Position omenSummonPos = {7558.993f, -2839.999f, 450.0214f, 4.46f};

class npc_firework : public CreatureScript
{
public:
    npc_firework() : CreatureScript("npc_firework") { }

    struct npc_fireworkAI : public ScriptedAI
    {
        npc_fireworkAI(Creature* creature) : ScriptedAI(creature) {}

        bool isCluster()
        {
            switch (me->GetEntry())
            {
                case NPC_FIREWORK_BLUE:
                case NPC_FIREWORK_GREEN:
                case NPC_FIREWORK_PURPLE:
                case NPC_FIREWORK_RED:
                case NPC_FIREWORK_YELLOW:
                case NPC_FIREWORK_WHITE:
                case NPC_FIREWORK_BIG_BLUE:
                case NPC_FIREWORK_BIG_GREEN:
                case NPC_FIREWORK_BIG_PURPLE:
                case NPC_FIREWORK_BIG_RED:
                case NPC_FIREWORK_BIG_YELLOW:
                case NPC_FIREWORK_BIG_WHITE:
                    return false;
                case NPC_CLUSTER_BLUE:
                case NPC_CLUSTER_GREEN:
                case NPC_CLUSTER_PURPLE:
                case NPC_CLUSTER_RED:
                case NPC_CLUSTER_YELLOW:
                case NPC_CLUSTER_WHITE:
                case NPC_CLUSTER_BIG_BLUE:
                case NPC_CLUSTER_BIG_GREEN:
                case NPC_CLUSTER_BIG_PURPLE:
                case NPC_CLUSTER_BIG_RED:
                case NPC_CLUSTER_BIG_YELLOW:
                case NPC_CLUSTER_BIG_WHITE:
                case NPC_CLUSTER_ELUNE:
                default:
                    return true;
            }
        }

        GameObject* FindNearestLauncher()
        {
            GameObject* launcher = NULL;

            if (isCluster())
            {
                GameObject* launcher1 = GetClosestGameObjectWithEntry(me, GO_CLUSTER_LAUNCHER_1, 0.5f);
                GameObject* launcher2 = GetClosestGameObjectWithEntry(me, GO_CLUSTER_LAUNCHER_2, 0.5f);
                GameObject* launcher3 = GetClosestGameObjectWithEntry(me, GO_CLUSTER_LAUNCHER_3, 0.5f);
                GameObject* launcher4 = GetClosestGameObjectWithEntry(me, GO_CLUSTER_LAUNCHER_4, 0.5f);

                if (launcher1)
                    launcher = launcher1;
                else if (launcher2)
                    launcher = launcher2;
                else if (launcher3)
                    launcher = launcher3;
                else if (launcher4)
                    launcher = launcher4;
            }
            else
            {
                GameObject* launcher1 = GetClosestGameObjectWithEntry(me, GO_FIREWORK_LAUNCHER_1, 0.5f);
                GameObject* launcher2 = GetClosestGameObjectWithEntry(me, GO_FIREWORK_LAUNCHER_2, 0.5f);
                GameObject* launcher3 = GetClosestGameObjectWithEntry(me, GO_FIREWORK_LAUNCHER_3, 0.5f);

                if (launcher1)
                    launcher = launcher1;
                else if (launcher2)
                    launcher = launcher2;
                else if (launcher3)
                    launcher = launcher3;
            }

            return launcher;
        }

        uint32 GetFireworkSpell(uint32 entry)
        {
            switch (entry)
            {
                case NPC_FIREWORK_BLUE:
                    return SPELL_ROCKET_BLUE;
                case NPC_FIREWORK_GREEN:
                    return SPELL_ROCKET_GREEN;
                case NPC_FIREWORK_PURPLE:
                    return SPELL_ROCKET_PURPLE;
                case NPC_FIREWORK_RED:
                    return SPELL_ROCKET_RED;
                case NPC_FIREWORK_YELLOW:
                    return SPELL_ROCKET_YELLOW;
                case NPC_FIREWORK_WHITE:
                    return SPELL_ROCKET_WHITE;
                case NPC_FIREWORK_BIG_BLUE:
                    return SPELL_ROCKET_BIG_BLUE;
                case NPC_FIREWORK_BIG_GREEN:
                    return SPELL_ROCKET_BIG_GREEN;
                case NPC_FIREWORK_BIG_PURPLE:
                    return SPELL_ROCKET_BIG_PURPLE;
                case NPC_FIREWORK_BIG_RED:
                    return SPELL_ROCKET_BIG_RED;
                case NPC_FIREWORK_BIG_YELLOW:
                    return SPELL_ROCKET_BIG_YELLOW;
                case NPC_FIREWORK_BIG_WHITE:
                    return SPELL_ROCKET_BIG_WHITE;
                default:
                    return 0;
            }
        }

        uint32 GetFireworkGameObjectId()
        {
            uint32 spellId = 0;

            switch (me->GetEntry())
            {
                case NPC_CLUSTER_BLUE:
                    spellId = GetFireworkSpell(NPC_FIREWORK_BLUE);
                    break;
                case NPC_CLUSTER_GREEN:
                    spellId = GetFireworkSpell(NPC_FIREWORK_GREEN);
                    break;
                case NPC_CLUSTER_PURPLE:
                    spellId = GetFireworkSpell(NPC_FIREWORK_PURPLE);
                    break;
                case NPC_CLUSTER_RED:
                    spellId = GetFireworkSpell(NPC_FIREWORK_RED);
                    break;
                case NPC_CLUSTER_YELLOW:
                    spellId = GetFireworkSpell(NPC_FIREWORK_YELLOW);
                    break;
                case NPC_CLUSTER_WHITE:
                    spellId = GetFireworkSpell(NPC_FIREWORK_WHITE);
                    break;
                case NPC_CLUSTER_BIG_BLUE:
                    spellId = GetFireworkSpell(NPC_FIREWORK_BIG_BLUE);
                    break;
                case NPC_CLUSTER_BIG_GREEN:
                    spellId = GetFireworkSpell(NPC_FIREWORK_BIG_GREEN);
                    break;
                case NPC_CLUSTER_BIG_PURPLE:
                    spellId = GetFireworkSpell(NPC_FIREWORK_BIG_PURPLE);
                    break;
                case NPC_CLUSTER_BIG_RED:
                    spellId = GetFireworkSpell(NPC_FIREWORK_BIG_RED);
                    break;
                case NPC_CLUSTER_BIG_YELLOW:
                    spellId = GetFireworkSpell(NPC_FIREWORK_BIG_YELLOW);
                    break;
                case NPC_CLUSTER_BIG_WHITE:
                    spellId = GetFireworkSpell(NPC_FIREWORK_BIG_WHITE);
                    break;
                case NPC_CLUSTER_ELUNE:
                    spellId = GetFireworkSpell(urand(NPC_FIREWORK_BLUE, NPC_FIREWORK_WHITE));
                    break;
            }

            const SpellInfo* spellInfo = sSpellMgr->GetSpellInfo(spellId);

            if (spellInfo && spellInfo->Effects[0].Effect == SPELL_EFFECT_SUMMON_OBJECT_WILD)
                return spellInfo->Effects[0].MiscValue;

            return 0;
        }

        void Reset()
        {
            if (GameObject* launcher = FindNearestLauncher())
            {
                launcher->SendCustomAnim(ANIM_GO_LAUNCH_FIREWORK);
                me->SetOrientation(launcher->GetOrientation() + M_PI/2);
            }
            else
                return;

            if (isCluster())
            {
                // Check if we are near Elune'ara lake south, if so try to summon Omen or a minion
                if (me->GetZoneId() == ZONE_MOONGLADE)
                {
                    if (!me->FindNearestCreature(NPC_OMEN, 100.0f, false) && me->GetDistance2d(omenSummonPos.GetPositionX(), omenSummonPos.GetPositionY()) <= 100.0f)
                    {
                        switch (urand(0,9))
                        {
                            case 0:
                            case 1:
                            case 2:
                            case 3:
                                if (Creature* minion = me->SummonCreature(NPC_MINION_OF_OMEN, me->GetPositionX()+frand(-5.0f, 5.0f), me->GetPositionY()+frand(-5.0f, 5.0f), me->GetPositionZ(), 0.0f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 20000))
                                    minion->AI()->AttackStart(me->SelectNearestPlayer(20.0f));
                                break;
                            case 9:
                                me->SummonCreature(NPC_OMEN, omenSummonPos);
                                break;
                        }
                    }
                }
                if (me->GetEntry() == NPC_CLUSTER_ELUNE)
                    DoCast(SPELL_LUNAR_FORTUNE);

                float displacement = 0.7f;
                for (uint8 i = 0; i < 4; i++)
                    me->SummonGameObject(GetFireworkGameObjectId(), me->GetPositionX() + (i%2 == 0 ? displacement : -displacement), me->GetPositionY() + (i > 1 ? displacement : -displacement), me->GetPositionZ() + 4.0f, me->GetOrientation(), 0.0f, 0.0f, 0.0f, 0.0f, 1);
            }
            else
                //me->CastSpell(me, GetFireworkSpell(me->GetEntry()), true);
                me->CastSpell(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), GetFireworkSpell(me->GetEntry()), true);
        }
    };

    CreatureAI *GetAI(Creature* creature) const
    {
        return new npc_fireworkAI(creature);
    }
};

// Uncomment this once guardians are able to cast spells
// on owner at AI initialization and be able to cast spells based on owner's triggered spellcasts.

/* enum GuardianSpellsAndEntries
{
    NPC_PROTECTION_GUARDIAN         = 46490,
    NPC_HOLY_GUARDIAN               = 46499,
    NPC_RETRI_GUARDIAN              = 46506,
    SPELL_ANCIENT_GUARDIAN_VISUAL   = 86657,
    SPELL_ANCIENT_CRUSADER_PLAYER   = 86703,
    SPELL_ANCIENT_CRUSADER_GUARDIAN = 86701
};

class npc_guardian_of_ancient_kings : public CreatureScript
{
public:
    npc_guardian_of_ancient_kings() : CreatureScript("npc_guardian_of_ancient_kings") { }

    struct npc_guardian_of_ancient_kingsAI : public ScriptedAI
    {
        npc_guardian_of_ancient_kingsAI(Creature *creature) : ScriptedAI(creature)
        {
            _healcount = 0;
        }

        void InitializeAI()
        {
            Unit* owner = me->GetOwner();

            if (!owner || owner->GetTypeId() != TYPEID_PLAYER)
                return;

            if (me->GetEntry() == NPC_PROTECTION_GUARDIAN)
            {
                me->SetUnitMovementFlags(MOVEMENTFLAG_ROOT);
                me->SetReactState(REACT_PASSIVE);
                me->CastSpell(owner, SPELL_ANCIENT_GUARDIAN_VISUAL, true);
            }
        }

        void UpdateAI(const uint32 diff)
        {
            Unit* owner = me->GetOwner();

            if (!UpdateVictim() || !owner)
                return;

            if (me->GetEntry() == NPC_RETRI_GUARDIAN) // Only the retpaladin guardian hits melee,
                DoMeleeAttackIfReady();  // at least is what i saw on vids.
        }

        void SpellHitTarget(Unit* /*target*, SpellInfo const* spell)
        {
            if (me->GetEntry() == NPC_HOLY_GUARDIAN) // Holy paladin guardian
            {
                for (uint32 i = 0; i = MAX_SPELL_EFFECTS;)
                {
                    if (spell->Effects[i].Effect == SPELL_EFFECT_HEAL // if the spell is a heal...
                        && spell->Effects[i].TargetA.GetTarget() == TARGET_UNIT_TARGET_ALLY) //... and is single target
                        _healcount++;

                    if (_healcount == 5) // "Your Guardian of Ancient Spirits heals the target of your next 5 heals for the same amount as your heal"
                        me->DespawnOrUnsummon();
                }
            }
        }
        private:
            uint32 _healcount;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_guardian_of_ancient_kingsAI(creature);
    }
};*/

class npc_ring_of_frost : public CreatureScript
{
public:
    npc_ring_of_frost() : CreatureScript("npc_ring_of_frost") { }

    struct npc_ring_of_frostAI : public ScriptedAI
    {
        npc_ring_of_frostAI(Creature *creature) : ScriptedAI(creature) {}
        bool Isready;
        uint32 timer;

        void Reset()
        {
            timer = 3000; // 3sec
            Isready = false;
        }

        void InitializeAI()
        {
            ScriptedAI::InitializeAI();
            Unit* owner = me->GetOwner();
            if (!owner || owner->GetTypeId() != TYPEID_PLAYER)
                return;

            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

            // Remove other ring spawned by the player
            std::list<Creature*> templist;
            float x, y, z;
            me->GetPosition(x, y, z);
            {
                CellCoord pair(SkyFire::ComputeCellCoord(x, y));
                Cell cell(pair);
                cell.SetNoCreate();

                SkyFire::AllFriendlyCreaturesInGrid check(me);
                SkyFire::CreatureListSearcher<SkyFire::AllFriendlyCreaturesInGrid> searcher(me, templist, check);

                TypeContainerVisitor<SkyFire::CreatureListSearcher<SkyFire::AllFriendlyCreaturesInGrid>, GridTypeMapContainer> cSearcher(searcher);

                cell.Visit(pair, cSearcher, *(me->GetMap()), *me, me->GetGridActivationRange());

                if (!templist.empty())
                    for (std::list<Creature*>::const_iterator itr = templist.begin(); itr != templist.end(); ++itr)
                        if((*itr)->GetEntry() == me->GetEntry() && ((*itr)->GetOwner() == me->GetOwner() && *itr != me))
                            (*itr)->DisappearAndDie();
                templist.clear();
            }
        }

        void EnterEvadeMode() { return; }

        void CheckIfMoveInRing(Unit* who)
        {
            if (who->isAlive() && me->IsInRange(who, 2.0f, 4.7f) && !who->HasAura(82691)/*<= target already frozen*/ && !Isready)
                me->CastSpell(who, 82691, true);
        }

        void UpdateAI(const uint32 diff)
        {
            if (timer <= diff)
            {
                if (!Isready)
                {
                    Isready = true;
                    timer = 9000; // 9sec
                }
                else
                    me->DisappearAndDie();
            }
            else
                timer -= diff;

            // Find all the enemies
            std::list<Unit*> targets;
            SkyFire::AnyUnfriendlyUnitInObjectRangeCheck u_check(me, me, 5.0f);
            SkyFire::UnitListSearcher<SkyFire::AnyUnfriendlyUnitInObjectRangeCheck> searcher(me, targets, u_check);
            me->VisitNearbyObject(5.0f, searcher);
            for (std::list<Unit*>::const_iterator iter = targets.begin(); iter != targets.end(); ++iter)
                CheckIfMoveInRing(*iter);
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_ring_of_frostAI(creature);
    }
};

// npc_flame_orb
enum FlameOrb
{
    SPELL_FLAME_ORB_DAMAGE          = 86719,
    FLAME_ORB_DISTANCE              = 120
};

class npc_flame_orb : public CreatureScript
{
public:
    npc_flame_orb() : CreatureScript("npc_flame_orb") {}

    struct npc_flame_orbAI : public ScriptedAI
    {
        npc_flame_orbAI(Creature *creature) : ScriptedAI(creature)
        {
            x = me->GetPositionX();
            y = me->GetPositionY();
            z = me->GetOwner()->GetPositionZ()+2;
            o = me->GetOrientation();
            me->NearTeleportTo(x, y, z, o, true);
            angle = me->GetOwner()->GetAngle(me);
            newx = me->GetPositionX() + FLAME_ORB_DISTANCE/2 * cos(angle);
            newy = me->GetPositionY() + FLAME_ORB_DISTANCE/2 * sin(angle);
            CombatCheck = false;
        }

        float x, y, z, o, newx, newy, angle;
        bool CombatCheck;
        uint32 DespawnTimer;
        uint32 DespawnCheckTimer;
        uint32 DamageTimer;

        void EnterCombat(Unit* /*target*/)
        {
            me->GetMotionMaster()->MoveCharge(newx, newy, z, 1.14286f);  // Normal speed
            DespawnTimer = 15 * IN_MILLISECONDS;
            CombatCheck = true;
        }

        void Reset()
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE|UNIT_FLAG_NON_ATTACKABLE);
            me->AddUnitMovementFlag(MOVEMENTFLAG_FLYING);
            me->SetReactState(REACT_PASSIVE);
            if (CombatCheck == true)
                DespawnTimer = 15 * IN_MILLISECONDS;
            else
                DespawnTimer = 4 * IN_MILLISECONDS;
            DamageTimer = 1 * IN_MILLISECONDS;
            me->GetMotionMaster()->MovePoint(0, newx, newy, z);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!me->isInCombat() && CombatCheck == false)
            {
                me->SetSpeed(MOVE_RUN, 2, true);
                me->SetSpeed(MOVE_FLIGHT, 2, true);
            }

            if (DespawnTimer <= diff)
            {
                me->SetVisible(false);
                me->DisappearAndDie();
            }
            else
                DespawnTimer -= diff;

            if (DamageTimer <= diff)
            {
                if (Unit* target = me->SelectNearestTarget(20))
                    DoCast(target, SPELL_FLAME_ORB_DAMAGE);

                DamageTimer = 1 * IN_MILLISECONDS;
            }
            else
                DamageTimer -= diff;
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_flame_orbAI(creature);
    }
};

// npc_frostfire_orb
enum FrostfireOrb
{
    SPELL_FROSTFIRE_ORB_DAMAGE_RANK_1   = 95969,
    SPELL_FROSTFIRE_ORB_DAMAGE_RANK_2   = 84721,
    FROSTFIRE_ORB_DISTANCE              = 120
};

class npc_frostfire_orb : public CreatureScript
{
public:
    npc_frostfire_orb() : CreatureScript("npc_frostfire_orb") {}

    struct npc_frostfire_orbAI : public ScriptedAI
    {
        npc_frostfire_orbAI(Creature* creature) : ScriptedAI(creature)
        {
            x = me->GetPositionX();
            y = me->GetPositionY();
            z = me->GetOwner()->GetPositionZ()+2;
            o = me->GetOrientation();
            me->NearTeleportTo(x, y, z, o, true);
            angle = me->GetOwner()->GetAngle(me);
            newx = me->GetPositionX() + FROSTFIRE_ORB_DISTANCE/2 * cos(angle);
            newy = me->GetPositionY() + FROSTFIRE_ORB_DISTANCE/2 * sin(angle);
            CombatCheck = false;
        }

        float x,y,z,o,newx,newy,angle;
        bool CombatCheck;
        uint32 despawnTimer;
        uint32 despawnCheckTimer;
        uint32 damageTimer;

        void EnterCombat(Unit* /*target*/)
        {
            me->GetMotionMaster()->MoveCharge(newx, newy, z, 1.14286f); // Normal speed
            despawnTimer = 15 * IN_MILLISECONDS;
            CombatCheck = true;
        }

        void Reset()
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
            me->AddUnitMovementFlag(MOVEMENTFLAG_FLYING);
            me->SetReactState(REACT_PASSIVE);
            if (CombatCheck == true)
                despawnTimer = 15 * IN_MILLISECONDS;
            else
                despawnTimer = 4 * IN_MILLISECONDS;
            damageTimer = 1 * IN_MILLISECONDS;
            me->GetMotionMaster()->MovePoint(0, newx, newy, z);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!me->isInCombat() && CombatCheck == false)
            {
                me->SetSpeed(MOVE_RUN, 2, true);
                me->SetSpeed(MOVE_FLIGHT, 2, true);
            }

            if (despawnTimer <= diff)
                me->DisappearAndDie();
            else
                despawnTimer -= diff;

            if (damageTimer <= diff)
            {
                if (Unit* target = me->SelectNearestTarget(20))
                    if (me->GetOwner()->HasAura(84726))
                        DoCast(target, SPELL_FROSTFIRE_ORB_DAMAGE_RANK_1);
                    else
                        DoCast(target, SPELL_FROSTFIRE_ORB_DAMAGE_RANK_2);

                damageTimer = 1 * IN_MILLISECONDS;
            }
            else
                damageTimer -= diff;
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_frostfire_orbAI(creature);
    }
};

// Power Word Barrier
class npc_power_word_barrier : public CreatureScript
{
    public:
    npc_power_word_barrier() : CreatureScript("npc_power_word_barrier") { }

    struct npc_power_word_barrierAI : public ScriptedAI
    {
        npc_power_word_barrierAI(Creature *creature) : ScriptedAI(creature) {}

        bool checker;
        uint32 cron; // Duration

        void Reset()
        {
            checker = false;
            cron = 10000;
            DoCast(me, 81781);
        }

        void InitializeAI()
        {
            ScriptedAI::InitializeAI();
            Unit* owner = me->GetOwner();
            if (!owner || owner->GetTypeId() != TYPEID_PLAYER)
                return;

            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        }

        void BarrierChecker(Unit* who)
        {
            if (who->isAlive() && !who->HasAura(81782))
            {
                me->CastSpell(who, 81782, true);
            }

            if (who->isAlive() && who->HasAura(81782))
            {
                if (AuraEffect const* aur = who->GetAuraEffect(81782, 0))
                    aur->GetBase()->SetDuration(aur->GetSpellInfo()->GetMaxDuration(), true);
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (cron <= diff)
            {
                if (!checker)
                {
                    checker = true;
                    cron = 10000;   //10 seconds
                }
                else
                    me->DisappearAndDie();
            }
            else
                cron -= diff;

           //Check friendly entities
           std::list<Unit*> targets;
            SkyFire::AnyFriendlyUnitInObjectRangeCheck u_check(me, me, 7.0f);
            SkyFire::UnitListSearcher<SkyFire::AnyFriendlyUnitInObjectRangeCheck>searcher(me, targets, u_check);

            me->VisitNearbyObject(7.0f, searcher);
            for (std::list<Unit*>::const_iterator iter = targets.begin(); iter != targets.end(); ++iter)
                BarrierChecker(*iter);
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_power_word_barrierAI(creature);
    }
};

class npc_shadowy_apparition: public CreatureScript
{
public:
    npc_shadowy_apparition () : CreatureScript("npc_shadowy_apparition")
    {
    }

    struct npc_shadowy_apparitionAI: public ScriptedAI
    {
        npc_shadowy_apparitionAI (Creature* c) : ScriptedAI(c) {}

        uint64 targetGuid;

        void Reset ()
        {
            Unit* owner = me->GetOwner();
            if (!owner)
                return;
            
            me->SetWalk(true);
            owner->CastSpell(me, 87213, false);
            me->CastSpell(me, 87427, true);

            if (me->GetCharmInfo())
            {
                me->GetCharmInfo()->SetIsAtStay(true);
                me->GetCharmInfo()->SetIsFollowing(false);
                me->GetCharmInfo()->SetIsReturning(false);
            }
        }

        void MoveInLineOfSight (Unit* who)
        {
            if (who->GetGUID() == targetGuid && me->GetDistance(who) <= 1.0f)
            {
                me->CastCustomSpell(who, 87532, NULL, NULL, NULL, true, 0, 0, me->GetOwnerGUID());
                me->CastSpell(me, 87529, true);
                me->DisappearAndDie();
            }
        }

        void UpdateAI (const uint32 /*diff*/)
        {
            if (!UpdateVictim())
            {
                Unit* owner = me->GetOwner();

                if (!owner)
                    return;

                if (Unit* target = owner->getAttackerForHelper())
                {
                    me->Attack(target, false);
                    me->AddThreat(target, 100.0f);
                    me->GetMotionMaster()->MoveChase(target, 0.0f, 0.0f);
                    targetGuid = target->GetGUID();
                }
            }
        }
    };

    CreatureAI* GetAI (Creature* creature) const
    {
        return new npc_shadowy_apparitionAI(creature);
    }
};

void AddSC_npcs_special()
{
	new npc_gnoll_sh();
    new npc_air_force_bots;
    new npc_lunaclaw_spirit;
    new npc_chicken_cluck;
    new npc_dancing_flames;
    new npc_doctor;
    new npc_injured_patient;
    new npc_garments_of_quests;
    new npc_guardian;
    new npc_kingdom_of_dalaran_quests;
    new npc_mount_vendor;
    new npc_rogue_trainer;
    new npc_sayge;
    new npc_steam_tonk;
    new npc_tonk_mine;
    new npc_winter_reveler;
    new npc_brewfest_reveler;
    new npc_snake_trap;
    new npc_mirror_image;
    new npc_ebon_gargoyle;
    new npc_lightwell;
    new mob_mojo;
    new npc_training_dummy;
    new npc_shadowfiend;
    new npc_wormhole;
    new npc_pet_trainer;
    new npc_locksmith;
    new npc_tabard_vendor;
    new npc_experience;
    new npc_firework;
    // new npc_guardian_of_ancient_kings;
    new npc_flame_orb;
    new npc_ring_of_frost;
    new npc_frostfire_orb;
    new npc_power_word_barrier;
    new npc_shadowy_apparition;
}
