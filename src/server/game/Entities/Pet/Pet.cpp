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

#include "Common.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "WorldPacket.h"
#include "ObjectMgr.h"
#include "SpellMgr.h"
#include "Pet.h"
#include "Formulas.h"
#include "SpellAuras.h"
#include "SpellAuraEffects.h"
#include "CreatureAI.h"
#include "Unit.h"
#include "Util.h"
#include "Group.h"

#define PET_XP_FACTOR 0.05f

Pet::Pet(Player* owner, PetType type) : Guardian(NULL, owner, true),
_usedTalentCount(0), m_removed(false), _owner(owner),
m_happinessTimer(7500), m_petType(type), m_duration(0),
_auraRaidUpdateMask(0), m_loading(false), _declinedname(NULL)
{
    m_unitTypeMask |= UNIT_MASK_PET;
    if (type == HUNTER_PET)
        m_unitTypeMask |= UNIT_MASK_HUNTER_PET;

    if (!(m_unitTypeMask & UNIT_MASK_CONTROLABLE_GUARDIAN))
    {
        m_unitTypeMask |= UNIT_MASK_CONTROLABLE_GUARDIAN;
        InitCharmInfo();
    }

    m_name = "Pet";
    _regenTimer = PET_FOCUS_REGEN_INTERVAL;
}

Pet::~Pet()
{
    delete _declinedname;
}

void Pet::AddToWorld()
{
    ///- Register the pet for guid lookup
    if (!IsInWorld())
    {
        ///- Register the pet for guid lookup
        sObjectAccessor->AddObject(this);
        Unit::AddToWorld();
        AIM_Initialize();
    }

    // Prevent stuck pets when zoning. Pets default to "follow" when added to world
    // so we'll reset flags and let the AI handle things
    if (GetCharmInfo() && GetCharmInfo()->HasCommandState(COMMAND_FOLLOW))
    {
        GetCharmInfo()->SetIsCommandAttack(false);
        GetCharmInfo()->SetIsCommandFollow(false);
        GetCharmInfo()->SetIsAtStay(false);
        GetCharmInfo()->SetIsFollowing(false);
        GetCharmInfo()->SetIsReturning(false);
    }
}

void Pet::RemoveFromWorld()
{
    ///- Remove the pet from the accessor
    if (IsInWorld())
    {
        ///- Don't call the function for Creature, normal mobs + totems go in a different storage
        Unit::RemoveFromWorld();
        sObjectAccessor->RemoveObject(this);
    }
}

bool Pet::LoadPetFromDB(Player* owner, uint32 petentry, uint32 petnumber, bool current, PetSlot slotID)
{
    m_loading = true;

    if (slotID == PET_SAVE_AS_CURRENT)
        slotID = owner->_currentPetSlot;

    uint32 ownerid = owner->GetGUIDLow();

    QueryResult result;

    if (petnumber)
        // known petnumber entry                  0   1      2(?)   3        4      5    6           7     8     9        10         11       12            13      14        15              16
        result = CharacterDatabase.PQuery("SELECT id, entry, owner, modelid, level, exp, Reactstate, slot, name, renamed, curhealth, curmana, curhappiness, abdata, savetime, CreatedBySpell, PetType "
            "FROM character_pet WHERE owner = '%u' AND id = '%u'",
            ownerid, petnumber);

    else if (current && slotID != PET_SLOT_UNK_SLOT)
        // current pet (slot 0)                   0   1      2(?)   3        4      5    6           7     8     9        10         11       12            13      14        15              16
        result = CharacterDatabase.PQuery("SELECT id, entry, owner, modelid, level, exp, Reactstate, slot, name, renamed, curhealth, curmana, curhappiness, abdata, savetime, CreatedBySpell, PetType "
            "FROM character_pet WHERE owner = '%u' AND slot = '%u'",
            ownerid, slotID);
    else if (petentry)
        // known petentry entry (unique for summoned pet, but non unique for hunter pet (only from current or not stabled pets)
        //                                        0   1      2(?)   3        4      5    6           7     8     9        10         11       12           13       14        15              16
        result = CharacterDatabase.PQuery("SELECT id, entry, owner, modelid, level, exp, Reactstate, slot, name, renamed, curhealth, curmana, curhappiness, abdata, savetime, CreatedBySpell, PetType "
            "FROM character_pet WHERE owner = '%u' AND entry = '%u' AND ((slot >= '%u' AND slot <= '%u') AND slot = '%u')",
            ownerid, petentry, PET_SLOT_HUNTER_FIRST, PET_SLOT_HUNTER_LAST, PET_SLOT_STABLE_LAST);
    else
        // any current or other non-stabled pet (for hunter "call pet")
        //                                        0   1      2(?)   3        4      5    6           7     8     9        10         11       12            13      14        15              16
        result = CharacterDatabase.PQuery("SELECT id, entry, owner, modelid, level, exp, Reactstate, slot, name, renamed, curhealth, curmana, curhappiness, abdata, savetime, CreatedBySpell, PetType "
            "FROM character_pet WHERE owner = '%u' AND ((slot >= '%u' AND slot <= '%u') OR slot > '%u')",
            ownerid, PET_SLOT_HUNTER_FIRST, PET_SLOT_HUNTER_LAST, slotID);

    if (!result)
    {
        m_loading = false;
        return false;
    }

    Field* fields = result->Fetch();

    // update for case of current pet "slot = 0"
    petentry = fields[1].GetUInt32();
    if (!petentry)
    {
        m_loading = false;
        return false;
    }

    uint32 summon_spell_id = fields[15].GetUInt32();
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(summon_spell_id);

    bool is_temporary_summoned = spellInfo && spellInfo->GetDuration() > 0;

    // check temporary summoned pets like mage water elemental
    if (current && is_temporary_summoned)
    {
        m_loading = false;
        return false;
    }

    PetType pet_type = PetType(fields[16].GetUInt8());
    if (pet_type == HUNTER_PET)
    {
        CreatureTemplate const* creatureInfo = sObjectMgr->GetCreatureTemplate(petentry);
        if (!creatureInfo || !creatureInfo->isTameable(owner->CanTameExoticPets()))
        {
            m_loading = false;
            return false;
        }
    }
    else if (pet_type == SUMMON_PET && summon_spell_id && !owner->HasSpell(summon_spell_id))
    {
        // pet is summon but owner has no summon spell (e.g.: Water Elemental)
        m_loading = false;
        return false;
    }

    uint32 pet_number = fields[0].GetUInt32();

    if (current && owner->IsPetNeedBeTemporaryUnsummoned())
    {
        owner->SetTemporaryUnsummonedPetNumber(pet_number);
        m_loading = false;
        return false;
    }

    Map* map = owner->GetMap();
    uint32 guid = sObjectMgr->GenerateLowGuid(HIGHGUID_PET);
    if (!Create(guid, map, owner->GetPhaseMask(), petentry, pet_number))
    {
        m_loading = false;
        return false;
    }

    float px, py, pz;
    owner->GetClosePoint(px, py, pz, GetObjectSize(), PET_FOLLOW_DIST, GetFollowAngle());
    Relocate(px, py, pz, owner->GetOrientation());

    if (!IsPositionValid())
    {
        sLog->outError("Pet (guidlow %d, entry %d) not loaded. Suggested coordinates isn't valid (X: %f Y: %f)",
            GetGUIDLow(), GetEntry(), GetPositionX(), GetPositionY());
        m_loading = false;
        return false;
    }

	//SetEntry(petentry);
    setPetType(pet_type);
    setFaction(owner->getFaction());
    SetUInt32Value(UNIT_CREATED_BY_SPELL, summon_spell_id);

    CreatureTemplate const* cinfo = GetCreatureTemplate();
    if (cinfo->type == CREATURE_TYPE_CRITTER)
    {
        m_loading = false;
        map->AddToMap(this->ToCreature());
        return true;
    }

    m_charmInfo->SetPetNumber(pet_number, IsPermanentPetFor(owner));

    SetDisplayId(fields[3].GetUInt32());
    SetNativeDisplayId(fields[3].GetUInt32());
    uint32 petLevel = fields[4].GetUInt16();
    SetUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_NONE);
    SetName(fields[8].GetString());

    switch (getPetType())
    {
        case SUMMON_PET:
            petLevel = owner->getLevel();

            SetUInt32Value(UNIT_FIELD_BYTES_0, 0x800); // class = mage
            SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE);
                                                            // this enables popup window (pet dismiss, cancel)
            break;
        case HUNTER_PET:
            SetUInt32Value(UNIT_FIELD_BYTES_0, 0x02020100); // class = warrior, gender = none, power = focus
            SetSheath(SHEATH_STATE_MELEE);
            SetByteFlag(UNIT_FIELD_BYTES_2, 2, fields[9].GetBool() ? UNIT_CAN_BE_ABANDONED : UNIT_CAN_BE_RENAMED | UNIT_CAN_BE_ABANDONED);

            SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE);
                                                            // this enables popup window (pet abandon, cancel)
            SetMaxPower(POWER_HAPPINESS, GetCreatePowers(POWER_HAPPINESS));
            SetPower(POWER_HAPPINESS, fields[12].GetUInt32());
            setPowerType(POWER_FOCUS);
            break;
        default:
            if (!IsPetGhoul())
                sLog->outError("Pet have incorrect type (%u) for pet loading.", getPetType());
            break;
    }

    SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP, uint32(time(NULL))); // cast can't be helped here
    SetCreatorGUID(owner->GetGUID());

    InitStatsForLevel(petLevel);
    SetUInt32Value(UNIT_FIELD_PETEXPERIENCE, fields[5].GetUInt32());

    SynchronizeLevelWithOwner();

    SetReactState(ReactStates(fields[6].GetUInt8()));
    SetCanModifyStats(true);

    if (getPetType() == SUMMON_PET && !current)              //all (?) summon pets come with full health when called, but not when they are current
        SetPower(POWER_MANA, GetMaxPower(POWER_MANA));
    else
    {
        uint32 savedhealth = fields[10].GetUInt32();
        uint32 savedmana = fields[11].GetUInt32();
        if (!savedhealth && getPetType() == HUNTER_PET)
            setDeathState(JUST_DIED);
        else
        {
            SetHealth(savedhealth > GetMaxHealth() ? GetMaxHealth() : savedhealth);
            SetPower(POWER_MANA, savedmana > uint32(GetMaxPower(POWER_MANA)) ? uint32(GetMaxPower(POWER_MANA)) : savedmana);
        }
    }

    // Send fake summon spell cast - this is needed for correct cooldown application for spells
    // Example: 46584 - without this cooldown (which should be set always when pet is loaded) isn't set clientside
    // TODO: pets should be summoned from real cast instead of just faking it?
    if (summon_spell_id)
    {
        WorldPacket data(SMSG_SPELL_GO, (8+8+4+4+2));
        data.append(owner->GetPackGUID());
        data.append(owner->GetPackGUID());
        data << uint8(0);
        data << uint32(summon_spell_id);
        data << uint32(256); // CAST_FLAG_UNKNOWN3
        data << uint32(0);
        owner->SendMessageToSet(&data, true);
    }

    owner->SetMinion(this, true, slotID == PET_SLOT_UNK_SLOT ? PET_SLOT_OTHER_PET : slotID);
    map->AddToMap(this->ToCreature());

    InitTalentForLevel();                                   // set original talents points before spell loading

    uint32 timediff = uint32(time(NULL) - fields[14].GetUInt32());
    _LoadAuras(timediff);

    // load action bar, if data broken will fill later by default spells.
    if (!is_temporary_summoned)
    {
        m_charmInfo->LoadPetActionBar(fields[13].GetString());

        _LoadSpells();
        InitTalentForLevel();                               // re-init to check talent count
        _LoadSpellCooldowns();
        LearnPetPassives();
        InitLevelupSpellsForLevel();
        CastPetAuras(current);
    }

    CleanupActionBar();                                     // remove unknown spells from action bar after load

    sLog->outDebug(LOG_FILTER_PETS, "New Pet has guid %u", GetGUIDLow());

    owner->PetSpellInitialize();

    if (owner->GetGroup())
        owner->SetGroupUpdateFlag(GROUP_UPDATE_PET);

    owner->SendTalentsInfoData(true);

    if (getPetType() == HUNTER_PET)
    {
        result = CharacterDatabase.PQuery("SELECT genitive, dative, accusative, instrumental, prepositional FROM character_pet_declinedname WHERE owner = '%u' AND id = '%u'", owner->GetGUIDLow(), GetCharmInfo()->GetPetNumber());

        if (result)
        {
            delete _declinedname;
            _declinedname = new DeclinedName;
            Field* fields2 = result->Fetch();
            for (uint8 i = 0; i < MAX_DECLINED_NAME_CASES; ++i)
            {
                _declinedname->name[i] = fields2[i].GetString();
            }
        }
    }

    //set last used pet number (for use in BG's)
    if (owner->GetTypeId() == TYPEID_PLAYER && isControlled() && !isTemporarySummoned() && (getPetType() == SUMMON_PET || getPetType() == HUNTER_PET))
        owner->ToPlayer()->SetLastPetNumber(pet_number);

    m_loading = false;

    return true;
}

