
#include "ScriptPCH.h"
#include "the_stonecore.h"

enum Spells
{
	SPELL_CURSE_OF_BLOOD = 79345,
	SPELL_CURSE_OF_BLOOD_H = 92663,
	SPELL_ARCANE_SHIELD = 79050,
	SPELL_ARCANE_SHIELD_H = 92667,
	SPELL_SUMMON_GRAVITY_WELL = 79340,
	SPELL_GRAVITY_WELL_AURA_0 = 79245,
	SPELL_GRAVITY_WELL_AURA_1 = 79244,
	SPELL_GRAVITY_WELL_AURA_1_T = 79251,
	SPELL_GRAVITY_WELL_AURA_MOD = 92475,
	SPELL_GRAVITY_WELL_AURA_DMG = 79249,
};

enum Events
{
	EVENT_CURSE_OF_BLOOD = 1,
	EVENT_SHIELD = 2,
	EVENT_GRAVITY_WELL = 3,
	EVENT_GRAVITY_WELL_1 = 4,
	EVENT_SUMMON_ADDS = 5,
	EVENT_MOVE = 6,
	EVENT_MOVE_2 = 7,
	EVENT_ROTTEN_TO_THE_CORE = 8,
};

enum Adds
{
	NPC_ADVOUT_FOLLOWER = 42428,
	NPC_GRAVITY_WELL = 42499,
};

Position highpriestessazilAddsPos[2] =
{
	{1263.20f, 966.03f, 205.81f, 0.0f},
	{1278.51f, 1021.72f, 209.08f, 0.0f}
};

Position highpriestessazilStandPos = { 1337.79f, 963.39f, 214.18f, 1.8f };
uint32 DiscipleAmt;
bool RottenToTheCore;

class boss_priestess_azil : public CreatureScript
{
public:
	boss_priestess_azil() : CreatureScript("boss_priestess_azil") { }

	CreatureAI* GetAI(Creature* pCreature) const
	{
		return new boss_priestess_azilAI(pCreature);
	}

	struct boss_priestess_azilAI : public BossAI
	{
		boss_priestess_azilAI(Creature* pCreature) : BossAI(pCreature, DATA_HIGH_PRIESTESS_AZIL), summons(me)
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
		}

		EventMap events;
		SummonList summons;

		/*void InitializeAI()
		{
			if (!instance || static_cast<InstanceMap*>(me->GetMap())->GetScriptId() != sObjectMgr->GetScriptId(TSScriptName))
				me->IsAIEnabled = false;
			else if (!me->isDead())
				Reset();
		}*/

		void Reset()
		{
			_Reset();

			DiscipleAmt = 0;
			RottenToTheCore = false;
			summons.DespawnAll();
			events.Reset();
		}

		void JustSummoned(Creature* summon)
		{
			summons.Summon(summon);
			switch (summon->GetEntry())
			{
			case NPC_ADVOUT_FOLLOWER:
				if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
				{
					summon->AddThreat(target, 10.0f);
					summon->Attack(target, true);
					summon->GetMotionMaster()->MoveChase(target);
				}
				break;
			}
		}

		void SummonedCreatureDespawn(Creature* summon)
		{
			summons.Despawn(summon);
		}

		void EnterCombat(Unit* who)
		{
			me->BossYell("The world will be reborn in flames!", 21634);
			events.ScheduleEvent(EVENT_SHIELD, 45000);
			events.ScheduleEvent(EVENT_CURSE_OF_BLOOD, urand(5000, 8000));
			events.ScheduleEvent(EVENT_SUMMON_ADDS, urand(10000, 15000));
			events.ScheduleEvent(EVENT_GRAVITY_WELL, 10000);
			events.ScheduleEvent(EVENT_ROTTEN_TO_THE_CORE, 10000); // Achievement Check
			instance->SetBossState(DATA_HIGH_PRIESTESS_AZIL, IN_PROGRESS);
		}

		void KilledUnit(Unit * victim)
		{
			if (victim->GetTypeId() == TYPEID_PLAYER)
				me->BossYell("A sacrifice for you, master.", 21635);
		}

		void JustDied(Unit* killer)
		{
			_JustDied();
			summons.DespawnAll();
			me->BossYell("For my death, countless more will fall. The burden is now yours to bear.", 21633);

			if (RottenToTheCore == true && IsHeroic())
			{
				AchievementEntry const* RottenToThecore = sAchievementStore.LookupEntry(5287);
				Map::PlayerList const& players = me->GetMap()->GetPlayers();
				if (!players.isEmpty())
				{
					for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
					{
						if (Player* player = itr->getSource())
								player->CompletedAchievement(RottenToThecore);
					}
				}
			}

			if (IsHeroic())
			{
				me->RewardCurrency(CURRENCY_TYPE_JUSTICE_POINTS, 70);
			}
		}

		void MovementInform(uint32 type, uint32 id)
		{
			if (type == POINT_MOTION_TYPE)
			{
				switch (id)
				{
				case 1:
					events.ScheduleEvent(EVENT_MOVE_2, 20000);
					break;
				}
			}
		}

