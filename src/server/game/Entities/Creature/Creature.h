/*
 * Copyright (C) 2011-2019 Project SkyFire <http://www.projectskyfire.org/>
 * Copyright (C) 2008-2019 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2019 MaNGOS <https://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SKYFIRE_CREATURE_H
#define SKYFIRE_CREATURE_H

#include "Common.h"
#include "Unit.h"
#include "UpdateMask.h"
#include "ItemPrototype.h"
#include "LootMgr.h"
#include "DatabaseEnv.h"
#include "Cell.h"

#include <list>

class SpellInfo;

class CreatureAI;
class Quest;
class Player;
class WorldSession;
class CreatureGroup;

enum CreatureFlagsExtra
{
    CREATURE_FLAG_EXTRA_INSTANCE_BIND   = 0x00000001,       // creature kill bind instance with killer and killer's group
    CREATURE_FLAG_EXTRA_CIVILIAN        = 0x00000002,       // not aggro (ignore faction/reputation hostility)
    CREATURE_FLAG_EXTRA_NO_PARRY        = 0x00000004,       // creature can't parry
    CREATURE_FLAG_EXTRA_NO_PARRY_HASTEN = 0x00000008,       // creature can't counter-attack at parry
    CREATURE_FLAG_EXTRA_NO_BLOCK        = 0x00000010,       // creature can't block
    CREATURE_FLAG_EXTRA_NO_CRUSH        = 0x00000020,       // creature can't do crush attacks
    CREATURE_FLAG_EXTRA_NO_XP_AT_KILL   = 0x00000040,       // creature kill not provide XP
    CREATURE_FLAG_EXTRA_TRIGGER         = 0x00000080,       // trigger creature
    CREATURE_FLAG_EXTRA_NO_TAUNT        = 0x00000100,       // creature is immune to taunt auras and effect attack me
    CREATURE_FLAG_EXTRA_WORLDEVENT      = 0x00004000,       // custom flag for world event creatures (left room for merging)
    CREATURE_FLAG_EXTRA_GUARD           = 0x00008000,       // Creature is guard
    CREATURE_FLAG_EXTRA_NO_CRIT         = 0x00020000,       // creature can't do critical strikes
    CREATURE_FLAG_EXTRA_NO_SKILLGAIN    = 0x00040000,       // creature won't increase weapon skills
    CREATURE_FLAG_EXTRA_TAUNT_DIMINISH  = 0x00080000,       // Taunt is a subject to diminishing returns on this creautre
    CREATURE_FLAG_EXTRA_ALL_DIMINISH    = 0x00100000,       // Creature is subject to all diminishing returns as player are
    CREATURE_FLAG_EXTRA_DUNGEON_BOSS    = 0x10000000,       // creature is a dungeon boss (SET DYNAMICALLY, DO NOT ADD IN DB)
};

#define CREATURE_FLAG_EXTRA_DB_ALLOWED (CREATURE_FLAG_EXTRA_INSTANCE_BIND | CREATURE_FLAG_EXTRA_CIVILIAN | \
    CREATURE_FLAG_EXTRA_NO_PARRY | CREATURE_FLAG_EXTRA_NO_PARRY_HASTEN | CREATURE_FLAG_EXTRA_NO_BLOCK | \
    CREATURE_FLAG_EXTRA_NO_CRUSH | CREATURE_FLAG_EXTRA_NO_XP_AT_KILL | CREATURE_FLAG_EXTRA_TRIGGER | \
    CREATURE_FLAG_EXTRA_NO_TAUNT | CREATURE_FLAG_EXTRA_WORLDEVENT | CREATURE_FLAG_EXTRA_NO_CRIT | \
    CREATURE_FLAG_EXTRA_NO_SKILLGAIN | CREATURE_FLAG_EXTRA_TAUNT_DIMINISH | CREATURE_FLAG_EXTRA_ALL_DIMINISH | \
    CREATURE_FLAG_EXTRA_GUARD)

// GCC have alternative #pragma pack(N) syntax and old gcc version not support pack(push, N), also any gcc version not support it at some platform
#if defined(__GNUC__)
#pragma pack(1)
#else
#pragma pack(push, 1)
#endif

#define MAX_KILL_CREDIT 2
#define CREATURE_REGEN_INTERVAL 2 * IN_MILLISECONDS

#define MAX_CREATURE_QUEST_ITEMS 6

#define MAX_EQUIPMENT_ITEMS 3

// from `creature_template` table
struct CreatureTemplate
{
    uint32  Entry;
    uint32  DifficultyEntry[MAX_DIFFICULTY - 1];
    uint32  KillCredit[MAX_KILL_CREDIT];
    uint32  Modelid1;
    uint32  Modelid2;
    uint32  Modelid3;
    uint32  Modelid4;
    std::string  Name;
    std::string  SubName;
    std::string  IconName;
    uint32  GossipMenuId;
    uint8   minlevel;
    uint8   maxlevel;
    uint32  expansion;
    uint32  faction_A;
    uint32  faction_H;
    uint32  npcflag;
    float   speed_walk;
    float   speed_run;
    float   scale;
    uint32  rank;
    float   mindmg;
    float   maxdmg;
    uint32  dmgschool;
    uint32  attackpower;
    float   dmg_multiplier;
    uint32  baseattacktime;
    uint32  rangeattacktime;
    uint32  unit_class;                                     // enum Classes. Note only 4 classes are known for creatures.
    uint32  unit_flags;                                     // enum UnitFlags mask values
    uint32  dynamicflags;
    uint32  family;                                         // enum CreatureFamily values (optional)
    uint32  trainer_type;
    uint32  trainer_spell;
    uint32  trainer_class;
    uint32  trainer_race;
    float   minrangedmg;
    float   maxrangedmg;
    uint32  rangedattackpower;
    uint32  type;                                           // enum CreatureType values
    uint32  type_flags;                                     // enum CreatureTypeFlags mask values
    uint32  lootid;
    uint32  pickpocketLootId;
    uint32  SkinLootId;
    int32   resistance[MAX_SPELL_SCHOOL];
    uint32  spells[CREATURE_MAX_SPELLS];
    uint32  PetSpellDataId;
    uint32  VehicleId;
    uint32  mingold;
    uint32  maxgold;
    std::string AIName;
    uint32  MovementType;
    uint32  InhabitType;
    float   ModHealth;
    float   ModMana;
    float   ModArmor;
    bool    RacialLeader;
    uint32  questItems[MAX_CREATURE_QUEST_ITEMS];
    uint32  movementId;
    bool    RegenHealth;
    uint32  equipmentId;
    uint32  MechanicImmuneMask;
    uint32  flags_extra;
    uint32  ScriptID;
    uint32  GetRandomValidModelId() const;
    uint32  GetFirstValidModelId() const;

    // helpers
    SkillType GetRequiredLootSkill() const
    {
        if (type_flags & CREATURE_TYPEFLAGS_HERBLOOT)
            return SKILL_HERBALISM;
        else if (type_flags & CREATURE_TYPEFLAGS_MININGLOOT)
            return SKILL_MINING;
        else if (type_flags & CREATURE_TYPEFLAGS_ENGINEERLOOT)
            return SKILL_ENGINEERING;
        else
            return SKILL_SKINNING;                          // normal case
    }

    bool isTameable(bool exotic) const
    {
        if (type != CREATURE_TYPE_BEAST || family == 0 || (type_flags & CREATURE_TYPEFLAGS_TAMEABLE) == 0)
            return false;

        // if can tame exotic then can tame any temable
        return exotic || (type_flags & CREATURE_TYPEFLAGS_EXOTIC) == 0;
    }
};

// Benchmarked: Faster than std::map (insert/find)
typedef UNORDERED_MAP<uint32, CreatureTemplate> CreatureTemplateContainer;

// Represents max amount of expansions.
// TODO: Add MAX_EXPANSION constant. hmmmz.bump~this needs scanned.
#define MAX_CREATURE_BASE_HP 4
#define MAX_EXPANSION 4

// Defines base stats for creatures (used to calculate HP/mana/armor).
struct CreatureBaseStats
{
    uint32 BaseHealth[MAX_CREATURE_BASE_HP];
    uint32 BaseMana;
    uint32 BaseArmor;

    // Helpers

    uint32 GenerateHealth(CreatureTemplate const* info) const
    {
        return uint32((BaseHealth[info->expansion] * info->ModHealth) + 0.5f);
    }

    uint32 GenerateMana(CreatureTemplate const* info) const
    {
        // Mana can be 0.
        if (!BaseMana)
            return 0;

        return uint32((BaseMana * info->ModMana) + 0.5f);
    }

    uint32 GenerateArmor(CreatureTemplate const* info) const
    {
        return uint32((BaseArmor * info->ModArmor) + 0.5f);
    }

    static CreatureBaseStats const* GetBaseStats(uint8 level, uint8 unitClass);
};

typedef UNORDERED_MAP<uint16, CreatureBaseStats> CreatureBaseStatsContainer;

struct CreatureLocale
{
    StringVector Name;
    StringVector SubName;
};

struct GossipMenuItemsLocale
{
    StringVector OptionText;
    StringVector BoxText;
};

struct PointOfInterestLocale
{
    StringVector IconName;
};

struct EquipmentInfo
{
    uint32  ItemEntry[MAX_EQUIPMENT_ITEMS];
};

// Benchmarked: Faster than std::map (insert/find)
typedef UNORDERED_MAP<uint16, EquipmentInfo> EquipmentInfoContainer;

// from `creature` table
struct CreatureData
{
    explicit CreatureData() : dbData(true) {}
    uint32 id;                                              // entry in creature_template
    uint16 mapid;
    uint16 phaseMask;
    uint32 displayid;
    int32 equipmentId;
    float posX;
    float posY;
    float posZ;
    float orientation;
    uint32 spawntimesecs;
    float spawndist;
    uint32 currentwaypoint;
    uint32 curhealth;
    uint32 curmana;
    uint8 movementType;
    uint8 spawnMask;
    uint32 npcflag;
    uint32 unit_flags;                                      // enum UnitFlags mask values
    uint32 dynamicflags;
    bool dbData;
	uint32 dbGuid;
};

// `creature_addon` table
struct CreatureAddon
{
    uint32 guidOrEntry;
    uint32 path_id;
    uint32 mount;
    uint32 bytes1;
    uint32 bytes2;
    uint32 emote;
    std::vector<uint32> auras;
};

typedef UNORDERED_MAP<uint32, CreatureAddon> CreatureAddonContainer;

struct CreatureModelInfo
{
    float bounding_radius;
    float combat_reach;
    uint8 gender;
    uint32 modelid_other_gender;
};

// Benchmarked: Faster than std::map (insert/find)
typedef UNORDERED_MAP<uint16, CreatureModelInfo> CreatureModelContainer;

enum InhabitTypeValues
{
    INHABIT_GROUND = 1,
    INHABIT_WATER  = 2,
    INHABIT_AIR    = 4,
    INHABIT_ANYWHERE = INHABIT_GROUND | INHABIT_WATER | INHABIT_AIR
};

// Enums used by StringTextData::Type (CreatureEventAI)
enum ChatType
{
    CHAT_TYPE_SAY               = 0,
    CHAT_TYPE_YELL              = 1,
    CHAT_TYPE_TEXT_EMOTE        = 2,
    CHAT_TYPE_BOSS_EMOTE        = 3,
    CHAT_TYPE_WHISPER           = 4,
    CHAT_TYPE_BOSS_WHISPER      = 5,
    CHAT_TYPE_ZONE_YELL         = 6,
    CHAT_TYPE_END               = 255
};

// GCC have alternative #pragma pack() syntax and old gcc version not support pack(pop), also any gcc version not support it at some platform
#if defined(__GNUC__)
#pragma pack()
#else
#pragma pack(pop)
#endif

// Vendors
struct VendorItem
{
    VendorItem(uint32 _item, int32 _maxcount, uint32 _incrtime, uint32 _ExtendedCost)
        : item(_item), maxcount(_maxcount), incrtime(_incrtime), ExtendedCost(_ExtendedCost) {}

    uint32 item;
    uint32  maxcount;                                       // 0 for infinity item amount
    uint32 incrtime;                                        // time for restore items amount if maxcount != 0
    uint32 ExtendedCost;

    //helpers
    bool IsGoldRequired(ItemTemplate const* proto) const { return proto->Flags2 & ITEM_FLAGS_EXTRA_EXT_COST_REQUIRES_GOLD || !ExtendedCost; }
};
typedef std::vector<VendorItem*> VendorItemList;

struct VendorItemData
{
    VendorItemList _items;

    VendorItem* GetItem(uint32 slot) const
    {
        if (slot >= _items.size()) return NULL;
        return _items[slot];
    }
    bool Empty() const { return _items.empty(); }
    uint8 GetItemCount() const { return _items.size(); }
    void AddItem(uint32 item, int32 maxcount, uint32 ptime, uint32 ExtendedCost)
    {
        _items.push_back(new VendorItem(item, maxcount, ptime, ExtendedCost));
    }
    bool RemoveItem(uint32 item_id);
    VendorItem const* FindItemCostPair(uint32 item_id, uint32 extendedCost) const;
    void Clear()
    {
        for (VendorItemList::const_iterator itr = _items.begin(); itr != _items.end(); ++itr)
            delete (*itr);
        _items.clear();
    }
};

struct VendorItemCount
{
    explicit VendorItemCount(uint32 _item, uint32 _count)
        : itemId(_item), count(_count), lastIncrementTime(time(NULL)) {}

    uint32 itemId;
    uint32 count;
    time_t lastIncrementTime;
};

typedef std::list<VendorItemCount> VendorItemCounts;

struct TrainerSpell
{
    TrainerSpell() : spell(0), spellCost(0), reqSkill(0), reqSkillValue(0), reqLevel(0)
    {
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            learnedSpell[i] = 0;
    }

    uint32 spell;
    uint32 spellCost;
    uint32 reqSkill;
    uint32 reqSkillValue;
    uint32 reqLevel;
    uint32 learnedSpell[3];

    // helpers
    bool IsCastable() const { return learnedSpell[0] != spell; }
};

typedef UNORDERED_MAP<uint32 /*spellid*/, TrainerSpell> TrainerSpellMap;

