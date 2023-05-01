/*
 * Copyright (C) 2011-2019 Project SkyFire <http://www.projectskyfire.org/>
 * Copyright (C) 2008-2019 TrinityCore <http://www.trinitycore.org/>
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

/*
 * Scripts for spells with SPELLFAMILY_MAGE and SPELLFAMILY_GENERIC spells used by mage players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_mage_".
 */

#include "ScriptPCH.h"

enum MageSpells
{
    SPELL_MAGE_COLD_SNAP                         = 11958,
    SPELL_MAGE_SQUIRREL_FORM                     = 32813,
    SPELL_MAGE_GIRAFFE_FORM                      = 32816,
    SPELL_MAGE_SERPENT_FORM                      = 32817,
    SPELL_MAGE_DRAGONHAWK_FORM                   = 32818,
    SPELL_MAGE_WORGEN_FORM                       = 32819,
    SPELL_MAGE_SHEEP_FORM                        = 32820,
    SPELL_MAGE_GLYPH_OF_ETERNAL_WATER            = 70937,
    SPELL_MAGE_SUMMON_WATER_ELEMENTAL_PERMANENT  = 70908,
    SPELL_MAGE_SUMMON_WATER_ELEMENTAL_TEMPORARY  = 70907,
    SPELL_MAGE_FLAMESTRIKE                       = 2120,
    SPELL_MAGE_BLASTWAVE                         = 11113,
    SPELL_MAGE_GLYPH_OF_ICE_BARRIER              = 63095,
	SPELL_MAGE_FINGERS_OF_FROST					 = 44544,
	SPELL_MAGE_SHATTERED_BARRIER_R1				 = 44745,
	SPELL_MAGE_SHATTERED_BARRIER_R2				 = 54787,
	SPELL_MAGE_SHATTERED_BARRIER_FREEZE_R1		 = 55080,
	SPELL_MAGE_SHATTERED_BARRIER_FREEZE_R2		 = 83073,
    SPELL_MAGE_CAUTERIZE_DAMAGE                  = 87023,
};

// 11426 - Ice Barrier
class spell_mage_ice_barrier : public SpellScriptLoader
{
public:
	spell_mage_ice_barrier() : SpellScriptLoader("spell_mage_ice_barrier") { }

	class spell_mage_ice_barrier_AuraScript : public AuraScript
	{
		PrepareAuraScript(spell_mage_ice_barrier_AuraScript);

		void AfterRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
		{
			if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_ENEMY_SPELL || aurEff->GetAmount() > 0)
				return;

			if (GetTarget()->HasAura(SPELL_MAGE_SHATTERED_BARRIER_R1))
				GetTarget()->CastSpell(GetTarget(), SPELL_MAGE_SHATTERED_BARRIER_FREEZE_R1, true);
			else if (GetTarget()->HasAura(SPELL_MAGE_SHATTERED_BARRIER_R2))
				GetTarget()->CastSpell(GetTarget(), SPELL_MAGE_SHATTERED_BARRIER_FREEZE_R2, true);
		}

		void CalculateAmount(AuraEffect const* aurEff, int32& amount, bool& canBeRecalculated)
		{
			canBeRecalculated = false;
			if (Unit* caster = GetCaster())
			{
				// +79.00% from sp bonus
				amount += floor(0.79f * caster->SpellBaseDamageBonus(GetSpellInfo()->GetSchoolMask()) + 0.5f);

				// Glyph of Ice barrier
				if (AuraEffect const* glyph = caster->GetDummyAuraEffect(SPELLFAMILY_MAGE, 32, EFFECT_0))
					AddPctN(amount, glyph->GetAmount());
			}
		}


		void Register()
		{
			DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_mage_ice_barrier_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
			AfterEffectRemove += AuraEffectRemoveFn(spell_mage_ice_barrier_AuraScript::AfterRemove, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL);
		}
	};

	AuraScript* GetAuraScript() const
	{
		return new spell_mage_ice_barrier_AuraScript();
	}
};

// HACK!
// TODO: Find a better way to build pet spellbar
// 31687
class spell_mage_water_elemental : public SpellScriptLoader
{
public:
	spell_mage_water_elemental() : SpellScriptLoader("spell_mage_water_elemental") { }

	class spell_mage_water_elemental_SpellScript : public SpellScript
	{
		PrepareSpellScript(spell_mage_water_elemental_SpellScript);

