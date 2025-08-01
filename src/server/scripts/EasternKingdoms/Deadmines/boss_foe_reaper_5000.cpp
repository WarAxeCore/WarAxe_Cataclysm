/*
*
* Copyright (C) 2012-2014 Cerber Project <https://bitbucket.org/mojitoice/>
*
*/

#include "ScriptMgr.h"
#include "deadmines.h"
#include "Object.h"
#include "MapManager.h"

enum eSpell
{
	SPELL_ENERGIZE = 89132,
	SPELL_ENERGIZED = 91733, // -> 89200,
	SPELL_ON_FIRE = 91737,
	SPELL_COSMETIC_STAND = 88906,

	// BOSS spells
	SPELL_OVERDRIVE = 88481, // 88484
	SPELL_HARVEST = 88495,
	SPELL_HARVEST_AURA = 88497,

	SPELL_HARVEST_SWEEP = 88521,
	SPELL_HARVEST_SWEEP_H = 91718,

	SPELL_REAPER_STRIKE = 88490,
	SPELL_REAPER_STRIKE_H = 91717,

	SPELL_SAFETY_REST_OFFLINE = 88522,
	SPELL_SAFETY_REST_OFFLINE_H = 91720,

	SPELL_SUMMON_MOLTEN_SLAG = 91839,
};

enum eAchievementMisc
{
	ACHIEVEMENT_PROTOTYPE_PRODIGY = 5368,
	DATA_ACHIV_PROTOTYPE_PRODIGY = 1,
};

const Position OverdrivePoint =
{
	-184.3978f, -565.997f, 19.30717f, 0.0f
};

enum Events
{
	EVENT_NULL,
	EVENT_START,
	EVENT_START_2,
	EVENT_SRO,
	EVENT_OVERDRIVE,
	EVENT_HARVEST,
	EVENT_HARVEST_SWEAP,
	EVENT_REAPER_STRIKE,
	EVENT_SAFETY_OFFLINE,
	EVENT_SWITCH_TARGET,
	EVENT_MOLTEN_SLAG,
	EVENT_START_ATTACK,
};

#define MONSTER_START "A stray jolt from the Foe Reaper has distrupted the foundry controls!"
#define MONSTER_SLAG "The monster slag begins to bubble furiously!"


Position const HarvestSpawn[] =
{
	{-229.72f, -590.37f, 19.38f, 0.71f},
	{-229.67f, -565.75f, 19.38f, 5.98f},
	{-205.53f, -552.74f, 19.38f, 4.53f},
	{-182.74f, -565.96f, 19.38f, 3.35f},
};

Position const PrototypeSpawn = { -200.499f, -553.946f, 51.2295f, 4.32651f };


class boss_foe_reaper_5000 : public CreatureScript
{
public:
	boss_foe_reaper_5000() : CreatureScript("boss_foe_reaper_5000")
	{}

	CreatureAI* GetAI(Creature* creature) const
	{
		return new boss_foe_reaper_5000AI(creature);
	}

	struct boss_foe_reaper_5000AI : public BossAI
	{
		boss_foe_reaper_5000AI(Creature* creature) : BossAI(creature, DATA_FOEREAPER)
		{
			me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_STUNNED);
			prototypeGUID = 0;
		}

		uint32 eventId;
		uint32 Step;
		uint64 prototypeGUID;

		bool Below;

		void Reset()
		{
			if (!me)
				return;

			_Reset();
			me->SetReactState(REACT_PASSIVE);
			me->SetPower(POWER_ENERGY, 100);
			me->SetMaxPower(POWER_ENERGY, 100);
			me->setPowerType(POWER_ENERGY);
			me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_PC);
			Step = 0;
			Below = false;

			me->SetFullHealth();
			me->SetOrientation(4.273f);

			DespawnOldWatchers();
			RespawnWatchers();

