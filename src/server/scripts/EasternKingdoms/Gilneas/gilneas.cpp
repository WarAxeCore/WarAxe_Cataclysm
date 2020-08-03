/*
 * Copyright (C) 2011-2019 Project SkyFire <http://www.projectskyfire.org/>
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

 /* Script Data Start
SFName: gilneas
SFAuthor:
SF%Complete: 80
SFComment: this is for scripting Gilneas City (map 638).
SFCategory: Zone script
Script Data End */

#include "ScriptPCH.h"
#include "Unit.h"
#include "gilneas.h"
#include "ScriptedEscortAI.h"
#include "Vehicle.h"

// Phase 1
/*######
## npc_prince_liam_greymane_phase1
######*/

class npc_prince_liam_greymane_phase1 : public CreatureScript
{
public:
    npc_prince_liam_greymane_phase1() : CreatureScript("npc_prince_liam_greymane_phase1") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_prince_liam_greymane_phase1AI (creature);
    }

    struct npc_prince_liam_greymane_phase1AI : public ScriptedAI
    {
        npc_prince_liam_greymane_phase1AI(Creature* creature) : ScriptedAI(creature) {}

        uint32 tSay; // Time left for say
        uint32 cSay; // Current Say

        // Evade or Respawn
        void Reset()
        {
            tSay = DELAY_SAY_PRINCE_LIAM_GREYMANE; // Reset timer
            cSay = 1; // Start from 1
        }

        // Timed events
        void UpdateAI(const uint32 diff)
        {
            // Out of combat
            if (!me->getVictim())
            {
                // Timed say
                if (tSay <= diff)
                {
                    switch (cSay)
                    {
                        default:
                        case 1:
                            DoScriptText(SAY_PRINCE_LIAM_GREYMANE_1, me);
                            cSay++;
                            break;
                        case 2:
                            DoScriptText(SAY_PRINCE_LIAM_GREYMANE_2, me);
                            cSay++;
                            break;
                        case 3:
                            DoScriptText(SAY_PRINCE_LIAM_GREYMANE_3, me);
                            cSay = 1; //Reset to 1
                            break;
                    }

                    tSay = DELAY_SAY_PRINCE_LIAM_GREYMANE; //Reset the timer
                }
                else
                {
                    tSay -= diff;
                }
                return;
            }
        }
    };
};

/*######
## npc_gilneas_city_guard_phase1
######*/

class npc_gilneas_city_guard_phase1 : public CreatureScript
{
public:
    npc_gilneas_city_guard_phase1() : CreatureScript("npc_gilneas_city_guard_phase1") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_gilneas_city_guard_phase1AI (creature);
    }

    struct npc_gilneas_city_guard_phase1AI : public ScriptedAI
    {
        npc_gilneas_city_guard_phase1AI(Creature* creature) : ScriptedAI(creature) {}

        uint32 tSay; // Time left for say

        // Evade or Respawn
        void Reset()
        {
            if (me->GetGUIDLow() == 3486400)
                tSay = DELAY_SAY_GILNEAS_CITY_GUARD_GATE; // Reset timer
        }

        void UpdateAI(const uint32 diff)
        {
            // Out of combat and
            if (me->GetGUIDLow() == 3486400)
            {
                // Timed say
                if (tSay <= diff)
                {
                    // Random say
                    DoScriptText(RAND(
                        SAY_GILNEAS_CITY_GUARD_GATE_1,
                        SAY_GILNEAS_CITY_GUARD_GATE_2,
                        SAY_GILNEAS_CITY_GUARD_GATE_3),
                    me);

                    tSay = DELAY_SAY_GILNEAS_CITY_GUARD_GATE; // Reset timer
                }
                else
                {
                    tSay -= diff;
                }
            }
        }
    };
};

// Phase 2
/*######
## npc_gilneas_city_guard_phase2
######*/

class npc_gilneas_city_guard_phase2 : public CreatureScript
{
public:
    npc_gilneas_city_guard_phase2() : CreatureScript("npc_gilneas_city_guard_phase2") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_gilneas_city_guard_phase2AI (creature);
    }

    struct npc_gilneas_city_guard_phase2AI : public ScriptedAI
    {
        npc_gilneas_city_guard_phase2AI(Creature* creature) : ScriptedAI(creature) {}

        uint32 tSeek;

        void Reset()
        {
            tSeek      = urand(1000, 2000);
        }

        void DamageTaken(Unit* who, uint32& damage)
        {
            if (who->GetTypeId() == TYPEID_PLAYER)
            {
                me->getThreatManager().resetAllAggro();
                who->AddThreat(me, 1.0f);
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
            }
            else if (who->isPet())
            {
                me->getThreatManager().resetAllAggro();
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
            }
            else if (who->GetEntry() == NPC_RAMPAGING_WORGEN_1 && me->HealthBelowPct(AI_MIN_HP))
                damage = 0;
        }

        void UpdateAI(const uint32 diff)
        {
            if (tSeek <= diff)
            {
                if ((me->isAlive()) && (!me->isInCombat() && (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) <= 1.0f)))
                    if (Creature* enemy = me->FindNearestCreature(NPC_RAMPAGING_WORGEN_1, 16.0f, true))
                        me->AI()->AttackStart(enemy);
                tSeek = urand(1000, 2000); // optimize cpu load, seeking only sometime between 1 and 2 seconds
            }
            else tSeek -= diff;

            if (!UpdateVictim())
                return;
            else
                DoMeleeAttackIfReady();
        }
    };
};

/*######
## npc_prince_liam_greymane_phase2
######*/

class npc_prince_liam_greymane_phase2 : public CreatureScript
{
public:
    npc_prince_liam_greymane_phase2() : CreatureScript("npc_prince_liam_greymane_phase2") {}

    bool OnQuestReward(Player* player, Creature* creature, Quest const* quest, uint32 opt)
    {
        if (quest->GetQuestId() == QUEST_SOMETHINGS_AMISS || quest->GetQuestId() == QUEST_ALL_HELL_BREAKS_LOOSE || quest->GetQuestId() == QUEST_EVAC_MERC_SQUA)
        {
            if (creature->isQuestGiver())
            {
                player->TalkedToCreature(creature->GetEntry(), creature->GetGUID());
                player->PrepareGossipMenu(creature, 0 ,true);
                player->SendPreparedGossip(creature);
            }
        }
        return true;
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_prince_liam_greymane_phase2AI (creature);
    }

    struct npc_prince_liam_greymane_phase2AI : public ScriptedAI
    {
        npc_prince_liam_greymane_phase2AI(Creature* creature) : ScriptedAI(creature) {}

        uint32 tYell, tSeek;
        bool doYell;

        void Reset()
        {
            tSeek     = urand(1000, 2000);
            doYell    = true;
            tYell     = DELAY_YELL_PRINCE_LIAM_GREYMANE;
        }

        // There is NO phase shift here!!!!
        void DamageTaken(Unit* who, uint32& damage)
        {
            if (who->GetTypeId() == TYPEID_PLAYER)
            {
                me->getThreatManager().resetAllAggro();
                who->AddThreat(me, 1.0f);
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
            }
            else if (who->isPet())
            {
                me->getThreatManager().resetAllAggro();
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
            }
            else if (who->GetEntry() == NPC_RAMPAGING_WORGEN_1 && me->HealthBelowPct(AI_MIN_HP))
                damage = 0;
        }

        void UpdateAI(const uint32 diff)
        {
            // If creature has no target
            if (!UpdateVictim())
            {
                if (tSeek <= diff)
                {
                    // Find worgen nearby
                    if (me->isAlive() && !me->isInCombat() && (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) <= 1.0f))
                        if (Creature* enemy = me->FindNearestCreature(NPC_RAMPAGING_WORGEN_1, 16.0f, true))
                            me->AI()->AttackStart(enemy);
                    tSeek = urand(1000, 2000);//optimize cpu load
                }
                else tSeek -= diff;

                // Yell only once after Reset()
                if (doYell)
                {
                    // Yell Timer
                    if (tYell <= diff)
                    {
                        // Random yell
                        DoScriptText(RAND(
                            YELL_PRINCE_LIAM_GREYMANE_1,
                            YELL_PRINCE_LIAM_GREYMANE_2,
                            YELL_PRINCE_LIAM_GREYMANE_3,
                            YELL_PRINCE_LIAM_GREYMANE_4,
                            YELL_PRINCE_LIAM_GREYMANE_5),
                        me);

                        doYell = false;
                    }
                    else
                        tYell -= diff;
                }
            }
            else
            {
                DoMeleeAttackIfReady(); // Stop yell timer on combat
                doYell = false;
            }
        }
    };
};

/*######
## npc_gwen_armstead_p2
######*/

class npc_gwen_armstead_p2 : public CreatureScript
{
public:
    npc_gwen_armstead_p2() : CreatureScript("npc_gwen_armstead_p2") {}

    bool OnQuestReward(Player* player, Creature* creature, Quest const* quest, uint32 opt)
    {
        if (quest->GetQuestId() == QUEST_ROYAL_ORDERS)
        {
            if (creature->isQuestGiver())
            {
                player->TalkedToCreature(creature->GetEntry(), creature->GetGUID());
                player->PrepareGossipMenu(creature, 0 ,true);
                player->SendPreparedGossip(creature);
            }
        }
        return true;
    }
};

/*######
## npc_rampaging_worgen
######*/

class npc_rampaging_worgen : public CreatureScript
{
public:
    npc_rampaging_worgen() : CreatureScript("npc_rampaging_worgen") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_rampaging_worgenAI (creature);
    }

    struct npc_rampaging_worgenAI : public ScriptedAI
    {
        npc_rampaging_worgenAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 tEnrage;
        bool willCastEnrage;

        void Reset()
        {
            tEnrage        = 0;
            willCastEnrage = urand(0, 1);
        }

        void DamageTaken(Unit* who, uint32& damage)
        {
            if (who->GetTypeId() == TYPEID_PLAYER)
            {
                me->getThreatManager().resetAllAggro();
                who->AddThreat(me, 1.0f);
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
            }
            else if (who->isPet())
            {
                me->getThreatManager().resetAllAggro();
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
            }
            else if (me->HealthBelowPct(AI_MIN_HP) && who->GetEntry() == NPC_GILNEAS_CITY_GUARD || who->GetEntry() == NPC_PRINCE_LIAM_GREYMANE)
            {
                me->AI()->AttackStart(who);
                if (me->GetHealthPct() <= AI_MIN_HP)
                damage = 0;
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;
            else
            {
                DoMeleeAttackIfReady();

                if (tEnrage <= diff && willCastEnrage)
                {
                    if (me->GetHealthPct() <= 30)
                    {
                        me->MonsterTextEmote(-106, 0);
                        DoCast(me, SPELL_ENRAGE);
                        tEnrage = CD_ENRAGE;
                    }
                }
                else tEnrage -= diff;
            }
        }
    };
};

/*######
## npc_rampaging_worgen2
######*/

class npc_rampaging_worgen2 : public CreatureScript
{
public:
    npc_rampaging_worgen2() : CreatureScript("npc_rampaging_worgen2") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_rampaging_worgen2AI (creature);
    }

    struct npc_rampaging_worgen2AI : public ScriptedAI
    {
        npc_rampaging_worgen2AI(Creature* creature) : ScriptedAI(creature) {}

        uint16 tRun, tEnrage;
        bool onceRun, willCastEnrage;
        float x, y, z;

        void JustRespawned()
        {
            tEnrage = 0;
            tRun = 500;
            onceRun = true;
            x = me->m_positionX + cos(me->_orientation)*8;
            y = me->m_positionY + sin(me->_orientation)*8;
            z = me->m_positionZ;
            willCastEnrage = urand(0, 1);
        }

        void UpdateAI(const uint32 diff)
        {
            if (tRun <= diff && onceRun)
            {
                me->GetMotionMaster()->MoveCharge(x, y, z, 8);
                onceRun = false;
            }
            else
                tRun -= diff;

            if (!UpdateVictim())
                return;

            if (tEnrage <= diff)
            {
                if (me->GetHealthPct() <= 30 && willCastEnrage)
                {
                    me->MonsterTextEmote(-106, 0);
                    DoCast(me, SPELL_ENRAGE);
                    tEnrage = CD_ENRAGE;
                }
            }
            else
                tEnrage -= diff;

            DoMeleeAttackIfReady();
        }
    };
};

/*######
## go_merchant_square_door
######*/

class go_merchant_square_door : public GameObjectScript
{
public:
    go_merchant_square_door() : GameObjectScript("go_merchant_square_door"), aPlayer(NULL) {}

    float x, y, z, wx, wy, angle, tQuestCredit;
    bool opened;
    uint8 spawnKind;
    Player* aPlayer;
    GameObject* go;
    uint32 DoorTimer;

    bool OnGossipHello(Player* player, GameObject* go)
    {
        if (player->GetQuestStatus(QUEST_EVAC_MERC_SQUA) == QUEST_STATUS_INCOMPLETE && go->GetGoState() == GO_STATE_READY)
        {
            aPlayer          = player;
            opened           = 1;
            tQuestCredit     = 2500;
            go->SetGoState(GO_STATE_ACTIVE);
            DoorTimer = DOOR_TIMER;
            spawnKind = urand(1, 3); // 1, 2=citizen, 3=citizen&worgen (66%, 33%)
            angle = go->GetOrientation();
            x = go->GetPositionX()-cos(angle)*2;
            y = go->GetPositionY()-sin(angle)*2;
            z = go->GetPositionZ();
            wx = x-cos(angle)*2;
            wy = y-sin(angle)*2;

            if (spawnKind < 3)
            {
                if (Creature* spawnedCreature = go->SummonCreature(NPC_FRIGHTENED_CITIZEN_1, x, y, z, angle, TEMPSUMMON_TIMED_DESPAWN, SUMMON1_TTL))
                {
                    spawnedCreature->SetPhaseMask(6, true);
                    spawnedCreature->Respawn(true);
                }
            }
            else
            {
                if (Creature* spawnedCreature = go->SummonCreature(NPC_FRIGHTENED_CITIZEN_2, x, y, z, angle, TEMPSUMMON_TIMED_DESPAWN, SUMMON1_TTL))
                {
                    spawnedCreature->SetPhaseMask(6, true);
                    spawnedCreature->Respawn(true);
                }
            }
            return true;
        }
        return false;
    }

    void OnUpdate(GameObject* go, uint32 diff)
    {
        if (opened == 1)
        {
            if (tQuestCredit <= ((float)diff/8))
            {
                opened = 0;

                if(aPlayer)
                    aPlayer->KilledMonsterCredit(35830, 0);

                if (spawnKind == 3)
                {
                    if (Creature* spawnedCreature = go->SummonCreature(NPC_RAMPAGING_WORGEN_2, wx, wy, z, angle, TEMPSUMMON_TIMED_DESPAWN, SUMMON1_TTL))
                    {
                        spawnedCreature->SetPhaseMask(6, 1);
                        spawnedCreature->Respawn(1);
                        spawnedCreature->getThreatManager().resetAllAggro();
                        if(aPlayer)
                            aPlayer->AddThreat(spawnedCreature, 1.0f);
                        spawnedCreature->AddThreat(aPlayer, 1.0f);
                    }
                }
            }
            else tQuestCredit -= ((float)diff/8);
        }
        if (DoorTimer <= diff)
            {
                if (go->GetGoState() == GO_STATE_ACTIVE)
                    go->SetGoState(GO_STATE_READY);

                DoorTimer = DOOR_TIMER;
            }
        else
            DoorTimer -= diff;
    }
};

/*######
## npc_frightened_citizen
######*/

struct Point
{
    float x, y;
};

struct WayPointID
{
    int pathID, pointID;
};

struct Paths
{
    uint8 pointsCount[8]; // pathID, pointsCount
    Point paths[8][10];   // pathID, pointID, Point
};

class npc_frightened_citizen : public CreatureScript
{
public:
    npc_frightened_citizen() : CreatureScript("npc_frightened_citizen") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_frightened_citizenAI (creature);
    }

    struct npc_frightened_citizenAI : public ScriptedAI
    {
        npc_frightened_citizenAI(Creature* creature) : ScriptedAI(creature) {}

        uint16 tRun, tRun2, tSay;
        bool onceRun, onceRun2, onceGet, onceSay;
        float x, y, z;
        WayPointID nearestPointID;
        Paths paths;

        Paths LoadPaths()
        {
            QueryResult result[PATHS_COUNT];
            result[0] = WorldDatabase.Query("SELECT `id`, `point`, `position_x`, `position_y` FROM waypoint_data WHERE id = 349810 ORDER BY `point`");
            result[1] = WorldDatabase.Query("SELECT `id`, `point`, `position_x`, `position_y` FROM waypoint_data WHERE id = 349811 ORDER BY `point`");
            if (result[0]) paths.pointsCount[0] = result[0]->GetRowCount();
            else
            {
                sLog->outError("waypoint_data for frightened citizen missing");
                return paths;
            }
            if (result[1]) paths.pointsCount[1] = result[1]->GetRowCount();
            else
            {
                sLog->outError("waypoint_data for frightened citizen missing");
                return paths;
            }
            uint8 j;
            for (uint8 i = 0; i < PATHS_COUNT; i ++)
            {
                j = 0;
                do
                {
                    Field* Fields = result[i]->Fetch();
                    paths.paths[i][j].x = Fields[2].GetFloat();
                    paths.paths[i][j].y = Fields[3].GetFloat();
                    j++;
                }
                while (result[i]->NextRow());
            }
            return paths;
        }

        void MultiDistanceMeter(Point *p, uint8 pointsCount, float *dist)
        {
            for (uint8 i = 0; i <= (pointsCount-1); i++)
                dist[i] = me->GetDistance2d(p[i].x, p[i].y);
        }

        WayPointID GetNearestPoint(Paths paths)
        {
            WayPointID nearestPointID;
            float dist[PATHS_COUNT][10], lowestDists[PATHS_COUNT];
            uint8 nearestPointsID[PATHS_COUNT], lowestDist;
            for (uint8 i = 0; i <= PATHS_COUNT-1; i++)
            {
                MultiDistanceMeter(paths.paths[i], paths.pointsCount[i], dist[i]);
                for (uint8 j = 0; j <= paths.pointsCount[i]-1; j++)
                {
                    if (j == 0)
                    {
                        lowestDists[i] = dist[i][j];
                        nearestPointsID[i] = j;
                    }
                    else if (lowestDists[i] > dist[i][j])
                    {
                        lowestDists[i] = dist[i][j];
                        nearestPointsID[i] = j;
                    }
                }
            }
            for (uint8 i = 0; i < PATHS_COUNT; i++)
            {
                if (i == 0)
                {
                    lowestDist = lowestDists[i];
                    nearestPointID.pointID = nearestPointsID[i];
                    nearestPointID.pathID = i;
                }
                else if (lowestDist > lowestDists[i])
                {
                    lowestDist = lowestDists[i];
                    nearestPointID.pointID = nearestPointsID[i];
                    nearestPointID.pathID = i;
                }
            }
            return nearestPointID;
        }

        void JustRespawned()
        {
            paths          = LoadPaths();
            tRun           = 500;
            tRun2          = 2500;
            tSay           = 1000;
            onceRun = onceRun2 = onceSay = onceGet = true;
            x = me->m_positionX + cos(me->_orientation)*5;
            y = me->m_positionY + sin(me->_orientation)*5;
            z = me->m_positionZ;
        }

        void UpdateAI(const uint32 diff)
        {
            if (tRun <= diff && onceRun)
            {
                me->GetMotionMaster()->MoveCharge(x, y, z, 8);
                onceRun = false;
            }
            else
                tRun -= diff;

            if (tSay <= diff && onceSay)
            {
                if (me->GetEntry() == NPC_FRIGHTENED_CITIZEN_1)
                    DoScriptText(RAND(SAY_CITIZEN_1, SAY_CITIZEN_2, SAY_CITIZEN_3, SAY_CITIZEN_4, SAY_CITIZEN_5, SAY_CITIZEN_6, SAY_CITIZEN_7, SAY_CITIZEN_8), me);
                else
                    DoScriptText(RAND(SAY_CITIZEN_1b, SAY_CITIZEN_2b, SAY_CITIZEN_3b, SAY_CITIZEN_4b, SAY_CITIZEN_5b), me);
                onceSay = false;
            }
            else tSay -= diff;

            if (tRun2 <= diff)
            {
                if (onceGet)
                {
                    nearestPointID = GetNearestPoint(paths);
                    onceGet = false;
                }
                else
                {
                    if (me->GetDistance2d(paths.paths[nearestPointID.pathID][nearestPointID.pointID].x, paths.paths[nearestPointID.pathID][nearestPointID.pointID].y) > 1)
                        me->GetMotionMaster()->MoveCharge(paths.paths[nearestPointID.pathID][nearestPointID.pointID].x, paths.paths[nearestPointID.pathID][nearestPointID.pointID].y, z, 8);
                    else
                        nearestPointID.pointID ++;
                    if (nearestPointID.pointID >= paths.pointsCount[nearestPointID.pathID]) me->DespawnOrUnsummon();
                }
            }
            else tRun2 -= diff;
        }
    };
};

