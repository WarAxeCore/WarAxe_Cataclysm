#include "InstanceScript.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "Cell.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "vortex_pinnacle.h"
#include "G3D/Vector3.h"
#include "G3D/Triangle.h"
#include "G3D/Plane.h"
#include "G3D/CollisionDetection.h"

using G3D::Vector3;

//Notes: Unstable Grounding Field does not work so it is hackfixed

//Current Hackfix: If player is within 3 yds of the Storm Target damage is zero.
// Mob: 46387 is storm target.

enum AsaadSpells
{
	SPELL_SUPREMACY_OF_THE_STORM = 86930,
	SPELL_UNSTABLE_GROUNDING_FIELD = 86911,
	SPELL_CHAIN_LIGHTNING = 87622,
	SPELL_SUMMON_SKYFALL = 96260,
	SPELL_SUMMON_UNSTABLE_GROUND_FIELD = 86658,
	SPELL_SUMMON_GROUNDING_FIELD = 87521,
	SPELL_CHANNEL_GROUNDING_FIELD = 86911,
	SPELL_SUPREMACY_TELEPORT = 87328,
	SPELL_GROUNDING_FIELD = 87514,
	SPELL_GROUNDING_FIELD_BUFF = 87474,
	SPELL_ARCANE_BARRAGE = 87854,

	// Heroic Mode
	SPELL_STATIC_CLING = 87618,

	SPELL_UG_TRIG = 86926,
	SPELL_STORM_RUNE_BEM = 86981,
	SPELL_STORM_RUNE_BEM_A = 86921,
	SPELL_STORM_RUNE_BEM_B = 86923,
	SPELL_STORM_RUNE_BEM_C = 86925
};

enum AsaadEvents
{
	EVENT_CHAIN_LIGHTNING = 1,
	EVENT_SUMMON_SKYFALL = 2,
	EVENT_STATIC_CLING = 3,
	EVENT_SUMMON_UNSTABLE_GROUND_FIELD = 4,
	EVENT_SUPREMACY_OF_THE_STORM = 5,
	EVENT_CLEAN_UP = 6,
	EVENT_ARCANE_BARRAGE = 7
};

enum AsaadData
{
	DATA_TRIGGER_GUID = 1,
	DATA_SUMMON_TRIGGER_GUID = 2,
};

enum Actions
{
	ACTION_CLEAN_UP = 1,
	ACTION_TELEPORT = 2,
	INTERUPT_SLIPSTREAM_CAST = 3
};

enum AsaadMovementPositionPoints
{
	DATA_FIRST_POSITION = 1,
	DATA_SECOND_POSITION = 2,
	DATA_THIRD_POSITION = 3
};

Position const centerPos = { -620.0f, 501.95f, 646.7f, 0.0f };

class boss_asaad : public CreatureScript
{
public:
	boss_asaad() : CreatureScript("boss_asaad")
	{
	}

	struct script_impl : public BossAI
	{
		script_impl(Creature* creature) : BossAI(creature, BOSS_ASAAD)
		{
		}

		std::list<uint64 > failedAchievementGUIDs;

		void Reset()
		{
			_Reset();
			//me->DisableMovementFlagUpdate(true);
			events.ScheduleEvent(EVENT_CHAIN_LIGHTNING, 5000);
			events.ScheduleEvent(EVENT_SUMMON_SKYFALL, urand(15000, 25000));
			if (IsHeroic())
				events.ScheduleEvent(EVENT_STATIC_CLING, urand(10000, 15000));
			events.ScheduleEvent(EVENT_SUMMON_UNSTABLE_GROUND_FIELD, 20000);
			failedAchievementGUIDs.clear();
			me->SetFlying(false);
			stormTargetGUID = 0;
		}

		void EnterCombat(Unit* /*who*/)
		{
			me->BossYell("YOU tread upon the sacrosanct! Mortals have no place amidst the clouds.", 20867);
			_EnterCombat();
		}

		void JustDied(Unit* /*who*/)
		{
			me->BossYell("The winds take me!", 20870);
			_JustDied();
		}	

		void JustReachedHome()
		{
			if (Creature *c = Unit::GetCreature(*me, stormTargetGUID))
				me->Kill(c);
		}