struct TrainerSpellData
{
    TrainerSpellData() : trainerType(0) {}
    ~TrainerSpellData() { spellList.clear(); }

    TrainerSpellMap spellList;
    uint32 trainerType;                                     // trainer type based at trainer spells, can be different from creature_template value.
                                                            // req. for correct show non-prof. trainers like weaponmaster, allowed values 0 and 2.
    TrainerSpell const* Find(uint32 spell_id) const;
};

typedef std::map<uint32, time_t> CreatureSpellCooldowns;

// max different by z coordinate for creature aggro reaction
#define CREATURE_Z_ATTACK_RANGE 3

#define MAX_VENDOR_ITEMS 300                                // Limitation in 4.x.x item count in SMSG_LIST_INVENTORY

enum CreatureCellMoveState
{
    CREATURE_CELL_MOVE_NONE, //not in move list
    CREATURE_CELL_MOVE_ACTIVE, //in move list
    CREATURE_CELL_MOVE_INACTIVE, //in move list but should not move
};

class MapCreature
{
    friend class Map; //map for moving creatures
    friend class ObjectGridLoader; //grid loader for loading creatures

protected:
    MapCreature() : _moveState(CREATURE_CELL_MOVE_NONE) {}

private:
    Cell _currentCell;
    Cell const& GetCurrentCell() const { return _currentCell; }
    void SetCurrentCell(Cell const& cell) { _currentCell = cell; }