// Phase 4
/*######
## npc_bloodfang_worgen
######*/

class npc_bloodfang_worgen : public CreatureScript
{
public:
    npc_bloodfang_worgen() : CreatureScript("npc_bloodfang_worgen") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_bloodfang_worgenAI (creature);
    }

    struct npc_bloodfang_worgenAI : public ScriptedAI
    {
        npc_bloodfang_worgenAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 tEnrage, tSeek;
        bool willCastEnrage;

        void Reset()
        {
            tEnrage           = 0;
            willCastEnrage    = urand(0, 1);
            tSeek             = 100; // On initial loading, we should find our target rather quickly
        }

        void DamageTaken(Unit* who, uint32& damage)
        {
            if (who->GetTypeId() == TYPEID_PLAYER)
            {
                me->getThreatManager().resetAllAggro();
                who->AddThreat(me, 1.0f);
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
				who->ToPlayer()->KilledMonsterCredit(44175, NULL);
            }
            else if (who->isPet())
            {
                me->getThreatManager().resetAllAggro();
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
            }
            else if (me->HealthBelowPct(AI_MIN_HP) && who->GetEntry() == NPC_GILNEAN_ROYAL_GUARD || who->GetEntry() == NPC_SERGEANT_CLEESE || who->GetEntry() == NPC_MYRIAM_SPELLWALKER)
                damage = 0;
        }

        void UpdateAI(const uint32 diff)
        {
            if (tSeek <= diff)
            {
                if ((me->isAlive()) && (!me->isInCombat() && (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) <= 1.0f)))
                    if (Creature* enemy = me->FindNearestCreature(NPC_SERGEANT_CLEESE || NPC_GILNEAN_ROYAL_GUARD, 10.0f, true))
                        me->AI()->AttackStart(enemy);
                tSeek = urand(1000, 2000); //optimize cpu load, seeking only sometime between 1 and 2 seconds
            }
            else tSeek -= diff;

            if (!UpdateVictim())
                return;

            if (tEnrage <= diff && willCastEnrage && me->GetHealthPct() <= 30)
            {
                me->MonsterTextEmote(-106, 0);
                DoCast(me, SPELL_ENRAGE);
                tEnrage = CD_ENRAGE;
            }
            else
                tEnrage -= diff;

            DoMeleeAttackIfReady();
        }
    };
};

/*######
## npc_sergeant_cleese
######*/

class npc_sergeant_cleese : public CreatureScript
{
public:
    npc_sergeant_cleese() : CreatureScript("npc_sergeant_cleese") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_sergeant_cleeseAI (creature);
    }

    struct npc_sergeant_cleeseAI : public ScriptedAI
    {
        npc_sergeant_cleeseAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 tSeek;

        void Reset()
        {
            tSeek      = urand(1000, 2000);
        }

        void DamageTaken(Unit* who, uint32& damage)
        {
            if (who->GetTypeId() == TYPEID_PLAYER)
            {
                me->getThreatManager().resetAllAggro();
                who->AddThreat(me, 1.0f);
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
            }
            else if (who->isPet())
            {
                me->getThreatManager().resetAllAggro();
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
            }
            else if (me->HealthBelowPct(AI_MIN_HP) && who->GetEntry() == NPC_BLOODFANG_WORGEN)
                damage = 0;
        }

        void UpdateAI(const uint32 diff)
        {
            if (tSeek <= diff)
            {
                if ((me->isAlive()) && (!me->isInCombat() && (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) <= 1.0f)))
                    if (Creature* enemy = me->FindNearestCreature(NPC_BLOODFANG_WORGEN, 10.0f, true))
                        me->AI()->AttackStart(enemy);
                tSeek = urand(1000, 2000); // optimize cpu load, seeking only sometime between 1 and 2 seconds
            }
            else tSeek -= diff;

            if (!UpdateVictim())
                return;
            else
                DoMeleeAttackIfReady();
        }
    };
};

/*######
## npc_gilnean_royal_guard
######*/

class npc_gilnean_royal_guard : public CreatureScript
{
public:
    npc_gilnean_royal_guard() : CreatureScript("npc_gilnean_royal_guard") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_gilnean_royal_guardAI (creature);
    }

    struct npc_gilnean_royal_guardAI : public ScriptedAI
    {
        npc_gilnean_royal_guardAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 tSeek;

        void Reset()
        {
            tSeek      = urand(1000, 2000);
        }

        void DamageTaken(Unit* who, uint32& damage)
        {
            if (who->GetTypeId() == TYPEID_PLAYER)
            {
                me->getThreatManager().resetAllAggro();
                who->AddThreat(me, 1.0f);
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
            }
            else if (who->isPet())
            {
                me->getThreatManager().resetAllAggro();
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
            }
            else if (me->HealthBelowPct(AI_MIN_HP) && who->GetEntry() == NPC_BLOODFANG_WORGEN)
                damage = 0;
        }

        void UpdateAI(const uint32 diff)
        {
            if (tSeek <= diff)
            {
                if ((me->isAlive()) && (!me->isInCombat() && (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) <= 1.0f)))
                    if (Creature* enemy = me->FindNearestCreature(NPC_BLOODFANG_WORGEN, 16.0f, true))
                        me->AI()->AttackStart(enemy);
                tSeek = urand(1000, 2000); // optimize cpu load, seeking only sometime between 1 and 2 seconds
            }
            else tSeek -= diff;

            if (!UpdateVictim())
                return;
            else
                DoMeleeAttackIfReady();
        }
    };
};

/*######
## npc_mariam_spellwalker
######*/

class npc_mariam_spellwalker : public CreatureScript
{
public:
    npc_mariam_spellwalker() : CreatureScript("npc_mariam_spellwalker") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_mariam_spellwalkerAI (creature);
    }

    struct npc_mariam_spellwalkerAI : public ScriptedAI
    {
        npc_mariam_spellwalkerAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 tSeek;

        void Reset()
        {
            tSeek = urand(1000, 2000);
        }

        void DamageTaken(Unit* who, uint32& damage)
        {
            if (me->HealthBelowPct(AI_MIN_HP) && who->GetEntry() == NPC_BLOODFANG_WORGEN)
                damage = 0;
        }

        void UpdateAI(const uint32 diff)
        {
            if (tSeek <= diff)
            {
                if ((me->isAlive()) && (!me->isInCombat() && (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) <= 1.0f)))
                    if (Creature* enemy = me->FindNearestCreature(NPC_BLOODFANG_WORGEN, 5.0f, true))
                        me->AI()->AttackStart(enemy); // She should really only grab agro when npc Cleese is not there, so we will keep this range small
                tSeek = urand(1000, 2000); // optimize cpu load, seeking only sometime between 1 and 2 seconds
            }
            else tSeek -= diff;

            if (!UpdateVictim())
                return;

            if (me->getVictim()->GetEntry() == NPC_BLOODFANG_WORGEN)
                DoSpellAttackIfReady(SPELL_FROSTBOLT_VISUAL_ONLY); //Dummy spell, visual only to prevent getting agro (Blizz-like)
            else
                DoMeleeAttackIfReady();
        }
    };
};

/* QUEST - 14154 - By The Skin of His Teeth - START */

/*######
## npc_sean_dempsey
######*/

class npc_sean_dempsey : public CreatureScript
{
public:
    npc_sean_dempsey() : CreatureScript("npc_sean_dempsey") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_sean_dempseyAI (creature);
    }

    struct npc_sean_dempseyAI : public ScriptedAI
    {
        npc_sean_dempseyAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 tSummon, tEvent_Timer, tWave_Time;
        bool EventActive, RunOnce;
        Player* player;

        void Reset()
        {
            EventActive      = false;
            RunOnce          = true;
            tSummon          = 0;
            tEvent_Timer     = 0;
            tWave_Time       = urand(9000, 15000); // How often we spawn
        }

        void SummonNextWave()
        {
            if (!EventActive)
                return;
            else
            {
                if (RunOnce) // Our inital spawn should always be the same
                {
                    me->SummonCreature(NPC_WORGEN_ALPHA_C2, SW_ROOF_SPAWN_LOC_1, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, WORGEN_EVENT_SPAWNTIME);
                    me->SummonCreature(NPC_WORGEN_ALPHA_C1, NW_ROOF_SPAWN_LOC_1, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, WORGEN_EVENT_SPAWNTIME);
                    RunOnce = false;
                }
                else
                {
                    switch (urand (1,5)) // After intial wave, wave spawns should be random
                    {
                        case 1: // One Alpha on SW Roof and One Alpha on NW Roof
                            me->SummonCreature(NPC_WORGEN_ALPHA_C2, SW_ROOF_SPAWN_LOC_1, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, WORGEN_EVENT_SPAWNTIME);
                            me->SummonCreature(NPC_WORGEN_ALPHA_C1, NW_ROOF_SPAWN_LOC_1, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, WORGEN_EVENT_SPAWNTIME);
                            break;

                        case 2: // 8 Runts on NW Roof
                            for (int i = 0; i < 5; i++)
                                me->SummonCreature(NPC_WORGEN_RUNT_C1, NW_ROOF_SPAWN_LOC_1, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, WORGEN_EVENT_SPAWNTIME);
                                me->SummonCreature(NPC_WORGEN_RUNT_C1, NW_ROOF_SPAWN_LOC_2, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, WORGEN_EVENT_SPAWNTIME);
                            break;

                        case 3: // 8 Runts on SW Roof
                            for (int i = 0; i < 5; i++)
                                me->SummonCreature(NPC_WORGEN_RUNT_C2, SW_ROOF_SPAWN_LOC_1, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, WORGEN_EVENT_SPAWNTIME);
                                me->SummonCreature(NPC_WORGEN_RUNT_C2, SW_ROOF_SPAWN_LOC_2, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, WORGEN_EVENT_SPAWNTIME);
                            break;

                        case 4: // One Alpha on SW Roof and One Alpha on N Roof
                            me->SummonCreature(NPC_WORGEN_ALPHA_C2, SW_ROOF_SPAWN_LOC_1, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, WORGEN_EVENT_SPAWNTIME);
                            me->SummonCreature(NPC_WORGEN_ALPHA_C1, N_ROOF_SPAWN_LOC, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, WORGEN_EVENT_SPAWNTIME);
                            break;
                        case 5: // 8 Runts - Half NW and Half SW
                            for (int i = 0; i < 5; i++)
                                me->SummonCreature(NPC_WORGEN_RUNT_C2, SW_ROOF_SPAWN_LOC_1, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, WORGEN_EVENT_SPAWNTIME);
                                me->SummonCreature(NPC_WORGEN_RUNT_C1, NW_ROOF_SPAWN_LOC_2, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, WORGEN_EVENT_SPAWNTIME);
                            break;
                    }
                }
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (!EventActive)
                return;
            else
            {
                if (tEvent_Timer <= diff)
                {
                    EventActive = false;
                    tEvent_Timer = false;
                    return;
                }
                else // Event is still active
                {
                    tEvent_Timer -= diff;
                    if (tSummon <= diff) // Time for next spawn wave
                    {
                        SummonNextWave(); // Activate next spawn wave
                        tSummon = tWave_Time; // Reset our spawn timer
                    }
                    else
                        tSummon -= diff;
                }
            }
        }
    };
};

/*######
## npc_lord_darius_crowley_c1
######*/

class npc_lord_darius_crowley_c1 : public CreatureScript
{
public:
    npc_lord_darius_crowley_c1() : CreatureScript("npc_lord_darius_crowley_c1") {}

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == QUEST_BY_THE_SKIN_ON_HIS_TEETH)
        {
            creature->CastSpell(player, SPELL_BY_THE_SKIN_ON_HIS_TEETH, true);
            if (Creature* dempsey = GetClosestCreatureWithEntry(creature, NPC_SEAN_DEMPSEY, 100.0f))
            {
                CAST_AI(npc_sean_dempsey::npc_sean_dempseyAI, dempsey->AI())->EventActive = true; // Start Event
                CAST_AI(npc_sean_dempsey::npc_sean_dempseyAI, dempsey->AI())->tEvent_Timer = Event_Time; // Event lasts for 2 minutes - We'll stop spawning a few seconds short (Blizz-like)
            }
        }
        return true;
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_lord_darius_crowley_c1AI (creature);
    }

    struct npc_lord_darius_crowley_c1AI : public ScriptedAI
    {
        npc_lord_darius_crowley_c1AI(Creature* creature) : ScriptedAI(creature) {}

        uint32 tAttack;

        void Reset()
        {
            tAttack = urand(1700, 2400);
        }

        void UpdateAI(const uint32 diff)
        {
        if (!UpdateVictim())
            {
                // Reset home if no target
                me->GetMotionMaster()->MoveCharge(me->GetHomePosition().GetPositionX(),me->GetHomePosition().GetPositionY(),me->GetHomePosition().GetPositionZ(),8.0f);
                me->SetOrientation(me->GetHomePosition().GetOrientation()); // Reset to my original orientation
                return;
            }

            if (tAttack <= diff) // If we have a target, and it is time for our attack
            {
                if (me->IsWithinMeleeRange(me->getVictim()))
                {
                    switch (urand(0, 2)) // Perform one of 3 random attacks
                    {
                        case 0: // Do Left Hook
                            if (me->GetOrientation() > 2.0f && me->GetOrientation() < 3.0f || me->GetOrientation() > 5.0f && me->GetOrientation() < 6.0f)
                                // If Orientation is outside of these ranges, there is a possibility the knockback could knock worgens off the platform
                                // After which, Crowley would chase
                            {
                                DoCast(me->getVictim(), SPELL_LEFT_HOOK, true);
                            }
                                tAttack = urand(1700, 2400);
                            break;

                        case 1: // Do Demoralizing Shout
                            DoCast(me->getVictim(), SPELL_DEMORALIZING_SHOUT, true);
                            tAttack = urand(1700, 2400);
                            break;

                        case 2: // Do Snap Kick
                            DoCast(me->getVictim(), SPELL_SNAP_KICK, true);
                            tAttack = urand(1700, 2400);
                            break;
                    }
                }
                else
                    me->GetMotionMaster()->MoveChase(me->getVictim());
            }
            else // If we have a target but our attack timer is still not ready, do regular attack
            {
                tAttack -= diff;
                DoMeleeAttackIfReady();
            }
        }
    };
};

/*######
## npc_worgen_runt_c1
######*/

class npc_worgen_runt_c1 : public CreatureScript
{
public:
    npc_worgen_runt_c1() : CreatureScript("npc_worgen_runt_c1") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_worgen_runt_c1AI (creature);
    }

    struct npc_worgen_runt_c1AI : public ScriptedAI
    {
        npc_worgen_runt_c1AI(Creature* creature) : ScriptedAI(creature) {}

        uint32 WaypointId, willCastEnrage, tEnrage, CommonWPCount;
        bool Run, Loc1, Loc2, Jump, Combat;

        void Reset()
        {
            Run = Loc1 = Loc2 = Combat= Jump = false;
            WaypointId          = 0;
            tEnrage             = 0;
            willCastEnrage      = urand(0, 1);
        }

        void UpdateAI(const uint32 diff)
        {
            if (me->GetPositionX() == -1611.40f && me->GetPositionY() == 1498.49f) // I was spawned in location 1
            {
                Run = true; // Start running across roof
                Loc1 = true;
            }
            else if (me->GetPositionX() == -1618.86f && me->GetPositionY() == 1505.68f) // I was spawned in location 2
            {
                Run = true; // Start running across roof
                Loc2 = true;
            }

            if (Run && !Jump && !Combat)
            {
                if (Loc1) // If I was spawned in Location 1
                {
                    if (WaypointId < 2)
                        me->GetMotionMaster()->MovePoint(WaypointId,NW_WAYPOINT_LOC1[WaypointId].X, NW_WAYPOINT_LOC1[WaypointId].Y, NW_WAYPOINT_LOC1[WaypointId].Z);
                }
                else if (Loc2)// If I was spawned in Location 2
                {
                    if (WaypointId < 2)
                        me->GetMotionMaster()->MovePoint(WaypointId,NW_WAYPOINT_LOC2[WaypointId].X, NW_WAYPOINT_LOC2[WaypointId].Y, NW_WAYPOINT_LOC2[WaypointId].Z);
                }
            }

            if (!Run && Jump && !Combat) // After Jump
            {
                if (me->GetPositionZ() == PLATFORM_Z) // Check that we made it to the platform
                {
                    me->GetMotionMaster()->Clear(); // Stop Movement
                    // Set our new home position so we don't try and run back to the rooftop on reset
                    me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
                    Combat = true; // Start Combat
                    Jump = false; // We have already Jumped
                }
            }

            if (Combat && !Run && !Jump) // Our Combat AI
            {
                if (Player* player = me->SelectNearestPlayer(40.0f)) // Try to attack nearest player 1st (Blizz-Like)
                    AttackStart(player);
                else
                    AttackStart(me->FindNearestCreature(NPC_LORD_DARIUS_CROWLEY_C1, 40.0f)); // Attack Darius 2nd - After that, doesn't matter

                if (!UpdateVictim())
                    return;

                if (tEnrage <= diff) // Our Enrage trigger
                {
                    if (me->GetHealthPct() <= 30 && willCastEnrage)
                    {
                        me->MonsterTextEmote(-106, 0);
                        DoCast(me, SPELL_ENRAGE);
                        tEnrage = CD_ENRAGE;
                    }
                }
                else
                    tEnrage -= diff;

                DoMeleeAttackIfReady();
            }
        }

        void MovementInform(uint32 Type, uint32 PointId)
        {
            if (Type != POINT_MOTION_TYPE)
                return;

            if (Loc1)
            {
                CommonWPCount = sizeof(NW_WAYPOINT_LOC1)/sizeof(Waypoint); // Count our waypoints
            }
            else if (Loc2)
            {
                CommonWPCount = sizeof(NW_WAYPOINT_LOC2)/sizeof(Waypoint); // Count our waypoints
            }

            WaypointId = PointId+1; // Increase to next waypoint

            if (WaypointId >= CommonWPCount) // If we have reached the last waypoint
            {
                if (Loc1)
                {
                    me->GetMotionMaster()->MoveJump(-1668.52f + irand(-3, 3), 1439.69f + irand(-3, 3), PLATFORM_Z, 20.0f, 22.0f);
                    Loc1 = false;
                }
                else if (Loc2)
                {
                    me->GetMotionMaster()->MoveJump(-1678.04f + irand(-3, 3), 1450.88f + irand(-3, 3), PLATFORM_Z, 20.0f, 22.0f);
                    Loc2 = false;
                }

                Run = false; // Stop running - Regardless of spawn location
                Jump = true; // Time to Jump - Regardless of spawn location
            }
        }
    };
};

