#include "deadmines.h"

enum ScriptTexts
{
	SAY_DEATH = 0,
	SAY_ARCANE_POWER = 1,
	SAY_AGGRO = 2,
	SAY_KILL = 3,
	SAY_FIRE = 4,
	SAY_HEAD1 = 5,
	SAY_FROST = 6,
	SAY_HEAD2 = 7
};
enum Spells
{
	SPELL_ARCANE_POWER = 88009,
	SPELL_FIST_OF_FLAME = 87859,
	SPELL_FIST_OF_FLAME_0 = 87896,
	SPELL_FIST_OF_FROST = 87861,
	SPELL_FIST_OF_FROST_0 = 87901,
	SPELL_FIRE_BLOSSOM = 88129,
	SPELL_FROST_BLOSSOM = 88169,
	SPELL_FROST_BLOSSOM_0 = 88177,
	SPELL_BLINK = 38932
};

enum Events
{
	EVENT_FIST_OF_FLAME = 1,
	EVENT_FIST_OF_FROST = 2,
	EVENT_BLINK = 3,
	EVENT_BLOSSOM = 4,
	EVENT_ARCANE_POWER1 = 5,
	EVENT_ARCANE_POWER2 = 6,
	EVENT_ARCANE_POWER3 = 7
};

enum Adds
{
	NPC_FIRE_BLOSSOM = 48957,
	NPC_FROST_BLOSSOM = 48958,
	NPC_FIREWALL_2A = 48976,
	NPC_FIREWALL_1A = 48975,
	NPC_FIREWALL_1B = 49039,
	NPC_FIREWALL_2B = 49041,
	NPC_FIREWALL_2C = 49042
};

class boss_glubtok : public CreatureScript
{
public:
	boss_glubtok() : CreatureScript("boss_glubtok") {}

	CreatureAI* GetAI(Creature* pCreature) const
	{
		return new boss_glubtokAI(pCreature);
	}

	struct boss_glubtokAI : public BossAI
	{
		boss_glubtokAI(Creature* pCreature) : BossAI(pCreature, DATA_GLUBTOK)
		{
			me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
			me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
			me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
			me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
			me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
			me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FREEZE, true);
			me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
			me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_HORROR, true);
			me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SAPPED, true);
			me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
			me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, true);
			me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CONFUSE, true);
			me->setActive(true);
		}

		uint8 stage;

		void Reset() override
		{
			_Reset();

			stage = 0;
			me->SetReactState(REACT_AGGRESSIVE);
		}

		void InitializeAI()
		{
			if (!instance || static_cast<InstanceMap*>(me->GetMap())->GetScriptId() != sObjectMgr->GetScriptId(DMScriptName))
				me->IsAIEnabled = false;
			else if (!me->isDead())
				Reset();
		}

		void EnterCombat(Unit* /*who*/) override
		{
			stage = 0;
			me->BossYell("Glubtok show you da power of de arcane!", 21151);
			events.RescheduleEvent(EVENT_FIST_OF_FLAME, 10000);
			DoZoneInCombat();
			instance->SetBossState(DATA_GLUBTOK, IN_PROGRESS);
		}

		void KilledUnit(Unit* /*victim*/) override
		{
			me->BossYell("'Sploded dat one!", 21155);
		}

		void JustDied(Unit* /*killer*/) override
		{
			_JustDied();
			me->BossYell("TOO...MUCH...POWER!!!", 21145);

			GameObject* go_door = me->FindNearestGameObject(GO_FACTORY_DOOR, 250.0f);
			if (go_door)
			{
				go_door->SetGoState(GO_STATE_ACTIVE);
			}
		}

		void UpdateAI(uint32 diff)
		{
			if (!UpdateVictim())
				return;

			if (me->GetHealthPct() <= 50 && stage == 0)
			{
				stage = 1;
				events.Reset();
				me->SetReactState(REACT_PASSIVE);
				me->BossYell("Glubtok ready?", 21154);
				events.RescheduleEvent(EVENT_ARCANE_POWER1, 1800);
				return;
			}

			events.Update(diff);

			if (me->HasUnitState(UNIT_STATE_CASTING))
				return;

			if (uint32 eventId = events.ExecuteEvent())
			{
				switch (eventId)
				{
				case EVENT_ARCANE_POWER1:
					me->BossYell("Let's do it!", 21157);
					events.RescheduleEvent(EVENT_ARCANE_POWER2, 2200);
					break;
				case EVENT_ARCANE_POWER2:
					me->NearTeleportTo(-193.43f, -437.86f, 54.38f, 4.88f, true);
					SetCombatMovement(false);
					me->AttackStop();
					me->RemoveAllAuras();
					events.RescheduleEvent(EVENT_ARCANE_POWER3, 1000);
					break;
				case EVENT_ARCANE_POWER3:
					SetCombatMovement(false);
					DoCast(me, SPELL_ARCANE_POWER, true);
					me->BossYell("ARCANE POWER!!!", 21146);
					events.RescheduleEvent(EVENT_BLOSSOM, 5000);
					break;
				case EVENT_BLOSSOM:
					if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
						DoCast(target, urand(0, 1) ? SPELL_FIRE_BLOSSOM : SPELL_FROST_BLOSSOM);
					events.RescheduleEvent(EVENT_BLOSSOM, 5000);
					break;
				case EVENT_FIST_OF_FLAME:
					DoCast(me, SPELL_FIST_OF_FLAME);
					me->BossYell("Fists of flame!", 21153);
					events.RescheduleEvent(EVENT_BLINK, 20000);
					events.RescheduleEvent(EVENT_FIST_OF_FROST, 20500);
					break;
				case EVENT_FIST_OF_FROST:
					DoCast(me, SPELL_FIST_OF_FROST);
					me->BossYell("Fists of frost!", 21156);
					events.RescheduleEvent(EVENT_BLINK, 20000);
					events.RescheduleEvent(EVENT_FIST_OF_FLAME, 20500);
					break;
				case EVENT_BLINK:
					if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
					{
						DoCast(target, SPELL_BLINK);
						if (IsHeroic())
							DoResetThreat();
					}
					break;
				}
			}
			DoMeleeAttackIfReady();
		}
	};
};

void AddSC_boss_glubtok()
{
	new boss_glubtok();
}