void Pet::SavePetToDB(PetSlot mode)
{
    if (!GetEntry())
    {
        if (!GetOwner()->GetPet()->GetEntry())
            return;
    }
    // save only fully controlled creature
    if (!isControlled())
        return;

    // not save not player pets
    if (!IS_PLAYER_GUID(GetOwnerGUID()))
        return;

    Player* owner = (Player*)GetOwner();
    if (!owner)
        return;

    if (mode == PET_SAVE_AS_CURRENT)
        mode = owner->_currentPetSlot;

    // not save pet as current if another pet temporary unsummoned
    if (mode == owner->_currentPetSlot && owner->GetTemporaryUnsummonedPetNumber() &&
        owner->GetTemporaryUnsummonedPetNumber() != m_charmInfo->GetPetNumber())
    {
        // pet will lost anyway at restore temporary unsummoned
        if (getPetType() == HUNTER_PET)
            return;

        // for warlock case
        mode = PET_SLOT_OTHER_PET;
    }

    uint32 curhealth = GetHealth();
    uint32 curmana = GetPower(POWER_MANA);

    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    // save auras before possibly removing them
    _SaveAuras(trans);

    // stable and not in slot saves
    if (mode > PET_SLOT_HUNTER_LAST && getPetType() == HUNTER_PET)
        RemoveAllAuras();

    _SaveSpells(trans);
    _SaveSpellCooldowns(trans);
    CharacterDatabase.CommitTransaction(trans);

    // current/stable/not_in_slot
    if (mode >= PET_SLOT_HUNTER_FIRST)
    {
        uint32 ownerLowGUID = GUID_LOPART(GetOwnerGUID());
        std::string name;

        if (!m_name.empty())
            name = m_name;
        else
            name = owner->GetPet()->GetName();

        CharacterDatabase.EscapeString(name);
        trans = CharacterDatabase.BeginTransaction();
        // remove current data
        trans->PAppend("DELETE FROM character_pet WHERE owner = '%u' AND id = '%u'", ownerLowGUID, m_charmInfo->GetPetNumber());
		//hack = delete slot 100 pet
		trans->PAppend("DELETE FROM character_pet WHERE owner = '%u' AND slot = '%d'", owner, 100);

        // save pet
        std::ostringstream ss;
        ss  << "INSERT INTO character_pet (id, entry,  owner, modelid, level, exp, Reactstate, slot, name, renamed, curhealth, curmana, curhappiness, abdata, savetime, CreatedBySpell, PetType) "
            << "VALUES ("
            << m_charmInfo->GetPetNumber() << ','
            << GetEntry() << ','
            << ownerLowGUID << ','
            << GetNativeDisplayId() << ','
            << uint32(getLevel()) << ','
            << GetUInt32Value(UNIT_FIELD_PETEXPERIENCE) << ','
            << uint32(GetReactState()) << ','
            << uint32(mode) << ", '"
            << name.c_str() << "', "
            << uint32(HasByteFlag(UNIT_FIELD_BYTES_2, 2, UNIT_CAN_BE_RENAMED) ? 0 : 1) << ','
            << curhealth << ','
            << curmana << ','
            << GetPower(POWER_HAPPINESS) << ", '";

        for (uint32 i = ACTION_BAR_INDEX_START; i < ACTION_BAR_INDEX_END; ++i)
        {
            ss << uint32(m_charmInfo->GetActionBarEntry(i)->GetType()) << ' '
               << uint32(m_charmInfo->GetActionBarEntry(i)->GetAction()) << ' ';
        };

        ss  << "', "
            << time(NULL) << ','
            << GetUInt32Value(UNIT_CREATED_BY_SPELL) << ','
            << uint32(getPetType()) << ')';

        trans->Append(ss.str().c_str());
        CharacterDatabase.CommitTransaction(trans);
    }
    // delete
    else
    {
        if (owner->_currentPetSlot >= PET_SLOT_HUNTER_FIRST && owner->_currentPetSlot <= PET_SLOT_HUNTER_LAST)
            owner->setPetSlotUsed(owner->_currentPetSlot, false);
        RemoveAllAuras();
        DeleteFromDB(m_charmInfo->GetPetNumber());
    }
}

void Pet::DeleteFromDB(uint32 guidlow)
{
    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    trans->PAppend("DELETE FROM character_pet WHERE id = '%u'", guidlow);
    trans->PAppend("DELETE FROM character_pet_declinedname WHERE id = '%u'", guidlow);
    trans->PAppend("DELETE FROM pet_aura WHERE guid = '%u'", guidlow);
    trans->PAppend("DELETE FROM pet_spell WHERE guid = '%u'", guidlow);
    trans->PAppend("DELETE FROM pet_spell_cooldown WHERE guid = '%u'", guidlow);
    CharacterDatabase.CommitTransaction(trans);
}

