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
  Kalimdor/RazorfenDowns/razorfen_downs.cpp
  Kalimdor/RazorfenDowns/instance_razorfen_downs.cpp
  Kalimdor/RazorfenDowns/boss_amnennar_the_coldbringer.cpp
  Kalimdor/RazorfenDowns/razorfen_downs.h

  Kalimdor/ZulFarrak/zulfarrak.h
  Kalimdor/ZulFarrak/zulfarrak.cpp
  Kalimdor/ZulFarrak/instance_zulfarrak.cpp

  Kalimdor/CavernsOfTime/EscapeFromDurnholdeKeep/boss_epoch_hunter.cpp
  Kalimdor/CavernsOfTime/EscapeFromDurnholdeKeep/old_hillsbrad.h
  Kalimdor/CavernsOfTime/EscapeFromDurnholdeKeep/boss_leutenant_drake.cpp
  Kalimdor/CavernsOfTime/EscapeFromDurnholdeKeep/old_hillsbrad.cpp
  Kalimdor/CavernsOfTime/EscapeFromDurnholdeKeep/instance_old_hillsbrad.cpp
  Kalimdor/CavernsOfTime/EscapeFromDurnholdeKeep/boss_captain_skarloc.cpp

  Kalimdor/CavernsOfTime/BattleForMountHyjal/boss_archimonde.cpp
  Kalimdor/CavernsOfTime/BattleForMountHyjal/hyjalAI.h
  Kalimdor/CavernsOfTime/BattleForMountHyjal/hyjal_trash.h
  Kalimdor/CavernsOfTime/BattleForMountHyjal/boss_kazrogal.cpp
  Kalimdor/CavernsOfTime/BattleForMountHyjal/hyjal_trash.cpp
  Kalimdor/CavernsOfTime/BattleForMountHyjal/hyjal.cpp
  Kalimdor/CavernsOfTime/BattleForMountHyjal/hyjalAI.cpp
  Kalimdor/CavernsOfTime/BattleForMountHyjal/instance_hyjal.cpp
  Kalimdor/CavernsOfTime/BattleForMountHyjal/boss_rage_winterchill.cpp
  Kalimdor/CavernsOfTime/BattleForMountHyjal/hyjal.h
  Kalimdor/CavernsOfTime/BattleForMountHyjal/boss_azgalor.cpp
  Kalimdor/CavernsOfTime/BattleForMountHyjal/boss_anetheron.cpp

  Kalimdor/CavernsOfTime/CullingOfStratholme/boss_infinite.cpp
  Kalimdor/CavernsOfTime/CullingOfStratholme/boss_salramm.cpp
  Kalimdor/CavernsOfTime/CullingOfStratholme/boss_meathook.cpp
  Kalimdor/CavernsOfTime/CullingOfStratholme/boss_mal_ganis.cpp
  Kalimdor/CavernsOfTime/CullingOfStratholme/boss_epoch.cpp
  Kalimdor/CavernsOfTime/CullingOfStratholme/culling_of_stratholme.cpp
  Kalimdor/CavernsOfTime/CullingOfStratholme/instance_culling_of_stratholme.cpp
  Kalimdor/CavernsOfTime/CullingOfStratholme/culling_of_stratholme.h

  Kalimdor/CavernsOfTime/DarkPortal/dark_portal.h
  Kalimdor/CavernsOfTime/DarkPortal/instance_dark_portal.cpp
  Kalimdor/CavernsOfTime/DarkPortal/boss_chrono_lord_deja.cpp
  Kalimdor/CavernsOfTime/DarkPortal/dark_portal.cpp
  Kalimdor/CavernsOfTime/DarkPortal/boss_aeonus.cpp
  Kalimdor/CavernsOfTime/DarkPortal/boss_temporus.cpp

  #Kalimdor/DireMaul/npc_pusillin.cpp

  Kalimdor/BlackfathomDeeps/boss_kelris.cpp
  Kalimdor/BlackfathomDeeps/instance_blackfathom_deeps.cpp
  Kalimdor/BlackfathomDeeps/boss_gelihast.cpp
  Kalimdor/BlackfathomDeeps/blackfathom_deeps.cpp
  Kalimdor/BlackfathomDeeps/boss_aku_mai.cpp
  Kalimdor/BlackfathomDeeps/blackfathom_deeps.h

  Kalimdor/LostCityOfTheTolvir/instance_lost_city_of_the_tolvir.cpp
  Kalimdor/LostCityOfTheTolvir/lost_city_of_the_tolvir.cpp
  Kalimdor/LostCityOfTheTolvir/lost_city_of_the_tolvir.h
  Kalimdor/LostCityOfTheTolvir/boss_lockmaw.cpp
  Kalimdor/LostCityOfTheTolvir/boss_hussam.cpp
  Kalimdor/LostCityOfTheTolvir/boss_siamat.cpp
  Kalimdor/LostCityOfTheTolvir/boss_high_prophet_barim.cpp

  Kalimdor/HallsOfOrigination/instance_halls_of_origination.cpp
  Kalimdor/HallsOfOrigination/halls_of_origination.h
  Kalimdor/HallsOfOrigination/boss_temple_guardian_anhuur.cpp
  Kalimdor/HallsOfOrigination/boss_earthrager_ptah.cpp
  Kalimdor/HallsOfOrigination/boss_anraphet.cpp
  Kalimdor/HallsOfOrigination/boss_ammunae.cpp
  Kalimdor/HallsOfOrigination/boss_setesh.cpp
  Kalimdor/HallsOfOrigination/boss_rajh.cpp
  Kalimdor/HallsOfOrigination/boss_isiset.cpp

  Kalimdor/Maraudon/boss_princess_theradras.cpp
  Kalimdor/Maraudon/boss_landslide.cpp
  Kalimdor/Maraudon/boss_celebras_the_cursed.cpp
  Kalimdor/Maraudon/boss_noxxion.cpp

  Kalimdor/TempleOfAhnQiraj/boss_fankriss.cpp
  Kalimdor/TempleOfAhnQiraj/boss_huhuran.cpp
  Kalimdor/TempleOfAhnQiraj/instance_temple_of_ahnqiraj.cpp
  Kalimdor/TempleOfAhnQiraj/mob_anubisath_sentinel.cpp
  Kalimdor/TempleOfAhnQiraj/boss_viscidus.cpp
  Kalimdor/TempleOfAhnQiraj/boss_twinemperors.cpp
  Kalimdor/TempleOfAhnQiraj/boss_sartura.cpp
  Kalimdor/TempleOfAhnQiraj/boss_cthun.cpp
  Kalimdor/TempleOfAhnQiraj/temple_of_ahnqiraj.h
  Kalimdor/TempleOfAhnQiraj/boss_skeram.cpp
  Kalimdor/TempleOfAhnQiraj/boss_ouro.cpp
  Kalimdor/TempleOfAhnQiraj/boss_bug_trio.cpp

  Kalimdor/VortexPinnacle/boss_altairus.cpp
  Kalimdor/VortexPinnacle/boss_asaad.cpp
  Kalimdor/VortexPinnacle/boss_grand_vizier_ertan.cpp
  Kalimdor/VortexPinnacle/instance_vortex_pinnacle.cpp
  Kalimdor/VortexPinnacle/vortex_pinnacle.cpp
  Kalimdor/VortexPinnacle/vortex_pinnacle.h

  Kalimdor/RuinsOfAhnQiraj/boss_buru.cpp
  Kalimdor/RuinsOfAhnQiraj/instance_ruins_of_ahnqiraj.cpp
  Kalimdor/RuinsOfAhnQiraj/boss_rajaxx.cpp
  Kalimdor/RuinsOfAhnQiraj/boss_ossirian.cpp
  Kalimdor/RuinsOfAhnQiraj/boss_ayamiss.cpp
  Kalimdor/RuinsOfAhnQiraj/boss_moam.cpp
  Kalimdor/RuinsOfAhnQiraj/ruins_of_ahnqiraj.h
  Kalimdor/RuinsOfAhnQiraj/boss_kurinnaxx.cpp

  Kalimdor/WailingCaverns/wailing_caverns.h
  Kalimdor/WailingCaverns/instance_wailing_caverns.cpp
  Kalimdor/WailingCaverns/wailing_caverns.cpp

  Kalimdor/RazorfenKraul/razorfen_kraul.h
  Kalimdor/RazorfenKraul/instance_razorfen_kraul.cpp
  Kalimdor/RazorfenKraul/razorfen_kraul.cpp

  Kalimdor/OnyxiasLair/boss_onyxia.cpp
  Kalimdor/OnyxiasLair/onyxias_lair.h
  Kalimdor/OnyxiasLair/instance_onyxias_lair.cpp

  Kalimdor/Zones/thunder_bluff.cpp
  Kalimdor/Zones/azshara.cpp
  Kalimdor/Zones/orgrimmar.cpp
  Kalimdor/Zones/desolace.cpp
  Kalimdor/Zones/feralas.cpp
  Kalimdor/Zones/stonetalon_mountains.cpp
  Kalimdor/Zones/silithus.cpp
  Kalimdor/Zones/moonglade.cpp
  Kalimdor/Zones/bloodmyst_isle.cpp
  Kalimdor/Zones/azuremyst_isle.cpp
  Kalimdor/Zones/mulgore.cpp
  Kalimdor/Zones/darkshore.cpp
  Kalimdor/Zones/the_barrens.cpp
  Kalimdor/Zones/ungoro_crater.cpp
  Kalimdor/Zones/durotar.cpp
  Kalimdor/Zones/felwood.cpp
  Kalimdor/Zones/boss_azuregos.cpp
  Kalimdor/Zones/tanaris.cpp
  Kalimdor/Zones/dustwallow_marsh.cpp
  Kalimdor/Zones/winterspring.cpp
  Kalimdor/Zones/thousand_needles.cpp
  Kalimdor/Zones/ashenvale.cpp
  Kalimdor/Zones/teldrassil.cpp
)

message("  -> Prepared: Kalimdor")