		void HandleExtraEffect()
		{
			Unit* caster = GetCaster();
			
			if (!caster || caster->GetTypeId() != TYPEID_PLAYER)
				return;

			caster->ToPlayer()->PetSpellInitialize();
		}

		void Register()
		{
			AfterCast += SpellCastFn(spell_mage_water_elemental_SpellScript::HandleExtraEffect);
		}
	};

	SpellScript* GetSpellScript() const
	{
		return new spell_mage_water_elemental_SpellScript();
	}
};

class spell_mage_blast_wave : public SpellScriptLoader
{
public:
    spell_mage_blast_wave() : SpellScriptLoader("spell_mage_blast_wave") { }

    class spell_mage_blast_wave_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_mage_blast_wave_SpellScript);

        uint32 count;
        float x;
        float y;
        float z;

        bool Validate(SpellInfo const* /*spellEntry*/)
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_FLAMESTRIKE) ||
                !sSpellMgr->GetSpellInfo(SPELL_MAGE_BLASTWAVE))
                return false;
            return true;
        }

        bool Load()
        {
            if (GetCaster()->GetTypeId() != TYPEID_PLAYER)
                return false;

            x = GetExplTargetDest()->GetPositionX();
            y = GetExplTargetDest()->GetPositionY();
            z = GetExplTargetDest()->GetPositionZ();
            count = 0;
            return true;
        }

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            count = targets.size();
        }

        void HandleExtraEffect()
        {
            Unit* caster = GetCaster();
            if (AuraEffect const* impFlamestrike = caster->GetDummyAuraEffect(SPELLFAMILY_MAGE, 37, EFFECT_0))
            {
                if (count >= 2 && roll_chance_i(impFlamestrike->GetAmount()))
                    caster->CastSpell(x, y, z, SPELL_MAGE_FLAMESTRIKE, true);
            }
        }

        void Register()
        {
            AfterCast += SpellCastFn(spell_mage_blast_wave_SpellScript::HandleExtraEffect);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_mage_blast_wave_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_mage_blast_wave_SpellScript();
    }
};

class spell_mage_reactive_barrier : public SpellScriptLoader
{
public:
	spell_mage_reactive_barrier() : SpellScriptLoader("spell_mage_reactive_barrier") { }

	class spell_mage_reactive_barrier_AuraScript : public AuraScript
	{
		PrepareAuraScript(spell_mage_reactive_barrier_AuraScript);

		Player * caster;

		enum Spels
		{
			ICE_BARRIER = 11426,
			REACTIVE_BARRIER_R1 = 86303,
			REACTIVE_BARRIER_R2 = 86304,
			RB_REDUCE_MANA_COST = 86347,
		};

		bool Load()
		{
			if (!GetCaster() || !GetCaster()->ToPlayer() || GetCaster()->ToPlayer()->getClass() != CLASS_MAGE)
				return false;
			else
			{
				caster = GetCaster()->ToPlayer();
				return true;
			}
		}

		void CalculateAmount(AuraEffect const * /*aurEff*/, int32 & amount, bool & /*canBeRecalculated*/)
		{
			amount = -1; // Set absorbtion amount to unlimited
		}

		void Absorb(AuraEffect * /*aurEff*/, DamageInfo & dmgInfo, uint32 & absorbAmount)
		{
			if (caster->GetHealth() < caster->GetMaxHealth() / 2) // must be above 50 % of health
				return;

			if (caster->GetHealth() - dmgInfo.GetDamage() > caster->GetMaxHealth() / 2) // Health after damage have to be less than 50 % of health
				return;

			if (caster->HasSpellCooldown(ICE_BARRIER)) // This talent obeys Ice Barrier's cooldown
				return;

			if (caster->HasAura(REACTIVE_BARRIER_R1))
			{
				if (roll_chance_i(50)) // 50 % chance for rank 1
				{
					caster->CastSpell(caster, RB_REDUCE_MANA_COST, true);
					caster->CastSpell(caster, ICE_BARRIER, true);
				}
			}
			else // 100 % chance for rank 2
			{
				caster->CastSpell(caster, RB_REDUCE_MANA_COST, true);
				caster->CastSpell(caster, ICE_BARRIER, true);
			}
		}

