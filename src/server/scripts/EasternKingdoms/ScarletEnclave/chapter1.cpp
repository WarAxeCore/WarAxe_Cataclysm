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

#include "ScriptPCH.h"
#include "Vehicle.h"
#include "ObjectMgr.h"
#include "ScriptedEscortAI.h"

/*######
##Quest 12848
######*/

#define GCD_CAST    1

enum eDeathKnightSpells
{
    SPELL_SOUL_PRISON_CHAIN_SELF    = 54612,
    SPELL_SOUL_PRISON_CHAIN         = 54613,
    SPELL_DK_INITIATE_VISUAL        = 51519,

    SPELL_ICY_TOUCH                 = 52372,
    SPELL_PLAGUE_STRIKE             = 52373,
    SPELL_BLOOD_STRIKE              = 52374,
    SPELL_DEATH_COIL                = 52375
};

#define EVENT_ICY_TOUCH                 1
#define EVENT_PLAGUE_STRIKE             2
#define EVENT_BLOOD_STRIKE              3
#define EVENT_DEATH_COIL                4

//used by 29519, 29520, 29565, 29566, 29567 but signed for 29519
int32 say_event_start[8] =
{
    -1609000, -1609001, -1609002, -1609003,
    -1609004, -1609005, -1609006, -1609007
};

int32 say_event_attack[9] =
{
    -1609008, -1609009, -1609010, -1609011, -1609012,
    -1609013, -1609014, -1609015, -1609016
};

uint32 acherus_soul_prison[12] =
{
    191577,
    191580,
    191581,
    191582,
    191583,
    191584,
    191585,
    191586,
    191587,
    191588,
    191589,
    191590
};

uint32 acherus_unworthy_initiate[5] =
{
    29519,
    29520,
    29565,
    29566,
    29567
};

enum UnworthyInitiatePhase
{
    PHASE_CHAINED,
    PHASE_TO_EQUIP,
    PHASE_EQUIPING,
    PHASE_TO_ATTACK,
    PHASE_ATTACKING,
};

