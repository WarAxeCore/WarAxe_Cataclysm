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
 * Scripts for spells with SPELLFAMILY_PALADIN and SPELLFAMILY_GENERIC spells used by paladin players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_pal_".
 */

#include "ScriptPCH.h"
#include "SpellAuraEffects.h"

enum PaladinSpells
{
	PALADIN_SPELL_JUDGEMENTS_AURA				 = 31878,
	PALADIN_SPELL_JUDGEMENTS_AURA_2				 = 31930,
	PALADIN_SPELL_COMMUNION						 = 31876,

    PALADIN_SPELL_DIVINE_PLEA                    = 54428,

	SPELL_PALADIN_ILLUMINATED_HEALING			 = 86273,

    PALADIN_SPELL_HOLY_SHOCK_R1                  = 20473,
    PALADIN_SPELL_HOLY_SHOCK_R1_DAMAGE           = 25912,
    PALADIN_SPELL_HOLY_SHOCK_R1_HEALING          = 25914,

    SPELL_BLESSING_OF_LOWER_CITY_DRUID           = 37878,
    SPELL_BLESSING_OF_LOWER_CITY_PALADIN         = 37879,
    SPELL_BLESSING_OF_LOWER_CITY_PRIEST          = 37880,
    SPELL_BLESSING_OF_LOWER_CITY_SHAMAN          = 37881,

    SPELL_DIVINE_PURPOSE_PROC                    = 90174,
    SPELL_PALADIN_WORD_OF_GLORY                  = 85673,
    SPELL_PALADIN_JUDG_BOLD_OVERTIME             = 89906,
    SPELL_PALADIN_SELFLESS_HEALER_PROC           = 90811,

    SPELL_PALADIN_GUARDIAN_ANCIENT_KINGS         = 86150,
    SPELL_PALADIN_RETRI_GUARDIAN                 = 86698,
    SPELL_PALADIN_HOLY_GUARDIAN                  = 86669,
    SPELL_PALADIN_PROT_GUARDIAN                  = 86659,

    SPELL_DIVINE_STORM                           = 53385,
    SPELL_DIVINE_STORM_DUMMY                     = 54171,
    SPELL_DIVINE_STORM_HEAL                      = 54172,

    SPELL_PALADIN_CONSECRATION_SUMMON            = 82366,
    SPELL_PALADIN_CONSECRATION_DAMAGE            = 81297,

    SPELL_RIGHTEOUS_DEFENCE                      = 31789,
    SPELL_RIGHTEOUS_DEFENCE_EFFECT_1             = 31790,

    PALADIN_SPELL_BLESSING_OF_KINGS_1            = 79062,
    PALADIN_SPELL_BLESSING_OF_KINGS_2            = 79063,

    PALADIN_SPELL_BLESSING_OF_MIGHT_1            = 79101,
    PALADIN_SPELL_BLESSING_OF_MIGHT_2            = 79102
};

//4.0.6a - 
//Handles 31878 Judgements Of The Wise
class spell_pal_judgementwise : public SpellScriptLoader
{
public:
	spell_pal_judgementwise() : SpellScriptLoader("spell_pal_judgementwise") { }

	class spell_pal_judgementwise_SpellScript : public SpellScript
	{
		PrepareSpellScript(spell_pal_judgementwise_SpellScript);

		void JudgementsOfTheWise()
		{
			Unit* caster = GetCaster();
			Unit* target = GetHitUnit();

			if (!caster)
				return;

			if (!caster->HasAura(PALADIN_SPELL_JUDGEMENTS_AURA))
				return;

			int32 basepoints0 = int32(caster->GetCreateMana() * 0.030);
			caster->CastCustomSpell(target, PALADIN_SPELL_JUDGEMENTS_AURA_2, &basepoints0, NULL, NULL, true);
		}

		void Replenishment() // Can only affect up to 10 party/raid members needs fixed.
		{
			Unit* caster = GetCaster();

			if (!caster)
				return;

			if (!caster->HasAura(PALADIN_SPELL_COMMUNION))
				return;

			caster->CastSpell(caster, 57669, true);
		}

		void Register()
		{
			AfterHit += SpellHitFn(spell_pal_judgementwise_SpellScript::JudgementsOfTheWise);
			AfterHit += SpellHitFn(spell_pal_judgementwise_SpellScript::Replenishment);
		}
	};

	SpellScript* GetSpellScript() const
	{
		return new spell_pal_judgementwise_SpellScript();
	}
};

