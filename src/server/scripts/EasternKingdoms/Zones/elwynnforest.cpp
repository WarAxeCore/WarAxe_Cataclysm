/*
 * Copyright (C) 2011-2019 Project SkyFire <http://www.projectskyfire.org/>
 * Copyright (C) 2008-2019 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2006-2019 ScriptDev2 <https://github.com/scriptdev2/scriptdev2/>
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
SDName: Elwynn_Forest
SD%Complete: 50
SDComment: Quest support: 1786
SDCategory: Elwynn Forest
EndScriptData */

/* ContentData
npc_henze_faulk
npc_blackrock_spy
npc_blackrock_invader
npc_stormwind_infantry
npc_blackrock_battle_worg
npc_goblin_assassin
EndContentData */

#include "ScriptPCH.h"

enum Northshire
{
    SAY_BLACKROCK_COMBAT_1    = -1000015,
    SAY_BLACKROCK_COMBAT_2    = -1000016,
    SAY_BLACKROCK_COMBAT_3    = -1000017,
    SAY_BLACKROCK_COMBAT_4    = -1000018,
    SAY_BLACKROCK_COMBAT_5    = -1000019,
    SAY_ASSASSIN_COMBAT_1     = -1000020,
    SAY_ASSASSIN_COMBAT_2     = -1000021,
    SPELL_SPYING              = 92857,
    SPELL_SNEAKING            = 93046,
    SPELL_SPYGLASS            = 80676,
    NPC_BLACKROCK_BATTLE_WORG = 49871,      //Blackrock Battle Worg NPC ID
    NPC_STORMWIND_INFANTRY    = 49869,      //Stormwind Infantry NPC ID
    WORG_FIGHTING_FACTION     = 232,        //Faction used by worgs to be able to attack infantry
    WORG_FACTION_RESTORE      = 32,         //Default Blackrock Battle Worg Faction
    WORG_GROWL                = 2649,       //Worg Growl Spell
    AI_HEALTH_MIN             = 85,         //Minimum health for AI staged fight between Blackrock Battle Worgs and Stormwind Infantry
    SAY_INFANTRY_YELL         = 1,          //Stormwind Infantry Yell phrase from Group 1
    INFANTRY_YELL_CHANCE      = 10           //% Chance for Stormwind Infantry to Yell - May need further adjustment... should be low chance
};

/*######
## npc_henze_faulk
######*/
enum eHenzeFaulkData
{
    SAY_HEAL = -1000187,
};

class npc_henze_faulk : public CreatureScript
{
public:
    npc_henze_faulk() : CreatureScript("npc_henze_faulk") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_henze_faulkAI (creature);
    }

    struct npc_henze_faulkAI : public ScriptedAI
    {
        uint32 lifeTimer;
        bool spellHit;

        npc_henze_faulkAI(Creature* creature) : ScriptedAI(creature) {}

        void Reset()
        {
            lifeTimer = 120000;
            me->SetUInt32Value(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);
            me->SetStandState(UNIT_STAND_STATE_DEAD);   // lay down
            spellHit = false;
        }

        void EnterCombat(Unit* /*who*/)
        {
        }

        void MoveInLineOfSight(Unit* /*who*/)
        {
        }

        void UpdateAI(const uint32 diff)
        {
            if (me->IsStandState())
            {
                if (lifeTimer <= diff)
                {
                    EnterEvadeMode();
                    return;
                }
                else
                    lifeTimer -= diff;
            }
        }

        void SpellHit(Unit* /*Hitter*/, const SpellEntry* Spellkind)
        {
            if (Spellkind->Id == 8593 && !spellHit)
            {
                DoCast(me, 32343);
                me->SetStandState(UNIT_STAND_STATE_STAND);
                me->SetUInt32Value(UNIT_DYNAMIC_FLAGS, 0);
                //me->RemoveAllAuras();
                DoScriptText(SAY_HEAL, me);
                spellHit = true;
            }
        }
    };
};

/*######
## npc_blackrock_spy
######*/