class npc_unworthy_initiate : public CreatureScript
{
public:
    npc_unworthy_initiate() : CreatureScript("npc_unworthy_initiate") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_unworthy_initiateAI(creature);
    }

    struct npc_unworthy_initiateAI : public ScriptedAI
    {
        npc_unworthy_initiateAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetReactState(REACT_PASSIVE);
            if (!me->GetEquipmentId())
                if (const CreatureTemplate* info = sObjectMgr->GetCreatureTemplate(28406))
                    if (info->equipmentId)
                        const_cast<CreatureTemplate*>(me->GetCreatureTemplate())->equipmentId = info->equipmentId;
        }

        uint64 playerGUID;
        UnworthyInitiatePhase phase;
        uint32 wait_timer;
        float anchorX, anchorY;
        uint64 anchorGUID;

        EventMap events;

        void Reset()
        {
            anchorGUID = 0;
            phase = PHASE_CHAINED;
            events.Reset();
            me->setFaction(7);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
            me->SetUInt32Value(UNIT_FIELD_BYTES_1, 8);
            me->LoadEquipment(0, true);
        }

        void EnterCombat(Unit* /*who*/)
        {
            events.ScheduleEvent(EVENT_ICY_TOUCH, 1000, GCD_CAST);
            events.ScheduleEvent(EVENT_PLAGUE_STRIKE, 3000, GCD_CAST);
            events.ScheduleEvent(EVENT_BLOOD_STRIKE, 2000, GCD_CAST);
            events.ScheduleEvent(EVENT_DEATH_COIL, 5000, GCD_CAST);
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type != POINT_MOTION_TYPE)
                return;

            if (id == 1)
            {
                wait_timer = 5000;
                me->CastSpell(me, SPELL_DK_INITIATE_VISUAL, true);

                if (Player* starter = Unit::GetPlayer(*me, playerGUID))
                    DoScriptText(say_event_attack[rand()%9], me, starter);

                phase = PHASE_TO_ATTACK;
            }
        }

        void EventStart(Creature* anchor, Player* target)
        {
            wait_timer = 5000;
            phase = PHASE_TO_EQUIP;

            me->SetUInt32Value(UNIT_FIELD_BYTES_1, 0);
            me->RemoveAurasDueToSpell(SPELL_SOUL_PRISON_CHAIN_SELF);
            me->RemoveAurasDueToSpell(SPELL_SOUL_PRISON_CHAIN);

            float z;
            anchor->GetContactPoint(me, anchorX, anchorY, z, 1.0f);

            playerGUID = target->GetGUID();
            DoScriptText(say_event_start[rand()%8], me, target);
        }

        void UpdateAI(const uint32 diff)
        {
            switch (phase)
            {
            case PHASE_CHAINED:
                if (!anchorGUID)
                {
                    if (Creature* anchor = me->FindNearestCreature(29521, 30))
                    {
                        anchor->AI()->SetGUID(me->GetGUID());
                        anchor->CastSpell(me, SPELL_SOUL_PRISON_CHAIN, true);
                        anchorGUID = anchor->GetGUID();
                    }
                    else
                        sLog->outError("npc_unworthy_initiateAI: unable to find anchor!");

                    float dist = 99.0f;
                    GameObject* prison = NULL;

                    for (uint8 i = 0; i < 12; ++i)
                    {
                        if (GameObject* temp_prison = me->FindNearestGameObject(acherus_soul_prison[i], 30))
                        {
                            if (me->IsWithinDist(temp_prison, dist, false))
                            {
                                dist = me->GetDistance2d(temp_prison);
                                prison = temp_prison;
                            }
                        }
                    }

                    if (prison)
                        prison->ResetDoorOrButton();
                    else
                        sLog->outError("npc_unworthy_initiateAI: unable to find prison!");
                }
                break;
            case PHASE_TO_EQUIP:
                if (wait_timer)
                {
                    if (wait_timer > diff)
                        wait_timer -= diff;
                    else
                    {
                        me->GetMotionMaster()->MovePoint(1, anchorX, anchorY, me->GetPositionZ());
                        //sLog->outDebug(LOG_FILTER_TSCR, "npc_unworthy_initiateAI: move to %f %f %f", anchorX, anchorY, me->GetPositionZ());
                        phase = PHASE_EQUIPING;
                        wait_timer = 0;
                    }
                }
                break;
            case PHASE_TO_ATTACK:
                if (wait_timer)
                {
                    if (wait_timer > diff)
                        wait_timer -= diff;
                    else
                    {
                        me->setFaction(14);
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                        phase = PHASE_ATTACKING;

                        if (Player* target = Unit::GetPlayer(*me, playerGUID))
                            me->AI()->AttackStart(target);
                        wait_timer = 0;
                    }
                }
                break;
            case PHASE_ATTACKING:
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                    case EVENT_ICY_TOUCH:
                        DoCast(me->getVictim(), SPELL_ICY_TOUCH);
                        events.DelayEvents(1000, GCD_CAST);
                        events.ScheduleEvent(EVENT_ICY_TOUCH, 5000, GCD_CAST);
                        break;
                    case EVENT_PLAGUE_STRIKE:
                        DoCast(me->getVictim(), SPELL_PLAGUE_STRIKE);
                        events.DelayEvents(1000, GCD_CAST);
                        events.ScheduleEvent(SPELL_PLAGUE_STRIKE, 5000, GCD_CAST);
                        break;
                    case EVENT_BLOOD_STRIKE:
                        DoCast(me->getVictim(), SPELL_BLOOD_STRIKE);
                        events.DelayEvents(1000, GCD_CAST);
                        events.ScheduleEvent(EVENT_BLOOD_STRIKE, 5000, GCD_CAST);
                        break;
                    case EVENT_DEATH_COIL:
                        DoCast(me->getVictim(), SPELL_DEATH_COIL);
                        events.DelayEvents(1000, GCD_CAST);
                        events.ScheduleEvent(EVENT_DEATH_COIL, 5000, GCD_CAST);
                        break;
                    }
                }

                DoMeleeAttackIfReady();
                break;
            default:
                break;
            }
        }
    };
};

class npc_unworthy_initiate_anchor : public CreatureScript
{
public:
    npc_unworthy_initiate_anchor() : CreatureScript("npc_unworthy_initiate_anchor") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_unworthy_initiate_anchorAI(creature);
    }

    struct npc_unworthy_initiate_anchorAI : public PassiveAI
    {
        npc_unworthy_initiate_anchorAI(Creature* creature) : PassiveAI(creature), prisonerGUID(0) {}

        uint64 prisonerGUID;

        void SetGUID(uint64 guid, int32 /*id*/)
        {
            if (!prisonerGUID)
                prisonerGUID = guid;
        }

        uint64 GetGUID(int32 /*id*/) { return prisonerGUID; }
    };
};

class go_acherus_soul_prison : public GameObjectScript
{
public:
    go_acherus_soul_prison() : GameObjectScript("go_acherus_soul_prison") { }

    bool OnGossipHello(Player* player, GameObject* go)
    {
        if (Creature* anchor = go->FindNearestCreature(29521, 15))
            if (uint64 prisonerGUID = anchor->AI()->GetGUID())
                if (Creature* prisoner = Creature::GetCreature(*player, prisonerGUID))
                    CAST_AI(npc_unworthy_initiate::npc_unworthy_initiateAI, prisoner->AI())->EventStart(anchor, player);

        return false;
    }
};

/*######
## npc_eye_of_acherus
######*/

enum EyeOfAcherus
{
    DISPLAYID_EYE_HUGE          = 26320, // Big Blue
    DISPLAYID_EYE_SMALL         = 25499, // Little green

    SPELL_EYE_PHASEMASK         = 70889,
    SPELL_EYE_VISUAL            = 51892,
    SPELL_EYE_FL_BOOST_RUN      = 51923, // Flight boost to reach new avalon
    SPELL_EYE_FL_BOOST_FLY      = 51890, // Player flight control
    SPELL_EYE_CONTROL           = 51852, // Player control aura