    CreatureCellMoveState _moveState;
    Position _newPosition;
    void SetNewCellPosition(float x, float y, float z, float o)
    {
        _moveState = CREATURE_CELL_MOVE_ACTIVE;
        _newPosition.Relocate(x, y, z, o);
    }
};

class Creature : public Unit, public GridObject<Creature>, public MapCreature
{
    public:

        explicit Creature(bool isWorldObject = false);
        virtual ~Creature();

        void AddToWorld();
        void RemoveFromWorld();

        void DisappearAndDie();

        bool Create(uint32 guidlow, Map* map, uint32 phaseMask, uint32 Entry, uint32 vehId, uint32 team, float x, float y, float z, float ang, const CreatureData* data = NULL);
        bool LoadCreaturesAddon(bool reload = false);
        void SelectLevel(const CreatureTemplate* cinfo);
        void LoadEquipment(uint32 equip_entry, bool force=false);

        uint32 GetDBTableGUIDLow() const { return _DBTableGuid; }

        void Update(uint32 time);                         // overwrited Unit::Update
        void GetRespawnPosition(float &x, float &y, float &z, float* ori = NULL, float* dist =NULL) const;
        uint32 GetEquipmentId() const { return GetCreatureTemplate()->equipmentId; }

        void SetCorpseDelay(uint32 delay) { _corpseDelay = delay; }
        uint32 GetCorpseDelay() const { return _corpseDelay; }
        bool isRacialLeader() const { return GetCreatureTemplate()->RacialLeader; }
        bool isCivilian() const { return GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_CIVILIAN; }
        bool isTrigger() const { return GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_TRIGGER; }
        bool isGuard() const { return GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_GUARD; }
        bool canWalk() const { return GetCreatureTemplate()->InhabitType & INHABIT_GROUND; }
        bool canSwim() const { return GetCreatureTemplate()->InhabitType & INHABIT_WATER; }
        bool canFly()  const { return GetCreatureTemplate()->InhabitType & INHABIT_AIR; }