/*######
## npc_worgen_runt_c2
######*/

class npc_worgen_runt_c2 : public CreatureScript
{
public:
    npc_worgen_runt_c2() : CreatureScript("npc_worgen_runt_c2") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_worgen_runt_c2AI (creature);
    }

    struct npc_worgen_runt_c2AI : public ScriptedAI
    {
        npc_worgen_runt_c2AI(Creature* creature) : ScriptedAI(creature) {}

        uint32 WaypointId, willCastEnrage, tEnrage, CommonWPCount;
        bool Run, Loc1, Loc2, Jump, Combat;

        void Reset()
        {
            Run = Loc1 = Loc2 = Combat= Jump = false;
            WaypointId          = 0;
            tEnrage             = 0;
            willCastEnrage      = urand(0, 1);
        }

        void UpdateAI(const uint32 diff)
        {
            if (me->GetPositionX() == -1732.81f && me->GetPositionY() == 1526.34f) // I was spawned in location 1
            {
                Run = true; // Start running across roof
                Loc1 = true;
            }
            else if (me->GetPositionX() == -1737.49f && me->GetPositionY() == 1526.11f) // I was spawned in location 2
            {
                Run = true; // Start running across roof
                Loc2 = true;
            }

            if (Run && !Jump && !Combat)
            {
                if (Loc1) // If I was spawned in Location 1
                {
                    if (WaypointId < 2)
                        me->GetMotionMaster()->MovePoint(WaypointId,SW_WAYPOINT_LOC1[WaypointId].X, SW_WAYPOINT_LOC1[WaypointId].Y, SW_WAYPOINT_LOC1[WaypointId].Z);
                }
                else if (Loc2)// If I was spawned in Location 2
                {
                    if (WaypointId < 2)
                        me->GetMotionMaster()->MovePoint(WaypointId,SW_WAYPOINT_LOC2[WaypointId].X, SW_WAYPOINT_LOC2[WaypointId].Y, SW_WAYPOINT_LOC2[WaypointId].Z);
                }
            }

            if (!Run && Jump && !Combat) // After Jump
            {
                if (me->GetPositionZ() == PLATFORM_Z) // Check that we made it to the platform
                {
                    me->GetMotionMaster()->Clear(); // Stop Movement
                    // Set our new home position so we don't try and run back to the rooftop on reset
                    me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
                    Combat = true; // Start Combat
                    Jump = false; // We have already Jumped
                }
            }

            if (Combat && !Run && !Jump) // Our Combat AI
            {
                if (Player* player = me->SelectNearestPlayer(50.0f)) // Try to attack nearest player 1st (Blizz-Like)
                    AttackStart(player);
                else
                    AttackStart(me->FindNearestCreature(NPC_LORD_DARIUS_CROWLEY_C1, 50.0f)); // Attack Darius 2nd - After that, doesn't matter

                if (!UpdateVictim())
                    return;

                if (tEnrage <= diff) // Our Enrage trigger
                {
                    if (me->GetHealthPct() <= 30 && willCastEnrage)
                    {
                        me->MonsterTextEmote(-106, 0);
                        DoCast(me, SPELL_ENRAGE);
                        tEnrage = CD_ENRAGE;
                    }
                }
                else
                    tEnrage -= diff;

                DoMeleeAttackIfReady();
            }
        }

        void MovementInform(uint32 Type, uint32 PointId)
        {
            if (Type != POINT_MOTION_TYPE)
                return;

            if (Loc1)
            {
                CommonWPCount = sizeof(SW_WAYPOINT_LOC1)/sizeof(Waypoint); // Count our waypoints
            }
            else if (Loc2)
            {
                CommonWPCount = sizeof(SW_WAYPOINT_LOC2)/sizeof(Waypoint); // Count our waypoints
            }

            WaypointId = PointId+1; // Increase to next waypoint

            if (WaypointId >= CommonWPCount) // If we have reached the last waypoint
            {
                if (Loc1)
                {
                    me->GetMotionMaster()->MoveJump(-1685.521f + irand(-3, 3), 1458.48f + irand(-3, 3), PLATFORM_Z, 20.0f, 22.0f);
                    Loc1 = false;
                }
                else if (Loc2)
                {
                    me->GetMotionMaster()->MoveJump(-1681.81f + irand(-3, 3), 1445.54f + irand(-3, 3), PLATFORM_Z, 20.0f, 22.0f);
                    Loc2 = false;
                }

                Run = false; // Stop running - Regardless of spawn location
                Jump = true; // Time to Jump - Regardless of spawn location
            }
        }
    };
};

/*######
## npc_worgen_alpha_c1
######*/

class npc_worgen_alpha_c1 : public CreatureScript
{
public:
    npc_worgen_alpha_c1() : CreatureScript("npc_worgen_alpha_c1") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_worgen_alpha_c1AI (creature);
    }

    struct npc_worgen_alpha_c1AI : public ScriptedAI
    {
        npc_worgen_alpha_c1AI(Creature* creature) : ScriptedAI(creature) {}

        uint32 WaypointId, willCastEnrage, tEnrage, CommonWPCount;
        bool Run, Loc1, Loc2, Jump, Combat;

        void Reset()
        {
            Run = Loc1 = Loc2 = Combat= Jump = false;
            WaypointId          = 0;
            tEnrage             = 0;
            willCastEnrage      = urand(0, 1);
        }

        void UpdateAI(const uint32 diff)
        {
            if (me->GetPositionX() == -1618.86f && me->GetPositionY() == 1505.68f) // I was spawned in location 1 on NW Rooftop
            {
                Run = true; // Start running across roof
                Loc1 = true;
            }
            else if (me->GetPositionX() == -1562.59f && me->GetPositionY() == 1409.35f) // I was spawned on the North Rooftop
            {
                Run = true; // Start running across roof
                Loc2 = true;
            }

            if (Run && !Jump && !Combat)
            {
                if (Loc1) // If I was spawned in Location 1
                {
                    if (WaypointId < 2)
                        me->GetMotionMaster()->MovePoint(WaypointId,NW_WAYPOINT_LOC1[WaypointId].X, NW_WAYPOINT_LOC1[WaypointId].Y, NW_WAYPOINT_LOC1[WaypointId].Z);
                }
                else if (Loc2)// If I was spawned in Location 2
                {
                    if (WaypointId < 2)
                        me->GetMotionMaster()->MovePoint(WaypointId,N_WAYPOINT_LOC[WaypointId].X, N_WAYPOINT_LOC[WaypointId].Y, N_WAYPOINT_LOC[WaypointId].Z);
                }
            }

            if (!Run && Jump && !Combat) // After Jump
            {
                if (me->GetPositionZ() == PLATFORM_Z) // Check that we made it to the platform
                {
                    me->GetMotionMaster()->Clear(); // Stop Movement
                    // Set our new home position so we don't try and run back to the rooftop on reset
                    me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
                    Combat = true; // Start Combat
                    Jump = false; // We have already Jumped
                }
            }

            if (Combat && !Run && !Jump) // Our Combat AI
            {
                if (Player* player = me->SelectNearestPlayer(40.0f)) // Try to attack nearest player 1st (Blizz-Like)
                    AttackStart(player);
                else
                    AttackStart(me->FindNearestCreature(NPC_LORD_DARIUS_CROWLEY_C1, 40.0f)); // Attack Darius 2nd - After that, doesn't matter

                if (!UpdateVictim())
                    return;

                if (tEnrage <= diff) // Our Enrage trigger
                {
                    if (me->GetHealthPct() <= 30 && willCastEnrage)
                    {
                        me->MonsterTextEmote(-106, 0);
                        DoCast(me, SPELL_ENRAGE);
                        tEnrage = CD_ENRAGE;
                    }
                }
                else
                    tEnrage -= diff;

                DoMeleeAttackIfReady();
            }
        }

        void MovementInform(uint32 Type, uint32 PointId)
        {
            if (Type != POINT_MOTION_TYPE)
                return;

            if (Loc1)
            {
                CommonWPCount = sizeof(NW_WAYPOINT_LOC1)/sizeof(Waypoint); // Count our waypoints
            }
            else if (Loc2)
            {
                CommonWPCount = sizeof(N_WAYPOINT_LOC)/sizeof(Waypoint); // Count our waypoints
            }

            WaypointId = PointId+1; // Increase to next waypoint

            if (WaypointId >= CommonWPCount) // If we have reached the last waypoint
            {
                if (Loc1)
                {
                    me->GetMotionMaster()->MoveJump(-1668.52f + irand(-3, 3), 1439.69f + irand(-3, 3), PLATFORM_Z, 20.0f, 22.0f);
                    Loc1 = false;
                }
                else if (Loc2)
                {
                    me->GetMotionMaster()->MoveJump(-1660.17f + irand(-3, 3), 1429.55f + irand(-3, 3), PLATFORM_Z, 22.0f, 20.0f);
                    Loc2 = false;
                }

                Run = false; // Stop running - Regardless of spawn location
                Jump = true; // Time to Jump - Regardless of spawn location
            }
        }
    };
};

/*######
## npc_worgen_alpha_c2
######*/

class npc_worgen_alpha_c2 : public CreatureScript
{
public:
    npc_worgen_alpha_c2() : CreatureScript("npc_worgen_alpha_c2") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_worgen_alpha_c2AI (creature);
    }

    struct npc_worgen_alpha_c2AI : public ScriptedAI
    {
        npc_worgen_alpha_c2AI(Creature* creature) : ScriptedAI(creature) {}

        uint32 WaypointId, willCastEnrage, tEnrage, CommonWPCount;
        bool Run, Jump, Combat;

        void Reset()
        {
            Run = Combat= Jump = false;
            WaypointId          = 0;
            tEnrage             = 0;
            willCastEnrage      = urand(0, 1);
        }

        void UpdateAI(const uint32 diff)
        {
            if (me->GetPositionX() == -1732.81f && me->GetPositionY() == 1526.34f) // I was just spawned
            {
                Run = true; // Start running across roof
            }

            if (Run && !Jump && !Combat)
            {
                if (WaypointId < 2)
                    me->GetMotionMaster()->MovePoint(WaypointId,SW_WAYPOINT_LOC1[WaypointId].X, SW_WAYPOINT_LOC1[WaypointId].Y, SW_WAYPOINT_LOC1[WaypointId].Z);
            }

            if (!Run && Jump && !Combat) // After Jump
            {
                if (me->GetPositionZ() == PLATFORM_Z) // Check that we made it to the platform
                {
                    me->GetMotionMaster()->Clear(); // Stop Movement
                    // Set our new home position so we don't try and run back to the rooftop on reset
                    me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
                    Combat = true; // Start Combat
                    Jump = false; // We have already Jumped
                }
            }

            if (Combat && !Run && !Jump) // Our Combat AI
            {
                if (Player* player = me->SelectNearestPlayer(40.0f)) // Try to attack nearest player 1st (Blizz-Like)
                    AttackStart(player);
                else
                    AttackStart(me->FindNearestCreature(NPC_LORD_DARIUS_CROWLEY_C1, 40.0f)); // Attack Darius 2nd - After that, doesn't matter

                if (!UpdateVictim())
                    return;

                if (tEnrage <= diff) // Our Enrage trigger
                {
                    if (me->GetHealthPct() <= 30 && willCastEnrage)
                    {
                        me->MonsterTextEmote(-106, 0);
                        DoCast(me, SPELL_ENRAGE);
                        tEnrage = CD_ENRAGE;
                    }
                }
                else
                    tEnrage -= diff;

                DoMeleeAttackIfReady();
            }
        }

        void MovementInform(uint32 Type, uint32 PointId)
        {
            if (Type != POINT_MOTION_TYPE)
                return;

            CommonWPCount = sizeof(SW_WAYPOINT_LOC1)/sizeof(Waypoint); // Count our waypoints

            WaypointId = PointId+1; // Increase to next waypoint

            if (WaypointId >= CommonWPCount) // If we have reached the last waypoint
            {
                me->GetMotionMaster()->MoveJump(-1685.52f + irand(-3, 3), 1458.48f + irand(-3, 3), PLATFORM_Z, 20.0f, 22.0f);
                Run = false; // Stop running
                Jump = true; // Time to Jump
            }
        }
    };
};

/* QUEST - 14154 - By The Skin of His Teeth - END */

/* QUEST - 14159 - The Rebel Lord's Arsenal - START */
// Also Phase shift from Phase 2 to Phase 4

/*######
## npc_josiah_avery_trigger
######*/

class npc_josiah_avery_trigger : public CreatureScript
{
public:
    npc_josiah_avery_trigger() : CreatureScript("npc_josiah_avery_trigger") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_josiah_avery_triggerAI (creature);
    }

    struct npc_josiah_avery_triggerAI : public ScriptedAI
    {
        npc_josiah_avery_triggerAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 Phase, tEvent;
        uint64 PlayerGUID;

        void Reset()
        {
            Phase       = 0;
            PlayerGUID  = 0;
            tEvent      = 0;
        }

        void UpdateAI(const uint32 diff)
        {
            if (Creature* Lorna = me->FindNearestCreature(NPC_LORNA_CROWLEY_P4, 60.0f, true))
            if (Creature* BadAvery = me->FindNearestCreature(NPC_JOSIAH_AVERY_P4, 80.0f, true))
            if (Player* player = me->SelectNearestPlayer(50.0f))
            {
                if (!player->HasAura(SPELL_WORGEN_BITE))
                    return;
                else
                    PlayerGUID = player->GetGUID();
                    if (tEvent <= diff)
                    {
                        switch (Phase)
                        {
                            case (0):
                            {
                                me->AI()->Talk(SAY_JOSAIH_AVERY_TRIGGER, PlayerGUID); // Tell Player they have been bitten
                                tEvent = 200;
                                Phase++;
                                break;
                            }

                            case (1):
                            {
                                BadAvery->SetOrientation(BadAvery->GetAngle(player)); // Face Player
                                BadAvery->CastSpell(player, 69873, true); // Do Cosmetic Attack
                                player->GetMotionMaster()->MoveKnockTo(-1791.94f, 1427.29f, 12.4584f, 22.0f, 8.0f, PlayerGUID);
                                BadAvery->getThreatManager().resetAllAggro();
                                tEvent = 1200;
                                Phase++;
                                break;
                            }

                            case (2):
                            {
                                BadAvery->GetMotionMaster()->MoveJump(-1791.94f, 1427.29f, 12.4584f, 18.0f, 7.0f);
                                tEvent = 600;
                                Phase++;
                                break;
                            }

                            case (3):
                            {
                                Lorna->CastSpell(BadAvery, SPELL_SHOOT, true);
                                tEvent = 200;
                                Phase++;
                                break;
                            }

                            case (4):
                            {
                                BadAvery->CastSpell(BadAvery, SPELL_GET_SHOT, true);
                                BadAvery->setDeathState(JUST_DIED);
                                player->SaveToDB();
                                BadAvery->DespawnOrUnsummon(1000);
                                me->DespawnOrUnsummon(1000);
                                tEvent = 5000;
                                Phase++;
                                break;
                            }
                        }
                    }
                    else tEvent -= diff;
            }
        }
    };
};

/*######
## npc_josiah_avery_p2
######*/

class npc_josiah_avery_p2 : public CreatureScript
{
public:
    npc_josiah_avery_p2() : CreatureScript("npc_josiah_avery_p2") {}

    bool OnQuestReward(Player* player, Creature* creature, Quest const* quest, uint32 opt)
    {
        if (quest->GetQuestId() == QUEST_THE_REBEL_LORDS_ARSENAL)
        {
            creature->AddAura(SPELL_WORGEN_BITE, player);
            player->RemoveAura(SPELL_PHASE_QUEST_2);
            creature->SetPhaseMask(4, 1);
            creature->CastSpell(creature, SPELL_SUMMON_JOSIAH_AVERY);
            creature->SetPhaseMask(2, 1);
        }
        return true;
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_josiah_avery_p2AI (creature);
    }

    struct npc_josiah_avery_p2AI : public ScriptedAI
    {
        npc_josiah_avery_p2AI(Creature* creature) : ScriptedAI(creature) {}

        uint32 tSay, tPlayerCheck;
        uint64 PlayerGUID;
        bool Talk;

        void Reset()
        {
            tSay        = urand(2000, 4000);
            PlayerGUID  = 0;
            tPlayerCheck= 500;
            Talk        = false;
        }

        void UpdateAI(const uint32 diff)
        {
            if (tPlayerCheck <= diff)
            {
                if (Player* player = me->SelectNearestPlayer(10.0f)) // We should only talk when player is close
                {
                    Talk = true;
                }
                else Talk = false;
            }
            else tPlayerCheck -= diff;

            if (Talk && tSay <= diff)
            {
                me->AI()->Talk(SAY_JOSIAH_AVERY_P2);
                tSay = urand(2000, 4000);
            }
            else tSay -= diff;
        }
    };
};

/* QUEST - 14159 - The Rebel Lord's Arsenal - END */

