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
 * Scripts for spells with SPELLFAMILY_DEATHKNIGHT and SPELLFAMILY_GENERIC spells used by deathknight players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_dk_".
 */

#include "ScriptPCH.h"
#include "Spell.h"

enum DeathKnightSpells
{
    DK_SPELL_RUNIC_POWER_ENERGIZE               = 49088,
    DK_SPELL_ANTI_MAGIC_SHELL_TALENT            = 51052,
    DK_SPELL_SCOURGE_STRIKE_TRIGGERED           = 70890,
    DK_SPELL_BLOOD_BOIL_TRIGGERED               = 65658,
    DK_SPELL_WILL_OF_THE_NECROPOLIS_TALENT_R1   = 49189,
    DK_SPELL_WILL_OF_THE_NECROPOLIS_AURA_R1     = 52284,
    DK_SPELL_BLOOD_PRESENCE                     = 48266,
    DK_SPELL_IMPROVED_BLOOD_PRESENCE_TRIGGERED  = 63611,
    DK_SPELL_UNHOLY_PRESENCE                    = 48265,
    DK_SPELL_IMPROVED_UNHOLY_PRESENCE_TRIGGERED = 63622,
    DK_SPELL_NECROTIC_STRIKE                    = 73975,
};

// HACK!
// 50422
class spell_dk_sob : public SpellScriptLoader
{
public:
	spell_dk_sob() : SpellScriptLoader("spell_dk_sob") { }

	class spell_dk_sob_SpellScript : public SpellScript
	{
		PrepareSpellScript(spell_dk_sob_SpellScript);

		void HandleExtraEffect()
		{
			Unit* caster = GetCaster();

			if (!caster->GetAura(50421))
				return;

			if (!caster || caster->GetTypeId() != TYPEID_PLAYER)
				return;

			uint32 previousStacks = caster->GetAura(50421)->GetStackAmount();
			uint32 duration = caster->GetAura(50421)->GetDuration();
			caster->RemoveAurasDueToSpell(50421);
			if (previousStacks == 3) 
			{
				caster->CastSpell(caster, 50421, true);
				caster->CastSpell(caster, 50421, true);
				if (caster->GetAura(50421))
				{
					caster->GetAura(50421)->SetDuration(duration);
				}
			}
			if (previousStacks == 2)
			{
				caster->CastSpell(caster, 50421, true);
				if (caster->GetAura(50421))
				{
					caster->GetAura(50421)->SetDuration(duration);
				}
			}
			if (previousStacks == 1)
			{
				caster->RemoveAurasDueToSpell(50421);
			}
		}

		void Register()
		{
			AfterCast += SpellCastFn(spell_dk_sob_SpellScript::HandleExtraEffect);
		}
	};

	SpellScript* GetSpellScript() const
	{
		return new spell_dk_sob_SpellScript();
	}
};

class spell_dk_necrotic_strike : public SpellScriptLoader
{
public:
    spell_dk_necrotic_strike() : SpellScriptLoader("spell_dk_necrotic_strike") { }

    class spell_dk_necrotic_strike_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_dk_necrotic_strike_SpellScript);

        bool Validate(SpellEntry const* /*spellEntry*/)
        {
            return sSpellStore.LookupEntry(DK_SPELL_NECROTIC_STRIKE);
        }

        void HandleBeforeHit()
        {
            if (Unit* target = GetHitUnit())
            {
                if (!target->HasAura(73975))
                    target->SetHealAbsorb(0.0f);
            }
        }

        void HandleAfterHit()
        {
            if (Unit* target = GetHitUnit())
            {
                float absorb = GetCaster()->GetTotalAttackPowerValue(BASE_ATTACK) * 0.75f + target->GetHealAbsorb();
                target->SetHealAbsorb(absorb);
            }
        }

        void Register()
        {
            BeforeHit += SpellHitFn(spell_dk_necrotic_strike_SpellScript::HandleBeforeHit);
            AfterHit += SpellHitFn(spell_dk_necrotic_strike_SpellScript::HandleAfterHit);
        }
    };

    SpellScript *GetSpellScript() const
    {
        return new spell_dk_necrotic_strike_SpellScript();
    }
};

// 50462 - Anti-Magic Shell (on raid member)
class spell_dk_anti_magic_shell_raid : public SpellScriptLoader
{
    public:
        spell_dk_anti_magic_shell_raid() : SpellScriptLoader("spell_dk_anti_magic_shell_raid") { }