// 879 Exorcism
// 4.0.6a
class spell_pal_exorcism : public SpellScriptLoader
{
public:
	spell_pal_exorcism() : SpellScriptLoader("spell_pal_exorcism") { }

	class spell_pal_exorcism_SpellScript : public SpellScript
	{
		PrepareSpellScript(spell_pal_exorcism_SpellScript)

		enum
		{
			GLYPH_OF_EXORCISM = 54934,
			SPELL_EXORCISM_TRIGGERED = 879
		};

		void CalculateDamage(SpellEffIndex /*effIndex*/)
		{
			Unit* caster = GetCaster();
			if (!caster || !GetHitUnit())
				return;

			PreventHitAura();

			damageAmount = (GetHitDamage() * 0.20f) / 3;
		}

		void HandleGlyphOfExorcism()
		{
			if (Unit* caster = GetCaster())
			{
				if (Unit* target = GetHitUnit())
				{
					if (caster->HasAura(GLYPH_OF_EXORCISM) && damageAmount > 0)
					{
						caster->AddAura(SPELL_EXORCISM_TRIGGERED, target);

						// Set pre-calculated amount
						if (Aura* glyphOfExorcism = target->GetAura(SPELL_EXORCISM_TRIGGERED, caster->GetGUID()))
							glyphOfExorcism->GetEffect(EFFECT_1)->SetAmount(damageAmount);
					}
				}
			}
		}

	protected:
		int32 damageAmount;

		void Register()
		{
			OnEffectHitTarget += SpellEffectFn(spell_pal_exorcism_SpellScript::CalculateDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
			AfterHit += SpellHitFn(spell_pal_exorcism_SpellScript::HandleGlyphOfExorcism);
		}
	};

	SpellScript* GetSpellScript() const
	{
		return new spell_pal_exorcism_SpellScript();
	}
};

// 31850 - Ardent Defender
class spell_pal_ardent_defender : public SpellScriptLoader
{
public:
    spell_pal_ardent_defender() : SpellScriptLoader("spell_pal_ardent_defender") { }

    class spell_pal_ardent_defender_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_pal_ardent_defender_AuraScript);

        uint32 absorbPct, healPct;

        enum Spell
        {
            PAL_SPELL_ARDENT_DEFENDER_HEAL = 66235,
        };

        bool Load()
        {
            healPct = GetSpellInfo()->Effects[EFFECT_1].CalcValue();
            absorbPct = GetSpellInfo()->Effects[EFFECT_0].CalcValue();
            return GetUnitOwner()->ToPlayer();
        }

        void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
        {
            // Set absorbtion amount to unlimited
            amount = -1;
        }

        void Absorb(AuraEffect* aurEff, DamageInfo& dmgInfo, uint32& absorbAmount)
        {
            Unit* victim = GetTarget();
            int32 remainingHealth = victim->GetHealth() - dmgInfo.GetDamage();
            uint32 allowedHealth = victim->CountPctFromMaxHealth(35);
            // If damage kills us
            if (remainingHealth <= 0 && !victim->ToPlayer()->HasSpellCooldown(PAL_SPELL_ARDENT_DEFENDER_HEAL))
            {
                // Cast healing spell, completely avoid damage
                absorbAmount = dmgInfo.GetDamage();

                uint32 defenseSkillValue = victim->GetDefenseSkillValue();
                // Max heal when defense skill denies critical hits from raid bosses
                // Formula: max defense at level + 140 (raiting from gear)
                uint32 reqDefForMaxHeal  = victim->getLevel() * 5 + 140;
                float pctFromDefense = (defenseSkillValue >= reqDefForMaxHeal)
                    ? 1.0f
                    : float(defenseSkillValue) / float(reqDefForMaxHeal);

                int32 healAmount = int32(victim->CountPctFromMaxHealth(uint32(healPct * pctFromDefense)));
                victim->CastCustomSpell(victim, PAL_SPELL_ARDENT_DEFENDER_HEAL, &healAmount, NULL, NULL, true, NULL, aurEff);
                victim->ToPlayer()->AddSpellCooldown(PAL_SPELL_ARDENT_DEFENDER_HEAL, 0, time(NULL) + 120);
            }
            else if (remainingHealth < int32(allowedHealth))
            {
                // Reduce damage that brings us under 35% (or full damage if we are already under 35%) by x%
                uint32 damageToReduce = (victim->GetHealth() < allowedHealth)
                    ? dmgInfo.GetDamage()
                    : allowedHealth - remainingHealth;
                absorbAmount = CalculatePctN(damageToReduce, absorbPct);
            }
        }

