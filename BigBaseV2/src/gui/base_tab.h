#pragma once

#include "common.hpp"
#include "natives.hpp"

namespace big
{
	class main_tab
	{
	public:
		static void render_setting_tab();
		static void render_player_tab();
	private:
		static inline Date write_date;
		static inline char write_stat_name[128];
		static inline float write_float_value = 0;
		static inline float write_increment_value = 0;
		static inline int64_t write_integer_value = 0;
		static inline char write_string_value[128];
		static inline const uint64_t step_one = 100;
		static inline bool write_bool_value = true;
		static inline bool inputs_step = false;

		static inline char read_stat_name[128];
		static inline int64_t read_integer_value;
		static inline float read_float_value;
		static inline int read_bool_value;
		static inline char read_string_value[128];
		static inline Date read_date;

		static inline bool get_step = false;
		static inline char bool_to_text[20];
		static inline uint64_t Val;

		static inline int write_packed_ints_value = 0;
		static inline bool write_packed_bools_value = 0;
		static inline char write_packed_stat_name[50];

		static inline int read_packed_ints_value = 0;
		static inline bool read_packed_bools_value = 0;
		static inline char read_packed_stat_name[50];
		static inline int packed_stat_index;
		static inline int packed_stat_hash;
		static inline int packed_stat_value;
		static inline bool packed_bool_value;
		static inline char bool_string[20];

		static inline const char* const packed_bool_list[]{ "_NGPSTAT_BOOL", "_NGPSTAT_BOOL", "_NGTATPSTAT_BOOL", "_NGDLCPSTAT_BOOL", "_NGDLCPSTAT_BOOL" , "_DLCBIKEPSTAT_BOOL", "_DLCGUNPSTAT_BOOL", "_GUNTATPSTAT_BOOL", "_DLCSMUGCHARPSTAT_BOOL", "_GANGOPSPSTAT_BOOL" , "_BUSINESSBATPSTAT_BOOL" , "_ARENAWARSPSTAT_BOOL" , "_CASINOPSTAT_BOOL" , "_CASINOHSTPSTAT_BOOL" , "_HEIST3TATTOOSTAT_BOOL" , "_SU20PSTAT_BOOL" , "_SU20TATTOOSTAT_BOOL" , "_HISLANDPSTAT_BOOL", "_TUNERPSTAT_BOOL", "_FIXERPSTAT_BOOL", "_FIXERTATTOOSTAT_BOOL" };
		static inline int selected_packed_bools = 0;

		static inline int selected_unlocker = 0;
		static inline const char* const unlock_list[]{ "Basic Unlock", "Unlock All Bool Stat", "Unlock All Int Stat", "Unlock All Achievements", "Unlock Casino Shop Stuff", "Unhide Weapons from Gunlocker", "Tuner New Costumes" };
		static inline int LevelArray;
		static inline int level_type = 0;
	};
}