        void SetReactState(ReactStates st) { _reactState = st; }
        ReactStates GetReactState() { return _reactState; }
        bool HasReactState(ReactStates state) const { return (_reactState == state); }
        void InitializeReactState()
        {
            if (isTotem() || isTrigger() || GetCreatureType() == CREATURE_TYPE_CRITTER || isSpiritService())
                SetReactState(REACT_PASSIVE);
            else
                SetReactState(REACT_AGGRESSIVE);
            /*else if (isCivilian())
            SetReactState(REACT_DEFENSIVE);*/;
        }

        ///// TODO RENAME THIS!!!!!
        bool isCanTrainingOf(Player* player, bool msg) const;
        bool isCanInteractWithBattleMaster(Player* player, bool msg) const;
        bool isCanTrainingAndResetTalentsOf(Player* player) const;
        bool canCreatureAttack(Unit const* victim, bool force = true) const;
        bool IsImmunedToSpell(SpellInfo const* spellInfo);
                                                            // redefine Unit::IsImmunedToSpell
        bool IsImmunedToSpellEffect(SpellInfo const* spellInfo, uint32 index) const;
                                                            // redefine Unit::IsImmunedToSpellEffect
        bool isElite() const
        {
            if (isPet())
                return false;

            uint32 rank = GetCreatureTemplate()->rank;
            return rank != CREATURE_ELITE_NORMAL && rank != CREATURE_ELITE_RARE;
        }