        void Register()
        {
             DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pal_ardent_defender_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
             OnEffectAbsorb += AuraEffectAbsorbFn(spell_pal_ardent_defender_AuraScript::Absorb, EFFECT_0);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_pal_ardent_defender_AuraScript();
    }
};

class spell_pal_blessing_of_faith : public SpellScriptLoader
{
public:
    spell_pal_blessing_of_faith() : SpellScriptLoader("spell_pal_blessing_of_faith") { }

    class spell_pal_blessing_of_faith_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_pal_blessing_of_faith_SpellScript)
        bool Validate(SpellInfo const* /*spellEntry*/)
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_BLESSING_OF_LOWER_CITY_DRUID))
                return false;
            if (!sSpellMgr->GetSpellInfo(SPELL_BLESSING_OF_LOWER_CITY_PALADIN))
                return false;
            if (!sSpellMgr->GetSpellInfo(SPELL_BLESSING_OF_LOWER_CITY_PRIEST))
                return false;
            if (!sSpellMgr->GetSpellInfo(SPELL_BLESSING_OF_LOWER_CITY_SHAMAN))
                return false;
            return true;
        }

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            if (Unit* unitTarget = GetHitUnit())
            {
                uint32 spell_id = 0;
                switch (unitTarget->getClass())
                {
                    case CLASS_DRUID:   spell_id = SPELL_BLESSING_OF_LOWER_CITY_DRUID; break;
                    case CLASS_PALADIN: spell_id = SPELL_BLESSING_OF_LOWER_CITY_PALADIN; break;
                    case CLASS_PRIEST:  spell_id = SPELL_BLESSING_OF_LOWER_CITY_PRIEST; break;
                    case CLASS_SHAMAN:  spell_id = SPELL_BLESSING_OF_LOWER_CITY_SHAMAN; break;
                    default: return;                    // ignore for non-healing classes
                }

                GetCaster()->CastSpell(GetCaster(), spell_id, true);
            }
        }

        void Register()
        {
            // add dummy effect spell handler to Blessing of Faith
            OnEffectHitTarget += SpellEffectFn(spell_pal_blessing_of_faith_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_pal_blessing_of_faith_SpellScript();
    }
};

class spell_pal_holy_shock : public SpellScriptLoader
{
public:
    spell_pal_holy_shock() : SpellScriptLoader("spell_pal_holy_shock") { }

    class spell_pal_holy_shock_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_pal_holy_shock_SpellScript)
        bool Validate(SpellInfo const* spellEntry)
        {
            if (!sSpellMgr->GetSpellInfo(PALADIN_SPELL_HOLY_SHOCK_R1))
                return false;

            // can't use other spell than holy shock due to spell_ranks dependency
            if (sSpellMgr->GetFirstSpellInChain(PALADIN_SPELL_HOLY_SHOCK_R1) != sSpellMgr->GetFirstSpellInChain(spellEntry->Id))
                return false;

            uint8 rank = sSpellMgr->GetSpellRank(spellEntry->Id);
            if (!sSpellMgr->GetSpellWithRank(PALADIN_SPELL_HOLY_SHOCK_R1_DAMAGE, rank, true))
                return false;
            if (!sSpellMgr->GetSpellWithRank(PALADIN_SPELL_HOLY_SHOCK_R1_HEALING, rank, true))
                return false;

            return true;
        }

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            if (Unit* unitTarget = GetHitUnit())
            {
                Unit* caster = GetCaster();

                uint8 rank = sSpellMgr->GetSpellRank(GetSpellInfo()->Id);

                if (caster->IsFriendlyTo(unitTarget))
                    caster->CastSpell(unitTarget, sSpellMgr->GetSpellWithRank(PALADIN_SPELL_HOLY_SHOCK_R1_HEALING, rank), true, 0);
                else
                    caster->CastSpell(unitTarget, sSpellMgr->GetSpellWithRank(PALADIN_SPELL_HOLY_SHOCK_R1_DAMAGE, rank), true, 0);
            }
        }

        void Register()
        {
            // add dummy effect spell handler to Holy Shock
            OnEffectHitTarget += SpellEffectFn(spell_pal_holy_shock_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_pal_holy_shock_SpellScript();
    }
};

// 76669 - Illuminated Healing
class spell_pal_illuminated_healing : public SpellScriptLoader
{
public:
	spell_pal_illuminated_healing() : SpellScriptLoader("spell_pal_illuminated_healing") { }

