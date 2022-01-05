#include "common.hpp"
#include "diamondheistinfo.h"
#include "imgui.h"
#include "script.hpp"
#include "fiber_pool.hpp"
#include "natives.hpp"
#include "gta_util.hpp"

namespace big


void online_tab::render_diamond_heist_hacks()
    	{
    		if (ImGui::CollapsingHeader("Diamond Heist Hacks"))
    		{
    			const char* const approach_list[] = { "Unselected", "Silent & Sneaky", "The Big Con", "Aggressive" };
    			ImGui::PushItemWidth(160);
    			if (ImGui::Combo("Select Approach", &g_local_player.diamond_heist_info.approach, approach_list, IM_ARRAYSIZE(approach_list)))
    				set_stat(g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3OPT_APPROACH") : RAGE_JOAAT("MP1_H3OPT_APPROACH"), g_local_player.diamond_heist_info.approach);
    			const char* const target_list[] = { "Money", "Gold", "Art", "Diamonds" };
    			ImGui::PopItemWidth();
    			ImGui::SameLine();
    			if (ImGui::Combo("Select Target", &g_local_player.diamond_heist_info.target, target_list, IM_ARRAYSIZE(target_list)))
    				set_stat(g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3OPT_TARGET") : RAGE_JOAAT("MP1_H3OPT_TARGET"), g_local_player.diamond_heist_info.target); bool vault_contents = (g_local_player.diamond_heist_info.bitsetone & CasinoHeistBitsetOne::VaultContents);
    			ImGui::SameLine();
    			if (ImGui::Checkbox("Vault Contents", &vault_contents))
    				set_bitset_one(CasinoHeistBitsetOne::VaultContents);
     
    			ImGui::PushItemWidth(160);
    			if (ImGui::Combo("Last Heist", &g_local_player.diamond_heist_info.last_approach, approach_list, IM_ARRAYSIZE(approach_list)))
    				set_stat(g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3_LAST_APPROACH") : RAGE_JOAAT("MP1_H3_LAST_APPROACH"), g_local_player.diamond_heist_info.last_approach);
    			ImGui::SameLine();
    			if (ImGui::Combo("Hard Approach", &g_local_player.diamond_heist_info.hard_approach, approach_list, IM_ARRAYSIZE(approach_list)))
    				set_stat(g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3_HARD_APPROACH") : RAGE_JOAAT("MP1_H3_HARD_APPROACH"), g_local_player.diamond_heist_info.hard_approach);
    			ImGui::PopItemWidth();
     
    			ImGui::Separator();
     
    			const char* const gunman_list[] = { "Unselected", "Karl Abolaji", "Gustavo Mota", "Charlie Reed", "Chester McCoy", "Patrick McReary" };
    			ImGui::PushItemWidth(175);
    			if (ImGui::Combo("Gunman", &g_local_player.diamond_heist_info.crewweap, gunman_list, IM_ARRAYSIZE(gunman_list)))
    				set_stat(g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3OPT_CREWWEAP") : RAGE_JOAAT("MP1_H3OPT_CREWWEAP"), g_local_player.diamond_heist_info.crewweap);
    			ImGui::SameLine();
    			const char* const driver_list[] = { "Unselected", "Karim Deniz", "Taliana Martinez", "Eddie Toh", "Zach Nelson", "Chester McCoy" };
    			if (ImGui::Combo("Driver", &g_local_player.diamond_heist_info.crewdriver, driver_list, IM_ARRAYSIZE(driver_list)))
    				set_stat(g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3OPT_CREWDRIVER") : RAGE_JOAAT("MP1_H3OPT_CREWDRIVER"), g_local_player.diamond_heist_info.crewdriver);
    			ImGui::SameLine();
    			const char* const hacker_list[] = { "Unselected", "Rickie Lukens", "Christian Feltz", "Yohan Blair", "Avi Schwartzman", "Page Harris" };
    			if (ImGui::Combo("Hacker", &g_local_player.diamond_heist_info.crewhacker, hacker_list, IM_ARRAYSIZE(hacker_list)))
    				set_stat(g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3OPT_CREWHACKER") : RAGE_JOAAT("MP1_H3OPT_CREWHACKER"), g_local_player.diamond_heist_info.crewhacker);
    			ImGui::PopItemWidth();
    			switch (g_local_player.diamond_heist_info.crewweap)
    			{
    				case 1: //Karl Abolaji
    				{
    					const char* const karl_gun_list[4][2] = { {"##1", "##2"}, { "Micro SMG Loadout", "Machine Pistol Loadout" }, { "Micro SMG Loadout", "Shotgun Loadout" }, { "Shotgun Loadout", "Revolver Loadout" } };
    					ImGui::PushItemWidth(230);
    					if (ImGui::Combo("Unmarked Weapons##2", &g_local_player.diamond_heist_info.weaps, karl_gun_list[g_local_player.diamond_heist_info.approach], 2))
    						set_stat(g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3OPT_WEAPS") : RAGE_JOAAT("MP1_H3OPT_WEAPS"), g_local_player.diamond_heist_info.weaps);
    					ImGui::SameLine();
    					ImGui::PopItemWidth();
    					break;
    				}
    				case 2: //Gustavo Mota
    				{
    					const char* const gus_gun_list[] = { "Rifle Loadout", "Shotgun Loadout" };
    					ImGui::PushItemWidth(190);
    					if (ImGui::Combo("Unmarked Weapons##2", &g_local_player.diamond_heist_info.weaps, gus_gun_list, IM_ARRAYSIZE(gus_gun_list)))
    						set_stat(g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3OPT_WEAPS") : RAGE_JOAAT("MP1_H3OPT_WEAPS"), g_local_player.diamond_heist_info.weaps);
    					ImGui::SameLine();
    					ImGui::PopItemWidth();
    					break;
    				}
    				case 3: //Charlie Reed
    				{
    					const char* const charlie_gun_list[4][2] = { {"##1", "##2"}, { "SMG Loadout", "Shotgun Loadout" }, { "Machine Pistol Loadout", "Shotgun Loadout" }, { "SMG Loadout", "Shotgun Loadout" } };
    					ImGui::PushItemWidth(230);
    					if (ImGui::Combo("Unmarked Weapons##2", &g_local_player.diamond_heist_info.weaps, charlie_gun_list[g_local_player.diamond_heist_info.approach], 2))
    						set_stat(g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3OPT_WEAPS") : RAGE_JOAAT("MP1_H3OPT_WEAPS"), g_local_player.diamond_heist_info.weaps);
    					ImGui::SameLine();
    					ImGui::PopItemWidth();
    					break;
    				}
    				case 4: //Chester McCoy
    				{
    					const char* const chester_gun_list[4][2] = { {"##1", "##2"}, { "MK II Shotgun Loadout", "MK II Rifle Loadout" }, { "MK II SMG Loadout", "MK II Rifle Loadout" }, { "MK II Shotgun Loadout", "MK II Rifle Loadout" } };
    					ImGui::PushItemWidth(230);
    					if (ImGui::Combo("Unmarked Weapons##2", &g_local_player.diamond_heist_info.weaps, chester_gun_list[g_local_player.diamond_heist_info.approach], 2))
    						set_stat(g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3OPT_WEAPS") : RAGE_JOAAT("MP1_H3OPT_WEAPS"), g_local_player.diamond_heist_info.weaps);
    					ImGui::SameLine();
    					ImGui::PopItemWidth();
    					break;
    				}
    				case 5: //Patrick McReary
    				{
    					const char* const packie_gun_list[4][2] = { {"##1", "##2"}, { "Combat PDW Loadout", "Rifle Loadout" }, { "Shotgun Loadout", "Rifle Loadout" }, { "Shotgun Loadout", "Combat MG Loadout" } };
    					ImGui::PushItemWidth(230);
    					if (ImGui::Combo("Unmarked Weapons##2", &g_local_player.diamond_heist_info.weaps, packie_gun_list[g_local_player.diamond_heist_info.approach], 2))
    						set_stat(g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3OPT_WEAPS") : RAGE_JOAAT("MP1_H3OPT_WEAPS"), g_local_player.diamond_heist_info.weaps);
    					ImGui::SameLine();
    					ImGui::PopItemWidth();
    				}
    			}
     
    			switch (g_local_player.diamond_heist_info.crewdriver)
    			{
    				case 1: //Karim Deniz
    				{
    					const char* const karim_vehicle_list[] = { "Issi Classic", "Asbo", "Kanjo", "Sentinel Classic" };
    					ImGui::PushItemWidth(170);
    					if (ImGui::Combo("Getaway Vehicles##2", &g_local_player.diamond_heist_info.vehs, karim_vehicle_list, IM_ARRAYSIZE(karim_vehicle_list)))
    						set_stat(g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3OPT_VEHS") : RAGE_JOAAT("MP1_H3OPT_VEHS"), g_local_player.diamond_heist_info.vehs);
    					ImGui::PopItemWidth();
    					break;
    				}
    				case 2: //Taliana Martinez
    				{
    					const char* const taliana_vehicle_list[] = { "Retinue MK II", "Drift Yosemite", "Sugoi", "Jugular" };
    					if (ImGui::Combo("Getaway Vehicles##2", &g_local_player.diamond_heist_info.vehs, taliana_vehicle_list, IM_ARRAYSIZE(taliana_vehicle_list)))
    						set_stat(g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3OPT_VEHS") : RAGE_JOAAT("MP1_H3OPT_VEHS"), g_local_player.diamond_heist_info.vehs);
    					break;
    				}
    				case 3: //Eddie Toh
    				{
    					const char* const eddie_vehicle_list[] = { "Sultan Classic", "Guantlet Classic", "Ellie", "Komoda" };
    					ImGui::PushItemWidth(170);
    					if (ImGui::Combo("Getaway Vehicles##2", &g_local_player.diamond_heist_info.vehs, eddie_vehicle_list, IM_ARRAYSIZE(eddie_vehicle_list)))
    						set_stat(g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3OPT_VEHS") : RAGE_JOAAT("MP1_H3OPT_VEHS"), g_local_player.diamond_heist_info.vehs);
    					ImGui::PopItemWidth();
    					break;
    				}
    				case 4: //Zach Nelson
    				{
    					const char* const zach_vehicle_list[] = { "Manchez", "Stryder", "Defiler", "Lectro" };
    					if (ImGui::Combo("Getaway Vehicles##2", &g_local_player.diamond_heist_info.vehs, zach_vehicle_list, IM_ARRAYSIZE(zach_vehicle_list)))
    						set_stat(g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3OPT_VEHS") : RAGE_JOAAT("MP1_H3OPT_VEHS"), g_local_player.diamond_heist_info.vehs);
    					break;
    				}
    				case 5: //Chester McCoy
    				{
    					const char* const chester_vehicle_list[] = { "Zhaba", "Vagrant", "Outlaw", "Everon" };
    					if (ImGui::Combo("Getaway Vehicles##2", &g_local_player.diamond_heist_info.vehs, chester_vehicle_list, IM_ARRAYSIZE(chester_vehicle_list)))
    						set_stat(g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3OPT_VEHS") : RAGE_JOAAT("MP1_H3OPT_VEHS"), g_local_player.diamond_heist_info.vehs);
    					break;
    				}
    			}
     
    			bool weapons_mission = (g_local_player.diamond_heist_info.bitsetone & CasinoHeistBitsetOne::WeaponsMissionFinished);
    			if (ImGui::Checkbox("Unmarked Weapons##1", &weapons_mission))
    				set_bitset_one(CasinoHeistBitsetOne::WeaponsMissionFinished);
    			ImGui::SameLine();
    			bool patrol_routes = (g_local_player.diamond_heist_info.bitset & CasinoHeistBitsetZero::PatrolRoutes);
    			if (ImGui::Checkbox("Patrol Routes", &patrol_routes))
    				set_bitset(CasinoHeistBitsetZero::PatrolRoutes);
     
    			bool vehicle_mission = (g_local_player.diamond_heist_info.bitsetone & CasinoHeistBitsetOne::VehicleMissionFinished);
    			if (ImGui::Checkbox("Getaway Vehicles##1", &vehicle_mission))
    				set_bitset_one(CasinoHeistBitsetOne::VehicleMissionFinished);
    			ImGui::SameLine();
    			bool duggan_shipments = (g_local_player.diamond_heist_info.bitset & CasinoHeistBitsetZero::DugganShipments);
    			if (ImGui::Checkbox("Duggan Shipments", &duggan_shipments))
    				set_bitset(CasinoHeistBitsetZero::DugganShipments);
     
    			bool hacking_device = (g_local_player.diamond_heist_info.bitsetone & CasinoHeistBitsetOne::HackingDevice);
    			if (ImGui::Checkbox("Hacking Device", &hacking_device))
    				set_bitset_one(CasinoHeistBitsetOne::HackingDevice);
     
    			bool vault_keys = (g_local_player.diamond_heist_info.bitsetone & CasinoHeistBitsetOne::VaultKeys);
    			if (ImGui::Checkbox("Vault Keycards", &vault_keys))
    				set_bitset_one(CasinoHeistBitsetOne::VaultKeys);
    			ImGui::SameLine();
    			bool power_drills = (g_local_player.diamond_heist_info.bitset & CasinoHeistBitsetZero::PowerDrills);
    			if (ImGui::Checkbox("Power Drills", &power_drills))
    				set_bitset(CasinoHeistBitsetZero::PowerDrills);
     
    			if (g_local_player.diamond_heist_info.approach == 1)
    			{
    				bool nano_drone = (g_local_player.diamond_heist_info.bitsetone & CasinoHeistBitsetOne::NanoDrone);
    				if (ImGui::Checkbox("Nano Drone", &nano_drone))
    					set_bitset_one(CasinoHeistBitsetOne::NanoDrone);
    				ImGui::SameLine();
    				bool steal_emp = (g_local_player.diamond_heist_info.bitset & CasinoHeistBitsetZero::EMP);
    				if (ImGui::Checkbox("Steal EMP", &steal_emp))
    					set_bitset(CasinoHeistBitsetZero::EMP);
     
    				bool vault_laser = (g_local_player.diamond_heist_info.bitsetone & CasinoHeistBitsetOne::VaultLaser);
    				if (ImGui::Checkbox("Vault Laser", &vault_laser))
    					set_bitset_one(CasinoHeistBitsetOne::VaultLaser);
    				ImGui::SameLine();
    				bool infiltration_suits = (g_local_player.diamond_heist_info.bitset & CasinoHeistBitsetZero::InfiltrationSuits);
    				if (ImGui::Checkbox("Infiltration Suits", &infiltration_suits))
    					set_bitset(CasinoHeistBitsetZero::InfiltrationSuits);
    			}
    			else if (g_local_player.diamond_heist_info.approach == 2)
    			{
    				bool bugstars_part_one = (g_local_player.diamond_heist_info.bitset & CasinoHeistBitsetZero::BugstarsPartOne);
    				if (ImGui::Checkbox("Bugstars I", &bugstars_part_one))
    					set_bitset(CasinoHeistBitsetZero::BugstarsPartOne);
    				ImGui::SameLine();
    				bool bugstars_part_two = (g_local_player.diamond_heist_info.bitset & CasinoHeistBitsetZero::BugstarsPartTwo);
    				if (ImGui::Checkbox("II##bug", &bugstars_part_two))
    					set_bitset(CasinoHeistBitsetZero::BugstarsPartTwo);
    				ImGui::SameLine();
    				bool maintenance_part_one = (g_local_player.diamond_heist_info.bitset & CasinoHeistBitsetZero::MaintenancePartOne);
    				if (ImGui::Checkbox("Maintenance I", &maintenance_part_one))
    					set_bitset(CasinoHeistBitsetZero::MaintenancePartOne);
    				ImGui::SameLine();
    				bool maintenance_part_two = (g_local_player.diamond_heist_info.bitset & CasinoHeistBitsetZero::MaintenancePartTwo);
    				if (ImGui::Checkbox("II##maint", &maintenance_part_two))
    					set_bitset(CasinoHeistBitsetZero::MaintenancePartTwo);
     
    				bool gruppe_sechs_part_one = (g_local_player.diamond_heist_info.bitset & CasinoHeistBitsetZero::GruppeSechsPartOne);
    				if (ImGui::Checkbox("Gruppe Sechs I", &gruppe_sechs_part_one))
    					set_bitset(CasinoHeistBitsetZero::GruppeSechsPartOne);
    				ImGui::SameLine();
    				bool gruppe_sechs_part_two = (g_local_player.diamond_heist_info.bitset & CasinoHeistBitsetZero::GruppeSechsPartTwo);
    				if (ImGui::Checkbox("II##gruppe", &gruppe_sechs_part_two))
    					set_bitset(CasinoHeistBitsetZero::GruppeSechsPartTwo);
    				ImGui::SameLine();
    				bool yung_ancestor_part_one = (g_local_player.diamond_heist_info.bitset & CasinoHeistBitsetZero::YungAncestorPartOne);
    				if (ImGui::Checkbox("Yung Ancestor I", &yung_ancestor_part_one))
    					set_bitset(CasinoHeistBitsetZero::YungAncestorPartOne);
    				ImGui::SameLine();
    				bool yung_ancestor_part_two = (g_local_player.diamond_heist_info.bitset & CasinoHeistBitsetZero::YungAncestorPartTwo);
    				if (ImGui::Checkbox("II##yung", &yung_ancestor_part_two))
    					set_bitset(CasinoHeistBitsetZero::YungAncestorPartTwo);
     
    				bool vault_drills = (g_local_player.diamond_heist_info.bitsetone & CasinoHeistBitsetOne::VaultDrill);
    				if (ImGui::Checkbox("Vault Drills", &vault_drills))
    					set_bitset_one(CasinoHeistBitsetOne::VaultDrill);
    				ImGui::SameLine();
    				bool noose_exit = (g_local_player.diamond_heist_info.bitset & CasinoHeistBitsetZero::NOOSEGearExitDisguise);
    				if (ImGui::Checkbox("NOOSE Exit", &noose_exit))
    					set_bitset(CasinoHeistBitsetZero::NOOSEGearExitDisguise);
    				ImGui::SameLine();
    				bool firefighter_exit = (g_local_player.diamond_heist_info.bitset & CasinoHeistBitsetZero::FirefighterGearExitDisguise);
    				if (ImGui::Checkbox("Firefighters Exit", &firefighter_exit))
    					set_bitset(CasinoHeistBitsetZero::FirefighterGearExitDisguise);
    				ImGui::SameLine();
    				bool highroller_exit = (g_local_player.diamond_heist_info.bitset & CasinoHeistBitsetZero::HighRollerExitDisguise);
    				if (ImGui::Checkbox("High Rollers Exit", &highroller_exit))
    					set_bitset(CasinoHeistBitsetZero::HighRollerExitDisguise);
    			}
    			else if (g_local_player.diamond_heist_info.approach == 3)
    			{
    				bool thermal_charges = (g_local_player.diamond_heist_info.bitsetone & CasinoHeistBitsetOne::ThermalCharges);
    				if (ImGui::Checkbox("Thermal Charges", &thermal_charges))
    					set_bitset_one(CasinoHeistBitsetOne::ThermalCharges);
    				ImGui::SameLine();
    				bool reinforced_armor = (g_local_player.diamond_heist_info.bitset & CasinoHeistBitsetZero::ReinforcedArmor);
    				if (ImGui::Checkbox("Reinforced Armor", &reinforced_armor))
    					set_bitset(CasinoHeistBitsetZero::ReinforcedArmor);
    				ImGui::SameLine();
    				const char* const armor_levels[] = { "Unselected", "One", "Two", "Three" };
    				if (ImGui::Combo("Armor Level", &g_local_player.diamond_heist_info.bodyarmorlvl, armor_levels, IM_ARRAYSIZE(armor_levels)))
    					set_stat(g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3OPT_BODYARMORLVL") : RAGE_JOAAT("MP1_H3OPT_BODYARMORLVL"), g_local_player.diamond_heist_info.bodyarmorlvl);
     
    				bool vault_explosives = (g_local_player.diamond_heist_info.bitsetone & CasinoHeistBitsetOne::VaultExplosives);
    				if (ImGui::Checkbox("Vault Explosives", &vault_explosives))
    					set_bitset_one(CasinoHeistBitsetOne::VaultExplosives);
    				ImGui::SameLine();
    				bool boring_machine = (g_local_player.diamond_heist_info.bitset & CasinoHeistBitsetZero::BoringMachine);
    				if (ImGui::Checkbox("Boring Machine", &boring_machine))
    					set_bitset(CasinoHeistBitsetZero::BoringMachine);
    			}
     
    			const char* const mask_list[] = { "Unselected", "Geometric Set", "Hunter Set", "Oni Half Mask Set", "Emoji Set", "Ornate Skull Set", "Lucky Fruit Set", "Gurilla Set", "Clown Set", "Animal Set", "Riot Set", "Oni Set", "Hockey Set" };
    			ImGui::PushItemWidth(185);
    			if (ImGui::Combo("Masks", &g_local_player.diamond_heist_info.masks, mask_list, IM_ARRAYSIZE(mask_list)))
    				set_stat(g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3OPT_MASKS") : RAGE_JOAAT("MP1_H3OPT_MASKS"), g_local_player.diamond_heist_info.masks);
    			ImGui::PopItemWidth();
    			ImGui::SameLine();
    			const char* const security_pass_list[] = { "Unselected", "Level 1", "Level 2" };
    			if (ImGui::Combo("Security Pass", &g_local_player.diamond_heist_info.keylevels, security_pass_list, IM_ARRAYSIZE(security_pass_list)))
    				set_stat(g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3OPT_KEYLEVELS") : RAGE_JOAAT("MP1_H3OPT_KEYLEVELS"), g_local_player.diamond_heist_info.keylevels);
    			ImGui::SameLine();
    			const char* const disrupt_levels[] = { "Unselected", "One", "Two", "Three" };
    			if (ImGui::Combo("Disrupt Level", &g_local_player.diamond_heist_info.disrupt, disrupt_levels, IM_ARRAYSIZE(disrupt_levels)))
    				set_stat(g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3OPT_DISRUPTSHIP") : RAGE_JOAAT("MP1_H3OPT_DISRUPTSHIP"), g_local_player.diamond_heist_info.disrupt);
     
    			bool clean_vehicle = (g_local_player.diamond_heist_info.bitset & CasinoHeistBitsetZero::CleanVehicle);
    			if (ImGui::Checkbox("Clean Vehicle", &clean_vehicle))
    				set_bitset(CasinoHeistBitsetZero::CleanVehicle);
    			ImGui::SameLine();
    			bool gunman_decoy = (g_local_player.diamond_heist_info.bitset & CasinoHeistBitsetZero::GunmanDecoy);
    			if (ImGui::Checkbox("Gunman Decoy", &gunman_decoy))
    				set_bitset(CasinoHeistBitsetZero::GunmanDecoy);
     
    			//if(ImGui::InputScalar("BITSET0", ImGuiDataType_U32, &g_local_player.diamond_heist_info.bitset, NULL, NULL, "%08X", ImGuiInputTextFlags_CharsHexadecimal))
    			//	set_stat(g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3OPT_BITSET0") : RAGE_JOAAT("MP1_H3OPT_BITSET0"), g_local_player.diamond_heist_info.bitset);
    			//if(ImGui::InputScalar("BITSET1", ImGuiDataType_U32, &g_local_player.diamond_heist_info.bitsetone, NULL, NULL, "%08X", ImGuiInputTextFlags_CharsHexadecimal))
    			//	set_stat(g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3OPT_BITSET1") : RAGE_JOAAT("MP1_H3OPT_BITSET0"), g_local_player.diamond_heist_info.bitsetone);
     
    			if (ImGui::Button("Unlock All Heist Options"))
    			{
    				QUEUE_JOB_BEGIN_CLAUSE()
    				{
    					STATS::STAT_SET_INT(g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3OPT_ACCESSPOINTS") : RAGE_JOAAT("MP1_H3OPT_ACCESSPOINTS"), -1, TRUE);
    					STATS::STAT_SET_INT(g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3OPT_POI") : RAGE_JOAAT("MP1_H3OPT_POI"), -1, TRUE);
    					g_local_player.diamond_heist_info.bitset = -1;
    					set_stat(g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3OPT_BITSET0") : RAGE_JOAAT("MP1_H3OPT_BITSET0"), g_local_player.diamond_heist_info.bitsetone);
    					g_local_player.diamond_heist_info.bitsetone = -1;
    					set_stat(g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3OPT_BITSET1") : RAGE_JOAAT("MP1_H3OPT_BITSET1"), g_local_player.diamond_heist_info.bitsetone);
    					g_local_player.diamond_heist_info.bodyarmorlvl = 3;
    					set_stat(g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3OPT_BODYARMORLVL") : RAGE_JOAAT("MP1_H3OPT_BODYARMORLVL"), g_local_player.diamond_heist_info.bodyarmorlvl);
    					g_local_player.diamond_heist_info.disrupt = 3;
    					set_stat(g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3OPT_DISRUPTSHIP") : RAGE_JOAAT("MP1_H3OPT_DISRUPTSHIP"), g_local_player.diamond_heist_info.disrupt);
    					g_local_player.diamond_heist_info.keylevels = 2;
    					set_stat(g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3OPT_KEYLEVELS") : RAGE_JOAAT("MP1_H3OPT_KEYLEVELS"), g_local_player.diamond_heist_info.keylevels);
    					STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_H3_COOLDOWN"), 0, TRUE);
    					STATS::STAT_SET_INT(g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3_COMPLETEDPOSIX") : RAGE_JOAAT("MP1_H3_COMPLETEDPOSIX"), 0, TRUE);
    					STATS::STAT_SET_INT(g_local_player.is_mp0_char ? -1836134507 : 123524422, -1, TRUE); //Unlock Packie and Avi.
    					STATS::STAT_SET_BOOL_MASKED(STATS::_GET_NGSTAT_BOOL_HASH((26969 - 26810), 0, 1, g_local_player.character_index, "_CASINOPSTAT_BOOL"),
    						TRUE, ((26969 - 26810) - STATS::_0xF4D8E7AC2A27758C((26969 - 26810)) * 64), 1); //Unlock High Roller.
    				} QUEUE_JOB_END_CLAUSE
    			}
    		}
    	}
    	
    	void online_tab::get_diamond_heist_stats()
    	{
    		QUEUE_JOB_BEGIN_CLAUSE()
    		{
    			STATS::STAT_GET_INT(g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3OPT_APPROACH") : RAGE_JOAAT("MP1_H3OPT_APPROACH"), &g_local_player.diamond_heist_info.approach, -1);
    			STATS::STAT_GET_INT(g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3OPT_TARGET") : RAGE_JOAAT("MP1_H3OPT_TARGET"), &g_local_player.diamond_heist_info.target, -1);
    			STATS::STAT_GET_INT(g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3OPT_DISRUPTSHIP") : RAGE_JOAAT("MP1_H3OPT_DISRUPTSHIP"), &g_local_player.diamond_heist_info.disrupt, -1);
    			STATS::STAT_GET_INT(g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3OPT_KEYLEVELS") : RAGE_JOAAT("MP1_H3OPT_KEYLEVELS"), &g_local_player.diamond_heist_info.keylevels, -1);
    			STATS::STAT_GET_INT(g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3OPT_CREWWEAP") : RAGE_JOAAT("MP1_H3OPT_CREWWEAP"), &g_local_player.diamond_heist_info.crewweap, -1);
    			STATS::STAT_GET_INT(g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3OPT_CREWDRIVER") : RAGE_JOAAT("MP1_H3OPT_CREWDRIVER"), &g_local_player.diamond_heist_info.crewdriver, -1);
    			STATS::STAT_GET_INT(g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3OPT_CREWHACKER") : RAGE_JOAAT("MP1_H3OPT_CREWHACKER"), &g_local_player.diamond_heist_info.crewhacker, -1);
    			STATS::STAT_GET_INT(g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3OPT_VEHS") : RAGE_JOAAT("MP1_H3OPT_VEHS"), &g_local_player.diamond_heist_info.vehs, -1);
    			STATS::STAT_GET_INT(g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3OPT_WEAPS") : RAGE_JOAAT("MP1_H3OPT_WEAPS"), &g_local_player.diamond_heist_info.weaps, -1);
    			STATS::STAT_GET_INT(g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3OPT_MASKS") : RAGE_JOAAT("MP1_H3OPT_MASKS"), &g_local_player.diamond_heist_info.masks, -1);
    			STATS::STAT_GET_INT(g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3OPT_BODYARMORLVL") : RAGE_JOAAT("MP1_H3OPT_BODYARMORLVL"), &g_local_player.diamond_heist_info.bodyarmorlvl, -1);
    			STATS::STAT_GET_INT(g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3OPT_BITSET0") : RAGE_JOAAT("MP1_H3OPT_BITSET0"), &g_local_player.diamond_heist_info.bitset, -1); //Optional Missions
    			STATS::STAT_GET_INT(g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3OPT_BITSET1") : RAGE_JOAAT("MP1_H3OPT_BITSET1"), &g_local_player.diamond_heist_info.bitsetone, -1); //Required Missions
    			STATS::STAT_GET_INT(g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3_LAST_APPROACH") : RAGE_JOAAT("MP1_H3_LAST_APPROACH"), &g_local_player.diamond_heist_info.last_approach, -1);
    			STATS::STAT_GET_INT(g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3_HARD_APPROACH") : RAGE_JOAAT("MP1_H3_HARD_APPROACH"), &g_local_player.diamond_heist_info.hard_approach, -1);
    		} QUEUE_JOB_END_CLAUSE
    	}
    	
    	void online_tab::set_stat(Hash stat, int value)
    	{
    		QUEUE_JOB_BEGIN_CLAUSE(stat, value)
    		{
    			STATS::STAT_SET_INT(stat, value, TRUE);
    		} QUEUE_JOB_END_CLAUSE
    	}
     
    	void online_tab::set_bitset(int value)
    	{
    		g_local_player.diamond_heist_info.bitset ^= value;
    		QUEUE_JOB_BEGIN_CLAUSE()
    		{
    			auto stat = g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3OPT_BITSET0") : RAGE_JOAAT("MP1_H3OPT_BITSET0");
    			STATS::STAT_SET_INT(stat, g_local_player.diamond_heist_info.bitset, TRUE);
    		} QUEUE_JOB_END_CLAUSE
    	}
     
    	void online_tab::set_bitset_one(int value)
    	{
    		g_local_player.diamond_heist_info.bitsetone ^= value;
    		QUEUE_JOB_BEGIN_CLAUSE()
    		{
    			auto stat = g_local_player.is_mp0_char ? RAGE_JOAAT("MP0_H3OPT_BITSET1") : RAGE_JOAAT("MP1_H3OPT_BITSET1");
    			STATS::STAT_SET_INT(stat, g_local_player.diamond_heist_info.bitsetone, TRUE);
    		} QUEUE_JOB_END_CLAUSE
    	}
    }