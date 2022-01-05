#pragma once

#include "common.hpp"
#include "natives.hpp"
#include "gta\blip.hpp"

#define arraysize(_ARR)			 ((int)(sizeof(_ARR) / sizeof(*(_ARR))))
#define ARRAYSIZE_(_ARR)		 ((int)(sizeof(_ARR) / sizeof(*(_ARR))))

namespace big
{
	static class diamondheistinfo
	{
	public:
		static inline int approach{}, target{}, last_approach{}, hard_approach{};
		static inline int disrupt{}, keylevels{}, crewweap{}, crewdriver{}, crewhacker{}, vehs{}, weaps{}, masks{}, bodyarmorlvl{}, bitset{}, bitsetone{};
	} diamond_heist_info;

	static class constant_global
	{
	public:
		static class crew_gunman
		{
		public:
			//Gunman
			static constexpr inline uint32_t karl_cut = 262145 + 28465; //5
			static constexpr inline uint32_t gustavo_cut = 262145 + 28466; //9
			static constexpr inline uint32_t charlie_cut = 262145 + 28467; //7
			static constexpr inline uint32_t chester_cut = 262145 + 28468; //10
			static constexpr inline uint32_t packie_cut = 262145 + 28469; //8
		} ch_gunman;
		static class crew_driver
		{
		public:
			//Driver
			static constexpr inline uint32_t karim_cut = 262145 + 28470; //5
			static constexpr inline uint32_t taliana_cut = 262145 + 28471; //7
			static constexpr inline uint32_t eddie_cut = 262145 + 28472; //9
			static constexpr inline uint32_t zach_cut = 262145 + 28473; //6
			static constexpr inline uint32_t chester_cut = 262145 + 28474; //10
		}ch_driver;
		static class crew_hacker
		{
		public:
			//Hacker
			static constexpr inline uint32_t rickie_cut = 262145 + 28475; //3
			static constexpr inline uint32_t christian_cut = 262145 + 28476; //7
			static constexpr inline uint32_t yohan_cut = 262145 + 28477; //5
			static constexpr inline uint32_t avi_cut = 262145 + 28478; //10
			static constexpr inline uint32_t paige_cut = 262145 + 28479; //9
		}ch_hacker;

	public:
		static constexpr inline uint32_t special_cargo_price_1 = 262145 + 15508; //10000; // 1
		static constexpr inline uint32_t special_cargo_price_2 = 262145 + 15509; // = 11000; // 2
		static constexpr inline uint32_t special_cargo_price_3 = 262145 + 15510; // = 12000; // 3
		static constexpr inline uint32_t special_cargo_price_4 = 262145 + 15511; // = 13000; // 4-5
		static constexpr inline uint32_t special_cargo_price_5 = 262145 + 15512; // = 13500; // 6-7
		static constexpr inline uint32_t special_cargo_price_6 = 262145 + 15513; // = 14000; // 8-9
		static constexpr inline uint32_t special_cargo_price_7 = 262145 + 15514; // = 14500; // 10-14
		static constexpr inline uint32_t special_cargo_price_8 = 262145 + 15515; // = 15000; // 15-19
		static constexpr inline uint32_t special_cargo_price_9 = 262145 + 15516; // = 16000; // 25-29
		static constexpr inline uint32_t special_cargo_price_10 = 262145 + 15517; // = 16500; // 30-34
		static constexpr inline uint32_t special_cargo_price_11 = 262145 + 15518; // = 17000; // 35-39
		static constexpr inline uint32_t special_cargo_price_12 = 262145 + 15519; // = 17500; // 40-44
		static constexpr inline uint32_t special_cargo_price_13 = 262145 + 15520; // = 17750; // 45-49
		static constexpr inline uint32_t special_cargo_price_14 = 262145 + 15521; // = 18000; // 50-59
		static constexpr inline uint32_t special_cargo_price_15 = 262145 + 15522; // = 18250; // 60-69
		static constexpr inline uint32_t special_cargo_price_16 = 262145 + 15523; // = 18500; // 70-79
		static constexpr inline uint32_t special_cargo_price_17 = 262145 + 15524; // = 18750; // 80-89
		static constexpr inline uint32_t special_cargo_price_18 = 262145 + 15525; // = 19000; // 90-99
		static constexpr inline uint32_t special_cargo_price_19 = 262145 + 15526; // = 19500; // 100-110
		static constexpr inline uint32_t special_cargo_price_20 = 262145 + 15527; // = 20000; // 111