	class spell_pal_illuminated_healing_AuraScript : public AuraScript
	{
		PrepareAuraScript(spell_pal_illuminated_healing_AuraScript);

		bool Validate(SpellInfo const* /*spellInfo*/) override
		{
			if (!sSpellMgr->GetSpellInfo(SPELL_PALADIN_ILLUMINATED_HEALING))
				return false;

			return true;
		}

		void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
		{
			PreventDefaultAction();
			if (Unit* caster = GetCaster())
			{
				if (Unit* target = eventInfo.GetProcTarget())
				{
					uint32 shieldAmount = CalculatePctN(eventInfo.GetHealInfo()->GetHeal(), aurEff->GetAmount());
					caster->CastCustomSpell(SPELL_PALADIN_ILLUMINATED_HEALING, SPELLVALUE_BASE_POINT0, shieldAmount, target, true, nullptr, aurEff);
				}
			}
		}

		void Register() override
		{
			OnEffectProc2 += AuraEffectProcFn2(spell_pal_illuminated_healing_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
		}
	};

	AuraScript* GetAuraScript() const override
	{
		return new spell_pal_illuminated_healing_AuraScript();
	}
};

class spell_pal_judgements_of_the_bold : public SpellScriptLoader
{
    public:
        spell_pal_judgements_of_the_bold() : SpellScriptLoader("spell_pal_judgements_of_the_bold") { }

        class spell_pal_judgements_of_the_bold_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_judgements_of_the_bold_AuraScript);

            bool Validate (SpellInfo const* /*spellEntry*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_PALADIN_JUDG_BOLD_OVERTIME))
                    return false;

                return true;
            }

            bool Load()
            {
                if (GetCaster()->GetTypeId() != TYPEID_PLAYER)
                    return false;
                return true;
            }

            void CalculateMana(AuraEffect const* /*aurEff*/, int32& amount, bool& canBeRecalculated)
            {
                if (Unit* caster = GetCaster())
                {
                    canBeRecalculated = true;
                    int32 basemana = caster->ToPlayer()->GetCreateMana();
                    amount = (3 * basemana) / 100; // 3% of base mana
                }
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pal_judgements_of_the_bold_AuraScript::CalculateMana, EFFECT_0, SPELL_AURA_PERIODIC_ENERGIZE);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pal_judgements_of_the_bold_AuraScript();
        }
};

// Shield of Righteous
// Spell Id: 53600
class spell_pal_shield_of_righteous : public SpellScriptLoader
{
public:
    spell_pal_shield_of_righteous() : SpellScriptLoader("spell_pal_shield_of_righteous") { }

    class spell_pal_shield_of_righteous_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_pal_shield_of_righteous_SpellScript)

        bool Load()
        {
            if (GetCaster()->GetTypeId() != TYPEID_PLAYER)
                return false;
            return true;
        }

        void CalculateDamage(SpellEffIndex /*effIndex*/)
        {
            if (Unit* caster = GetCaster())
            {
                int32 damage = GetHitDamage();
                int32 power = caster->GetPower(POWER_HOLY_POWER);
                switch (power)
                {
                    case 0:
                        damage = int32(damage * - 1.0f);
                        break;
                    case 1:
                        damage = int32((damage * 3.0f) - 3);
                        break;
                    case 2:
                        damage = int32((damage * 6.0f) - 6);
                        break;
                }
                SetHitDamage(damage);
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_pal_shield_of_righteous_SpellScript::CalculateDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_pal_shield_of_righteous_SpellScript();
    }
};

class spell_pal_word_of_glory : public SpellScriptLoader
{
public:
    spell_pal_word_of_glory() : SpellScriptLoader("spell_pal_word_of_glory") { }