void Pet::setDeathState(DeathState s)                       // overwrite virtual Creature::setDeathState and Unit::setDeathState
{
    Creature::setDeathState(s);
    if (getDeathState() == CORPSE)
    {
        if (getPetType() == HUNTER_PET)
        {
            // pet corpse non lootable and non skinnable
            SetUInt32Value(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_NONE);
            RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);

             //lose happiness when died and not in BG/Arena
            MapEntry const* mapEntry = sMapStore.LookupEntry(GetMapId());
            if (!mapEntry || (mapEntry->map_type != MAP_ARENA && mapEntry->map_type != MAP_BATTLEGROUND))
                ModifyPower(POWER_HAPPINESS, -HAPPINESS_LEVEL_SIZE);

            //SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
        }
    }
    else if (getDeathState() == ALIVE)
    {
        //RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
        CastPetAuras(true);
    }
}

void Pet::Update(uint32 diff)
{
    if (m_removed)                                           // pet already removed, just wait in remove queue, no updates
        return;

    if (m_loading)
        return;

    switch (_deathState)
    {
        case CORPSE:
        {
            if (getPetType() != HUNTER_PET || _corpseRemoveTime <= time(NULL))
            {
                Remove(PET_SAVE_AS_CURRENT);               //hunters' pets never get removed because of death, NEVER!
                return;
            }
            break;
        }
        case ALIVE:
        {
            // unsummon pet that lost owner
            Player* owner = GetOwner();
            if (!owner || (!IsWithinDistInMap(owner, GetMap()->GetVisibilityRange()) && !isPossessed()) || (isControlled() && !owner->GetPetGUID()))
            //if (!owner || (!IsWithinDistInMap(owner, GetMap()->GetVisibilityDistance()) && (owner->GetCharmGUID() && (owner->GetCharmGUID() != GetGUID()))) || (isControlled() && !owner->GetPetGUID()))
            {
                Remove(PET_SAVE_AS_CURRENT, true);
                return;
            }

            if (isControlled())
            {
                if (owner->GetPetGUID() != GetGUID())
                {
                    sLog->outError("Pet %u is not pet of owner %s, removed", GetEntry(), _owner->GetName());
                    Remove(PET_SAVE_AS_CURRENT);
                    return;
                }
            }

            if (m_duration > 0)
            {
                if (uint32(m_duration) > diff)
                    m_duration -= diff;
                else
                {
                    Remove(PET_SAVE_AS_CURRENT);
                    return;
                }
            }

            //regenerate focus for hunter pets or energy for deathknight's ghoul
            if (_regenTimer)
            {
                if (_regenTimer > diff)
                    _regenTimer -= diff;
                else
                {
                    switch (getPowerType())
                    {
                        case POWER_FOCUS:
                            Regenerate(POWER_FOCUS);
                            _regenTimer += PET_FOCUS_REGEN_INTERVAL - diff;
                            if (!_regenTimer)
                                ++_regenTimer;

                            // MrSmite
                            //  Fix for focus regen getting stuck
                            if (_regenTimer > PET_FOCUS_REGEN_INTERVAL)
                                _regenTimer = PET_FOCUS_REGEN_INTERVAL;
                            break;
                        // in creature::update
                        //case POWER_ENERGY:
                        //    Regenerate(POWER_ENERGY);
                        //    _regenTimer += CREATURE_REGEN_INTERVAL - diff;
                        //    if (!_regenTimer) ++_regenTimer;
                        //    break;
                        default:
                            _regenTimer = 0;
                            break;
                    }
                }
            }

            if (getPetType() != HUNTER_PET)
                break;

            if (m_happinessTimer <= diff)
            {
                LoseHappiness();
                m_happinessTimer = 7500;
            }
            else
                m_happinessTimer -= diff;

            break;
        }
        default:
            break;
    }
    Creature::Update(diff);
}

void Creature::Regenerate(Powers power)
{
    uint32 curValue = GetPower(power);
    uint32 maxValue = GetMaxPower(power);

    if (curValue >= maxValue)
        return;

    float addvalue = 0.0f;

    switch (power)
    {
        case POWER_FOCUS:
        {
            // For hunter pets.
            addvalue = 24 * sWorld->getRate(RATE_POWER_FOCUS);
            break;
        }
        case POWER_ENERGY:
        {
            // For deathknight's ghoul.
            addvalue = 20;
            break;
        }
        default:
            return;
    }

    // Apply modifiers (if any).
    AuraEffectList const& ModPowerRegenPCTAuras = GetAuraEffectsByType(SPELL_AURA_MOD_POWER_REGEN_PERCENT);
    for (AuraEffectList::const_iterator i = ModPowerRegenPCTAuras.begin(); i != ModPowerRegenPCTAuras.end(); ++i)
        if (Powers((*i)->GetMiscValue()) == power)
            AddPctN(addvalue, (*i)->GetAmount());

    addvalue += GetTotalAuraModifierByMiscValue(SPELL_AURA_MOD_POWER_REGEN, power) * (isHunterPet()? PET_FOCUS_REGEN_INTERVAL : CREATURE_REGEN_INTERVAL) / (5 * IN_MILLISECONDS);

    ModifyPower(power, int32(addvalue));
}

void Pet::LoseHappiness()
{
    uint32 curValue = GetPower(POWER_HAPPINESS);
    if (curValue <= 0)
        return;
    int32 addvalue = 670;                                   //value is 70/35/17/8/4 (per min) * 1000 / 8 (timer 7.5 secs)
    if (isInCombat())                                        //we know in combat happiness fades faster, multiplier guess
        addvalue = int32(addvalue * 1.5f);
    ModifyPower(POWER_HAPPINESS, -addvalue);
}

HappinessState Pet::GetHappinessState()
{
    if (GetPower(POWER_HAPPINESS) < HAPPINESS_LEVEL_SIZE)
        return UNHAPPY;
    else if (GetPower(POWER_HAPPINESS) >= HAPPINESS_LEVEL_SIZE * 2)
        return HAPPY;
    else
        return CONTENT;
}

void Pet::Remove(PetSlot mode, bool returnreagent)
{
    _owner->RemovePet(this, mode, returnreagent);
}

void Pet::GivePetXP(uint32 xp)
{
    if (getPetType() != HUNTER_PET)
        return;

    if (xp < 1)
        return;

    if (!isAlive())
        return;

    uint8 maxlevel = std::min((uint8)sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL), GetOwner()->getLevel());
    uint8 petLevel = getLevel();

    // If pet is detected to be at, or above(?) the players level, don't hand out XP
    if (petLevel >= maxlevel)
       return;

    uint32 nextLvlXP = GetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP);
    uint32 curXP = GetUInt32Value(UNIT_FIELD_PETEXPERIENCE);
    uint32 newXP = curXP + xp;

    // Check how much XP the pet should receive, and hand off have any left from previous levelups
    while (newXP >= nextLvlXP && petLevel < maxlevel)
    {
        // Subtract newXP from amount needed for nextlevel, and give pet the level
        newXP -= nextLvlXP;
        ++petLevel;

        GivePetLevel(petLevel);

        nextLvlXP = GetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP);
    }
    // Not affected by special conditions - give it new XP
    SetUInt32Value(UNIT_FIELD_PETEXPERIENCE, petLevel < maxlevel ? newXP : 0);
}

void Pet::GivePetLevel(uint8 level)
{
    if (!level || level == getLevel())
        return;

    if (getPetType()==HUNTER_PET)
    {
        SetUInt32Value(UNIT_FIELD_PETEXPERIENCE, 0);
        SetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP, uint32(sObjectMgr->GetXPForLevel(level)*PET_XP_FACTOR));
    }

    InitStatsForLevel(level);
    InitLevelupSpellsForLevel();
    InitTalentForLevel();
}

bool Pet::CreateBaseAtCreature(Creature* creature)
{
    ASSERT(creature);

    if (!CreateBaseAtTamed(creature->GetCreatureTemplate(), creature->GetMap(), creature->GetPhaseMask()))
        return false;

    Relocate(creature->GetPositionX(), creature->GetPositionY(), creature->GetPositionZ(), creature->GetOrientation());

    if (!IsPositionValid())
    {
        sLog->outError("Pet (guidlow %d, entry %d) not created base at creature. Suggested coordinates isn't valid (X: %f Y: %f)",
            GetGUIDLow(), GetEntry(), GetPositionX(), GetPositionY());
        return false;
    }

    CreatureTemplate const* cinfo = GetCreatureTemplate();
    if (!cinfo)
    {
        sLog->outError("CreateBaseAtCreature() failed, creatureInfo is missing!");
        return false;
    }

    SetDisplayId(creature->GetDisplayId());

    if (CreatureFamilyEntry const* cFamily = sCreatureFamilyStore.LookupEntry(cinfo->family))
        SetName(cFamily->Name);
    else
        SetName(creature->GetNameForLocaleIdx(sObjectMgr->GetDBCLocaleIndex()));

    return true;
}

bool Pet::CreateBaseAtCreatureInfo(CreatureTemplate const* cinfo, Unit* owner)
{
    if (!CreateBaseAtTamed(cinfo, owner->GetMap(), owner->GetPhaseMask()))
        return false;

    if (CreatureFamilyEntry const* cFamily = sCreatureFamilyStore.LookupEntry(cinfo->family))
        SetName(cFamily->Name);

    Relocate(owner->GetPositionX(), owner->GetPositionY(), owner->GetPositionZ(), owner->GetOrientation());

    return true;
}