		void DoAction(const int32 act)
		{
			switch (act)
			{
			case INTERUPT_SLIPSTREAM_CAST:
			{
				me->InterruptNonMeleeSpells(false);
				me->CastStop();
				me->SetFlying(true);
				me->NearTeleportTo(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ() + 10, pos.GetOrientation());
				if (Creature *c = Unit::GetCreature(*me, stormTargetGUID))
				{
					c->NearTeleportTo(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ() + 10, pos.GetOrientation());
					if (Creature *c1 = c->SummonCreature(463874, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ() + 10, TEMPSUMMON_MANUAL_DESPAWN))
						c1->CastSpell(c1, SPELL_STORM_RUNE_BEM_A, true);
					if (Creature *c1 = c->SummonCreature(463874, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ() + 10, TEMPSUMMON_MANUAL_DESPAWN))
						c1->CastSpell(c1, SPELL_STORM_RUNE_BEM_B, true);
					if (Creature *c1 = c->SummonCreature(463874, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ() + 10, TEMPSUMMON_MANUAL_DESPAWN))
						c1->CastSpell(c1, SPELL_STORM_RUNE_BEM_C, true);
					for (int i = 0; i < 40; i++)
					{
						me->GetRandomPoint(centerPos, 40, pos);
						Creature * stormtarget = c->SummonCreature(46387, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ() + 10, TEMPSUMMON_MANUAL_DESPAWN);
						if (stormtarget)
						{
							//stormtarget->SetVisible(false);
							stormtarget->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
							stormtarget->SetReactState(REACT_PASSIVE);
						}
					}
				}
				me->CastSpell(me, SPELL_SUPREMACY_OF_THE_STORM, true);
				events.ScheduleEvent(EVENT_CLEAN_UP, 1);
				break;
			}
			default:
				break;
			}
		}

		uint64 GetRandomStormTarget() const
		{
			std::list<Creature* > triggerList;
			me->GetCreatureListWithEntryInGrid(triggerList, NPC_STORM_TARGET, 100.0f);
			if (!triggerList.empty())
				return SkyFire::Containers::SelectRandomContainerElement(triggerList)->GetGUID();
			return 0;
		}

		void JustSummoned(Creature* summoned)
		{
			switch (summoned->GetEntry())
			{
			case 46492:
				me->CastSpell(summoned, SPELL_UNSTABLE_GROUNDING_FIELD, true);
				break;
			case NPC_SKYFALL:
				summoned->SetInCombatWithZone();
				summoned->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
				break;
			default:
				break;
			}
		}

		void SpellHitTarget(Unit* target, SpellInfo const* spell)
		{
			if (spell->Id == SPELL_STATIC_CLING && IsHeroic())
				failedAchievementGUIDs.push_back(target->GetGUID());
		}

		void UpdateAI(uint32 const diff)
		{
			if (!UpdateVictim())
				return;

			if (me->HasUnitState(UNIT_STATE_CASTING))
				return;

			events.Update(diff);

			while (uint32 eventId = events.ExecuteEvent())
			{
				switch (eventId)
				{
				case EVENT_CHAIN_LIGHTNING:
					if (Unit* const target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
						me->CastSpell(target, SPELL_CHAIN_LIGHTNING, false);
					events.ScheduleEvent(EVENT_CHAIN_LIGHTNING, urand(50000, 65000));
					break;
				case EVENT_SUMMON_SKYFALL:
					me->CastSpell(me, SPELL_SUMMON_SKYFALL, true);
					events.ScheduleEvent(EVENT_SUMMON_SKYFALL, 20000);
					break;
				case EVENT_STATIC_CLING:
					me->CastSpell(me, SPELL_STATIC_CLING, false);
					events.ScheduleEvent(EVENT_STATIC_CLING, urand(12000, 15000));
					break;
				case EVENT_SUMMON_UNSTABLE_GROUND_FIELD:
					me->BossYell("Al'Akir your servant calls for aid!", 20869);
					me->GetRandomPoint(centerPos, 25, pos);
					if (Creature *c = me->SummonCreature(46492, pos, TEMPSUMMON_MANUAL_DESPAWN))
						stormTargetGUID = c->GetGUID();
					break;
				case EVENT_CLEAN_UP:
					me->SetFlying(false);
					if (Creature *c = Unit::GetCreature(*me, stormTargetGUID))
						me->Kill(c);
					events.ScheduleEvent(EVENT_SUMMON_UNSTABLE_GROUND_FIELD, urand(20000, 25000));
					break;
				default:
					break;
				}
			}

			DoMeleeAttackIfReady();
		}

	private:
		Position pos;
		uint64 stormTargetGUID;
	};

	CreatureAI* GetAI(Creature* creature) const
	{
		return GetVortexPinnacleAI<script_impl>(creature);
	}
};

class npc_asaad_grounding_field_trigger : public CreatureScript
{
public:
	npc_asaad_grounding_field_trigger() : CreatureScript("npc_asaad_grounding_field_trigger")
	{
	}