		static constexpr inline uint32_t level_array = 294328;
		static constexpr inline uint32_t special_crates = 1946637;
		static constexpr inline uint32_t rare_item = 1946791;
		static constexpr inline uint32_t rc_bandito = 2810287 + 6713;
		static constexpr inline uint32_t mini_tank = 2810287 + 6714;
		static constexpr inline uint32_t drone = 1958920;
		static constexpr inline uint32_t moc = 2810287 + 913;
		static constexpr inline uint32_t avanger = 2810287 + 921;
		static constexpr inline uint32_t terorbyte = 2810287 + 925;
		static constexpr inline uint32_t kosatka = 2810287 + 933;
		static constexpr inline uint32_t dinghy = 2810287 + 945;
		static constexpr inline uint32_t player_exp = 1655444 + 185;

		//Heist Global
		static constexpr inline uint32_t lester_cut = 262145 + 28289;
		static constexpr inline uint32_t real_player_cut = 2715542 + 6546;

		static constexpr inline uint32_t casino_cut_1 = 1966718 + 1497 + 736 + 92 + 1;
		static constexpr inline uint32_t casino_cut_2 = 1966718 + 1497 + 736 + 92 + 2;
		static constexpr inline uint32_t casino_cut_3 = 1966718 + 1497 + 736 + 92 + 3;
		static constexpr inline uint32_t casino_cut_4 = 1966718 + 1497 + 736 + 92 + 4;

		static constexpr inline uint32_t doomsday_cut_1 = 1962755 + 812 + 50 + 1;
		static constexpr inline uint32_t doomsday_cut_2 = 1962755 + 812 + 50 + 2;
		static constexpr inline uint32_t doomsday_cut_3 = 1962755 + 812 + 50 + 3;
		static constexpr inline uint32_t doomsday_cut_4 = 1962755 + 812 + 50 + 4;

		static constexpr inline uint32_t apartmen_cut_1 = 1934631 + 3008 + 1;
		static constexpr inline uint32_t apartmen_cut_2 = 1934631 + 3008 + 2;
		static constexpr inline uint32_t apartmen_cut_3 = 1934631 + 3008 + 3;
		static constexpr inline uint32_t apartmen_cut_4 = 1934631 + 3008 + 4;

		static constexpr inline uint32_t cayo_cut_1 = 1973496 + 823 + 56 + 1;
		static constexpr inline uint32_t cayo_cut_2 = 1973496 + 823 + 56 + 2;
		static constexpr inline uint32_t cayo_cut_3 = 1973496 + 823 + 56 + 3;
		static constexpr inline uint32_t cayo_cut_4 = 1973496 + 823 + 56 + 4;

		static constexpr inline uint32_t diamond = 262145 + 28456;
		static constexpr inline uint32_t gold = 262145 + 28455;
		static constexpr inline uint32_t artwork = 262145 + 28454;
		static constexpr inline uint32_t cash = 262145 + 28453;

		static constexpr inline uint32_t casino_take_indicator = 2453903 + 6251;

		static constexpr inline uint32_t tequila = 262145 + 29616;
		static constexpr inline uint32_t ruby = 262145 + 29617;
		static constexpr inline uint32_t bearer_bond = 262145 + 29618;
		static constexpr inline uint32_t pink_diamond = 262145 + 29619;
		static constexpr inline uint32_t madrazo_file = 262145 + 29620;
		static constexpr inline uint32_t saphire = 262145 + 29621;
		static constexpr inline uint32_t bag_size = 262146 + 29622;

		static constexpr inline uint32_t fleeca = 262145 + 8920;
		static constexpr inline uint32_t prison_break = 262145 + 8921;
		static constexpr inline uint32_t human_lab = 262145 + 8922;
		static constexpr inline uint32_t a_series = 262145 + 8923;
		static constexpr inline uint32_t pasific_standard = 262145 + 8924;

		static constexpr inline uint32_t act_1 = 262145 + 8925;
		static constexpr inline uint32_t act_2 = 262145 + 8926;
		static constexpr inline uint32_t act_3 = 262145 + 8927;

		static constexpr inline uint32_t casino_stat = 1971081;

		static constexpr inline uint32_t session_change = 1574587;
		static constexpr inline uint32_t session_type = 1575005;
		static constexpr inline uint32_t is_session_private = 2714627 + 744;//2453009.f_744