			if (IsHeroic())
			{
				if (Creature* Reaper = me->GetCreature(*me, prototypeGUID))
					Reaper->DespawnOrUnsummon();

				if (Creature *prototype = me->SummonCreature(NPC_PROTOTYPE_REAPER, PrototypeSpawn, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000))
				{
					prototype->SetFullHealth();
					prototypeGUID = prototype->GetGUID();
				}
			}
		}

		void EnterCombat(Unit* /*who*/)
		{
			_EnterCombat();
			events.ScheduleEvent(EVENT_REAPER_STRIKE, 10000);
			events.ScheduleEvent(EVENT_OVERDRIVE, 11000);
			events.ScheduleEvent(EVENT_HARVEST, 25000);

			instance->SetBossState(DATA_FOEREAPER, IN_PROGRESS);

			if (IsHeroic())
			{
				events.ScheduleEvent(EVENT_MOLTEN_SLAG, 15000);
			}

			if (!me)
				return;

		}

		void JustDied(Unit* /*Killer*/)
		{
			if (!me)
				return;

			_JustDied();
			DespawnOldWatchers();
			me->BossYell("Overheat threshold exceeded. System failure. Wheat clog in port two. Shutting down.", 22138);

			instance->SetBossState(DATA_FOEREAPER, DONE);

			if (IsHeroic())
			{			
				if (Creature* Reaper = me->GetCreature(*me, prototypeGUID))
				{
					Reaper->DespawnOrUnsummon();
				}
			}

			GameObject* go_door = me->FindNearestGameObject(GO_FOUNDRY_DOOR, 250.0f);
			if (go_door)
			{
				go_door->SetGoState(GO_STATE_ACTIVE);
			}
		}

		uint32 GetData(uint32 type) const
		{
			if (type == DATA_ACHIV_PROTOTYPE_PRODIGY)
			{
				if (!IsHeroic())
					return false;

				if (Creature *prototype_reaper = me->GetCreature(*me, prototypeGUID))
					if (prototype_reaper->GetHealth() >= 0.9 * prototype_reaper->GetMaxHealth())
						return true;
			}

			return false;
		}

		void JustReachedHome()
		{
			if (!me)
				return;

			_JustReachedHome();
			me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_STUNNED);
			instance->SetBossState(DATA_FOEREAPER, FAIL);
		}

		void DespawnOldWatchers()
		{
			std::list<Creature*> reapers;
			me->GetCreatureListWithEntryInGrid(reapers, NPC_DEFIAS_REAPER, 250.0f);

			reapers.sort(SkyFire::ObjectDistanceOrderPred(me));
			for (std::list<Creature*>::iterator itr = reapers.begin(); itr != reapers.end(); ++itr)
			{
				if ((*itr) && (*itr)->GetTypeId() == TYPEID_UNIT)
				{
					(*itr)->DespawnOrUnsummon();
				}
			}
		}

		void RespawnWatchers()
		{
			for (uint8 i = 0; i < 4; ++i)
			{
				me->SummonCreature(NPC_DEFIAS_REAPER, HarvestSpawn[i], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
			}
		}

		void MovementInform(uint32 /*type*/, uint32 id)
		{
			if (id == 0)
			{
				if (Creature* HarvestTarget = me->FindNearestCreature(NPC_HARVEST_TARGET, 200.0f, true))
				{
					//DoCast(HarvestTarget, IsHeroic() ? SPELL_HARVEST_SWEEP_H : SPELL_HARVEST_SWEEP);
					me->RemoveAurasDueToSpell(SPELL_HARVEST_AURA);
					events.ScheduleEvent(EVENT_START_ATTACK, 1000);
				}
			}
		}

		void HarvestChase()
		{
			if (Creature* HarvestTarget = me->FindNearestCreature(NPC_HARVEST_TARGET, 200.0f, true))
			{
				me->SetSpeed(MOVE_RUN, 3.0f, true);
				me->GetMotionMaster()->MoveCharge(HarvestTarget->GetPositionX(), HarvestTarget->GetPositionY(), HarvestTarget->GetPositionZ(), 5.0f, 0);
				HarvestTarget->DespawnOrUnsummon(8500);
			}
		}

		void DoAction(int32 const action)
		{
			switch (action)
			{
			case 1:
			{
				if (Step == 3)
				{
					events.ScheduleEvent(EVENT_START, 3000);
				}
				Step++;
				break;
			}
			default:
				break;
			}
		}

		void UpdateAI(uint32 const uiDiff)
		{
			if (!me)
				return;

			DoMeleeAttackIfReady();

			events.Update(uiDiff);

			while (uint32 eventId = events.ExecuteEvent())
			{
				switch (eventId)
				{
				case EVENT_START:
					me->BossYell("Foe Reaper 5000 on-line. All systems nominal.", 22137);
					me->AddAura(SPELL_ENERGIZED, me);
					me->MonsterTextEmote(MONSTER_START, 0, true);
					events.ScheduleEvent(EVENT_START_2, 5000);
					break;

				case EVENT_START_2:
					me->MonsterTextEmote(MONSTER_SLAG, 0, true);
					me->SetHealth(me->GetMaxHealth());
					DoZoneInCombat();
					me->SetReactState(REACT_AGGRESSIVE);
					me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
					me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
					me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
					me->RemoveAurasDueToSpell(SPELL_ENERGIZED);
					events.ScheduleEvent(EVENT_SRO, 1000);
					break;

				case EVENT_SRO:
					me->RemoveAurasDueToSpell(SPELL_OFFLINE);

					if (Player* victim = me->FindNearestPlayer(40.0f))
						me->Attack(victim, false);
					break;

				case EVENT_START_ATTACK:
					me->BossYell("Target acquired. Harvesting servos engaged.", 22141);
					me->RemoveAurasDueToSpell(SPELL_HARVEST_AURA);
					me->SetSpeed(MOVE_RUN, 2.0f, true);
					if (Player* victim = me->FindNearestPlayer(40.0f))
						me->Attack(victim, true);
					break;

				case EVENT_OVERDRIVE:
					if (!UpdateVictim())
						return;

					me->MonsterTextEmote("|TInterface\\Icons\\ability_whirlwind.blp:20|tFoe Reaper 5000 begins to activate |cFFFF0000|Hspell:91716|h[Overdrive]|h|r!", 0, true);
					me->AddAura(SPELL_OVERDRIVE, me);
					me->SetSpeed(MOVE_RUN, 4.0f, true);
					events.ScheduleEvent(EVENT_SWITCH_TARGET, 1500);
					events.ScheduleEvent(EVENT_OVERDRIVE, 45000);
					break;

				case EVENT_SWITCH_TARGET:
					if (Unit* victim = SelectTarget(SELECT_TARGET_RANDOM, 0, 150, true))
						me->Attack(victim, true);

					if (me->HasAura(SPELL_OVERDRIVE))
					{
						events.ScheduleEvent(EVENT_SWITCH_TARGET, 1500);
					}
					break;

				case EVENT_HARVEST:
					if (!UpdateVictim())
						return;

					if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 150, true))
						me->CastSpell(target, SPELL_HARVEST);

					events.RescheduleEvent(EVENT_HARVEST_SWEAP, 5500);
					break;

				case EVENT_HARVEST_SWEAP:
					if (!UpdateVictim())
						return;

					HarvestChase();
					me->BossYell("Acquiring target...", 22140);
					events.ScheduleEvent(EVENT_START_ATTACK, 8000);
					events.RescheduleEvent(EVENT_HARVEST, 45000);
					break;

				case EVENT_REAPER_STRIKE:
					if (!UpdateVictim())
						return;

					if (Unit* victim = me->getVictim())
					{
						if (me->IsWithinDist(victim, 25.0f))
						{
							DoCast(victim, IsHeroic() ? SPELL_REAPER_STRIKE_H : SPELL_REAPER_STRIKE);
						}
					}
					events.ScheduleEvent(EVENT_REAPER_STRIKE, urand(9000, 12000));
					break;

				case EVENT_MOLTEN_SLAG:
					me->MonsterTextEmote(MONSTER_SLAG, 0, true);
					me->CastSpell(-213.21f, -576.85f, 20.97f, SPELL_SUMMON_MOLTEN_SLAG, false);
					events.ScheduleEvent(EVENT_MOLTEN_SLAG, 20000);
					break;

				case EVENT_SAFETY_OFFLINE:
					me->BossYell("Safety restrictions off-line. Catastrophic system failure imminent.", 22143);
					DoCast(me, IsHeroic() ? SPELL_SAFETY_REST_OFFLINE_H : SPELL_SAFETY_REST_OFFLINE);
					break;
				}

				if (HealthBelowPct(30) && !Below)
				{
					events.ScheduleEvent(EVENT_SAFETY_OFFLINE, 0);
					Below = true;
				}
			}
		}
	};
};