    POINT_EYE_DESTINATION       = 0,

    EMOTE_DESTINATION           = -1609017,
    EMOTE_CONTROL               = -1609018
};

static Position Center[]=
{
    { 2361.21f, -5660.45f, 496.7444f, 0.0f },
};

class npc_eye_of_acherus : public CreatureScript
{
public:
    npc_eye_of_acherus() : CreatureScript("npc_eye_of_acherus") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_eye_of_acherusAI(creature);
    }

    struct npc_eye_of_acherusAI : public ScriptedAI
    {
        npc_eye_of_acherusAI(Creature* creature) : ScriptedAI(creature)
        {
            Reset();
        }

        uint32 startTimer;
        bool IsActive;

        void Reset()
        {
            if (Unit* controller = me->GetCharmer())
                me->SetLevel(controller->getLevel());
                me->CastSpell(me, SPELL_EYE_FL_BOOST_FLY, true);                
                
                me->SetDisplayId(DISPLAYID_EYE_HUGE);
                me->SetHomePosition(2361.21f, -5660.45f, 496.7444f, 0);
                me->GetMotionMaster()->MoveCharge(1758.007f, -5876.785f, 166.8667f, 0); // Position center                
                if (Player* player = me->GetCharmerOrOwnerPlayerOrPlayerItself())
                DoScriptText(EMOTE_DESTINATION, me, player);
                
                me->SetReactState(REACT_AGGRESSIVE);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_STUNNED);

            IsActive   = false;
            startTimer = 4500; // Increased startTimer for testing (orig.2000)
        }

        void AttackStart(Unit *) {}
        void MoveInLineOfSight(Unit *) {}

        void JustDied(Unit* /*who*/)
        {
            if (Unit* charmer = me->GetCharmer())
               charmer->RemoveAurasDueToSpell(SPELL_EYE_CONTROL);
        }

        void UpdateAI(const uint32 diff)
        {
            if (me->isCharmed())
            {
                if (startTimer <= diff && !IsActive)    // Summon to start point
                {
                    me->CastSpell(me, SPELL_EYE_PHASEMASK, true);
                    me->CastSpell(me, SPELL_EYE_VISUAL, true);
                    me->CastSpell(me, SPELL_EYE_FL_BOOST_FLY, true);

                    me->CastSpell(me, SPELL_EYE_FL_BOOST_RUN, true);
                    me->SetSpeed(MOVE_FLIGHT, 3.5f, true); // 6.5spd by sniffs, much to fast.
                    me->GetMotionMaster()->MovePoint(0, 1758.0f, -5876.7f, 166.8f); // Travel to end point.
                    return;
                }
                else
                    startTimer -= diff;
            }
            else
                me->ForcedDespawn();
        }

        void MovementInform(Player* player, uint32 type, uint32 pointId)
        {
            if (type != POINT_MOTION_TYPE || pointId != POINT_EYE_DESTINATION)
               return;

            // This should never happen
            me->SetDisplayId(DISPLAYID_EYE_SMALL);

            // This spell does not work if casted before the wp movement.
            me->CastSpell(me, SPELL_EYE_VISUAL, true);
            me->CastSpell(me, SPELL_EYE_FL_BOOST_FLY, true);
            ((Player*)(me->GetCharmer()))->SetClientControl(me, 1);
            DoScriptText(EMOTE_CONTROL, me, player); 
        }
    };
};

/*######
## npc_death_knight_initiate
######*/

#define GOSSIP_ACCEPT_DUEL      "I challenge you, death knight!"

enum DuelEnums
{
    SAY_DUEL_A                  = -1609080,
    SAY_DUEL_B                  = -1609081,
    SAY_DUEL_C                  = -1609082,
    SAY_DUEL_D                  = -1609083,
    SAY_DUEL_E                  = -1609084,
    SAY_DUEL_F                  = -1609085,
    SAY_DUEL_G                  = -1609086,
    SAY_DUEL_H                  = -1609087,
    SAY_DUEL_I                  = -1609088,

    SPELL_DUEL                  = 52996,
    //SPELL_DUEL_TRIGGERED        = 52990,
    SPELL_DUEL_VICTORY          = 52994,
    SPELL_DUEL_FLAG             = 52991,
    SPELL_GROVEL                = 7267,

    QUEST_DEATH_CHALLENGE       = 12733,
    FACTION_HOSTILE             = 2068
};

int32 _auiRandomSay[] =
{
    SAY_DUEL_A, SAY_DUEL_B, SAY_DUEL_C, SAY_DUEL_D, SAY_DUEL_E, SAY_DUEL_F, SAY_DUEL_G, SAY_DUEL_H, SAY_DUEL_I
};

