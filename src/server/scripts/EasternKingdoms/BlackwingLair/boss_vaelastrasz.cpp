/*
 * WarAxeCore - Blackwing Lair: Vaelastrasz the Corrupt
 */

#include "ScriptPCH.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "blackwing_lair.h"

constexpr float aNefariusSpawnLoc[4] = { -7466.16f, -1040.80f, 412.053f, 2.14675f };

enum Gossip
{
	GOSSIP_ID = 21334,
};

#define GOSSIP_ITEM         "Vaelastraz, no!!"

enum Spells
{
	SPELL_ESSENCE_OF_THE_RED = 23513,
	SPELL_FLAME_BREATH = 23461,
	SPELL_FIRE_NOVA = 23462,
	SPELL_TAIL_SWEEP = 15847,
	SPELL_CLEAVE = 19983,
	SPELL_NEFARIUS_CORRUPTION = 23642,
	SPELL_RED_LIGHTNING = 19484,
	SPELL_BURNING_ADRENALINE = 18173,
	SPELL_BURNING_ADRENALINE_EXPLOSION = 23478,
	SPELL_BURNING_ADRENALINE_INSTAKILL = 23644
};

enum Events
{
	EVENT_SPEECH_1 = 1,
	EVENT_SPEECH_2 = 2,
	EVENT_SPEECH_3 = 3,
	EVENT_SPEECH_4 = 4,
	EVENT_SPEECH_5 = 5,
	EVENT_SPEECH_6 = 6,
	EVENT_SPEECH_7 = 7,
	EVENT_FLAME_BREATH = 8,
	EVENT_FIRE_NOVA = 9,
	EVENT_TAIL_SWEEP = 10,
	EVENT_CLEAVE = 11,
	EVENT_BURNING_ADRENALINE = 12,
};

class boss_vaelastrasz : public CreatureScript
{
public:
	boss_vaelastrasz() : CreatureScript("boss_vaelastrasz") { }

	bool OnGossipHello(Player* player, Creature* creature) override
	{
		// Show your custom gossip menu
		player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Vaelastraz, no!!", GOSSIP_SENDER_MAIN, 1000);
		player->PlayerTalkClass->SendGossipMenu(907, creature->GetGUID()); // 1 = dummy, no DB text
		return true;
	}

	bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
	{
		player->PlayerTalkClass->ClearMenus();
		if (action == 1000)
		{
			player->PlayerTalkClass->SendCloseGossip();
			if (boss_vaelAI* ai = dynamic_cast<boss_vaelAI*>(creature->AI()))
				ai->BeginSpeech(player);
		}
		return true;
	}

	struct boss_vaelAI : public BossAI
	{
		boss_vaelAI(Creature* creature) : BossAI(creature, DATA_VAELASTRAZ_THE_CORRUPT)
		{
			Initialize();
		}

		void Initialize()
		{
			PlayerGUID = 0;
			HasYelled = false;
			_introDone = false;
			_burningAdrenalineCast = 0;
			me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
			me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
			me->setFaction(35); // Friendly ID
			me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
		}

		void Reset()
		{
			_Reset();
			me->setRegeneratingHealth(false);
			me->SetHealth(me->CountPctFromMaxHealth(30));

			if (!_introDone)
			{
				me->SetStandState(UNIT_STAND_STATE_DEAD);
				me->SetReactState(REACT_PASSIVE);
				Initialize();
				_eventsIntro.Reset();
			}
			else
			{
				HasYelled = false;
				_burningAdrenalineCast = 0;
			}
		}

		void EnterCombat(Unit* who)
		{
			BossAI::EnterCombat(who);

			DoCastAOE(SPELL_ESSENCE_OF_THE_RED);
			me->ResetPlayerDamageReq();

			events.ScheduleEvent(EVENT_CLEAVE, 10000);
			events.ScheduleEvent(EVENT_FLAME_BREATH, 15000);
			events.ScheduleEvent(EVENT_FIRE_NOVA, 5000);
			events.ScheduleEvent(EVENT_TAIL_SWEEP, 11000);
			events.ScheduleEvent(EVENT_BURNING_ADRENALINE, 15000);
		}

		void BeginSpeech(Unit* target)
		{
			PlayerGUID = target->GetGUID();
			me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
			_eventsIntro.ScheduleEvent(EVENT_SPEECH_1, 1000);
		}

		void KilledUnit(Unit* victim)
		{
			if (urand(0, 4))
				return;

			me->BossYell("Forgive me, friend! Your death only adds to my failure!", 8284);
		}

