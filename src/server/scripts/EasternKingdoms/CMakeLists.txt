# Copyright (C) 2011-2019 Project SkyFire <http://www.projectskyfire.org/>
# Copyright (C) 2008-2019 TrinityCore <http://www.trinitycore.org/>
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

set(scripts_STAT_SRCS
  ${scripts_STAT_SRCS}
  EasternKingdoms/AlteracValley/boss_galvangar.cpp
  EasternKingdoms/AlteracValley/boss_balinda.cpp
  EasternKingdoms/AlteracValley/boss_drekthar.cpp
  EasternKingdoms/AlteracValley/boss_vanndar.cpp
  EasternKingdoms/AlteracValley/alterac_valley.cpp

  EasternKingdoms/Scholomance/boss_the_ravenian.cpp
  EasternKingdoms/Scholomance/boss_instructor_malicia.cpp
  EasternKingdoms/Scholomance/boss_death_knight_darkreaver.cpp
  EasternKingdoms/Scholomance/boss_illucia_barov.cpp
  EasternKingdoms/Scholomance/scholomance.h
  EasternKingdoms/Scholomance/boss_vectus.cpp
  EasternKingdoms/Scholomance/boss_jandice_barov.cpp
  EasternKingdoms/Scholomance/boss_kormok.cpp
  EasternKingdoms/Scholomance/boss_lord_alexei_barov.cpp
  EasternKingdoms/Scholomance/boss_doctor_theolen_krastinov.cpp
  EasternKingdoms/Scholomance/boss_darkmaster_gandling.cpp
  EasternKingdoms/Scholomance/instance_scholomance.cpp
  EasternKingdoms/Scholomance/boss_lorekeeper_polkelt.cpp
  EasternKingdoms/Scholomance/boss_ras_frostwhisper.cpp

  EasternKingdoms/ZulGurub/boss_hakkar.cpp
  EasternKingdoms/ZulGurub/boss_mandokir.cpp
  EasternKingdoms/ZulGurub/boss_marli.cpp
  EasternKingdoms/ZulGurub/boss_hazzarah.cpp
  EasternKingdoms/ZulGurub/boss_jeklik.cpp
  EasternKingdoms/ZulGurub/boss_grilek.cpp
  EasternKingdoms/ZulGurub/zulgurub.h
  EasternKingdoms/ZulGurub/boss_renataki.cpp
  EasternKingdoms/ZulGurub/boss_arlokk.cpp
  EasternKingdoms/ZulGurub/boss_gahzranka.cpp
  EasternKingdoms/ZulGurub/boss_venoxis.cpp
  EasternKingdoms/ZulGurub/instance_zulgurub.cpp
  EasternKingdoms/ZulGurub/boss_jindo.cpp
  EasternKingdoms/ZulGurub/boss_wushoolay.cpp
  EasternKingdoms/ZulGurub/boss_thekal.cpp

  EasternKingdoms/Gnomeregan/instance_gnomeregan.cpp
  EasternKingdoms/Gnomeregan/gnomeregan.cpp
  EasternKingdoms/Gnomeregan/gnomeregan.h

  EasternKingdoms/GrimBatol/grim_batol.h
  EasternKingdoms/GrimBatol/boss_drahga_shadowburner.cpp
  EasternKingdoms/GrimBatol/boss_erudax.cpp
  EasternKingdoms/GrimBatol/boss_forgemaster_throngus.cpp
  EasternKingdoms/GrimBatol/boss_general_umbriss.cpp
  EasternKingdoms/GrimBatol/instance_grim_batol.cpp

  EasternKingdoms/Gilneas/gilneas.cpp
# EasternKingdoms/Gilneas/gilneas2.cpp 
  EasternKingdoms/Gilneas/gilneas.h

  EasternKingdoms/BlackrockDepths/boss_high_interrogator_gerstahn.cpp
  EasternKingdoms/BlackrockDepths/boss_gorosh_the_dervish.cpp
  EasternKingdoms/BlackrockDepths/blackrock_depths.cpp
  EasternKingdoms/BlackrockDepths/boss_anubshiah.cpp
  EasternKingdoms/BlackrockDepths/boss_tomb_of_seven.cpp
  EasternKingdoms/BlackrockDepths/boss_general_angerforge.cpp
  EasternKingdoms/BlackrockDepths/boss_grizzle.cpp
  EasternKingdoms/BlackrockDepths/boss_ambassador_flamelash.cpp
  EasternKingdoms/BlackrockDepths/instance_blackrock_depths.cpp
  EasternKingdoms/BlackrockDepths/boss_moira_bronzebeard.cpp
  EasternKingdoms/BlackrockDepths/blackrock_depths.h
  EasternKingdoms/BlackrockDepths/boss_emperor_dagran_thaurissan.cpp
  EasternKingdoms/BlackrockDepths/boss_magmus.cpp

  EasternKingdoms/BastionOfTwilight/instance_bastion_of_twilight.cpp
  EasternKingdoms/BastionOfTwilight/boss_assembly_of_ascendents.cpp
  EasternKingdoms/BastionOfTwilight/boss_halfus_wyrmbreaker.cpp
  EasternKingdoms/BastionOfTwilight/boss_theralion_and_valiona.cpp
  EasternKingdoms/BastionOfTwilight/boss_chogall.cpp
  EasternKingdoms/BastionOfTwilight/boss_sinestra.cpp
  EasternKingdoms/BastionOfTwilight/bastion_of_twilight.h

  EasternKingdoms/ScarletEnclave/chapter2.cpp
  EasternKingdoms/ScarletEnclave/chapter5.cpp
  EasternKingdoms/ScarletEnclave/chapter1.cpp
  EasternKingdoms/ScarletEnclave/the_scarlet_enclave.cpp

  EasternKingdoms/MoltenCore/boss_gehennas.cpp
  EasternKingdoms/MoltenCore/boss_lucifron.cpp
  EasternKingdoms/MoltenCore/boss_golemagg.cpp
  EasternKingdoms/MoltenCore/boss_majordomo_executus.cpp
  EasternKingdoms/MoltenCore/boss_baron_geddon.cpp
  EasternKingdoms/MoltenCore/boss_ragnaros.cpp
  EasternKingdoms/MoltenCore/boss_garr.cpp
  EasternKingdoms/MoltenCore/molten_core.h
  EasternKingdoms/MoltenCore/instance_molten_core.cpp
  EasternKingdoms/MoltenCore/boss_sulfuron_harbinger.cpp
  EasternKingdoms/MoltenCore/boss_magmadar.cpp
  EasternKingdoms/MoltenCore/boss_shazzrah.cpp

  EasternKingdoms/Stratholme/boss_baroness_anastari.cpp
  EasternKingdoms/Stratholme/boss_nerubenkan.cpp
  EasternKingdoms/Stratholme/instance_stratholme.cpp
  EasternKingdoms/Stratholme/boss_dathrohan_balnazzar.cpp
  EasternKingdoms/Stratholme/boss_timmy_the_cruel.cpp
  EasternKingdoms/Stratholme/boss_baron_rivendare.cpp
  EasternKingdoms/Stratholme/boss_magistrate_barthilas.cpp
  EasternKingdoms/Stratholme/boss_order_of_silver_hand.cpp
  EasternKingdoms/Stratholme/boss_ramstein_the_gorger.cpp
  EasternKingdoms/Stratholme/boss_cannon_master_willey.cpp
  EasternKingdoms/Stratholme/boss_maleki_the_pallid.cpp
  EasternKingdoms/Stratholme/boss_postmaster_malown.cpp
  EasternKingdoms/Stratholme/stratholme.h
  EasternKingdoms/Stratholme/stratholme.cpp

  EasternKingdoms/ThroneOfTheTides/throne_of_the_tides.h
  EasternKingdoms/ThroneOfTheTides/boss_commander_ulthok.cpp
  EasternKingdoms/ThroneOfTheTides/boss_erunak_stonespeaker.cpp
  EasternKingdoms/ThroneOfTheTides/boss_lady_nazjar.cpp
  EasternKingdoms/ThroneOfTheTides/npc_neptulon.cpp
  EasternKingdoms/ThroneOfTheTides/instance_throne_of_the_tides.cpp

  EasternKingdoms/SunkenTemple/sunken_temple.cpp
  EasternKingdoms/SunkenTemple/sunken_temple.h
  EasternKingdoms/SunkenTemple/instance_sunken_temple.cpp

  EasternKingdoms/MagistersTerrace/boss_felblood_kaelthas.cpp
  EasternKingdoms/MagistersTerrace/magisters_terrace.h
  EasternKingdoms/MagistersTerrace/boss_priestess_delrissa.cpp
  EasternKingdoms/MagistersTerrace/instance_magisters_terrace.cpp
  EasternKingdoms/MagistersTerrace/boss_selin_fireheart.cpp
  EasternKingdoms/MagistersTerrace/boss_vexallus.cpp
  EasternKingdoms/MagistersTerrace/magisters_terrace.cpp

  EasternKingdoms/Uldaman/uldaman.cpp
  EasternKingdoms/Uldaman/boss_ironaya.cpp
  EasternKingdoms/Uldaman/uldaman.h
  EasternKingdoms/Uldaman/instance_uldaman.cpp
  EasternKingdoms/Uldaman/boss_archaedas.cpp

  EasternKingdoms/BaradinHold/baradin_hold.h
  EasternKingdoms/BaradinHold/instance_baradin_hold.cpp
  EasternKingdoms/BaradinHold/boss_argaloth.cpp

  EasternKingdoms/BlackrockCaverns/blackrock_caverns.h
  EasternKingdoms/BlackrockCaverns/boss_ascendant_lord_obsidius.cpp
  EasternKingdoms/BlackrockCaverns/boss_beauty.cpp
  EasternKingdoms/BlackrockCaverns/boss_corla_herald_of_twilight.cpp
  EasternKingdoms/BlackrockCaverns/boss_karsh_steelbender.cpp
  EasternKingdoms/BlackrockCaverns/boss_romogg_bonecrusher.cpp
  EasternKingdoms/BlackrockCaverns/instance_blackrock_caverns.cpp

  EasternKingdoms/BlackrockSpire/boss_pyroguard_emberseer.cpp
  EasternKingdoms/BlackrockSpire/boss_drakkisath.cpp
  EasternKingdoms/BlackrockSpire/boss_warmaster_voone.cpp
  EasternKingdoms/BlackrockSpire/boss_mother_smolderweb.cpp
  EasternKingdoms/BlackrockSpire/boss_quartermaster_zigris.cpp
  EasternKingdoms/BlackrockSpire/boss_halycon.cpp
  EasternKingdoms/BlackrockSpire/boss_overlord_wyrmthalak.cpp
  EasternKingdoms/BlackrockSpire/boss_shadow_hunter_voshgajin.cpp
  EasternKingdoms/BlackrockSpire/boss_gyth.cpp
  EasternKingdoms/BlackrockSpire/boss_rend_blackhand.cpp
  EasternKingdoms/BlackrockSpire/boss_highlord_omokk.cpp
  EasternKingdoms/BlackrockSpire/boss_the_beast.cpp
  EasternKingdoms/BlackrockSpire/blackrock_spire.h
  EasternKingdoms/BlackrockSpire/instance_blackrock_spire.cpp

  EasternKingdoms/SunwellPlateau/boss_eredar_twins.cpp
  EasternKingdoms/SunwellPlateau/boss_kiljaeden.cpp
  EasternKingdoms/SunwellPlateau/sunwell_plateau.h
  EasternKingdoms/SunwellPlateau/boss_muru.cpp
  EasternKingdoms/SunwellPlateau/instance_sunwell_plateau.cpp
  EasternKingdoms/SunwellPlateau/boss_kalecgos.cpp
  EasternKingdoms/SunwellPlateau/boss_brutallus.cpp
  EasternKingdoms/SunwellPlateau/sunwell_plateau.cpp
  EasternKingdoms/SunwellPlateau/boss_felmyst.cpp

  EasternKingdoms/TheStonecore/instance_the_stonecore.cpp
  EasternKingdoms/TheStonecore/the_stonecore.cpp
  EasternKingdoms/TheStonecore/boss_corborus.cpp
  EasternKingdoms/TheStonecore/boss_slabhide.cpp
  EasternKingdoms/TheStonecore/boss_ozruk.cpp
  EasternKingdoms/TheStonecore/boss_priestess_azil.cpp
  EasternKingdoms/TheStonecore/the_stonecore.h
 
  EasternKingdoms/Deadmines/deadmines.h
  EasternKingdoms/Deadmines/deadmines.cpp
  EasternKingdoms/Deadmines/boss_admiral_ripsnarl.cpp
  EasternKingdoms/Deadmines/boss_captain_cookie.cpp
  EasternKingdoms/Deadmines/boss_foe_reaper_5000.cpp
  EasternKingdoms/Deadmines/boss_glubtok.cpp
  EasternKingdoms/Deadmines/boss_helix_gearbreaker.cpp
  EasternKingdoms/Deadmines/boss_vanessa_vancleef.cpp
  EasternKingdoms/Deadmines/instance_deadmines.cpp

  EasternKingdoms/ScarletMonastery/boss_azshir_the_sleepless.cpp
  EasternKingdoms/ScarletMonastery/boss_mograine_and_whitemane.cpp
  EasternKingdoms/ScarletMonastery/boss_bloodmage_thalnos.cpp
  EasternKingdoms/ScarletMonastery/boss_interrogator_vishas.cpp
  EasternKingdoms/ScarletMonastery/boss_headless_horseman.cpp
  EasternKingdoms/ScarletMonastery/instance_scarlet_monastery.cpp
  EasternKingdoms/ScarletMonastery/boss_houndmaster_loksey.cpp
  EasternKingdoms/ScarletMonastery/scarlet_monastery.h
  EasternKingdoms/ScarletMonastery/boss_high_inquisitor_fairbanks.cpp
  EasternKingdoms/ScarletMonastery/boss_arcanist_doan.cpp
  EasternKingdoms/ScarletMonastery/boss_herod.cpp
  EasternKingdoms/ScarletMonastery/boss_scorn.cpp

  EasternKingdoms/ShadowfangKeep/boss_baron_ashbury.cpp
  EasternKingdoms/ShadowfangKeep/boss_baron_silverlaine.cpp
  EasternKingdoms/ShadowfangKeep/boss_commander_springvale.cpp
  EasternKingdoms/ShadowfangKeep/boss_lord_godfrey.cpp
  EasternKingdoms/ShadowfangKeep/boss_lord_walden.cpp
  EasternKingdoms/ShadowfangKeep/shadowfang_keep.cpp
  EasternKingdoms/ShadowfangKeep/instance_shadowfang_keep.cpp
  EasternKingdoms/ShadowfangKeep/shadowfang_keep.h

  EasternKingdoms/BlackwingDescent/instance_blackwing_descent.cpp
  EasternKingdoms/BlackwingDescent/boss_omnotron_defense_system.cpp
  EasternKingdoms/BlackwingDescent/boss_nefarian.cpp
  EasternKingdoms/BlackwingDescent/boss_maloriak.cpp
  EasternKingdoms/BlackwingDescent/boss_magmaw.cpp
  EasternKingdoms/BlackwingDescent/boss_chimaeron.cpp
  EasternKingdoms/BlackwingDescent/boss_atramedes.cpp
  EasternKingdoms/BlackwingDescent/blackwing_descent.h

  EasternKingdoms/BlackwingLair/boss_chromaggus.cpp
  EasternKingdoms/BlackwingLair/boss_razorgore.cpp
  EasternKingdoms/BlackwingLair/boss_firemaw.cpp
  EasternKingdoms/BlackwingLair/boss_broodlord_lashlayer.cpp
  EasternKingdoms/BlackwingLair/boss_ebonroc.cpp
  EasternKingdoms/BlackwingLair/instance_blackwing_lair.cpp
  EasternKingdoms/BlackwingLair/blackwing_lair.h
  EasternKingdoms/BlackwingLair/boss_vaelastrasz.cpp
  EasternKingdoms/BlackwingLair/boss_nefarian.cpp
  EasternKingdoms/BlackwingLair/boss_flamegor.cpp
  EasternKingdoms/BlackwingLair/boss_victor_nefarius.cpp

  EasternKingdoms/Karazhan/instance_karazhan.cpp
  EasternKingdoms/Karazhan/boss_nightbane.cpp
  EasternKingdoms/Karazhan/karazhan.cpp
  EasternKingdoms/Karazhan/boss_curator.cpp
  EasternKingdoms/Karazhan/boss_shade_of_aran.cpp
  EasternKingdoms/Karazhan/boss_netherspite.cpp
  EasternKingdoms/Karazhan/boss_maiden_of_virtue.cpp
  EasternKingdoms/Karazhan/boss_midnight.cpp
  EasternKingdoms/Karazhan/boss_prince_malchezaar.cpp
  EasternKingdoms/Karazhan/bosses_opera.cpp
  EasternKingdoms/Karazhan/boss_moroes.cpp
  EasternKingdoms/Karazhan/boss_terestian_illhoof.cpp
  EasternKingdoms/Karazhan/karazhan.h

  EasternKingdoms/ZulAman/boss_halazzi.cpp
  EasternKingdoms/ZulAman/boss_hexlord.cpp
  EasternKingdoms/ZulAman/boss_zuljin.cpp
  EasternKingdoms/ZulAman/boss_akilzon.cpp
  EasternKingdoms/ZulAman/instance_zulaman.cpp
  EasternKingdoms/ZulAman/boss_janalai.cpp
  EasternKingdoms/ZulAman/boss_nalorakk.cpp
  EasternKingdoms/ZulAman/zulaman.cpp
  EasternKingdoms/ZulAman/zulaman.h

  EasternKingdoms/Zones/ghostlands.cpp
  EasternKingdoms/Zones/eversong_woods.cpp
  EasternKingdoms/Zones/isle_of_queldanas.cpp
  EasternKingdoms/Zones/boss_kruul.cpp
  EasternKingdoms/Zones/searing_gorge.cpp
  EasternKingdoms/Zones/wetlands.cpp
  EasternKingdoms/Zones/arathi_highlands.cpp
  EasternKingdoms/Zones/redridge_mountains.cpp
  EasternKingdoms/Zones/ironforge.cpp
  EasternKingdoms/Zones/eastern_plaguelands.cpp
  EasternKingdoms/Zones/duskwood.cpp
  EasternKingdoms/Zones/undercity.cpp
  EasternKingdoms/Zones/silvermoon_city.cpp
  EasternKingdoms/Zones/loch_modan.cpp
  EasternKingdoms/Zones/burning_steppes.cpp
  EasternKingdoms/Zones/blasted_lands.cpp
  EasternKingdoms/Zones/stormwind_city.cpp
  EasternKingdoms/Zones/hinterlands.cpp
  EasternKingdoms/Zones/western_plaguelands.cpp
  EasternKingdoms/Zones/alterac_mountains.cpp
  EasternKingdoms/Zones/westfall.cpp
  EasternKingdoms/Zones/silverpine_forest.cpp
  EasternKingdoms/Zones/stranglethorn_vale.cpp
  EasternKingdoms/Zones/swamp_of_sorrows.cpp
  EasternKingdoms/Zones/elwynnforest.cpp
  EasternKingdoms/Zones/dun_morogh.cpp 
  EasternKingdoms/Zones/tirisfal_glades.cpp
  EasternKingdoms/Zones/vashjir.cpp
)

message("  -> Prepared: Eastern Kingdoms")