class npc_death_knight_initiate : public CreatureScript
{
public:
    npc_death_knight_initiate() : CreatureScript("npc_death_knight_initiate") { }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*Sender*/, uint32 Action)
    {
        player->PlayerTalkClass->ClearMenus();
        if (Action == GOSSIP_ACTION_INFO_DEF)
        {
            player->CLOSE_GOSSIP_MENU();

            if (player->isInCombat() || creature->isInCombat())
                return true;

            if (npc_death_knight_initiateAI* pInitiateAI = CAST_AI(npc_death_knight_initiate::npc_death_knight_initiateAI, creature->AI()))
            {
                if (pInitiateAI->_bIsDuelInProgress)
                    return true;
            }

            creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
            creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNK_15);

            int32 uiSayId = rand()% (sizeof(_auiRandomSay)/sizeof(int32));
            DoScriptText(_auiRandomSay[uiSayId], creature, player);

            player->CastSpell(creature, SPELL_DUEL, false);
            player->CastSpell(player, SPELL_DUEL_FLAG, true);
        }
        return true;
    }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (player->GetQuestStatus(QUEST_DEATH_CHALLENGE) == QUEST_STATUS_INCOMPLETE && creature->IsFullHealth())
        {
            if (player->HealthBelowPct(10))
                return true;

            if (player->isInCombat() || creature->isInCombat())
                return true;

            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ACCEPT_DUEL, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
            player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
        }
        return true;
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_death_knight_initiateAI(creature);
    }

    struct npc_death_knight_initiateAI : public CombatAI
    {
        npc_death_knight_initiateAI(Creature* creature) : CombatAI(creature)
        {
            _bIsDuelInProgress = false;
        }

        bool lose;
        uint64 DuelerGUID;
        uint32 DuelTimer;
        bool _bIsDuelInProgress;

        void Reset()
        {
            lose = false;
            me->RestoreFaction();
            CombatAI::Reset();

            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNK_15);

            DuelerGUID = 0;
            DuelTimer = 5000;
            _bIsDuelInProgress = false;
        }

        void SpellHit(Unit* caster, const SpellInfo* spell)
        {
            if (!_bIsDuelInProgress && spell->Id == SPELL_DUEL)
            {
                DuelerGUID = caster->GetGUID();
                _bIsDuelInProgress = true;
            }
        }

       void DamageTaken(Unit* doneBy, uint32 &Damage)
        {
            if (_bIsDuelInProgress && doneBy->IsControlledByPlayer())
            {
                if (doneBy->GetGUID() != DuelerGUID && doneBy->GetOwnerGUID() != DuelerGUID) // other players cannot help
                    Damage = 0;
                else if (Damage >= me->GetHealth())
                {
                    Damage = 0;

                    if (!lose)
                    {
                        doneBy->RemoveGameObject(SPELL_DUEL_FLAG, true);
                        doneBy->AttackStop();
                        doneBy->getHostileRefManager().deleteReferences();
                        me->CastSpell(doneBy, SPELL_DUEL_VICTORY, true);
                        lose = true;
                        me->CastSpell(me, SPELL_GROVEL, true);
                        me->RestoreFaction();
                    }
                }
            }
        }

        void UpdateAI(const uint32 Diff)
        {
            if (!UpdateVictim())
            {
                if (_bIsDuelInProgress)
                {
                    if (DuelTimer <= Diff)
                    {
                        me->setFaction(FACTION_HOSTILE);

                        if (Unit* unit = Unit::GetUnit(*me, DuelerGUID))
                            AttackStart(unit);
                    }
                    else
                        DuelTimer -= Diff;
                }
                return;
            }

            if (_bIsDuelInProgress)
            {
                if (lose)
                {
                    if (!me->HasAura(SPELL_GROVEL))
                        EnterEvadeMode();
                    return;
                }
                else if (me->getVictim()->GetTypeId() == TYPEID_PLAYER && me->getVictim()->HealthBelowPct(10))
                {
                    me->getVictim()->CastSpell(me->getVictim(), SPELL_GROVEL, true); // beg
                    me->getVictim()->RemoveGameObject(SPELL_DUEL_FLAG, true);
                    EnterEvadeMode();
                    return;
                }
            }

            // TODO: spells

            CombatAI::UpdateAI(Diff);
        }
    };
};

/*######
## npc_dark_rider_of_acherus
######*/

enum DarkRider
{
    SPELL_DESPAWN_HORSE    = 52267
};

#define SAY_DARK_RIDER      "The realm of shadows awaits..."

