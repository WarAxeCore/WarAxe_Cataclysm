/* Copyright (C) WoW Source 4.3.4 */

/* Name: boss_grand_vizier_ertan
* Progress: 100%
* Author: Demigodess
* Comments:
*/

#include "ScriptMgr.h"
#include "vortex_pinnacle.h"

enum Spells
{
	SPELL_LIGHTING_BOLT = 86331, // SPELL_EFFECT_SCHOOL_DAMAGE

	SPELL_STORMS_EDGE_AURA = 86295, // + SPELL_AURA_PERIODIC_TRIGGER_SPELL
	SPELL_STORMS_EDGE_PERIODIC = 86284, // +->(+)SPELL_EFFECT_SCRIPT_EFFECT

	SPELL_STORMS_EDGE_PULL = 86329, // SPELL_AURA_DUMMY
	SPELL_STORMS_EDGE_INNER = 86310, // + SPELL_AURA_PERIODIC_TRIGGER_SPELL
	SPELL_STORMS_EDGE_KNOCKBACK = 86311, // +->(+)SPELL_EFFECT_SCRIPT_EFFECT | SPELL_EFFECT_KNOCK_BACK

	SPELL_STORMS_EDGE = 86299, // +->(+)SPELL_EFFECT_SCRIPT_EFFECT
	SPELL_STORMS_EDGE_DAMAGE = 86309, // +-> SPELL_EFFECT_SCHOOL_DAMAGE

	SPELL_CYCLONE_SHIELD_PERIODIC = 86267, // + SPELL_AURA_PERIODIC_TRIGGER_SPELL
	SPELL_CYCLONE_SHIELD_TICK = 86292, // +-> SPELL_AURA_MOD_DECREASE_SPEED | SPELL_AURA_MELEE_SLOW | SPELL_EFFECT_SCHOOL_DAMAGE

	SPELL_SUMMON_TEMPEST = 86340, // SPELL_EFFECT_SUMMON (45704)
	SPELL_LURK = 85467,
	SPELL_TEMPEST_LIGHTING_BOLT = 92776
};

enum ErtanEvents
{
	EVENT_SUMMON_TEMPEST = 1,
	EVENT_PULL_TORNADO_SHIELD,
	EVENT_STORMS_EDGE
};

enum ErtanTimer
{
	TIMER_PULL_TORNADO_SHIELD = 24 * IN_MILLISECONDS, // sniffed

	TIMER_STORMS_EDGE_INNER = 3 * IN_MILLISECONDS,  // sniffed
	TIMER_STORMS_EDGE = 9 * IN_MILLISECONDS,  // sniffed

	TIMER_SUMMON_TEMPEST_MIN = 15 * IN_MILLISECONDS, // sniffed
	TIMER_SUMMON_TEMPEST_MAX = 25 * IN_MILLISECONDS  // sniffed
};

enum Actions
{
	ACTION_PULL = 1,
	ACTION_MOVE_OUT = 2
};

Position const TornadoPositions[] =
{
	{ -702.228f, -13.440f, 635.672f, 2.338f },
	{ -719.625f, -20.499f, 635.672f, 1.427f },
	{ -736.997f, -13.297f, 635.672f, 0.715f },
	{ -744.433f, 4.024f, 635.672f, 6.260f },
	{ -737.178f, 21.330f, 635.672f, 5.561f },
	{ -719.958f, 28.493f, 635.672f, 4.689f },
	{ -702.408f, 21.482f, 635.674f, 3.923f },
	{ -695.130f, 4.116f, 635.672f, 3.173f }
};

uint32 const NUM_POSITIONS = 8;
float const STORMS_EDGE_RADIUS_LARGE = 25.0f;
float const STORMS_EDGE_RADIUS_SMALL = 5.0f;

Position const CenterPosition = { -719.435f, 3.839f, 635.671f };

class boss_grand_vizier_ertan : public CreatureScript
{
public:
	boss_grand_vizier_ertan() : CreatureScript("boss_grand_vizier_ertan") {}

	struct boss_grand_vizier_ertanAI : public BossAI
	{
		boss_grand_vizier_ertanAI(Creature* creature) : BossAI(creature, BOSS_GRAND_VIZIER_ERTAN) { }

		void Reset()
		{
			BossAI::Reset();
			me->DespawnCreaturesInArea(NPC_LURKING_TEMPEST, 120.0f);
			me->DespawnCreaturesInArea(NPC_CYCLONE_SHIELD, 120.0f);

			instance->SendEncounterUnit(ENCOUNTER_FRAME_REMOVE, me);

			me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
			me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
			me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
		}