	struct script_impl : public ScriptedAI
	{
		script_impl(Creature* creature) : ScriptedAI(creature), summons(creature), instance(creature->GetInstanceScript()) { }


		void Reset()
		{
			pointId = 0;
			GenerateTrianglePos();
			isMoving = false;
			mui_timerMove = 1000;
			triggerCount = 0;
			me->SetSpeed(MOVE_WALK, 1.0f);
			me->SetSpeed(MOVE_RUN, 1.0f);
		}

		void CleanUp()
		{
			if (InstanceScript* const instance = me->GetInstanceScript())
				instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_GROUNDING_FIELD_BUFF);
			me->DespawnOrUnsummon();
		}

		void GenerateTrianglePos()
		{
			float x, y;
			me->GetNearPoint2D(x, y, urand(5.0f, 7.0f), me->GetOrientation());

			_trianglePos[0].m_positionX = x;
			_trianglePos[0].m_positionY = y;
			_trianglePos[0].m_positionZ = me->GetPositionZ();

			me->GetNearPoint2D(x, y, urand(5.0f, 7.0f), me->GetOrientation() + M_PI / 2);

			_trianglePos[1].m_positionX = x;
			_trianglePos[1].m_positionY = y;
			_trianglePos[1].m_positionZ = me->GetPositionZ();

			me->GetNearPoint2D(x, y, urand(5.0f, 7.0f), me->GetOrientation() + M_PI);

			_trianglePos[2].m_positionX = x;
			_trianglePos[2].m_positionY = y;
			_trianglePos[2].m_positionZ = me->GetPositionZ();

			me->NearTeleportTo(_trianglePos[0].m_positionX, _trianglePos[0].m_positionY, _trianglePos[0].m_positionZ, 0.0f);
		}

		void MovementInform(uint32 type, uint32 id)
		{
			if (type != POINT_MOTION_TYPE)
				return;

			switch (id)
			{
			case DATA_FIRST_POSITION:
			{
				me->GetMotionMaster()->Clear();
				me->InterruptNonMeleeSpells(true);
				Position _centerPos = me->GetHomePosition();
				me->GetMotionMaster()->MovePoint(42, _centerPos.m_positionX, _centerPos.m_positionY, _centerPos.m_positionZ + 10.0f);
				break;
			}
			case DATA_SECOND_POSITION:
			case DATA_THIRD_POSITION:
				isMoving = false;
				break;
			case 42:
				me->SummonCreature(463871, _trianglePos[0], TEMPSUMMON_MANUAL_DESPAWN);
				me->SummonCreature(463872, _trianglePos[1], TEMPSUMMON_MANUAL_DESPAWN);
				me->SummonCreature(463873, _trianglePos[2], TEMPSUMMON_MANUAL_DESPAWN);
				if (Creature *c = me->FindNearestCreature(NPC_ASAAD, 100.0f))
					c->AI()->DoAction(INTERUPT_SLIPSTREAM_CAST);
				break;
			default:
				break;
			}
		}

		void JustSummoned(Creature* summoned)
		{
			me->InterruptNonMeleeSpells(true);
			if (triggerCount <= 3)
				triggerId[triggerCount] = summoned->GetGUID();
			triggerCount++;
			summons.Summon(summoned);
			summoned->SetSpeed(MOVE_WALK, 1.0f);
			summoned->SetSpeed(MOVE_RUN, 1.0f);
		}

		void JustDied(Unit* /*killer*/)
		{
			summons.DespawnAll();
		}

		void UpdateAI(uint32 const diff)
		{
			if (pointId == 4 || isMoving)
				return;

			if (mui_timerMove <= diff)
			{
				if (pointId == 2)
				{
					if (Creature *c1 = me->SummonCreature(46387, _trianglePos[pointId], TEMPSUMMON_MANUAL_DESPAWN))
						if (Creature *c2 = me->SummonCreature(463870, _trianglePos[pointId], TEMPSUMMON_MANUAL_DESPAWN))
						{
							c2->CastSpell(c1, SPELL_STORM_RUNE_BEM, true);
							c2->GetMotionMaster()->MovePoint(1, _trianglePos[0]);
						}
					me->GetMotionMaster()->MovePoint(1, _trianglePos[0]);
				}
				else
				{
					if (Creature *c1 = me->SummonCreature(46387, _trianglePos[pointId], TEMPSUMMON_MANUAL_DESPAWN))
						if (Creature *c2 = me->SummonCreature(463870, _trianglePos[pointId], TEMPSUMMON_MANUAL_DESPAWN))
						{
							c2->CastSpell(c1, SPELL_STORM_RUNE_BEM, true);
							c2->GetMotionMaster()->MovePoint(pointId + 2, _trianglePos[pointId + 1]);
						}
					me->GetMotionMaster()->MovePoint(pointId + 2, _trianglePos[pointId + 1]);
				}
				pointId++;
				isMoving = true;
				mui_timerMove = 2500;
			}
			else
				mui_timerMove -= diff;
		}