        bool isWorldBoss() const
        {
            if (isPet())
                return false;

            return GetCreatureTemplate()->rank == CREATURE_ELITE_WORLDBOSS;
        }

        bool IsDungeonBoss() const;

        uint8 getLevelForTarget(WorldObject const* target) const; // overwrite Unit::getLevelForTarget for boss level support

        bool IsInEvadeMode() const { return HasUnitState(UNIT_STATE_EVADE); }

        bool AIM_Initialize(CreatureAI* ai = NULL);
        void Motion_Initialize();

        void AI_SendMoveToPacket(float x, float y, float z, uint32 time, uint32 MovementFlags, uint8 type);
        CreatureAI* AI() const { return (CreatureAI*)i_AI; }

        bool SetWalk(bool enable);
        bool SetLevitate(bool enable);

		void RewardCurrency(uint32 currencyid, uint32 currencyamt);

        uint32 GetShieldBlockValue() const                  //dunno mob block value
        {
            return (getLevel()/2 + uint32(GetStat(STAT_STRENGTH)/20));
        }

        SpellSchoolMask GetMeleeDamageSchoolMask() const { return _meleeDamageSchoolMask; }
        void SetMeleeDamageSchool(SpellSchools school) { _meleeDamageSchoolMask = SpellSchoolMask(1 << school); }