class npc_dark_rider_of_acherus : public CreatureScript
{
public:
    npc_dark_rider_of_acherus() : CreatureScript("npc_dark_rider_of_acherus") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_dark_rider_of_acherusAI(creature);
    }

    struct npc_dark_rider_of_acherusAI : public ScriptedAI
    {
        npc_dark_rider_of_acherusAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 PhaseTimer;
        uint32 Phase;
        bool Intro;
        uint64 TargetGUID;

        void Reset()
        {
            PhaseTimer = 4000;
            Phase = 0;
            Intro = false;
            TargetGUID = 0;
        }

        void UpdateAI(const uint32 diff)
        {
            if (!Intro || !TargetGUID)
                return;

            if (PhaseTimer <= diff)
            {
                switch (Phase)
                {
                   case 0:
                        me->MonsterSay(SAY_DARK_RIDER, LANGUAGE_UNIVERSAL, 0);
                        PhaseTimer = 5000;
                        Phase = 1;
                        break;
                    case 1:
                        if (Unit* target = Unit::GetUnit(*me, TargetGUID))
                            DoCast(target, SPELL_DESPAWN_HORSE, true);
                        PhaseTimer = 3000;
                        Phase = 2;
                        break;
                    case 2:
                        me->SetVisible(false);
                        PhaseTimer = 2000;
                        Phase = 3;
                        break;
                    case 3:
                        me->DespawnOrUnsummon();
                        break;
                    default:
                        break;
                }
            } else PhaseTimer -= diff;
        }

        void InitDespawnHorse(Unit* who)
        {
            if (!who)
                return;

            TargetGUID = who->GetGUID();
            me->SetWalk(true);
            me->SetSpeed(MOVE_RUN, 0.4f);
            me->GetMotionMaster()->MoveChase(who);
            me->SetTarget(TargetGUID);
            Intro = true;
        }
    };
};

/*######
## npc_salanar_the_horseman
######*/

enum SalanarTheHorseman
{
    REALM_OF_SHADOWS            = 52693,
    EFFECT_STOLEN_HORSE         = 52263,
    DELIVER_STOLEN_HORSE        = 52264,
    CALL_DARK_RIDER             = 52266,
    SPELL_EFFECT_OVERTAKE       = 52349,
    QUEST_REALM_OF_SHADOWS      = 12687,
    NPC_DARK_RIDER_OF_ACHERUS   = 28654,
    NPC_SALANAR_THE_HORSEMAN    = 28788
};

class npc_salanar_the_horseman : public CreatureScript
{
public:
    npc_salanar_the_horseman() : CreatureScript("npc_salanar_the_horseman") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_salanar_the_horsemanAI(creature);
    }

    struct npc_salanar_the_horsemanAI : public ScriptedAI
    {
        npc_salanar_the_horsemanAI(Creature* creature) : ScriptedAI(creature) {}

        void SpellHit(Unit* caster, const SpellInfo* spell)
        {
            if (spell->Id == DELIVER_STOLEN_HORSE)
            {
                if (caster->GetTypeId() == TYPEID_UNIT && caster->IsVehicle())
                {
                    if (Unit* charmer = caster->GetCharmer())
                    {
                        if (charmer->HasAura(EFFECT_STOLEN_HORSE))
                        {
                            charmer->RemoveAurasDueToSpell(EFFECT_STOLEN_HORSE);
                            caster->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
                            caster->setFaction(35);
                            DoCast(caster, CALL_DARK_RIDER, true);
                            if (Creature* Dark_Rider = me->FindNearestCreature(NPC_DARK_RIDER_OF_ACHERUS, 15))
                                CAST_AI(npc_dark_rider_of_acherus::npc_dark_rider_of_acherusAI, Dark_Rider->AI())->InitDespawnHorse(caster);
                        }
                    }
                }
            }
        }

        void MoveInLineOfSight(Unit* who)
        {
            ScriptedAI::MoveInLineOfSight(who);

			if (who->GetTypeId() == TYPEID_PLAYER && who->ToPlayer()->GetQuestStatus(QUEST_REALM_OF_SHADOWS) == QUEST_STATUS_INCOMPLETE)
			{
				who->ToPlayer()->GroupEventHappens(QUEST_REALM_OF_SHADOWS, me);
			}

            if (who->GetTypeId() == TYPEID_UNIT && who->IsVehicle() && me->IsWithinDistInMap(who, 5.0f))
            {
                if (Unit* charmer = who->GetCharmer())
                {
                    if (charmer->GetTypeId() == TYPEID_PLAYER)
                    {
                        // for quest Into the Realm of Shadows(QUEST_REALM_OF_SHADOWS)
                        if (me->GetEntry() == NPC_SALANAR_THE_HORSEMAN && CAST_PLR(charmer)->GetQuestStatus(QUEST_REALM_OF_SHADOWS) == QUEST_STATUS_INCOMPLETE)
                        {
                            CAST_PLR(charmer)->GroupEventHappens(QUEST_REALM_OF_SHADOWS, me);
                            charmer->RemoveAurasDueToSpell(SPELL_EFFECT_OVERTAKE);
                            CAST_CRE(who)->ForcedDespawn();
                            //CAST_CRE(who)->Respawn(true);
                        }

                        if (CAST_PLR(charmer)->HasAura(REALM_OF_SHADOWS))
                            charmer->RemoveAurasDueToSpell(REALM_OF_SHADOWS);
                    }
                }
            }
        }
    };
};

/*######
## npc_ros_dark_rider
######*/

enum RosDarkRider
{
    NPC_DEATHCHARGER     = 28782
};