		Position _trianglePos[3];
	private:
		uint32 mui_timerMove;
		uint8 pointId;
		uint8 triggerCount;
		SummonList summons;
		InstanceScript const* instance;
		uint64 triggerId[3];
		bool isMoving;
	};

	CreatureAI* GetAI(Creature* creature) const
	{
		return GetVortexPinnacleAI<script_impl>(creature);
	}
};

class npc_skyfall_star : public CreatureScript
{
public:
	npc_skyfall_star() : CreatureScript("npc_skyfall_star") { }

	struct npc_skyfall_starAI : public ScriptedAI
	{
		npc_skyfall_starAI(Creature* creature) : ScriptedAI(creature) { }

		void EnterCombat(Unit* /*who*/)
		{
			_events.ScheduleEvent(EVENT_ARCANE_BARRAGE, urand(3000, 10000));
		}

		void UpdateAI(const uint32 diff)
		{
			if (!UpdateVictim())
				return;

			_events.Update(diff);

			if (me->HasUnitState(UNIT_STATE_CASTING))
				return;

			while (uint32 eventId = _events.ExecuteEvent())
			{
				switch (eventId)
				{
				case EVENT_ARCANE_BARRAGE:
					if (Player* player = me->FindNearestPlayer(55.0f))
					{
						me->AddThreat(player, 55.0f);
						me->AI()->AttackStart(player);
						DoCast(SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true), SPELL_ARCANE_BARRAGE, true);
					}
					_events.ScheduleEvent(EVENT_ARCANE_BARRAGE, urand(3000, 5000));
					break;
				default:
					break;
				}
			}
		}

	private:
		EventMap _events;

	};

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_skyfall_starAI(creature);
	}
};

class TriangleCheck
{
	WorldObject* const _target;
	Position const _position1;
	Position const _position2;
	Position const _position3;

public:
	TriangleCheck(WorldObject* target, Position position1, Position position2, Position position3)
		: _target(target), _position1(position1), _position2(position2), _position3(position3) {}

	bool Check() const
	{
		if (IsInTriangle())
			return true;

		return false;
	}

private:
	bool IsInTriangle() const
	{
		G3D::Triangle const triangle(PositionToVector3(_position1), PositionToVector3(_position2), PositionToVector3(_position3));
		G3D::Vector3 const vector(_target->GetPositionX(), _target->GetPositionY(), _target->GetPositionZ());
		float b[3];

		return G3D::CollisionDetection::isPointInsideTriangle(triangle.vertex(0), triangle.vertex(1), triangle.vertex(2), triangle.normal(), vector, b, triangle.primaryAxis());
	}

	G3D::Vector3 PositionToVector3(Position const& position) const
	{
		return G3D::Vector3(position.GetPositionX(), position.GetPositionY(), position.GetPositionZ());
	}
};

class achievement_not_static_at_all : public AchievementCriteriaScript
{
	bool OnCheck(Player* player, Unit* target)
	{
		if (!target || !target->ToCreature())
			return false;

		std::list<uint64> failedAchievementGUIDs = CAST_AI(boss_asaad::script_impl, target->ToCreature()->AI())->failedAchievementGUIDs;
		if (!failedAchievementGUIDs.empty())
			for (std::list<uint64>::iterator itr = failedAchievementGUIDs.begin(); itr != failedAchievementGUIDs.end(); ++itr)
				if (player->GetGUID() == *itr)
					return false;

		return true;
	}

public:
	achievement_not_static_at_all() : AchievementCriteriaScript("achievement_not_static_at_all") {}
};

class spell_asaad_static_cling : public SpellScriptLoader
{
	class StaticClingCheck
	{
	public:
		StaticClingCheck()
		{
		}

		bool operator() (WorldObject* obj)
		{
			if (Unit *target = obj->ToUnit())
				if (target->HasUnitMovementFlag(MOVEMENTFLAG_FALLING))
					return true;
			return false;
		}
	};

public:
	spell_asaad_static_cling() : SpellScriptLoader("spell_asaad_static_cling")
	{
	}

	class script_impl : public SpellScript
	{
		PrepareSpellScript(script_impl);

