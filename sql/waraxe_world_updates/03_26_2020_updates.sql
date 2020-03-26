-- Fix incorrect Control Demon spell
UPDATE `npc_trainer` SET `spell` = '93375' WHERE `npc_trainer`.`spell` =80388;