bool Pet::CreateBaseAtTamed(CreatureTemplate const* cinfo, Map* map, uint32 phaseMask)
{
    sLog->outDebug(LOG_FILTER_PETS, "Pet::CreateBaseForTamed");
    uint32 guid = sObjectMgr->GenerateLowGuid(HIGHGUID_PET);
    uint32 pet_number = sObjectMgr->GeneratePetNumber();
    if (!Create(guid, map, phaseMask, cinfo->Entry, pet_number))
        return false;

    SetMaxPower(POWER_HAPPINESS, GetCreatePowers(POWER_HAPPINESS));
    SetPower(POWER_HAPPINESS, 166500);
    setPowerType(POWER_FOCUS);
    SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP, 0);
    SetUInt32Value(UNIT_FIELD_PETEXPERIENCE, 0);
    SetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP, uint32(sObjectMgr->GetXPForLevel(getLevel()+1)*PET_XP_FACTOR));
    SetUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_NONE);

    if (cinfo->type == CREATURE_TYPE_BEAST)
    {
        SetUInt32Value(UNIT_FIELD_BYTES_0, 0x02020100);
        SetSheath(SHEATH_STATE_MELEE);
        SetByteFlag(UNIT_FIELD_BYTES_2, 2, UNIT_CAN_BE_RENAMED | UNIT_CAN_BE_ABANDONED);
    }

    return true;
}

// TODO: Move stat mods code to pet passive auras
bool Guardian::InitStatsForLevel(uint8 petLevel)
{
    CreatureTemplate const* cinfo = GetCreatureTemplate();
    ASSERT(cinfo);

    SetLevel(petLevel);

    //Determine pet type
    PetType petType = MAX_PET_TYPE;
    if (isPet() && _owner->GetTypeId() == TYPEID_PLAYER)
    {
        if ((_owner->getClass() == CLASS_WARLOCK)
            || (_owner->getClass() == CLASS_SHAMAN)        // Fire Elemental
            || (_owner->getClass() == CLASS_PRIEST)        // Shadowfiend
			|| (_owner->getClass() == CLASS_MAGE)	// Water Elemental		
            || (_owner->getClass() == CLASS_DEATH_KNIGHT)) // Risen Ghoul
            petType = SUMMON_PET;
        else if (_owner->getClass() == CLASS_HUNTER)
        {
            petType = HUNTER_PET;
            m_unitTypeMask |= UNIT_MASK_HUNTER_PET;
        }
        else
            sLog->outError("Unknown type pet %u is summoned by player class %u", GetEntry(), _owner->getClass());
    }

    uint32 creature_ID = (petType == HUNTER_PET) ? 1 : cinfo->Entry;

    SetMeleeDamageSchool(SpellSchools(cinfo->dmgschool));

    SetModifierValue(UNIT_MOD_ARMOR, BASE_VALUE, float(petLevel*50));

    SetAttackTime(BASE_ATTACK, BASE_ATTACK_TIME);
    SetAttackTime(OFF_ATTACK, BASE_ATTACK_TIME);
    SetAttackTime(RANGED_ATTACK, BASE_ATTACK_TIME);

    SetFloatValue(UNIT_MOD_CAST_SPEED, 1.0f);

    // Scale
    CreatureFamilyEntry const* cFamily = sCreatureFamilyStore.LookupEntry(cinfo->family);
    if (cFamily && cFamily->minScale > 0.0f && petType == HUNTER_PET)
    {
        float scale;
        // min scale = 0.8 // max scale = 1.8 <-Changed to 1.0 //
        scale = 0.8 + (getLevel()  * ((1.0 - 0.8) / 85));

        SetFloatValue(OBJECT_FIELD_SCALE_X, scale);
    }

    // Resistance
    for (uint8 i = SPELL_SCHOOL_HOLY; i < MAX_SPELL_SCHOOL; ++i)
        SetModifierValue(UnitMods(UNIT_MOD_RESISTANCE_START + i), BASE_VALUE, float(cinfo->resistance[i]));

    //health, mana, armor and resistance
    PetLevelInfo const* pInfo = sObjectMgr->GetPetLevelInfo(creature_ID, petLevel);
    if (pInfo)                                      // exist in DB
    {
        SetCreateHealth(pInfo->health);
        if (petType != HUNTER_PET) //hunter pet use focus
            SetCreateMana(pInfo->mana);

        if (pInfo->armor > 0)
            SetModifierValue(UNIT_MOD_ARMOR, BASE_VALUE, float(pInfo->armor));

        for (uint8 stat = 0; stat < MAX_STATS; ++stat)
            SetCreateStat(Stats(stat), float(pInfo->stats[stat]));
    }
    else                                            // not exist in DB, use some default fake data
    {
        // remove elite bonuses included in DB values
        CreatureBaseStats const* stats = sObjectMgr->GetCreatureBaseStats(petLevel, cinfo->unit_class);
        SetCreateHealth(stats->BaseHealth[cinfo->expansion]);
        SetCreateMana(stats->BaseMana);

        SetCreateStat(STAT_STRENGTH,    22);
        SetCreateStat(STAT_AGILITY,     22);
        SetCreateStat(STAT_STAMINA,     25);
        SetCreateStat(STAT_INTELLECT,   28);
        SetCreateStat(STAT_SPIRIT,      27);
    }

    SetBonusDamage(0);
    switch (petType)
    {
        case SUMMON_PET:
        {
            //the damage bonus used for pets is either fire or shadow damage, whatever is higher
            uint32 fire  = _owner->GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_FIRE);
            uint32 shadow = _owner->GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_SHADOW);
            uint32 val  = (fire > shadow) ? fire : shadow;
            SetBonusDamage(int32 (val * 0.15f));
            //bonusAP += val * 0.57;

            SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float(petLevel - (petLevel / 4)));
            SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float(petLevel + (petLevel / 4)));

            //SetModifierValue(UNIT_MOD_ATTACK_POWER, BASE_VALUE, float(cinfo->attackpower));
            break;
        }
        case HUNTER_PET:
        {
            SetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP, uint32(sObjectMgr->GetXPForLevel(petLevel)*PET_XP_FACTOR));
            //these formula may not be correct; however, it is designed to be close to what it should be
            //this makes dps 0.5 of pets level
            SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float(petLevel - (petLevel / 4)));
            //damage range is then petLevel / 2
            SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float(petLevel + (petLevel / 4)));
            //damage is increased afterwards as strength and pet scaling modify attack power
            SetModifierValue(UNIT_MOD_ARMOR, BASE_VALUE, float(_owner->GetArmor()) * 0.7f);  //  Bonus Armor (70% of player armor)
            break;
        }
        default:
        {
            switch (GetEntry())
            {
                case 510: // mage Water Elemental
                {
                    SetBonusDamage(int32(_owner->SpellBaseDamageBonus(SPELL_SCHOOL_MASK_FROST) * 0.33f));
                    break;
                }
                case 10467: // Mana Tide Totem
                {
                    SetCreateHealth(_owner->GetMaxHealth() * 0.1f);
                    break;
                }
                case 1964: // force of nature
                {
                    if (!pInfo)
                        SetCreateHealth(30 + 30*petLevel);

                    int32 bonusDamage = _owner->SpellBaseDamageBonus(SPELL_SCHOOL_MASK_NATURE)*0.15f;
                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float(petLevel * 7.5f - (petLevel / 2)+ bonusDamage));
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float(petLevel * 7.5f + (petLevel / 2)+ bonusDamage));
                    break;
                }
                case 15352: // earth elemental 36213
                {
                    if (!pInfo)
                        SetCreateHealth(100 + 120 * petLevel);
                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float(petLevel - (petLevel / 4)));
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float(petLevel + (petLevel / 4)));
                    break;
                }
                case 15438: // fire elemental
                {
                    if (!pInfo)
                    {
                        SetCreateHealth(40 * petLevel);
                        SetCreateMana(28 + 10 * petLevel);
                    }
                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float(petLevel * 4 - petLevel));
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float(petLevel * 4 + petLevel));
                    break;
                }
                case 28017: // Bloodworms
                {
                    SetCreateHealth(4 * petLevel);
                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float(petLevel - 30 -(petLevel / 4)) + _owner->GetTotalAttackPowerValue(BASE_ATTACK) * 0.006f);
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float(petLevel - 30 +(petLevel / 4)) + _owner->GetTotalAttackPowerValue(BASE_ATTACK) * 0.006f);
                    break;
                }

                case 17252: // Felguard
                {
                    if (!pInfo)
                    {
                        SetCreateMana(28 + 10 * petLevel);
                        SetCreateHealth(28 + 30 * petLevel);
                    }

                    /*if (_owner->HasAuraEffect(56246, 0)) // Glyph of Felguard
                    int32 bonusDamage = (int32(_owner->SpellBaseDamageBonus(SPELL_SCHOOL_MASK_SHADOW)* 0.5f));
                    else*/
                    int32 bonusDamage = (int32(_owner->SpellBaseDamageBonus(SPELL_SCHOOL_MASK_SHADOW)* 0.3f));
                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float((petLevel * 4 - petLevel) + bonusDamage));
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float((petLevel * 4 + petLevel) + bonusDamage));
                    break;
                }
                case 19668: // Shadowfiend
                {
                    if (!pInfo)
                    {
                        SetCreateMana(28 + 10*petLevel);
                        SetCreateHealth(28 + 30*petLevel);
                    }
                    int32 bonusDamage = (int32(_owner->SpellBaseDamageBonus(SPELL_SCHOOL_MASK_SHADOW)* 0.3f));
                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float((petLevel * 4 - petLevel) + bonusDamage));
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float((petLevel * 4 + petLevel) + bonusDamage));

                    break;
                }
                case 19833: // Snake Trap - Venomous Snake
                {
                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float((petLevel / 2) - 25));
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float((petLevel / 2) - 18));
                    break;
                }
                case 19921: // Snake Trap - Viper
                {
                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float(petLevel / 2 - 10));
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float(petLevel / 2));
                    break;
                }
                case 29264: // Feral Spirit
                {
                    if (!pInfo)
                        SetCreateHealth(30*petLevel);

                    float dmg_multiplier = 0.3f;
                    if (_owner->GetAuraEffect(63271, 0)) // Glyph of Feral Spirit
                        dmg_multiplier = 0.6f;

                    SetBonusDamage(int32(_owner->GetTotalAttackPowerValue(BASE_ATTACK) * dmg_multiplier));

                    // 14AP == 1dps, wolf's strike speed == 2s so dmg = basedmg + AP / 14 * 2
                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float((petLevel * 4 - petLevel) + (_owner->GetTotalAttackPowerValue(BASE_ATTACK) * dmg_multiplier * 2 / 14)));
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float((petLevel * 4 + petLevel) + (_owner->GetTotalAttackPowerValue(BASE_ATTACK) * dmg_multiplier * 2 / 14)));

                    SetModifierValue(UNIT_MOD_ARMOR, BASE_VALUE, float(_owner->GetArmor()) * 0.35f);  //  Bonus Armor (35% of player armor)
                    SetModifierValue(UNIT_MOD_STAT_STAMINA, BASE_VALUE, float(_owner->GetStat(STAT_STAMINA)) * 0.3f);  //  Bonus Stamina (30% of player stamina)
                    if (!HasAura(58877))//prevent apply twice for the 2 wolves
                        AddAura(58877, this);//Spirit Hunt, passive, Spirit Wolves' attacks heal them and their master for 150% of damage done.
                    break;
                }
                case 31216: // Mirror Image
                {
                    SetBonusDamage(int32(_owner->SpellBaseDamageBonus(SPELL_SCHOOL_MASK_FROST) * 0.33f));
                    SetDisplayId(_owner->GetDisplayId());
                    if (!pInfo)
                    {
                        SetCreateMana(28 + 30*petLevel);
                        SetCreateHealth(28 + 10*petLevel);
                    }
                    break;
                }
                case 27829: // Ebon Gargoyle
                {
                    if (!pInfo)
                    {
                        SetCreateMana(28 + 10*petLevel);
                        SetCreateHealth(28 + 30*petLevel);
                    }

                    // Impurity
                    float impurityMod = 1.0f;
                    if (Player* pOwner = _owner->ToPlayer())
                    {
                        PlayerSpellMap playerSpells = pOwner->GetSpellMap();
                        for (PlayerSpellMap::const_iterator itr = playerSpells.begin(); itr != playerSpells.end(); ++itr)
                        {
                            if (itr->second->state == PLAYERSPELL_REMOVED || itr->second->disabled)
                                continue;

                            switch (itr->first)
                            {
                                case 49220:
                                case 49633:
                                case 49635:
                                case 49636:
                                case 49638:
                                {
                                    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(itr->first);
                                        AddPctN(impurityMod, spellInfo->Effects[EFFECT_0].CalcValue());
                                }
                                break;
                            }
                        }
                    }
                    SetBonusDamage(int32(_owner->GetTotalAttackPowerValue(BASE_ATTACK) * 0.5f * impurityMod));
                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float(petLevel - (petLevel / 4)));
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float(petLevel + (petLevel / 4)));
                    break;
                }
                case 30230: // Risen Ally
                {
                    if (!pInfo)
                        SetCreateHealth(_owner->GetMaxHealth());
                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float(400) + (_owner->GetTotalAttackPowerValue(BASE_ATTACK)));
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float(650) + (_owner->GetTotalAttackPowerValue(BASE_ATTACK)));
                    break;
                }
                case 50675: // Ebon Imp
                {
                    if (!pInfo)
                    {
                        SetCreateMana(28 + 10*petLevel);
                        SetCreateHealth(28 + 30*petLevel);
                    }
                    int32 bonus_dmg = (int32(_owner->SpellBaseDamageBonus(SPELL_SCHOOL_MASK_SHADOW)* 0.3f));
                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float((petLevel * 4 - petLevel) + bonus_dmg));
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float((petLevel * 4 + petLevel) + bonus_dmg));
                    break;
                }
            }
            break;
        }
    }

    UpdateAllStats();

    SetFullHealth();
    SetPower(POWER_MANA, GetMaxPower(POWER_MANA));
    return true;
}

