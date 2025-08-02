/*
 * WarAxeCore - Blackwing Lair: Broodlord Lashlayer & Suppression Devices
 */

#include "ScriptPCH.h"
#include "GameObject.h"
#include "GameObjectAI.h"
#include "InstanceScript.h"
#include "ScriptedCreature.h"
#include "blackwing_lair.h"

enum Say
{
	SAY_AGGRO = 0,
	SAY_LEASH = 1
};

enum Spells
{
	SPELL_CLEAVE = 26350,
	SPELL_BLASTWAVE = 23331,
	SPELL_MORTALSTRIKE = 24573,
	SPELL_KNOCKBACK = 25778,
	SPELL_SUPPRESSION_AURA = 22247
};

enum Events
{
	EVENT_CLEAVE = 1,
	EVENT_BLASTWAVE = 2,
	EVENT_MORTALSTRIKE = 3,
	EVENT_KNOCKBACK = 4,
	EVENT_CHECK = 5,
	EVENT_SUPPRESSION_CAST = 6,
	EVENT_SUPPRESSION_RESET = 7
};

enum Actions
{
	ACTION_DEACTIVATE = 0,
	ACTION_DISARMED = 1
};

class boss_broodlord : public CreatureScript
{
public:
	boss_broodlord() : CreatureScript("boss_broodlord") { }

	struct boss_broodlordAI : public BossAI
	{
		boss_broodlordAI(Creature* creature) : BossAI(creature, DATA_BROODLORD_LASHLAYER) { }

		void EnterCombat(Unit* who)
		{
			BossAI::EnterCombat(who);
			Talk(SAY_AGGRO);

			events.ScheduleEvent(EVENT_CLEAVE, 8000);
			events.ScheduleEvent(EVENT_BLASTWAVE, 12000);
			events.ScheduleEvent(EVENT_MORTALSTRIKE, 20000);
			events.ScheduleEvent(EVENT_KNOCKBACK, 30000);
			events.ScheduleEvent(EVENT_CHECK, 1000);
		}

		void JustDied(Unit* /*killer*/)
		{
			_JustDied();

			std::list<GameObject*> goList;
			GetGameObjectListWithEntryInGrid(goList, me, GO_SUPPRESSION_DEVICE, 200.0f);
			for (std::list<GameObject*>::const_iterator itr = goList.begin(); itr != goList.end(); ++itr)
			{
				((*itr)->AI()->DoAction(ACTION_DEACTIVATE));
			}
		}

		void UpdateAI(uint32 diff)
		{
			if (!UpdateVictim())
				return;

			events.Update(diff);

			while (uint32 eventId = events.ExecuteEvent())
			{
				switch (eventId)
				{
				case EVENT_CLEAVE:
					DoCastVictim(SPELL_CLEAVE);
					events.ScheduleEvent(EVENT_CLEAVE, 7000);
					break;
				case EVENT_BLASTWAVE:
					DoCastVictim(SPELL_BLASTWAVE);
					events.ScheduleEvent(EVENT_BLASTWAVE, urand(20000, 35000));
					break;
				case EVENT_MORTALSTRIKE:
					DoCastVictim(SPELL_MORTALSTRIKE);
					events.ScheduleEvent(EVENT_MORTALSTRIKE, urand(25000, 35000));
					break;
				case EVENT_KNOCKBACK:
					DoCastVictim(SPELL_KNOCKBACK);
					if (DoGetThreat(me->getVictim()))
						me->getThreatManager().modifyThreatPercent(me->getVictim(), -50.0f);
					events.ScheduleEvent(EVENT_KNOCKBACK, urand(15000, 30000));
					break;
				case EVENT_CHECK:
					if (me->GetDistance(me->GetHomePosition()) > 150.0f)
					{
						Talk(SAY_LEASH);
						EnterEvadeMode();
					}
					events.ScheduleEvent(EVENT_CHECK, 1000);
					break;
				}
			}

			DoMeleeAttackIfReady();
		}
	};

