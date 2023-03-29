////////////////////////////////////////////////////////////////////////////////
//
//  MILLENIUM-STUDIO
//  Copyright 2016 Millenium-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

#include "ScriptPCH.h"
#include "halls_of_origination.h"

enum Spells
{
	SPELL_ALPHA_BEAMS = 76184,
	SPELL_ALPHA_BEAMS_AOE = 76904,
	SPELL_ALPHA_BEAM = 76912,
	SPELL_ALPHA_BEAM_DMG = 76956,
	SPELL_ALPHA_BEAM_DMG_H = 91177,
	SPELL_CRUMBLING_RUIN = 75609,
	SPELL_CRUMBLING_RUIN_H = 91206,
	SPELL_DESTRUCTION_PROTOCOL = 77437,
	SPELL_NEMESIS_STRIKE = 75604,
	SPELL_OMEGA_STANCE = 75622
};

enum Events
{
	EVENT_ALPHA_BEAMS = 1,
	EVENT_CRUMBLING_RUIN = 2,
	EVENT_DESTRUCTION_PROTOCOL = 3,
	EVENT_NEMESIS_STRIKE = 4,
	EVENT_INTRO = 5
};

enum Adds
{
	NPC_ALPHA_BEAM = 41133,
	NPC_OMEGA_STANCE = 41194, // 77137 77117
};

class boss_anraphet : public CreatureScript
{
public:
	boss_anraphet() : CreatureScript("boss_anraphet") {}

	CreatureAI* GetAI(Creature* pCreature) const
	{
		return new boss_anraphetAI(pCreature);
	}

	struct boss_anraphetAI : public BossAI
	{
		boss_anraphetAI(Creature* pCreature) : BossAI(pCreature, DATA_EARTHRAGER_PTAH)
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
			//me->SetFlag(UNIT_FIELD_FLAGS,UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_SELECTABLE);
		}

		uint8 spells;

		void Reset()
		{
			_Reset();

			spells = 0;
		}

		void EnterCombat(Unit * /*p_Who*/)
		{
			me->BossYell("Purge of unauthorized entities commencing.", 20862);

			events.ScheduleEvent(EVENT_ALPHA_BEAMS, 7000, 10000);
			events.ScheduleEvent(EVENT_NEMESIS_STRIKE, urand(3000, 7000));
			events.ScheduleEvent(EVENT_CRUMBLING_RUIN, 20000);

			DoZoneInCombat();
			instance->SetBossState(DATA_ANRAPHET, IN_PROGRESS);
		}

		void JustDied(Unit* /*killer*/)
		{
			_JustDied();
			me->BossYell("Anraphet unit shutting down...", 20856);
			instance->SetBossState(DATA_ANRAPHET, DONE);
		}

		void KilledUnit(Unit* /*p_Who*/)
		{
			me->BossYell("Purge Complete.", 20859);
		}

		void DoAction(const int32 action)
		{
			if (action == 1)
			{
				me->BossYell("This unit has been activated outside normal operating protocols. Downloading new operational parameters. Download complete. Full unit self defense routines are now active. Destruction of foreign units in this system shall now commence.", 20857);
				me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_SELECTABLE);
				me->SetHomePosition(-203.93f, 368.71f, 75.92f, me->GetOrientation());
				//DoTeleportTo(-203.93f, 368.71f, 75.92f);
				me->GetMotionMaster()->MovePoint(0, -203.93f, 368.71f, 75.92f);
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
				case EVENT_ALPHA_BEAMS:
					if (spells == 3)
					{
						me->BossYell("Omega Stance activated. Annihilation of foreign unit is now imminent.", 20861);
						DoCast(me, SPELL_OMEGA_STANCE);
						spells = 0;
					}
					else
					{
						if (spells == 0)
							me->BossYell("Alpha beams activated. Target tracking commencing.", 20860);
						DoCast(me, SPELL_ALPHA_BEAMS);
						spells++;
					}
					events.ScheduleEvent(EVENT_ALPHA_BEAMS, 15000);
					break;
				case EVENT_CRUMBLING_RUIN:
					DoCastAOE(SPELL_CRUMBLING_RUIN);
					events.ScheduleEvent(EVENT_CRUMBLING_RUIN, 40000);
					break;
				case EVENT_NEMESIS_STRIKE:
					DoCastVictim(SPELL_NEMESIS_STRIKE);
					events.ScheduleEvent(EVENT_NEMESIS_STRIKE, urand(15000, 20000));
					break;
				}
			}

			DoMeleeAttackIfReady();
		}
	};
};

class npc_alpha_beam : public CreatureScript
{
public:
	npc_alpha_beam() : CreatureScript("npc_alpha_beam") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_alpha_beamAI(creature);
	}

	struct npc_alpha_beamAI : public Scripted_NoMovementAI
	{
		npc_alpha_beamAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
		{
			pCreature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
			pCreature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
		}

		void Reset()
		{
			DoCast(me, SPELL_ALPHA_BEAM);
		}

		void UpdateAI(const uint32 /*p_Diff*/)
		{
		}
	};
};

void AddSC_boss_anraphet()
{
	new boss_anraphet();
	new npc_alpha_beam();
}