bool Pet::HaveInDiet(ItemTemplate const* item) const
{
    if (!item->FoodType)
        return false;

    CreatureTemplate const* cInfo = GetCreatureTemplate();
    if (!cInfo)
        return false;

    CreatureFamilyEntry const* cFamily = sCreatureFamilyStore.LookupEntry(cInfo->family);
    if (!cFamily)
        return false;

    uint32 diet = cFamily->petFoodMask;
    uint32 FoodMask = 1 << (item->FoodType-1);
    return diet & FoodMask;
}

uint32 Pet::GetCurrentFoodBenefitLevel(uint32 itemlevel)
{
    // -5 or greater food level
    if (getLevel() <= itemlevel + 5)                         //possible to feed level 60 pet with level 55 level food for full effect
        return 35000;
    // -10..-6
    else if (getLevel() <= itemlevel + 10)                   //pure guess, but sounds good
        return 17000;
    // -14..-11
    else if (getLevel() <= itemlevel + 14)                   //level 55 food gets green on 70, makes sense to me
        return 8000;
    // -15 or less
    else
        return 0;                                           //food too low level
}

void Pet::_LoadSpellCooldowns()
{
    _CreatureSpellCooldowns.clear();
    _CreatureCategoryCooldowns.clear();

    QueryResult result = CharacterDatabase.PQuery("SELECT spell, time FROM pet_spell_cooldown WHERE guid = '%u'", m_charmInfo->GetPetNumber());

    if (result)
    {
        time_t curTime = time(NULL);

        WorldPacket data(SMSG_SPELL_COOLDOWN, size_t(8+1+result->GetRowCount()*8));
        data << GetGUID();
        data << uint8(0x0);                                 // flags (0x1, 0x2)

        do
        {
            Field* fields = result->Fetch();

            uint32 spell_id = fields[0].GetUInt32();
            time_t db_time  = time_t(fields[1].GetUInt32());

            if (!sSpellMgr->GetSpellInfo(spell_id))
            {
                sLog->outError("Pet %u have unknown spell %u in `pet_spell_cooldown`, skipping.", m_charmInfo->GetPetNumber(), spell_id);
                continue;
            }

            // skip outdated cooldown
            if (db_time <= curTime)
                continue;

            data << uint32(spell_id);
            data << uint32(uint32(db_time-curTime)*IN_MILLISECONDS);

            _AddCreatureSpellCooldown(spell_id, db_time);

            sLog->outDebug(LOG_FILTER_PETS, "Pet (Number: %u) spell %u cooldown loaded (%u secs).", m_charmInfo->GetPetNumber(), spell_id, uint32(db_time-curTime));
        }
        while (result->NextRow());

        if (!_CreatureSpellCooldowns.empty() && GetOwner())
            ((Player*)GetOwner())->GetSession()->SendPacket(&data);
    }
}

void Pet::_SaveSpellCooldowns(SQLTransaction& trans)
{
    trans->PAppend("DELETE FROM pet_spell_cooldown WHERE guid = '%u'", m_charmInfo->GetPetNumber());

    time_t curTime = time(NULL);

    // remove outdated and save active
    for (CreatureSpellCooldowns::iterator itr = _CreatureSpellCooldowns.begin(); itr != _CreatureSpellCooldowns.end();)
    {
        if (itr->second <= curTime)
            _CreatureSpellCooldowns.erase(itr++);
        else
        {
            trans->PAppend("INSERT INTO pet_spell_cooldown (guid, spell, time) VALUES ('%u', '%u', '%u')", m_charmInfo->GetPetNumber(), itr->first, uint32(itr->second));
            ++itr;
        }
    }
}

void Pet::_LoadSpells()
{
    QueryResult result = CharacterDatabase.PQuery("SELECT spell, active FROM pet_spell WHERE guid = '%u'", m_charmInfo->GetPetNumber());

    if (result)
    {
        do
        {
            Field* fields = result->Fetch();

            addSpell(fields[0].GetUInt32(), ActiveStates(fields[1].GetUInt8()), PETSPELL_UNCHANGED);
        }
        while (result->NextRow());
    }
}