class npc_defias_watcher : public CreatureScript
{
public:
	npc_defias_watcher() : CreatureScript("npc_defias_watcher")
	{}

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_defias_watcherAI(creature);
	}

	struct npc_defias_watcherAI : public ScriptedAI
	{
		npc_defias_watcherAI(Creature* creature) : ScriptedAI(creature)
		{
			instance = creature->GetInstanceScript();
			Status = false;
		}

		InstanceScript* instance;
		bool Status;

		void Reset()
		{
			if (!me)
				return;

			me->SetPower(POWER_ENERGY, 100);
			me->SetMaxPower(POWER_ENERGY, 100);
			me->setPowerType(POWER_ENERGY);
			if (Status == true)
			{
				if (!me->HasAura(SPELL_ON_FIRE))
					me->AddAura(SPELL_ON_FIRE, me);
				me->setFaction(35);
			}
		}

		void EnterCombat(Unit* /*who*/)
		{}

		void JustDied(Unit* /*Killer*/)
		{
			if (!me || Status == true)
				return;

			Energizing();
		}

		void Energizing()
		{
			Status = true;
			me->SetHealth(15);
			me->setRegeneratingHealth(false);
			me->setFaction(35);
			me->AddAura(SPELL_ON_FIRE, me);
			me->CastSpell(me, SPELL_ON_FIRE);
			me->SetInCombatWithZone();

			if (Creature* reaper = me->FindNearestCreature(NPC_FOEREAPER, 200.0f))
			{
				reaper->AI()->DoAction(1);
				me->CastSpell(reaper, SPELL_ENERGIZE);
			}
		}

		void DamageTaken(Unit* /*attacker*/, uint32& damage)
		{
			if (!me || damage <= 0 || Status == true)
				return;

			if (me->GetHealth() - damage <= me->GetMaxHealth()*0.10)
			{
				damage = 0;
				Energizing();
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

class achievement_prototype_reaper : public AchievementCriteriaScript
{
public:
	achievement_prototype_reaper() : AchievementCriteriaScript("achievement_prototype_reaper")
	{}

	bool OnCheck(Player* source, Unit* target)
	{
		if (target && target->IsAIEnabled)
		{
			return target->GetAI()->GetData(DATA_ACHIV_PROTOTYPE_PRODIGY);
		}

		return false;
	}
};

void AddSC_boss_foe_reaper_5000()
{
	new npc_defias_watcher();
	new boss_foe_reaper_5000();
	new achievement_prototype_reaper();
}