/* QUEST - 14204 - FROM THE SHADOWS - START */

/*######
## npc_lorna_crowley_p4
######*/

class npc_lorna_crowley_p4 : public CreatureScript
{
public:
    npc_lorna_crowley_p4() : CreatureScript("npc_lorna_crowley_p4") {}

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == QUEST_FROM_THE_SHADOWS)
        {
            player->CastSpell(player, SPELL_SUMMON_GILNEAN_MASTIFF);
            creature->AI()->Talk(SAY_LORNA_CROWLEY_P4);
        }
        return true;
    }
};

/*######
# npc_gilnean_mastiff
######*/

class npc_gilnean_mastiff : public CreatureScript
{
public:
    npc_gilnean_mastiff() : CreatureScript("npc_gilnean_mastiff") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_gilnean_mastiffAI(creature);
    }

    struct npc_gilnean_mastiffAI : public ScriptedAI
    {
        npc_gilnean_mastiffAI(Creature* creature) : ScriptedAI(creature) {}

        void Reset()
        {
            me->GetCharmInfo()->InitEmptyActionBar(false);
            me->GetCharmInfo()->SetActionBar(0, SPELL_ATTACK_LURKER, ACT_PASSIVE);
            me->SetReactState(REACT_DEFENSIVE);
            me->GetCharmInfo()->SetIsFollowing(true);
        }

        void UpdateAI(uint32 const /*diff*/)
        {
            Player* player = me->GetOwner()->ToPlayer();

            if (player->GetQuestStatus(QUEST_FROM_THE_SHADOWS) == QUEST_STATUS_REWARDED)
            {
                me->DespawnOrUnsummon(1);
            }

            if (!UpdateVictim())
            {
                me->GetCharmInfo()->SetIsFollowing(true);
                me->SetReactState(REACT_DEFENSIVE);
                return;
            }

            DoMeleeAttackIfReady();
        }

        void SpellHitTarget(Unit* Mastiff, SpellInfo const* cSpell)
        {
            if (cSpell->Id == SPELL_ATTACK_LURKER)
            {
                Mastiff->RemoveAura(SPELL_SHADOWSTALKER_STEALTH);
                Mastiff->AddThreat(me, 1.0f);
                me->AddThreat(Mastiff, 1.0f);
                me->AI()->AttackStart(Mastiff);
            }
        }

        void JustDied(Unit* /*killer*/) // Otherwise, player is stuck with pet corpse they cannot remove from world
        {
            me->DespawnOrUnsummon(1);
        }

        void KilledUnit(Unit* /*victim*/)
        {
            Reset();
        }
    };
};

/*######
## npc_bloodfang_lurker
######*/

class npc_bloodfang_lurker : public CreatureScript
{
public:
    npc_bloodfang_lurker() : CreatureScript("npc_bloodfang_lurker") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_bloodfang_lurkerAI (creature);
    }

    struct npc_bloodfang_lurkerAI : public ScriptedAI
    {
        npc_bloodfang_lurkerAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 tEnrage, tSeek;
        bool willCastEnrage;

        void Reset()
        {
            tEnrage           = 0;
            willCastEnrage    = urand(0, 1);
            tSeek             = urand(5000, 10000);
            DoCast(me, SPELL_SHADOWSTALKER_STEALTH);
        }

        void UpdateAI(const uint32 diff)
        {
            if (tSeek <= diff)
            {
                if ((me->isAlive()) && (!me->isInCombat() && (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) <= 2.0f)))
                    if (Player* player = me->SelectNearestPlayer(2.0f))
                    {
                        if (!player->isInCombat())
                        {
                            me->AI()->AttackStart(player);
                            tSeek = urand(5000, 10000);
                        }
                    }
            }
            else tSeek -= diff;

            if (!UpdateVictim())
                return;

            if (tEnrage <= diff && willCastEnrage && me->GetHealthPct() <= 30)
            {
                me->MonsterTextEmote(-106, 0);
                DoCast(me, SPELL_ENRAGE);
                tEnrage = CD_ENRAGE;
            }
            else
                tEnrage -= diff;

            DoMeleeAttackIfReady();
        }
    };
};

/* QUEST - 14204 - FROM THE SHADOWS - END */

/*######
## npc_king_genn_greymane_p4
######*/

class npc_king_genn_greymane_p4 : public CreatureScript
{
public:
    npc_king_genn_greymane_p4() : CreatureScript("npc_king_genn_greymane_p4") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_king_genn_greymane_p4AI (creature);
    }

    struct npc_king_genn_greymane_p4AI : public ScriptedAI
    {
        npc_king_genn_greymane_p4AI(Creature* creature) : ScriptedAI(creature) {}

        uint32 tSummon, tSay;
        bool EventActive, RunOnce;

        void Reset()
        {
            tSay            = urand(10000, 20000);
            tSummon         = urand(3000, 5000); // How often we spawn
        }

        void SummonNextWave()
        {
            switch (urand (1,4))
            {
                case (1):
                    for (int i = 0; i < 5; i++)
                        me->SummonCreature(NPC_BLOODFANG_RIPPER_P4, -1781.173f + irand(-15, 15), 1372.90f + irand(-15, 15), 19.7803f, urand(0, 6), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 15000);
                    break;
                case (2):
                    for (int i = 0; i < 5; i++)
                        me->SummonCreature(NPC_BLOODFANG_RIPPER_P4, -1756.30f + irand(-15, 15), 1380.61f + irand(-15, 15), 19.7652f, urand(0, 6), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 15000);
                    break;
                case (3):
                    for (int i = 0; i < 5; i++)
                        me->SummonCreature(NPC_BLOODFANG_RIPPER_P4, -1739.84f + irand(-15, 15), 1384.87f + irand(-15, 15), 19.841f, urand(0, 6), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 15000);
                    break;
                case (4):
                    for (int i = 0; i < 5; i++)
                        me->SummonCreature(NPC_BLOODFANG_RIPPER_P4, -1781.173f + irand(-15, 15), 1372.90f + irand(-15, 15), 19.7803f, urand(0, 6), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 15000);
                    break;
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (tSay <= diff) // Time for next spawn wave
            {
                Talk(SAY_KING_GENN_GREYMANE_P4);
                tSay = urand(10000, 20000);
            }
            else tSay -= diff;

            if (tSummon <= diff) // Time for next spawn wave
            {
                SummonNextWave(); // Activate next spawn wave
                tSummon = urand(3000, 5000); // Reset our spawn timer
            }
            else tSummon -= diff;
        }

        void JustSummoned(Creature* summoned)
        {
            summoned->GetDefaultMovementType();
            summoned->SetReactState(REACT_AGGRESSIVE);
        }
    };
};

/*######
## npc_gilneas_city_guard_p8
######*/

class npc_gilneas_city_guard_p8 : public CreatureScript
{
public:
    npc_gilneas_city_guard_p8() : CreatureScript("npc_gilneas_city_guard_p8") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_gilneas_city_guard_p8AI (creature);
    }

    struct npc_gilneas_city_guard_p8AI : public ScriptedAI
    {
        npc_gilneas_city_guard_p8AI(Creature* creature) : ScriptedAI(creature) {}

        uint32 tYell, SayChance, WillSay;

        void Reset()
        {
            WillSay     = urand(0,100);
            SayChance   = 10;
            tYell       = urand(10000, 20000);
        }

        void DamageTaken(Unit* who, uint32& damage)
        {
            if (who->GetEntry() == NPC_AFFLICTED_GILNEAN_P8 && me->GetHealthPct() <= AI_MIN_HP)
            {
                damage = 0;
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if(tYell <= diff)
            {
                if (WillSay <= SayChance)
                {
                    Talk(SAY_GILNEAS_CITY_GUARD_P8);
                    tYell = urand(10000, 20000);
                }
            }
            else tYell -= diff;

            if (!UpdateVictim())
                return;
            else
                DoMeleeAttackIfReady();
        }
    };
};

/*######
## npc_afflicted_gilnean_p8
######*/

class npc_afflicted_gilnean_p8 : public CreatureScript
{
public:
    npc_afflicted_gilnean_p8() : CreatureScript("npc_afflicted_gilnean_p8") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_afflicted_gilnean_p8AI (creature);
    }

    struct npc_afflicted_gilnean_p8AI : public ScriptedAI
    {
        npc_afflicted_gilnean_p8AI(Creature* creature) : ScriptedAI(creature) {}

        uint32 tEnrage, tSeek;
        bool willCastEnrage;

        void Reset()
        {
            tEnrage           = 0;
            willCastEnrage    = urand(0, 1);
            tSeek             = 100; // On initial loading, we should find our target rather quickly
        }

        void DamageTaken(Unit* who, uint32& damage)
        {
            if (who->GetTypeId() == TYPEID_PLAYER)
            {
                me->getThreatManager().resetAllAggro();
                who->AddThreat(me, 1.0f);
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
            }
            else if (who->isPet())
            {
                me->getThreatManager().resetAllAggro();
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
            }
            else if (who->GetEntry() == NPC_GILNEAS_CITY_GUARD_P8)
            {
                if (damage >= me->GetHealth())
                {
                    damage = 0;
                    me->SetHealth(me->GetMaxHealth() * .85);
                }
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (tSeek <= diff)
            {
                if ((me->isAlive()) && (!me->isInCombat() && (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) <= 1.0f)))
                    if (Creature* enemy = me->FindNearestCreature(NPC_GILNEAS_CITY_GUARD_P8, 5.0f, true))
                        me->AI()->AttackStart(enemy);
                tSeek = urand(1000, 2000);
            }
            else tSeek -= diff;

            if (!UpdateVictim())
                return;

            if (tEnrage <= diff && willCastEnrage && me->GetHealthPct() <= 30)
            {
                me->MonsterTextEmote(-106, 0);
                DoCast(me, SPELL_ENRAGE);
                tEnrage = CD_ENRAGE;
            }
            else
                tEnrage -= diff;

            DoMeleeAttackIfReady();
        }
    };
};

/*######
## npc_king_genn_greymane_c2
######*/

class npc_king_genn_greymane_c2 : public CreatureScript
{
public:
    npc_king_genn_greymane_c2() : CreatureScript("npc_king_genn_greymane_c2") {}

    bool OnQuestComplete(Player* player, Creature* /*creature*/, Quest const* /*quest*/)
    {
        player->RemoveAurasDueToSpell(68630);
        player->RemoveAurasDueToSpell(76642);
		player->RemoveAurasDueToSpell(69196);
        player->CastSpell(player, 68481, true);
        return true;
    }
};

/*######
## npc_greymane_horse
######*/

class npc_greymane_horse : public CreatureScript
{
public:
    npc_greymane_horse() : CreatureScript("npc_greymane_horse") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_greymane_horseAI (creature);
    }

    struct npc_greymane_horseAI : public npc_escortAI
    {
        npc_greymane_horseAI(Creature* creature) : npc_escortAI(creature) {}

        uint32 krennansay;
        bool PlayerOn, KrennanOn;

        void AttackStart(Unit* /*who*/) {}
        void EnterCombat(Unit* /*who*/) {}
        void EnterEvadeMode() {}

        void Reset()
        {
             krennansay     = 500;//Check every 500ms initially
             PlayerOn       = false;
             KrennanOn      = false;
        }

        void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply)
        {
            if (who->GetTypeId() == TYPEID_PLAYER)
            {
                PlayerOn = true;
                if (apply)
                {
                    Start(false, true, who->GetGUID());
                }
            }
            else if (who->GetTypeId() == TYPEID_UNIT)
            {
                KrennanOn = true;
                SetEscortPaused(false);
            }
        }

        void WaypointReached(uint32 i)
        {
            Player* player = GetPlayerForEscort();

            switch(i)
            {
                case 5:
                    Talk(SAY_GREYMANE_HORSE, player->GetGUID());
                    me->GetMotionMaster()->MoveJump(-1679.089f, 1348.42f, 15.31f, 25.0f, 15.0f);
                    if (me->GetVehicleKit()->HasEmptySeat(1))
                    {
                        SetEscortPaused(true);
                        break;
                    }
                    else
                    break;
                case 12:
                    player->ExitVehicle();
                    player->SetClientControl(me, 1);
                    break;
            }
        }

        void JustDied(Unit* /*killer*/)
        {
            if (Player* player = GetPlayerForEscort())
               player->FailQuest(QUEST_SAVE_KRENNAN_ARANAS);
        }

        void OnCharmed(bool /*apply*/)
        {
        }

        void UpdateAI(const uint32 diff)
        {
            npc_escortAI::UpdateAI(diff);
            Player* player = GetPlayerForEscort();

            if (PlayerOn)
            {
                player->SetClientControl(me, 0);
                PlayerOn = false;
            }

            if (KrennanOn) // Do Not yell for help after krennan is on
                return;

            if (krennansay <=diff)
            {
                if (Creature* krennan = me->FindNearestCreature(NPC_KRENNAN_ARANAS_TREE, 70.0f, true))
                {
                    krennan->AI()->Talk(SAY_NPC_KRENNAN_ARANAS_TREE, player->GetGUID());
                    krennansay = urand(4000,7000);//Repeat every 4 to 7 seconds
                }
            }
            else
                krennansay -= diff;
        }
    };
};

/*######
## npc_krennan_aranas_c2
######*/

class npc_krennan_aranas_c2 : public CreatureScript
{
public:
    npc_krennan_aranas_c2() : CreatureScript("npc_krennan_aranas_c2") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_krennan_aranas_c2AI(creature);
    }

    struct npc_krennan_aranas_c2AI : public ScriptedAI
    {
        npc_krennan_aranas_c2AI(Creature* creature) : ScriptedAI(creature) {}

        bool Say, Move, Cast, KrennanDead;
        uint32 SayTimer;

        void AttackStart(Unit* /*who*/) {}
        void EnterCombat(Unit* /*who*/) {}
        void EnterEvadeMode() {}

        void Reset()
        {
            Say             = false;
            Move            = true;
            Cast            = true;
            KrennanDead     = false;
            SayTimer        = 500;
        }

        void UpdateAI(const uint32 diff)
        {
            if (Creature* krennan = me->FindNearestCreature(NPC_KRENNAN_ARANAS_TREE, 50.0f))
            {
                if (!KrennanDead)
                {
                    krennan->ForcedDespawn(0);
                    KrennanDead = true;
                }
            }

            if (Creature* horse = me->FindNearestCreature(NPC_GREYMANE_HORSE_P4, 20.0f))//Jump onto horse in seat 2
            {
                if (Cast)
                {
                    DoCast(horse, 84275, true);
                }

                if (me->HasAura(84275))
                {
                    Cast = false;
                }
            }

            if (!me->HasAura(84275) && Move)
            {
                me->NearTeleportTo(KRENNAN_END_X, KRENNAN_END_Y, KRENNAN_END_Z, KRENNAN_END_O);
                Say = true;
                Move = false;
                SayTimer = 500;
            }

            if (Say && SayTimer <= diff)
            {
                Talk(SAY_KRENNAN_C2);
                me->ForcedDespawn(6000);
                Say = false;
            }
            else
                SayTimer -= diff;
        }
    };
};

/*######
## npc_commandeered_cannon
######*/

class npc_commandeered_cannon : public CreatureScript
{
public:
    npc_commandeered_cannon() : CreatureScript("npc_commandeered_cannon") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_commandeered_cannonAI (creature);
    }

    struct npc_commandeered_cannonAI : public ScriptedAI
    {
        npc_commandeered_cannonAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 tEvent;
        uint8 Count, Phase;
        bool EventStart;

        void Reset()
        {
            tEvent          = 1400;
            Phase           = 0;
            Count           = 0;
            EventStart      = false;
        }

        void UpdateAI(const uint32 diff)
        {
            if (!EventStart)
                return;

            if (Count > 2)
            {
                Reset();
                return;
            }

            if (tEvent <= diff)
            {
                switch (Phase)
                {
                case (0):
                    for (int i = 0; i < 12; i++)
                    {
                        me->SummonCreature(NPC_BLOODFANG_WORGEN, -1757.65f + irand(-6, 6), 1384.01f + irand(-6, 6), 19.872f, urand(0, 6), TEMPSUMMON_TIMED_DESPAWN, 5000);
                    }
                    tEvent = 400;
                    Phase++;
                    break;

                case (1):
                    if (Creature* Worgen = me->FindNearestCreature(NPC_BLOODFANG_WORGEN, 50.0f, true))
                    {
                        me->CastSpell(Worgen, SPELL_CANNON_FIRE, true);
                        tEvent = 1700;
                        Phase = 0;
                        Count++;
                    }
                    break;
                }
            } else tEvent -= diff;
        }

        void JustSummoned(Creature* summon)
        {
            summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
        }
    };
};

/*######
## npc_lord_godfrey_p4_8
######*/

class npc_lord_godfrey_p4_8 : public CreatureScript
{
public:
    npc_lord_godfrey_p4_8() : CreatureScript("npc_lord_godfrey_p4_8") { }

    bool OnQuestReward(Player* player, Creature* godfrey, Quest const* quest, uint32 opt)
    {
        if (quest->GetQuestId() == QUEST_SAVE_KRENNAN_ARANAS)
        {
            godfrey->AI()->Talk(SAY_LORD_GODFREY_P4);
            player->RemoveAurasDueToSpell(SPELL_WORGEN_BITE);
            godfrey->AddAura(SPELL_INFECTED_BITE, player);
            player->CastSpell(player, SPELL_GILNEAS_CANNON_CAMERA);
            player->SaveToDB();
            if (Creature* cannon = GetClosestCreatureWithEntry(godfrey, NPC_COMMANDEERED_CANNON, 50.0f))
            {
                CAST_AI(npc_commandeered_cannon::npc_commandeered_cannonAI, cannon->AI())->EventStart = true; // Start Event
            }
        }
        return true;
    }
};

class spell_keg_placed : public SpellScriptLoader
{
public:
    spell_keg_placed() : SpellScriptLoader("spell_keg_placed") {}

    class spell_keg_placed_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_keg_placed_AuraScript);

        uint32 tick, tickcount;

        void HandleEffectApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            tick = urand(1, 4);
            tickcount = 0;
        }

        void HandlePeriodic(AuraEffect const* /*aurEff*/)
        {
            PreventDefaultAction();
            if (Unit* caster = GetCaster())
            {
                if (tickcount > tick)
                {
                    if (caster->GetTypeId() != TYPEID_PLAYER)
                        return;

                    caster->ToPlayer()->KilledMonsterCredit(36233, 0);
                    if (Unit* target = GetTarget())
                        target->Kill(target);
                }
                tickcount++;
            }
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_keg_placed_AuraScript::HandleEffectApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_keg_placed_AuraScript::HandlePeriodic, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_keg_placed_AuraScript();
    }
};

/*######
## npc_crowley_horse
######*/

class npc_crowley_horse : public CreatureScript
{
public:
    npc_crowley_horse() : CreatureScript("npc_crowley_horse") {}

