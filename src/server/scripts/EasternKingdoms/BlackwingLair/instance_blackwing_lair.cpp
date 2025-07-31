/*
 * This file is part of the WarAxeCore Project.
 */

#include "ScriptPCH.h"
#include "blackwing_lair.h"

DoorData const doorData[] =
{
	{ GO_PORTCULLIS_RAZORGORE,      DATA_RAZORGORE_THE_UNTAMED,  DOOR_TYPE_PASSAGE },
	{ GO_PORTCULLIS_RAZORGORE_ROOM, DATA_RAZORGORE_THE_UNTAMED,  DOOR_TYPE_ROOM   },
	{ GO_PORTCULLIS_VAELASTRASZ,    DATA_VAELASTRAZ_THE_CORRUPT, DOOR_TYPE_PASSAGE },
	{ GO_PORTCULLIS_BROODLORD,      DATA_BROODLORD_LASHLAYER,    DOOR_TYPE_PASSAGE },
	{ GO_PORTCULLIS_CHROMAGGUS_EXIT,DATA_CHROMAGGUS,             DOOR_TYPE_PASSAGE },
	{ GO_PORTCULLIS_CHROMAGGUS_EXIT,DATA_NEFARIAN,               DOOR_TYPE_ROOM    },
	{ GO_PORTCULLIS_NEFARIAN,       DATA_NEFARIAN,               DOOR_TYPE_ROOM    },
	{ 0,                            0,                           DOOR_TYPE_ROOM    }
};

struct ObjectData
{
	uint32 entry;
	uint32 type;
};

static ObjectData const creatureData[] =
{
	{ NPC_GRETHOK,         DATA_GRETHOK              },
	{ NPC_NEFARIAN_TROOPS, DATA_NEFARIAN_TROOPS      },
	{ NPC_VICTOR_NEFARIUS, DATA_LORD_VICTOR_NEFARIUS },
	{ NPC_CHROMAGGUS,      DATA_CHROMAGGUS           },
	{ 0,                   0                         }
};

static ObjectData const objectData[] =
{
	{ GO_PORTCULLIS_CHROMAGGUS,      DATA_GO_CHROMAGGUS_DOOR      },
	{ GO_PORTCULLIS_CHROMAGGUS_EXIT, DATA_GO_CHROMAGGUS_DOOR_EXIT },
	{ 0,                             0                            }
};

static Position const SummonPosition[8] =
{
	{ -7661.207520f, -1043.268188f, 407.199554f, 6.280452f },
	{ -7644.145020f, -1065.628052f, 407.204956f, 0.501492f },
	{ -7624.260742f, -1095.196899f, 407.205017f, 0.544694f },
	{ -7608.501953f, -1116.077271f, 407.199921f, 0.816443f },
	{ -7531.841797f, -1063.765381f, 407.199615f, 2.874187f },
	{ -7547.319336f, -1040.971924f, 407.205078f, 3.789175f },
	{ -7568.547852f, -1013.112488f, 407.204926f, 3.773467f },
	{ -7584.175781f, -989.6691289f, 407.199585f, 4.527447f }
};

static uint32 const Entry[3] = { NPC_BLACKWING_DRAGON, NPC_BLACKWING_LEGIONAIRE, NPC_BLACKWING_MAGE };

class instance_blackwing_lair : public InstanceMapScript
{
public:
	instance_blackwing_lair() : InstanceMapScript("instance_blackwing_lair", 469) { }

	struct instance_blackwing_lair_InstanceMapScript : public InstanceScript
	{
		instance_blackwing_lair_InstanceMapScript(Map* map) : InstanceScript(map) { }

		void Initialize()
		{
			EggCount = 0;
			EggEvent = NOT_STARTED;
			NefarianLeftTunnel = 0;
			NefarianRightTunnel = 0;
			addsCount[0] = 0;
			addsCount[1] = 0;
			EggList.clear();
			guardList.clear();
		}