		static constexpr inline uint32_t mc_supplies_delay = 262145 + 18748;
		static constexpr inline uint32_t bunker_supplies_delay = 262145 + 21348;
		static constexpr inline uint32_t bunker_selling_time_1 = 262145 + 21486; //308825481
		static constexpr inline uint32_t bunker_selling_time_2 = 262145 + 21490; //1832002632
		static constexpr inline uint32_t bunker_selling_time_3 = 262145 + 21480; //-338923123
		static constexpr inline uint32_t bunker_selling_time_4 = 262145 + 21488; //2645121
		static constexpr inline uint32_t bunker_selling_time_5 = 262145 + 21493; //-1312217355
		static constexpr inline uint32_t bunker_selling_time_6 = 262145 + 21482; //245293410
		static constexpr inline uint32_t bunker_selling_mult_far = 262145 + 21303; //1021567941
		static constexpr inline uint32_t bunker_selling_mult_near = 262145 + 21302; //1021567941

		static constexpr inline uint32_t mc_selling_time_1 = 262145 + 18319; //-986183860
		static constexpr inline uint32_t mc_selling_time_2 = 262145 + 18316; //-605226498
		static constexpr inline uint32_t mc_selling_time_3 = 262145 + 18306; //1084174861
		static constexpr inline uint32_t mc_selling_time_4 = 262145 + 18310; //643059010
		static constexpr inline uint32_t mc_selling_time_5 = 262145 + 18312; //-2012578927
		static constexpr inline uint32_t mc_selling_time_6 = 262145 + 18346; //2037477066
		static constexpr inline uint32_t mc_selling_time_7 = 262145 + 18343; //-152621008
		static constexpr inline uint32_t mc_selling_time_8 = 262145 + 18353; //-186413482
		static constexpr inline uint32_t mc_selling_time_9 = 262145 + 18357; //286174536
		static constexpr inline uint32_t mc_selling_time_10 = 262145 + 18348; //-2110637751

		static constexpr inline uint32_t mc_sell_mult_far = 262145 + 18861;//1763638426
		static constexpr inline uint32_t mc_sell_mult_near = 262145 + 18862;//1763638426
		static constexpr inline uint32_t mc_document = 262145 + 17222;
		static constexpr inline uint32_t mc_cash = 262145 + 17223;
		static constexpr inline uint32_t mc_coke = 262145 + 17224;
		static constexpr inline uint32_t mc_meth = 262145 + 17225;
		static constexpr inline uint32_t mc_weed = 262145 + 17226;

		static constexpr inline uint32_t special_cargo_selling_cooldown = 262145 + 15361;//1291620941
		static constexpr inline uint32_t special_cargo_buying_cooldown = 262145 + 15360;//153204142
		static constexpr inline uint32_t special_cargo_selling_time = 262145 + 15374;//389090185
		static inline uint32_t business_index{};

		static constexpr inline uint32_t nc_trigger_product = 2783654;

		static constexpr inline uint32_t sctv = 2676084;
		static constexpr inline uint32_t session_id = 1575004;
		static constexpr inline uint32_t session_unk_1 = 31506;
		static constexpr inline uint32_t session_unk_2 = 1574626;
		static constexpr inline uint32_t session_unk_3 = 1574999;
		static inline uint32_t session_unk_4{};
		static inline uint32_t vision{};
		static constexpr inline uint32_t host_kick = 1622591 + 1;
		static const inline int32_t repair_vehicle = 2426865 + 1 + (g_local.player * 449) + 231;
		static constexpr inline uint32_t disconnect = 31782;

		//Player Stats
		static constexpr inline int32_t player_stat = 1853128;
		static constexpr inline int32_t player_size = 888;
		static constexpr inline int32_t player_offset = 205;

		static constexpr inline int32_t radar_toggle = 2689156;
		static constexpr inline int32_t radar_size = 453;
		static constexpr inline int32_t radar_time = 2703656;
		static constexpr inline int32_t radar_offset = 209;

		static constexpr inline int32_t blind_cops = 2810287;
		static constexpr inline int32_t blind_cops_offset = 4623;

		static constexpr inline int32_t bullshark_time = radar_time;
		static constexpr inline int32_t suicide_time = radar_time;