		void EnterCombat(Unit* who)
		{
			me->BossYell("Filthy Beasts! Your presence in Skywall will not be tolerated!", 20876);
			instance->SendEncounterUnit(ENCOUNTER_FRAME_ADD, me);
			DoCast(SPELL_STORMS_EDGE_AURA);

			events.ScheduleEvent(EVENT_PULL_TORNADO_SHIELD, TIMER_PULL_TORNADO_SHIELD);
			if (IsHeroic())
				events.ScheduleEvent(EVENT_SUMMON_TEMPEST, urand(TIMER_SUMMON_TEMPEST_MIN, TIMER_SUMMON_TEMPEST_MAX));

			BossAI::EnterCombat(who);
		}

		void KilledUnit(Unit* killed)
		{
			me->BossYell("RAVAGED!", 20877);
			BossAI::KilledUnit(killed);
		}

		void JustDied(Unit* /*who*/)
		{
			me->SendPlaySound(20878, false);
			instance->SendEncounterUnit(ENCOUNTER_FRAME_REMOVE, me);
			me->DespawnCreaturesInArea(NPC_LURKING_TEMPEST, 120.0f);
			me->DespawnCreaturesInArea(NPC_CYCLONE_SHIELD, 120.0f);

			Creature * Slipstream = me->SummonCreature(45455, -765.79f, -44.01f, 641.91f, 4.0f, TEMPSUMMON_CORPSE_DESPAWN, 0);
			Slipstream->SetUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
		}

		void UpdateAI(uint32 const diff)
		{
			if (!UpdateVictim())
				return;

			events.Update(diff);

			while (uint32 EventId = events.ExecuteEvent())
				ExecuteEvent(EventId);

			DoSpellAttackIfReady(SPELL_LIGHTING_BOLT);
		}

		void ExecuteEvent(uint32 const eventId)
		{
			switch (eventId)
			{
			case EVENT_PULL_TORNADO_SHIELD:
				me->MonsterTextEmote("Grand Vizier Ertan retracts his cyclone shield!", NULL, true);
				me->RemoveAura(SPELL_STORMS_EDGE_AURA);
				DoCast(SPELL_STORMS_EDGE_PULL);
				DoCastDelayed(me, SPELL_STORMS_EDGE_INNER, TIMER_STORMS_EDGE_INNER);
				events.ScheduleEvent(EVENT_STORMS_EDGE, TIMER_STORMS_EDGE);
				break;
			case EVENT_STORMS_EDGE:
				DoCast(SPELL_STORMS_EDGE_AURA);
				events.ScheduleEvent(EVENT_PULL_TORNADO_SHIELD, TIMER_PULL_TORNADO_SHIELD);
				break;
			case EVENT_SUMMON_TEMPEST:
				DoCast(SPELL_SUMMON_TEMPEST);
				events.ScheduleEvent(EVENT_SUMMON_TEMPEST, urand(TIMER_SUMMON_TEMPEST_MIN, TIMER_SUMMON_TEMPEST_MAX));
				break;
			}
		}
	};

	CreatureAI* GetAI(Creature* Creature) const
	{
		return new boss_grand_vizier_ertanAI(Creature);
	}
};

Position const Center = { -719.435f, 3.839f, 635.671f };

class npc_cyclone_shield : public CreatureScript
{
public:
	npc_cyclone_shield() : CreatureScript("npc_cyclone_shield") {}

	struct npc_cyclone_shieldAI : public ScriptedAI
	{
		npc_cyclone_shieldAI(Creature* creature) : ScriptedAI(creature), position(0) { }

		uint32 position;
		EventMap events;

		void Reset()
		{
			ScriptedAI::Reset();
			events.Reset();
			me->SetReactState(REACT_PASSIVE);
			me->CastSpell(me, 86134, true); // Tornado Visual
		}

		void DoAction(int32 const action)
		{
			switch (action)
			{
			case ACTION_PULL:
			{
				Position pos(*me);
				pos.Subtract(CenterPosition);
				pos.Scale(STORMS_EDGE_RADIUS_SMALL / me->GetDistance2d(CenterPosition.GetPositionX(), CenterPosition.GetPositionY()));
				pos.Add(CenterPosition);

				me->GetMotionMaster()->MovementExpired();
				me->GetMotionMaster()->MoveCharge_NoPath(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), me->GetSpeed(MOVE_WALK) * 2, NUM_POSITIONS, false);
				break;
			}
			case ACTION_MOVE_OUT:
				me->GetMotionMaster()->MovePoint(position, TornadoPositions[position]);
				break;
			}
		}