class npc_ros_dark_rider : public CreatureScript
{
public:
    npc_ros_dark_rider() : CreatureScript("npc_ros_dark_rider") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_ros_dark_riderAI(creature);
    }

    struct npc_ros_dark_riderAI : public ScriptedAI
    {
        npc_ros_dark_riderAI(Creature* creature) : ScriptedAI(creature) {}

        void EnterCombat(Unit* /*who*/)
        {
            me->ExitVehicle();
        }

        void Reset()
        {
            Creature* deathcharger = me->FindNearestCreature(NPC_DEATHCHARGER, 30);
            if (!deathcharger) return;
            deathcharger->RestoreFaction();
            deathcharger->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
            deathcharger->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            //if (!me->GetVehicle() && deathcharger->IsVehicle() && deathcharger->GetVehicleKit()->HasEmptySeat(0))
                //me->EnterVehicle(deathcharger);
        }

        void JustDied(Unit* killer)
        {
            Creature* deathcharger = me->FindNearestCreature(NPC_DEATHCHARGER, 30);
            if (!deathcharger) return;
            if (killer->GetTypeId() == TYPEID_PLAYER && deathcharger->GetTypeId() == TYPEID_UNIT && deathcharger->IsVehicle())
            {
				deathcharger->setFaction(35);
                deathcharger->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
                deathcharger->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

				if (!killer->GetVehicle() && deathcharger->IsVehicle() && deathcharger->GetVehicleKit()->HasEmptySeat(0))
					killer->EnterVehicle(deathcharger);
            }
        }
    };
};

// correct way: 52312 52314 52555 ...
enum SG
{
    NPC_SCARLET_GHOULS     = 28845,
    NPC_SCARLET_GHOSTS     = 28846,

    QUEST_GIFT_THAT_KEEPS_GIVING   = 12698,
    SPELL_SCARLET_GHOUL_CREDIT     = 52517
};

class npc_dkc1_gothik : public CreatureScript
{
public:
    npc_dkc1_gothik() : CreatureScript("npc_dkc1_gothik") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_dkc1_gothikAI(creature);
    }

    struct npc_dkc1_gothikAI : public ScriptedAI
    {
        npc_dkc1_gothikAI(Creature* creature) : ScriptedAI(creature) {}

        void MoveInLineOfSight(Unit* who)
        {
            ScriptedAI::MoveInLineOfSight(who);

            if (who->GetEntry() == NPC_SCARLET_GHOULS && me->IsWithinDistInMap(who, 10.0f))
            {
                if (Unit* owner = who->GetOwner())
                {
                    if (owner->GetTypeId() == TYPEID_PLAYER)
                    {
                        if (CAST_PLR(owner)->GetQuestStatus(QUEST_GIFT_THAT_KEEPS_GIVING) == QUEST_STATUS_INCOMPLETE)
                            CAST_CRE(who)->CastSpell(owner, SPELL_SCARLET_GHOUL_CREDIT, true);

                        //Todo: Creatures must not be removed, but, must instead
                        //      stand next to Gothik and be commanded into the pit
                        //      and dig into the ground.
                        CAST_CRE(who)->ForcedDespawn();

                        if (CAST_PLR(owner)->GetQuestStatus(QUEST_GIFT_THAT_KEEPS_GIVING) == QUEST_STATUS_COMPLETE)
                            owner->RemoveAllMinionsByEntry(NPC_SCARLET_GHOULS);
                    }
                }
            }
        }
    };
};

class npc_scarlet_ghoul : public CreatureScript
{
public:
    npc_scarlet_ghoul() : CreatureScript("npc_scarlet_ghoul") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_scarlet_ghoulAI(creature);
    }

    struct npc_scarlet_ghoulAI : public ScriptedAI
    {
        npc_scarlet_ghoulAI(Creature* creature) : ScriptedAI(creature)
        {
            // Ghouls should display their Birth Animation
            // Crawling out of the ground
            //DoCast(me, 35177, true);
            //me->MonsterSay("Mommy?", LANGUAGE_UNIVERSAL, 0);
            me->SetReactState(REACT_DEFENSIVE);
        }

        void FindMinions(Unit* owner)
        {
            std::list<Creature*> MinionList;
            owner->GetAllMinionsByEntry(MinionList, NPC_SCARLET_GHOULS);

            if (!MinionList.empty())
            {
                for (std::list<Creature*>::const_iterator itr = MinionList.begin(); itr != MinionList.end(); ++itr)
                {
                    if ((*itr)->GetOwner()->GetGUID() == me->GetOwner()->GetGUID())
                    {
                        if ((*itr)->isInCombat() && (*itr)->getAttackerForHelper())
                        {
                            AttackStart((*itr)->getAttackerForHelper());
                        }
                    }
                }
            }
        }

        void UpdateAI(const uint32 /*diff*/)
        {
            if (!me->isInCombat())
            {
                if (Unit* owner = me->GetOwner())
                {
                    if (owner->GetTypeId() == TYPEID_PLAYER && CAST_PLR(owner)->isInCombat())
                    {
                        if (CAST_PLR(owner)->getAttackerForHelper() && CAST_PLR(owner)->getAttackerForHelper()->GetEntry() == NPC_SCARLET_GHOSTS)
                        {
                            AttackStart(CAST_PLR(owner)->getAttackerForHelper());
                        }
                        else
                        {
                            FindMinions(owner);
                        }
                    }
                }
            }

            if (!UpdateVictim())
                return;

            //ScriptedAI::UpdateAI(diff);
            //Check if we have a current target
            if (me->getVictim()->GetEntry() == NPC_SCARLET_GHOSTS)
            {
                if (me->isAttackReady())
                {
                    //If we are within range melee the target
                    if (me->IsWithinMeleeRange(me->getVictim()))
                    {
                        me->AttackerStateUpdate(me->getVictim());
                        me->resetAttackTimer();
                    }
                }
            }
        }
    };
};