        class spell_dk_anti_magic_shell_raid_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dk_anti_magic_shell_raid_AuraScript);

            uint32 absorbPct;

            bool Load()
            {
                absorbPct = GetSpellInfo()->Effects[EFFECT_0].CalcValue(GetCaster());
                return true;
            }

            void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
            {
                // TODO: this should absorb limited amount of damage, but no info on calculation formula
                amount = -1;
            }

            void Absorb(AuraEffect* /*aurEff*/, DamageInfo& dmgInfo, uint32& absorbAmount)
            {
                 absorbAmount = CalculatePctN(dmgInfo.GetDamage(), absorbPct);
            }

            void Register()
            {
                 DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_anti_magic_shell_raid_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                 OnEffectAbsorb += AuraEffectAbsorbFn(spell_dk_anti_magic_shell_raid_AuraScript::Absorb, EFFECT_0);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dk_anti_magic_shell_raid_AuraScript();
        }
};

// 48707 - Anti-Magic Shell (on self)
class spell_dk_anti_magic_shell_self : public SpellScriptLoader
{
    public:
        spell_dk_anti_magic_shell_self() : SpellScriptLoader("spell_dk_anti_magic_shell_self") { }

        class spell_dk_anti_magic_shell_self_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dk_anti_magic_shell_self_AuraScript);

            uint32 absorbPct, hpPct;
            bool Load()
            {
                absorbPct = GetSpellInfo()->Effects[EFFECT_0].CalcValue(GetCaster());
                if (GetCaster()->HasSpell(49224))
                    absorbPct += 8;
                if (GetCaster()->HasSpell(49610))
                    absorbPct += 16;
                if (GetCaster()->HasSpell(49611))
                    absorbPct += 25;
                hpPct = GetSpellInfo()->Effects[EFFECT_1].CalcValue(GetCaster());
                return true;
            }

            bool Validate(SpellInfo const* /*spellEntry*/)
            {
                return sSpellMgr->GetSpellInfo(DK_SPELL_RUNIC_POWER_ENERGIZE);
            }

            void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
            {
                // Set absorbtion amount to unlimited
                amount = -1;
            }

            void Absorb(AuraEffect* /*aurEff*/, DamageInfo& dmgInfo, uint32& absorbAmount)
            {
                absorbAmount = std::min(CalculatePctN(dmgInfo.GetDamage(), absorbPct), GetTarget()->CountPctFromMaxHealth(hpPct));
            }

            void Trigger(AuraEffect* aurEff, DamageInfo& /*dmgInfo*/, uint32& absorbAmount)
            {
                Unit* target = GetTarget();
                // damage absorbed by Anti-Magic Shell energizes the DK with additional runic power.
                // This, if I'm not mistaken, shows that we get back ~20% of the absorbed damage as runic power.
                int32 bp = absorbAmount * 2 / 10;
                target->CastCustomSpell(target, DK_SPELL_RUNIC_POWER_ENERGIZE, &bp, NULL, NULL, true, NULL, aurEff);
            }

            void Register()
            {
                 DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_anti_magic_shell_self_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                 OnEffectAbsorb += AuraEffectAbsorbFn(spell_dk_anti_magic_shell_self_AuraScript::Absorb, EFFECT_0);
                 AfterEffectAbsorb += AuraEffectAbsorbFn(spell_dk_anti_magic_shell_self_AuraScript::Trigger, EFFECT_0);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dk_anti_magic_shell_self_AuraScript();
        }
};

// 50461 - Anti-Magic Zone
class spell_dk_anti_magic_zone : public SpellScriptLoader
{
    public:
        spell_dk_anti_magic_zone() : SpellScriptLoader("spell_dk_anti_magic_zone") { }

        class spell_dk_anti_magic_zone_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dk_anti_magic_zone_AuraScript);

            uint32 absorbPct;

            bool Load()
            {
                absorbPct = GetSpellInfo()->Effects[EFFECT_0].CalcValue(GetCaster());
                return true;
            }

            bool Validate(SpellInfo const* /*spellEntry*/)
            {
                return sSpellMgr->GetSpellInfo(DK_SPELL_ANTI_MAGIC_SHELL_TALENT);
            }

            void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& /*canBeRecalculated*/)
            {
                SpellInfo const* talentSpell = sSpellMgr->GetSpellInfo(DK_SPELL_ANTI_MAGIC_SHELL_TALENT);
                amount = talentSpell->Effects[EFFECT_0].CalcValue(GetCaster());
                if (Unit* caster = GetCaster())
                {
                    if (Player* player = caster->ToPlayer())
                        amount += int32(2 * player->GetTotalAttackPowerValue(BASE_ATTACK));
                }
            }

            void Absorb(AuraEffect* /*aurEff*/, DamageInfo& dmgInfo, uint32& absorbAmount)
            {
                 absorbAmount = CalculatePctN(dmgInfo.GetDamage(), absorbPct);
            }

            void Register()
            {
                 DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_anti_magic_zone_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                 OnEffectAbsorb += AuraEffectAbsorbFn(spell_dk_anti_magic_zone_AuraScript::Absorb, EFFECT_0);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_dk_anti_magic_zone_AuraScript();
        }
};

