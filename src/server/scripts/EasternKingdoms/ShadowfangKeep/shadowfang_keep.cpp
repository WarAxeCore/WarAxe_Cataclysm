/*
 * Copyright (C) 2011-2019 Project SkyFire <http://www.projectskyfire.org/>
 * Copyright (C) 2008-2019 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2006-2019 ScriptDev2 <https://github.com/scriptdev2/scriptdev2/>
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

/* ScriptData
SDName: Shadowfang_Keep
SD%Complete: 75
SDComment: npc_shadowfang_prisoner using escortAI for movement to door. Might need additional code in case being attacked. Add proper texts/say().
SDCategory: Shadowfang Keep
EndScriptData */

/* ContentData
npc_shadowfang_prisoner
EndContentData */

#include "ScriptPCH.h"
#include "ScriptedEscortAI.h"
#include "shadowfang_keep.h"

class npc_haunted_stable_hand : public CreatureScript
{
public:
	npc_haunted_stable_hand() : CreatureScript("npc_haunted_stable_hand") { }

	bool OnGossipSelect(Player* player, Creature* /*creature*/, uint32 Sender, uint32 action) override
	{
		player->PlayerTalkClass->ClearMenus();

		if (Sender != GOSSIP_SENDER_MAIN)
			return true;
		if (!player->getAttackers().empty())
			return true;

		switch (action)
		{
		case GOSSIP_ACTION_INFO_DEF + 1:
			player->TeleportTo(33, -225.70f, 2269.67f, 74.999f, 2.76f);
			player->CLOSE_GOSSIP_MENU();
			break;
		case GOSSIP_ACTION_INFO_DEF + 2:
			player->TeleportTo(33, -260.66f, 2246.97f, 100.89f, 2.43f);
			player->CLOSE_GOSSIP_MENU();
			break;
		case GOSSIP_ACTION_INFO_DEF + 3:
			player->TeleportTo(33, -171.28f, 2182.020f, 129.255f, 5.92f);
			player->CLOSE_GOSSIP_MENU();
			break;
		}
		return true;
	}

	bool OnGossipHello(Player* player, Creature* creature) override
	{
		InstanceScript* instance = creature->GetInstanceScript();

		if (instance && instance->GetData(DATA_BARON_SILVERLAINE_EVENT) == DONE)
			player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Teleport me to Baron Silverlaine", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
		if (instance && instance->GetData(DATA_COMMANDER_SPRINGVALE_EVENT) == DONE)
			player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Teleport me to Commander Springvale", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
		if (instance && instance->GetData(DATA_LORD_WALDEN_EVENT) == DONE)
			player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Teleport me to Lord Walden", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);

		player->SEND_GOSSIP_MENU(2475, creature->GetGUID());
		return true;
	}
};

void AddSC_shadowfang_keep()
{
	new npc_haunted_stable_hand();
}