        void _AddCreatureSpellCooldown(uint32 spell_id, time_t end_time);
        void _AddCreatureCategoryCooldown(uint32 category, time_t apply_time);
        void AddCreatureSpellCooldown(uint32 spellid);
        bool HasSpellCooldown(uint32 spell_id) const;
        bool HasCategoryCooldown(uint32 spell_id) const;

        bool HasSpell(uint32 spellID) const;

        bool UpdateEntry(uint32 entry, uint32 team=ALLIANCE, const CreatureData* data=NULL);
        bool UpdateStats(Stats stat);
        bool UpdateAllStats();
        void UpdateResistances(uint32 school);
        void UpdateArmor();
        void UpdateMaxHealth();
        void UpdateMaxPower(Powers power);
        void UpdateAttackPowerAndDamage(bool ranged = false);
        void UpdateDamagePhysical(WeaponAttackType attType);
        uint32 GetCurrentEquipmentId() { return _equipmentId; }
        void SetCurrentEquipmentId(uint32 entry) { _equipmentId = entry; }
        float GetSpellDamageMod(int32 Rank);

        VendorItemData const* GetVendorItems() const;
        uint32 GetVendorItemCurrentCount(VendorItem const* vItem);
        uint32 UpdateVendorItemCurrentCount(VendorItem const* vItem, uint32 used_count);

        TrainerSpellData const* GetTrainerSpells() const;

        CreatureTemplate const* GetCreatureTemplate() const { return _creatureInfo; }
        CreatureData const* GetCreatureData() const { return _creatureData; }
        CreatureAddon const* GetCreatureAddon() const;

        std::string GetAIName() const;
        std::string GetScriptName() const;
        uint32 GetScriptId() const;

        void Say(int32 textId, uint32 language, uint64 TargetGuid) { MonsterSay(textId, language, TargetGuid); }
        void Yell(int32 textId, uint32 language, uint64 TargetGuid) { MonsterYell(textId, language, TargetGuid); }
        void TextEmote(int32 textId, uint64 TargetGuid, bool IsBossEmote = false) { MonsterTextEmote(textId, TargetGuid, IsBossEmote); }
        void Whisper(int32 textId, uint64 receiver, bool IsBossWhisper = false) { MonsterWhisper(textId, receiver, IsBossWhisper); }
        void YellToZone(int32 textId, uint32 language, uint64 TargetGuid) { MonsterYellToZone(textId, language, TargetGuid); }

        // override WorldObject function for proper name localization
        const char* GetNameForLocaleIdx(LocaleConstant locale_idx) const;

        void setDeathState(DeathState s);                   // override virtual Unit::setDeathState

        bool LoadFromDB(uint32 guid, Map* map) { return LoadCreatureFromDB(guid, map, false); }
        bool LoadCreatureFromDB(uint32 guid, Map* map, bool addToMap = true);
        void SaveToDB();
                                                            // overriden in Pet
        virtual void SaveToDB(uint32 mapid, uint8 spawnMask, uint32 phaseMask);
        virtual void DeleteFromDB();                        // overriden in Pet

        Loot loot;
        bool lootForPickPocketed;
        bool lootForBody;
        Player* GetLootRecipient() const;
        Group* GetLootRecipientGroup() const;
        bool hasLootRecipient() const { return _lootRecipient || _lootRecipientGroup; }
        bool isTappedBy(Player const* player) const;                          // return true if the creature is tapped by the player or a member of his party.

        void SetLootRecipient (Unit* unit);
        void AllLootRemovedFromCorpse();

        uint16 GetLootMode() { return m_LootMode; }
        bool HasLootMode(uint16 lootMode) { return m_LootMode & lootMode; }
        void SetLootMode(uint16 lootMode) { m_LootMode = lootMode; }
        void AddLootMode(uint16 lootMode) { m_LootMode |= lootMode; }
        void RemoveLootMode(uint16 lootMode) { m_LootMode &= ~lootMode; }
        void ResetLootMode() { m_LootMode = LOOT_MODE_DEFAULT; }

        SpellInfo const* reachWithSpellAttack(Unit* victim);
        SpellInfo const* reachWithSpellCure(Unit* victim);