		void OnCreatureCreate(Creature* creature)
		{
			if (creature->GetEntry() == NPC_VICTOR_NEFARIUS && creature->ToTempSummon())
				return;

			InstanceScript::OnCreatureCreate(creature);

			switch (creature->GetEntry())
			{
			case NPC_NEFARIAN_TROOPS:
				nefarianTroopsGUID = creature->GetGUID();
				break;
			case NPC_RAZORGORE:
				razorgoreGUID = creature->GetGUID();
				sLog->outError("INSTANCE: razorgoreGUID set to: " UI64FMTD, razorgoreGUID);
				break;
			case NPC_GRETHOK:
				grethokGUID = creature->GetGUID();
				break;
			case NPC_BLACKWING_DRAGON:
				++addsCount[0];
				if (Creature* razor = instance->GetCreature(razorgoreGUID))
					if (razor->AI())
						razor->AI()->JustSummoned(creature);
				break;
			case NPC_BLACKWING_LEGIONAIRE:
			case NPC_BLACKWING_MAGE:
				++addsCount[1];
				if (Creature* razor = instance->GetCreature(razorgoreGUID))
					if (razor->AI())
						razor->AI()->JustSummoned(creature);
				break;
			case NPC_BLACKWING_GUARDSMAN:
				guardList.push_back(creature->GetGUID());
				break;
			case NPC_NEFARIAN:
				nefarianGUID = creature->GetGUID();
				break;
			case NPC_BLACK_DRAKONID:
			case NPC_BLUE_DRAKONID:
			case NPC_BRONZE_DRAKONID:
			case NPC_CHROMATIC_DRAKONID:
			case NPC_GREEN_DRAKONID:
			case NPC_RED_DRAKONID:
				if (Creature* nefarius = instance->GetCreature(victorNefariusGUID))
					if (nefarius->AI())
						nefarius->AI()->JustSummoned(creature);
				break;
			default:
				break;
			}
		}

		void OnGameObjectCreate(GameObject* go)
		{
			InstanceScript::OnGameObjectCreate(go);

			if (go->GetEntry() == GO_BLACK_DRAGON_EGG)
			{
				// Black dragon egg should be unselectable
				go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
				if (GetBossState(DATA_FIREMAW) == DONE)
					go->SetPhaseMask(2, true);
				else
					EggList.push_back(go->GetGUID());
			}
		}

		void OnGameObjectRemove(GameObject* go)
		{
			InstanceScript::OnGameObjectRemove(go);

			if (go->GetEntry() == GO_BLACK_DRAGON_EGG)
				EggList.remove(go->GetGUID());
		}

		uint32 GetData(uint32 type) const
		{
			switch (type)
			{
			case DATA_NEFARIAN_LEFT_TUNNEL:
				return NefarianLeftTunnel;
			case DATA_NEFARIAN_RIGHT_TUNNEL:
				return NefarianRightTunnel;
			case DATA_EGG_EVENT:
				return EggEvent;
			default:
				break;
			}
			return 0;
		}

		bool CheckRequiredBosses(uint32 bossId, Player const* /* player */) const
		{
			if (bossId == DATA_BROODLORD_LASHLAYER)
				if (GetBossState(DATA_VAELASTRAZ_THE_CORRUPT) != DONE)
					return false;
			return true;
		}

		bool SetBossState(uint32 type, EncounterState state)
		{
			if (!InstanceScript::SetBossState(type, state))
				return false;

			if (type == DATA_RAZORGORE_THE_UNTAMED && state == DONE)
			{
				for (std::list<uint64>::iterator itr = EggList.begin(); itr != EggList.end(); ++itr)
					if (GameObject* egg = instance->GetGameObject(*itr))
						egg->SetPhaseMask(2, true);
			}

			if (type == DATA_NEFARIAN)
			{
				if (state == FAIL || state == NOT_STARTED)
				{
					_events.ScheduleEvent(EVENT_RESPAWN_NEFARIUS, 15 * 60 * 1000);
					if (Creature* nefarian = instance->GetCreature(nefarianGUID))
						nefarian->DespawnOrUnsummon();
				}
			}
			return true;
		}