void Pet::_SaveSpells(SQLTransaction& trans)
{
    for (PetSpellMap::iterator itr = _spells.begin(), next = _spells.begin(); itr != _spells.end(); itr = next)
    {
        ++next;

        // prevent saving family passives to DB
        if (itr->second.type == PETSPELL_FAMILY)
            continue;

        switch (itr->second.state)
        {
            case PETSPELL_REMOVED:
                trans->PAppend("DELETE FROM pet_spell WHERE guid = '%u' and spell = '%u'", m_charmInfo->GetPetNumber(), itr->first);
                _spells.erase(itr);
                continue;
            case PETSPELL_CHANGED:
                trans->PAppend("DELETE FROM pet_spell WHERE guid = '%u' and spell = '%u'", m_charmInfo->GetPetNumber(), itr->first);
                trans->PAppend("INSERT INTO pet_spell (guid, spell, active) VALUES ('%u', '%u', '%u')", m_charmInfo->GetPetNumber(), itr->first, itr->second.active);
                break;
            case PETSPELL_NEW:
                trans->PAppend("INSERT INTO pet_spell (guid, spell, active) VALUES ('%u', '%u', '%u')", m_charmInfo->GetPetNumber(), itr->first, itr->second.active);
                break;
            case PETSPELL_UNCHANGED:
                continue;
        }
        itr->second.state = PETSPELL_UNCHANGED;
    }
}

void Pet::_LoadAuras(uint32 timediff)
{
    sLog->outDebug(LOG_FILTER_PETS, "Loading auras for pet %u", GetGUIDLow());

    QueryResult result = CharacterDatabase.PQuery("SELECT caster_guid, spell, effect_mask, recalculate_mask, stackcount, amount0, amount1, amount2, base_amount0, base_amount1, base_amount2, maxduration, remaintime, remaincharges FROM pet_aura WHERE guid = '%u'", m_charmInfo->GetPetNumber());

    if (result)
    {
        do
        {
            int32 damage[3];
            int32 baseDamage[3];
            Field* fields = result->Fetch();
            uint64 caster_guid = fields[0].GetUInt64();
            // NULL guid stored - pet is the caster of the spell - see Pet::_SaveAuras
            if (!caster_guid)
                caster_guid = GetGUID();
            uint32 spellid = fields[1].GetUInt32();
            uint8 effmask = fields[2].GetUInt8();
            uint8 recalculatemask = fields[3].GetUInt8();
            uint8 stackcount = fields[4].GetUInt8();
            damage[0] = fields[5].GetInt32();
            damage[1] = fields[6].GetInt32();
            damage[2] = fields[7].GetInt32();
            baseDamage[0] = fields[8].GetInt32();
            baseDamage[1] = fields[9].GetInt32();
            baseDamage[2] = fields[10].GetInt32();
            int32 maxduration = fields[11].GetInt32();
            int32 remaintime = fields[12].GetInt32();
            uint8 remaincharges = fields[13].GetUInt8();

            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellid);
            if (!spellInfo)
            {
                sLog->outError("Unknown aura (spellid %u), ignore.", spellid);
                continue;
            }

            // negative effects should continue counting down after logout
            if (remaintime != -1 && !spellInfo->IsPositive())
            {
                if (remaintime/IN_MILLISECONDS <= int32(timediff))
                    continue;

                remaintime -= timediff*IN_MILLISECONDS;
            }

            // prevent wrong values of remaincharges
            if (spellInfo->ProcCharges)
            {
                if (remaincharges <= 0 || remaincharges > spellInfo->ProcCharges)
                    remaincharges = spellInfo->ProcCharges;
            }
            else
                remaincharges = 0;

            if (Aura* aura = Aura::TryCreate(spellInfo, effmask, this, NULL, &baseDamage[0], NULL, caster_guid))
            {
                if (!aura->CanBeSaved())
                {
                    aura->Remove();
                    continue;
                }
                aura->SetLoadedState(maxduration, remaintime, remaincharges, stackcount, recalculatemask, &damage[0]);
                aura->ApplyForTargets();
                sLog->outDetail("Added aura spellid %u, effectmask %u", spellInfo->Id, effmask);
            }
        }
        while (result->NextRow());
    }
}

void Pet::_SaveAuras(SQLTransaction& trans)
{
    trans->PAppend("DELETE FROM pet_aura WHERE guid = '%u'", m_charmInfo->GetPetNumber());

    for (AuraMap::const_iterator itr = m_ownedAuras.begin(); itr != m_ownedAuras.end(); ++itr)
    {
        // check if the aura has to be saved
        if (!itr->second->CanBeSaved() || IsPetAura(itr->second))
            continue;

        Aura* aura = itr->second;

        int32 damage[MAX_SPELL_EFFECTS];
        int32 baseDamage[MAX_SPELL_EFFECTS];
        uint8 effMask = 0;
        uint8 recalculateMask = 0;
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (aura->GetEffect(i))
            {
                baseDamage[i] = aura->GetEffect(i)->GetBaseAmount();
                damage[i] = aura->GetEffect(i)->GetAmount();
                effMask |= (1<<i);
                if (aura->GetEffect(i)->CanBeRecalculated())
                    recalculateMask |= (1<<i);
            }
            else
            {
                baseDamage[i] = 0;
                damage[i] = 0;
            }
        }

        // don't save guid of caster in case we are caster of the spell - guid for pet is generated every pet load, so it won't match saved guid anyways
        uint64 casterGUID = (itr->second->GetCasterGUID() == GetGUID()) ? 0 : itr->second->GetCasterGUID();

        trans->PAppend("INSERT INTO pet_aura (guid, caster_guid, spell, effect_mask, recalculate_mask, stackcount, amount0, amount1, amount2, base_amount0, base_amount1, base_amount2, maxduration, remaintime, remaincharges) "
            "VALUES ('%u', '" UI64FMTD "', '%u', '%u', '%u', '%u', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%u')",
            m_charmInfo->GetPetNumber(), casterGUID, itr->second->GetId(), effMask, recalculateMask,
            itr->second->GetStackAmount(), damage[0], damage[1], damage[2], baseDamage[0], baseDamage[1], baseDamage[2],
            itr->second->GetMaxDuration(), itr->second->GetDuration(), itr->second->GetCharges());
    }
}

bool Pet::addSpell(uint32 spellId, ActiveStates active /*= ACT_DECIDE*/, PetSpellState state /*= PETSPELL_NEW*/, PetSpellType type /*= PETSPELL_NORMAL*/)
{
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo)
    {
        // do pet spell book cleanup
        if (state == PETSPELL_UNCHANGED)                    // spell load case
        {
            sLog->outError("Pet::addSpell: Non-existed in SpellStore spell #%u request, deleting for all pets in `pet_spell`.", spellId);

            PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_DELETE_INVALID_PET_SPELL);

            stmt->setUInt32(0, spellId);

            CharacterDatabase.Execute(stmt);
        }
        else
            sLog->outError("Pet::addSpell: Non-existed in SpellStore spell #%u request.", spellId);

        return false;
    }

    PetSpellMap::iterator itr = _spells.find(spellId);
    if (itr != _spells.end())
    {
        if (itr->second.state == PETSPELL_REMOVED)
        {
            _spells.erase(itr);
            state = PETSPELL_CHANGED;
        }
        else if (state == PETSPELL_UNCHANGED && itr->second.state != PETSPELL_UNCHANGED)
        {
            // can be in case spell loading but learned at some previous spell loading
            itr->second.state = PETSPELL_UNCHANGED;

            if (active == ACT_ENABLED)
                ToggleAutocast(spellInfo, true);
            else if (active == ACT_DISABLED)
                ToggleAutocast(spellInfo, false);

            return false;
        }
        else
            return false;
    }

    PetSpell newspell;
    newspell.state = state;
    newspell.type = type;

    if (active == ACT_DECIDE)                               // active was not used before, so we save it's autocast/passive state here
    {
        if (spellInfo->IsAutocastable())
            newspell.active = ACT_DISABLED;
        else
            newspell.active = ACT_PASSIVE;
    }
    else
        newspell.active = active;

    // talent: unlearn all other talent ranks (high and low)
    if (TalentSpellPos const* talentPos = GetTalentSpellPos(spellId))
    {
        if (TalentEntry const* talentInfo = sTalentStore.LookupEntry(talentPos->talent_id))
        {
            for (uint8 i = 0; i < MAX_TALENT_RANK; ++i)
            {
                // skip learning spell and no rank spell case
                uint32 rankSpellId = talentInfo->RankID[i];
                if (!rankSpellId || rankSpellId == spellId)
                    continue;

                // skip unknown ranks
                if (!HasSpell(rankSpellId))
                    continue;
                removeSpell(rankSpellId, false, false);
            }
        }
    }
    else if (spellInfo->IsRanked())
    {
        for (PetSpellMap::const_iterator itr2 = _spells.begin(); itr2 != _spells.end(); ++itr2)
        {
            if (itr2->second.state == PETSPELL_REMOVED) continue;

            SpellInfo const* oldRankSpellInfo = sSpellMgr->GetSpellInfo(itr2->first);

            if (!oldRankSpellInfo)
                continue;

            if (spellInfo->IsDifferentRankOf(oldRankSpellInfo))
            {
                // replace by new high rank
                if (spellInfo->IsHighRankOf(oldRankSpellInfo))
                {
                    newspell.active = itr2->second.active;

                    if (newspell.active == ACT_ENABLED)
                        ToggleAutocast(oldRankSpellInfo, false);

                    unlearnSpell(itr2->first, false, false);
                    break;
                }
                // ignore new lesser rank
                else
                    return false;
            }
        }
    }

    _spells[spellId] = newspell;

    if (spellInfo->IsPassive() && (!spellInfo->CasterAuraState || HasAuraState(AuraStateType(spellInfo->CasterAuraState))))
        CastSpell(this, spellId, true);
    else
        m_charmInfo->AddSpellToActionBar(spellInfo);

    if (newspell.active == ACT_ENABLED)
        ToggleAutocast(spellInfo, true);

    uint32 talentCost = GetTalentSpellCost(spellId);
    if (talentCost)
    {
        int32 free_points = GetMaxTalentPointsForLevel(getLevel());
        _usedTalentCount += talentCost;
        // update free talent points
        free_points-=_usedTalentCount;
        SetFreeTalentPoints(free_points > 0 ? free_points : 0);
    }
    return true;
}