        uint32 _spells[CREATURE_MAX_SPELLS];
        CreatureSpellCooldowns _CreatureSpellCooldowns;
        CreatureSpellCooldowns _CreatureCategoryCooldowns;

        bool canStartAttack(Unit const* u, bool force) const;
        float GetAttackDistance(Unit const* player) const;

        void SendAIReaction(AiReaction reactionType);

        Unit* SelectNearestTarget(float dist = 0) const;
        Unit* SelectNearestTargetInAttackDistance(float dist = 0) const;
        Player* SelectNearestPlayer(float distance = 0) const;

        void DoFleeToGetAssistance();
        void CallForHelp(float fRadius);
        void CallAssistance();
        void SetNoCallAssistance(bool val) { _AlreadyCallAssistance = val; }
        void SetNoSearchAssistance(bool val) { _AlreadySearchedAssistance = val; }
        bool HasSearchedAssistance() { return _AlreadySearchedAssistance; }
        bool CanAssistTo(const Unit* u, const Unit* enemy, bool checkfaction = true) const;
        bool _IsTargetAcceptable(const Unit* target) const;

        MovementGeneratorType GetDefaultMovementType() const { return _defaultMovementType; }
        void SetDefaultMovementType(MovementGeneratorType mgt) { _defaultMovementType = mgt; }

        void RemoveCorpse(bool setSpawnTime = true);

        void ForcedDespawn(uint32 timeMSToDespawn = 0);
        void DespawnOrUnsummon(uint32 msTimeToDespawn = 0);
		void DespawnCreaturesInArea(uint32 entry, float range = 125.0f);

        time_t const& GetRespawnTime() const { return _respawnTime; }
        time_t GetRespawnTimeEx() const;
        void SetRespawnTime(uint32 respawn) { _respawnTime = respawn ? time(NULL) + respawn : 0; }
        void Respawn(bool force = false);
        void SaveRespawnTime();

        uint32 GetRespawnDelay() const { return _respawnDelay; }
        void SetRespawnDelay(uint32 delay) { _respawnDelay = delay; }

        float GetRespawnRadius() const { return _respawnradius; }
        void SetRespawnRadius(float dist) { _respawnradius = dist; }

        uint32 _groupLootTimer;                            // (msecs)timer used for group loot
        uint32 lootingGroupLowGUID;                         // used to find group which is looting corpse

        void SendZoneUnderAttackMessage(Player* attacker);

        void SetInCombatWithZone();

        bool hasQuest(uint32 quest_id) const;
        bool hasInvolvedQuest(uint32 quest_id)  const;

		void setRegeneratingHealth(bool regenHealth) { _regenHealth = regenHealth; }

        bool isRegeneratingHealth() { return _regenHealth; }
        virtual uint8 GetPetAutoSpellSize() const { return MAX_SPELL_CHARM; }
        virtual uint32 GetPetAutoSpellOnPos(uint8 pos) const
        {
            if (pos >= MAX_SPELL_CHARM || m_charmInfo->GetCharmSpell(pos)->GetType() != ACT_ENABLED)
                return 0;
            else
                return m_charmInfo->GetCharmSpell(pos)->GetAction();
        }