    struct npc_crowley_horseAI : public npc_escortAI
    {
        npc_crowley_horseAI(Creature* creature) : npc_escortAI(creature) {}

        bool CrowleyOn;
        bool CrowleySpawn;
        bool Run;

        void AttackStart(Unit* /*who*/) {}
        void EnterCombat(Unit* /*who*/) {}
        void EnterEvadeMode() {}

        void Reset()
        {
            CrowleyOn = false;
            CrowleySpawn = false;
            Run = false;
        }

        void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply)
        {
            if (who->GetTypeId() == TYPEID_PLAYER)
            {
                if (apply)
                {
                    Start(false, true, who->GetGUID());
                }
            }
        }

        void WaypointReached(uint32 i)
        {
            Player* player = GetPlayerForEscort();
            Creature* crowley = me->FindNearestCreature(NPC_DARIUS_CROWLEY, 5, true);

            switch(i)
            {
                case 1:
                    player->SetClientControl(me, 0);
                    crowley->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    me->GetMotionMaster()->MoveJump(-1714.02f, 1666.37f, 20.57f, 25.0f, 15.0f);
                    break;
                case 4:
                    crowley->AI()->Talk(SAY_CROWLEY_HORSE_1);
                    break;
                case 10:
                    me->GetMotionMaster()->MoveJump(-1571.32f, 1710.58f, 20.49f, 25.0f, 15.0f);
                    break;
                case 11:
                    crowley->AI()->Talk(SAY_CROWLEY_HORSE_2);
                    break;
                case 16:
                    crowley->AI()->Talk(SAY_CROWLEY_HORSE_2);
                    break;
                case 20:
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    me->getThreatManager().resetAllAggro();
                    player->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    player->getThreatManager().resetAllAggro();
                    break;
                case 21:
                    player->SetClientControl(me, 1);
                    player->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    player->ExitVehicle();
                    break;
            }
        }

        void JustDied(Unit* /*killer*/)
        {
            if (Player* player = GetPlayerForEscort())
               player->FailQuest(QUEST_SACRIFICES);
        }

        void OnCharmed(bool /*apply*/)
        {
        }

        void UpdateAI(const uint32 diff)
        {
            npc_escortAI::UpdateAI(diff);

            if (!CrowleySpawn)
            {
                DoCast(SPELL_SUMMON_CROWLEY);
                if (Creature* crowley = me->FindNearestCreature(NPC_DARIUS_CROWLEY, 5, true))
                {
                    CrowleySpawn = true;
                }
            }

            if (CrowleySpawn && !CrowleyOn)
            {
                Creature* crowley = me->FindNearestCreature(NPC_DARIUS_CROWLEY, 5, true);
                crowley->CastSpell(me, SPELL_RIDE_HORSE, true);//Mount Crowley in seat 1
                CrowleyOn = true;
            }

            if (!Run)
            {
                me->SetSpeed(MOVE_RUN, CROWLEY_SPEED);
                Run = true;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_crowley_horseAI (creature);
    }
};

/*######
## npc_bloodfang_stalker_c1
######*/
class npc_bloodfang_stalker_c1 : public CreatureScript
{
public:
    npc_bloodfang_stalker_c1() : CreatureScript("npc_bloodfang_stalker_c1") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_bloodfang_stalker_c1AI (creature);
    }

    struct npc_bloodfang_stalker_c1AI : public ScriptedAI
    {
        npc_bloodfang_stalker_c1AI(Creature* creature) : ScriptedAI(creature) {}

        Player* player;
        uint32 tEnrage;
        uint32 tAnimate;
        uint32 BurningReset;
        bool Miss, willCastEnrage, Burning;

        void Reset()
        {
            tEnrage    = 0;
            tAnimate   = DELAY_ANIMATE;
            Miss  = false;
            willCastEnrage = urand(0, 1);
            BurningReset = 3000;
            Burning = false;
        }

        void UpdateAI(const uint32 diff)
        {
            if(me->HasAura(SPELL_THROW_TORCH))
            {
                Burning = true;
            }
            else
                Burning = false;

            if (Burning && BurningReset <=diff)//Extra fail-safe in case for some reason the aura fails to automatically remove itself (happened a few times during testing - cause is still unknown at this time)
            {
                me->RemoveAllAuras();
                BurningReset = 5000;
                Burning = false;
            }
            else
                BurningReset -= diff;

            if (!UpdateVictim())
            {
                return;
            }

            if (tEnrage <= diff && willCastEnrage)
            {
                if (me->GetHealthPct() <= 30)
                {
                    me->MonsterTextEmote(-106, 0);
                    DoCast(me, SPELL_ENRAGE);
                    tEnrage = CD_ENRAGE;
                }
            }
            else tEnrage -= diff;

            if (me->getVictim()->GetTypeId() == TYPEID_PLAYER)
            {
                Miss = false;
            }
            else if (me->getVictim()->isPet())
            {
                Miss = false;
            }
            else if (me->getVictim()->GetEntry() == NPC_NORTHGATE_REBEL_1)
            {
                if (me->getVictim()->GetHealthPct() < 90)
                {
                    Miss = true;
                }
            }

            if (Miss && tAnimate <= diff)
            {
                me->HandleEmoteCommand(EMOTE_ONESHOT_ATTACKUNARMED);
                me->PlayDistanceSound(SOUND_WORGEN_ATTACK);
                tAnimate = DELAY_ANIMATE;
            }
            else
                tAnimate -= diff;

            if (!Miss)
            {
                DoMeleeAttackIfReady();
            }
        }

        void SpellHit(Unit* caster, const SpellInfo* spell)
        {
            Creature* horse = me->FindNearestCreature(NPC_CROWLEY_HORSE, 100, true);
            if (spell->Id == SPELL_THROW_TORCH)
            {
                Burning = true;

                if(me->getVictim()->GetTypeId() == TYPEID_PLAYER)//We should ONLY switch our victim if we currently have the player targeted
                {
                    me->getThreatManager().resetAllAggro();//We need to aggro on crowley's horse, not the player
                    horse->AddThreat(me, 1.0f);
                    me->AddThreat(horse, 1.0f);
                    me->AI()->AttackStart(horse);
                }

                if (caster->GetTypeId() == TYPEID_PLAYER && caster->ToPlayer()->GetQuestStatus(QUEST_SACRIFICES) == QUEST_STATUS_INCOMPLETE)
                {
                    caster->ToPlayer()->KilledMonsterCredit(NPC_BLOODFANG_STALKER_CREDIT, me->GetGUID());
					me->DespawnOrUnsummon(5000);
                }
            }
        }
    };
};

/*######
## npc_gilnean_crow
######*/

class npc_gilnean_crow : public CreatureScript
{
public:
    npc_gilnean_crow() : CreatureScript("npc_gilnean_crow") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_gilnean_crowAI (creature);
    }

    struct npc_gilnean_crowAI : public ScriptedAI
    {
        npc_gilnean_crowAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 tSpawn;
        bool Move;

        void Reset()
        {
            Move            = false;
            tSpawn          = 0;
            me->SetPosition(me->GetCreatureData()->posX,me->GetCreatureData()->posY, me->GetCreatureData()->posZ, me->GetCreatureData()->orientation);
        }

        void SpellHit(Unit* /*caster*/, const SpellInfo* spell)
        {
            if (spell->Id == SPELL_PING_GILNEAN_CROW)
            {
                if (!Move)
                {
                    me->SetUInt32Value(UNIT_NPC_EMOTESTATE , EMOTE_ONESHOT_NONE); // Change our emote state to allow flight
                    me->SetFlying(true);
                    Move = true;
                }
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (!Move)
                return;

            if (tSpawn <= diff)
            {
                me->GetMotionMaster()->MovePoint(0, (me->GetPositionX() + irand(-15, 15)), (me->GetPositionY() + irand(-15, 15)), (me->GetPositionZ() + irand(5, 15)), true);
                tSpawn = urand (500, 1000);
            }
            else tSpawn -= diff;

            if ((me->GetPositionZ() - me->GetCreatureData()->posZ) >= 20.0f)
            {
                me->DisappearAndDie();
                me->RemoveCorpse(true);
                Move = false;
            }
        }
    };
};

/*######
## npc_lord_darius_crowley_c3
######*/
class npc_lord_darius_crowley_c3 : public CreatureScript
{
public:
	npc_lord_darius_crowley_c3() : CreatureScript("npc_lord_darius_crowley_c3") {}

	bool OnQuestReward(Player* player, Creature* creature, const Quest *_Quest, uint32)
	{
		if (_Quest->GetQuestId() == 14222)
		{
			WorldLocation loc;
			loc.m_mapId = 654;
			loc.m_positionX = -1818.4f;
			loc.m_positionY = 2294.25f;
			loc.m_positionZ = 42.2135f;
			loc._orientation = 3.14f;

			player->SetHomebind(loc, 4786);

			player->RemoveAura(72872);
			player->RemoveAura(72870);
			player->CastSpell(player, 93477, true);
			player->CastSpell(player, 94293, true);
			//player->CastSpell(player, 68996, true);
			player->CastSpell(player, 69196, true);
			player->CastSpell(player, 72788, true);
			player->CastSpell(player, 72857, true);
			player->TeleportTo(loc);
		}
		return true;
	}
};

// Duskhaven Start
enum ZonePhasesDH
{
	SPELL_ZONE_SPECIFIC_01 = 59073, // Phase 2
	SPELL_ZONE_SPECIFIC_06 = 68481, // Phase 4096
	SPELL_ZONE_SPECIFIC_07 = 68482,// Phase 8192
	SPELL_ZONE_SPECIFIC_08 = 68483,// Phase 16384
	SPELL_ZONE_SPECIFIC_11 = 69484,// Phase 131072 (100 yds)
	SPELL_ZONE_SPECIFIC_12 = 69485,// Phase 262144
	SPELL_ZONE_SPECIFIC_13 = 69486,// Phase 524288
	SPELL_ZONE_SPECIFIC_14 = 70695,// Phase 1048576
	SPELL_ZONE_SPECIFIC_19 = 74096,// Phase 33554432
};

/*######
## go_mandragore
######*/
class go_mandragore : public GameObjectScript
{
public:
	go_mandragore() : GameObjectScript("go_mandragore") {}

	bool OnQuestReward(Player* player, GameObject *, Quest const* _Quest, uint32)
	{
		if (_Quest->GetQuestId() == 14320)
		{
			player->SendCinematicStart(168);
		}
		return true;
	}
};

/*######
## Quest Invasion 14321
######*/

enum sWatchman
{
	QUEST_INVASION = 14321,
	NPC_FORSAKEN_ASSASSIN = 36207,
	SPELL_BACKSTAB = 75360,
};

class npc_slain_watchman : public CreatureScript
{
public:
	npc_slain_watchman() : CreatureScript("npc_slain_watchman") { }

	bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
	{
		enum
		{
			SPELL_FORCECAST_SUMMON_FORSAKEN_ASSASSIN = 68492,
		};

		if (quest->GetQuestId() == QUEST_INVASION)
			creature->CastSpell(player, SPELL_FORCECAST_SUMMON_FORSAKEN_ASSASSIN, false, NULL, NULL, player->GetGUID());

		return false;
	}
};

class npc_gwen_armstead_phase_6 : public CreatureScript
{
public:
	npc_gwen_armstead_phase_6() : CreatureScript("npc_gwen_armstead_phase_6") { }

	bool OnQuestComplete(Player* player, Creature* armstead, Quest const* quest)
	{
		if (quest->GetQuestId() == QUEST_INVASION)
		{
			player->RemoveAurasByType(SPELL_AURA_PHASE);
			player->SetPhaseMask(2, true);
			player->SaveToDB();
		}

		return true;
	}
};

enum greymane_dh
{
	NPC_FORSAKEN_INVADER = 34511,
	SPELL_THROW_BOTTLE = 68552
};

class npc_prince_liam_greymane_dh : public CreatureScript
{
public:
	npc_prince_liam_greymane_dh() : CreatureScript("npc_prince_liam_greymane_dh") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_prince_liam_greymane_dhAI(creature);
	}

	struct npc_prince_liam_greymane_dhAI : public ScriptedAI
	{
		npc_prince_liam_greymane_dhAI(Creature* creature) : ScriptedAI(creature)
		{
			SetCombatMovement(false);
		}

		uint32 uiShootTimer;
		bool miss;

		void Reset()
		{
			uiShootTimer = 1000;
			miss = false;

			if (Unit* target = me->FindNearestCreature(NPC_FORSAKEN_INVADER, 40.0f))
				AttackStart(target);
		}

		void UpdateAI(uint32 const diff)
		{
			if (!UpdateVictim())
				return;

			if (me->getVictim()->GetTypeId() == TYPEID_UNIT)
			{
				if (me->getVictim()->GetHealthPct() < 90)
					miss = true;
				else
					miss = false;
			}

			if (uiShootTimer <= diff)
			{
				uiShootTimer = 1500;

				if (!me->IsWithinMeleeRange(me->getVictim(), 0.0f))
					if (Unit* target = me->FindNearestCreature(NPC_FORSAKEN_INVADER, 40.0f))
						if (target != me->getVictim())
						{
							me->getThreatManager().modifyThreatPercent(me->getVictim(), -100);
							me->CastSpell(me->getVictim(), SPELL_THROW_BOTTLE, false);
							me->CombatStart(target);
							me->AddThreat(target, 1000);
						}

				if (!me->IsWithinMeleeRange(me->getVictim(), 0.0f))
				{
					if (me->HasUnitState(UNIT_STATE_MELEE_ATTACKING))
					{
						me->SetUInt32Value(UNIT_NPC_EMOTESTATE, 0);
						me->ClearUnitState(UNIT_STATE_MELEE_ATTACKING);
						me->SendMeleeAttackStop(me->getVictim());
					}

					me->CastSpell(me->getVictim(), SPELL_SHOOT, false);
				}
				else
					if (!me->HasUnitState(UNIT_STATE_MELEE_ATTACKING))
					{
						me->AddUnitState(UNIT_STATE_MELEE_ATTACKING);
						me->SendMeleeAttackStart(me->getVictim());
					}
			}
			else
				uiShootTimer -= diff;

			DoMeleeAttackIfReady();
		}
	};
};

/* ######
## You Can't Take 'Em Alone - 14348
###### */

enum eHorrid
{
	NPC_HORRID_ABOMINATION_KILL_CREDIT = 36233,
	NPC_PRINCE_LIAM_GREYMANE_QYCTEA = 36140,

	SAY_BARREL = 0,

	SPELL_KEG_PLACED = 68555,
	SPELL_SHOOT_QYCTEA = 68559,   // 68559
	SPELL_RESTITCHING = 68864,
	SPELL_EXPLOSION = 68560,
	SPELL_EXPLOSION_POISON = 42266,
	SPELL_EXPLOSION_BONE_TYPE_ONE = 42267,
	SPELL_EXPLOSION_BONE_TYPE_TWO = 42274,
};

class npc_horrid_abomination : public CreatureScript
{
public:
	npc_horrid_abomination() : CreatureScript("npc_horrid_abomination") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_horrid_abominationAI(creature);
	}

	struct npc_horrid_abominationAI : public ScriptedAI
	{
		npc_horrid_abominationAI(Creature* creature) : ScriptedAI(creature)
		{
			uiRestitchingTimer = 3000;
			uiShootTimer = 3000;
			uiPlayerGUID = 0;
			shoot = false;
			miss = false;
			me->_SightDistance = 10.0f;
		}

		uint64 uiPlayerGUID;
		uint32 uiShootTimer;
		uint32 uiRestitchingTimer;
		bool shoot;
		bool miss;

		void SpellHit(Unit* caster, const SpellInfo* spell)
		{
			if (spell->Id == SPELL_KEG_PLACED)
			{
				switch (urand(0, 5))
				{
				case 0:
					me->MonsterSay(SAY_BARREL_1, LANGUAGE_UNIVERSAL, 0);
					break;
				case 1:
					me->MonsterSay(SAY_BARREL_2, LANGUAGE_UNIVERSAL, 0);
					break;
				case 2:
					me->MonsterSay(SAY_BARREL_3, LANGUAGE_UNIVERSAL, 0);
					break;
				case 3:
					me->MonsterSay(SAY_BARREL_4, LANGUAGE_UNIVERSAL, 0);
					break;
				case 4:
					me->MonsterSay(SAY_BARREL_5, LANGUAGE_UNIVERSAL, 0);
					break;
				case 5:
					me->MonsterSay(SAY_BARREL_6, LANGUAGE_UNIVERSAL, 0);
					break;
				}
				shoot = true;
				uiPlayerGUID = caster->GetGUID();
				me->SetReactState(REACT_PASSIVE);
				me->GetMotionMaster()->MoveRandom(5.0f);
				me->CombatStop();

			}

			if (spell->Id == SPELL_SHOOT_QYCTEA)
				ShootEvent();
		}

		void ShootEvent()
		{
			me->RemoveAura(SPELL_KEG_PLACED);

			for (int i = 0; i < 11; ++i)
				DoCast(SPELL_EXPLOSION_POISON);

			for (int i = 0; i < 6; ++i)
				DoCast(SPELL_EXPLOSION_BONE_TYPE_ONE);

			for (int i = 0; i < 4; ++i)
				DoCast(SPELL_EXPLOSION_BONE_TYPE_TWO);

			DoCast(SPELL_EXPLOSION);

			if (Player* player = Unit::GetPlayer(*me, uiPlayerGUID))
				player->KilledMonsterCredit(NPC_HORRID_ABOMINATION_KILL_CREDIT, 0);

			me->DespawnOrUnsummon(1000);
		}

		void DamageTaken(Unit* attacker, uint32 &/*damage*/)
		{
			if (attacker->GetTypeId() != TYPEID_PLAYER)
				return;

			Unit* victim = NULL;

			if (Unit* victim = me->getVictim())
				if (victim->GetTypeId() == TYPEID_PLAYER)
					return;

			if (victim)
				me->getThreatManager().modifyThreatPercent(victim, -100);

			AttackStart(attacker);
			me->AddThreat(attacker, 10005000);
		}

		void UpdateAI(uint32 const diff)
		{
			if (shoot)
			{
				if (uiShootTimer <= diff)
				{
					shoot = false;
					uiShootTimer = 3000;
					std::list<Creature*> liamList;
					GetCreatureListWithEntryInGrid(liamList, me, NPC_PRINCE_LIAM_GREYMANE_QYCTEA, 50.0f);

					if (liamList.empty())
						ShootEvent();
					else
						(*liamList.begin())->CastSpell(me, SPELL_SHOOT_QYCTEA, false);
				}
				else
					uiShootTimer -= diff;
			}

			if (!UpdateVictim())
				return;

			if (me->getVictim() && me->getVictim()->GetTypeId() == TYPEID_UNIT)
			{
				if (me->getVictim()->GetHealthPct() < 90)
					miss = true;
				else
					miss = false;
			}

			if (uiRestitchingTimer <= diff)
			{
				uiRestitchingTimer = 8000;
				DoCast(SPELL_RESTITCHING);
			}
			else
				uiRestitchingTimer -= diff;

			DoMeleeAttackIfReady();
		}
	};
};