		void Register()
		{
			DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_mage_reactive_barrier_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
			OnEffectAbsorb += AuraEffectAbsorbFn(spell_mage_reactive_barrier_AuraScript::Absorb, EFFECT_0);
		}
	};

	AuraScript *GetAuraScript() const
	{
		return new spell_mage_reactive_barrier_AuraScript();
	}
};

class spell_mage_cold_snap : public SpellScriptLoader
{
public:
    spell_mage_cold_snap() : SpellScriptLoader("spell_mage_cold_snap") { }

    class spell_mage_cold_snap_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_mage_cold_snap_SpellScript)
        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            Unit* caster = GetCaster();

            if (caster->GetTypeId() != TYPEID_PLAYER)
                return;

            // immediately finishes the cooldown on Frost spells
            const SpellCooldowns& cm = caster->ToPlayer()->GetSpellCooldownMap();
            for (SpellCooldowns::const_iterator itr = cm.begin(); itr != cm.end();)
            {
                SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(itr->first);

                if (spellInfo->SpellFamilyName == SPELLFAMILY_MAGE &&
                    (spellInfo->GetSchoolMask() & SPELL_SCHOOL_MASK_FROST) &&
                    spellInfo->Id != SPELL_MAGE_COLD_SNAP && spellInfo->GetRecoveryTime() > 0)
                {
                    caster->ToPlayer()->RemoveSpellCooldown((itr++)->first, true);
                }
                else
                    ++itr;
            }
        }

        void Register()
        {
            // add dummy effect spell handler to Cold Snap
            OnEffectHit += SpellEffectFn(spell_mage_cold_snap_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_mage_cold_snap_SpellScript();
    }
};

// Frost Warding
class spell_mage_frost_warding_trigger : public SpellScriptLoader
{
public:
    spell_mage_frost_warding_trigger() : SpellScriptLoader("spell_mage_frost_warding_trigger") { }

    class spell_mage_frost_warding_trigger_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_mage_frost_warding_trigger_AuraScript);

        enum Spells
        {
            SPELL_MAGE_FROST_WARDING_TRIGGERED = 57776,
            SPELL_MAGE_FROST_WARDING_R1 = 28332,
        };

        bool Validate(SpellInfo const* /*spellEntry*/)
        {
            return sSpellMgr->GetSpellInfo(SPELL_MAGE_FROST_WARDING_TRIGGERED)
                && sSpellMgr->GetSpellInfo(SPELL_MAGE_FROST_WARDING_R1);
        }

        void Absorb(AuraEffect* aurEff, DamageInfo & dmgInfo, uint32 & absorbAmount)
        {
            Unit* target = GetTarget();
            if (AuraEffect* talentAurEff = target->GetAuraEffectOfRankedSpell(SPELL_MAGE_FROST_WARDING_R1, EFFECT_0))
            {
                int32 chance = talentAurEff->GetSpellInfo()->Effects[EFFECT_1].CalcValue();

                if (roll_chance_i(chance))
                {
                    absorbAmount = dmgInfo.GetDamage();
                    int32 bp = absorbAmount;
                    target->CastCustomSpell(target, SPELL_MAGE_FROST_WARDING_TRIGGERED, &bp, NULL, NULL, true, NULL, aurEff);
                }
            }
        }

        void Register()
        {
             OnEffectAbsorb += AuraEffectAbsorbFn(spell_mage_frost_warding_trigger_AuraScript::Absorb, EFFECT_0);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_mage_frost_warding_trigger_AuraScript();
    }
};

class spell_mage_incanters_absorbtion_base_AuraScript : public AuraScript
{
public:
    enum Spells
    {
        SPELL_MAGE_INCANTERS_ABSORBTION_TRIGGERED = 44413,
        SPELL_MAGE_INCANTERS_ABSORBTION_R1 = 44394,
    };

    bool Validate(SpellInfo const* /*spellEntry*/)
    {
        return sSpellMgr->GetSpellInfo(SPELL_MAGE_INCANTERS_ABSORBTION_TRIGGERED)
            && sSpellMgr->GetSpellInfo(SPELL_MAGE_INCANTERS_ABSORBTION_R1);
    }

