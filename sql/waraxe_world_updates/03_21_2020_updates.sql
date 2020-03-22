-- Soothing Rain talent fix
INSERT INTO `skyfire_world`.`spell_script_names`(`spell_id`, `ScriptName`) VALUES (52041, 'spell_sha_healing_stream_totem');
-- Heroic Leap damage fix
INSERT INTO `skyfire_world`.`spell_bonus_data`(`entry`, `direct_bonus`, `dot_bonus`, `ap_bonus`, `ap_dot_bonus`, `comments`) VALUES (52174, 0, 0, 0.5, 0, 'Warrior - Heroic Leap');