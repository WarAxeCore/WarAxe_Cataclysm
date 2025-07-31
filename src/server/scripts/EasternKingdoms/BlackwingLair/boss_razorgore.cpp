/*
 * This file is part of the WarAxeCore Project, ported from AzerothCore.
 */

#include "ScriptPCH.h"
#include "blackwing_lair.h"

enum Spells
{
	SPELL_MINDCONTROL = 19832,
	SPELL_MINDCONTROL_VISUAL = 45537,
	SPELL_EGG_DESTROY = 19873,
	SPELL_MIND_EXHAUSTION = 23958,

	SPELL_CLEAVE = 19632,
	SPELL_WARSTOMP = 24375,
	SPELL_FIREBALLVOLLEY = 22425,
	SPELL_CONFLAGRATION = 23023,

	SPELL_EXPLODE_ORB = 20037,
	SPELL_EXPLOSION = 20038, // Instakill everything.

	SPELL_WARMING_FLAMES = 23040,
};

enum Summons
{
	NPC_ELITE_DRACHKIN = 12422,
	NPC_ELITE_WARRIOR = 12458,
	NPC_WARRIOR = 12416,
	NPC_MAGE = 12420,
	NPC_WARLOCK = 12459,

	GO_EGG = 177807
};

enum EVENTS
{
	EVENT_CLEAVE = 1,
	EVENT_STOMP = 2,
	EVENT_FIREBALL = 3,
	EVENT_CONFLAGRATION = 4
};

class boss_razorgore : public CreatureScript
{
public:
	boss_razorgore() : CreatureScript("boss_razorgore") { }

	struct boss_razorgoreAI : public BossAI
	{
		boss_razorgoreAI(Creature* creature) : BossAI(creature, DATA_RAZORGORE_THE_UNTAMED)
		{
			secondPhase = false;
			charmerGUID = 0;
		}

		void Reset()
		{
			_Reset();
			charmerGUID = 0;
			secondPhase = false;
			summons.DespawnAll();
			if (instance)
			{
				instance->SetData(DATA_EGG_EVENT, NOT_STARTED);
			}
			summonGUIDs.clear();
		}

		void JustDied(Unit* /*killer*/)
		{
			if (secondPhase)
			{
				_JustDied();
			}
			else
			{
				// Respawn shorty in case of failure during phase 1.
				me->RemoveCorpse(false);
				me->SetRespawnTime(30);
				me->SaveRespawnTime();

				// Might not be required, safe measure.
				me->SetLootRecipient(NULL);

				instance->SetData(DATA_EGG_EVENT, FAIL);
			}
		}

		bool CanAIAttack(Unit const* target) const
		{
			return !(target->GetTypeId() == TYPEID_UNIT && !secondPhase);
		}

		void EnterCombat(Unit* who)
		{
			BossAI::EnterCombat(who);

			events.ScheduleEvent(EVENT_CLEAVE, 15000);
			events.ScheduleEvent(EVENT_STOMP, 35000);
			events.ScheduleEvent(EVENT_FIREBALL, 7000);
			events.ScheduleEvent(EVENT_CONFLAGRATION, 12000);

			if (instance)
			{
				instance->SetData(DATA_EGG_EVENT, IN_PROGRESS);
			}
		}

		void DoChangePhase()
		{
			secondPhase = true;
			charmerGUID = 0;
			me->RemoveAllAuras();

			DoCast(me, SPELL_WARMING_FLAMES, true);

			//if (Creature* troops = ObjectAccessor::GetCreature(*me, nefarianTroopsGUID))
				//troops->AI()->Talk(EMOTE_TROOPS_RETREAT);

			for (std::vector<uint64>::iterator itr = summonGUIDs.begin(); itr != summonGUIDs.end(); ++itr)
			{
				if (Creature* creature = ObjectAccessor::GetCreature(*me, *itr))
				{
					if (creature->isAlive())
					{
						creature->CombatStop(true);
						creature->SetReactState(REACT_PASSIVE);
						creature->GetMotionMaster()->MovePoint(0, -7560.568848f, -1028.553345f, 408.491211f, 0.523858f);
					}
				}
			}
		}

		void SetGUID(uint64 guid, int32 /*id*/)
		{
			charmerGUID = guid;
		}

		void OnCharmed(bool apply)
		{
			if (apply)
			{
				if (Unit* charmer = ObjectAccessor::GetUnit(*me, charmerGUID))
				{
					charmer->CastSpell(charmer, SPELL_MIND_EXHAUSTION, true);
					charmer->CastSpell(me, SPELL_MINDCONTROL_VISUAL, false);
				}
			}
			else
			{
				if (Unit* charmer = ObjectAccessor::GetUnit(*me, charmerGUID))
				{
					charmer->RemoveAurasDueToSpell(SPELL_MINDCONTROL_VISUAL);
					me->TauntApply(charmer);
				}
			}
		}

