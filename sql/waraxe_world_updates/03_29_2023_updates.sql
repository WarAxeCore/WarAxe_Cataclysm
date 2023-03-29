-- Halls of Orgination
UPDATE `skyfire_world`.`creature_template` SET `ScriptName` = 'boss_temple_guardian_anhuur' WHERE `entry` = 39425;
UPDATE `skyfire_world`.`creature_template` SET `ScriptName` = 'npc_pit_snake' WHERE `entry` = 39444;
UPDATE `skyfire_world`.`gameobject_template` SET `ScriptName` = 'go_beacon_of_light' WHERE `entry` = 203136;
UPDATE `skyfire_world`.`gameobject_template` SET `ScriptName` = 'go_beacon_of_light' WHERE `entry` = 203133;
UPDATE `skyfire_world`.`gameobject_template` SET `ScriptName` = 'go_beacon_of_light' WHERE `entry` = 207219;
UPDATE `skyfire_world`.`gameobject_template` SET `ScriptName` = 'go_beacon_of_light' WHERE `entry` = 207218;
DELETE FROM `skyfire_world`.`achievement_criteria_data` WHERE `criteria_id` = 15988;
INSERT INTO `skyfire_world`.`achievement_criteria_data` (`criteria_id`, `type`, `value1`, `value2`, `ScriptName`) VALUES (15988, 11, 0, 0, 'achievement_i_hate_that_song');
UPDATE `skyfire_world`.`creature_template` SET `ScriptName` = 'boss_earthrager_ptah' WHERE `entry` = 39428;
UPDATE `skyfire_world`.`creature_template` SET `ScriptName` = 'npc_ptah_dustbone_horror' WHERE `entry` = 40450;
UPDATE `skyfire_world`.`creature_template` SET `ScriptName` = 'boss_ammunae' WHERE `entry` = 39731;
UPDATE `skyfire_world`.`creature_template` SET `ScriptName` = 'npc_ammunae_seedling_pod' WHERE `entry` = 40550;
UPDATE `skyfire_world`.`creature_template` SET `ScriptName` = 'npc_ammunae_spore' WHERE `entry` = 40585;
UPDATE `skyfire_world`.`creature_template` SET `ScriptName` = 'boss_anraphet' WHERE `entry` = 39788;
UPDATE `skyfire_world`.`creature_template` SET `ScriptName` = 'npc_alpha_beam' WHERE `entry` = 41144;
UPDATE `skyfire_world`.`creature_template` SET `ScriptName` = 'boss_isiset' WHERE `entry` = 39587;
UPDATE `skyfire_world`.`creature_template` SET `ScriptName` = 'npc_isiset_astral_rain' WHERE `entry` = 39720;
UPDATE `skyfire_world`.`creature_template` SET `ScriptName` = 'npc_isiset_celestial_call' WHERE `entry` = 39721;
UPDATE `skyfire_world`.`creature_template` SET `ScriptName` = 'npc_isiset_veil_of_sky' WHERE `entry` = 39722;
UPDATE `skyfire_world`.`creature_template` SET `ScriptName` = 'npc_isiset_astral_familiar' WHERE `entry` = 39795;
DELETE FROM `skyfire_world`.`spell_script_names` WHERE `spell_id` = 74137;
DELETE FROM `skyfire_world`.`spell_script_names` WHERE `spell_id` = 76670;
INSERT INTO `skyfire_world`.`spell_script_names`(`spell_id`, `ScriptName`) VALUES (74137, 'spell_isiset_supernova_dis');
INSERT INTO `skyfire_world`.`spell_script_names`(`spell_id`, `ScriptName`) VALUES (76670, 'spell_isiset_supernova_dmg');
UPDATE `skyfire_world`.`creature_template` SET `ScriptName` = 'boss_rajh' WHERE `entry` = 39378;
UPDATE `skyfire_world`.`creature_template` SET `ScriptName` = 'npc_rajh_solar_wind' WHERE `entry` = 39364;
DELETE FROM `skyfire_world`.`achievement_criteria_data` WHERE `criteria_id` = 15990;
INSERT INTO `skyfire_world`.`achievement_criteria_data` (`criteria_id`, `type`, `value1`, `value2`, `ScriptName`) VALUES (15990, 11, 0, 0, 'achievement_sun_of_a');
UPDATE `skyfire_world`.`creature_template` SET `ScriptName` = 'boss_setesh' WHERE `entry` = 39732;
UPDATE `skyfire_world`.`creature_template` SET `ScriptName` = 'npc_setesh_chaos_portal' WHERE `entry` = 41055;
UPDATE `skyfire_world`.`creature_template` SET `ScriptName` = 'npc_setesh_void_sentinel' WHERE `entry` = 41208;
UPDATE `skyfire_world`.`creature_template` SET `ScriptName` = 'npc_setesh_void_seeker' WHERE `entry` = 41371;

-- Throne of Tides
UPDATE `skyfire_world`.`creature_template` SET `ScriptName` = 'boss_lady_nazjar' WHERE `entry` = 40586;
UPDATE `skyfire_world`.`creature_template` SET `ScriptName` = 'npc_lady_nazjar_honnor_guard' WHERE `entry` = 40633;
UPDATE `skyfire_world`.`creature_template` SET `ScriptName` = 'npc_lady_nazjar_tempest_witch' WHERE `entry` = 44404;
UPDATE `skyfire_world`.`creature_template` SET `ScriptName` = 'npc_lady_nazjar_waterspout' WHERE `entry` = 48571;
UPDATE `skyfire_world`.`creature_template` SET `ScriptName` = 'npc_lady_nazjar_geyser' WHERE `entry` = 40597;

UPDATE `skyfire_world`.`creature_template` SET `ScriptName` = 'boss_commander_ulthok' WHERE `entry` = 40765;
UPDATE `skyfire_world`.`creature_template` SET `ScriptName` = 'npc_ulthok_dark_fissure' WHERE `entry` = 40784;
DELETE FROM `skyfire_world`.`areatrigger_scripts` WHERE `entry` = 5873;
INSERT INTO `skyfire_world`.`areatrigger_scripts` (`entry`, `ScriptName`) VALUES (5873, 'at_tott_commander_ulthok');