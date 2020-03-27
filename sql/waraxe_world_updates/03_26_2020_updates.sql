-- Fix incorrect Control Demon spell
UPDATE `npc_trainer` SET `spell` = '93375' WHERE `npc_trainer`.`spell` =80388;

--fix currency for cata
update skyfire_world.quest_template set rewcurrencyid1 = 395, rewcurrencycount1 = 140 where entry = 28907;
update skyfire_world.quest_template set rewcurrencyid1 = 395, rewcurrencycount1 = 70 where entry = 28906;
update skyfire_world.quest_template set rewcurrencyid1 = 396, rewcurrencycount1 = 70 where entry = 28905;