/*####
## npc_scarlet_miner_cart
####*/

enum MinerCart
{
    SPELL_CART_CHECK     = 54173,
    SPELL_CART_DRAG      = 52465
};

class npc_scarlet_miner_cart : public CreatureScript
{
public:
    npc_scarlet_miner_cart() : CreatureScript("npc_scarlet_miner_cart") { }

    CreatureAI* GetAI(Creature* _Creature) const
    {
        return new npc_scarlet_miner_cartAI(_Creature);
    }

    struct npc_scarlet_miner_cartAI : public PassiveAI
    {
        npc_scarlet_miner_cartAI(Creature* creature) : PassiveAI(creature), minerGUID(0)
        {
            me->setFaction(35);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
            me->SetDisplayId(me->GetCreatureTemplate()->Modelid1); // Modelid2 is a horse.
        }

        uint64 minerGUID;

        void SetGUID(uint64 guid, int32 /*id*/)
        {
            minerGUID = guid;
        }

        void DoAction(const int32 /*param*/)
        {
            if (Creature* miner = Unit::GetCreature(*me, minerGUID))
            {
                me->SetWalk(false);

                //Not 100% correct, but movement is smooth. Sometimes miner walks faster
                //than normal, this speed is fast enough to keep up at those times.
                me->SetSpeed(MOVE_RUN, 1.25f);

                me->GetMotionMaster()->MoveFollow(miner, 1.0f, 0);
            }
        }

        void PassengerBoarded(Unit* /*who*/, int8 /*seatId*/, bool apply)
        {
            if (!apply)
                if (Creature* miner = Unit::GetCreature(*me, minerGUID))
                    miner->DisappearAndDie();
        }
    };
};

/*####
## npc_scarlet_miner
####*/

#define SAY_SCARLET_MINER1  "Where'd this come from? I better get this down to the ships before the foreman sees it!"
#define SAY_SCARLET_MINER2  "Now I can have a rest!"

class npc_scarlet_miner : public CreatureScript
{
public:
    npc_scarlet_miner() : CreatureScript("npc_scarlet_miner") { }

    CreatureAI* GetAI(Creature* _Creature) const
    {
        return new npc_scarlet_minerAI(_Creature);
    }

    struct npc_scarlet_minerAI : public npc_escortAI
    {
        npc_scarlet_minerAI(Creature* creature) : npc_escortAI(creature)
        {
            me->SetReactState(REACT_PASSIVE);
        }

        uint32 IntroTimer;
        uint32 IntroPhase;
        uint64 carGUID;

        void Reset()
        {
            carGUID = 0;
            IntroTimer = 0;
            IntroPhase = 0;
            me->RestoreFaction();
        }

        void InitWaypoint()
        {
            AddWaypoint(1, 2389.03f,     -5902.74f,     109.014f, 5000);
            AddWaypoint(2, 2341.812012f, -5900.484863f, 102.619743f);
            AddWaypoint(3, 2306.561279f, -5901.738281f, 91.792419f);
            AddWaypoint(4, 2300.098389f, -5912.618652f, 86.014885f);
            AddWaypoint(5, 2294.142090f, -5927.274414f, 75.316849f);
            AddWaypoint(6, 2286.984375f, -5944.955566f, 63.714966f);
            AddWaypoint(7, 2280.001709f, -5961.186035f, 54.228283f);
            AddWaypoint(8, 2259.389648f, -5974.197754f, 42.359348f);
            AddWaypoint(9, 2242.882812f, -5984.642578f, 32.827850f);
            AddWaypoint(10, 2217.265625f, -6028.959473f, 7.675705f);
            AddWaypoint(11, 2202.595947f, -6061.325684f, 5.882018f);
            AddWaypoint(12, 2188.974609f, -6080.866699f, 3.370027f);

            if (urand(0, 1))
            {
                AddWaypoint(13, 2176.483887f, -6110.407227f, 1.855181f);
                AddWaypoint(14, 2172.516602f, -6146.752441f, 1.074235f);
                AddWaypoint(15, 2138.918457f, -6158.920898f, 1.342926f);
                AddWaypoint(16, 2129.866699f, -6174.107910f, 4.380779f);
                AddWaypoint(17, 2117.709473f, -6193.830078f, 13.3542f, 10000);
            }
            else
            {
                AddWaypoint(13, 2184.190186f, -6166.447266f, 0.968877f);
                AddWaypoint(14, 2234.265625f, -6163.741211f, 0.916021f);
                AddWaypoint(15, 2268.071777f, -6158.750977f, 1.822252f);
                AddWaypoint(16, 2270.028320f, -6176.505859f, 6.340538f);
                AddWaypoint(17, 2271.739014f, -6195.401855f, 13.3542f, 10000);
            }
        }