		void UpdateAI(uint32 const diff)
		{
			events.Update(diff);

			if (events.ExecuteEvent())
			{
				me->GetMotionMaster()->MovePoint(position, TornadoPositions[position].GetPositionX(), TornadoPositions[position].GetPositionY(), TornadoPositions[position].GetPositionZ(), false);
				//me->GetMotionMaster()->MovePoint(position, TornadoPositions[position], false);
			}
		}

		void MovementInform(uint32 type, uint32 pointId)
		{
			if (type != POINT_MOTION_TYPE)
				return;

			if (pointId < NUM_POSITIONS)
			{
				position = (pointId + 1) % NUM_POSITIONS;
				events.ScheduleEvent(1, 100);
			}
		}
	};

	CreatureAI* GetAI(Creature* Creature) const
	{
		return new npc_cyclone_shieldAI(Creature);
	}
};

class npc_lurking_tempest : public CreatureScript
{
public:
	npc_lurking_tempest() : CreatureScript("npc_lurking_tempest") { }

	struct npc_lurking_tempestAI : public ScriptedAI
	{
		npc_lurking_tempestAI(Creature* creature) : ScriptedAI(creature)
		{
			me->SetReactState(REACT_PASSIVE);
			instance = me->GetInstanceScript();
			DoCastAOE(SPELL_LURK);
			if (Unit* Boss = me->FindNearestCreature(BOSS_GRAND_VIZIER_ERTAN, 100.0f))
			{
				BossGUID = Boss->GetGUID();
				me->SetTarget(BossGUID);
			}
			Boo = urand(1000, 4000);
		}

		InstanceScript* instance;
		uint32 Boo;
		uint64 BossGUID;

		void IsSummonedBy(Unit* summoner)
		{
			me->RemoveAllAuras();
		}

		void UpdateAI(uint32 const diff)
		{
			if (Boo <= diff)
			{
				if (Unit* target = SelectTarget(SELECT_TARGET_NEAREST, 0, 40.0f, true))
				{
					if (target->isInFrontInMap(me, 40.0f))
						DoCastAOE(SPELL_LURK);
					else
					{
						me->RemoveAurasDueToSpell(SPELL_LURK);
						DoCast(target, SPELL_TEMPEST_LIGHTING_BOLT);
					}
				}
				Boo = urand(1000, 4000);
			}
			else
				Boo -= diff;
		}

	};

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_lurking_tempestAI(creature);
	}
};

class spell_storms_edge_aura : public SpellScriptLoader
{
public:
	spell_storms_edge_aura() : SpellScriptLoader("spell_storms_edge_aura") { }

	class spell_storms_edge_auraSpellScript : public SpellScript
	{
		PrepareSpellScript(spell_storms_edge_auraSpellScript);

		void HandleCast()
		{
			std::list<Creature*> creatures;
			GetCreatureListWithEntryInGrid(creatures, GetCaster(), NPC_CYCLONE_SHIELD, 150.0f);

			if (!creatures.empty())
			{
				for (std::list<Creature*>::iterator iter = creatures.begin(); iter != creatures.end(); ++iter)
					(*iter)->AI()->DoAction(ACTION_MOVE_OUT);
				return;
			}

			uint32 j = 0;
			for (uint32 i = 0; i < NUM_POSITIONS; ++i)
			{
				Creature *creature = GetCaster()->SummonCreature(NPC_CYCLONE_SHIELD, TornadoPositions[i], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);
				j = (i + 1) % NUM_POSITIONS;
				creature->GetMotionMaster()->MovePoint(j, TornadoPositions[j]);
			}
		}

		void Register()
		{
			OnCast += SpellCastFn(spell_storms_edge_auraSpellScript::HandleCast);
		}
	};

	SpellScript* GetSpellScript() const
	{
		return new spell_storms_edge_auraSpellScript();
	}
};

struct isInsideStormsEdgeLarge
{
	bool operator()(WorldObject *obj)
	{
		return (obj->GetDistance2d(CenterPosition.GetPositionX(), CenterPosition.GetPositionY()) < STORMS_EDGE_RADIUS_LARGE);
	}
};