class npc_blackrock_spy : public CreatureScript
{
public:
    npc_blackrock_spy() : CreatureScript("npc_blackrock_spy") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_blackrock_spyAI (creature);
    }

    struct npc_blackrock_spyAI : public ScriptedAI
    {
        npc_blackrock_spyAI(Creature* creature) : ScriptedAI(creature)
        {
            CastSpying();
        }

        void CastSpying()
        {
            GetCreature(-8868.88f, -99.1016f);
            GetCreature(-8936.5f, -246.743f);
            GetCreature(-8922.44f, -73.9883f);
            GetCreature(-8909.68f, -40.0247f);
            GetCreature(-8834.85f, -119.701f);
            GetCreature(-9022.08f, -163.965f);
            GetCreature(-8776.55f, -79.158f);
            GetCreature(-8960.08f, -63.767f);
            GetCreature(-8983.12f, -202.827f);
        }

        void GetCreature(float X, float Y)
        {
            if (me->GetHomePosition().GetPositionX() == X, me->GetHomePosition().GetPositionY() == Y)
                if (!me->isInCombat() && !me->HasAura(SPELL_SPYING))
                    DoCast(me, SPELL_SPYING);

            CastSpyglass();
        }

        void CastSpyglass()
        {
            Spyglass(-8868.88f, -99.1016f, -8936.5f, -246.743f, -8922.44f, -73.9883f, -8909.68f, -40.0247f, -8834.85f,
                -119.701f, -9022.08f, -163.965f, -8776.55f, -79.158f, -8960.08f, -63.767f, -8983.12f, -202.827f);
        }

        void Spyglass(float X1, float Y1, float X2, float Y2, float X3, float Y3, float X4, float Y4, float X5, float Y5,
            float X6, float Y6, float X7, float Y7, float X8, float Y8, float X9, float Y9)
        {
            if (me->GetHomePosition().GetPositionX() != X1, me->GetHomePosition().GetPositionY() != Y1)
            if (me->GetHomePosition().GetPositionX() != X2, me->GetHomePosition().GetPositionY() != Y2)
            if (me->GetHomePosition().GetPositionX() != X3, me->GetHomePosition().GetPositionY() != Y3)
            if (me->GetHomePosition().GetPositionX() != X4, me->GetHomePosition().GetPositionY() != Y4)
            if (me->GetHomePosition().GetPositionX() != X5, me->GetHomePosition().GetPositionY() != Y5)
            if (me->GetHomePosition().GetPositionX() != X6, me->GetHomePosition().GetPositionY() != Y6)
            if (me->GetHomePosition().GetPositionX() != X7, me->GetHomePosition().GetPositionY() != Y7)
            if (me->GetHomePosition().GetPositionX() != X8, me->GetHomePosition().GetPositionY() != Y8)
            if (me->GetHomePosition().GetPositionX() != X9, me->GetHomePosition().GetPositionY() != Y9)
                if (me->GetHomePosition().GetPositionX() == me->GetPositionX(), me->GetHomePosition().GetPositionY() == me->GetPositionY())
                    if (!me->isInCombat() && !me->HasAura(SPELL_SPYGLASS))
                        DoCast(me, SPELL_SPYGLASS);
        }

        void EnterCombat(Unit* who)
        {
            DoScriptText(RAND(SAY_BLACKROCK_COMBAT_1, SAY_BLACKROCK_COMBAT_2, SAY_BLACKROCK_COMBAT_3, SAY_BLACKROCK_COMBAT_4, SAY_BLACKROCK_COMBAT_5), me);
        }

        void UpdateAI(const uint32 /*diff*/)
        {
            CastSpyglass();

            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }
    };
};

/*######
## npc_blackrock_invader
######*/

class npc_blackrock_invader : public CreatureScript
{
public:
    npc_blackrock_invader() : CreatureScript("npc_blackrock_invader") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_blackrock_invaderAI (creature);
    }

    struct npc_blackrock_invaderAI : public ScriptedAI
    {
        npc_blackrock_invaderAI(Creature* creature) : ScriptedAI(creature) {}

        void EnterCombat(Unit* who)
        {
            DoScriptText(RAND(SAY_BLACKROCK_COMBAT_1, SAY_BLACKROCK_COMBAT_2, SAY_BLACKROCK_COMBAT_3, SAY_BLACKROCK_COMBAT_4, SAY_BLACKROCK_COMBAT_5), me);
        }

        void UpdateAI(const uint32 /*diff*/)
        {
            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }
    };
};