/*######
## Quest Two By Sea 14382
######*/

enum qTBS
{
	NPC_FORSACEN_CATAPILT = 36283,

};

class npc_forsaken_catapult_qtbs : public CreatureScript
{
public:
	npc_forsaken_catapult_qtbs() : CreatureScript("npc_forsaken_catapult_qtbs") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_forsaken_catapult_qtbsAI(creature);
	}

	struct npc_forsaken_catapult_qtbsAI : public ScriptedAI
	{
		npc_forsaken_catapult_qtbsAI(Creature* creature) : ScriptedAI(creature)
		{
			me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
			me->setActive(true);
			speedXY = 10.0f;
			speedZ = 10.0f;
			creature->GetPosition(x, y, z);
			uiCastTimer = urand(5000, 10000);
			uiRespawnTimer = 10000;
			CanCast = true;
			respawn = false;
		}

		float speedXY, speedZ, x, y, z;
		uint32 uiCastTimer;
		uint32 uiRespawnTimer;
		bool CanCast;
		bool respawn;

		void PassengerBoarded(Unit* /*who*/, int8 seatId, bool apply)
		{
			if (!apply)
			{
				respawn = true;
				CanCast = false;

				if (seatId == 2)
					me->setFaction(35);
			}
			else
			{
				respawn = false;

				if (seatId == 2)
					CanCast = true;
			}
		}

		void UpdateAI(uint32 const diff)
		{

			if (respawn)
			{
				if (uiRespawnTimer <= diff)
				{
					respawn = false;
					uiRespawnTimer = 10000;
					me->DespawnOrUnsummon();
				}
				else
					uiRespawnTimer -= diff;
			}

			if (CanCast)
			{
				if (uiCastTimer <= diff)
				{
					uiCastTimer = urand(7000, 20000);
					float x, y, z;
					me->GetNearPoint2D(x, y, urand(100, 150), me->GetOrientation());
					z = me->GetBaseMap()->GetHeight(x, y, MAX_HEIGHT);
					me->CastSpell(x, y, z, 89697, false); // this = 68591 the correct spell but needs scripting
				}
				else
					uiCastTimer -= diff;
			}

			if (!UpdateVictim())
				return;

			DoMeleeAttackIfReady();
		}
	};
};

/*######
## Quest Save the Children! 14368
######*/

enum qSTC
{
	QUEST_SAVE_THE_CHILDREN = 14368,

	NPC_CYNTHIA = 36287,
	NPC_ASHLEY = 36288,
	NPC_JAMES = 36289,

	SPELL_SAVE_CYNTHIA = 68597,
	SPELL_SAVE_ASHLEY = 68598,
	SPELL_SAVE_JAMES = 68596,

	GO_DOOR_TO_THE_BASEMENT = 206693,
};

#define    PLAYER_SAY_CYNTHIA    "Its not safe here. Go to the Allens basment."
#define    PLAYER_SAY_ASHLEY     "Join the others inside the basement next door. Hurry!"
#define    PLAYER_SAY_JAMES      "Your mothers in the basement next door. Get to her now!"

class npc_james : public CreatureScript
{
public:
	npc_james() : CreatureScript("npc_james") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_jamesAI(creature);
	}

	struct npc_jamesAI : public npc_escortAI
	{
		npc_jamesAI(Creature* creature) : npc_escortAI(creature)
		{
			uiEventTimer = 3500;
			uiPlayerGUID = 0;
			Event = false;
		}

		uint64 uiPlayerGUID;
		uint32 uiEventTimer;
		bool Event;

		void Reset()
		{
			me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);

		}

		void SpellHit(Unit* caster, const SpellInfo* spell)
		{
			if (spell->Id == SPELL_SAVE_JAMES)
				if (Player* player = caster->ToPlayer())
				{
					Event = true;
					uiPlayerGUID = player->GetGUID();
					player->Say(PLAYER_SAY_JAMES, LANGUAGE_UNIVERSAL);
					caster->ToPlayer()->KilledMonsterCredit(me->GetEntry(), me->GetGUID());
				}
		}

		void WaypointReached(uint32 point)
		{
			if (point == 7)
				if (GameObject* door = me->FindNearestGameObject(GO_DOOR_TO_THE_BASEMENT, 10.0f))
					door->UseDoorOrButton();
		}

		void UpdateAI(uint32 const diff)
		{
			npc_escortAI::UpdateAI(diff);

			if (Event)
			{
				if (uiEventTimer <= diff)
				{
					Event = false;
					uiEventTimer = 3500;

					if (Unit::GetPlayer(*me, uiPlayerGUID))
					{
						me->MonsterSay("Don't hurt me! I was just looking for my sisters! I think Ashley's inside that house!", 0, 0);
					}
				}
				else
					uiEventTimer -= diff;
			}
		}
	};
};

class npc_ashley : public CreatureScript
{
public:
	npc_ashley() : CreatureScript("npc_ashley") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_ashleyAI(creature);
	}

	struct npc_ashleyAI : public npc_escortAI
	{
		npc_ashleyAI(Creature* creature) : npc_escortAI(creature)
		{
			uiEventTimer = 3500;
			uiPlayerGUID = 0;
			Event = false;
		}

		uint64 uiPlayerGUID;
		uint32 uiEventTimer;
		bool Event;

		void Reset()
		{
			me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);

		}

		void SpellHit(Unit* caster, const SpellInfo* spell)
		{
			if (spell->Id == SPELL_SAVE_ASHLEY)
				if (Player* player = caster->ToPlayer())
				{
					Event = true;
					uiPlayerGUID = player->GetGUID();
					player->Say(PLAYER_SAY_ASHLEY, LANGUAGE_UNIVERSAL);
					caster->ToPlayer()->KilledMonsterCredit(me->GetEntry(), me->GetGUID());
				}
		}

		void WaypointReached(uint32 point)
		{
			if (point == 16)
				if (GameObject* door = me->FindNearestGameObject(GO_DOOR_TO_THE_BASEMENT, 10.0f))
					door->UseDoorOrButton();
		}

		void UpdateAI(uint32 const diff)
		{
			npc_escortAI::UpdateAI(diff);

			if (Event)
			{
				if (uiEventTimer <= diff)
				{
					Event = false;
					uiEventTimer = 3500;

					if (Unit::GetPlayer(*me, uiPlayerGUID))
					{
						me->MonsterSay("Are you one of the good worgen, mister? Did you see Cynthia hiding in the sheds outside?", 0, 0);
					}
				}
				else
					uiEventTimer -= diff;
			}
		}
	};
};

class npc_cynthia : public CreatureScript
{
public:
	npc_cynthia() : CreatureScript("npc_cynthia") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_cynthiaAI(creature);
	}

	struct npc_cynthiaAI : public npc_escortAI
	{
		npc_cynthiaAI(Creature* creature) : npc_escortAI(creature)
		{
			uiEventTimer = 3500;
			uiPlayerGUID = 0;
			Event = false;
		}

		uint64 uiPlayerGUID;
		uint32 uiEventTimer;
		bool Event;

		void Reset()
		{
			me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);

		}

		void SpellHit(Unit* caster, const SpellInfo* spell)
		{
			if (spell->Id == SPELL_SAVE_CYNTHIA)
				if (Player* player = caster->ToPlayer())
				{
					Event = true;
					uiPlayerGUID = player->GetGUID();
					player->Say(PLAYER_SAY_CYNTHIA, LANGUAGE_UNIVERSAL);
					caster->ToPlayer()->KilledMonsterCredit(me->GetEntry(), me->GetGUID());
				}
		}

		void WaypointReached(uint32 point)
		{
			if (point == 8)
				if (GameObject* door = me->FindNearestGameObject(GO_DOOR_TO_THE_BASEMENT, 10.0f))
					door->UseDoorOrButton();
		}

		void UpdateAI(uint32 const diff)
		{
			npc_escortAI::UpdateAI(diff);

			if (Event)
			{
				if (uiEventTimer <= diff)
				{
					Event = false;
					uiEventTimer = 3500;

					if (/*Player* player = */Unit::GetPlayer(*me, uiPlayerGUID))
					{
						me->MonsterSay("You are scary! I just want my mommy!", 0, 0);
					}
				}
				else
					uiEventTimer -= diff;
			}
		}
	};
};

// Quest Leader of the Pack 14386
class npc_dark_ranger_thyala : public CreatureScript
{
public:
	npc_dark_ranger_thyala() : CreatureScript("npc_dark_ranger_thyala") { }

private:
	CreatureAI* GetAI(Creature* creature) const
	{
		return new creature_script_impl(creature);
	}

	struct creature_script_impl : public ScriptedAI
	{
		creature_script_impl(Creature* creature)
			: ScriptedAI(creature)
			, summons(creature)
		{ }

		enum
		{
			EVENT_KNOCKBACK = 1,

			SPELL_SHOOT = 16100,
			SPELL_KNOCKBACK = 68683,

			NPC_ATTACK_MASTIFF = 36409,
		};

		EventMap events;
		SummonList summons;

		void InitializeAI()
		{
			Reset();
			SetCombatMovement(false);
			me->_CombatDistance = 45.0f;
		}

		void Reset()
		{
			events.Reset();

			events.ScheduleEvent(EVENT_KNOCKBACK, urand(2500, 5000));
		}

		void DamageTaken(Unit* attacker, uint32 &damage)
		{
			if (me->GetHealth() < damage)
			{
				std::list<HostileReference*> threatList = me->getThreatManager().getThreatList();
				for (std::list<HostileReference*>::const_iterator itr = threatList.begin(); itr != threatList.end(); ++itr)
					if (Player* player = ObjectAccessor::GetPlayer((*me), (*itr)->getUnitGuid()))
						player->KilledMonsterCredit(me->GetEntry(), me->GetGUID());
			}
		}

		void JustSummoned(Creature* summoned)
		{
			summons.Summon(summoned);
		}

		void JustDied(Unit* /*who*/)
		{
			summons.DespawnAll();
		}

		void UpdateAI(uint32 const diff)
		{
			events.Update(diff);

			if (events.ExecuteEvent() == EVENT_KNOCKBACK)
			{
				me->CastSpell((Unit*)NULL, SPELL_KNOCKBACK, false);
				events.ScheduleEvent(EVENT_KNOCKBACK, urand(7500, 15000));
			}
		}
	};
};

class npc_attack_mastiff : public CreatureScript
{
public:
	npc_attack_mastiff() : CreatureScript("npc_attack_mastiff") { }

private:
	CreatureAI* GetAI(Creature* creature) const
	{
		return new creature_script_impl(creature);
	}

	struct creature_script_impl : public ScriptedAI
	{
		creature_script_impl(Creature* creature) : ScriptedAI(creature) { }

		enum
		{
			EVENT_DEMORALIZING_ROAR = 1,
			EVENT_LEAP = 2,
			EVENT_TAUNT = 3,

			SPELL_DEMORALIZING_ROAR = 15971,
			SPELL_LEAP = 68687,
			SPELL_TAUNT = 26281,
		};

		EventMap events;

		void Reset()
		{
			events.Reset();
		}

		void EnterCombat(Unit* who)
		{
			events.ScheduleEvent(EVENT_DEMORALIZING_ROAR, urand(2500, 5000));
			events.ScheduleEvent(SPELL_TAUNT, urand(2500, 5000));
			events.ScheduleEvent(SPELL_LEAP, urand(5000, 10000));
		}

		void JustDied(Unit* /*killer*/)
		{
			events.Reset();
		}

		void UpdateAI(uint32 const diff)
		{
			if (!UpdateVictim())
				return;

			events.Update(diff);

			if (uint32 eventId = events.ExecuteEvent())
			{
				switch (eventId)
				{
				case EVENT_DEMORALIZING_ROAR:
					me->CastSpell((Unit*)NULL, SPELL_DEMORALIZING_ROAR, false);
					events.ScheduleEvent(EVENT_DEMORALIZING_ROAR, urand(5000, 15000));
					break;
				case SPELL_TAUNT:
					me->CastSpell(me->getVictim(), SPELL_TAUNT, false);
					events.ScheduleEvent(SPELL_TAUNT, urand(5000, 10000));
					break;
				case SPELL_LEAP:
					me->CastSpell(me->getVictim(), SPELL_LEAP, false);
					events.ScheduleEvent(SPELL_LEAP, urand(5000, 10000));
					break;
				}
			}

			DoMeleeAttackIfReady();
		}
	};
};

class spell_call_attack_mastiffs : public SpellScriptLoader
{
public:
	spell_call_attack_mastiffs() : SpellScriptLoader("spell_call_attack_mastiffs") { }

private:
	class spell_script_impl : public SpellScript
	{
		PrepareSpellScript(spell_script_impl)

			void SummonMastiffs(SpellEffIndex effIndex)
		{
			PreventHitDefaultEffect(effIndex);

			enum
			{
				NPC_ATTACK_MASTIFF = 36409,
			};

			Unit* caster = GetCaster();
			Creature* target = GetHitCreature();

			if (!(caster && target && target->isAlive()))
				return;

			float angle = target->GetHomePosition().GetOrientation();

			for (int i = 0; i < 12; ++i)
			{
				float x, y;
				float dist = urand(20.f, 40.f);
				float angleOffset = frand(-M_PI / 4, M_PI / 4);
				target->GetNearPoint2D(x, y, dist, angle + angleOffset);
				float z = 1;
				float summonAngle = target->GetAngle(x, y);

				if (Creature* mastiff = target->SummonCreature(NPC_ATTACK_MASTIFF, x, y, z, M_PI + summonAngle, TEMPSUMMON_TIMED_DESPAWN, 60000))
				{
					mastiff->Attack(target, true);
					mastiff->AddThreat(target, std::numeric_limits<float>::max());
					mastiff->GetMotionMaster()->MoveChase(target);
				}
				if (Creature* mastiff2 = target->SummonCreature(NPC_ATTACK_MASTIFF, x, y, z, M_PI + summonAngle, TEMPSUMMON_TIMED_DESPAWN, 60000))
				{
					mastiff2->Attack(target, true);
					mastiff2->AddThreat(target, std::numeric_limits<float>::max());
					mastiff2->GetMotionMaster()->MoveChase(target);
				}
				if (Creature* mastiff3 = target->SummonCreature(NPC_ATTACK_MASTIFF, x, y, z, M_PI + summonAngle, TEMPSUMMON_TIMED_DESPAWN, 60000))
				{
					mastiff3->Attack(target, true);
					mastiff3->AddThreat(target, std::numeric_limits<float>::max());
					mastiff3->GetMotionMaster()->MoveChase(target);
				}
				if (Creature* mastiff4 = target->SummonCreature(NPC_ATTACK_MASTIFF, x, y, z, M_PI + summonAngle, TEMPSUMMON_TIMED_DESPAWN, 60000))
				{
					mastiff4->Attack(target, true);
					mastiff4->AddThreat(target, std::numeric_limits<float>::max());
					mastiff4->GetMotionMaster()->MoveChase(target);
				}
				if (Creature* mastiff5 = target->SummonCreature(NPC_ATTACK_MASTIFF, x, y, z, M_PI + summonAngle, TEMPSUMMON_TIMED_DESPAWN, 60000))
				{
					mastiff5->Attack(target, true);
					mastiff5->AddThreat(target, std::numeric_limits<float>::max());
					mastiff5->GetMotionMaster()->MoveChase(target);
				}
			}
		}

		void Register()
		{
			OnEffectHitTarget += SpellEffectFn(spell_script_impl::SummonMastiffs, EFFECT_0, SPELL_EFFECT_DUMMY);
		}
	};

	SpellScript *GetSpellScript() const
	{
		return new spell_script_impl();
	}
};

class npc_lord_godfrey_phase_7 : public CreatureScript
{
public:
	npc_lord_godfrey_phase_7() : CreatureScript("npc_lord_godfrey_phase_7") { }

private:
	bool OnQuestReward(Player* player, Creature* creature, Quest const* quest, uint32 /*opt*/)
	{
		if (quest->GetQuestId() == 14386)
		{
			// Map phase shift works however world map doesn't update need to look into it.
			player->GetSession()->SendPhaseShift_Override(0, 655);
			creature->BossYell("Hold your positions, men!", 0);
			Creature* Melinda = creature->FindNearestCreature(36291, 100.0f);
			if (Melinda)
			{
				Melinda->MonsterSay("What's happening?!", 0, 0);
			}
			player->RemoveAurasByType(SPELL_AURA_PHASE);
			player->SetPhaseMask(1, true);
			player->SaveToDB();
		}

		return false;
	}
};

/*###### Quest Gasping for Breath  ######*/

enum qGFB
{
	QUEST_GASPING_FOR_BREATH = 14395,

	NPC_QGFB_KILL_CREDIT = 36450,
	NPC_DROWNING_WATCHMAN = 36440,

	SPELL_RESCUE_DROWNING_WATCHMAN = 68735,
	SPELL_SUMMON_SPARKLES = 69253,
	SPELL_DROWNING = 68730,

	GO_SPARKLES = 197333,
};

class npc_drowning_watchman : public CreatureScript
{
public:
	npc_drowning_watchman() : CreatureScript("npc_drowning_watchman") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_drowning_watchmanAI(creature);
	}

	struct npc_drowning_watchmanAI : public ScriptedAI
	{
		npc_drowning_watchmanAI(Creature* creature) : ScriptedAI(creature)
		{
			reset = true;
			despawn = false;
			exit = false;
			uiDespawnTimer = 10000;
		}

		uint32 uiDespawnTimer;
		bool reset;
		bool despawn;
		bool exit;

		void SpellHit(Unit* caster, const SpellInfo* spell)
		{
			if (spell->Id == SPELL_RESCUE_DROWNING_WATCHMAN)
			{
				despawn = false;
				uiDespawnTimer = 10000;
				me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
				me->RemoveAura(SPELL_DROWNING);
				me->EnterVehicle(caster);

				if (GameObject* go = me->FindNearestGameObject(GO_SPARKLES, 10.0f))
					go->Delete();
			}
		}

		void OnExitVehicle(Unit* /*vehicle*/, uint32 /*seatId*/)
		{
			if (!exit)
			{
				float x, y, z, o;
				me->GetPosition(x, y, z, o);
				me->SetHomePosition(x, y, z, o);
				me->Relocate(x, y, z, o);
				reset = true;
				despawn = true;
				Reset();
			}
		}

		void Reset()
		{
			exit = false;

			if (reset)
			{
				DoCast(SPELL_DROWNING);
				me->SetVisible(true);
				DoCast(SPELL_SUMMON_SPARKLES);
				me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
				reset = false;
			}
		}

		void UpdateAI(uint32 const diff)
		{
			if (despawn)
			{
				if (uiDespawnTimer <= diff)
				{
					if (GameObject* go = me->FindNearestGameObject(GO_SPARKLES, 10.0f))
						go->Delete();

					reset = true;
					despawn = false;
					uiDespawnTimer = 10000;
					me->DespawnOrUnsummon();
				}
				else
					uiDespawnTimer -= diff;
			}
		}
	};
};