class spell_storms_edge_periodic : public SpellScriptLoader
{
public:
	spell_storms_edge_periodic() : SpellScriptLoader("spell_storms_edge_periodic") { }

	class spell_storms_edge_periodicSpellScript : public SpellScript
	{
		PrepareSpellScript(spell_storms_edge_periodicSpellScript);

		void FilterTargets(std::list<WorldObject *> &targets)
		{
			targets.remove_if(isInsideStormsEdgeLarge());
		}

		void EffectScriptEffect(SpellEffIndex effIndex)
		{
			GetHitUnit()->CastSpell((Unit*)NULL, SPELL_STORMS_EDGE, true);
		}

		void Register()
		{
			OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_storms_edge_periodicSpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
			OnEffectHitTarget += SpellEffectFn(spell_storms_edge_periodicSpellScript::EffectScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
		}
	};

	SpellScript* GetSpellScript() const
	{
		return new spell_storms_edge_periodicSpellScript();
	}
};

class spell_storms_edge_pull : public SpellScriptLoader
{
public:
	spell_storms_edge_pull() : SpellScriptLoader("spell_storms_edge_pull") { }

	class spell_storms_edge_pullSpellScript : public SpellScript
	{
		PrepareSpellScript(spell_storms_edge_pullSpellScript);

		void HandleCast()
		{
			std::list<Creature*> creatures;
			GetCreatureListWithEntryInGrid(creatures, GetCaster(), NPC_CYCLONE_SHIELD, 150.0f);

			for (std::list<Creature*>::iterator iter = creatures.begin(); iter != creatures.end(); ++iter)
				(*iter)->AI()->DoAction(ACTION_PULL);
		}

		void Register()
		{
			OnCast += SpellCastFn(spell_storms_edge_pullSpellScript::HandleCast);
		}
	};

	SpellScript* GetSpellScript() const
	{
		return new spell_storms_edge_pullSpellScript();
	}
};

struct isOutsideStormsEdgeSmall
{
	bool operator()(WorldObject *obj)
	{
		return (obj->GetDistance2d(CenterPosition.GetPositionX(), CenterPosition.GetPositionY()) > STORMS_EDGE_RADIUS_SMALL);
	}
};

class spell_storms_edge_knockback : public SpellScriptLoader
{
public:
	spell_storms_edge_knockback() : SpellScriptLoader("spell_storms_edge_knockback") { }

	class spell_storms_edge_knockbackSpellScript : public SpellScript
	{
		PrepareSpellScript(spell_storms_edge_knockbackSpellScript);

		void FilterTargets(std::list<WorldObject *> &targets)
		{
			targets.remove_if(isOutsideStormsEdgeSmall());
		}

		void HandleScript(SpellEffIndex effIndex)
		{
			GetHitUnit()->CastSpell((Unit*)NULL, SPELL_STORMS_EDGE, true);
		}

		void Register()
		{
			OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_storms_edge_knockbackSpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
			OnEffectHitTarget += SpellEffectFn(spell_storms_edge_knockbackSpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
		}
	};

	SpellScript* GetSpellScript() const
	{
		return new spell_storms_edge_knockbackSpellScript();
	}
};

class spell_storms_edge : public SpellScriptLoader
{
public:
	spell_storms_edge() : SpellScriptLoader("spell_storms_edge") { }

	class spell_storms_edgeSpellScript : public SpellScript
	{
		PrepareSpellScript(spell_storms_edgeSpellScript);

		void HandleScript(SpellEffIndex effIndex)
		{
			GetHitUnit()->CastSpell(GetCaster(), SPELL_STORMS_EDGE_DAMAGE, true);
		}

		void Register()
		{
			OnEffectHitTarget += SpellEffectFn(spell_storms_edgeSpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
		}
	};

	SpellScript* GetSpellScript() const
	{
		return new spell_storms_edgeSpellScript();
	}
};

void AddSC_boss_grand_vizier_ertan()
{
	new boss_grand_vizier_ertan();// ADDED IN WCDB
	new npc_cyclone_shield();// ADDED IN WCDB
	new npc_lurking_tempest();// ADDED IN WCDB
	new spell_storms_edge_aura;// ADDED IN WCDB
	new spell_storms_edge_periodic;// ADDED IN WCDB
	new spell_storms_edge_pull;// ADDED IN WCDB
	new spell_storms_edge_knockback;// ADDED IN WCDB
	new spell_storms_edge; // ADDED IN WCDB
}