bool Pet::learnSpell(uint32 spell_id)
{
    // prevent duplicated entires in spell book
    if (!addSpell(spell_id))
        return false;

    if (!m_loading)
    {
        WorldPacket data(SMSG_PET_LEARNED_SPELL, 4);
        data << uint32(spell_id);
        _owner->GetSession()->SendPacket(&data);
        _owner->PetSpellInitialize();
    }
    return true;
}

void Pet::InitLevelupSpellsForLevel()
{
    uint8 level = getLevel();

    if (PetLevelupSpellSet const* levelupSpells = GetCreatureTemplate()->family ? sSpellMgr->GetPetLevelupSpellList(GetCreatureTemplate()->family) : NULL)
    {
        // PetLevelupSpellSet ordered by levels, process in reversed order
        for (PetLevelupSpellSet::const_reverse_iterator itr = levelupSpells->rbegin(); itr != levelupSpells->rend(); ++itr)
        {
            // will called first if level down
            if (itr->first > level)
                unlearnSpell(itr->second, true);                 // will learn prev rank if any
            // will called if level up
            else
                learnSpell(itr->second);                        // will unlearn prev rank if any
        }
    }

    int32 petSpellsId = GetCreatureTemplate()->PetSpellDataId ? -(int32)GetCreatureTemplate()->PetSpellDataId : GetEntry();

    // default spells (can be not learned if pet level (as owner level decrease result for example) less first possible in normal game)
    if (PetDefaultSpellsEntry const* defSpells = sSpellMgr->GetPetDefaultSpellsEntry(petSpellsId))
    {
        for (uint8 i = 0; i < MAX_CREATURE_SPELL_DATA_SLOT; ++i)
        {
            SpellInfo const* spellEntry = sSpellMgr->GetSpellInfo(defSpells->spellid[i]);
            if (!spellEntry)
                continue;

            // will called first if level down
            if (spellEntry->SpellLevel > level)
                unlearnSpell(spellEntry->Id, true);
            // will called if level up
            else
                learnSpell(spellEntry->Id);
        }
    }
}

bool Pet::unlearnSpell(uint32 spell_id, bool learn_prev, bool clear_ab)
{
    if (removeSpell(spell_id, learn_prev, clear_ab))
    {
        if (!m_loading)
        {
            WorldPacket data(SMSG_PET_REMOVED_SPELL, 4);
            data << uint32(spell_id);
            _owner->GetSession()->SendPacket(&data);
        }
        return true;
    }
    return false;
}

bool Pet::removeSpell(uint32 spell_id, bool learn_prev, bool clear_ab)
{
    PetSpellMap::iterator itr = _spells.find(spell_id);
    if (itr == _spells.end())
        return false;

    if (itr->second.state == PETSPELL_REMOVED)
        return false;

    if (itr->second.state == PETSPELL_NEW)
        _spells.erase(itr);
    else
        itr->second.state = PETSPELL_REMOVED;

    RemoveAurasDueToSpell(spell_id);

    uint32 talentCost = GetTalentSpellCost(spell_id);
    if (talentCost > 0)
    {
        if (_usedTalentCount > talentCost)
            _usedTalentCount -= talentCost;
        else
            _usedTalentCount = 0;
        // update free talent points
        int32 free_points = GetMaxTalentPointsForLevel(getLevel()) - _usedTalentCount;
        SetFreeTalentPoints(free_points > 0 ? free_points : 0);
    }

    if (learn_prev)
    {
        if (uint32 prev_id = sSpellMgr->GetPrevSpellInChain (spell_id))
            learnSpell(prev_id);
        else
            learn_prev = false;
    }

    // if remove last rank or non-ranked then update action bar at server and client if need
    if (clear_ab && !learn_prev && m_charmInfo->RemoveSpellFromActionBar(spell_id))
    {
        if (!m_loading)
        {
            // need update action bar for last removed rank
            if (Unit* owner = GetOwner())
                if (owner->GetTypeId() == TYPEID_PLAYER)
                    owner->ToPlayer()->PetSpellInitialize();
        }
    }

    return true;
}

void Pet::CleanupActionBar()
{
    for (uint8 i = 0; i < MAX_UNIT_ACTION_BAR_INDEX; ++i)
        if (UnitActionBarEntry const* ab = m_charmInfo->GetActionBarEntry(i))
            if (ab->GetAction() && ab->IsActionBarForSpell())
            {
                if (!HasSpell(ab->GetAction()))
                    m_charmInfo->SetActionBar(i, 0, ACT_PASSIVE);
                else if (ab->GetType() == ACT_ENABLED)
                {
                    if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(ab->GetAction()))
                        ToggleAutocast(spellInfo, true);
                }
            }
}

void Pet::InitPetCreateSpells()
{
    m_charmInfo->InitPetActionBar();
    _spells.clear();

    LearnPetPassives();
    InitLevelupSpellsForLevel();

    CastPetAuras(false);
}

bool Pet::resetTalents()
{
    Unit* owner = GetOwner();
    if (!owner || owner->GetTypeId() != TYPEID_PLAYER)
        return false;

    // not need after this call
    if (owner->ToPlayer()->HasAtLoginFlag(AT_LOGIN_RESET_PET_TALENTS))
        owner->ToPlayer()->RemoveAtLoginFlag(AT_LOGIN_RESET_PET_TALENTS, true);

    CreatureTemplate const* ci = GetCreatureTemplate();
    if (!ci)
        return false;
    // Check pet talent type
    CreatureFamilyEntry const* pet_family = sCreatureFamilyStore.LookupEntry(ci->family);
    if (!pet_family || pet_family->petTalentType == PET_TALENT_TYPE_NOT_HUNTER_PET)
        return false;

    Player* player = owner->ToPlayer();

    uint8 level = getLevel();
    uint32 talentPointsForLevel = GetMaxTalentPointsForLevel(level);

    if (_usedTalentCount == 0)
    {
        SetFreeTalentPoints(talentPointsForLevel);
        return false;
    }

    for (uint32 i = 0; i < sTalentStore.GetNumRows(); ++i)
    {
        TalentEntry const* talentInfo = sTalentStore.LookupEntry(i);

        if (!talentInfo)
            continue;

        TalentTabEntry const* talentTabInfo = sTalentTabStore.LookupEntry(talentInfo->TalentTab);

        if (!talentTabInfo)
            continue;

        // unlearn only talents for pets family talent type
        if (!((1 << pet_family->petTalentType) & talentTabInfo->petTalentMask))
            continue;

        for (uint8 j = 0; j < MAX_TALENT_RANK; ++j)
        {
            for (PetSpellMap::const_iterator itr = _spells.begin(); itr != _spells.end();)
            {
                if (itr->second.state == PETSPELL_REMOVED)
                {
                    ++itr;
                    continue;
                }
                // remove learned spells (all ranks)
                uint32 itrFirstId = sSpellMgr->GetFirstSpellInChain(itr->first);

                // unlearn if first rank is talent or learned by talent
                if (itrFirstId == talentInfo->RankID[j] || sSpellMgr->IsSpellLearnToSpell(talentInfo->RankID[j], itrFirstId))
                {
                    unlearnSpell(itr->first, false);
                    itr = _spells.begin();
                    continue;
                }
                else
                    ++itr;
            }
        }
    }

    SetFreeTalentPoints(talentPointsForLevel);

    if (!m_loading)
        player->PetSpellInitialize();
    return true;
}