		static constexpr inline int32_t garage = 1585844;
		static constexpr inline int32_t call_personal_vehicle = 2810287;
		static constexpr inline int32_t show_take = 4718592 + 87192;
	} g_global;

	static class constant_local
	{
	public:
		static class blackjack_local
		{
		public:
			static inline uint32_t current_table{};
			static constexpr inline uint32_t current_card = 128 + 1;
			static inline uint32_t bet_trigger{};
			static constexpr inline uint32_t button_ready = 2045 + 874 + 693;
			static constexpr inline uint32_t unk_1 = 2045;
			static constexpr inline uint32_t unk_2 = 3632;
			static constexpr inline uint32_t unk_3 = 120;//122
		}blackjack;
		static class casino_slot_local
		{
		public:
			static inline uint32_t current_slot{};
			static constexpr inline int32_t jackpot_trigger = 1360;
			static constexpr inline int32_t animation = 1652;
			static constexpr inline int32_t slot_autoplay = 1650;
			static constexpr inline int32_t entry_func = 3413;
			static constexpr inline uint32_t struct_21 = 225;
			static constexpr inline uint32_t unk_1 = 2137;
			static constexpr inline uint32_t unk_2 = 2140;
		}slot;
		static class casino_lucky_wheel
		{
		public:
			static constexpr inline uint32_t podium_vehicle = 274 + 14;
			static constexpr inline uint32_t trigger_wheel = 274 + 45;
		}lucky_wheel;
	public:
		static constexpr inline uint32_t take_casino = 19422 + 2685;//19406
		static constexpr inline uint32_t take_cayo = 39782 + 1392 + 53;//39493 + 1536 + 53 //31600 + 9482 (41082) 39493.f_1589
		static constexpr inline uint32_t take_cayo_2 = 41112;
		static constexpr inline uint32_t vault_door = 9846 + 7;//9842
		static constexpr inline uint32_t vault_door_total = 9846 + 37;
		static constexpr inline uint32_t tc_live_c = 42837 + 865 + 1;//42692 + 1009 + 1;
		static constexpr inline uint32_t tc_live = 25847 + 1322 + 1;
		static constexpr inline uint32_t finger_clone = 23181;
		static constexpr inline uint32_t finger_print = 52667; //52649 DLC_H3_Fingerprint_Hack_Scene
		static constexpr inline uint32_t door_hack = 53729; //53711 DLC_H3_Door_Hack_Scene
		static constexpr inline uint32_t board_hacking_x = 11485 + 26;//11471
		static constexpr inline uint32_t board_hacking_y = 11485 + 27;
		static constexpr inline uint32_t board_hacking_requirement = 11485 + 39;
		static constexpr inline uint32_t voltage_hack_target = 1797;
		static constexpr inline uint32_t voltage_hack_current = 1798;
		static constexpr inline uint32_t glass_cutter_heat = 28273 + 4;
		static constexpr inline uint32_t underwater_torch_total = 28248;
		static constexpr inline uint32_t underwater_torch_sound = 28253;
		static constexpr inline int32_t safe_hack = 28255;
		static constexpr inline uint32_t set_vault_door_broken = 59295;//59294

		static constexpr inline uint32_t doomsday_act_3_hack = 1347 + 4;//1343
		static constexpr inline uint32_t submarine_computer = 1542;

		static constexpr inline uint32_t special_cargo_buying = 615;
		static constexpr inline uint32_t special_cargo_sell = 539;
		static constexpr inline uint32_t bunker_sell = 1220;
		static constexpr inline uint32_t bunker_sell_time_remaining = 1202;

		static constexpr inline uint32_t mc_sell = 713;

		static constexpr inline uint32_t nc_sell = 2321;
		static constexpr inline uint32_t nc_time_remaining = 762;
		static constexpr inline uint32_t host_kick = 6076;
	} m_local;

	static class script_event
	{
	public:
		static inline int clear_wanted{};
	}g_script_event;

}
//LA(11471 + 26,'fm_mission_controller')
//LA(11471 + 27,'fm_mission_controller')
//LA(11471 + 39,'fm_mission_controller')
/*
http://jobstreet.co.id/id/job/management-trainee-store-supervisor-penempatan-seluruh-indonesia-3722969?jobid=jobstreet-id-job-3722969&sectionRank=2&token=0~c283643e-11d6-4e4d-b154-f8c4bbc3477&fr=SRP+Job+Listing
*/