	CreatureAI* GetAI(Creature* creature) const
	{
		return new boss_broodlordAI(creature);
	}
};

class go_suppression_device : public GameObjectScript
{
public:
	go_suppression_device() : GameObjectScript("go_suppression_device") { }

	void OnLootStateChanged(GameObject* go, uint32 state, Unit* /*unit*/)
	{
		switch (state)
		{
		case GO_JUST_DEACTIVATED:
			go->SetLootState(GO_READY);
			// [[fallthrough]]; // Not supported in VS2015
		case GO_ACTIVATED:
			go->AI()->DoAction(ACTION_DISARMED);
			break;
		}
	}

	struct go_suppression_deviceAI : public GameObjectAI
	{
		go_suppression_deviceAI(GameObject* go)
			: GameObjectAI(go), _instance(go->GetInstanceScript()), _active(true) { }

		void InitializeAI()
		{
			if (_instance && _instance->GetBossState(DATA_BROODLORD_LASHLAYER) == DONE)
			{
				Deactivate();
				return;
			}
			_events.ScheduleEvent(EVENT_SUPPRESSION_CAST, 5000);
		}

		void UpdateAI(uint32 diff)
		{
			_events.Update(diff);

			while (uint32 eventId = _events.ExecuteEvent())
			{
				switch (eventId)
				{
				case EVENT_SUPPRESSION_CAST:
					if (go->GetGoState() == GO_STATE_READY)
					{
						go->CastSpell(nullptr, SPELL_SUPPRESSION_AURA);
						go->SendCustomAnim(0);
					}
					_events.ScheduleEvent(EVENT_SUPPRESSION_CAST, 5000);
					break;
				case EVENT_SUPPRESSION_RESET:
					Activate();
					break;
				}
			}
		}

		void DoAction(int32 action)
		{
			if (action == ACTION_DEACTIVATE)
			{
				_events.CancelEvent(EVENT_SUPPRESSION_RESET);
			}
			else if (action == ACTION_DISARMED)
			{
				Deactivate();
				_events.CancelEvent(EVENT_SUPPRESSION_CAST);
				if (_instance && _instance->GetBossState(DATA_BROODLORD_LASHLAYER) != DONE)
				{
					_events.ScheduleEvent(EVENT_SUPPRESSION_RESET, urand(30000, 120000));
				}
			}
		}

		void Activate()
		{
			if (_active)
				return;
			_active = true;
			if (go->GetGoState() == GO_STATE_ACTIVE)
				go->SetGoState(GO_STATE_READY);
			go->SetLootState(GO_READY);
			go->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
			_events.ScheduleEvent(EVENT_SUPPRESSION_CAST, 5000);
			go->Respawn();
		}

		void Deactivate()
		{
			if (!_active)
				return;
			_active = false;
			go->SetGoState(GO_STATE_ACTIVE);
			go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
			_events.CancelEvent(EVENT_SUPPRESSION_CAST);
		}

	private:
		InstanceScript* _instance;
		EventMap _events;
		bool _active;
	};

	GameObjectAI* GetAI(GameObject* go) const
	{
		return new go_suppression_deviceAI(go);
	}
};

class spell_suppression_aura : public SpellScript
{
	PrepareSpellScript(spell_suppression_aura);

	void FilterTargets(std::list<WorldObject*>& targets)
	{
		targets.remove_if([&](WorldObject* target) -> bool
		{
			Unit* unit = target->ToUnit();
			return !unit || unit->HasAuraType(SPELL_AURA_MOD_STEALTH);
		});
	}

	void Register()
	{
		OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_suppression_aura::FilterTargets, EFFECT_ALL, TARGET_UNIT_DEST_AREA_ENEMY);
	}
};

void AddSC_boss_broodlord()
{
	new boss_broodlord();
	new go_suppression_device();
	new spell_suppression_aura();
}