        void SetPosition(float x, float y, float z, float o);
        void SetPosition(const Position &pos) { SetPosition(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), pos.GetOrientation()); }

        void SetHomePosition(float x, float y, float z, float o) { m_homePosition.Relocate(x, y, z, o); }
        void SetHomePosition(const Position &pos) { m_homePosition.Relocate(pos); }
        void GetHomePosition(float &x, float &y, float &z, float &ori) { m_homePosition.GetPosition(x, y, z, ori); }
        Position GetHomePosition() { return m_homePosition; }

        uint32 GetWaypointPath(){return _path_id;}
        void LoadPath(uint32 pathid) { _path_id = pathid; }

        uint32 GetCurrentWaypointID(){return m_waypointID;}
        void UpdateWaypointID(uint32 wpID){m_waypointID = wpID;}

        void SearchFormation();
        CreatureGroup* GetFormation() {return _formation;}
        void SetFormation(CreatureGroup* formation) {_formation = formation;}

        Unit* SelectVictim();

        void SetDisableReputationGain(bool disable) { DisableReputationGain = disable; }
        bool IsReputationGainDisabled() { return DisableReputationGain; }
        bool IsDamageEnoughForLootingAndReward() const { return _PlayerDamageReq == 0; }
        void LowerPlayerDamageReq(uint32 unDamage)
        {
            if (_PlayerDamageReq)
                _PlayerDamageReq > unDamage ? _PlayerDamageReq -= unDamage : _PlayerDamageReq = 0;
        }
        void ResetPlayerDamageReq() { _PlayerDamageReq = GetHealth() / 2; }
        uint32 _PlayerDamageReq;

        uint32 GetOriginalEntry() const { return _originalEntry; }
        void SetOriginalEntry(uint32 entry) { _originalEntry = entry; }

        static float _GetDamageMod(int32 Rank);

        float _SightDistance, _CombatDistance;

        void SetGUIDTransport(uint32 guid) { guid_transport=guid; }
        uint32 GetGUIDTransport() { return guid_transport; }

        void FarTeleportTo(Map* map, float X, float Y, float Z, float O);

        bool _isTempWorldObject; //true when possessed

    protected:
        bool CreateFromProto(uint32 guidlow, uint32 Entry, uint32 vehId, uint32 team, const CreatureData* data = NULL);
        bool InitEntry(uint32 entry, uint32 team=ALLIANCE, const CreatureData* data=NULL);

        // vendor items
        VendorItemCounts _vendorItemCounts;

        static float _GetHealthMod(int32 Rank);

        uint64 _lootRecipient;
        uint32 _lootRecipientGroup;

        /// Timers
        time_t _corpseRemoveTime;                          // (msecs)timer for death or corpse disappearance
        time_t _respawnTime;                               // (secs) time of next respawn
        uint32 _respawnDelay;                              // (secs) delay between corpse disappearance and respawning
        uint32 _corpseDelay;                               // (secs) delay between death and corpse disappearance
        float _respawnradius;

        ReactStates _reactState;                           // for AI, not charmInfo
        void RegenerateMana();
        void RegenerateHealth();
        void Regenerate(Powers power);

        MovementGeneratorType _defaultMovementType;
        uint32 _DBTableGuid;                               ///< For new or temporary creatures is 0 for saved it is lowguid
        uint32 _equipmentId;

        bool _AlreadyCallAssistance;
        bool _AlreadySearchedAssistance;
        bool _regenHealth;
        bool _AI_locked;

        SpellSchoolMask _meleeDamageSchoolMask;
        uint32 _originalEntry;

        Position m_homePosition;

        bool DisableReputationGain;

        CreatureTemplate const* _creatureInfo;                 // in difficulty mode > 0 can different from sObjectMgr->GetCreatureTemplate(GetEntry())
        CreatureData const* _creatureData;

        uint16 m_LootMode;                                  // bitmask, default LOOT_MODE_DEFAULT, determines what loot will be lootable
        uint32 guid_transport;

        bool IsInvisibleDueToDespawn() const;
        bool CanAlwaysSee(WorldObject const* obj) const;
    private:
        //WaypointMovementGenerator vars
        uint32 m_waypointID;
        uint32 _path_id;

        //Formation var
        CreatureGroup* _formation;
        bool TriggerJustRespawned;
};

class AssistDelayEvent : public BasicEvent
{
    public:
        AssistDelayEvent(uint64 victim, Unit& owner) : BasicEvent(), m_victim(victim), _owner(owner) { }

        bool Execute(uint64 e_time, uint32 p_time);
        void AddAssistant(uint64 guid) { m_assistants.push_back(guid); }
    private:
        AssistDelayEvent();

        uint64            m_victim;
        std::list<uint64> m_assistants;
        Unit&             _owner;
};

class ForcedDespawnDelayEvent : public BasicEvent
{
    public:
        ForcedDespawnDelayEvent(Creature& owner) : BasicEvent(), _owner(owner) { }
        bool Execute(uint64 e_time, uint32 p_time);

    private:
        Creature& _owner;
};

#endif