		void SetData(uint32 type, uint32 data)
		{
			if (type == DATA_EGG_EVENT)
			{
				switch (data)
				{
				case DONE:
					EggEvent = data;
					break;
				case FAIL:
					_events.CancelEvent(EVENT_RAZOR_SPAWN);
					break;
				case IN_PROGRESS:
					_events.ScheduleEvent(EVENT_RAZOR_SPAWN, 45 * 1000);
					EggEvent = data;
					EggCount = 0;
					addsCount[0] = 0;
					addsCount[1] = 0;
					break;
				case NOT_STARTED:
					_events.CancelEvent(EVENT_RAZOR_SPAWN);
					EggEvent = data;
					EggCount = 0;
					addsCount[0] = 0;
					addsCount[1] = 0;
					for (std::list<uint64>::iterator itr = EggList.begin(); itr != EggList.end(); ++itr)
						DoRespawnGameObject(*itr, 0);
					DoRespawnCreature(grethokGUID);
					for (std::list<uint64>::iterator itr = guardList.begin(); itr != guardList.end(); ++itr)
						DoRespawnCreature(*itr);
					break;
				case SPECIAL:
					if (EggEvent == NOT_STARTED)
						SetData(DATA_EGG_EVENT, IN_PROGRESS);
					if (++EggCount >= EggList.size())
					{
						if (Creature* razor = instance->GetCreature(razorgoreGUID))
						{
							SetData(DATA_EGG_EVENT, DONE);
							razor->RemoveAurasDueToSpell(19832);
							DoRemoveAurasDueToSpellOnPlayers(19832);
						}
						_events.ScheduleEvent(EVENT_RAZOR_PHASE_TWO, 1000);
						_events.CancelEvent(EVENT_RAZOR_SPAWN);
					}
					break;
				}
			}
			if (type == DATA_NEFARIAN_LEFT_TUNNEL)
				NefarianLeftTunnel = data;
			if (type == DATA_NEFARIAN_RIGHT_TUNNEL)
				NefarianRightTunnel = data;
		}

		uint64 GetData64(uint32 type) const
		{
			switch (type)
			{
			case DATA_RAZORGORE_THE_UNTAMED:
				return razorgoreGUID;
			case DATA_LORD_VICTOR_NEFARIUS:
				return victorNefariusGUID;
			default:
				break;
			}
			return 0;
		}

		void OnCreatureDeath(Creature* creature)
		{
			switch (creature->GetEntry())
			{
			case NPC_BLACKWING_DRAGON:
				if (addsCount[0] > 0) --addsCount[0];
				if (EggEvent != DONE)
					_events.ScheduleEvent(EVENT_RAZOR_SPAWN, 1000);
				break;
			case NPC_BLACKWING_LEGIONAIRE:
			case NPC_BLACKWING_MAGE:
				if (addsCount[1] > 0) --addsCount[1];
				if (EggEvent != DONE)
					_events.ScheduleEvent(EVENT_RAZOR_SPAWN, 1000);
				break;
			default:
				break;
			}
		}

		void Update(uint32 diff)
		{
			_events.Update(diff);

			while (uint32 eventId = _events.ExecuteEvent())
			{
				switch (eventId)
				{
				case EVENT_RAZOR_SPAWN:
					if (EggEvent == IN_PROGRESS)
					{
						bool spawnMoreAdds = true;
						for (uint8 i = urand(2, 5); i > 0; --i)
						{
							uint32 mobEntry = Entry[urand(0, 2)];
							uint32 dragonkinsCount = addsCount[0];
							uint32 orcsCount = addsCount[1];

							if (dragonkinsCount >= 12)
							{
								if (orcsCount >= 40)
								{
									spawnMoreAdds = false;
									break;
								}
								else if (mobEntry == NPC_BLACKWING_DRAGON)
									continue;
							}
							else if (orcsCount >= 40 && mobEntry != NPC_BLACKWING_DRAGON)
								continue;

							if (Creature* summon = instance->SummonCreature(mobEntry, SummonPosition[urand(0, 7)]))
								if (summon->AI())
									summon->AI()->DoZoneInCombat();
						}
						if (spawnMoreAdds)
							_events.ScheduleEvent(EVENT_RAZOR_SPAWN, 15000);
					}
					break;
				case EVENT_RAZOR_PHASE_TWO:
					_events.CancelEvent(EVENT_RAZOR_SPAWN);
					if (Creature* razor = instance->GetCreature(razorgoreGUID))
						if (razor->AI())
							razor->AI()->DoAction(ACTION_PHASE_TWO);
					break;
				case EVENT_RESPAWN_NEFARIUS:
					if (Creature* nefarius = instance->GetCreature(GetData64(DATA_LORD_VICTOR_NEFARIUS)))
					{
						nefarius->SetPhaseMask(1, true);
						nefarius->setActive(true);
						nefarius->Respawn();
						nefarius->GetMotionMaster()->MoveTargetedHome();
					}
					break;
				}
			}
		}