    void Trigger(AuraEffect* aurEff, DamageInfo& /*dmgInfo*/, uint32& absorbAmount)
    {
        Unit* target = GetTarget();

        if (AuraEffect* talentAurEff = target->GetAuraEffectOfRankedSpell(SPELL_MAGE_INCANTERS_ABSORBTION_R1, EFFECT_0))
        {
            // Store the normal spellpower to prevent the nonstop aura stacking
            int32 bp = CalculatePctN(absorbAmount, talentAurEff->GetAmount());

            // If we dont get just the intellect spellpower, the aura will stack forever
            uint32 baseSpellPower = target->ToPlayer()->GetBaseSpellPower();

            if (AuraEffect* incanterTriggered = target->GetAuraEffect(SPELL_MAGE_INCANTERS_ABSORBTION_TRIGGERED, 0, target->GetGUID()))
            {
                // The aura will stack up to a value of 20% of the mage's spell power
                incanterTriggered->ChangeAmount(std::min<int32>(incanterTriggered->GetAmount() + bp, CalculatePctN(baseSpellPower, 20)));

                // Refresh and return to prevent replacing the aura
                incanterTriggered->GetBase()->RefreshDuration();

                return;
            }
            else // No incanters absorption aura
            {
                target->CastCustomSpell(target, SPELL_MAGE_INCANTERS_ABSORBTION_TRIGGERED, &bp, NULL, NULL, true, NULL, aurEff);
            }
        }
    }
};

// Incanter's Absorption
class spell_mage_incanters_absorbtion_absorb : public SpellScriptLoader
{
public:
    spell_mage_incanters_absorbtion_absorb() : SpellScriptLoader("spell_mage_incanters_absorbtion_absorb") { }

    class spell_mage_incanters_absorbtion_absorb_AuraScript : public spell_mage_incanters_absorbtion_base_AuraScript
    {
        PrepareAuraScript(spell_mage_incanters_absorbtion_absorb_AuraScript);

        void Register()
        {
             AfterEffectAbsorb += AuraEffectAbsorbFn(spell_mage_incanters_absorbtion_absorb_AuraScript::Trigger, EFFECT_0);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_mage_incanters_absorbtion_absorb_AuraScript();
    }
};

// Incanter's Absorption
class spell_mage_incanters_absorbtion_manashield : public SpellScriptLoader
{
public:
    spell_mage_incanters_absorbtion_manashield() : SpellScriptLoader("spell_mage_incanters_absorbtion_manashield") { }

    class spell_mage_incanters_absorbtion_manashield_AuraScript : public spell_mage_incanters_absorbtion_base_AuraScript
    {
        PrepareAuraScript(spell_mage_incanters_absorbtion_manashield_AuraScript);

        void Register()
        {
             AfterEffectManaShield += AuraEffectManaShieldFn(spell_mage_incanters_absorbtion_manashield_AuraScript::Trigger, EFFECT_0);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_mage_incanters_absorbtion_manashield_AuraScript();
    }
};

// 79684 - Arcane Missiles Proc.
class spell_mage_arcane_missile_proc : public SpellScriptLoader
{
public:
	spell_mage_arcane_missile_proc() : SpellScriptLoader("spell_mage_arcane_missile_proc") { }

	class spell_mage_arcane_missile_proc_AuraScript : public AuraScript
	{
		PrepareAuraScript(spell_mage_arcane_missile_proc_AuraScript);

		bool Validate(SpellInfo const* /*spellInfo*/)
		{
			if (!sSpellMgr->GetSpellInfo(5143) || !sSpellMgr->GetSpellInfo(44445) || !sSpellMgr->GetSpellInfo(44546))
				return false;
			return true;
		}

		bool CheckProc(ProcEventInfo& eventInfo)
		{
			Player* player = GetTarget()->ToPlayer();
			if (!player || !eventInfo.GetSpellInfo())
				return false;

			// Don't proc when caster does not know Arcane Missiles
			if (!player->HasSpell(5143))
				return false;

			// Hot Streak will no longer allow Arcane Missiles to proc
			if (player->HasAura(44445))
				return false;

			// Brain Freeze will no longer allow Arcane Missiles to proc
			if (player->GetAuraOfRankedSpell(44546))
				return false;

			return true;
		}

		void Register()
		{
			DoCheckProc += AuraCheckProcFn(spell_mage_arcane_missile_proc_AuraScript::CheckProc);
		}
	};