    class spell_pal_word_of_glory_heal_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_pal_word_of_glory_heal_SpellScript)

        int32 totalheal;

        bool Validate (SpellInfo const* /*spellEntry*/)
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_PALADIN_WORD_OF_GLORY))
                return false;

            return true;
        }

        bool Load()
        {
            if (GetCaster()->GetTypeId() != TYPEID_PLAYER)
                return false;

            if (GetCaster()->ToPlayer()->getClass() != CLASS_PALADIN)
                    return false;

            return true;
        }

        void ChangeHeal(SpellEffIndex /*effIndex*/)
        {
            totalheal = GetHitHeal();
            Unit* caster = GetCaster();
            Unit* target = GetHitUnit();

            if (!target)
                return;

            if (GetCaster()->HasAura(SPELL_DIVINE_PURPOSE_PROC))
            {
                totalheal *= 3;

                // Selfless Healer
                if (AuraEffect const* auraEff = caster->GetAuraEffect(SPELL_AURA_DUMMY, SPELLFAMILY_PALADIN, 3924, EFFECT_0))
                    if (target != caster)
                        totalheal += (totalheal * auraEff->GetAmount()) / 100;

                SetHitHeal(totalheal);
                const_cast<SpellValue*>(GetSpellValue())->EffectBasePoints[1] = totalheal;
                return;
            }

            switch (caster->GetPower(POWER_HOLY_POWER))
            {
                case 0: // 1 Holy Power
                {
                    totalheal = totalheal;
                    break;
                }
                case 1: // 2 Holy Power
                {
                    totalheal *= 2;
                    break;
                }
                case 2: // 3 Holy Power
                {
                    totalheal *= 3;
                    break;
                }
            }
            // Selfless Healer
            if (AuraEffect const* auraEff = caster->GetDummyAuraEffect(SPELLFAMILY_PALADIN, 3924, EFFECT_0))
                if (target != caster)
                    totalheal += (totalheal * auraEff->GetAmount()) / 100;

            SetHitHeal(totalheal);
            const_cast<SpellValue*>(GetSpellValue())->EffectBasePoints[1] = totalheal;
        }

        void PreventEffect(SpellEffIndex effIndex)
        {
            // Glyph of Long Word
            if (!GetCaster()->HasAura(93466))
                PreventHitDefaultEffect(effIndex);
        }

        void HandlePeriodic()
        {
            // Glyph of Long Word
            if (!GetCaster()->HasAura(93466))
                PreventHitAura();
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_pal_word_of_glory_heal_SpellScript::ChangeHeal, EFFECT_0, SPELL_EFFECT_HEAL);
            OnEffectHitTarget += SpellEffectFn(spell_pal_word_of_glory_heal_SpellScript::PreventEffect, EFFECT_1, SPELL_EFFECT_APPLY_AURA);
            AfterHit += SpellHitFn(spell_pal_word_of_glory_heal_SpellScript::HandlePeriodic);
        }
    };

    class spell_pal_word_of_glory_heal_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_pal_word_of_glory_heal_AuraScript)

        bool Validate (SpellInfo const* /*spellEntry*/)
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_PALADIN_WORD_OF_GLORY)->Effects[1].Effect)
                return false;

            return true;
        }

        bool Load()
        {
            if (GetCaster()->GetTypeId() != TYPEID_PLAYER)
                return false;

            if (GetCaster()->ToPlayer()->getClass() != CLASS_PALADIN)
                return false;

            return true;
        }

        void CalculateOvertime(AuraEffect const* /*aurEff*/, int32& amount, bool& canBeRecalculated)
        {
            // Had to do ALLLLLL the scaling manually because (afaik) there is no way to hook the GetHitHeal from the spell's effIndex 0
            Unit* caster = GetCaster();
            int32 bonus = (caster->ToPlayer()->GetTotalAttackPowerValue(BASE_ATTACK) * 0.198) + (caster->ToPlayer()->GetSpellPowerBonus() * 0.209);
            int32 divinebonus = (caster->ToPlayer()->GetTotalAttackPowerValue(BASE_ATTACK) * 0.594) + (caster->ToPlayer()->GetSpellPowerBonus() * 0.627);
            int32 multiplier;

            if (AuraEffect const* longWord = GetCaster()->GetDummyAuraEffect(SPELLFAMILY_PALADIN, 4127, 1))
            {
                canBeRecalculated = true;

                switch (caster->GetPower(POWER_HOLY_POWER))
                {
                    case 0:
                    {
                        bonus = bonus;
                        multiplier = 1;
                        break;
                    }
                    case 1:
                    {
                       bonus = (caster->ToPlayer()->GetTotalAttackPowerValue(BASE_ATTACK) * 0.396) + (caster->ToPlayer()->GetSpellPowerBonus() * 0.418);
                       multiplier = 2;
                       break;
                    }
                    case 2:
                    {
                       bonus = (caster->ToPlayer()->GetTotalAttackPowerValue(BASE_ATTACK) * 0.594) + (caster->ToPlayer()->GetSpellPowerBonus() * 0.627);
                       multiplier = 3;
                       break;
                    }
                }
                if (caster->HasAura(SPELL_DIVINE_PURPOSE_PROC))
                {
                    amount = ((((GetSpellInfo()->Effects[0].CalcValue(caster) * 3)  + divinebonus) * longWord->GetAmount()) / 100)  / 3;
                }
                else
                    amount = ((((GetSpellInfo()->Effects[0].CalcValue(caster) * multiplier)  + bonus) * longWord->GetAmount()) / 100)  / 3;
            }
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pal_word_of_glory_heal_AuraScript::CalculateOvertime, EFFECT_1, SPELL_AURA_PERIODIC_HEAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_pal_word_of_glory_heal_AuraScript();
    }
    SpellScript* GetSpellScript() const
    {
        return new spell_pal_word_of_glory_heal_SpellScript();
    }
};

