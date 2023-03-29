-- Halls of Orgination
UPDATE `skyfire_world`.`creature_template` SET `ScriptName` = 'boss_temple_guardian_anhuur' WHERE `entry` = 39425;
UPDATE `skyfire_world`.`creature_template` SET `ScriptName` = 'npc_pit_snake' WHERE `entry` = 39444;
UPDATE `skyfire_world`.`gameobject_template` SET `ScriptName` = 'go_beacon_of_light' WHERE `entry` = 203136;
UPDATE `skyfire_world`.`gameobject_template` SET `ScriptName` = 'go_beacon_of_light' WHERE `entry` = 203133;
UPDATE `skyfire_world`.`gameobject_template` SET `ScriptName` = 'go_beacon_of_light' WHERE `entry` = 207219;
UPDATE `skyfire_world`.`gameobject_template` SET `ScriptName` = 'go_beacon_of_light' WHERE `entry` = 207218;
DELETE FROM `skyfire_world`.`achievement_criteria_data` WHERE `criteria_id` = 15988;
INSERT INTO `skyfire_world`.`achievement_criteria_data` (`criteria_id`, `type`, `value1`, `value2`, `ScriptName`) VALUES (15988, 11, 0, 0, 'achievement_i_hate_that_song');