		void DoAction(int32 action)
		{
			if (action == ACTION_PHASE_TWO)
				DoChangePhase();

			if (action == TALK_EGG_BROKEN_RAND)
			{
				switch (urand(0, 2))
				{
				case 0:
					me->BossYell("Fools! These eggs are more precious than you know!", 8276);
					break;
				case 1:
					me->BossYell("No - not another one! I'll have your heads for this atrocity!", 8277);
					break;
				case 2:
					me->BossYell("You'll pay for forcing me to do this!", 8275);
					break;
				}
			}
		}

		void JustSummoned(Creature* summon)
		{
			summonGUIDs.push_back(summon->GetGUID());
			summon->SetUInt64Value(UNIT_FIELD_SUMMONEDBY, me->GetGUID());
			summons.Summon(summon);
		}

		void SummonMovementInform(Creature* summon, uint32 movementType, uint32 /*pathId*/)
		{
			if (movementType == POINT_MOTION_TYPE)
				summon->DespawnOrUnsummon();
		}

		void DamageTaken(Unit*, uint32& damage, DamageEffectType, SpellSchoolMask)
		{
			if (!secondPhase && damage >= me->GetHealth())
			{
				me->BossYell("If I fall into the abyss, I'll take all of you mortals with me!", 8278);
				DoCastAOE(SPELL_EXPLODE_ORB);
				DoCastAOE(SPELL_EXPLOSION);
			}
		}

		void UpdateAI(uint32 diff)
		{
			if (me->HasAura(11446))
			{
				if (me->GetAura(11446)->GetDuration() >= 90001)
					me->GetAura(11446)->SetDuration(90000);
			}

			if (!UpdateVictim())
				return;

			if (!me->isCharmed())
				events.Update(diff);

			if (me->HasUnitState(UNIT_STATE_CASTING))
				return;

			while (uint32 eventId = events.ExecuteEvent())
			{
				switch (eventId)
				{
				case EVENT_CLEAVE:
					DoCastVictim(SPELL_CLEAVE);
					events.ScheduleEvent(EVENT_CLEAVE, urand(7000, 10000));
					break;
				case EVENT_STOMP:
					DoCastVictim(SPELL_WARSTOMP);
					events.ScheduleEvent(EVENT_STOMP, urand(15000, 25000));
					break;
				case EVENT_FIREBALL:
					DoCastVictim(SPELL_FIREBALLVOLLEY);
					events.ScheduleEvent(EVENT_FIREBALL, urand(12000, 15000));
					break;
				case EVENT_CONFLAGRATION:
					DoCastVictim(SPELL_CONFLAGRATION);
					events.ScheduleEvent(EVENT_CONFLAGRATION, 30000);
					break;
				}
			}

			DoMeleeAttackIfReady();
		}

	private:
		bool secondPhase;
		uint64 charmerGUID;
		std::vector<uint64> summonGUIDs;
	};

	CreatureAI* GetAI(Creature* creature) const
	{
		return new boss_razorgoreAI(creature);
	}
};

class go_orb_of_domination : public GameObjectScript
{
public:
	go_orb_of_domination() : GameObjectScript("go_orb_of_domination") { }

	bool OnGossipHello(Player* player, GameObject* go)
	{
		if (InstanceScript* instance = go->GetInstanceScript())
			if (instance->GetData(DATA_EGG_EVENT) != DONE && !player->HasAura(SPELL_MIND_EXHAUSTION) && !player->GetPet())
				if (Creature* razor = go->FindNearestCreature(NPC_RAZORGORE, 250.0f, true))
				{
					razor->AI()->SetGUID(player->GetGUID(), 0);
					razor->Attack(player, true);
					player->CastSpell(razor, SPELL_MINDCONTROL);
					player->CastSpell(razor, 11446);
				}
		return true;
	}
};

class spell_egg_event : public SpellScriptLoader
{
public:
	spell_egg_event() : SpellScriptLoader("spell_egg_event") { }

	SpellScript* GetSpellScript() const override
	{
		return new spell_egg_event_SpellScript();
	}

	class spell_egg_event_SpellScript : public SpellScript
	{
		PrepareSpellScript(spell_egg_event_SpellScript);

		void HandleOnHit()
		{
			if (InstanceScript* instance = GetCaster()->GetInstanceScript())
				instance->SetData(DATA_EGG_EVENT, SPECIAL);

			if (Creature* razorgore = GetCaster()->ToCreature())
			{
				if (GameObject* egg = GetHitGObj())
				{
					razorgore->AI()->DoAction(TALK_EGG_BROKEN_RAND);
					egg->SetLootState(GO_READY);
					egg->UseDoorOrButton(10000);
					egg->SetRespawnTime(WEEK);
				}
			}
		}

		void Register()
		{
			OnHit += SpellHitFn(spell_egg_event_SpellScript::HandleOnHit);
		}
	};
};

void AddSC_boss_razorgore()
{
	new boss_razorgore();
	new go_orb_of_domination();
	new spell_egg_event();
}