class spell_pal_infusion_removal : public SpellScriptLoader
{
public:
	spell_pal_infusion_removal() : SpellScriptLoader("spell_pal_infusion_removal") { }

	class spell_pal_infusion_removal_SpellScript : public SpellScript
	{
		PrepareSpellScript(spell_pal_infusion_removal_SpellScript);

		void HandleExtraEffect()
		{
			Unit* caster = GetCaster();

			if (!caster || caster->GetTypeId() != TYPEID_PLAYER)
				return;

			// Holy Light or Divine Light casted

			if (caster->HasAura(53672)) // Infusion of Light rank 1 remove it.
			{
				caster->RemoveAura(53672);
			}
			if (caster->HasAura(54149)) // Infusion of Light rank 2 remove it.
			{
				caster->RemoveAura(54149);
			}
		}

		void Register()
		{
			AfterCast += SpellCastFn(spell_pal_infusion_removal_SpellScript::HandleExtraEffect);
		}
	};

	SpellScript* GetSpellScript() const
	{
		return new spell_pal_infusion_removal_SpellScript();
	}
};

class spell_pal_sol_movement_fix : public SpellScriptLoader
{
public:
	spell_pal_sol_movement_fix() : SpellScriptLoader("spell_pal_sol_movement_fix") { }

	class spell_pal_sol_movement_fix_SpellScript : public SpellScript
	{
		PrepareSpellScript(spell_pal_sol_movement_fix_SpellScript);

		void HandleExtraEffect()
		{
			Unit* caster = GetCaster();

			if (!caster || caster->GetTypeId() != TYPEID_PLAYER)
				return;

			// Holy Radiance casted and has Speed of light talents
			if (caster->HasAura(85495)) // Speed of Light rank 1
			{
				caster->CastCustomSpell(85497, SPELLVALUE_BASE_POINT0, 20, caster, true);
			}
			if (caster->HasAura(85498)) // Speed of Light rank 2
			{
				caster->CastCustomSpell(85497, SPELLVALUE_BASE_POINT0, 40, caster, true);
			}
			if (caster->HasAura(85499)) // Speed of Light rank 3
			{
				caster->CastCustomSpell(85497, SPELLVALUE_BASE_POINT0, 60, caster, true);
			}
		}

		void Register()
		{
			AfterCast += SpellCastFn(spell_pal_sol_movement_fix_SpellScript::HandleExtraEffect);
		}
	};

	SpellScript* GetSpellScript() const
	{
		return new spell_pal_sol_movement_fix_SpellScript();
	}
};

class spell_pal_selfless_healer : public SpellScriptLoader
{
    public:
        spell_pal_selfless_healer() : SpellScriptLoader("spell_pal_selfless_healer") { }

        class spell_pal_selfless_healer_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_selfless_healer_AuraScript);

            bool Load()
            {
                if (GetCaster()->GetTypeId() != TYPEID_PLAYER)
                    return false;

                if (GetCaster()->ToPlayer()->getClass() != CLASS_PALADIN)
                    return false;

                return true;
            }

            bool Validate (SpellInfo const* /*spellEntry*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_PALADIN_SELFLESS_HEALER_PROC))
                    return false;

                return true;
            }

            void CalculateBonus(AuraEffect const* /*aurEff*/, int32& amount, bool& canBeRecalculated)
            {
                if (Unit* caster = GetCaster())
                {
                    canBeRecalculated = true;

                    if (caster->HasAura(SPELL_DIVINE_PURPOSE_PROC))
                    {
                        amount = 12;
                        return;
                    }

                    switch (caster->GetPower(POWER_HOLY_POWER))
                    {
                        case 0: // 1 Holy Power
                        {
                            amount = 4;
                            break;
                        }
                        case 1: // 2 Holy Power
                        {
                            amount = 8;
                            break;
                        }
                        case 2: // 3 Holy Power
                        {
                            amount = 12;
                            break;
                        }
                    }
                }
            }

            void Register()
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pal_selfless_healer_AuraScript::CalculateBonus, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pal_selfless_healer_AuraScript();
        }
};