        void InitCartQuest(Player* who)
        {
            carGUID = who->GetVehicleBase()->GetGUID();
            InitWaypoint();
            Start(false, false, who->GetGUID());
            SetDespawnAtFar(false);
        }

        void WaypointReached(uint32 i)
        {
            switch (i)
            {
                case 1:
                    if (Unit* car = Unit::GetCreature(*me, carGUID))
                    {
                        me->SetInFront(car);
                        me->SendMovementFlagUpdate();
                    }
                    me->MonsterSay(SAY_SCARLET_MINER1, LANGUAGE_UNIVERSAL, 0);
                    SetRun(true);
                    IntroTimer = 4000;
                    IntroPhase = 1;
                    break;
                case 17:
                    if (Unit* car = Unit::GetCreature(*me, carGUID))
                    {
                        me->SetInFront(car);
                        me->SendMovementFlagUpdate();
                        car->Relocate(car->GetPositionX(), car->GetPositionY(), me->GetPositionZ() + 1);
                        car->StopMoving();
                        car->RemoveAura(SPELL_CART_DRAG);
                    }
                    me->MonsterSay(SAY_SCARLET_MINER2, LANGUAGE_UNIVERSAL, 0);
                    break;
                default:
                    break;
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (IntroPhase)
            {
                if (IntroTimer <= diff)
                {
                    if (IntroPhase == 1)
                    {
                        if (Creature* car = Unit::GetCreature(*me, carGUID))
                            DoCast(car, SPELL_CART_DRAG);
                        IntroTimer = 800;
                        IntroPhase = 2;
                    }
                    else
                    {
                        if (Creature* car = Unit::GetCreature(*me, carGUID))
                            car->AI()->DoAction(0);
                        IntroPhase = 0;
                    }
                } else IntroTimer-=diff;
            }
            npc_escortAI::UpdateAI(diff);
        }
    };
};

/*######
## go_inconspicuous_mine_car
######*/

enum MineCar
{
    SPELL_HIDE_IN_MINE_CAR          = 52463,
    NPC_SCARLET_MINER               = 28841,
    NPC_MINE_CAR                    = 28817,
    QUEST_MASSACRE_AT_LIGHTS_POINT  = 12701
};

class go_inconspicuous_mine_car : public GameObjectScript
{
public:
    go_inconspicuous_mine_car() : GameObjectScript("go_inconspicuous_mine_car") { }

    bool OnGossipHello(Player* player, GameObject* /*go*/)
    {
        if (player->GetQuestStatus(QUEST_MASSACRE_AT_LIGHTS_POINT) == QUEST_STATUS_INCOMPLETE)
        {
            // Hack Why SkyFire Dont Support Custom Summon Location
            if (Creature* miner = player->SummonCreature(NPC_SCARLET_MINER, 2383.869629f, -5900.312500f, 107.996086f, player->GetOrientation(), TEMPSUMMON_DEAD_DESPAWN, 1))
            {
                player->CastSpell(player, SPELL_HIDE_IN_MINE_CAR, true);
                if (Creature* car = player->GetVehicleCreatureBase())
                {
                    if (car->GetEntry() == NPC_MINE_CAR)
                    {
                        car->AI()->SetGUID(miner->GetGUID());
                        CAST_AI(npc_scarlet_miner::npc_scarlet_minerAI, miner->AI())->InitCartQuest(player);
                    } else sLog->outError("TSCR: OnGossipHello vehicle entry is not correct.");
                } else sLog->outError("TSCR: OnGossipHello player is not on the vehicle.");
            } else sLog->outError("TSCR: OnGossipHello Scarlet Miner cant be found by script.");
        }
        return true;
    }
};

// npc 28912 quest 17217 boss 29001 mob 29007 go 191092

void AddSC_the_scarlet_enclave_c1()
{
    new npc_unworthy_initiate();
    new npc_unworthy_initiate_anchor();
    new go_acherus_soul_prison();
    new npc_eye_of_acherus();
    new npc_death_knight_initiate();
    new npc_salanar_the_horseman();
    new npc_dark_rider_of_acherus();
    new npc_ros_dark_rider();
    new npc_dkc1_gothik();
    new npc_scarlet_ghoul();
    new npc_scarlet_miner();
    new npc_scarlet_miner_cart();
    new go_inconspicuous_mine_car();
}