/*######
## npc_goblin_assassin
######*/

class npc_goblin_assassin : public CreatureScript
{
public:
    npc_goblin_assassin() : CreatureScript("npc_goblin_assassin") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_goblin_assassinAI (creature);
    }

    struct npc_goblin_assassinAI : public ScriptedAI
    {
        npc_goblin_assassinAI(Creature* creature) : ScriptedAI(creature)
        {
            if (!me->isInCombat() && !me->HasAura(SPELL_SPYING))
                DoCast(SPELL_SNEAKING);
        }

        void EnterCombat(Unit* who)
        {
            DoScriptText(RAND(SAY_ASSASSIN_COMBAT_1, SAY_ASSASSIN_COMBAT_2), me);
        }

        void UpdateAI(const uint32 /*diff*/)
        {
            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }
    };
};

/*######
## npc_stormwind_infantry
######*/

class npc_stormwind_infantry : public CreatureScript
{
public:
    npc_stormwind_infantry() : CreatureScript("npc_stormwind_infantry") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_stormwind_infantryAI (creature);
    }

    struct npc_stormwind_infantryAI : public ScriptedAI
    {
        npc_stormwind_infantryAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 tSeek, cYell,tYell;

        void Reset()
        {
            tSeek=urand(1000,2000);
            cYell=urand(0, 100);
            tYell=urand(5000, 60000);
        }

        void DamageTaken(Unit* who, uint32& damage)
        {
            if (who->GetTypeId() == TYPEID_PLAYER)//If damage taken from player
            {
                me->getThreatManager().resetAllAggro();
                who->AddThreat(me, 1.0f);
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
            }
            else if (who->isPet())//If damage taken from pet
            {
                me->getThreatManager().resetAllAggro();
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
            }
            if (who->GetEntry() == NPC_BLACKROCK_BATTLE_WORG && me->HealthBelowPct(AI_HEALTH_MIN))//If damage taken from Blackrock Battle Worg
            {
                damage = 0;//We do not want to do damage if Blackrock Battle Worg is below preset HP level (currently 85% - Blizzlike)
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;
            else
            {
                if (tYell <= diff)//Chance to yell every 5 to 10 seconds
                {
                    if (cYell < INFANTRY_YELL_CHANCE)//Roll for random chance to Yell phrase
                    {
                        me->AI()->Talk(SAY_INFANTRY_YELL); //Yell phrase
                        tYell=urand(10000, 120000);//After First yell, change time range from 10 to 120 seconds
                    }
                    else
                        tYell=urand(10000, 120000);//From 10 to 120 seconds
                }
                else
                {
                    tYell -= diff;
                    DoMeleeAttackIfReady(); //Else do standard attack
                }
            }
        }
    };
};

/*######
## npc_blackrock_battle_worg
######*/

