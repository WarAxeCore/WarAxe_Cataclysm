-- Fix incorrect Control Demon spell
UPDATE `npc_trainer` SET `spell` = '93375' WHERE `npc_trainer`.`spell` = 80388;

-- fix currency for cata
update skyfire_world.quest_template set RewCurrencyId1 = 395, RewCurrencyCount1 = 140 where entry = 28907;
update skyfire_world.quest_template set RewCurrencyId1 = 395, RewCurrencyCount1 = 70 where entry = 28906;
update skyfire_world.quest_template set RewCurrencyId1 = 396, RewCurrencyCount1 = 70 where entry = 28905;