		void UpdateAI(const uint32 diff)
		{
			if (!UpdateVictim())
				return;

			events.Update(diff);

			if (me->HasUnitState(UNIT_STATE_CASTING))
				return;

			while (uint32 eventId = events.ExecuteEvent())
			{
				switch (eventId)
				{
				case EVENT_SUMMON_ADDS:
					for (uint8 i = 0; i < 3; i++)
						me->SummonCreature(NPC_ADVOUT_FOLLOWER, highpriestessazilAddsPos[urand(0, 1)]);
					events.ScheduleEvent(EVENT_SUMMON_ADDS, urand(5000, 7000));
					break;
				case EVENT_CURSE_OF_BLOOD:
					DoCast(DUNGEON_MODE(SPELL_CURSE_OF_BLOOD, SPELL_CURSE_OF_BLOOD_H));
					events.ScheduleEvent(EVENT_CURSE_OF_BLOOD, 15000);
					break;
				case EVENT_GRAVITY_WELL:
					if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
						DoCast(target, SPELL_SUMMON_GRAVITY_WELL);
					events.ScheduleEvent(EVENT_GRAVITY_WELL, urand(15000, 20000));
					break;
				case EVENT_SHIELD:
					me->BossYell("Witness the power bestowed upon me by Deathwing! Feel the fury of earth!", 21628);
					SetCombatMovement(false);
					events.CancelEvent(EVENT_CURSE_OF_BLOOD);
					DoCast(me, DUNGEON_MODE(SPELL_ARCANE_SHIELD, SPELL_ARCANE_SHIELD_H));
					events.ScheduleEvent(EVENT_MOVE, 2000);
					break;
				case EVENT_MOVE:
					me->GetMotionMaster()->MovePoint(1, highpriestessazilStandPos);
					break;
				case EVENT_MOVE_2:
					me->RemoveAurasDueToSpell(DUNGEON_MODE(SPELL_ARCANE_SHIELD, SPELL_ARCANE_SHIELD_H));
					SetCombatMovement(true);
					if (me->getVictim())
						me->GetMotionMaster()->MoveChase(me->getVictim());
					events.ScheduleEvent(EVENT_CURSE_OF_BLOOD, 3000);
					events.ScheduleEvent(EVENT_SHIELD, urand(40000, 45000));
					break;
				case EVENT_ROTTEN_TO_THE_CORE:
					if (DiscipleAmt >= 60) // Check if players killed 60 or more disciples and grant achievement
					{
						RottenToTheCore = true;
					}
					else // Players didn't make it so reset the counter
					{
						DiscipleAmt = 0;
					}
					if (RottenToTheCore == false)
						events.ScheduleEvent(EVENT_ROTTEN_TO_THE_CORE, 10000); // Check every 10 seconds
					break;
				}
			}

			DoMeleeAttackIfReady();
		}
	};
};

class npc_disciple_boss : public CreatureScript
{
public:
	npc_disciple_boss() : CreatureScript("npc_disciple_boss") { }

	CreatureAI* GetAI(Creature* pCreature) const
	{
		return new npc_disciple_bossAI(pCreature);
	}

	struct npc_disciple_bossAI : public ScriptedAI
	{
		npc_disciple_bossAI(Creature *c) : ScriptedAI(c)
		{
		}

		void JustDied(Unit* /*Kill*/)
		{
			Creature *azil = me->FindNearestCreature(BOSS_HIGH_PRIESTESS_AZIL, 200.0f);
			if (azil)
			{
				if (azil->isInCombat())
				{
					++DiscipleAmt; // Add to the amount
				}
			}
		}

	};
};

class npc_gravity_well : public CreatureScript
{
public:
	npc_gravity_well() : CreatureScript("npc_gravity_well") { }

	CreatureAI* GetAI(Creature* pCreature) const
	{
		return new npc_gravity_wellAI(pCreature);
	}

	struct npc_gravity_wellAI : public ScriptedAI
	{
		npc_gravity_wellAI(Creature *c) : ScriptedAI(c)
		{
		}

		EventMap events;
		uint32 uidespawnTimer;

		void Reset()
		{
			me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
			me->SetReactState(REACT_PASSIVE);
			uidespawnTimer = 20000;
			events.ScheduleEvent(EVENT_GRAVITY_WELL_1, 3000);
			DoCast(me, SPELL_GRAVITY_WELL_AURA_0);
		}

		void UpdateAI(const uint32 diff)
		{
			if (uidespawnTimer <= diff)
				me->DespawnOrUnsummon();
			else
				uidespawnTimer -= diff;

			events.Update(diff);

			while (uint32 eventId = events.ExecuteEvent())
			{
				switch (eventId)
				{
				case EVENT_GRAVITY_WELL_1:
					me->RemoveAurasDueToSpell(SPELL_GRAVITY_WELL_AURA_0);
					DoCast(me, SPELL_GRAVITY_WELL_AURA_1);
					break;
				}
			}
		}
	};
};

class spell_priestess_azil_gravity_well_script : public SpellScriptLoader
{
public:
	spell_priestess_azil_gravity_well_script() : SpellScriptLoader("spell_priestess_azil_gravity_well_script") { }


	class spell_priestess_azil_gravity_well_script_SpellScript : public SpellScript
	{
		PrepareSpellScript(spell_priestess_azil_gravity_well_script_SpellScript);


		void HandleScript(SpellEffIndex /*effIndex*/)
		{
			if (GetCaster() && GetHitUnit())
			{
				GetCaster()->CastSpell(GetHitUnit(), SPELL_GRAVITY_WELL_AURA_DMG, true);
			}
		}

		void Register()
		{
			OnEffectHitTarget += SpellEffectFn(spell_priestess_azil_gravity_well_script_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
		}
	};

	SpellScript* GetSpellScript() const
	{
		return new spell_priestess_azil_gravity_well_script_SpellScript();
	}
};

void AddSC_boss_priestess_azil()
{
	new boss_priestess_azil();
	new npc_disciple_boss();
	new npc_gravity_well();
	new spell_priestess_azil_gravity_well_script();
}