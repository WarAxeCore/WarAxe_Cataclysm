INSERT INTO `skyfire_world`.`spell_script_names`(`spell_id`, `ScriptName`) VALUES (20271, 'spell_pal_judgementwise');
INSERT INTO `skyfire_world`.`spell_script_names`(`spell_id`, `ScriptName`) VALUES (31687, 'spell_mage_water_elemental');

-- Delete spells that creature shouldn't have
delete from creature_ai_scripts where action1_param1 = 71116;