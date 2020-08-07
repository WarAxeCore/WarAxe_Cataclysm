//Copyright WarAxeCore Team 2020


class npc_eurak_qc : public CreatureScript
{
public:
	npc_eurak_qc() : CreatureScript("npc_eurak_qc") {}

	bool OnQuestComplete(Player* player, Creature* /*creature*/, Quest const* quest)
	{
		if (quest->GetQuestId() == 25281)
		{
			player->SetPhaseMask(1, true);
		}
		return true;
	}
};

void AddSC_vashjir()
{
	new npc_eurak_qc();
}