class npc_blackrock_battle_worg : public CreatureScript
{
public:
    npc_blackrock_battle_worg() : CreatureScript("npc_blackrock_battle_worg") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_blackrock_battle_worgAI (creature);
    }

    struct npc_blackrock_battle_worgAI : public ScriptedAI
    {
        npc_blackrock_battle_worgAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 tSeek, tGrowl;

        void Reset()
        {
            tSeek=urand(1000,2000);
            tGrowl=urand(8500,10000);
            me->setFaction(WORG_FACTION_RESTORE);//Restore our faction on reset
        }

        void DamageTaken(Unit* who, uint32& damage)
        {
            if (who->GetTypeId() == TYPEID_PLAYER)//If damage taken from player
            {
                me->getThreatManager().resetAllAggro();
                who->AddThreat(me, 1.0f);
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
            }
            else if (who->isPet())//If damage taken from pet
            {
                me->getThreatManager().resetAllAggro();
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
            }
            if (who->GetEntry() == NPC_STORMWIND_INFANTRY && me->HealthBelowPct(AI_HEALTH_MIN))//If damage taken from Stormwind Infantry
            {
                damage = 0;//We do not want to do damage if Stormwind Infantry is below preset HP level (currently 85% - Blizzlike)
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (tSeek <= diff)
            {
                if ((me->isAlive()) && (!me->isInCombat() && (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) <= 1.0f)))
                    if (Creature* enemy = me->FindNearestCreature(NPC_STORMWIND_INFANTRY,1.0f, true))
                    {
                        me->setFaction(WORG_FIGHTING_FACTION);//We must change our faction to one which is able to attack Stormwind Infantry (Faction 232 works well)
                        me->AI()->AttackStart(enemy);
                        tSeek = urand(1000,2000);
                    }
            }
            else
                tSeek -= diff;

            if (UpdateVictim())
            {
                if (tGrowl <=diff)
                {
                    DoCast(me->getVictim(), WORG_GROWL);//Do Growl if ready
                    tGrowl=urand(8500,10000);
                }
                else
                {
                   tGrowl -= diff;
                   DoMeleeAttackIfReady();//Else do standard attack
                }
            }
            else
            {
                me->setFaction(WORG_FACTION_RESTORE);//Reset my faction if not in combat
                return;
            }
        }
    };
};

/*################
npc_hogger#
################*/
enum Spells
{
	SPELL_EATING = 87351,
	SPELL_VICIOUS_SLICE = 87337,
	SPELL_SUMMON_MINIONS = 87366,
	SPELL_SIMPLY_TELEPORT = 64446,
};

class npc_hogger : public CreatureScript
{
public:
	npc_hogger() : CreatureScript("npc_hogger") { }

	CreatureAI* GetAI(Creature* pCreature) const
	{
		return new npc_hoggerAI(pCreature);
	}

	struct npc_hoggerAI : public ScriptedAI
	{
		uint8 phase;

		uint32 m_uiViciousSliceTimer;
		uint32 m_uiSummonMinions;
		uint32 m_uiPhaseTimer;
		uint32 m_uiDespawnTimer;

		uint64 GeneralGUID;
		uint64 Mage1GUID;
		uint64 Mage2GUID;
		uint64 Raga1GUID;
		uint64 Raga2GUID;
		uint64 PlayerGUID;

		bool bSay;
		bool bSay2;
		bool bSay3;
		bool Summon;
		bool bSummoned;
		bool bSummoned3;
		bool bGo1;
		bool bCasted;
		bool Credit;

		npc_hoggerAI(Creature *c) : ScriptedAI(c) {}

		void Reset()
		{
			me->RestoreFaction();
			me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

			m_uiViciousSliceTimer = 8000;
			m_uiSummonMinions = 15000;
			m_uiPhaseTimer = 0;
			m_uiDespawnTimer = 500;

			phase = 0;
			GeneralGUID = 0;
			Mage1GUID = 0;
			Mage2GUID = 0;
			Raga1GUID = 0;
			Raga2GUID = 0;
			PlayerGUID = 0;

			Summon = false;
			bSay = false;
			bCasted = false;
			bSay2 = false;
			bSay3 = false;
			bSummoned = false;
			bSummoned3 = false;
			bGo1 = false;
			Credit = false;
		}

		void JustDied(Unit* /*killer*/)
		{
			me->RestoreFaction();
		}

		void AttackedBy(Unit* pAttacker)
		{
			if (me->getVictim() || me->IsFriendlyTo(pAttacker))
				return;

			AttackStart(pAttacker);
		}

		void MovementInform(uint32 type, uint32 id)
		{
			if (id == 0)
			{
				DoCast(me, SPELL_EATING);
			}
		}