class npc_prince_liam_greymane : public CreatureScript
{
public:
	npc_prince_liam_greymane() : CreatureScript("npc_prince_liam_greymane") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_prince_liam_greymaneAI(creature);
	}

	struct npc_prince_liam_greymaneAI : public ScriptedAI
	{
		npc_prince_liam_greymaneAI(Creature* creature) : ScriptedAI(creature) { }

		void MoveInLineOfSight(Unit* who)
		{
			if (who->GetEntry() == NPC_DROWNING_WATCHMAN)
			{
				if (who->IsInWater() || !who->GetVehicle())
					return;

				if (who->GetDistance(-1897.0f, 2519.97f, 1.50667f) < 5.0f)
					if (Unit* unit = who->GetVehicleBase())
					{
						if (Creature* watchman = who->ToCreature())
						{
							watchman->MonsterSay("Thank you... *gasp* I owe you my life.", 0, 0);
							watchman->DespawnOrUnsummon(15000);
							watchman->SetStandState(UNIT_STAND_STATE_KNEEL);
							CAST_AI(npc_drowning_watchman::npc_drowning_watchmanAI, watchman->AI())->exit = true;
							CAST_AI(npc_drowning_watchman::npc_drowning_watchmanAI, watchman->AI())->reset = true;
							who->ExitVehicle();
							unit->RemoveAura(SPELL_RESCUE_DROWNING_WATCHMAN);
						}

						if (Player* player = unit->ToPlayer())
							player->KilledMonsterCredit(NPC_QGFB_KILL_CREDIT, 0);
					}
			}
		}

		void UpdateAI(uint32 const diff)
		{
			if (!UpdateVictim())
				return;

			DoMeleeAttackIfReady();
		}
	};
};

/*###### Quest Gasping for Breath END  ######*/

/*######
## spell_round_up_horse
######*/

class spell_round_up_horse : public SpellScriptLoader
{
public:
	spell_round_up_horse() : SpellScriptLoader("spell_round_up_horse") { }

	class spell_round_up_horse_SpellScript : public SpellScript
	{
		PrepareSpellScript(spell_round_up_horse_SpellScript)

			void Trigger(SpellEffIndex effIndex)
		{
			Unit* target = GetExplTargetUnit();

			if (Creature* horse = target->ToCreature())
				if (horse->GetEntry() == 36540)
					if (Vehicle* vehicle = target->GetVehicleKit())
						if (vehicle->HasEmptySeat(0))
							return;

			PreventHitDefaultEffect(effIndex);
		}

		void Register()
		{
			OnEffectLaunch += SpellEffectFn(spell_round_up_horse_SpellScript::Trigger, EFFECT_0, SPELL_EFFECT_TRIGGER_SPELL);
		}
	};

	SpellScript *GetSpellScript() const
	{
		return new spell_round_up_horse_SpellScript();
	}
};

enum qTHE
{
	NPC_MOUNTAIN_HORSE_VEHICLE = 36540,
	NPC_MOUNTAIN_HORSE_KILL_CREDIT = 36560,
	SPELL_ROPE_CHANNEL = 68940,
	SPELL_ROPE_IN_HORSE = 68908,
};

class npc_round_up_horse : public CreatureScript
{
public:
	npc_round_up_horse() : CreatureScript("npc_round_up_horse") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_round_up_horseAI(creature);
	}

	struct npc_round_up_horseAI : public ScriptedAI
	{
		npc_round_up_horseAI(Creature* creature) : ScriptedAI(creature)
		{
			//me->SetReactState(REACT_PASSIVE);

			if (me->isSummon())
				if (Unit* summoner = me->ToTempSummon()->GetSummoner())
					if (Player* player = summoner->ToPlayer())
					{
						me->GetMotionMaster()->MoveFollow(player, 1.0f, 0.0f);
						me->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, player->GetGUID());
						me->SetUInt32Value(UNIT_CHANNEL_SPELL, SPELL_ROPE_CHANNEL);

						if (Creature* horse = player->GetVehicleCreatureBase())
							horse->AI()->JustSummoned(me);
					}
		}
	};
};

class npc_mountain_horse_vehicle : public CreatureScript
{
public:
	npc_mountain_horse_vehicle() : CreatureScript("npc_mountain_horse_vehicle") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_mountain_horse_vehicleAI(creature);
	}

	struct npc_mountain_horse_vehicleAI : public ScriptedAI
	{
		npc_mountain_horse_vehicleAI(Creature* creature) : ScriptedAI(creature)
		{
			creature->SetReactState(REACT_PASSIVE);
			lSummons.clear();
			uiDespawnTimer = 10000;
			despawn = false;
		}

		std::vector<Creature*> lSummons;
		uint32 uiDespawnTimer;
		uint64 PlayerGUID;
		bool despawn;

		void OnCharmed(bool /*charm*/)
		{

		}

		void SpellHit(Unit* /*caster*/, const SpellInfo* spell)
		{
			if (spell->Id == SPELL_ROPE_IN_HORSE)
				me->DespawnOrUnsummon();
		}

		void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply)
		{
			if (apply)
			{
				despawn = false;

				if (lSummons.empty())
					return;

				for (std::vector<Creature*>::const_iterator itr = lSummons.begin(); itr != lSummons.end(); ++itr)
					if (*itr)
						(*itr)->DespawnOrUnsummon();

				lSummons.clear();
			}
			else
			{
				uiDespawnTimer = 10000;
				despawn = true;

				if (me->FindNearestCreature(36457, 30.0f))
				{
					if (lSummons.empty())
						return;

					Player* player = who->ToPlayer();

					if (!player)
						return;

					for (std::vector<Creature*>::const_iterator itr = lSummons.begin(); itr != lSummons.end(); ++itr)
						if (*itr)
						{
							player->KilledMonsterCredit(NPC_MOUNTAIN_HORSE_KILL_CREDIT, 0);
							(*itr)->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, 0);
							(*itr)->SetUInt32Value(UNIT_CHANNEL_SPELL, 0);
							(*itr)->DespawnOrUnsummon();
						}

					lSummons.clear();
				}
				else
				{
					if (lSummons.empty())
						return;

					for (std::vector<Creature*>::const_iterator itr = lSummons.begin(); itr != lSummons.end(); ++itr)
						if (*itr)
							(*itr)->DespawnOrUnsummon();

					lSummons.clear();
				}
			}
		}

		void JustSummoned(Creature* summoned)
		{
			if (summoned->GetEntry() == 36540)
				lSummons.push_back(summoned);
		}

		void DespawnAllSummons()
		{
			for (std::vector<Creature*>::const_iterator itr = lSummons.begin(); itr != lSummons.end(); ++itr)
				if (*itr)
				{
					(*itr)->DespawnOrUnsummon();
				}
			lSummons.clear();
		}

		void UpdateAI(uint32 const diff)
		{
			if (me->GetVehicleBase() && me->GetVehicleBase()->ToPlayer()->GetQuestStatus(14416) == QUEST_STATUS_COMPLETE) // Player has completed quest so dismount
			{
				me->DespawnOrUnsummon();
			}

			if (me->FindNearestCreature(36457, 10.0f))
			{
				if (lSummons.empty())
					return;

				for (std::vector<Creature*>::const_iterator itr = lSummons.begin(); itr != lSummons.end(); ++itr)
					if (*itr)
					{
						if ((*itr)->ToTempSummon()->GetSummoner())
						{
							(*itr)->ToTempSummon()->GetSummoner()->ToPlayer()->KilledMonsterCredit(NPC_MOUNTAIN_HORSE_KILL_CREDIT, 0);
						}
						(*itr)->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, 0);
						(*itr)->SetUInt32Value(UNIT_CHANNEL_SPELL, 0);
						(*itr)->DespawnOrUnsummon();
					}

				lSummons.clear();
			}

			if (despawn)
			{
				if (uiDespawnTimer <= diff)
				{
					despawn = false;
					uiDespawnTimer = 10000;
					me->DespawnOrUnsummon();
				}
				else
					uiDespawnTimer -= diff;
			}
		}
	};
};

class npc_lorna_ettins : public CreatureScript
{
public:
	npc_lorna_ettins() : CreatureScript("npc_lorna_ettins") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_lorna_ettinsAI(creature);
	}

	struct npc_lorna_ettinsAI : public ScriptedAI
	{
		npc_lorna_ettinsAI(Creature* creature) : ScriptedAI(creature) { }

		void MoveInLineOfSight(Unit* who)
		{
			if (who->GetEntry() == 36555)
			{
				if (!who->isSummon())
					return;

				if (who->GetDistance(-2060.90f, 2254.909f, 22.44f) < 7.0f)
				{
					if (who->ToTempSummon()->GetSummoner())
					{
						who->ToTempSummon()->GetSummoner()->ToPlayer()->KilledMonsterCredit(36555, 0);
						who->ToTempSummon()->DespawnOrUnsummon();
					}
				}
			}
		}

		void UpdateAI(uint32 const diff)
		{
			if (!UpdateVictim())
				return;

			DoMeleeAttackIfReady();
		}
	};
};

enum qTGM
{
	QUEST_TO_GREYMANE_MANOR = 14465,

	NPC_SWIFT_MOUNTAIN_HORSE = 36741,

	GO_FIRST_GATE = 196399,
	GO_SECOND_GATE = 196401,

};

class npc_swift_mountain_horse : public CreatureScript
{
public:
	npc_swift_mountain_horse() : CreatureScript("npc_swift_mountain_horse") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_swift_mountain_horseAI(creature);
	}

	struct npc_swift_mountain_horseAI : public npc_escortAI
	{
		npc_swift_mountain_horseAI(Creature* creature) : npc_escortAI(creature)
		{
			//me->SetReactState(REACT_PASSIVE);
		}

		bool PlayerOn;

		void Reset()
		{
			PlayerOn = false;
		}

		void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply)
		{
			if (who->GetTypeId() == TYPEID_PLAYER)
			{
				PlayerOn = true;
				if (apply)
				{
					Start(false, true, who->GetGUID());
				}
			}
		}

		void WaypointReached(uint32 point)
		{
			Player* player = GetPlayerForEscort();

			switch (point)
			{
			case 4:
				if (GameObject* go = me->FindNearestGameObject(GO_FIRST_GATE, 30.0f))
					go->UseDoorOrButton();
				break;
			case 8:
				if (GameObject* go = me->FindNearestGameObject(GO_SECOND_GATE, 30.0f))
					go->UseDoorOrButton();
				break;
			case 9:
				if (me->isSummon())
					if (Unit* summoner = me->ToTempSummon()->GetSummoner())
						if (Player* player = summoner->ToPlayer())
						{
							WorldLocation loc;
							loc.m_mapId = 654;
							loc.m_positionX = -1586.57f;
							loc.m_positionY = 2551.24f;
							loc.m_positionZ = 130.218f;
							player->SetHomebind(loc, 817);
							player->CompleteQuest(14465);
							player->SetPhaseMask(6, true);
							player->SaveToDB();
						}
				me->DespawnOrUnsummon();
				break;
			}
		}

		void OnCharmed(bool /*apply*/)
		{
		}

		void UpdateAI(const uint32 diff)
		{
			npc_escortAI::UpdateAI(diff);
			Player* player = GetPlayerForEscort();

			if (PlayerOn)
			{
				player->SetClientControl(me, 0);
				PlayerOn = false;
			}
		}
	};
};

class npc_gwen_armstead : public CreatureScript
{
public:
	npc_gwen_armstead() : CreatureScript("npc_gwen_armstead") { }

	bool OnQuestAccept(Player* player, Creature* /*creature*/, Quest const* quest)
	{
		if (quest->GetQuestId() == 14465)
		{
			if (Creature* horse = player->SummonCreature(36741, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN))
			{
				if (player->GetShapeshiftForm() == FORM_CAT || FORM_BEAR)
				{
					player->RemoveAura(768); // Cat Form
					player->RemoveAura(5487);
				}
				horse->SetPhaseMask(6, true);
				player->SetPhaseMask(6, true);
				player->EnterVehicle(horse, 0);
				player->CastCustomSpell(VEHICLE_SPELL_RIDE_HARDCODED, SPELLVALUE_BASE_POINT0, 1, horse, false);
				CAST_AI(npc_escortAI, (horse->AI()))->Start(false, true, player->GetGUID(), quest);
				player->SaveToDB();
			}
		}
		return true;
	}
};

/*######
## Quest The King's Observatory 14466, Alas, Gilneas! 14467
######*/

enum qTKO_AG
{
	QUEST_THE_KINGS_OBSERVATORY = 14466,
	QUEST_ALAS_GILNEAS = 14467,
	QUEST_EXODUS = 24438,

	SPELL_CATACLYSM_TYPE_1 = 80133,
	SPELL_CATACLYSM_TYPE_2 = 68953,
	SPELL_CATACLYSM_TYPE_3 = 80134,

	CINEMATIC_TELESCOPE = 167,
};

class npc_king_genn_greymane_c3 : public CreatureScript
{
public:
	npc_king_genn_greymane_c3() : CreatureScript("npc_king_genn_greymane_c3") { }

	bool OnQuestComplete(Player* player, Creature* /*creature*/, Quest const* quest)
	{
		if (quest->GetQuestId() == QUEST_THE_KINGS_OBSERVATORY)
		{
			player->CastSpell(player, SPELL_CATACLYSM_TYPE_3, true);
			player->GetSession()->SendPhaseShift_Override(0, 656);
			player->SaveToDB();
		}

		if (quest->GetQuestId() == QUEST_ALAS_GILNEAS)
			player->SendCinematicStart(CINEMATIC_TELESCOPE);

		return true;
	}

	bool OnQuestAccept(Player* player, Creature* /*creature*/, Quest const* quest)
	{
		if (quest->GetQuestId() == QUEST_EXODUS)
		{
			//player->RemoveAura(SPELL_ZONE_SPECIFIC_19);
			player->RemoveAura(99488);
			player->RemoveAura(69484);
			player->RemoveAura(68481);
			player->RemoveAura(68243);
			player->RemoveAura(59087);
			player->RemoveAura(59074);
			player->RemoveAura(59073);
			//player->CastSpell(player, SPELL_ZONE_SPECIFIC_11, true);
			player->GetSession()->SendPhaseShift_Override(0, 656);
			player->TeleportTo(654, -2224.82f, 1826.68f, 13.20f, 5.0f);
			player->SaveToDB();
		}

		return true;
	}
};

/*######
## Quest Introductions Are in Order 24472
######*/

enum qIAO
{
	QUEST_INTRODUCTIONS_ARE_IN_ORDER = 24472,

	NPC_KOROTH_THE_HILLBREAKER_QIAO = 36294,
	NPC_KOROTH_THE_HILLBREAKER_QIAO_FRIEND = 37808,
	NPC_CAPTAIN_ASTHER_QIAO = 37806,
	NPC_FORSAKEN_SOLDIER_QIAO = 37805,
	NPC_FORSAKEN_CATAPULT_QIAO = 37807,

	KOROTH_YELL_WHO_STEAL_BANNER = 0,
	KOROTH_YELL_FIND_YOU = 1,
	LIAN_SAY_HERE_FORSAKEN = 0,
	LIAM_YELL_YOU_CANT = 1,
	CAPITAN_YELL_WILL_ORDER = -1977089,
	KOROTH_YELL_MY_BANNER = 0,

	SPELL_PUSH_BANNER = 70511,
	SPELL_CLEAVE = 16044,
	SPELL_DEMORALIZING_SHOUT_QIAO = 16244,

	POINT_BANNER = 1,
	ACTION_KOROTH_ATTACK = 2,
};

struct Psc_qiao
{
	uint64 uiPlayerGUID;
	uint64 uiCapitanGUID;
	uint32 uiEventTimer;
	uint8 uiPhase;
};

struct sSoldier
{
	Creature* soldier;
	float follow_angle;
	float follow_dist;
};

const float AstherWP[18][3] =
{
	{ -2129.99f, 1824.12f, 25.234f }, { -2132.93f, 1822.23f, 23.984f }, { -2135.81f, 1820.23f, 22.770f },
	{ -2138.72f, 1818.29f, 21.595f }, { -2141.77f, 1816.57f, 20.445f }, { -2144.88f, 1814.96f, 19.380f },
	{ -2147.19f, 1813.85f, 18.645f }, { -2150.51f, 1812.73f, 17.760f }, { -2153.88f, 1811.80f, 16.954f },
	{ -2157.28f, 1810.95f, 16.194f }, { -2160.69f, 1810.20f, 15.432f }, { -2164.12f, 1809.46f, 14.688f },
	{ -2167.55f, 1808.81f, 13.961f }, { -2171.01f, 1808.27f, 13.316f }, { -2174.32f, 1808.00f, 12.935f },
	{ -2177.11f, 1807.75f, 12.717f }, { -2179.79f, 1807.67f, 12.573f }, { -2183.06f, 1807.59f, 12.504f },
};

const float KorothWP[14][3] =
{
	{ -2213.64f, 1863.51f, 15.404f }, { -2212.69f, 1860.14f, 15.302f }, { -2211.15f, 1853.31f, 15.078f },
	{ -2210.39f, 1849.90f, 15.070f }, { -2209.11f, 1845.65f, 15.377f }, { -2206.19f, 1839.29f, 15.147f },
	{ -2204.92f, 1836.03f, 14.420f }, { -2203.76f, 1832.73f, 13.432f }, { -2201.68f, 1826.04f, 12.296f },
	{ -2200.68f, 1822.69f, 12.194f }, { -2199.22f, 1818.77f, 12.175f }, { -2196.29f, 1813.86f, 12.253f },
	{ -2192.46f, 1811.06f, 12.445f }, { -2186.79f, 1808.90f, 12.513f },
};