void Pet::resetTalentsForAllPetsOf(Player* owner, Pet* online_pet /*= NULL*/)
{
    // not need after this call
    if (owner->ToPlayer()->HasAtLoginFlag(AT_LOGIN_RESET_PET_TALENTS))
        owner->ToPlayer()->RemoveAtLoginFlag(AT_LOGIN_RESET_PET_TALENTS, true);

    // reset for online
    if (online_pet)
        online_pet->resetTalents();

    // now need only reset for offline pets (all pets except online case)
    uint32 except_petnumber = online_pet ? online_pet->GetCharmInfo()->GetPetNumber() : 0;

    QueryResult resultPets = CharacterDatabase.PQuery(
        "SELECT id FROM character_pet WHERE owner = '%u' AND id <> '%u'",
        owner->GetGUIDLow(), except_petnumber);

    // no offline pets
    if (!resultPets)
        return;

    QueryResult result = CharacterDatabase.PQuery(
        "SELECT DISTINCT pet_spell.spell FROM pet_spell, character_pet "
        "WHERE character_pet.owner = '%u' AND character_pet.id = pet_spell.guid AND character_pet.id <> %u",
        owner->GetGUIDLow(), except_petnumber);

    if (!result)
        return;

    bool need_comma = false;
    std::ostringstream ss;
    ss << "DELETE FROM pet_spell WHERE guid IN (";

    do
    {
        Field* fields = resultPets->Fetch();

        uint32 id = fields[0].GetUInt32();

        if (need_comma)
            ss << ',';

        ss << id;

        need_comma = true;
    }
    while (resultPets->NextRow());

    ss << ") AND spell IN (";

    bool need_execute = false;
    do
    {
        Field* fields = result->Fetch();

        uint32 spell = fields[0].GetUInt32();

        if (!GetTalentSpellCost(spell))
            continue;

        if (need_execute)
            ss << ',';

        ss << spell;

        need_execute = true;
    }
    while (result->NextRow());

    if (!need_execute)
        return;

    ss << ')';

    CharacterDatabase.Execute(ss.str().c_str());
}

void Pet::InitTalentForLevel()
{
    uint8 level = getLevel();
    uint32 talentPointsForLevel = GetMaxTalentPointsForLevel(level);
    // Reset talents in case low level (on level down) or wrong points for level (hunter can unlearn TP increase talent)
    if (talentPointsForLevel == 0 || _usedTalentCount > talentPointsForLevel)
        resetTalents(); // Remove all talent points

    SetFreeTalentPoints(talentPointsForLevel - _usedTalentCount);

    Unit* owner = GetOwner();
    if (!owner || owner->GetTypeId() != TYPEID_PLAYER)
        return;

    if (!m_loading)
        owner->ToPlayer()->SendTalentsInfoData(true);
}

uint8 Pet::GetMaxTalentPointsForLevel(uint8 level)
{
    uint8 points = (level >= 20) ? ((level - 16) / 4) : 0;
    // Mod points from owner SPELL_AURA_MOD_PET_TALENT_POINTS
    if (Unit* owner = GetOwner())
        points+=owner->GetTotalAuraModifier(SPELL_AURA_MOD_PET_TALENT_POINTS);
    return points;
}

void Pet::ToggleAutocast(SpellInfo const* spellInfo, bool apply)
{
    if (!spellInfo->IsAutocastable())
        return;

    uint32 spellid = spellInfo->Id;

    PetSpellMap::iterator itr = _spells.find(spellid);
    if (itr == _spells.end())
        return;

    uint32 i;

    if (apply)
    {
        for (i = 0; i < m_autospells.size() && m_autospells[i] != spellid; ++i);     // just search

        if (i == m_autospells.size())
        {
            m_autospells.push_back(spellid);

            if (itr->second.active != ACT_ENABLED)
            {
                itr->second.active = ACT_ENABLED;
                if (itr->second.state != PETSPELL_NEW)
                    itr->second.state = PETSPELL_CHANGED;
            }
        }
    }
    else
    {
        AutoSpellList::iterator itr2 = m_autospells.begin();
        for (i = 0; i < m_autospells.size() && m_autospells[i] != spellid; ++i, ++itr2);      // just search

        if (i < m_autospells.size())
        {
            m_autospells.erase(itr2);
            if (itr->second.active != ACT_DISABLED)
            {
                itr->second.active = ACT_DISABLED;
                if (itr->second.state != PETSPELL_NEW)
                    itr->second.state = PETSPELL_CHANGED;
            }
        }
    }
}

bool Pet::IsPermanentPetFor(Player* owner)
{
    switch (getPetType())
    {
        case SUMMON_PET:
            switch (owner->getClass())
            {
                case CLASS_WARLOCK:
                    return GetCreatureTemplate()->type == CREATURE_TYPE_DEMON;
                case CLASS_DEATH_KNIGHT:
                    return GetCreatureTemplate()->type == CREATURE_TYPE_UNDEAD;
                case CLASS_MAGE:
                    return GetCreatureTemplate()->type == CREATURE_TYPE_ELEMENTAL;
                default:
                    return false;
            }
        case HUNTER_PET:
            return true;
        default:
            return false;
    }
}

bool Pet::Create(uint32 guidlow, Map* map, uint32 phaseMask, uint32 Entry, uint32 pet_number)
{
    ASSERT(map);
    SetMap(map);

    SetPhaseMask(phaseMask, false);
    Object::_Create(guidlow, pet_number, HIGHGUID_PET);

    _DBTableGuid = guidlow;
    _originalEntry = Entry;

    if (!InitEntry(Entry))
        return false;

    SetSheath(SHEATH_STATE_MELEE);

    return true;
}

bool Pet::HasSpell(uint32 spell) const
{
    PetSpellMap::const_iterator itr = _spells.find(spell);
    return itr != _spells.end() && itr->second.state != PETSPELL_REMOVED;
}

// Get all passive spells in our skill line
void Pet::LearnPetPassives()
{
    CreatureTemplate const* cInfo = GetCreatureTemplate();
    if (!cInfo)
        return;

    CreatureFamilyEntry const* cFamily = sCreatureFamilyStore.LookupEntry(cInfo->family);
    if (!cFamily)
        return;

    PetFamilySpellsStore::const_iterator petStore = sPetFamilySpellsStore.find(cFamily->ID);
    if (petStore != sPetFamilySpellsStore.end())
    {
        // For general hunter pets skill 270
        // Passive 01~10, Passive 00 (20782, not used), Ferocious Inspiration (34457)
        // Scale 01~03 (34902~34904, bonus from owner, not used)
        for (PetFamilySpellsSet::const_iterator petSet = petStore->second.begin(); petSet != petStore->second.end(); ++petSet)
            addSpell(*petSet, ACT_DECIDE, PETSPELL_NEW, PETSPELL_FAMILY);
    }
}

void Pet::CastPetAuras(bool current)
{
    Unit* owner = GetOwner();
    if (!owner || owner->GetTypeId() != TYPEID_PLAYER)
        return;

    if (!IsPermanentPetFor(owner->ToPlayer()))
        return;

    for (PetAuraSet::const_iterator itr = owner->m_petAuras.begin(); itr != owner->m_petAuras.end();)
    {
        PetAura const* pa = *itr;
        ++itr;

        if (!current && pa->IsRemovedOnChangePet())
            owner->RemovePetAura(pa);
        else
            CastPetAura(pa);
    }
}

void Pet::CastPetAura(PetAura const* aura)
{
    uint32 auraId = aura->GetAura(GetEntry());
    if (!auraId)
        return;

    if (auraId == 35696)                                      // Demonic Knowledge
    {
        int32 basePoints = CalculatePctF(aura->GetDamage(), GetStat(STAT_STAMINA) + GetStat(STAT_INTELLECT));
        CastCustomSpell(this, auraId, &basePoints, NULL, NULL, true);
    }
    else
        CastSpell(this, auraId, true);
}

bool Pet::IsPetAura(Aura const* aura)
{
    Unit* owner = GetOwner();

    if (!owner || owner->GetTypeId() != TYPEID_PLAYER)
        return false;

    // if the owner has that pet aura, return true
    for (PetAuraSet::const_iterator itr = owner->m_petAuras.begin(); itr != owner->m_petAuras.end(); ++itr)
    {
        if ((*itr)->GetAura(GetEntry()) == aura->GetId())
            return true;
    }
    return false;
}

void Pet::learnSpellHighRank(uint32 spellid)
{
    learnSpell(spellid);

    if (uint32 next = sSpellMgr->GetNextSpellInChain(spellid))
        learnSpellHighRank(next);
}

void Pet::SynchronizeLevelWithOwner()
{
    Unit* owner = GetOwner();
    if (!owner || owner->GetTypeId() != TYPEID_PLAYER)
        return;

    switch (getPetType())
    {
        // always same level
        case SUMMON_PET:
            GivePetLevel(owner->getLevel());
            break;
        // can't be greater owner level
        case HUNTER_PET:
            if (getLevel() > owner->getLevel())
                GivePetLevel(owner->getLevel());
            else if (getLevel() + 3 < owner->getLevel())
                GivePetLevel(owner->getLevel() - 3);
            break;
        default:
            break;
    }
}

PetTalentType Pet::GetTalentType()
{
    CreatureTemplate const* cInfo = GetCreatureTemplate();
    if (!cInfo)
        return PET_TALENT_TYPE_NOT_HUNTER_PET;

    CreatureFamilyEntry const *pet_family = sCreatureFamilyStore.LookupEntry(cInfo->family);
    if (!pet_family)
        return PET_TALENT_TYPE_NOT_HUNTER_PET;

    return (PetTalentType)pet_family->petTalentType;
}