// -86150 - Guardian of Ancient Kings
// Fix: Summon guardian, need to implement other spells yet like Ancient Power and Ancient Fury
class spell_pal_guardian_ancient_kings : public SpellScriptLoader
{
public:
    spell_pal_guardian_ancient_kings() : SpellScriptLoader("spell_pal_guardian_ancient_kings") {}

    class spell_pal_guardian_ancient_kings_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_pal_guardian_ancient_kings_SpellScript)

        bool Load()
        {
            if (GetCaster()->GetTypeId() != TYPEID_PLAYER)
                return false;
            return true;
        }

        bool Validate (SpellInfo const* /*spellEntry*/)
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_PALADIN_GUARDIAN_ANCIENT_KINGS) ||
                !sSpellMgr->GetSpellInfo(SPELL_PALADIN_HOLY_GUARDIAN) ||
                !sSpellMgr->GetSpellInfo(SPELL_PALADIN_RETRI_GUARDIAN) ||
                !sSpellMgr->GetSpellInfo(SPELL_PALADIN_PROT_GUARDIAN))
                return false;

            return true;
        }

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            if (Unit* caster = GetCaster())
            {
                if (caster->ToPlayer()->HasSpell(20473)) // Holy Shock
                {
                    caster->CastSpell(caster, SPELL_PALADIN_HOLY_GUARDIAN, false);
                    return;
                }

                if (caster->ToPlayer()->HasSpell(85256)) // Templar's Verdict
                {
                    caster->CastSpell(caster, SPELL_PALADIN_RETRI_GUARDIAN, false);
                    return;
                }

                if (caster->ToPlayer()->HasSpell(31935)) // Avenger's shield
                {
                    caster->CastSpell(caster, SPELL_PALADIN_PROT_GUARDIAN, false);
                    return;
                }
            }
        }

        void Register()
        {
            OnEffectLaunch += SpellEffectFn(spell_pal_guardian_ancient_kings_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_pal_guardian_ancient_kings_SpellScript();
    }
};

class spell_pal_divine_storm : public SpellScriptLoader
{
public:
    spell_pal_divine_storm() : SpellScriptLoader("spell_pal_divine_storm") { }

    class spell_pal_divine_storm_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_pal_divine_storm_SpellScript);

        uint32 healPct;

        bool Load()
        {
            healPct = GetSpellInfo()->Effects[EFFECT_1].CalcValue(GetCaster());
            return true;
        }

        void TriggerHeal()
        {
            GetCaster()->CastCustomSpell(SPELL_DIVINE_STORM_DUMMY, SPELLVALUE_BASE_POINT0, (GetHitDamage() * healPct) / 100, GetCaster(), true);
        }

        void Register()
        {
            AfterHit += SpellHitFn(spell_pal_divine_storm_SpellScript::TriggerHeal);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_pal_divine_storm_SpellScript();
    }
};

class spell_pal_consecration : public SpellScriptLoader
{
public:
    spell_pal_consecration() : SpellScriptLoader("spell_pal_consecration") { }

    class spell_pal_consecration_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_pal_consecration_AuraScript)

        float x, y, z;

        bool Load()
        {
            if (GetCaster()->GetTypeId() != TYPEID_PLAYER)
                return false;

            return true;
        }

        bool Validate (SpellInfo const* /*spellEntry*/)
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_PALADIN_CONSECRATION_DAMAGE) ||
                !sSpellMgr->GetSpellInfo(SPELL_PALADIN_CONSECRATION_SUMMON))
                return false;

            return true;
        }

        void HandlePeriodicDummy(AuraEffect const* /*aurEff*/)
        {
            uint64 consecrationNpcGUID = GetCaster()->m_SummonSlot[1];

            if (!consecrationNpcGUID)
                return;

            Unit* consecrationNpc = ObjectAccessor::GetCreature(*GetCaster(),consecrationNpcGUID);

            if (!consecrationNpc)
                return;

            consecrationNpc->GetPosition(x,y,z);
            consecrationNpc->CastSpell(x,y,z,SPELL_PALADIN_CONSECRATION_DAMAGE,true,NULL,NULL,GetCaster()->GetGUID());
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_pal_consecration_AuraScript::HandlePeriodicDummy,EFFECT_1,SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_pal_consecration_AuraScript();
    }
};