		void ReadSaveDataMore(std::istringstream& data)
		{
			data >> NefarianLeftTunnel;
			data >> NefarianRightTunnel;
		}

		void WriteSaveDataMore(std::ostringstream& data)
		{
			data << NefarianLeftTunnel << ' ' << NefarianRightTunnel;
		}

		// Variables
		uint64 nefarianTroopsGUID = 0;
		uint64 grethokGUID = 0;
		uint64 razorgoreGUID = 0;
		uint64 nefarianGUID = 0;
		uint64 victorNefariusGUID = 0;

		uint8 EggCount = 0;
		uint32 EggEvent = NOT_STARTED;
		std::list<uint64> EggList;
		std::list<uint64> guardList;
		uint32 addsCount[2] = { 0, 0 };

		uint32 NefarianLeftTunnel = 0;
		uint32 NefarianRightTunnel = 0;
		EventMap _events;
	};

	InstanceScript* GetInstanceScript(InstanceMap* map) const override
	{
		return new instance_blackwing_lair_InstanceMapScript(map);
	}
};

enum ShadowFlame
{
	SPELL_ONYXIA_SCALE_CLOAK = 22683,
	SPELL_SHADOW_FLAME_DOT = 22682
};

class spell_bwl_shadowflame : public SpellScriptLoader
{
public:
	spell_bwl_shadowflame() : SpellScriptLoader("spell_bwl_shadowflame") { }

	class spell_bwl_shadowflame_SpellScript : public SpellScript
	{
		PrepareSpellScript(spell_bwl_shadowflame_SpellScript);

		void HandleEffectScriptEffect(SpellEffIndex /*effIndex*/)
		{
			if (Unit* victim = GetHitUnit())
				if (!victim->HasAura(SPELL_ONYXIA_SCALE_CLOAK))
					victim->AddAura(SPELL_SHADOW_FLAME_DOT, victim);
		}

		void Register() override
		{
			OnEffectHitTarget += SpellEffectFn(spell_bwl_shadowflame_SpellScript::HandleEffectScriptEffect, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
		}
	};

	SpellScript* GetSpellScript() const override
	{
		return new spell_bwl_shadowflame_SpellScript();
	}
};

enum orb_of_command_misc
{
	QUEST_BLACKHANDS_COMMAND = 7761
};

const Position orbOfCommandTP = { -7672.46f, -1107.19f, 396.65f, 0.59f };

class at_orb_of_command : public AreaTriggerScript
{
public:
	at_orb_of_command() : AreaTriggerScript("at_orb_of_command") { }

	bool OnTrigger(Player* player, AreaTrigger const* /*trigger*/)
	{
		if (player->isAlive() && player->GetQuestRewardStatus(QUEST_BLACKHANDS_COMMAND))
		{
			player->TeleportTo(469, orbOfCommandTP.GetPositionX(), orbOfCommandTP.GetPositionY(), orbOfCommandTP.GetPositionZ(), orbOfCommandTP.GetOrientation());
			return true;
		}
		return false;
	}
};

void AddSC_instance_blackwing_lair()
{
	new instance_blackwing_lair();
	new spell_bwl_shadowflame();
	new at_orb_of_command();
}