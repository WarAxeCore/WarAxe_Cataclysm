#include "ScriptPCH.h"
#include "Spell.h"
#include "blackrock_caverns.h"

//TODO: Script needs updated but about 60% scripted.

enum Spells
{
	SPELL_AURA_ACELERATION = 75817,
	SPELL_AURA_ACELERATION_H = 87376,
	SPELL_DARK_COMMAND = 75823,
	SPELL_DARK_COMMAND_H = 93462,
	SPELL_DRAIN_ESSENCE = 75645,
	SPELL_TWILIGHT_EVOLUTION = 75732,
	SPELL_EVOLUTION_0 = 75697,
	SPELL_EVOLUTION = 75610,
	SPELL_KNEELING_IN_SUPPLICATION = 75608,
};

enum Adds
{
	NPC_CORLA_TWILIGHT_ZEALOT = 50284,
};

enum Events
{
	EVENT_DARK_COMMAND = 1,
};

const Position twilightzealotPos[3] =
{
	{574.16f, 974.27f, 155.35f, 4.74f},
	{563.06f, 982.78f, 155.35f, 4.63f},
	{587.57f, 984.64f, 155.35f, 4.49f}
};

class boss_corla_herald_of_twilight : public CreatureScript
{

public:
	boss_corla_herald_of_twilight() : CreatureScript("boss_corla_herald_of_twilight") { }

	CreatureAI* GetAI(Creature* pCreature) const
	{
		return new boss_corla_herald_of_twilightAI(pCreature);
	}

	struct boss_corla_herald_of_twilightAI : public ScriptedAI
	{
		boss_corla_herald_of_twilightAI(Creature* c) : ScriptedAI(c), summons(me)
		{
			me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
			me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
			pInstance = (InstanceScript*)c->GetInstanceScript();
		}

		InstanceScript* pInstance;
		EventMap events;
		SummonList summons;
		Creature* twilightzealots[3];

		void Reset()
		{
			events.Reset();
			summons.DespawnAll();
			if (pInstance)
			{
				if (IsHeroic())
					for (uint8 i = 0; i < 3; i++)
						twilightzealots[i] = me->SummonCreature(NPC_CORLA_TWILIGHT_ZEALOT, twilightzealotPos[i]);
				else
					for (uint8 i = 1; i < 3; i++)
						twilightzealots[i] = me->SummonCreature(NPC_CORLA_TWILIGHT_ZEALOT, twilightzealotPos[i]);

				pInstance->SetData(DATA_CORLA, NOT_STARTED);
			}
		}

		void SpellHit(Unit* caster, SpellInfo const* spell)
		{
			if (me->GetCurrentSpell(CURRENT_GENERIC_SPELL))
				if ((me->GetCurrentSpell(CURRENT_GENERIC_SPELL)->m_spellInfo->Id == SPELL_DARK_COMMAND) ||
					(me->GetCurrentSpell(CURRENT_GENERIC_SPELL)->m_spellInfo->Id == SPELL_DARK_COMMAND_H))
					for (uint8 i = 0; i < 3; ++i)
						if (spell->Effects[i].Effect == SPELL_EFFECT_INTERRUPT_CAST)
							me->InterruptSpell(CURRENT_GENERIC_SPELL);
		}

		void EnterCombat(Unit* /*who*/)
		{
			DoCast(SPELL_AURA_ACELERATION);
			events.ScheduleEvent(EVENT_DARK_COMMAND, urand(6000, 10000));
			me->BossYell("HERETICS! You will suffer for this interruption!", 18589);
			DoZoneInCombat();

			if (Creature * p1Raz = GetClosestCreatureWithEntry(me, NPC_RAZ_THE_CRAZED, 300.0f))
			{
				if (p1Raz->GetPositionX() == 417.744720f)
				{
					p1Raz->DespawnOrUnsummon(1000);
				}
			}

			if (pInstance)
				pInstance->SetData(DATA_CORLA, IN_PROGRESS);
		}

		void UpdateAI(const uint32 diff)
		{
			if (!UpdateVictim())
				return;

			if (me->GetDistance(me->GetHomePosition()) > 60.0f)
			{
				EnterEvadeMode();
				return;
			}

			events.Update(diff);

			if (me->HasUnitState(UNIT_STATE_CASTING))
				return;

			while (uint32 eventId = events.ExecuteEvent())
			{
				switch (eventId)
				{
				case EVENT_DARK_COMMAND:
					if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1))
						DoCast(target, DUNGEON_MODE(SPELL_DARK_COMMAND, SPELL_DARK_COMMAND_H));
					events.ScheduleEvent(EVENT_DARK_COMMAND, urand(23000, 25000));
					break;
				}
			}

			DoMeleeAttackIfReady();
		}

		void JustDied(Unit* /*killer*/)
		{
			summons.DespawnAll();
			me->BossYell("For the master I die a thousand times... A thousan...", 18594);

			if (Creature * pRaz = GetClosestCreatureWithEntry(me, NPC_RAZ_THE_CRAZED, 300.0f))
			{
				Map::PlayerList const& players2 = pInstance->instance->GetPlayers();
				if (!players2.isEmpty())
				{
					for (Map::PlayerList::const_iterator itr = players2.begin(); itr != players2.end(); ++itr)
						if (Player * player = itr->getSource())
						{
							if (player->GetTeam() == TEAM_ALLIANCE)
								pRaz->setFaction(85);
							else
								pRaz->setFaction(11);
						}
				}
				pRaz->SetSpeed(MOVE_WALK, 3.0f, true);
				pRaz->SetSpeed(MOVE_RUN, 2.0f, true);
				pRaz->GetMotionMaster()->MovePath(6972252, false);
			}

			if (pInstance)
				pInstance->SetData(DATA_CORLA, DONE);
		}

		void KilledUnit(Unit * victim)
		{
			me->BossYell("There is only one true path of enlightment! DEATH!", 18590);
		}

		void SummonedCreatureDespawn(Creature* summon)
		{
			summons.Despawn(summon);
		}

		void JustSummoned(Creature* summon)
		{
			summons.Summon(summon);
			switch (summon->GetEntry())
			{
			case NPC_CORLA_TWILIGHT_ZEALOT:
				summon->SetReactState(REACT_PASSIVE);
				summon->CastSpell(summon, SPELL_KNEELING_IN_SUPPLICATION, true);
				break;
			}
		}
	};
};

void AddSC_boss_corla_herald_of_twilight()
{
	new boss_corla_herald_of_twilight();
}