		bool Load()
		{
			return true;
		}

		void FilterTargets(std::list<WorldObject*>& unitList)
		{
			unitList.remove_if(StaticClingCheck());
		}

		void Register()
		{
			OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(script_impl::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
			OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(script_impl::FilterTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
		}
	};

	SpellScript* GetSpellScript() const
	{
		return new script_impl();
	}
};

class spell_asaad_supremacy_entry : public SpellScriptLoader
{
public:
	spell_asaad_supremacy_entry() : SpellScriptLoader("spell_asaad_supremacy_entry") {}

	class spell_asaad_supremacy_entry_SpellScript : public SpellScript
	{
		PrepareSpellScript(spell_asaad_supremacy_entry_SpellScript);

		Targets _targets;
		Position position[3];

		bool Load()
		{
			return true;
		}

		void FilterTargets(std::list<WorldObject*>& unitList)
		{
			Creature* const trigger = GetCaster()->FindNearestCreature(NPC_UNSTABLE_GROUNDING_FIELD, 100.0f, true);
			position[0] = CAST_AI(npc_asaad_grounding_field_trigger::script_impl, trigger->AI())->_trianglePos[0];
			position[1] = CAST_AI(npc_asaad_grounding_field_trigger::script_impl, trigger->AI())->_trianglePos[1];
			position[2] = CAST_AI(npc_asaad_grounding_field_trigger::script_impl, trigger->AI())->_trianglePos[2];

			for (std::list<WorldObject*>::iterator itr = unitList.begin(); itr != unitList.end();)
			{
				TriangleCheck const* const triangle = new TriangleCheck(*itr, position[0], position[1], position[2]);
				if (triangle->Check())
					unitList.remove(*itr++);
				else
					++itr;
			}
		}

		void HandleScript()
		{
			if (!GetCaster() || !GetHitUnit())
				return;

			if (GetHitUnit()->FindNearestCreature(46387, 3.5f))
				SetHitDamage(0);
		}

		void Register()
		{
			OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_asaad_supremacy_entry_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
			BeforeHit += SpellHitFn(spell_asaad_supremacy_entry_SpellScript::HandleScript);
		}
	};

	SpellScript* GetSpellScript() const
	{
		return new spell_asaad_supremacy_entry_SpellScript();
	}
};

class spell_asaad_supremacy_dummy : public SpellScriptLoader
{
public:
	spell_asaad_supremacy_dummy() : SpellScriptLoader("spell_asaad_supremacy_dummy") {}

	class spell_asaad_supremacy_dummy_SpellScript : public SpellScript
	{
		PrepareSpellScript(spell_asaad_supremacy_dummy_SpellScript);

		Targets _targets;
		Position position[3];

		bool Load()
		{
			return true;
		}

		void FilterTargets(std::list<WorldObject*>& unitList)
		{
			Creature* const trigger = GetCaster()->FindNearestCreature(NPC_UNSTABLE_GROUNDING_FIELD, 100.0f, true);
			position[0] = CAST_AI(npc_asaad_grounding_field_trigger::script_impl, trigger->AI())->_trianglePos[0];
			position[1] = CAST_AI(npc_asaad_grounding_field_trigger::script_impl, trigger->AI())->_trianglePos[1];
			position[2] = CAST_AI(npc_asaad_grounding_field_trigger::script_impl, trigger->AI())->_trianglePos[2];

			for (std::list<WorldObject*>::iterator itr = unitList.begin(); itr != unitList.end();)
			{
				TriangleCheck const* const triangle = new TriangleCheck(*itr, position[0], position[1], position[2]);
				if (triangle->Check())
					unitList.remove(*itr++);
				else
					++itr;
			}
		}

		void HandleScript()
		{
			if (!GetCaster() || !GetHitUnit())
				return;

			if (GetHitUnit()->FindNearestCreature(46387, 3.5f))
				SetHitDamage(0);
		}

		void Register()
		{
			OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_asaad_supremacy_dummy_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
			BeforeHit += SpellHitFn(spell_asaad_supremacy_dummy_SpellScript::HandleScript);
		}
	};

	SpellScript* GetSpellScript() const
	{
		return new spell_asaad_supremacy_dummy_SpellScript();
	}
};

void AddSC_boss_asaad()
{
	new boss_asaad();
	new npc_asaad_grounding_field_trigger();
	new npc_skyfall_star();
	new achievement_not_static_at_all();
	new spell_asaad_static_cling();
	new spell_asaad_supremacy_entry();
	new spell_asaad_supremacy_dummy();
}