	AuraScript* GetAuraScript() const
	{
		return new spell_mage_arcane_missile_proc_AuraScript();
	}
};

// 86948, 86949 Cauterize talent aura
class spell_mage_cauterize : public SpellScriptLoader
{
public:
    spell_mage_cauterize() : SpellScriptLoader("spell_mage_cauterize") {}

    class spell_mage_cauterize_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_mage_cauterize_AuraScript);

        int32 absorbChance;

        bool Validate(SpellInfo const* /*spellEntry*/)
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_CAUTERIZE_DAMAGE))
                return false;

            return true;
        }

        bool Load()
        {
            if (GetCaster()->GetTypeId() != TYPEID_PLAYER)
                return false;

            absorbChance = GetSpellInfo()->Effects[0].BasePoints;
            return true;
        }

        void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
        {
            // Set absorbtion amount to unlimited
            amount = -1;
        }

        void Absorb(AuraEffect* aurEff, DamageInfo& dmgInfo, uint32& absorbAmount)
        {
            Unit* caster = GetCaster();

            if (!caster)
                return;

            int32 remainingHealth = caster->GetHealth() - dmgInfo.GetDamage();
            int32 cauterizeHeal = caster->CountPctFromMaxHealth(40);

            if (caster->ToPlayer()->HasSpellCooldown(SPELL_MAGE_CAUTERIZE_DAMAGE))
                return;

            if (!roll_chance_i(absorbChance))
                return;

            if (remainingHealth <= 0)
            {
                absorbAmount = dmgInfo.GetDamage();
                caster->CastCustomSpell(caster, SPELL_MAGE_CAUTERIZE_DAMAGE, NULL,&cauterizeHeal, NULL, true, NULL, aurEff);
                caster->ToPlayer()->AddSpellCooldown(SPELL_MAGE_CAUTERIZE_DAMAGE, 0, time(NULL) + 60);
            }
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_mage_cauterize_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            OnEffectAbsorb += AuraEffectAbsorbFn(spell_mage_cauterize_AuraScript::Absorb, EFFECT_0);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_mage_cauterize_AuraScript();
    }
};

// 33395 Water Elemental's Freeze
class spell_mage_water_elemental_freeze : public SpellScriptLoader
{
public:
	spell_mage_water_elemental_freeze() : SpellScriptLoader("spell_mage_water_elemental_freeze") { }

	class spell_mage_water_elemental_freeze_SpellScript : public SpellScript
	{
		PrepareSpellScript(spell_mage_water_elemental_freeze_SpellScript);

		bool Validate(SpellInfo const* /*spellInfo*/)
		{
			if (!sSpellMgr->GetSpellInfo(SPELL_MAGE_FINGERS_OF_FROST))
				return false;
			return true;
		}

		void CountTargets(std::list<WorldObject*>& targetList)
		{
			_didHit = !targetList.empty();
		}

		void HandleImprovedFreeze()
		{
			if (!_didHit)
				return;

			Unit* owner = GetCaster()->GetOwner();
			if (!owner)
				return;

			if (AuraEffect* aurEff = owner->GetAuraEffect(SPELL_AURA_DUMMY, SPELLFAMILY_MAGE, 94, EFFECT_0))
			{
				if (roll_chance_i(aurEff->GetAmount()))
					owner->CastCustomSpell(SPELL_MAGE_FINGERS_OF_FROST, SPELLVALUE_AURA_STACK, 2, owner, true);
			}
		}

		void Register()
		{
			OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_mage_water_elemental_freeze_SpellScript::CountTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
			AfterCast += SpellCastFn(spell_mage_water_elemental_freeze_SpellScript::HandleImprovedFreeze);
		}

	private:
		bool _didHit;
	};

	SpellScript* GetSpellScript() const
	{
		return new spell_mage_water_elemental_freeze_SpellScript();
	}
};

void AddSC_mage_spell_scripts()
{
	new spell_mage_water_elemental();
    new spell_mage_blast_wave();
    new spell_mage_cold_snap();
    new spell_mage_frost_warding_trigger();
    new spell_mage_incanters_absorbtion_absorb();
    new spell_mage_incanters_absorbtion_manashield();
    new spell_mage_ice_barrier();
    new spell_mage_cauterize();
	new spell_mage_water_elemental_freeze();
	new spell_mage_arcane_missile_proc();
	new spell_mage_reactive_barrier();
}