class spell_dk_death_gate : public SpellScriptLoader
{
    public:
        spell_dk_death_gate() : SpellScriptLoader("spell_dk_death_gate") {}

        class spell_dk_death_gate_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_death_gate_SpellScript);

            SpellCastResult CheckClass()
            {
                if (GetCaster()->getClass() != CLASS_DEATH_KNIGHT)
                {
                    SetCustomCastResultMessage(SPELL_CUSTOM_ERROR_MUST_BE_DEATH_KNIGHT);
                    return SPELL_FAILED_CUSTOM_ERROR;
                }

                return SPELL_CAST_OK;
            }

            void HandleScript(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                if (!GetHitUnit())
                    return;
                GetHitUnit()->CastSpell(GetHitUnit(), GetEffectValue(), false);
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_dk_death_gate_SpellScript::CheckClass);
                OnEffectHitTarget += SpellEffectFn(spell_dk_death_gate_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dk_death_gate_SpellScript();
        }
};

class spell_dk_death_pact : public SpellScriptLoader
{
    public:
        spell_dk_death_pact() : SpellScriptLoader("spell_dk_death_pact") { }

        class spell_dk_death_pact_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_death_pact_SpellScript);

            void FilterTargets(std::list<WorldObject*>& unitList)
            {
                Unit* unit_to_add = NULL;
                for (std::list<WorldObject*>::iterator itr = unitList.begin() ; itr != unitList.end(); ++itr)
                {
                    if (Unit* unit = (*itr)->ToUnit())
                    {
                        if (unit->GetOwnerGUID() == GetCaster()->GetGUID() && unit->GetCreatureType() == CREATURE_TYPE_UNDEAD)
                        {
                            unit_to_add = unit;
                            break;
                        }
                    }
                }

                unitList.clear();
                if (unit_to_add)
                    unitList.push_back(unit_to_add);
                else
                {
                    // Pet not found - remove cooldown
                    if (Player* modOwner = GetCaster()->GetSpellModOwner())
                        modOwner->RemoveSpellCooldown(GetSpellInfo()->Id, true);
                    FinishCast(SPELL_FAILED_NO_PET);
                }
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dk_death_pact_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ALLY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dk_death_pact_SpellScript();
        }
};

// 55090 Scourge Strike (55265, 55270, 55271)
class spell_dk_scourge_strike : public SpellScriptLoader
{
    public:
        spell_dk_scourge_strike() : SpellScriptLoader("spell_dk_scourge_strike") { }

        class spell_dk_scourge_strike_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_scourge_strike_SpellScript);

            bool Validate(SpellInfo const* /*spellEntry*/)
            {
                if (!sSpellMgr->GetSpellInfo(DK_SPELL_SCOURGE_STRIKE_TRIGGERED))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                if (Unit* unitTarget = GetHitUnit())
                {
                    int32 bp = CalculatePctN(GetHitDamage(), GetEffectValue() * unitTarget->GetDiseasesByCaster(caster->GetGUID()));
                    caster->CastCustomSpell(unitTarget, DK_SPELL_SCOURGE_STRIKE_TRIGGERED, &bp, NULL, NULL, true);
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_dk_scourge_strike_SpellScript::HandleDummy, EFFECT_2, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dk_scourge_strike_SpellScript();
        }
};

// 48721 Blood Boil
class spell_dk_blood_boil : public SpellScriptLoader
{
    public:
        spell_dk_blood_boil() : SpellScriptLoader("spell_dk_blood_boil") { }

        class spell_dk_blood_boil_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_blood_boil_SpellScript);

            bool Validate(SpellInfo const* /*spellEntry*/)
            {
                if (!sSpellMgr->GetSpellInfo(DK_SPELL_BLOOD_BOIL_TRIGGERED))
                    return false;
                return true;
            }

            bool Load()
            {
                _executed = false;
                return GetCaster()->GetTypeId() == TYPEID_PLAYER && GetCaster()->getClass() == CLASS_DEATH_KNIGHT;
            }

            void HandleAfterHit()
            {
                if (_executed || !GetHitUnit())
                    return;

                _executed = true;
                GetCaster()->CastSpell(GetCaster(), DK_SPELL_BLOOD_BOIL_TRIGGERED, true);
            }

            void Register()
            {
                AfterHit += SpellHitFn(spell_dk_blood_boil_SpellScript::HandleAfterHit);
            }

            bool _executed;
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dk_blood_boil_SpellScript();
        }
};