		void UpdateAI(uint32 diff)
		{
			events.Update(diff);
			_eventsIntro.Update(diff);

			if (!_introDone)
			{
				while (uint32 eventId = _eventsIntro.ExecuteEvent())
				{
					switch (eventId)
					{
					case EVENT_SPEECH_1:
						me->SetStandState(UNIT_STAND_STATE_STAND);
						me->SummonCreature(NPC_VICTOR_NEFARIUS, aNefariusSpawnLoc[0], aNefariusSpawnLoc[1], aNefariusSpawnLoc[2], aNefariusSpawnLoc[3], TEMPSUMMON_TIMED_DESPAWN, 26000);
						_eventsIntro.ScheduleEvent(EVENT_SPEECH_2, 1000);
						me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
						break;
					case EVENT_SPEECH_2:
						if (Creature* nefarius = me->FindNearestCreature(NPC_VICTOR_NEFARIUS, 30.0f))
						{
							nefarius->CastSpell(me, SPELL_NEFARIUS_CORRUPTION, TRIGGERED_CAST_DIRECTLY);
							nefarius->BossYell("Ah...the heroes. You are persistent, aren't you? Your ally here attempted to match his power against mine - and paid the price. Now he shall serve me...by slaughtering you.", 0);
							nefarius->SetStandState(UNIT_STAND_STATE_STAND);
						}
						_eventsIntro.ScheduleEvent(EVENT_SPEECH_3, 18000);
						break;
					case EVENT_SPEECH_3:
						if (Creature* nefarius = me->FindNearestCreature(NPC_VICTOR_NEFARIUS, 30.0f))
						{
							nefarius->CastSpell(me, SPELL_RED_LIGHTNING, TRIGGERED_NONE);
						}

						_eventsIntro.ScheduleEvent(EVENT_SPEECH_4, 2000);
						break;
					case EVENT_SPEECH_4:
						me->BossYell("Too late, friends! Nefarius' corruption has taken hold...I cannot...control myself.", 8281);
						me->HandleEmoteCommand(EMOTE_ONESHOT_TALK);
						_eventsIntro.ScheduleEvent(EVENT_SPEECH_5, 12000);
						break;
					case EVENT_SPEECH_5:
						me->BossYell("I beg you, mortals - FLEE! Flee before I lose all sense of control! The black fire rages within my heart! I MUST- release it!", 8282);
						me->HandleEmoteCommand(EMOTE_ONESHOT_TALK);
						_eventsIntro.ScheduleEvent(EVENT_SPEECH_6, 12000);
						break;
					case EVENT_SPEECH_6:
						me->BossYell("FLAME! DEATH! DESTRUCTION! Cower, mortals before the wrath of Lord...NO - I MUST fight this! Alexstrasza help me, I MUST fight it!", 8283);
						me->HandleEmoteCommand(EMOTE_ONESHOT_TALK);
						_eventsIntro.ScheduleEvent(EVENT_SPEECH_7, 17000);
						me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
						break;
					case EVENT_SPEECH_7:
						me->setFaction(103); // Black Dragonflight hostile
						if (PlayerGUID && ObjectAccessor::GetUnit(*me, PlayerGUID))
							AttackStart(ObjectAccessor::GetUnit(*me, PlayerGUID));
						me->SetReactState(REACT_AGGRESSIVE);
						_introDone = true;
						break;
					}
				}
			}

			if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
				return;

			while (uint32 eventId = events.ExecuteEvent())
			{
				switch (eventId)
				{
				case EVENT_CLEAVE:
					events.ScheduleEvent(EVENT_CLEAVE, 15000);
					DoCastVictim(SPELL_CLEAVE);
					break;
				case EVENT_FLAME_BREATH:
					DoCastVictim(SPELL_FLAME_BREATH);
					events.ScheduleEvent(EVENT_FLAME_BREATH, urand(8000, 14000));
					break;
				case EVENT_FIRE_NOVA:
					DoCastVictim(SPELL_FIRE_NOVA);
					events.ScheduleEvent(EVENT_FIRE_NOVA, urand(3000, 5000));
					break;
				case EVENT_TAIL_SWEEP:
					DoCastAOE(SPELL_TAIL_SWEEP);
					events.ScheduleEvent(EVENT_TAIL_SWEEP, 15000);
					break;
				case EVENT_BURNING_ADRENALINE:
				{
					if (_burningAdrenalineCast < 2)
					{
						Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true, -SPELL_BURNING_ADRENALINE);
						if (target &&
							target->GetTypeId() == TYPEID_PLAYER &&
							target->getPowerType() == POWER_MANA &&
							target != me->getVictim() &&
							!target->HasAura(SPELL_BURNING_ADRENALINE))
						{
							me->CastSpell(target, SPELL_BURNING_ADRENALINE, true);
							++_burningAdrenalineCast;
						}
					}
					else
					{
						if (Unit* victim = me->getVictim())
							me->CastSpell(victim, SPELL_BURNING_ADRENALINE, true);
						_burningAdrenalineCast = 0;
					}
					events.ScheduleEvent(EVENT_BURNING_ADRENALINE, 15000);
					break;
				}
				}
			}

			if (HealthBelowPct(15) && !HasYelled)
			{
				me->BossYell("Nefarius' hate has made me stronger than ever before! You should have fled while you could, mortals! The fury of Blackrock courses through my veins!", 8285);
				HasYelled = true;
			}

			DoMeleeAttackIfReady();
		}

		void JustSummoned(Creature* summoned)
		{
			if (summoned->GetEntry() == NPC_VICTOR_NEFARIUS)
			{
				summoned->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
				m_nefariusGuid = summoned->GetGUID();
			}
		}

	private:
		uint64 PlayerGUID;
		uint64 m_nefariusGuid;
		bool HasYelled;
		bool _introDone;
		EventMap _eventsIntro;
		uint8 _burningAdrenalineCast;
	};

	CreatureAI* GetAI(Creature* creature) const
	{
		return new boss_vaelAI(creature);
	}
};

// Burning Adrenaline
class spell_vael_burning_adrenaline : public AuraScript
{
	PrepareAuraScript(spell_vael_burning_adrenaline);

	bool Validate(SpellEntry const* /*spellEntry*/)
	{
		return true;
	}

	void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
	{
		if (!GetTarget())
			return;

		GetTarget()->CastSpell(GetTarget(), SPELL_BURNING_ADRENALINE_EXPLOSION, true);
		GetTarget()->CastSpell(GetTarget(), SPELL_BURNING_ADRENALINE_INSTAKILL, true);
	}

	void Register() override
	{
		AfterEffectRemove += AuraEffectRemoveFn(spell_vael_burning_adrenaline::HandleRemove, EFFECT_2, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
	}
};

void AddSC_boss_vaelastrasz()
{
	new boss_vaelastrasz();
	new spell_vael_burning_adrenaline();
}