		void UpdateAI(const uint32 diff)
		{
			ScriptedAI::UpdateAI(diff);

			DoMeleeAttackIfReady();

			if (me->GetAreaId() != 5174) // Not in hogger hill anymore run back home.
			{
				me->CombatStop(true);
			}

			if (m_uiViciousSliceTimer <= diff)
			{
				DoCast(me->getVictim(), SPELL_VICIOUS_SLICE);
				m_uiViciousSliceTimer = 10000;
			}
			else
				m_uiViciousSliceTimer -= diff;

			if (HealthBelowPct(50))
			{
				if (!bSummoned)
				{
					DoCast(me->getVictim(), SPELL_SUMMON_MINIONS);
					bSummoned = true;
				}
				if (!bSay)
				{
					me->MonsterYell("Yipe! Help Hogger", 0, NULL);
					bSay = true;
				}
			}

			if (HealthBelowPct(35))
			{
				if (!bSay2)
				{
					me->MonsterTextEmote("Hogger is eating! Stop him!", NULL, true);
					bSay2 = true;
				}
				if (!bGo1)
				{
					me->AttackStop();
					me->SetReactState(REACT_PASSIVE);
					me->GetMotionMaster()->MovePoint(0, -10142.081f, 671.773f, 36.014f);
					bGo1 = true;
				}
			}

			if (HealthBelowPct(10))
			{
				if (!bSummoned3)
				{
					if (!bSay3)
					{
						me->MonsterYell("No hurt Hogger!", 0, NULL);
						bSay3 = true;
					}

					me->CombatStop(true);
					me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

					Creature* General = me->SummonCreature(46942, -10133.275f, 663.244f, 35.964616f, 2.45f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 180000);
					GeneralGUID = General->GetGUID();

					Creature* Mage1 = me->SummonCreature(46941, -10129.976f, 667.982f, 35.67f, 2.85f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 180000);
					Mage1GUID = Mage1->GetGUID();

					Creature* Mage2 = me->SummonCreature(46940, -10137.671f, 659.926f, 35.971f, 2.051f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 180000);
					Mage2GUID = Mage2->GetGUID();

					Creature* Raga1 = me->SummonCreature(46943, -10133.339f, 660.087f, 35.971f, 2.26f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 180000);
					Raga1GUID = Raga1->GetGUID();

					Creature* Raga2 = me->SummonCreature(42413, -10129.461f, 663.180f, 35.9491f, 2.37f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 180000);
					Raga2GUID = Raga2->GetGUID();

					Summon = true;

					m_uiPhaseTimer = 1500;
					phase = 1;
				}

				if (Summon)
				{
					Creature* General = Unit::GetCreature(*me, GeneralGUID);
					Creature* Raga2 = Unit::GetCreature(*me, Raga2GUID);
					Creature* Raga1 = Unit::GetCreature(*me, Raga1GUID);
					Creature* Mage2 = Unit::GetCreature(*me, Mage2GUID);
					Creature* Mage1 = Unit::GetCreature(*me, Mage1GUID);

					if (!bCasted)
					{
						General->CastSpell(General, 64446, true);
						Raga2->CastSpell(Raga2, 64446, true);
						Raga1->CastSpell(Raga1, 64446, true);
						Mage2->CastSpell(Raga2, 64446, true);
						Mage1->CastSpell(Raga1, 64446, true);
						bCasted = true;
					}

					bSummoned3 = true;
					me->SetSpeed(MOVE_RUN, 1.0f);
				}
			}

			if (bCasted)
			{
				if (m_uiPhaseTimer <= diff)
				{
					Creature* General = Unit::GetCreature(*me, GeneralGUID);
					Creature* Raga2 = Unit::GetCreature(*me, Raga2GUID);
					Creature* Raga1 = Unit::GetCreature(*me, Raga1GUID);
					Creature* Mage2 = Unit::GetCreature(*me, Mage2GUID);
					Creature* Mage1 = Unit::GetCreature(*me, Mage1GUID);

					switch (phase)
					{

					case 1: me->GetMotionMaster()->MovePoint(1, -10141.054f, 670.719f, 35.9569f); m_uiPhaseTimer = 3000; phase = 2; break;
					case 2: General->MonsterYell("Hold your blade, adventurer!", 0, NULL); m_uiPhaseTimer = 2500; phase = 3; break;
					case 3: Raga1->MonsterSay("General Jonathan Marcus!", 0, NULL); m_uiPhaseTimer = 1500; phase = 4; break;
					case 4: Raga2->MonsterSay("Wow!", 0, NULL); m_uiPhaseTimer = 1500; phase = 5; break;
					case 5:
					{
						if (Creature* General = Unit::GetCreature(*me, GeneralGUID))
						{
							General->MonsterSay("This beast leads the Riverpaw gnoll gang and may be the key to ending gnoll aggression in Elwynn!", 0, NULL);
							General->AddUnitMovementFlag(MOVEMENTFLAG_WALKING);
							General->GetMotionMaster()->MovePoint(0, -10137.162f, 667.919f, 35.937f);
							m_uiPhaseTimer = 3000;
						}
						phase = 6;
					} break;
					case 6: me->MonsterSay("Grrr...", 0, NULL); m_uiPhaseTimer = 4000; phase = 7; break;
					case 7:
					{
						General->HandleEmoteCommand(EMOTE_ONESHOT_POINT);
						m_uiPhaseTimer = 4500;
						phase = 8;
					} break;

					case 8: General->MonsterSay("We're taking him into custody in the name of King Varian Wrynn.", 0, NULL); m_uiPhaseTimer = 4000; phase = 9; break;
					case 9: me->MonsterSay("Nooooo...", 0, NULL); m_uiPhaseTimer = 4000; phase = 10; break;
					case 10: General->MonsterSay("Take us to the Stockades, Andromath.", 0, NULL); m_uiPhaseTimer = 4000; phase = 11; break;
						General->SetOrientation(6.08f);
					case 11:
					{
						General->CastSpell(General, 64446, true);
						Raga1->CastSpell(Raga1, 64446, true);
						Raga2->CastSpell(Raga2, 64446, true);
						Mage1->CastSpell(Mage1, 64446, true);
						Mage2->CastSpell(Mage2, 64446, true);
						me->CastSpell(me, 64446, true);

						General->DespawnOrUnsummon(500);
						Raga1->DespawnOrUnsummon(500);
						Raga2->DespawnOrUnsummon(500);
						Mage1->DespawnOrUnsummon(500);
						Mage2->DespawnOrUnsummon(500);
						me->DespawnOrUnsummon(500);

						std::list<Player*> players;

						SkyFire::AnyPlayerInObjectRangeCheck checker(me, 35.0f);
						SkyFire::PlayerListSearcher<SkyFire::AnyPlayerInObjectRangeCheck> searcher(me, players, checker);
						me->VisitNearbyWorldObject(35.0f, searcher);

						for (std::list<Player*>::const_iterator itr = players.begin(); itr != players.end(); ++itr)
							(*itr)->KilledMonsterCredit(448, NULL);

						phase = 0;
					} break;
					default: break;
					}
				}
				else m_uiPhaseTimer -= diff;
			}
			if (me->GetHealth() <= 1)
			{
				if (m_uiDespawnTimer <= diff)
				{
					me->CombatStop(true);
					me->AttackStop();
					me->ClearAllReactives();
					me->DeleteThreatList();
					me->SetHealth(me->GetMaxHealth());
				}
				else m_uiDespawnTimer -= diff;
			}
		}
		void DamageTaken(Unit* done_by, uint32 & damage)
		{
			if (PlayerGUID == 0)
			{
				if (Player *pPlayer = done_by->ToPlayer())
				{
					PlayerGUID = pPlayer->GetGUID();
				}
			}

			if (me->GetHealth() <= damage)
			{
				damage = me->GetHealth() - 1;

				if (Credit == false)
				{
					me->RemoveAllAuras();
					me->CombatStop(true);
					me->AttackStop();
					me->ClearAllReactives();
					me->DeleteThreatList();

				}Credit = true;
			}
		}
	};
};

void AddSC_elwynn_forest()
{
    new npc_henze_faulk();
    new npc_blackrock_spy();
    new npc_goblin_assassin();
    new npc_blackrock_invader();
    new npc_stormwind_infantry();
    new npc_blackrock_battle_worg();
	new npc_hogger();
}