// 50365, 50371 Improved Blood Presence
class spell_dk_improved_blood_presence : public SpellScriptLoader
{
public:
    spell_dk_improved_blood_presence() : SpellScriptLoader("spell_dk_improved_blood_presence") { }

    class spell_dk_improved_blood_presence_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dk_improved_blood_presence_AuraScript)
        bool Validate(SpellInfo const* /*entry*/)
        {
            if (!sSpellMgr->GetSpellInfo(DK_SPELL_BLOOD_PRESENCE))
                return false;
            if (!sSpellMgr->GetSpellInfo(DK_SPELL_IMPROVED_BLOOD_PRESENCE_TRIGGERED))
                return false;
            return true;
        }

        void HandleEffectApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            Unit* target = GetTarget();
            if (!target->HasAura(DK_SPELL_BLOOD_PRESENCE) && !target->HasAura(DK_SPELL_IMPROVED_BLOOD_PRESENCE_TRIGGERED))
            {
                int32 basePoints1 = aurEff->GetAmount();
                target->CastCustomSpell(target, 63611, NULL, &basePoints1, NULL, true, 0, aurEff);
            }
        }

        void HandleEffectRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            Unit* target = GetTarget();
            if (!target->HasAura(DK_SPELL_BLOOD_PRESENCE))
                target->RemoveAura(DK_SPELL_IMPROVED_BLOOD_PRESENCE_TRIGGERED);
        }

        void Register()
        {
            AfterEffectApply += AuraEffectApplyFn(spell_dk_improved_blood_presence_AuraScript::HandleEffectApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            AfterEffectRemove += AuraEffectRemoveFn(spell_dk_improved_blood_presence_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_dk_improved_blood_presence_AuraScript();
    }
};

// 50391, 50392 Improved Unholy Presence
class spell_dk_improved_unholy_presence : public SpellScriptLoader
{
public:
    spell_dk_improved_unholy_presence() : SpellScriptLoader("spell_dk_improved_unholy_presence") { }

    class spell_dk_improved_unholy_presence_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dk_improved_unholy_presence_AuraScript)
        bool Validate(SpellInfo const* /*entry*/)
        {
            if (!sSpellMgr->GetSpellInfo(DK_SPELL_UNHOLY_PRESENCE))
                return false;
            if (!sSpellMgr->GetSpellInfo(DK_SPELL_IMPROVED_UNHOLY_PRESENCE_TRIGGERED))
                return false;
            return true;
        }

        void HandleEffectApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            Unit* target = GetTarget();
            if (target->HasAura(DK_SPELL_UNHOLY_PRESENCE) && !target->HasAura(DK_SPELL_IMPROVED_UNHOLY_PRESENCE_TRIGGERED))
            {
                // Not listed as any effect, only base points set in dbc
                int32 basePoints0 = aurEff->GetSpellInfo()->Effects[EFFECT_1].CalcValue();
                target->CastCustomSpell(target, DK_SPELL_IMPROVED_UNHOLY_PRESENCE_TRIGGERED, &basePoints0 , &basePoints0, &basePoints0, true, 0, aurEff);
            }
        }

        void HandleEffectRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            Unit* target = GetTarget();
            if (target->HasAura(DK_SPELL_UNHOLY_PRESENCE))
                target->RemoveAura(DK_SPELL_IMPROVED_UNHOLY_PRESENCE_TRIGGERED);
        }

        void Register()
        {
            AfterEffectApply += AuraEffectApplyFn(spell_dk_improved_unholy_presence_AuraScript::HandleEffectApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            AfterEffectRemove += AuraEffectRemoveFn(spell_dk_improved_unholy_presence_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_dk_improved_unholy_presence_AuraScript();
    }
};

// Festering Strike
// Spell Id: 85948
class spell_dk_festering_strike : public SpellScriptLoader
{
    public:
        spell_dk_festering_strike() : SpellScriptLoader("spell_dk_festering_strike") { }

        class spell_dk_festering_strike_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_festering_strike_SpellScript);

            void HandleScript(SpellEffIndex /*eff*/)
            {
                if (Unit* target = GetHitUnit())
                {
                    uint32 addDuration = urand(2, 6);
                    if (target->HasAura(45524)) // Chains of Ice
                        target->GetAura(45524)->SetDuration(target->GetAura(45524)->GetDuration() + (addDuration * 1000), true);
                    if (target->HasAura(55095)) // Frost Fever
                        target->GetAura(55095)->SetDuration(target->GetAura(55095)->GetDuration() + (addDuration * 1000), true);
                    if (target->HasAura(55078)) // Blood Plague
                        target->GetAura(55078)->SetDuration(target->GetAura(55078)->GetDuration() + (addDuration * 1000), true);
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_dk_festering_strike_SpellScript::HandleScript, EFFECT_2, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dk_festering_strike_SpellScript();
        }
};

// Chains Of Ice
// Spell Id: 45524
class spell_dk_chains_of_ice : public SpellScriptLoader
{
    public:
        spell_dk_chains_of_ice() : SpellScriptLoader("spell_dk_chains_of_ice") { }

        class spell_dk_chains_of_ice_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_chains_of_ice_SpellScript);

            void HandleEffect(SpellEffIndex /*eff*/)
            {
                if (Unit* target = GetHitUnit())
                {
                    if (GetCaster()->HasAura(50041)) // Chilblains Rank 2
                        GetCaster()->CastSpell(target, 96294, true);
                    else if (GetCaster()->HasAura(50040)) // Chilblains Rank 1
                        GetCaster()->CastSpell(target, 96293, true);
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_dk_chains_of_ice_SpellScript::HandleEffect, EFFECT_2, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_dk_chains_of_ice_SpellScript();
        }
};

class spell_dk_death_grip : public SpellScriptLoader
{
public:
    spell_dk_death_grip() : SpellScriptLoader("spell_dk_death_grip") { }

    class spell_dk_death_grip_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_dk_death_grip_SpellScript);

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            int32 damage = GetEffectValue();
            Position const* pos = GetExplTargetDest();
            if (Unit* target = GetHitUnit())
            {
                if (!target->HasAuraType(SPELL_AURA_DEFLECT_SPELLS)) // Deterrence
                    target->CastSpell(pos->GetPositionX(), pos->GetPositionY(), pos->GetPositionZ(), damage, true);
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_dk_death_grip_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_dk_death_grip_SpellScript();
    }
};