class spell_pal_righteous_defense : public SpellScriptLoader
{
    public:
        spell_pal_righteous_defense() : SpellScriptLoader("spell_pal_righteous_defense") { }

        class spell_pal_righteous_defense_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_righteous_defense_SpellScript);

            SpellCastResult CheckCast()
            {
                Unit* caster = GetCaster();
                if (caster->GetTypeId() != TYPEID_PLAYER)
                    return SPELL_FAILED_DONT_REPORT;

                if (Unit* target = GetExplTargetUnit())
                {
                    if (!target->IsFriendlyTo(caster) || target->getAttackers().empty())
                        return SPELL_FAILED_BAD_TARGETS;
                }
                else
                    return SPELL_FAILED_BAD_TARGETS;

                return SPELL_CAST_OK;
            }

            void HandleSpellEffectTriggerSpell(SpellEffIndex /*effIndex*/)
            {
                if (Unit* caster = GetCaster())
                    if (Unit* targetUnit = GetHitUnit())
                        caster->CastSpell(targetUnit, SPELL_RIGHTEOUS_DEFENCE_EFFECT_1, true);
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_pal_righteous_defense_SpellScript::CheckCast);
                OnEffectHitTarget += SpellEffectFn(spell_pal_righteous_defense_SpellScript::HandleSpellEffectTriggerSpell, EFFECT_1, SPELL_EFFECT_TRIGGER_SPELL);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pal_righteous_defense_SpellScript();
        }
};

// 19740 Blessing of Might
class spell_pal_bless_of_might : public SpellScriptLoader
{
    public:
    spell_pal_bless_of_might() : SpellScriptLoader("spell_pal_bless_of_might") {}

    class spell_pal_bless_of_might_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_pal_bless_of_might_SpellScript);

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            if (Unit* caster = GetCaster())
            {
                if (caster->GetTypeId() != TYPEID_PLAYER)
                    return;

                std::list<Unit*> PartyMembers;
                caster->GetPartyMembers(PartyMembers);

                if (PartyMembers.size() > 1)
                    caster->CastSpell(GetHitUnit(), PALADIN_SPELL_BLESSING_OF_MIGHT_2, true); // Blessing of Might (Raid)
                else
                    caster->CastSpell(GetHitUnit(), PALADIN_SPELL_BLESSING_OF_MIGHT_1, true); // Blessing of Might (Caster)
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_pal_bless_of_might_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_pal_bless_of_might_SpellScript;
    }
};

// 20217 Blessing of King
class spell_pal_bless_of_king : public SpellScriptLoader
{
    public:
    spell_pal_bless_of_king() : SpellScriptLoader("spell_pal_bless_of_king") {}

    class spell_pal_bless_of_king_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_pal_bless_of_king_SpellScript);

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            if (Unit* caster = GetCaster())
            {
                if (caster->GetTypeId() != TYPEID_PLAYER)
                    return;

                std::list<Unit*> PartyMembers;
                caster->GetPartyMembers(PartyMembers);

                if (PartyMembers.size() > 1)
                    caster->CastSpell(GetHitUnit(), PALADIN_SPELL_BLESSING_OF_KINGS_2, true); // Blessing of Kings (Raid)
                else
                    caster->CastSpell(GetHitUnit(), PALADIN_SPELL_BLESSING_OF_KINGS_1, true); // Blessing of Kings (Caster)
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_pal_bless_of_king_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_pal_bless_of_king_SpellScript;
    }
};

void AddSC_paladin_spell_scripts()
{
	new spell_pal_judgementwise();
    new spell_pal_ardent_defender();
    new spell_pal_blessing_of_faith();
    new spell_pal_holy_shock();
    new spell_pal_shield_of_righteous();
    new spell_pal_righteous_defense();
    new spell_pal_judgements_of_the_bold();
    new spell_pal_word_of_glory();
    new spell_pal_selfless_healer();
    new spell_pal_guardian_ancient_kings();
    new spell_pal_divine_storm();
    new spell_pal_consecration();
    new spell_pal_bless_of_might();
    new spell_pal_bless_of_king();
	new spell_pal_illuminated_healing();
	new spell_pal_exorcism();
	new spell_pal_infusion_removal();
	new spell_pal_sol_movement_fix();
}
