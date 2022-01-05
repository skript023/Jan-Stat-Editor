#include "common.hpp"
#include "features.hpp"
#include "logger.hpp"
#include "natives.hpp"
#include "fiber_pool.hpp"
#include "script.hpp"
#include "script_global.hpp"
#include "gta_util.hpp"
#include "script_local.hpp"
#include "hooking.hpp"
#include "gui.hpp"
#include "gui\base_tab.h"
#include "gui\ScriptControl.h"


    // auto vehicle = *(std::uintptr_t*)(local_ped + 0xD28);
    // auto offset2 = *(std::uintptr_t*)(vehicle + 0x8A8);
    // auto& offset3 = *(float*)(offset2 + 0x4C);
    // offset3 = 12.f;

namespace big::features
{	
	void run_tick()
	{
		if (g_local.player != PLAYER::PLAYER_ID() || g_local.ped != PLAYER::PLAYER_PED_ID())
		{
			g_local.player = PLAYER::PLAYER_ID();
			g_local.ped = PLAYER::PLAYER_PED_ID();
		}
		g_local.character = gta_util::get_character();
	}

	void script_func()
	{
		while (true)
		{
			TRY_CLAUSE
			{
				run_tick();
			}
			EXCEPT_CLAUSE
			script::get_current()->yield();
		}
	}
}