class npc_koroth_the_hillbreaker_qiao_friend : public CreatureScript
{
public:
	npc_koroth_the_hillbreaker_qiao_friend() : CreatureScript("npc_koroth_the_hillbreaker_qiao_friend") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_koroth_the_hillbreaker_qiao_friendAI(creature);
	}

	struct npc_koroth_the_hillbreaker_qiao_friendAI : public npc_escortAI
	{
		npc_koroth_the_hillbreaker_qiao_friendAI(Creature* creature) : npc_escortAI(creature)
		{
			me->setActive(true);
			me->SetReactState(REACT_PASSIVE);
			me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
			uiCleaveTimer = 250;
			uiDemoralizingShoutTimer = 500;

			if (me->isSummon())
			{
				for (int i = 0; i < 14; ++i)
					AddWaypoint(i, KorothWP[i][0], KorothWP[i][1], KorothWP[i][2]);

				Start();
				SetRun(true);
			}
		}

		uint32 uiCleaveTimer;
		uint32 uiDemoralizingShoutTimer;

		void FinishEscort()
		{
			if (me->isSummon())
				if (Unit* summoner = me->ToTempSummon()->GetSummoner())
				{
					me->SetReactState(REACT_AGGRESSIVE);
					me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

					if (Creature* capitan = summoner->ToCreature())
						capitan->AI()->DoAction(ACTION_KOROTH_ATTACK);

					/*  if (summoner->isSummon())
					if (Unit* _summoner = summoner->ToTempSummon()->GetSummoner())
					if (Player* player = _summoner->ToPlayer())*/
					me->MonsterYell("Corpse-men take Koroth's banner! Corpse-men get smashed to bitses!!!!", 0, 0);
				}
		}
		void WaypointReached(uint32 /*point*/) { }

		void UpdateAI(uint32 const diff)
		{
			npc_escortAI::UpdateAI(diff);

			if (!UpdateVictim())
				return;

			if (uiCleaveTimer <= diff)
			{
				uiCleaveTimer = urand(2500, 15000);
				DoCastVictim(SPELL_CLEAVE);
			}
			else
				uiCleaveTimer -= diff;

			if (uiDemoralizingShoutTimer <= diff)
			{
				uiDemoralizingShoutTimer = 5000;
				DoCast(SPELL_DEMORALIZING_SHOUT_QIAO);
			}
			else
				uiDemoralizingShoutTimer -= diff;

			DoMeleeAttackIfReady();
		}
	};
};

class npc_captain_asther_qiao : public CreatureScript
{
public:
	npc_captain_asther_qiao() : CreatureScript("npc_captain_asther_qiao") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_captain_asther_qiaoAI(creature);
	}

	struct npc_captain_asther_qiaoAI : public npc_escortAI
	{
		npc_captain_asther_qiaoAI(Creature* creature) : npc_escortAI(creature)
		{
			me->setActive(true);
			uiKorothGUID = 0;
			lSoldiers.clear();
			me->SetReactState(REACT_PASSIVE);
			me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

			if (me->isSummon())
				StartEvent();
		}

		std::list<sSoldier> lSoldiers;
		uint64 uiKorothGUID;

		void JustDied(Unit* /*who*/)
		{
			me->DespawnOrUnsummon(15000);

			if (Creature* koroth = Unit::GetCreature(*me, uiKorothGUID))
				koroth->DespawnOrUnsummon(15000);

			if (lSoldiers.empty())
				return;

			for (std::list<sSoldier>::iterator itr = lSoldiers.begin(); itr != lSoldiers.end(); ++itr)
			{
				Creature* soldier = itr->soldier;
				soldier->DespawnOrUnsummon(15000);
			}
		}

		void DoAction(int32 const action)
		{
			if (action == ACTION_KOROTH_ATTACK)
				if (Creature* koroth = Unit::GetCreature(*me, uiKorothGUID))
				{
					me->SetReactState(REACT_AGGRESSIVE);
					me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
					AttackStart(koroth);
					me->AddThreat(koroth, 100500);
					koroth->AddThreat(me, 100500);
					float x, y, z;
					koroth->GetNearPoint(koroth, x, y, z, 0.0f, 1.0f, koroth->GetOrientation() + M_PI);
					me->GetMotionMaster()->MoveCharge(x, y, z);

					if (lSoldiers.empty())
						return;

					for (std::list<sSoldier>::iterator itr = lSoldiers.begin(); itr != lSoldiers.end(); ++itr)
					{
						Creature* soldier = itr->soldier;
						soldier->SetReactState(REACT_AGGRESSIVE);
						soldier->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
						soldier->AI()->AttackStart(koroth);
						soldier->AddThreat(koroth, 100500);

						if (soldier->GetEntry() == NPC_FORSAKEN_CATAPULT_QIAO)
						{
							koroth->AddThreat(soldier, 200500);
							koroth->AI()->AttackStart(soldier);
							continue;
						}

						koroth->AddThreat(soldier, 10000);
						soldier->GetMotionMaster()->MoveCharge(x, y, z);
					}
				}
		}

		void StartMoveTo(float x, float y, float z)
		{
			if (lSoldiers.empty())
				return;

			float pathangle = atan2(me->GetPositionY() - y, me->GetPositionX() - x);

			for (std::list<sSoldier>::iterator itr = lSoldiers.begin(); itr != lSoldiers.end(); ++itr)
			{
				Creature* member = itr->soldier;

				if (!member->isAlive() || member->getVictim())
					continue;

				float angle = itr->follow_angle;
				float dist = itr->follow_dist;

				float dx = x - std::cos(angle + pathangle) * dist;
				float dy = y - std::sin(angle + pathangle) * dist;
				float dz = z;

				SkyFire::NormalizeMapCoord(dx);
				SkyFire::NormalizeMapCoord(dy);

				member->UpdateGroundPositionZ(dx, dy, dz);

				if (member->IsWithinDist(me, dist + 5.0f))
					member->SetUnitMovementFlags(me->GetUnitMovementFlags());
				else
					member->RemoveUnitMovementFlag(MOVEMENTFLAG_WALKING);

				member->GetMotionMaster()->MovePoint(0, dx, dy, dz);
				member->SetHomePosition(dx, dy, dz, pathangle);
			}
		}

		void SummonSoldier(uint64 guid, uint32 SoldierEntry, float dist, float angle)
		{
			float x, y;
			me->GetNearPoint2D(x, y, dist, me->GetOrientation() + angle);
			float z = me->GetBaseMap()->GetHeight(x, y, MAX_HEIGHT);

			if (Creature* soldier = me->SummonCreature(SoldierEntry, x, y, z))
			{
				soldier->SetReactState(REACT_PASSIVE);
				soldier->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
				//soldier->SetSeerGUID(guid);
				soldier->SetVisible(false);
				soldier->setActive(true);
				sSoldier new_soldier;
				new_soldier.soldier = soldier;
				new_soldier.follow_angle = angle;
				new_soldier.follow_dist = dist;
				lSoldiers.push_back(new_soldier);
			}
		}

		void StartEvent()
		{
			if (Unit* summoner = me->ToTempSummon()->GetSummoner())
				if (Player* player = summoner->ToPlayer())
				{
					uint64 uiPlayerGUID = player->GetGUID();

					for (int i = 1; i < 4; ++i)
					{
						SummonSoldier(uiPlayerGUID, NPC_FORSAKEN_SOLDIER_QIAO, i * 2, M_PI);
						SummonSoldier(uiPlayerGUID, NPC_FORSAKEN_SOLDIER_QIAO, i * 2, M_PI - M_PI / (2 * i));
						SummonSoldier(uiPlayerGUID, NPC_FORSAKEN_SOLDIER_QIAO, i * 2, M_PI + M_PI / (2 * i));
					}

					SummonSoldier(uiPlayerGUID, NPC_FORSAKEN_CATAPULT_QIAO, 8.0f, M_PI);

					for (int i = 0; i < 18; ++i)
						AddWaypoint(i, AstherWP[i][0], AstherWP[i][1], AstherWP[i][2]);

					Start();
				}
		}

		void WaypointReached(uint32 point)
		{
			if (point == 15)
				if (Creature* koroth = me->SummonCreature(NPC_KOROTH_THE_HILLBREAKER_QIAO_FRIEND, -2213.64f, 1863.51f, 15.404f))
					uiKorothGUID = koroth->GetGUID();
		}

		void UpdateAI(uint32 const diff)
		{
			npc_escortAI::UpdateAI(diff);

			if (!UpdateVictim())
				return;

			DoMeleeAttackIfReady();
		}
	};
};

class npc_prince_liam_greymane_qiao : public CreatureScript
{
public:
	npc_prince_liam_greymane_qiao() : CreatureScript("npc_prince_liam_greymane_qiao") { }

	bool OnQuestComplete(Player* player, Creature* creature, Quest const* quest)
	{
		if (quest->GetQuestId() == QUEST_INTRODUCTIONS_ARE_IN_ORDER)
			CAST_AI(npc_prince_liam_greymane_qiaoAI, creature->AI())->StartEvent(player);

		if (quest->GetQuestId() == QUEST_EXODUS)
		{
			WorldLocation loc;
			loc.m_mapId = 654;
			loc.m_positionX = -245.84f;
			loc.m_positionY = 1550.56f;
			loc.m_positionZ = 18.6917f;
			player->SetHomebind(loc, 4731);
			player->SaveToDB();
		}

		return true;
	}

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_prince_liam_greymane_qiaoAI(creature);
	}

	struct npc_prince_liam_greymane_qiaoAI : public ScriptedAI
	{
		npc_prince_liam_greymane_qiaoAI(Creature* creature) : ScriptedAI(creature)
		{
			lPlayerList.clear();
		}

		std::list<Psc_qiao> lPlayerList;

		void StartEvent(Player* player)
		{
			if (!player)
				return;

			uint64 uiPlayerGUID = player->GetGUID();
			Psc_qiao new_player;
			new_player.uiPlayerGUID = uiPlayerGUID;
			new_player.uiCapitanGUID = 0;
			new_player.uiEventTimer = 0;
			new_player.uiPhase = 0;
			Creature* capitan = NULL;

			if (Creature* capitan = player->SummonCreature(NPC_CAPTAIN_ASTHER_QIAO, -2120.19f, 1833.06f, 30.1510f, 3.87363f))
			{
				//capitan->SetSeerGUID(player->GetGUID());
				capitan->SetVisible(false);
				new_player.uiCapitanGUID = capitan->GetGUID();
			}

			if (!capitan)
				return;

			lPlayerList.push_back(new_player);
		}

		void UpdateAI(uint32 const diff)
		{
			if (!lPlayerList.empty())
				for (std::list<Psc_qiao>::iterator itr = lPlayerList.begin(); itr != lPlayerList.end();)
				{
					if (itr->uiEventTimer <= diff)
					{
						++itr->uiPhase;

						if (/*Player* player = */Unit::GetPlayer(*me, itr->uiPlayerGUID))
							switch (itr->uiPhase)
							{
							case 1:
								itr->uiEventTimer = 8000;
								Talk(LIAN_SAY_HERE_FORSAKEN);
								break;
							case 2:
								itr->uiEventTimer = 5000;
								me->HandleEmoteCommand(EMOTE_ONESHOT_ROAR);
								Talk(LIAM_YELL_YOU_CANT);
								break;
							case 3:
								itr->uiEventTimer = 3000;
								me->CastSpell(me, SPELL_PUSH_BANNER/*, false*/);
								break;
							case 4:
								if (Unit::GetCreature(*me, itr->uiCapitanGUID))
									if (Creature* capitan = me->FindNearestCreature(NPC_CAPTAIN_ASTHER_QIAO, 20.0f, true))
										capitan->MonsterYell("Worthless mongrel. I will order our outhouses cleaned with this rag you call a banner.", 0, 0);
								itr = lPlayerList.erase(itr);
								break;
							}
					}
					else
					{
						itr->uiEventTimer -= diff;
						++itr;
					}
				}

			if (!UpdateVictim())
				return;

			DoMeleeAttackIfReady();
		}
	};
};

class npc_koroth_the_hillbreaker_qiao : public CreatureScript
{
public:
	npc_koroth_the_hillbreaker_qiao() : CreatureScript("npc_koroth_the_hillbreaker_qiao") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_koroth_the_hillbreaker_qiaoAI(creature);
	}

	struct npc_koroth_the_hillbreaker_qiaoAI : public ScriptedAI
	{
		npc_koroth_the_hillbreaker_qiaoAI(Creature* creature) : ScriptedAI(creature)
		{
			uiEventTimer = 5000;
			Event = false;
		}

		uint32 uiEventTimer;
		bool Event;

		void Reset()
		{
			me->SetReactState(REACT_PASSIVE);
		}

		void MovementInform(uint32 type, uint32 id)
		{
			if (type != POINT_MOTION_TYPE)
				return;

			if (id == POINT_BANNER)
				me->GetMotionMaster()->MoveTargetedHome();
		}

		void UpdateAI(uint32 const diff)
		{
			if (Event)
			{
				if (uiEventTimer <= diff)
				{
					Event = false;
					uiEventTimer = 8000;
					me->MonsterYell("You puny thief! Koroth find you! Koroth smash your face in!", 0, 0);
				}
				else
					uiEventTimer -= diff;
			}

			if (!UpdateVictim())
				return;

			DoMeleeAttackIfReady();
		}
	};
};

class go_koroth_banner : public GameObjectScript
{
public:
	go_koroth_banner() : GameObjectScript("go_koroth_banner") { }

	bool OnGossipHello(Player* player, GameObject* go)
	{
		if (player->GetQuestStatus(QUEST_INTRODUCTIONS_ARE_IN_ORDER) == QUEST_STATUS_INCOMPLETE)
			if (Creature* koroth = go->FindNearestCreature(NPC_KOROTH_THE_HILLBREAKER_QIAO, 30.0f))
			{
				koroth->MonsterYell("Who dares to touch Koroth's banner!?!", 0, 0);
				float x, y;
				go->GetNearPoint2D(x, y, 5.0f, go->GetOrientation() + M_PI / 2);
				koroth->GetMotionMaster()->MovePoint(POINT_BANNER, x, y, go->GetPositionZ());
				CAST_AI(npc_koroth_the_hillbreaker_qiao::npc_koroth_the_hillbreaker_qiaoAI, koroth->AI())->Event = true;
			}

		return false;
	}
};

class npc_koroth_the_hillbreaker : public CreatureScript
{
public:
	npc_koroth_the_hillbreaker() : CreatureScript("npc_koroth_the_hillbreaker") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_koroth_the_hillbreakerAI(creature);
	}

	struct npc_koroth_the_hillbreakerAI : public ScriptedAI
	{
		npc_koroth_the_hillbreakerAI(Creature* creature) : ScriptedAI(creature) { }

		void UpdateAI(const uint32 /*diff*/)
		{
			if (!UpdateVictim())
				return;

			DoMeleeAttackIfReady();
		}
	};
};

//Liberation Day
// HACK! Gameobject does not work..
class npc_liberations_day : public CreatureScript
{
public:
	npc_liberations_day() : CreatureScript("npc_liberations_day") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_liberations_dayAI(creature);
	}

	struct npc_liberations_dayAI : public ScriptedAI
	{
		npc_liberations_dayAI(Creature* creature) : ScriptedAI(creature) 
		{ 
			uiEventTimer = 3000;
		}

		uint32 uiEventTimer;

		void MoveInLineOfSight(Unit* who)
		{
			if (who->GetTypeId() == TYPEID_PLAYER)
			{
				if (who->GetDistance(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ()) < 4.0f)
				{
					if (who->ToPlayer()->HasItemCount(49881, 1)) // Check for a slaver key
					{
						if (who->ToPlayer()->GetQuestStatus(24575) != QUEST_STATUS_COMPLETE)
						{
							who->ToPlayer()->KilledMonsterCredit(37694, 0);
							uiEventTimer = urand(3000, 5000);
						}
					}
				}
			}
		}

		void UpdateAI(uint32 const diff)
		{
			if (!UpdateVictim())
				return;

			if (uiEventTimer <= diff)
			{
				me->MonsterSay("The forsaken will pay for what they've done!", 0, 0);
				me->DespawnOrUnsummon();
			}
			else
				uiEventTimer -= diff;

			DoMeleeAttackIfReady();
		}
	};
};

class npc_lorna_crowley_livery_outpost : public CreatureScript
{
public:
	npc_lorna_crowley_livery_outpost() : CreatureScript("npc_lorna_crowley_livery_outpost") { }

private:
	bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
	{
		enum
		{
			NPC_KRENNAN_ARANAS = 38553,
			TALK_TIME_TO_START_BATTLE = 1,
		};

		if (quest->GetQuestId() == 24904)
		{
			// Temp Fix so worgens can complete there area
			player->TeleportTo(1, 10286.23f, 2440.69f, 1330.24f, 3.7f);
			player->AddQuest(sObjectMgr->GetQuestTemplate(26385), NULL);
			player->SetPhaseMask(1, true);
			player->RemoveActiveQuest(24904);
			player->UpdateForQuestWorldObjects();
			player->SaveToDB();
		}

		return false;
	}
};

void AddSC_gilneas()
{
    new npc_gilneas_city_guard_phase1();
    new npc_prince_liam_greymane_phase1();
    new npc_gilneas_city_guard_phase2();
    new npc_prince_liam_greymane_phase2();
    new npc_gwen_armstead_p2();
    new npc_rampaging_worgen();
    new npc_rampaging_worgen2();
    new go_merchant_square_door();
    new npc_sergeant_cleese();
    new npc_bloodfang_worgen();
    new npc_frightened_citizen();
    new npc_gilnean_royal_guard();
    new npc_mariam_spellwalker();
    new npc_sean_dempsey();
    new npc_lord_darius_crowley_c1();
    new npc_worgen_runt_c1();
    new npc_worgen_alpha_c1();
    new npc_worgen_runt_c2();
    new npc_worgen_alpha_c2();
    new npc_josiah_avery_p2();
    new npc_josiah_avery_trigger();
    new npc_lorna_crowley_p4();
    new npc_gilnean_mastiff();
    new npc_bloodfang_lurker();
    new npc_king_genn_greymane_p4();
    new npc_gilneas_city_guard_p8();
    new npc_afflicted_gilnean_p8();
    new npc_lord_darius_crowley_c3();
    new npc_king_genn_greymane_c2();
    new npc_crowley_horse();
    new spell_keg_placed();
    new npc_greymane_horse();
    new npc_krennan_aranas_c2();
    new npc_lord_godfrey_p4_8();
    new npc_commandeered_cannon();
    new npc_bloodfang_stalker_c1();
    new npc_gilnean_crow();
	//Duskhaven
	new go_mandragore();
	new npc_slain_watchman();
	new npc_gwen_armstead_phase_6();
	new npc_horrid_abomination();
	new npc_prince_liam_greymane_dh();
	new npc_forsaken_catapult_qtbs();
	new npc_james();
	new npc_ashley();
	new npc_cynthia();
	new npc_dark_ranger_thyala();
    new spell_call_attack_mastiffs();
	new npc_attack_mastiff();
	new npc_lord_godfrey_phase_7();
	new npc_drowning_watchman();
	new npc_prince_liam_greymane();
	new npc_round_up_horse();
	new npc_mountain_horse_vehicle();
	new spell_round_up_horse();
	new npc_lorna_ettins();
	new npc_gwen_armstead();
	new npc_swift_mountain_horse();
	new npc_king_genn_greymane_c3();
	new npc_koroth_the_hillbreaker();
	new go_koroth_banner();
	new npc_koroth_the_hillbreaker_qiao();
	new npc_prince_liam_greymane_qiao();
	new npc_captain_asther_qiao();
	new npc_koroth_the_hillbreaker_qiao_friend();
	new npc_liberations_day();
	new npc_lorna_crowley_livery_outpost();
}