enum DeathCoil
{
    SPELL_DEATH_COIL_DAMAGE     = 47632,
    SPELL_DEATH_COIL_HEAL       = 47633,
    SPELL_SIGIL_VENGEFUL_HEART  = 64962,
};

class spell_dk_death_coil : public SpellScriptLoader
{
public:
    spell_dk_death_coil() : SpellScriptLoader("spell_dk_death_coil") { }

    class spell_dk_death_coil_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_dk_death_coil_SpellScript);

        bool Validate(SpellInfo const* /*SpellEntry*/)
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_DEATH_COIL_DAMAGE) || !sSpellMgr->GetSpellInfo(SPELL_DEATH_COIL_HEAL))
                return false;
            return true;
        }

        void HandleDummy(SpellEffIndex /* effIndex */)
        {
            int32 damage = GetEffectValue();
            Unit* caster = GetCaster();
            if (Unit* target = GetHitUnit())
            {
                if (caster->IsFriendlyTo(target))
                {
                    int32 bp = int32(damage * 1.5f);
                    caster->CastCustomSpell(target, SPELL_DEATH_COIL_HEAL, &bp, NULL, NULL, true);
                }
                else
                {
                    if (AuraEffect const* auraEffect = caster->GetAuraEffect(SPELL_SIGIL_VENGEFUL_HEART, EFFECT_1))
                        damage += auraEffect->GetBaseAmount();
                    caster->CastCustomSpell(target, SPELL_DEATH_COIL_DAMAGE, &damage, NULL, NULL, true);
                }
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_dk_death_coil_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_dk_death_coil_SpellScript();
    }
};

void AddSC_deathknight_spell_scripts()
{
	new spell_dk_sob();
    new spell_dk_necrotic_strike();
    new spell_dk_anti_magic_shell_raid();
    new spell_dk_anti_magic_shell_self();
    new spell_dk_anti_magic_zone();
    new spell_dk_death_gate();
    new spell_dk_death_pact();
    new spell_dk_scourge_strike();
    new spell_dk_blood_boil();
    new spell_dk_improved_blood_presence();
    new spell_dk_improved_unholy_presence();
    new spell_dk_festering_strike();
    new spell_dk_chains_of_ice();
    new spell_dk_death_grip();
    new spell_dk_death_coil();
}
