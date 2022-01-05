#include "common.hpp"
#include "base_tab.h"
#include "imgui.h"
#include "script.hpp"
#include "fiber_pool.hpp"
#include "natives.hpp"
#include "gta_util.hpp"
#include "ImGuiBitfield.h"
#include "script_global.hpp"
#include "features.hpp"
#include "gta\VehicleValues.h"
#include "gui.hpp"
#include "gta\Weapons.h"
#include "script_local.hpp"
#include "gui\ScriptControl.h"
#include "gui/xorstr.h"
#include "gui/global_index.hpp"

namespace big
{
    void main_tab::render_setting_tab()
    {
        if (ImGui::BeginTabItem(xorstr("Loader")))
        {
            if (ImGui::Button(xorstr("End Session")))
            {
                g_fiber_pool->queue_job([]
                    {
                        NETWORK::NETWORK_SESSION_END(FALSE, FALSE);
                    });
            }
            ImGui::SameLine();
            if (ImGui::Button(xorstr("End Cutscene")))
            {
                g_fiber_pool->queue_job([]
                    {
                        CUTSCENE::STOP_CUTSCENE_IMMEDIATELY();
                    });
            }
            ImGui::SameLine();
            if (ImGui::Button(xorstr("Bail from Session"))) {
                g_fiber_pool->queue_job([] {
                    NETWORK::NETWORK_BAIL(49, 0, 0);
                    controller::ShowMessage("Disconnected!", false);
                });
            }
            if (*g_pointers->m_script_globals != nullptr)
            {
                if (ImGui::CollapsingHeader(xorstr("Global Script Editor")))
                {
                    static global_test_json::global_test_json global_test{};
                    static script_global glo_bal_sunday = script_global(global_test.global_index);
                    ImGui::SetNextItemWidth(200.f);
                    if (ImGui::InputScalar("Global", ImGuiDataType_U64, &global_test.global_index))
                        glo_bal_sunday = script_global(global_test.global_index);

                    for (int i = 0; i < global_test.global_appendages.size(); i++)
                    {
                        auto item = global_test.global_appendages[i];
                        switch (item.type)
                        {
                        case GlobalAppendageType_At:
                            ImGui::SetNextItemWidth(200.f);
                            ImGui::InputScalar(fmt::format("At##{}{}", i, item.type).c_str(), ImGuiDataType_S64, &global_test.global_appendages[i].index);
                            ImGui::SameLine();
                            ImGui::SetNextItemWidth(200.f);
                            ImGui::InputScalar(fmt::format("Size##{}{}", i, item.type).c_str(), ImGuiDataType_S64, &global_test.global_appendages[i].size);
                            break;
                        case GlobalAppendageType_ReadGlobal:
                            ImGui::Text(fmt::format("Read Global {}", item.global_name).c_str());
                            ImGui::SameLine();
                            ImGui::SetNextItemWidth(200.f);
                            ImGui::InputScalar(fmt::format("Size##{}{}", i, item.type).c_str(), ImGuiDataType_S64, &global_test.global_appendages[i].size);
                            break;
                        case GlobalAppendageType_PlayerId:
                            ImGui::SetNextItemWidth(200.f);
                            ImGui::InputScalar(fmt::format("Read Player ID Size##{}{}", i, item.type).c_str(), ImGuiDataType_S64, &global_test.global_appendages[i].size);
                            break;
                        }
                    }

                    if (ImGui::Button("Add Offset"))
                        global_test.global_appendages.push_back({ GlobalAppendageType_At, 0LL, 0ULL });
                    ImGui::SameLine();
                    if (ImGui::Button("Add Read Player Id"))
                        global_test.global_appendages.push_back({ GlobalAppendageType_PlayerId, 0LL, 0ULL });

                    if (global_test.global_appendages.size() > 0 && ImGui::Button("Remove Offset"))
                        global_test.global_appendages.pop_back();
                    static int Global_Type = 0;
                    ImGui::RadioButton("Integer##Global", &Global_Type, 0);
                    ImGui::SameLine();
                    ImGui::RadioButton("Float##Global", &Global_Type, 1);
                    ImGui::SameLine();
                    ImGui::RadioButton("String##Global", &Global_Type, 2);

                    if (auto ptr = controller::get_global_ptr(global_test))
                    {
                        ImGui::SetNextItemWidth(200.f);
                        if (Global_Type != 2)
                            ImGui::InputScalar("Value", Global_Type == 0 ? ImGuiDataType_S64 : Global_Type == 1 ? ImGuiDataType_Float : ImGuiDataType_S64, ptr);

                    }
                    else
                        ImGui::Text("INVALID_GLOBAL_READ");

                    auto globals = list_globals();
                    static std::string selected_global;
                    ImGui::Text("Saved Globals");
                    if (ImGui::ListBoxHeader("##savedglobals", ImVec2(200, 200)))
                    {
                        for (auto pair : globals)
                        {
                            if (ImGui::Selectable(pair.first.c_str(), selected_global == pair.first))
                                selected_global = std::string(pair.first);
                        }
                        ImGui::ListBoxFooter();
                    }
                    ImGui::SameLine();
                    if (ImGui::ListBoxHeader("##globalvalues", ImVec2(200, 200)))
                    {
                        for (auto pair : globals)
                        {
                            if (auto ptr = controller::get_global_ptr(pair.second))
                                ImGui::Selectable(fmt::format("{}", *ptr).c_str(), false, ImGuiSelectableFlags_Disabled);
                            else
                                ImGui::Selectable("INVALID_GLOBAL_READ", false, ImGuiSelectableFlags_Disabled);
                        }
                        ImGui::ListBoxFooter();
                    }
                    ImGui::SameLine();
                    ImGui::BeginGroup();
                    static char global_name[50]{};
                    ImGui::SetNextItemWidth(200.f);
                    ImGui::InputText("##GlobalName", global_name, IM_ARRAYSIZE(global_name));

                    if (ImGui::Button("Save Global"))
                    {
                        controller::save_global(global_name, global_test);
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Load Global"))
                    {
                        controller::load_global_menu(selected_global, global_test);
                    }

                    if (ImGui::Button("Delete Global"))
                    {
                        if (!selected_global.empty())
                        {
                            controller::delete_global(selected_global);
                            selected_global.clear();
                        }
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Add Read Global"))
                    {
                        global_test.global_appendages.push_back({ GlobalAppendageType_ReadGlobal, 0LL, 0ULL, selected_global });
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Clear"))
                    {
                        global_test.global_index = 0;
                        global_test.global_appendages.clear();
                    }
                    ImGui::EndGroup();
                }
            }
            if (ImGui::CollapsingHeader(xorstr("Local Script Editor")))
            {
                static GtaThread* selected_thread{};
                if (ImGui::ListBoxHeader("##scriptslocal", ImVec2(250, 500)))
                {
                    if (g_settings.options["script monitor sorted"])
                    {
                        std::map<std::string, GtaThread*> sorted_threads;
                        for (auto thread : *g_pointers->m_script_threads)
                            if (thread && thread->m_context.m_thread_id && thread->m_handler)
                                sorted_threads[thread->m_name] = thread;
                        for (auto thread : sorted_threads)
                            if (ImGui::Selectable(thread.second->m_name, thread.second == selected_thread))
                                selected_thread = thread.second;
                    }
                    else
                    {
                        for (auto thread : *g_pointers->m_script_threads)
                            if (thread && thread->m_context.m_thread_id && thread->m_handler)
                                if (ImGui::Selectable(thread->m_name, thread == selected_thread))
                                    selected_thread = thread;
                    }
                    ImGui::ListBoxFooter();
                }
                if (selected_thread != nullptr)
                {
                    ImGui::SameLine();
                    ImGui::BeginGroup();
                    if (ImGui::Checkbox("Sort", g_settings.options["script monitor sorted"].get<bool*>()))
                        g_settings.save();
                    static local_test_json::local_test_json local_test{};
                    static script_local local_sunday = script_local(selected_thread, local_test.local_index);
                    ImGui::SetNextItemWidth(200.f);
                    if (ImGui::InputScalar("Local", ImGuiDataType_U64, &local_test.local_index))
                        local_sunday = script_local(selected_thread, local_test.local_index);

                    for (int i = 0; i < local_test.local_appendages.size(); i++)
                    {
                        auto item = local_test.local_appendages[i];
                        switch (item.type)
                        {
                        case LocalAppendageType_At:
                            ImGui::SetNextItemWidth(200.f);
                            ImGui::InputScalar(fmt::format("At##{}{}", i, item.type).c_str(), ImGuiDataType_S64, &local_test.local_appendages[i].index);
                            ImGui::SameLine();
                            ImGui::SetNextItemWidth(200.f);
                            ImGui::InputScalar(fmt::format("Size##{}{}", i, item.type).c_str(), ImGuiDataType_S64, &local_test.local_appendages[i].size);
                            break;
                        case LocalAppendageType_ReadGlobal:
                            ImGui::Text(fmt::format("Read Global {}", item.local_name).c_str());
                            ImGui::SameLine();
                            ImGui::SetNextItemWidth(200.f);
                            ImGui::InputScalar(fmt::format("Size##{}{}", i, item.type).c_str(), ImGuiDataType_S64, &local_test.local_appendages[i].size);
                            break;
                        case LocalAppendageType_PlayerId:
                            ImGui::SetNextItemWidth(200.f);
                            ImGui::InputScalar(fmt::format("Read Player ID Size##{}{}", i, item.type).c_str(), ImGuiDataType_S64, &local_test.local_appendages[i].size);
                            break;
                        }
                    }

                    if (ImGui::Button("Add Offset##Local"))
                        local_test.local_appendages.push_back({ LocalAppendageType_At, 0LL, 0ULL });
                    ImGui::SameLine();
                    if (ImGui::Button("Add Read Player Id##Local"))
                        local_test.local_appendages.push_back({ LocalAppendageType_PlayerId, 0LL, 0ULL });

                    if (local_test.local_appendages.size() > 0 && ImGui::Button("Remove Offset"))
                        local_test.local_appendages.pop_back();
                    static int Local_Type = 0;
                    ImGui::RadioButton("Integer##Local", &Local_Type, 0);
                    ImGui::SameLine();
                    ImGui::RadioButton("Float##Local", &Local_Type, 1);
                    ImGui::SameLine();
                    ImGui::RadioButton("String##Local", &Local_Type, 2);

                    if (auto ptr = controller::get_local_ptr(selected_thread, local_test))
                    {
                        ImGui::SetNextItemWidth(200.f);
                        if (Local_Type != 2)
                            ImGui::InputScalar("Value##Local", Local_Type == 0 ? ImGuiDataType_S64 : Local_Type == 1 ? ImGuiDataType_Float : ImGuiDataType_S64, ptr);

                    }
                    else
                        ImGui::Text("INVALID_LOCAL_READ");

                    auto locals = list_locals();
                    static std::string selected_local;
                    ImGui::Text("Saved Locals");
                    if (ImGui::ListBoxHeader("##savedlocals", ImVec2(200, 200)))
                    {
                        for (auto pair : locals)
                        {
                            if (ImGui::Selectable(pair.first.c_str(), selected_local == pair.first))
                                selected_local = std::string(pair.first);
                        }
                        ImGui::ListBoxFooter();
                    }
                    ImGui::SameLine();
                    if (ImGui::ListBoxHeader("##localvalues", ImVec2(200, 200)))
                    {
                        for (auto pair : locals)
                        {
                            if (auto ptr = controller::get_local_ptr(selected_thread, pair.second))
                                ImGui::Selectable(fmt::format("{}", *ptr).c_str(), false, ImGuiSelectableFlags_Disabled);
                            else
                                ImGui::Selectable("INVALID_LOCAL_READ", false, ImGuiSelectableFlags_Disabled);
                        }
                        ImGui::ListBoxFooter();
                    }
                    static char local_name[50]{};
                    ImGui::SetNextItemWidth(200.f);
                    ImGui::InputText("##LocalName", local_name, IM_ARRAYSIZE(local_name));

                    if (ImGui::Button("Save Local"))
                    {
                        controller::save_local(local_name, local_test);
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Load Local"))
                    {
                        controller::load_local_menu(selected_local, local_test);
                    }

                    if (ImGui::Button("Delete Local"))
                    {
                        if (!selected_local.empty())
                        {
                            controller::delete_local(selected_local);
                            selected_local.clear();
                        }
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Add Read Local"))
                    {
                        local_test.local_appendages.push_back({ LocalAppendageType_ReadGlobal, 0LL, 0ULL, selected_local });
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Clear Local"))
                    {
                        local_test.local_index = 0;
                        local_test.local_appendages.clear();
                    }
                    ImGui::EndGroup();
                }
            }
            if (ImGui::CollapsingHeader(xorstr("Stat Loader")))
            {
                static stats_json::stats_json stat_obj{};
                static std::string* StatString[250];
                static bool loaded = false;
                const int64_t step = 1;
                ImGui::PushItemWidth(150);
                if (ImGui::InputInt("Stats", &stat_obj.stats, 1, 80))
                {
                    if (stat_obj.stats == 0)
                        stat_obj.stats = 1;
                    if (stat_obj.stats > 250)
                        stat_obj.stats = 250;
                }
                ImGui::PopItemWidth();
                stat_obj.stat_type.resize(stat_obj.stats);
                stat_obj.stat_name.resize(stat_obj.stats);
                stat_obj.stat_int_value.resize(stat_obj.stats);
                stat_obj.stat_bool_value.resize(stat_obj.stats);
                stat_obj.stat_float_value.resize(stat_obj.stats);
                ImGui::Separator();
                for (int i = 0; i < stat_obj.stats; i++)
                {
                    ImGui::Text(fmt::format("Stat #{}", i + 1).c_str());
                    ImGui::RadioButton(fmt::format("INT##{}", i + 1).c_str(), &stat_obj.stat_type[i], 0); ImGui::SameLine();
                    ImGui::RadioButton(fmt::format("BOOL##{}", i + 1).c_str(), &stat_obj.stat_type[i], 1); ImGui::SameLine();
                    ImGui::RadioButton(fmt::format("FLOAT##{}", i + 1).c_str(), &stat_obj.stat_type[i], 2);
                    ImGui::PushItemWidth(450);
                    StatString[i] = &stat_obj.stat_name[i];

                    if (ImGui::InputText(fmt::format("##name{}", i + 1).c_str(), (char*)StatString[i]->c_str(), StatString[i]->capacity() * 2, ImGuiInputTextFlags_CharsUppercase))
                        stat_obj.stat_name[i] = (char*)StatString[i]->c_str();

                    ImGui::PopItemWidth();
                    if (stat_obj.stat_type[i] == 0)
                    {
                        ImGui::SameLine();
                        ImGui::PushItemWidth(250);
                        ImGui::InputScalar(fmt::format("##int{}", i + 1).c_str(), ImGuiDataType_S64, &stat_obj.stat_int_value[i]);
                        ImGui::PopItemWidth();
                    }
                    else if (stat_obj.stat_type[i] == 1)
                    {
                        ImGui::SameLine();
                        bool stat_bool_value_tmp = stat_obj.stat_bool_value[i];
                        if (ImGui::Checkbox(fmt::format("##bool{}", i + 1).c_str(), &stat_bool_value_tmp))
                            stat_obj.stat_bool_value[i] = stat_bool_value_tmp;
                    }
                    else if (stat_obj.stat_type[i] == 2)
                    {
                        ImGui::SameLine();
                        ImGui::PushItemWidth(250);
                        ImGui::InputScalar(fmt::format("##float{}", i + 1).c_str(), ImGuiDataType_Float, &stat_obj.stat_float_value[i], NULL, NULL);
                        ImGui::PopItemWidth();
                    }
                }
                if (ImGui::Button("Set Stats"))
                {
                    for (int i = 0; i < stat_obj.stats; i++)
                    {
                        const auto chara = std::to_string(g_local.character);
                        std::string found_set_stat = stat_obj.stat_name[i].length() >= 3 ? stat_obj.stat_name[i].substr(2, 1) : stat_obj.stat_name[i];
                        if (found_set_stat.compare("X") == 0)
                        {
                            stat_obj.stat_name[i].replace(2, 1, chara);
                        }
                        const auto hash = rage::joaat(stat_obj.stat_name[i]);
                        LOG(INFO) << "Hash String : " << stat_obj.stat_name[i] << " Hash Number : " << hash;
                        g_fiber_pool->queue_job([i, hash]
                            {
                                if (stat_obj.stat_type[i] == 0)
                                {
                                    STATS::STAT_SET_INT(hash, stat_obj.stat_int_value[i], true);
                                }
                                else if (stat_obj.stat_type[i] == 1)
                                {
                                    STATS::STAT_SET_BOOL(hash, stat_obj.stat_bool_value[i], true);
                                }
                                else if (stat_obj.stat_type[i] == 2)
                                {
                                    STATS::STAT_SET_FLOAT(hash, stat_obj.stat_float_value[i], true);
                                }
                            });
                    }
                }
                ImGui::Separator();
                auto stats = controller::list_stats();
                static std::string selected_stat;
                ImGui::PushItemWidth(250);
                ImGui::Text("Saved Stats");
                if (ImGui::ListBoxHeader("##empty", ImVec2(200, 200)))
                {
                    for (auto pair : stats)
                    {
                        if (ImGui::Selectable(pair.c_str(), selected_stat == pair))
                            selected_stat = pair;
                    }
                    ImGui::ListBoxFooter();
                }
                ImGui::PopItemWidth();
                ImGui::SameLine();
                ImGui::BeginGroup();
                static char save_stat_name[50]{};
                ImGui::PushItemWidth(250);
                ImGui::InputText("##save_stat_name", save_stat_name, IM_ARRAYSIZE(save_stat_name));
                if (ImGui::Button("Save Stat"))
                {
                    controller::save_stat(save_stat_name, stat_obj);
                }
                ImGui::SameLine();
                if (ImGui::Button("Load Stat"))
                {
                    controller::load_stat_menu(selected_stat, stat_obj);
                }
                ImGui::SameLine();
                if (ImGui::Button("Delete Stat"))
                {
                    if (!selected_stat.empty())
                    {
                        controller::delete_stat(selected_stat);
                        selected_stat.clear();
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("Clear"))
                {
                    stat_obj.stats = 1;
                    stat_obj.stat_type.clear();
                    stat_obj.stat_name.clear();
                    stat_obj.stat_int_value.clear();
                    stat_obj.stat_bool_value.clear();
                    stat_obj.stat_float_value.clear();
                }
                ImGui::PopItemWidth();
                ImGui::EndGroup();
            }
            ImGui::Separator();

            if (ImGui::Button(xorstr("Unload")))
            {
                g_running = false;
            }
            ImGui::EndTabItem();
        }
    }
    void main_tab::render_player_tab()
    {
        if (ImGui::BeginTabItem(xorstr("Stat Editor")))
        {
            static int e = 0;
            ImGui::Text(xorstr("Write Stat"));
            ImGui::RadioButton(xorstr("Integer"), &e, 0);
            ImGui::SameLine();
            ImGui::RadioButton(xorstr("Increment"), &e, 1);
            ImGui::SameLine();
            ImGui::RadioButton(xorstr("Bool"), &e, 2);
            ImGui::SameLine();
            ImGui::RadioButton(xorstr("Float"), &e, 3);
            ImGui::SameLine();
            ImGui::RadioButton(xorstr("Date"), &e, 4);
            ImGui::SameLine();
            ImGui::RadioButton(xorstr("String"), &e, 5);
            ImGui::Separator();

            ImGui::InputText(xorstr("##Stat Name"), write_stat_name, IM_ARRAYSIZE(write_stat_name), ImGuiInputTextFlags_CharsUppercase);

            switch (e)
            {
            case 0:
                ImGui::InputScalar(xorstr("##Integer"), ImGuiDataType_S64, &write_integer_value);
                break;

            case 1:
                ImGui::InputScalar(xorstr("##Increment"), ImGuiDataType_Float, &write_increment_value);
                break;

            case 2:
                ImGui::Checkbox(xorstr("##BoolValue"), &write_bool_value);
                break;

            case 3:
                ImGui::InputScalar(xorstr("##Float"), ImGuiDataType_Float, &write_float_value);
                break;
            case 4:
                ImGui::PushItemWidth(70);
                ImGui::InputScalar(xorstr("##Year"), ImGuiDataType_U32, &write_date.year);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip(xorstr("Year"));
                ImGui::SameLine();
                ImGui::InputScalar(xorstr("##month"), ImGuiDataType_U32, &write_date.month);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip(xorstr("Month"));
                ImGui::SameLine();
                ImGui::InputScalar(xorstr("##day"), ImGuiDataType_U32, &write_date.day);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip(xorstr("Day"));
                ImGui::PopItemWidth();
                break;
            case 5:
                ImGui::InputText(xorstr("##Strings"), write_string_value, IM_ARRAYSIZE(write_string_value));
                break;
            }

            if (ImGui::Button(xorstr("Write Stat")))
            {
                g_fiber_pool->queue_job([]
                    {
                        strcpy(write_stat_name, std::regex_replace(write_stat_name, std::regex(R"(\$)"), "").c_str());
                        strcpy(write_stat_name, std::regex_replace(write_stat_name, std::regex(R"(\MPX)"), "MP" + std::to_string(gta_util::get_character())).c_str());
                        strcpy(write_stat_name, std::regex_replace(write_stat_name, std::regex(R"(\MPx)"), "MP" + std::to_string(gta_util::get_character())).c_str());
                        const auto stat_hash = rage::joaat(write_stat_name);

                        switch (e)
                        {
                        case 0:
                            STATS::STAT_SET_INT(stat_hash, write_integer_value, TRUE);
                            break;

                        case 1:
                            STATS::STAT_INCREMENT(stat_hash, write_increment_value);
                            break;

                        case 2:
                            STATS::STAT_SET_BOOL(stat_hash, write_bool_value, TRUE);
                            break;

                        case 3:
                            STATS::STAT_SET_FLOAT(stat_hash, write_float_value, TRUE);
                            break;

                        case 4:
                            STATS::STAT_SET_DATE(stat_hash, &write_date, 7, TRUE);
                            break;
                        case 5:
                            STATS::STAT_SET_STRING(stat_hash, write_string_value, TRUE);
                            break;
                        }
                    });
            }
            ImGui::Separator();
            static int r = 0;
            ImGui::Text(xorstr("Read Stat"));
            ImGui::RadioButton(xorstr("Integer##Read"), &r, 0);
            ImGui::SameLine();
            ImGui::RadioButton(xorstr("Bool##Read"), &r, 1);
            ImGui::SameLine();
            ImGui::RadioButton(xorstr("Float##Read"), &r, 2);
            ImGui::SameLine();
            ImGui::RadioButton(xorstr("Date##Read"), &r, 3);
            ImGui::SameLine();
            ImGui::RadioButton(xorstr("String##Read"), &r, 4);
            ImGui::Separator();

            ImGui::InputText(xorstr("##ReadStat"), read_stat_name, IM_ARRAYSIZE(read_stat_name), ImGuiInputTextFlags_CharsUppercase);

            switch (r)
            {
            case 0:
                ImGui::InputScalar(xorstr("##Get Int"), ImGuiDataType_U64, &read_integer_value, get_step ? &step_one : NULL, NULL, "%u", ImGuiInputTextFlags_ReadOnly);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip(xorstr("Value of the stat"));
                break;

            case 1:
                ImGui::InputText(xorstr("##Get Bool"), bool_to_text, IM_ARRAYSIZE(bool_to_text), ImGuiInputTextFlags_ReadOnly);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip(xorstr("Value of the stat"));
                break;

            case 2:
                ImGui::InputScalar(xorstr("##Get Float"), ImGuiDataType_Float, &read_float_value, get_step ? &step_one : NULL, NULL, "%f", ImGuiInputTextFlags_ReadOnly);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip(xorstr("Value of the stat"));
                break;
            case 3:
                ImGui::Text("Year : %d Month : %d Day : %d", read_date.year, read_date.month, read_date.day);
                ImGui::PushItemWidth(70);
                ImGui::InputScalar(xorstr("##Get year"), ImGuiDataType_U32, &read_date.year, get_step ? &step_one : NULL, NULL, "%u", ImGuiInputTextFlags_ReadOnly);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip(xorstr("Year"));
                ImGui::SameLine();
                ImGui::InputScalar(xorstr("##Get month"), ImGuiDataType_U32, &read_date.month, get_step ? &step_one : NULL, NULL, "%u", ImGuiInputTextFlags_ReadOnly);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip(xorstr("Month"));
                ImGui::SameLine();
                ImGui::InputScalar(xorstr("##Get day"), ImGuiDataType_U32, &read_date.day, get_step ? &step_one : NULL, NULL, "%u", ImGuiInputTextFlags_ReadOnly);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip(xorstr("Day"));
                ImGui::PopItemWidth();
                break;
            case 4:
                ImGui::InputText(xorstr("##Get String"), read_string_value, IM_ARRAYSIZE(read_string_value));
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip(xorstr("Value of the stat"));
                break;
            }

            if (ImGui::Button(xorstr("Read Stat")))
            {
                g_fiber_pool->queue_job([]
                {
                    strcpy(read_stat_name, std::regex_replace(read_stat_name, std::regex(R"(\$)"), "").c_str());
                    strcpy(read_stat_name, std::regex_replace(read_stat_name, std::regex(R"(\MPX)"), "MP" + std::to_string(gta_util::get_character())).c_str());
                    strcpy(read_stat_name, std::regex_replace(read_stat_name, std::regex(R"(\MPx)"), "MP" + std::to_string(gta_util::get_character())).c_str());
                    const auto stat_hash = rage::joaat(read_stat_name);

                    switch (r)
                    {
                    case 0:
                        STATS::STAT_GET_INT(stat_hash, &read_integer_value, -1);
                        break;
                    case 1:
                        STATS::STAT_GET_BOOL(stat_hash, &read_bool_value, -1);
                        strcpy(bool_to_text, read_bool_value ? "TRUE" : "FALSE");
                        break;
                    case 2:
                        STATS::STAT_GET_FLOAT(stat_hash, &read_float_value, -1);
                        break;
                    case 3:
                        STATS::STAT_GET_DATE(stat_hash, &read_date, 7, -1);
                        break;
                    case 4:
                        strcpy(read_string_value, STATS::STAT_GET_STRING(stat_hash, -1));
                        break;
                    }
                });
            }
            ImGui::Separator();

            static int write_packed_type = 0;

            ImGui::Text(xorstr("Write Packed Stat"));
            ImGui::RadioButton(xorstr("Packed Ints##Write"), &write_packed_type, 0);
            ImGui::SameLine();
            ImGui::RadioButton(xorstr("Packed Bools##Write"), &write_packed_type, 1);

            ImGui::InputText(xorstr("##WritePackedStatName"), write_packed_stat_name, IM_ARRAYSIZE(write_packed_stat_name), ImGuiInputTextFlags_CharsUppercase);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip(xorstr("Must be a number of stat index"));

            switch (write_packed_type)
            {
            case 0:
                ImGui::InputScalar(xorstr("##Packed Ints Value"), ImGuiDataType_S32, &write_packed_ints_value);
                break;
            case 1:
                ImGui::Checkbox(xorstr("##Packed Bools Value"), &write_packed_bools_value);
                break;
            }

            if (ImGui::Button(xorstr("Write Packed Stat")))
            {
                g_fiber_pool->queue_job([]
                {
                    std::string packed_stat = systems::is_not_number(write_packed_stat_name) ? "0" : write_packed_stat_name;
                    int packed_index = stoi(packed_stat);
                    switch (write_packed_type)
                    {
                    case 0:
                        controller::set_packed_int(packed_index, write_packed_ints_value, -1);
                        break;
                    case 1:
                        controller::set_packed_bool(packed_index, write_packed_bools_value, -1);
                        break;
                    }
                });
            }

            ImGui::Separator();

            static int read_packed_type = 0;

            ImGui::Text(xorstr("Read Packed Stat"));
            ImGui::RadioButton(xorstr("Packed Ints##Read"), &read_packed_type, 0);
            ImGui::SameLine();
            ImGui::RadioButton(xorstr("Packed Bools##Read"), &read_packed_type, 1);

            ImGui::InputText(xorstr("##ReadPackedStatName"), read_packed_stat_name, IM_ARRAYSIZE(read_packed_stat_name), ImGuiInputTextFlags_CharsUppercase);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip(xorstr("Must be a number of stat index"));

            switch (read_packed_type)
            {
            case 0:
                ImGui::InputScalar(xorstr("##Get Packed Int"), ImGuiDataType_U64, &packed_stat_value, get_step ? &step_one : NULL, NULL, "%u", ImGuiInputTextFlags_ReadOnly);
                ImGui::Text("(Int32)Hash : %d | Name : %s | Index : %d | Value : %d", static_cast<uint32_t>(packed_stat_hash), controller::get_packed_stat_name_from_hash(static_cast<uint32_t>(packed_stat_hash)).c_str(), packed_stat_index, packed_stat_value);
                break;
            case 1:
                ImGui::InputText(xorstr("##Get Packed Bool"), bool_string, IM_ARRAYSIZE(bool_string), ImGuiInputTextFlags_ReadOnly);
                //ImGui::Text("(Int32)Hash : %d | Name : %s | Index : %d | Value : %d", static_cast<uint32_t>(packed_stat_hash), controller::get_packed_stat_name_from_hash(static_cast<uint32_t>(packed_stat_hash)).c_str(), packed_stat_index, packed_stat_value);
                break;

            }

            if (ImGui::Button(xorstr("Read Packed Stat")))
            {
                g_fiber_pool->queue_job([]
                    {
                        std::string to_number = systems::is_not_number(read_packed_stat_name) ? "0" : read_packed_stat_name;
                        int x = stoi(to_number);
                        switch (read_packed_type)
                        {
                        case 0:

                            packed_stat_value = controller::get_packed_int(x, -1, -1);
                            packed_stat_hash = controller::packed_stat_name(x, -1);
                            packed_stat_index = controller::packed_stat_mask(x);
                            break;
                        case 1:
                            packed_bool_value = controller::get_packed_bool(x, -1);
                            strcpy(bool_string, packed_bool_value ? "TRUE" : "FALSE");
                            break;
                        }
                    });
            }

            ImGui::Separator();

            ImGui::Text("Packed Bools");
            ImGui::Combo("##PackedStat", &selected_packed_bools, packed_bool_list, IM_ARRAYSIZE(packed_bool_list));
            if (ImGui::Button(xorstr("Set Packed Bools")))
            {
                g_fiber_pool->queue_job([]
                    {
                        int character = g_local.character;
                        switch (selected_packed_bools)
                        {
                        case 0:
                            for (int i = 4207; i <= 4335; i++)
                            {
                                controller::set_packed_bool(i, true, -1);
                            }
                            break;
                        case 1:
                            for (int i = 4335; i < 4399; i++)
                            {
                                controller::set_packed_bool(i, true, -1);
                            }
                            break;
                        case 2:
                            for (int i = 6029; i < 6413; i++)
                            {
                                controller::set_packed_bool(i, true, -1);
                            }
                            break;
                        case 3:
                            for (int i = 7385; i < 7641; i++)
                            {
                                controller::set_packed_bool(i, true, -1);
                            }
                            break;
                        case 4:
                            for (int i = 7321; i < 7385; i++)
                            {
                                controller::set_packed_bool(i, true, -1);
                            }
                            break;
                        case 5:
                            for (int i = 9361; i < 9553; i++)
                            {
                                controller::set_packed_bool(i, true, -1);
                            }
                            break;
                        case 6:
                            for (int i = 15369; i < 15561; i++)
                            {
                                controller::set_packed_bool(i, true, -1);
                            }
                            break;
                        case 7:
                            for (int i = 15562; i < 15946; i++)
                            {
                                controller::set_packed_bool(i, true, -1);
                            }
                            break;
                        case 8:
                            for (int i = 15946; i < 16010; i++)
                            {
                                controller::set_packed_bool(i, true, -1);
                            }
                            break;
                        case 9:
                            for (int i = 18098; i < 18162; i++)
                            {
                                controller::set_packed_bool(i, true, -1);
                            }
                            break;
                        case 10:
                            for (int i = 22066; i < 22194; i++)
                            {
                                controller::set_packed_bool(i, true, -1);
                            }
                            break;
                        case 11:
                            for (int i = 24962; i < 25538; i++)
                            {
                                controller::set_packed_bool(i, true, -1);
                            }
                            break;
                        case 12:
                            for (int i = 26810; i < 27258; i++)
                            {
                                controller::set_packed_bool(i, true, -1);
                            }
                            break;
                        case 13:
                            for (int i = 28098; i < 28354; i++)
                            {
                                controller::set_packed_bool(i, true, -1);
                            }
                            break;
                        case 14:
                            for (int i = 28355; i < 28483; i++)
                            {
                                controller::set_packed_bool(i, true, -1);
                            }
                            break;
                        case 15:
                            for (int i = 30227; i < 30355; i++)
                            {
                                controller::set_packed_bool(i, true, -1);
                            }
                            break;
                        case 16:
                            for (int i = 30355; i < 30483; i++)
                            {
                                controller::set_packed_bool(i, true, -1);
                            }
                            break;
                        case 17:
                            for (int i = 30515; i < 30707; i++)
                            {
                                controller::set_packed_bool(i, true, -1);
                            }
                            break;
                        case 18:
                            for (int i = 31707; i < 32283; i++)
                            {
                                controller::set_packed_bool(i, true, -1);
                            }
                            break;
                        case 19:
                            for (int i = 32283; i < 32411; i++)
                            {
                                controller::set_packed_bool(i, true, -1);
                            }
                        case 20:
                            for (int i = 32411; i < 32475; i++)
                            {
                                controller::set_packed_bool(i, true, -1);
                            }
                            break;
                        }
                    });
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip(xorstr("This option will set stat to true"));
            ImGui::SameLine();
            if (ImGui::Button(xorstr("Revert Packed Bools")))
            {
                g_fiber_pool->queue_job([]
                    {
                        int character = g_local.character;
                        switch (selected_packed_bools)
                        {
                        case 0:
                            for (int i = 4207; i < 4335; i++)
                            {
                                controller::set_packed_bool(i, false, -1);
                            }
                            break;
                        case 1:
                            for (int i = 4335; i < 4399; i++)
                            {
                                controller::set_packed_bool(i, false, -1);
                            }
                            break;
                        case 2:
                            for (int i = 6029; i < 6413; i++)
                            {
                                controller::set_packed_bool(i, false, -1);
                            }
                            break;
                        case 3:
                            for (int i = 7385; i < 7641; i++)
                            {
                                controller::set_packed_bool(i, false, -1);
                            }
                            break;
                        case 4:
                            for (int i = 7321; i < 7385; i++)
                            {
                                controller::set_packed_bool(i, false, -1);
                            }
                            break;
                        case 5:
                            for (int i = 9361; i < 9553; i++)
                            {
                                controller::set_packed_bool(i, false, -1);
                            }
                            break;
                        case 6:
                            for (int i = 15369; i < 15561; i++)
                            {
                                controller::set_packed_bool(i, false, -1);
                            }
                            break;
                        case 7:
                            for (int i = 15562; i < 15946; i++)
                            {
                                controller::set_packed_bool(i, false, -1);
                            }
                            break;
                        case 8:
                            for (int i = 15946; i < 16010; i++)
                            {
                                controller::set_packed_bool(i, false, -1);
                            }
                            break;
                        case 9:
                            for (int i = 18098; i < 18162; i++)
                            {
                                controller::set_packed_bool(i, false, -1);
                            }
                            break;
                        case 10:
                            for (int i = 22066; i < 22194; i++)
                            {
                                controller::set_packed_bool(i, false, -1);
                            }
                            break;
                        case 11:
                            for (int i = 24962; i < 25538; i++)
                            {
                                controller::set_packed_bool(i, false, -1);
                            }
                            break;
                        case 12:
                            for (int i = 26810; i < 27258; i++)
                            {
                                controller::set_packed_bool(i, false, -1);
                            }
                            break;
                        case 13:
                            for (int i = 28098; i < 28354; i++)
                            {
                                controller::set_packed_bool(i, false, -1);
                            }
                            break;
                        case 14:
                            for (int i = 28355; i < 28483; i++)
                            {
                                controller::set_packed_bool(i, false, -1);
                            }
                            break;
                        case 15:
                            for (int i = 30227; i < 30355; i++)
                            {
                                controller::set_packed_bool(i, false, -1);
                            }
                            break;
                        case 16:
                            for (int i = 30355; i < 30483; i++)
                            {
                                controller::set_packed_bool(i, false, -1);
                            }
                            break;
                        case 17:
                            for (int i = 30515; i < 30707; i++)
                            {
                                controller::set_packed_bool(i, false, -1);
                            }
                            break;
                        case 18:
                            for (int i = 31707; i < 32283; i++)
                            {
                                controller::set_packed_bool(i, false, -1);
                            }
                            break;
                        case 19:
                            for (int i = 32283; i < 32411; i++)
                            {
                                controller::set_packed_bool(i, false, -1);
                            }
                            break;
                        case 20:
                            for (int i = 32411; i < 32475; i++)
                            {
                                controller::set_packed_bool(i, false, -1);
                            }
                            break;
                        }
                    });
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip(xorstr("This option will set stat to false"));


            ImGui::Text(xorstr("Unlocker"));
            ImGui::Combo(xorstr("##Unlocker"), &selected_unlocker, unlock_list, IM_ARRAYSIZE(unlock_list));
            if (ImGui::Button(xorstr("Set Unlock")))
            {
                g_fiber_pool->queue_job([]
                {
                    switch (selected_unlocker)
                    {
                        case 0:
                        {
                            int character = gta_util::get_character();
                            for (int i = 4207; i <= 4335; i++)
                            {
                                controller::set_packed_bool(i, true, -1);
                            }

                            for (int i = 4335; i < 4399; i++)
                            {
                                controller::set_packed_bool(i, true, -1);
                            }

                            for (int i = 6029; i < 6413; i++)
                            {
                                controller::set_packed_bool(i, true, -1);
                            }

                            for (int i = 7385; i < 7641; i++)
                            {
                                controller::set_packed_bool(i, true, -1);
                            }

                            for (int i = 7321; i < 7385; i++)
                            {
                                controller::set_packed_bool(i, true, -1);
                            }

                            for (int i = 9361; i < 9553; i++)
                            {
                                controller::set_packed_bool(i, true, -1);
                            }

                            for (int i = 15369; i < 15561; i++)
                            {
                                controller::set_packed_bool(i, true, -1);
                            }

                            for (int i = 15562; i < 15946; i++)
                            {
                                controller::set_packed_bool(i, true, -1);
                            }

                            for (int i = 15946; i < 16010; i++)
                            {
                                controller::set_packed_bool(i, true, -1);
                            }

                            for (int i = 18098; i < 18162; i++)
                            {
                                controller::set_packed_bool(i, true, -1);
                            }

                            for (int i = 22066; i < 22194; i++)
                            {
                                controller::set_packed_bool(i, true, -1);
                            }

                            for (int i = 24962; i < 25538; i++)
                            {
                                controller::set_packed_bool(i, true, -1);
                            }

                            for (int i = 26810; i < 27258; i++)
                            {
                                controller::set_packed_bool(i, true, -1);
                            }

                            for (int i = 28098; i < 28354; i++)
                            {
                                controller::set_packed_bool(i, true, -1);
                            }

                            for (int i = 28355; i < 28483; i++)
                            {
                                controller::set_packed_bool(i, true, -1);
                            }

                            for (int i = 30227; i < 30355; i++)
                            {
                                controller::set_packed_bool(i, true, -1);
                            }

                            for (int i = 30355; i < 30483; i++)
                            {
                                controller::set_packed_bool(i, true, -1);
                            }

                            for (int i = 30515; i < 30707; i++)
                            {
                                controller::set_packed_bool(i, true, -1);
                            }

                            for (int i = 31707; i < 32283; i++)
                            {
                                controller::set_packed_bool(i, true, -1);
                            }

                            for (int i = 32283; i < 32411; i++)
                            {
                                controller::set_packed_bool(i, true, -1);
                            }

                            for (int i = 32411; i < 32475; i++)
                            {
                                controller::set_packed_bool(i, true, -1);
                            }
                            STATS::STAT_SET_INT(rage::joaat("MPPLY_H3_COOLDOWN"), 0, true);
                            /* LSC selling vehicles */
                            STATS::STAT_SET_INT(rage::joaat("MPPLY_VEHICLE_SELL_TIME"), 0, true);
                            STATS::STAT_SET_INT(rage::joaat("MPPLY_NUM_CARS_SOLD_TODAY"), 0, true);
                            /* Mental state */
                            STATS::STAT_SET_FLOAT(rage::joaat("MPPLY_PLAYER_MENTAL_STATE"), 0.f, true);
                            /* Apartament Heists */
                            STATS::STAT_SET_BOOL(rage::joaat("MPPLY_AWD_COMPLET_HEIST_1STPER"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MPPLY_AWD_COMPLET_HEIST_MEM"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MPPLY_AWD_FLEECA_FIN"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MPPLY_AWD_PRISON_FIN"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MPPLY_AWD_HUMANE_FIN"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MPPLY_AWD_SERIESA_FIN"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MPPLY_AWD_PACIFIC_FIN"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MPPLY_AWD_HST_ORDER"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MPPLY_AWD_HST_SAME_TEAM"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MPPLY_AWD_HST_ULT_CHAL"), true, true);
                            STATS::STAT_SET_INT(rage::joaat("MPPLY_HEIST_ACH_TRACKER"), -1, true);
                            /* Pilot school */
                            STATS::STAT_SET_INT(rage::joaat("MPPLY_PILOT_SCHOOL_MEDAL_0"), 1, true);
                            STATS::STAT_SET_INT(rage::joaat("MPPLY_PILOT_SCHOOL_MEDAL_1"), 1, true);
                            STATS::STAT_SET_INT(rage::joaat("MPPLY_PILOT_SCHOOL_MEDAL_2"), 1, true);
                            STATS::STAT_SET_INT(rage::joaat("MPPLY_PILOT_SCHOOL_MEDAL_3"), 1, true);
                            STATS::STAT_SET_INT(rage::joaat("MPPLY_PILOT_SCHOOL_MEDAL_4"), 1, true);
                            STATS::STAT_SET_INT(rage::joaat("MPPLY_PILOT_SCHOOL_MEDAL_5"), 1, true);
                            STATS::STAT_SET_INT(rage::joaat("MPPLY_PILOT_SCHOOL_MEDAL_6"), 1, true);
                            STATS::STAT_SET_INT(rage::joaat("MPPLY_PILOT_SCHOOL_MEDAL_7"), 1, true);
                            STATS::STAT_SET_INT(rage::joaat("MPPLY_PILOT_SCHOOL_MEDAL_8"), 1, true);
                            STATS::STAT_SET_INT(rage::joaat("MPPLY_PILOT_SCHOOL_MEDAL_9"), 1, true);
                            const auto mpx = std::to_string(g_local.character);
                            /* Mental state */
                            STATS::STAT_SET_FLOAT(rage::joaat("MP" + mpx + "_PLAYER_MENTAL_STATE"), 0.f, true);
                            /* Benny's */
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_LOW_FLOW_CS_DRV_SEEN"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_LOW_FLOW_CS_TRA_SEEN"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_LOW_FLOW_CS_FUN_SEEN"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_LOW_FLOW_CS_PHO_SEEN"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_LOW_FLOW_CS_FIN_SEEN"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_LOW_BEN_INTRO_CS_SEEN"), true, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LOWRIDER_FLOW_COMPLETE"), 3, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LOW_FLOW_CURRENT_PROG"), 9, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LOW_FLOW_CURRENT_CALL"), 9, true);
                            /* Apartament Heists */
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_HEIST_1_UNLCK"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_VEHICLE_1_UNLCK"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_VEHICLE_2_UNLCK"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_HEIST_COMPLETION"), -1, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_ACTIVATE_2_PERSON_KEY"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_ALL_ROLES_HEIST"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_HEIST_PLANNING_DONE_PRINT"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_HEIST_PLANNING_DONE_HELP_0"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_HEIST_PLANNING_DONE_HELP_1"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_HEIST_PRE_PLAN_DONE_HELP_0"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_HEIST_CUTS_DONE_FINALE"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_HEIST_CUTS_DONE_ORNATE"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_HEIST_CUTS_DONE_BIOLAB"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_HEIST_CUTS_DONE_PRISON"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_HEIST_CUTS_DONE_NARCOTIC"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_HEIST_CUTS_DONE_TUTORIAL"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_HEIST_AWARD_DONE_PREP"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_HEIST_AWARD_BOUGHT_IN"), true, true);
                            /* Agatha Baker */
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_VCM_FLOW_CS_RSC_SEEN"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_VCM_FLOW_CS_BWL_SEEN"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_VCM_FLOW_CS_MTG_SEEN"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_VCM_FLOW_CS_OIL_SEEN"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_VCM_FLOW_CS_DEF_SEEN"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_VCM_FLOW_CS_FIN_SEEN"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_CAS_VEHICLE_REWARD"), false, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_VCM_FLOW_PROGRESS"), 1839072, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_VCM_STORY_PROGRESS"), 0, true);
                            /* Facility heists */
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_GANGOPS_HEIST_STATUS"), 9999, true);
                            /* Skip cutscenes */
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_FM_INTRO_CUT_DONE"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_FM_CUT_DONE"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_FM_CUT_DONE_2"), -1, true);
                            /* Pilot school */
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_PILOT_ASPASSEDLESSON_0"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_PILOT_ASPASSEDLESSON_1"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_PILOT_ASPASSEDLESSON_2"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_PILOT_ASPASSEDLESSON_3"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_PILOT_ASPASSEDLESSON_4"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_PILOT_ASPASSEDLESSON_5"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_PILOT_ASPASSEDLESSON_6"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_PILOT_ASPASSEDLESSON_7"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_PILOT_ASPASSEDLESSON_8"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_PILOT_ASPASSEDLESSON_9"), true, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_PILOT_SCHOOL_MEDAL_0"), 1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_PILOT_SCHOOL_MEDAL_1"), 1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_PILOT_SCHOOL_MEDAL_2"), 1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_PILOT_SCHOOL_MEDAL_3"), 1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_PILOT_SCHOOL_MEDAL_4"), 1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_PILOT_SCHOOL_MEDAL_5"), 1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_PILOT_SCHOOL_MEDAL_6"), 1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_PILOT_SCHOOL_MEDAL_7"), 1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_PILOT_SCHOOL_MEDAL_8"), 1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_PILOT_SCHOOL_MEDAL_9"), 1, true);
                            /* Unlock shotaro */
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CRDEADLINE"), 5, true);
                            /* Shooting range */
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_SR_HIGHSCORE_1"), 690, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_SR_HIGHSCORE_2"), 1860, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_SR_HIGHSCORE_3"), 2690, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_SR_HIGHSCORE_4"), 2660, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_SR_HIGHSCORE_5"), 2650, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_SR_HIGHSCORE_6"), 450, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_SR_WEAPON_BIT_SET"), -1, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_SR_TIER_1_REWARD"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_SR_TIER_3_REWARD"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_SR_INCREASE_THROW_CAP"), true, true);
                            /* LSC */
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_CL_PLAY_GTA_RACE"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_CL_RACE_MODDED_CAR"), true, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_CARMOD_1_UNLCK"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_CARMOD_2_UNLCK"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_CARMOD_3_UNLCK"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_CARMOD_4_UNLCK"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_CARMOD_5_UNLCK"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_CARMOD_6_UNLCK"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_CARMOD_7_UNLCK"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_FM_RACES_FASTEST_LAP"), 82, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_NUMBER_TURBO_STARTS_IN_RACE"), 63, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_CARMOD_7_UNLCK"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_USJS_FOUND"), 50, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_USJS_COMPLETED"), 50, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_NUMBER_SLIPSTREAMS_IN_RACE"), 127, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_FINISH_HEISTS"), 50, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_FINISH_HEIST_SETUP_JOB"), 50, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_FMRALLYWONDRIVE "), 1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_FMRALLYWONNAV"), 1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_FMWINAIRRACE "), 1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_FMWINSEARACE"), 1, true);
                            STATS::STAT_SET_INT(rage::joaat("MPPLY_XMASLIVERIES0"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MPPLY_XMASLIVERIES1"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MPPLY_XMASLIVERIES2"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MPPLY_XMASLIVERIES3"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MPPLY_XMASLIVERIES4"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MPPLY_XMASLIVERIES5"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MPPLY_XMASLIVERIES6"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MPPLY_XMASLIVERIES7"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MPPLY_XMASLIVERIES8"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MPPLY_XMASLIVERIES9"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MPPLY_XMASLIVERIES10"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MPPLY_XMASLIVERIES11"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MPPLY_XMASLIVERIES12"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MPPLY_XMASLIVERIES13"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MPPLY_XMASLIVERIES14"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MPPLY_XMASLIVERIES15"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MPPLY_XMASLIVERIES16"), -1, true);
                            /* Arcade */
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_H3_BOARD_DIALOGUE0"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_H3_BOARD_DIALOGUE1"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_H3_BOARD_DIALOGUE2"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_H3_VEHICLESUSED"), -1, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_SCOPEOUT"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_CREWEDUP"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_MOVINGON"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_PROMOCAMP"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_GUNMAN"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_SMASHNGRAB"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_INPLAINSI"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_UNDETECTED"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_ALLROUND"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_ELITETHEIF"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_PRO"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_SUPPORTACT"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_SHAFTED"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_COLLECTOR"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_DEADEYE"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_PISTOLSATDAWN"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_TRAFFICAVOI"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_CANTCATCHBRA"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_WIZHARD"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_APEESCAPE"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_MONKEYKIND"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_AQUAAPE"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_NEMESIS"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_TRUELOVE"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_FRIENDZONED"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_KINGOFQUB3D"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_QUBISM"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_QUIBITS"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_GODOFQUB3D"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_GOFOR11TH"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_ELEVENELEVEN"), true, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_PREPARATION"), 40, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_ASLEEPONJOB"), 20, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_DAICASHCRAB"), 100000, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_BIGBRO"), 40, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_SHARPSHOOTER"), 40, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_RACECHAMP"), 40, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_BATSWORD"), 1000000, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_COINPURSE"), 950000, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_ASTROCHIMP"), 3000000, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_MASTERFUL"), 40000, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_COINPURSE"), 950000, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CH_ARC_CAB_CLAW_TROPHY"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CH_ARC_CAB_LOVE_TROPHY"), -1, true);

                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_LOSTANDFOUND"), 500000, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_SUNSET"), 1800000, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_TREASURE_HUNTER"), 1000000, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_WRECK_DIVING"), 1000000, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_KEINEMUSIK"), 1800000, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_PALMS_TRAX"), 1800000, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_MOODYMANN"), 1800000, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_FILL_YOUR_BAGS"), 10000000, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_WELL_PREPARED"), 50, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_H4_PLAYTHROUGH_STATUS"), -1, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_INTELGATHER"), TRUE, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_COMPOUNDINFILT"), TRUE, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_LOOT_FINDER"), TRUE, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_MAX_DISRUPT"), TRUE, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_THE_ISLAND_HEIST"), TRUE, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_GOING_ALONE"), TRUE, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_TEAM_WORK"), TRUE, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_MIXING_UP"), TRUE, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_PRO_THIEF"), TRUE, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_CAT_BURGLAR"), TRUE, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_ONE_OF_THEM"), TRUE, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_GOLDEN_GUN"), TRUE, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_ELITE_THIEF"), TRUE, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_PROFESSIONAL"), TRUE, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_HELPING_OUT"), TRUE, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_COURIER"), TRUE, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_PARTY_VIBES"), TRUE, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_HELPING_HAND"), TRUE, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_ELEVENELEVEN"), TRUE, true);

                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_WEAP_UNLOCKED"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_WEAP_UNLOCKED2"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_WEAP_ADDON_1_UNLCK"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_WEAP_ADDON_2_UNLCK"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_WEAP_ADDON_3_UNLCK"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_WEAP_ADDON_4_UNLCK"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_WEAP_UNLOCKED"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_WEAP_UNLOCKED2"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_WEAP_UNLOCKED3"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_WEAP_ADDON_1_UNLCK"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_WEAP_ADDON_2_UNLCK"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_WEAP_ADDON_3_UNLCK"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_WEAP_ADDON_4_UNLCK"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_WEAP_ADDON_5_UNLCK"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_WEAP_ADDON_6_UNLCK"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_WEAP_ADDON_7_UNLCK"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_WEAP_ADDON_8_UNLCK"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_WEAP_ADDON_9_UNLCK"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_WEAP_ADDON_10_UNLCK"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_WEAP_ADDON_11_UNLCK"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_WEAP_ADDON_12_UNLCK"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_WEAP_ADDON_13_UNLCK"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_WEAP_ADDON_14_UNLCK"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_WEAP_ADDON_15_UNLCK"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_WEAP_ADDON_16_UNLCK"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_WEAP_ADDON_17_UNLCK"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_WEAP_ADDON_18_UNLCK"), -1, true);

                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_SCRIPT_INCREASE_STAM"), 100, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_SCRIPT_INCREASE_STRN"), 100, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_SCRIPT_INCREASE_FLY"), 100, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_SCRIPT_INCREASE_STL"), 100, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_SCRIPT_INCREASE_LUNG"), 100, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_SCRIPT_INCREASE_DRIV"), 100, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_SCRIPT_INCREASE_SHO"), 100, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_HEALTH_1_UNLCK"), 0xFFFFFFFF, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_HEALTH_2_UNLCK"), 0xFFFFFFFF, true);

                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_H3_BOARD_DIALOGUE0"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_H3_BOARD_DIALOGUE1"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_H3_BOARD_DIALOGUE2"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_H3_VEHICLESUSED"), -1, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_SCOPEOUT"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_CREWEDUP"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_MOVINGON"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_PROMOCAMP"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_GUNMAN"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_SMASHNGRAB"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_INPLAINSI"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_UNDETECTED"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_ALLROUND"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_ELITETHEIF"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_PRO"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_SUPPORTACT"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_SHAFTED"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_COLLECTOR"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_DEADEYE"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_PISTOLSATDAWN"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_TRAFFICAVOI"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_CANTCATCHBRA"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_WIZHARD"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_APEESCAPE"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_MONKEYKIND"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_AQUAAPE"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_NEMESIS"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_TRUELOVE"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_FRIENDZONED"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_KINGOFQUB3D"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_QUBISM"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_QUIBITS"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_GODOFQUB3D"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_GOFOR11TH"), true, true);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_ELEVENELEVEN"), true, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_PREPARATION"), 40, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_ASLEEPONJOB"), 20, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_DAICASHCRAB"), 100000, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_BIGBRO"), 40, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_SHARPSHOOTER"), 40, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_RACECHAMP"), 40, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_BATSWORD"), 1000000, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_COINPURSE"), 950000, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_ASTROCHIMP"), 3000000, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_MASTERFUL"), 40000, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_COINPURSE"), 950000, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CH_ARC_CAB_CLAW_TROPHY"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CH_ARC_CAB_LOVE_TROPHY"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_H3_COMPLETEDPOSIX"), 0, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CAS_HEIST_FLOW"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_H3OPT_ACCESSPOINTS"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_H3OPT_POI"), -1, true);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_CAR_CLUB_MEM"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_SPRINTRACER"), 250, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_STREETRACER"), 250, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_PURSUITRACER"), 250, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_TEST_CAR"), 1800000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_CAR_EXPORT"), 100, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_GROUNDWORK"), 250, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_ROBBERY_CONTRACT"), 100, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_FACES_OF_DEATH"), 250, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_AUTO_SHOP"), 100, TRUE);
                            controller::set_packed_int(7315, 6, -1, TRUE);
                            controller::set_packed_int(18981, 4, -1, TRUE);
                            controller::set_packed_int(18988, 24, -1, TRUE);
                            controller::set_packed_int(22032, 100, -1, TRUE);
                            controller::set_packed_int(22050, 100, -1, TRUE);
                            controller::set_packed_int(22051, 100, -1, TRUE);
                            controller::set_packed_int(22052, 100, -1, TRUE);
                            controller::set_packed_int(22053, 100, -1, TRUE);
                            controller::set_packed_int(22054, 100, -1, TRUE);
                            controller::set_packed_int(22055, 100, -1, TRUE);
                            controller::set_packed_int(22056, 100, -1, TRUE);
                            controller::set_packed_int(22057, 100, -1, TRUE);
                            controller::set_packed_int(22058, 100, -1, TRUE);
                            controller::set_packed_int(22063, 20, -1, TRUE);
                            Any date[]{ 2013, 0, 9, 0, 17, 0, 20, 0, 30, 0, 1, 0, 1 };
                            STATS::STAT_SET_DATE(rage::joaat("MP" + mpx + "_CHAR_DATE_CREATED"), &date[0], 7, TRUE);
                            STATS::STAT_SET_DATE(rage::joaat("MP" + mpx + "_CHAR_DATE_RANKUP"), &date[0], 7, TRUE);
                            STATS::STAT_SET_DATE(rage::joaat("MPPLY_STARTED_MP"), &date[0], 7, TRUE);
                            STATS::STAT_SET_DATE(rage::joaat("MPPLY_NON_CHEATER_CASH"), &date[0], 7, TRUE);
                            STATS::STAT_SET_DATE(rage::joaat("MP" + mpx + "_CHAR_LAST_PLAY_TIME"), &date[0], 7, TRUE);
                            STATS::STAT_INCREMENT(rage::joaat("MPPLY_TOTAL_TIME_IN_LOBBY"), systems::int_to_float(INT_MAX));
                            STATS::STAT_INCREMENT(rage::joaat("MP" + mpx + "_CLOUD_TIME_CHAR_CREATED"), systems::int_to_float(1379449800));
                            STATS::STAT_INCREMENT(rage::joaat("MP" + mpx + "_PS_TIME_CHAR_CREATED"), systems::int_to_float(1379449800));
                            STATS::STAT_INCREMENT(rage::joaat("MPPLY_TOTAL_TIME_SPENT_FREEMODE"), systems::int_to_float(INT_MAX));
                            STATS::STAT_INCREMENT(rage::joaat("LEADERBOARD_PLAYING_TIME"), systems::int_to_float(INT_MAX)); // found in mpstatssetup.
                            STATS::STAT_INCREMENT(rage::joaat("MP" + mpx + "_TOTAL_PLAYING_TIME"), systems::int_to_float(INT_MAX));
                            STATS::STAT_INCREMENT(rage::joaat("MP_PLAYING_TIME_NEW"), systems::int_to_float(INT_MAX)); // found in mpstatssetup.
                            STATS::STAT_INCREMENT(rage::joaat("MP_PLAYING_TIME"), systems::int_to_float(INT_MAX)); // found in mpstatssetupui.
                            STATS::STAT_INCREMENT(rage::joaat("MP" + mpx + "_LONGEST_PLAYING_TIME"), systems::int_to_float(INT_MAX));
                            STATS::STAT_SAVE(0, 0, 3, 0);
                            break;
                        }
                        case 1:
                        {
                            const auto mpx = std::to_string(gta_util::get_character());
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_CL_RACE_MODDED_CAR"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_CL_DRIVE_RALLY"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_CL_PLAY_GTA_RACE"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_CL_PLAY_BOAT_RACE"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_CL_PLAY_FOOT_RACE"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_CL_PLAY_TEAM_DM"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_CL_PLAY_VEHICLE_DM"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_CL_PLAY_MISSION_CONTACT"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_CL_PLAY_A_PLAYLIST"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_CL_PLAY_POINT_TO_POINT"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_CL_PLAY_ONE_ON_ONE_DM"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_CL_PLAY_ONE_ON_ONE_RACE"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_CL_SURV_A_BOUNTY"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_CL_SET_WANTED_LVL_ON_PLAY"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_CL_GANG_BACKUP_GANGS"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_CL_GANG_BACKUP_LOST"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_CL_GANG_BACKUP_VAGOS"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_CL_CALL_MERCENARIES"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_CL_PHONE_MECH_DROP_CAR"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_CL_GONE_OFF_RADAR"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_CL_FILL_TITAN"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_CL_MOD_CAR_USING_APP"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_CL_BUY_APPARTMENT"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_CL_BUY_INSURANCE"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_CL_BUY_GARAGE"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_CL_ENTER_FRIENDS_HOUSE"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_CL_CALL_STRIPPER_HOUSE"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_CL_CALL_FRIEND"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_CL_SEND_FRIEND_REQUEST"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_CL_W_WANTED_PLAYER_TV"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_PILOT_ASPASSEDLESSON_0"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_PILOT_ASPASSEDLESSON_1"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_PILOT_ASPASSEDLESSON_2"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_PILOT_ASPASSEDLESSON_3"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_PILOT_ASPASSEDLESSON_4"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_PILOT_ASPASSEDLESSON_5"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_PILOT_ASPASSEDLESSON_6"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_PILOT_ASPASSEDLESSON_7"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_PILOT_ASPASSEDLESSON_8"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_PILOT_ASPASSEDLESSON_9"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_FIRST_TIME1"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_FIRST_TIME2"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_FIRST_TIME3"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_FIRST_TIME4"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_FIRST_TIME5"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_FIRST_TIME6"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_ALL_IN_ORDER"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_SUPPORTING_ROLE"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_ACTIVATE_2_PERSON_KEY"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_ALL_ROLES_HEIST"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_LEADER"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_SURVIVALIST"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_SCOPEOUT"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_CREWEDUP"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_MOVINGON"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_PROMOCAMP "), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_GUNMAN"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_SMASHNGRAB"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_INPLAINSI "), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_UNDETECTED"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_ALLROUND"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_ELITETHEIF"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_PRO"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_SUPPORTACT"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_SHAFTED"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_COLLECTOR "), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_DEADEYE"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_PISTOLSATDAWN "), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_TRAFFICAVOI"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_CANTCATCHBRA"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_WIZHARD"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_APEESCAPE "), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_MONKEYKIND"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_AQUAAPE"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_KEEPFAITH "), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_TRUELOVE"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_NEMESIS"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_FRIENDZONED"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MPPLY_AWD_GANGOPS_ALLINORDER"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MPPLY_AWD_GANGOPS_SUPPORT"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MPPLY_AWD_GANGOPS_LOYALTY2"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MPPLY_AWD_GANGOPS_CRIMMASMD2"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MPPLY_AWD_GANGOPS_LOYALTY3"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MPPLY_AWD_GANGOPS_CRIMMASMD3"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MPPLY_AWD_GANGOPS_LOYALTY"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MPPLY_AWD_GANGOPS_CRIMMASMD"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_BUY_EVERY_GUN"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_DAILYOBJMONTHBONUS"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_DAILYOBJWEEKBONUS"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_DRIVELESTERCAR5MINS"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_FINISH_HEIST_NO_DAMAGE"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_FM25DIFFERENTDM"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_FM25DIFFERENTRACES"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_FM25DIFITEMSCLOTHES"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_FMFURTHESTWHEELIE"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_FM6DARTCHKOUT"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_FM_GOLF_HOLE_IN_1"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_FM_SHOOTRANG_GRAN_WON"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_FM_TENNIS_5_SET_WINS"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_FMATTGANGHQ"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_FMFULLYMODDEDCAR"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_FMKILL3ANDWINGTARACE"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_FMKILLSTREAKSDM"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_FMMOSTKILLSGANGHIDE"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_FMMOSTKILLSSURVIVE"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_FMPICKUPDLCCRATE1ST"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_FMRACEWORLDRECHOLDER"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_FMTATTOOALLBODYPARTS"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_FMWINALLRACEMODES"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_FMWINCUSTOMRACE"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_FMWINEVERYGAMEMODE"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_SPLIT_HEIST_TAKE_EVENLY"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_STORE_20_CAR_IN_GARAGES"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MPPLY_MELEECHLENGECOMPLETED"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MPPLY_HEADSHOTCHLENGECOMPLETED"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_SR_TIER_1_REWARD"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_SR_TIER_3_REWARD"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_SR_INCREASE_THROW_CAP"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MPPLY_AWD_COMPLET_HEIST_MEM"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MPPLY_AWD_COMPLET_HEIST_1STPER"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MPPLY_AWD_FLEECA_FIN"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MPPLY_AWD_HST_ORDER"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MPPLY_AWD_HST_SAME_TEAM"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MPPLY_AWD_HST_ULT_CHAL"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MPPLY_AWD_HUMANE_FIN"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MPPLY_AWD_PACIFIC_FIN"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MPPLY_AWD_PRISON_FIN"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MPPLY_AWD_SERIESA_FIN"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MPPLY_AWD_GANGOPS_IAA"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MPPLY_AWD_GANGOPS_SUBMARINE"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MPPLY_AWD_GANGOPS_MISSILE"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_CLUB_COORD"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_CLUB_HOTSPOT"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_CLUB_CLUBBER"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_BEGINNER"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_FIELD_FILLER"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_ARMCHAIR_RACER"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_LEARNER"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_SUNDAY_DRIVER"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_THE_ROOKIE"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_BUMP_AND_RUN"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_GEAR_HEAD"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_DOOR_SLAMMER"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_HOT_LAP"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_ARENA_AMATEUR"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_PAINT_TRADER"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_SHUNTER"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_JOCK"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_WARRIOR"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_T_BONE"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_MAYHEM"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_WRECKER"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_CRASH_COURSE"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_ARENA_LEGEND"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_PEGASUS"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_CONTACT_SPORT"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_UNSTOPPABLE"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_LOW_FLOW_CS_DRV_SEEN"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_LOW_FLOW_CS_TRA_SEEN"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_LOW_FLOW_CS_FUN_SEEN"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_LOW_FLOW_CS_PHO_SEEN"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_LOW_FLOW_CS_FIN_SEEN"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_LOW_BEN_INTRO_CS_SEEN"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_HELP_FURIA"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_HELP_MINITANK"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_HELP_YOSEMITE2"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_HELP_ZHABA"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_HELP_IMORGEN"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_HELP_SULTAN2"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_HELP_VAGRANT"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_HELP_VSTR"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_HELP_STRYDER"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_HELP_SUGOI"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_HELP_KANJO"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_HELP_FORMULA"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_HELP_FORMULA2"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_HELP_JB7002"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_UNLOCK_RACE_HIPSTER_TSHIRT"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_UNLOCK_DM_HIPSTER_TSHIRT"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_UNLOCK_HIPSTER_TSHIRT_DOG"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_UNLOCK_HIPSTER_TSHIRT_VINYL"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_UNLOCK_HIPSTER_TSHIRT_MESS"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_KINGOFQUB3D"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_QUBISM"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_QUIBITS"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_GODOFQUB3D"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_GOFOR11TH"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_ELEVENELEVEN"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_QUIBITS"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_QUIBITS"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_INTELGATHER"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_COMPOUNDINFILT"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_LOOT_FINDER"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_MAX_DISRUPT"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_THE_ISLAND_HEIST"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_GOING_ALONE"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_TEAM_WORK"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_MIXING_UP"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_PRO_THIEF"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_CAT_BURGLAR"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_ONE_OF_THEM"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_GOLDEN_GUN"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_ELITE_THIEF"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_PROFESSIONAL"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_HELPING_OUT"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_COURIER"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_PARTY_VIBES"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_HELPING_HAND"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_ELEVENELEVEN"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_COMPLETE_H4_F_USING_VETIR"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_COMPLETE_H4_F_USING_LONGFIN"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_COMPLETE_H4_F_USING_ANNIH"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_COMPLETE_H4_F_USING_ALKONOS"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_COMPLETE_H4_F_USING_PATROLB"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_INTELGATHER"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_COMPOUNDINFILT"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_LOOT_FINDER"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_MAX_DISRUPT"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_THE_ISLAND_HEIST"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_GOING_ALONE"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_TEAM_WORK"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_MIXING_UP"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_PRO_THIEF"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_CAT_BURGLAR"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_ONE_OF_THEM"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_GOLDEN_GUN"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_ELITE_THIEF"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_PROFESSIONAL"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_HELPING_OUT"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_COURIER"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_PARTY_VIBES"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_HELPING_HAND"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_ELEVENELEVEN"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_COMPLETE_H4_F_USING_VETIR"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_COMPLETE_H4_F_USING_LONGFIN"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_COMPLETE_H4_F_USING_ANNIH"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_COMPLETE_H4_F_USING_ALKONOS"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_COMPLETE_H4_F_USING_PATROLB"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_CAR_CLUB"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_PRO_CAR_EXPORT"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_UNION_DEPOSITORY"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_MILITARY_CONVOY"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_FLEECA_BANK"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_FREIGHT_TRAIN"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_BOLINGBROKE_ASS"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_IAA_RAID"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_METH_JOB"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_BUNKER_RAID"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_STRAIGHT_TO_VIDEO"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_MONKEY_C_MONKEY_DO"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_TRAINED_TO_KILL"), TRUE, TRUE);
                            STATS::STAT_SET_BOOL(rage::joaat("MP" + mpx + "_AWD_DIRECTOR"), TRUE, TRUE);
                            break;
                        }
                        case 2:
                        {
                            const auto mpx = std::to_string(gta_util::get_character());
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_LOSTANDFOUND"), 500000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_SUNSET"), 1800000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_TREASURE_HUNTER"), 1000000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_WRECK_DIVING"), 1000000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_KEINEMUSIK"), 1800000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_PALMS_TRAX"), 1800000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_MOODYMANN"), 1800000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_FILL_YOUR_BAGS"), 200000000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_WELL_PREPARED"), 50, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_H4_PLAYTHROUGH_STATUS"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_LOSTANDFOUND"), 500000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_SUNSET"), 1800000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_TREASURE_HUNTER"), 1000000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_WRECK_DIVING"), 1000000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_KEINEMUSIK"), 1800000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_PALMS_TRAX"), 1800000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_MOODYMANN"), 1800000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_FILL_YOUR_BAGS"), 200000000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_WELL_PREPARED"), 50, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_H4_PLAYTHROUGH_STATUS"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MKRIFLE_MK2_KILLS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MKRIFLE_MK2_DEATHS"), 100, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MKRIFLE_MK2_SHOTS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MKRIFLE_MK2_HITS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MKRIFLE_MK2_HEADSHOTS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MKRIFLE_MK2_HELDTIME"), 5963259, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MKRIFLE_MK2_DB_HELDTIME"), 5963259, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MKRIFLE_MK2_ENEMY_KILLS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_ODD_JOBS"), 52, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_PREPARATION"), 50, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_ASLEEPONJOB"), 20, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_DAICASHCRAB"), 100000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_BIGBRO"), 40, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_SHARPSHOOTER"), 40, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_RACECHAMP"), 40, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_BATSWORD"), 1000000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_COINPURSE"), 950000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_ASTROCHIMP"), 3000000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_MASTERFUL"), 40000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DM_CURRENT_KILLS"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DM_CURRENT_ASSISTS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DM_CURRENT_DEATHS"), 600, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DM_HIGHEST_KILLSTREAK"), 900, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_HIGHEST_SKITTLES"), 900, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_NUMBER_NEAR_MISS"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LAP_DANCED_BOUGHT"), 100, TRUE);
                            STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_NUM_CAPTURES_CREATED"), 100, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CARS_EXPLODED"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CARS_COPS_EXPLODED"), 300, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_BIKES_EXPLODED"), 100, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_BOATS_EXPLODED"), 168, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_HELIS_EXPLODED"), 98, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_PLANES_EXPLODED"), 138, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_QUADBIKE_EXPLODED"), 50, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_BICYCLE_EXPLODED"), 48, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_SUBMARINE_EXPLODED"), 28, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DEATHS"), 4998, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DIED_IN_DROWNING"), 833, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DIED_IN_DROWNINGINVEHICLE"), 833, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DIED_IN_EXPLOSION"), 833, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DIED_IN_FALL"), 833, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DIED_IN_FIRE"), 833, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DIED_IN_ROAD"), 833, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_NO_PHOTOS_TAKEN"), 100, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_PROSTITUTES_FREQUENTED"), 100, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_BOUNTSONU"), 200, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_BOUNTPLACED"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_PASS_DB_KILLS"), 300, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_PASS_DB_PLAYER_KILLS"), 300, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_PASS_DB_SHOTS"), 300, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_PASS_DB_HITS"), 300, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_PASS_DB_HITS_PEDS_VEHICLES"), 300, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_PASS_DB_HEADSHOTS"), 300, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_TIRES_POPPED_BY_GUNSHOT"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_NUMBER_CRASHES_CARS"), 300, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_NUMBER_CRASHES_BIKES"), 300, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_BAILED_FROM_VEHICLE"), 300, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_NUMBER_CRASHES_QUADBIKES"), 300, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_NUMBER_STOLEN_COP_VEHICLE"), 300, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_NUMBER_STOLEN_CARS"), 300, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_NUMBER_STOLEN_BIKES"), 300, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_NUMBER_STOLEN_BOATS"), 300, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_NUMBER_STOLEN_HELIS"), 300, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_NUMBER_STOLEN_PLANES"), 300, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_NUMBER_STOLEN_QUADBIKES"), 300, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_NUMBER_STOLEN_BICYCLES"), 300, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_FAVOUTFITBIKETIMECURRENT"), 884483972, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_FAVOUTFITBIKETIME1ALLTIME"), 884483972, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_FAVOUTFITBIKETYPECURRENT"), 884483972, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_FAVOUTFITBIKETYPEALLTIME"), 884483972, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MC_CONTRIBUTION_POINTS"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MEMBERSMARKEDFORDEATH"), 268, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MCKILLS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MCDEATHS"), 200, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_RIVALPRESIDENTKILLS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_RIVALCEOANDVIPKILLS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MELEEKILLS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLUBHOUSECONTRACTSCOMPLETE"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLUBHOUSECONTRACTEARNINGS"), 32698547, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLUBCHALLENGESCOMPLETED"), 300, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MEMBERCHALLENGESCOMPLETED"), 100, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LONGEST_PLAYING_TIME"), 1047483647, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_TIME_DRIVING_CAR"), 1047483647, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_TIME_DRIVING_PLANE"), 1047483647, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_TIME_DRIVING_HELI"), 1047483647, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_TIME_DRIVING_QUADBIKE"), 1047483647, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_TIME_DRIVING_BIKE"), 1047483647, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_TIME_DRIVING_BICYCLE"), 1047483647, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_TIME_DRIVING_BOAT"), 1047483647, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_TIME_DRIVING_SUBMARINE"), 1047483647, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_TIME_SWIMMING"), 1047483647, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_TIME_WALKING"), 1047483647, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_TIME_UNDERWATER"), 1047483647, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_TIME_IN_WATER"), 1047483647, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_TIME_IN_COVER"), 1047483647, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_TIME_DRIVING_PASSENGER"), 1047483647, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_TOTAL_TIME_SPENT_ON_PHONE"), 1047483647, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_TIME_AS_A_PASSENGER"), 1047483647, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_TIME_AS_A_DRIVER"), 1047483647, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_TIME_SPENT_FLYING"), 1047483647, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_TIME_IN_CAR"), 1047483647, TRUE);
                            STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_TOTAL_TIME_MISSION_CREATO"), 1047483647, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_TOTAL_TIME_UNDERWATER"), 1047483647, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_TOTAL_TIME_CINEMA"), 1047483647, TRUE);
                            STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_TOTAL_TIME_LOAD_SCREEN"), 1047483647, TRUE);
                            STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_TOTAL_TIME_IN_LOBBY"), 1047483647, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_HITS"), 100000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MKRIFLE_KILLS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MKRIFLE_DEATHS"), 100, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MKRIFLE_SHOTS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MKRIFLE_HITS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MKRIFLE_HEADSHOTS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MKRIFLE_HELDTIME"), 5963259, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MKRIFLE_DB_HELDTIME"), 5963259, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MKRIFLE_ENEMY_KILLS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_BETAMOUNT"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_GHKILLS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_HORDELVL"), 10, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_HORDKILLS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_UNIQUECRATES"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_BJWINS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_HORDEWINS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MCMWINS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_GANGHIDWINS"), 500, TRUE);
                            STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_FM_MISSION_LIKES"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_KILLS"), 800, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_HITS_PEDS_VEHICLES"), 100, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_SHOTS"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_HEADSHOTS"), 100, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_KILLS_ARMED"), 650, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_SUCCESSFUL_COUNTERS"), 100, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_KILLS_PLAYERS"), 3593, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DEATHS_PLAYER"), 1002, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_KILLS_STEALTH"), 100, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_KILLS_INNOCENTS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_KILLS_ENEMY_GANG_MEMBERS"), 100, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_KILLS_FRIENDLY_GANG_MEMBERS"), 100, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_KILLS_BY_OTHERS"), 100, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_HITS"), 600, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_BIGGEST_VICTIM_KILLS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARCHENEMY_KILLS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CRARMWREST"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CRBASEJUMP"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CRDARTS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CRDM"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CRGANGHIDE"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CRGOLF"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CRHORDE"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CRMISSION"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CRSHOOTRNG"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CRTENNIS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_TOTAL_TIME_CINEMA"), 2147483647, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_NO_TIMES_CINEMA"), 500, TRUE);
                            STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_PLAYER_MENTAL_STATE"), 100, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_TIME_AS_A_PASSENGER"), 2147483647, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_TIME_AS_A_DRIVER"), 2147483647, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_TIME_SPENT_FLYING"), 2147483647, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_TIME_IN_CAR"), 2147483647, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BIKER_BUY_UNDERTA"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BIKER_BUY_COMPLET"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BIKER_SELL_UNDERTA"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BIKER_SELL_COMPLET"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BKR_SELL_UNDERTABC"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BKR_SELL_COMPLETBC"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_BKR_PROD_STOP_COUT_S1_0"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_BKR_PROD_STOP_COUT_S2_0"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_BKR_PROD_STOP_COUT_S3_0"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BKR_SELL_EARNINGS0"), 10000000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LFETIME_BIKER_BUY_UNDERTA1"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LFETIME_BIKER_BUY_COMPLET1"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LFETIME_BIKER_SELL_UNDERTA1"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LFETIME_BIKER_SELL_COMPLET1"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BKR_SEL_UNDERTABC1"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BKR_SEL_COMPLETBC1"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_BKR_PROD_STOP_COUT_S1_1"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_BKR_PROD_STOP_COUT_S2_1"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_BKR_PROD_STOP_COUT_S3_1"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BKR_SELL_EARNINGS1"), 10000000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LFETIME_BIKER_BUY_UNDERTA2"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LFETIME_BIKER_BUY_COMPLET2"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LFETIME_BIKER_SELL_UNDERTA2"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LFETIME_BIKER_SELL_COMPLET2"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BKR_SEL_UNDERTABC2"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BKR_SEL_COMPLETBC2"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_BKR_PROD_STOP_COUT_S1_2"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_BKR_PROD_STOP_COUT_S2_2"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_BKR_PROD_STOP_COUT_S3_2"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BKR_SELL_EARNINGS2"), 10000000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LFETIME_BIKER_BUY_UNDERTA3"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LFETIME_BIKER_BUY_COMPLET3"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LFETIME_BIKER_SELL_UNDERTA3"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LFETIME_BIKER_SELL_COMPLET3"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BKR_SEL_UNDERTABC3"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BKR_SEL_COMPLETBC3"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_BKR_PROD_STOP_COUT_S1_3"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_BKR_PROD_STOP_COUT_S2_3"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_BKR_PROD_STOP_COUT_S3_3"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BKR_SELL_EARNINGS3"), 10000000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LFETIME_BIKER_BUY_UNDERTA4"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LFETIME_BIKER_BUY_COMPLET4"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LFETIME_BIKER_SELL_UNDERTA4"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LFETIME_BIKER_SELL_COMPLET4"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BKR_SEL_UNDERTABC4"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BKR_SEL_COMPLETBC4"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_BKR_PROD_STOP_COUT_S1_4"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_BKR_PROD_STOP_COUT_S2_4"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_BKR_PROD_STOP_COUT_S3_4"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BKR_SELL_EARNINGS4"), 10000000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LFETIME_BIKER_BUY_UNDERTA5"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LFETIME_BIKER_BUY_COMPLET5"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BKR_SEL_UNDERTABC5"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BKR_SEL_COMPLETBC5"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LFETIME_BIKER_SELL_UNDERTA5"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LFETIME_BIKER_SELL_COMPLET5"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_BUNKER_UNITS_MANUFAC"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BKR_SELL_EARNINGS5"), 10000000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_ABILITY_1_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_ABILITY_2_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_ABILITY_3_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_ABILITY_1_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_ABILITY_2_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_ABILITY_3_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LFETIME_HANGAR_BUY_UNDETAK"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LFETIME_HANGAR_BUY_COMPLET"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LFETIME_HANGAR_SEL_UNDETAK"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LFETIME_HANGAR_SEL_COMPLET"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LFETIME_HANGAR_EARNINGS"), 29654123, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LFETIME_HANGAR_EARN_BONUS"), 15987456, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_RIVAL_HANGAR_CRATES_STOLEN"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LFETIME_IE_STEAL_STARTED"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LFETIME_IE_EXPORT_STARTED"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LFETIME_IE_EXPORT_COMPLETED"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LFETIME_IE_MISSION_EARNINGS"), 59654897, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AT_FLOW_IMPEXP_NUM"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLUB_POPULARITY"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_NIGHTCLUB_VIP_APPEAR"), 300, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_NIGHTCLUB_JOBS_DONE"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_NIGHTCLUB_EARNINGS"), 39856412, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_HUB_SALES_COMPLETED"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_HUB_EARNINGS"), 29865423, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DANCE_COMBO_DURATION_MINS"), 86400000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_NIGHTCLUB_PLAYER_APPEAR"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_HUB_GOODS_SOLD"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_HUB_GOODS_MADE"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ADMIN_CLOTHES_GV_BS_1"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ADMIN_CLOTHES_GV_BS_10"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ADMIN_CLOTHES_GV_BS_11"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ADMIN_CLOTHES_GV_BS_12"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ADMIN_CLOTHES_GV_BS_2"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ADMIN_CLOTHES_GV_BS_3"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ADMIN_CLOTHES_GV_BS_4"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ADMIN_CLOTHES_GV_BS_5"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ADMIN_CLOTHES_GV_BS_6"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ADMIN_CLOTHES_GV_BS_7"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ADMIN_CLOTHES_GV_BS_8"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ADMIN_CLOTHES_GV_BS_9"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ADMIN_WEAPON_GV_BS_1"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AIR_LAUNCHES_OVER_40M"), 25, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_5STAR_WANTED_AVOIDANCE"), 50, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_CAR_BOMBS_ENEMY_KILLS"), 25, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_CARS_EXPORTED"), 50, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_CONTROL_CROWDS"), 25, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_DAILYOBJCOMPLETED"), 100, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_DO_HEIST_AS_MEMBER"), 25, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_DO_HEIST_AS_THE_LEADER"), 25, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_DROPOFF_CAP_PACKAGES"), 100, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_FINISH_HEIST_SETUP_JOB"), 50, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_FINISH_HEISTS"), 50, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_FM_DM_3KILLSAMEGUY"), 50, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_FM_DM_KILLSTREAK"), 100, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_FM_DM_STOLENKILL"), 50, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_FM_DM_TOTALKILLS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_FM_DM_WINS"), 50, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_FM_GOLF_HOLE_IN_1"), 300, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_FM_GOLF_BIRDIES"), 25, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_FM_GOLF_WON"), 25, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_FM_GTA_RACES_WON"), 50, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_FM_RACE_LAST_FIRST"), 25, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_FM_RACES_FASTEST_LAP"), 50, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_FM_SHOOTRANG_CT_WON"), 25, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_FM_SHOOTRANG_RT_WON"), 25, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_FM_SHOOTRANG_TG_WON"), 25, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_FM_TDM_MVP"), 50, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_FM_TDM_WINS"), 50, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_FM_TENNIS_ACE"), 25, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_FM_TENNIS_WON"), 25, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_FMBASEJMP"), 25, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_FMBBETWIN"), 50000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_FMCRATEDROPS"), 25, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_FMDRIVEWITHOUTCRASH"), 30, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_FMHORDWAVESSURVIVE"), 10, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_FMKILLBOUNTY"), 25, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_FMRALLYWONDRIVE"), 25, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_FMRALLYWONNAV"), 25, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_FMREVENGEKILLSDM"), 50, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_FMSHOOTDOWNCOPHELI"), 25, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_FMWINAIRRACE"), 25, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_FMWINRACETOPOINTS"), 25, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_FMWINSEARACE"), 25, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_HOLD_UP_SHOPS"), 20, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_KILL_CARRIER_CAPTURE"), 100, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_KILL_PSYCHOPATHS"), 100, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_KILL_TEAM_YOURSELF_LTS"), 25, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_LAPDANCES"), 25, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_LESTERDELIVERVEHICLES"), 25, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_MENTALSTATE_TO_NORMAL"), 50, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_NIGHTVISION_KILLS"), 100, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_NO_HAIRCUTS"), 25, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_ODISTRACTCOPSNOEATH"), 25, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_ONLY_PLAYER_ALIVE_LTS"), 50, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_PARACHUTE_JUMPS_20M"), 25, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_PARACHUTE_JUMPS_50M"), 25, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_PASSENGERTIME"), 4, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_PICKUP_CAP_PACKAGES"), 100, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_RACES_WON"), 50, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_SECURITY_CARS_ROBBED"), 25, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_TAKEDOWNSMUGPLANE"), 50, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_TIME_IN_HELICOPTER"), 4, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_TRADE_IN_YOUR_PROPERTY"), 25, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_VEHICLES_JACKEDR"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_WIN_AT_DARTS"), 25, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_WIN_CAPTURE_DONT_DYING"), 25, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_WIN_CAPTURES"), 50, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_WIN_GOLD_MEDAL_HEISTS"), 25, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_WIN_LAST_TEAM_STANDINGS"), 50, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_BOTTLE_IN_POSSESSION"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_CARMOD_1_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_CARMOD_2_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_CARMOD_3_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_CARMOD_4_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_CARMOD_5_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_CARMOD_6_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_CARMOD_7_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_VEHICLE_1_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_VEHICLE_2_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_WEAP_ADDON_1_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_WEAP_ADDON_2_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_WEAP_ADDON_3_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_WEAP_ADDON_4_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_WEAP_ADDON_5_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_WEAP_ADDON_6_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_WEAP_ADDON_7_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_WEAP_ADDON_8_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_WEAP_ADDON_9_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_WEAP_ADDON_10_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_WEAP_ADDON_11_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_WEAP_ADDON_12_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_WEAP_ADDON_13_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_WEAP_ADDON_14_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_WEAP_ADDON_15_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_WEAP_ADDON_16_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_WEAP_ADDON_17_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_WEAP_ADDON_18_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_WEAP_UNLOCKED"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_WEAP_UNLOCKED2"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_WEAP_UNLOCKED3"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_10_FM_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_11_FM_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_12_FM_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_1_FM_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_2_FM_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_3_FM_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_4_FM_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_5_FM_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_6_FM_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_7_FM_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_8_FM_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_9_FM_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_10_FM_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_11_FM_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_12_FM_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_13_FM_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_14_FM_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_15_FM_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_16_FM_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_17_FM_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_18_FM_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_19_FM_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_20_FM_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_21_FM_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_22_FM_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_23_FM_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_24_FM_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_25_FM_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_26_FM_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_27_FM_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_28_FM_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_29_FM_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_30_FM_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_31_FM_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_32_FM_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_33_FM_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_34_FM_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_35_FM_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_36_FM_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_37_FM_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_38_FM_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_39_FM_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_40_FM_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_FM_PURCHASE"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_FM_PURCHASE10"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_FM_PURCHASE11"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_FM_PURCHASE12"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_FM_PURCHASE2"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_FM_PURCHASE3"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_FM_PURCHASE4"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_FM_PURCHASE5"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_FM_PURCHASE6"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_FM_PURCHASE7"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_FM_PURCHASE8"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_KIT_FM_PURCHASE9"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_WANTED_LEVEL_TIME5STAR"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_WEAP_FM_PURCHASE"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_WEAP_FM_PURCHASE2"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_BERD"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_BERD_1"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_BERD_2"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_BERD_3"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_BERD_4"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_BERD_5"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_BERD_6"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_DECL"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_FEET"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_FEET_1"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_FEET_2"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_FEET_3"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_FEET_4"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_FEET_5"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_FEET_6"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_FEET_7"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_JBIB"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_JBIB_1"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_JBIB_2"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_JBIB_3"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_JBIB_4"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_JBIB_5"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_JBIB_6"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_JBIB_7"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_LEGS"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_LEGS_1"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_LEGS_2"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_LEGS_3"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_LEGS_4"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_LEGS_5"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_LEGS_6"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_LEGS_7"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_OUTFIT"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_PROPS"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_PROPS_1"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_PROPS_10"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_PROPS_2"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_PROPS_3"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_PROPS_4"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_PROPS_5"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_PROPS_6"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_PROPS_7"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_PROPS_8"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_PROPS_9"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_SPECIAL"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_SPECIAL2"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_SPECIAL2_1"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_SPECIAL_1"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_SPECIAL_2"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_SPECIAL_3"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_SPECIAL_4"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_SPECIAL_5"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_SPECIAL_6"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_SPECIAL_7"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_TEETH"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_TEETH_1"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_TEETH_2"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_ACQUIRED_TORSO"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_BERD"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_BERD_1"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_BERD_2"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_BERD_3"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_BERD_4"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_BERD_5"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_BERD_6"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_BERD_7"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_DECL"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_FEET"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_FEET_1"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_FEET_2"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_FEET_3"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_FEET_4"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_FEET_5"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_FEET_6"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_FEET_7"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_HAIR"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_HAIR_1"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_HAIR_2"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_HAIR_3"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_HAIR_4"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_HAIR_5"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_HAIR_6"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_HAIR_7"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_JBIB"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_JBIB_1"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_JBIB_2"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_JBIB_3"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_JBIB_4"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_JBIB_5"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_JBIB_6"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_JBIB_7"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_LEGS"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_LEGS_1"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_LEGS_2"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_LEGS_3"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_LEGS_4"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_LEGS_5"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_LEGS_6"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_LEGS_7"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_OUTFIT"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_PROPS"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_PROPS_1"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_PROPS_10"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_PROPS_2"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_PROPS_3"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_PROPS_4"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_PROPS_5"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_PROPS_6"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_PROPS_7"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_PROPS_8"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_PROPS_9"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_SPECIAL"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_SPECIAL2"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_SPECIAL2_1"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_SPECIAL_1"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_SPECIAL_2"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_SPECIAL_3"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_SPECIAL_4"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_SPECIAL_5"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_SPECIAL_6"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_SPECIAL_7"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_TEETH"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_TEETH_1"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_TEETH_2"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CLTHS_AVAILABLE_TORSO"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DLC_APPAREL_ACQUIRED_0"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DLC_APPAREL_ACQUIRED_1"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DLC_APPAREL_ACQUIRED_10"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DLC_APPAREL_ACQUIRED_11"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DLC_APPAREL_ACQUIRED_12"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DLC_APPAREL_ACQUIRED_13"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DLC_APPAREL_ACQUIRED_14"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DLC_APPAREL_ACQUIRED_15"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DLC_APPAREL_ACQUIRED_16"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DLC_APPAREL_ACQUIRED_17"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DLC_APPAREL_ACQUIRED_18"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DLC_APPAREL_ACQUIRED_19"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DLC_APPAREL_ACQUIRED_2"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DLC_APPAREL_ACQUIRED_20"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DLC_APPAREL_ACQUIRED_21"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DLC_APPAREL_ACQUIRED_22"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DLC_APPAREL_ACQUIRED_23"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DLC_APPAREL_ACQUIRED_24"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DLC_APPAREL_ACQUIRED_25"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DLC_APPAREL_ACQUIRED_26"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DLC_APPAREL_ACQUIRED_27"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DLC_APPAREL_ACQUIRED_28"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DLC_APPAREL_ACQUIRED_29"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DLC_APPAREL_ACQUIRED_3"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DLC_APPAREL_ACQUIRED_30"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DLC_APPAREL_ACQUIRED_31"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DLC_APPAREL_ACQUIRED_32"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DLC_APPAREL_ACQUIRED_33"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DLC_APPAREL_ACQUIRED_34"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DLC_APPAREL_ACQUIRED_35"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DLC_APPAREL_ACQUIRED_36"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DLC_APPAREL_ACQUIRED_37"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DLC_APPAREL_ACQUIRED_38"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DLC_APPAREL_ACQUIRED_39"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DLC_APPAREL_ACQUIRED_4"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DLC_APPAREL_ACQUIRED_40},"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DLC_APPAREL_ACQUIRED_5"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DLC_APPAREL_ACQUIRED_6"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DLC_APPAREL_ACQUIRED_7"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DLC_APPAREL_ACQUIRED_8"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DLC_APPAREL_ACQUIRED_9"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_GRENADE_ENEMY_KILLS"), 50, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MICROSMG_ENEMY_KILLS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_SMG_ENEMY_KILLS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ASLTSMG_ENEMY_KILLS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ASLTRIFLE_ENEMY_KILLS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CRBNRIFLE_ENEMY_KILLS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ADVRIFLE_ENEMY_KILLS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MG_ENEMY_KILLS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CMBTMG_ENEMY_KILLS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ASLTMG_ENEMY_KILLS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_RPG_ENEMY_KILLS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_KILLS_PLAYERS"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LONGEST_WHEELIE_DIST"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MOST_ARM_WRESTLING_WINS"), 25, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_NO_CARS_REPAIR"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_VEHICLES_SPRAYED"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_NUMBER_NEAR_MISS_NOCRASH"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_USJS_FOUND"), 50, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_USJS_FOUND_MASK"), 50, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_USJS_COMPLETED"), 50, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_USJS_TOTAL_COMPLETED"), 50, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_USJS_COMPLETED_MASK"), 50, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MOST_FLIPS_IN_ONE_JUMP"), 5, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MOST_SPINS_IN_ONE_JUMP"), 5, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_NUMBER_SLIPSTREAMS_IN_RACE"), 100, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_NUMBER_TURBO_STARTS_IN_RACE"), 50, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_PASS_DB_PLAYER_KILLS"), 100, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_PISTOL_ENEMY_KILLS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_PLAYER_HEADSHOTS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_RACES_WON"), 50, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_SAWNOFF_ENEMY_KILLS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_SCRIPT_INCREASE_DRIV"), 100, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_SCRIPT_INCREASE_FLY"), 100, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_SCRIPT_INCREASE_LUNG"), 100, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_SCRIPT_INCREASE_MECH"), 100, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_SCRIPT_INCREASE_SHO"), 100, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_SCRIPT_INCREASE_STAM"), 100, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_SCRIPT_INCREASE_STL"), 100, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_SCRIPT_INCREASE_STRN"), 100, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_STKYBMB_ENEMY_KILLS"), 50, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_UNARMED_ENEMY_KILLS"), 50, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_USJS_COMPLETED"), 50, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_WEAP_FM_ADDON_PURCH"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_WEAP_FM_ADDON_PURCH2"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_WEAP_FM_ADDON_PURCH3"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_WEAP_FM_ADDON_PURCH4"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_WEAP_FM_ADDON_PURCH5"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CRDEADLINE"), 5, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_ABILITY_1_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_ABILITY_2_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_ABILITY_3_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_ABILITY_1_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_ABILITY_2_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_ABILITY_3_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BUY_COMPLETE"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BUY_UNDERTAKEN"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_SELL_COMPLETE"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_SELL_UNDERTAKEN"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_CONTRA_EARNINGS"), 20000000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BIKER_BUY_COMPLET"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BIKER_BUY_UNDERTA"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BIKER_SELL_COMPLET"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BIKER_SELL_UNDERTA"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BIKER_BUY_COMPLET1"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BIKER_BUY_UNDERTA1"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BIKER_SELL_COMPLET1"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BIKER_SELL_UNDERTA1"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BIKER_BUY_COMPLET2"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BIKER_BUY_UNDERTA2"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BIKER_SELL_COMPLET2"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BIKER_SELL_UNDERTA2"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BIKER_BUY_COMPLET3"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BIKER_BUY_UNDERTA3"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BIKER_SELL_COMPLET3"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BIKER_SELL_UNDERTA3"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BIKER_BUY_COMPLET4"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BIKER_BUY_UNDERTA4"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BIKER_SELL_COMPLET4"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BIKER_SELL_UNDERTA4"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BIKER_BUY_COMPLET5"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BIKER_BUY_UNDERTA5"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BIKER_SELL_COMPLET5"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BIKER_SELL_UNDERTA5"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BKR_SELL_EARNINGS0"), 20000000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BKR_SELL_EARNINGS1"), 20000000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BKR_SELL_EARNINGS2"), 20000000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BKR_SELL_EARNINGS3"), 20000000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BKR_SELL_EARNINGS4"), 20000000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LIFETIME_BKR_SELL_EARNINGS5"), 20000000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LFETIME_IE_EXPORT_COMPLETED"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LFETIME_IE_MISSION_EARNINGS"), 20000000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LFETIME_HANGAR_SEL_UNDETAK"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LFETIME_HANGAR_SEL_COMPLET"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LFETIME_HANGAR_EARNINGS"), 20000000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MPPLY_PILOT_SCHOOL_MEDAL_0"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MPPLY_PILOT_SCHOOL_MEDAL_1"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MPPLY_PILOT_SCHOOL_MEDAL_2"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MPPLY_PILOT_SCHOOL_MEDAL_3"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MPPLY_PILOT_SCHOOL_MEDAL_4"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MPPLY_PILOT_SCHOOL_MEDAL_5"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MPPLY_PILOT_SCHOOL_MEDAL_6"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MPPLY_PILOT_SCHOOL_MEDAL_7"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MPPLY_PILOT_SCHOOL_MEDAL_8"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MPPLY_PILOT_SCHOOL_MEDAL_9"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_SR_HIGHSCORE_1"), 690, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_SR_HIGHSCORE_2"), 1860, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_SR_HIGHSCORE_3"), 2690, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_SR_HIGHSCORE_4"), 2660, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_SR_HIGHSCORE_5"), 2650, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_SR_HIGHSCORE_6"), 450, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_SR_TARGETS_HIT"), 269, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_SR_WEAPON_BIT_SET"), -1, TRUE);
                            STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_HEIST_ACH_TRACKER"), -1, TRUE);
                            STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_GANGOPS_ALLINORDER"), 100, TRUE);
                            STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_GANGOPS_LOYALTY"), 100, TRUE);
                            STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_GANGOPS_CRIMMASMD"), 100, TRUE);
                            STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_GANGOPS_LOYALTY2"), 100, TRUE);
                            STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_GANGOPS_LOYALTY3"), 100, TRUE);
                            STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_GANGOPS_CRIMMASMD2"), 100, TRUE);
                            STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_GANGOPS_CRIMMASMD3"), 100, TRUE);
                            STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_GANGOPS_SUPPORT"), 100, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_GANGOPS_HEIST_STATUS"), 9999, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_NO_BOUGHT_YUM_SNACKS"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_DANCE_TO_SOLOMUN"), 100, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_DANCE_TO_TALEOFUS"), 100, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_DANCE_TO_DIXON"), 100, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_DANCE_TO_BLKMAD"), 100, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_CLUB_DRUNK"), 200, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_NIGHTCLUB_PLAYER_APPEAR"), 100, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_NUMUNIQUEPLYSINCLUB"), 100, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DANCETODIFFDJS"), 4, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DANCEPERFECTOWNCLUB"), 100, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_NIGHTCLUB_HOTSPOT_TIME_MS"), 3600000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_WATCH_YOUR_STEP"), 50, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_TOWER_OFFENSE"), 50, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_READY_FOR_WAR"), 50, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_THROUGH_A_LENS"), 50, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_SPINNER"), 50, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_YOUMEANBOOBYTRAPS"), 50, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_MASTER_BANDITO"), 50, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_SITTING_DUCK"), 50, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_CROWDPARTICIPATION"), 50, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_KILL_OR_BE_KILLED"), 50, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_MASSIVE_SHUNT"), 50, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_YOURE_OUTTA_HERE"), 200, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_WEVE_GOT_ONE"), 50, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_ARENA_WAGEWORKER"), 20000000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_TIME_SERVED"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_TOP_SCORE"), 500000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_CAREER_WINNER"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARENAWARS_SKILL_LEVEL"), 20, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARENAWARS_AP_TIER"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARENAWARS_AP_LIFETIME"), 47551850, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_W_THEME_SCIFI"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_W_THEME_APOC"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_W_THEME_CONS"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_W_PASS_THE_BOMB"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_W_DETONATION"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_W_ARCADE_RACE"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_W_CTF"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_W_TAG_TEAM"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_W_DESTR_DERBY"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_W_CARNAGE"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_W_MONSTER_JAM"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_W_GAMES_MASTERS"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_L_PASS_THE_BOMB"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_L_DETONATION"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_L_ARCADE_RACE"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_L_CTF"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_L_TAG_TEAM"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_L_DESTR_DERBY"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_L_CARNAGE"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_L_MONSTER_JAM"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_L_GAMES_MASTERS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_NUMBER_OF_CHAMP_BOUGHT"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_SPECTATOR_KILLS"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_LIFETIME_KILLS"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_LIFETIME_DEATHS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARENAWARS_CARRER_WINS"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARENAWARS_CARRER_WINT"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARENAWARS_MATCHES_PLYD"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARENAWARS_MATCHES_PLYDT"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_SPEC_BOX_TIME_MS"), 86400000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_SPECTATOR_DRONE"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_SPECTATOR_CAMS"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_SMOKE"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_DRINK"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_VEH_MONSTER3"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_VEH_MONSTER4"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_VEH_MONSTER5"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_VEH_CERBERUS"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_VEH_CERBERUS2"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_VEH_CERBERUS3"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_VEH_BRUISER"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_VEH_BRUISER2"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_VEH_BRUISER3"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_VEH_SLAMVAN4"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_VEH_SLAMVAN5"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_VEH_SLAMVAN6"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_VEH_BRUTUS"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_VEH_BRUTUS2"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_VEH_BRUTUS3"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_VEH_SCARAB"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_VEH_SCARAB2"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_VEH_SCARAB3"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_VEH_DOMINATOR4"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_VEH_DOMINATOR5"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_VEH_DOMINATOR6"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_VEH_IMPALER2"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_VEH_IMPALER3"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_VEH_IMPALER4"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_VEH_ISSI4"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_VEH_ISSI5"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_VEH_ISSI6"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_VEH_IMPERATOR"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_VEH_IMPERATOR2"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_VEH_IMPERATOR3"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_VEH_ZR380"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_VEH_ZR3802"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_VEH_ZR3803"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_VEH_DEATHBIKE"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_VEH_DEATHBIKE2"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_VEH_DEATHBIKE3"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_NO_BOUGHT_HEALTH_SNACKS"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_NO_BOUGHT_EPIC_SNACKS"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_NUMBER_OF_ORANGE_BOUGHT"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MP_CHAR_ARMOUR_1_COUNT"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MP_CHAR_ARMOUR_2_COUNT"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MP_CHAR_ARMOUR_3_COUNT"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MP_CHAR_ARMOUR_4_COUNT"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MP_CHAR_ARMOUR_5_COUNT"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_NUMBER_OF_BOURGE_BOUGHT"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CIGARETTES_BOUGHT"), 1000, TRUE);
                            STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_AWD_FM_CR_DM_MADE"), 600, TRUE);
                            STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_AWD_FM_CR_PLAYED_BY_PEEP"), 1168, TRUE);
                            STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_AWD_FM_CR_RACES_MADE"), 1000, TRUE);
                            STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_AWD_FM_CR_MISSION_SCORE"), 100, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_FM_ACT_PHN"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_FM_ACT_PH2"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_FM_ACT_PH3"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_FM_ACT_PH4"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_FM_ACT_PH5"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_FM_VEH_TX1"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_FM_ACT_PH6"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_FM_ACT_PH7"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_FM_ACT_PH8"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_FM_ACT_PH9"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LOWRIDER_FLOW_COMPLETE"), 3, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LOW_FLOW_CURRENT_PROG"), 9, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LOW_FLOW_CURRENT_CALL"), 9, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CR_GANGOP_MORGUE"), 10, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CR_GANGOP_DELUXO"), 10, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CR_GANGOP_SERVERFARM"), 10, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CR_GANGOP_IAABASE_FIN"), 10, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CR_GANGOP_STEALOSPREY"), 10, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CR_GANGOP_FOUNDRY"), 10, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CR_GANGOP_RIOTVAN"), 10, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CR_GANGOP_SUBMARINECAR"), 10, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CR_GANGOP_SUBMARINE_FIN"), 10, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CR_GANGOP_PREDATOR"), 10, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CR_GANGOP_BMLAUNCHER"), 10, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CR_GANGOP_BCCUSTOM"), 10, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CR_GANGOP_STEALTHTANKS"), 10, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CR_GANGOP_SPYPLANE"), 10, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CR_GANGOP_FINALE"), 10, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CR_GANGOP_FINALE_P2"), 10, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CR_GANGOP_FINALE_P3"), 10, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_SNIPERRFL_ENEMY_KILLS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_HVYSNIPER_ENEMY_KILLS"), 500, TRUE);
                            STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_TOTAL_TIME_SPENT_DEATHMAT"), 2147483647, TRUE);
                            STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_TOTAL_TIME_SPENT_RACES"), 2147483647, TRUE);
                            STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_TOTAL_RACES_WON"), 500, TRUE);
                            STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_TOTAL_RACES_LOST"), 250, TRUE);
                            STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_TOTAL_CUSTOM_RACES_WON"), 500, TRUE);
                            STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_TOTAL_DEATHMATCH_LOST"), 250, TRUE);
                            STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_TOTAL_DEATHMATCH_WON"), 500, TRUE);
                            STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_TOTAL_TDEATHMATCH_LOST"), 250, TRUE);
                            STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_TOTAL_TDEATHMATCH_WON"), 500, TRUE);
                            STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_SHOOTINGRANGE_WINS"), 500, TRUE);
                            STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_SHOOTINGRANGE_LOSSES"), 250, TRUE);
                            STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_TENNIS_MATCHES_WON"), 500, TRUE);
                            STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_TENNIS_MATCHES_LOST"), 250, TRUE);
                            STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_GOLF_WINS"), 500, TRUE);
                            STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_GOLF_LOSSES"), 250, TRUE);
                            STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_DARTS_TOTAL_WINS"), 500, TRUE);
                            STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_DARTS_TOTAL_MATCHES"), 750, TRUE);
                            STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_SHOOTINGRANGE_TOTAL_MATCH"), 800, TRUE);
                            STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_BJ_WINS"), 500, TRUE);
                            STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_BJ_LOST"), 250, TRUE);
                            STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_RACE_2_POINT_WINS"), 500, TRUE);
                            STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_RACE_2_POINT_LOST"), 250, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MONEY_SPENT_WEAPON_ARMOR"), 136985, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MONEY_SPENT_VEH_MAINTENANCE"), 33698562, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MONEY_SPENT_STYLE_ENT"), 5698456, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MONEY_SPENT_PROPERTY_UTIL"), 19874563, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MONEY_SPENT_JOB_ACTIVITY"), 1569856, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MONEY_SPENT_BETTING"), 986542, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MONEY_SPENT_CONTACT_SERVICE"), 15698745, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MONEY_SPENT_DROPPED_STOLEN"), 12356, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MONEY_SPENT_SHARED"), 986543, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MONEY_SPENT_JOBSHARED"), 459856, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MONEY_SPENT_HEALTHCARE"), 1569875, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MONEY_SPENT_EXEC_PA"), 1698564, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MONEY_SPENT_BUY_CONTRABAND"), 12398654, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MONEY_EARN_JOBS"), 786000000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MONEY_EARN_SELLING_VEH"), 12356897, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MONEY_EARN_BETTING"), 1265798, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MONEY_EARN_GOOD_SPORT"), 156987, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MONEY_EARN_PICKED_UP"), 125987, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MONEY_EARN_SHARED"), 598756, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MONEY_EARN_JOBSHARED"), 365798, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_MONEY_EARN_ROCKSTAR_AWARD"), 25654789, TRUE);
                            STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_KILLS_PLAYERS"), 3593, TRUE);
                            STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_DEATHS_PLAYER"), 1002, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_KILLS_COP"), 4500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_KILLS_SWAT"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_WANTED_LEVEL_TIME"), 5000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_NUMBER_STOLEN_COP_VEHICLE"), 200, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_NUMBER_STOLEN_CARS"), 200, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_NUMBER_STOLEN_BIKES"), 200, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_NUMBER_STOLEN_BOATS"), 200, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_NUMBER_STOLEN_HELIS"), 200, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_NUMBER_STOLEN_PLANES"), 200, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_NUMBER_STOLEN_QUADBIKES"), 200, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_NUMBER_STOLEN_BICYCLES"), 200, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_STARS_ATTAINED"), 5000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_STARS_EVADED"), 4000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_TOTAL_CHASE_TIME"), 1547483647, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_VEHEXPORTED"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_TOTAL_NO_SHOPS_HELD_UP"), 100, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_KILLS_ENEMY_GANG_MEMBERS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_KILLS_FRIENDLY_GANG_MEMBERS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CR_GANGATTACK_CITY"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CR_GANGATTACK_COUNTRY"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CR_GANGATTACK_LOST"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CR_GANGATTACK_VAGOS"), 500, TRUE);
                            STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_MISSIONS_CREATED"), 500, TRUE);
                            STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_LTS_CREATED"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_HORDKILLS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_GHKILLS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_NO_NON_CONTRACT_RACE_WIN"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DB_SHOTTIME"), 596, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DB_KILLS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DB_PLAYER_KILLS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DB_SHOTS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DB_HITS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DB_HITS_PEDS_VEHICLES"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_DB_HEADSHOTS"), 500, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_RANKAP_UNLK_0"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_RANKAP_UNLK_1"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_RANKAP_UNLK_2"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_RANKAP_UNLK_3"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_CREWUNLOCK_1_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_HEIST_1_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_PACKAGE_1_COLLECT"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_PACKAGE_2_COLLECT"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_PACKAGE_3_COLLECT"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_PACKAGE_5_COLLECT"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_PACKAGE_6_COLLECT"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_PACKAGE_7_COLLECT"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_PACKAGE_8_COLLECT"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_NO_FM_PACKAGES_COL"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_BS_TRINKET_SAVED"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_ARN_BS_TRINKET_TICKERS"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_H3_BOARD_DIALOGUE0"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_H3_BOARD_DIALOGUE1"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_H3_BOARD_DIALOGUE2"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_H3_VEHICLESUSED"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_GANGOPS_FLOW_BITSET_MISS0"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LFETIME_HANGAR_BUY_UNDETAK"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_LFETIME_HANGAR_BUY_COMPLET"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AT_FLOW_VEHICLE_BS"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_WVM_FLOW_VEHICLE_BS"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_TATTOO_FM_UNLOCKS_0"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_TATTOO_FM_UNLOCKS_1"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_TATTOO_FM_UNLOCKS_2"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_TATTOO_FM_UNLOCKS_3"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_TATTOO_FM_UNLOCKS_4"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_TATTOO_FM_UNLOCKS_5"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_TATTOO_FM_UNLOCKS_6"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_TATTOO_FM_UNLOCKS_7"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_TATTOO_FM_UNLOCKS_8"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_TATTOO_FM_UNLOCKS_9"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_TATTOO_FM_UNLOCKS_10"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_TATTOO_FM_UNLOCKS_11"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_TATTOO_FM_UNLOCKS_12"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_CLOTHES_1_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_CLOTHES_2_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_CLOTHES_3_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_CLOTHES_4_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_CLOTHES_5_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_CLOTHES_6_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_CLOTHES_7_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_CLOTHES_8_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_CLOTHES_9_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_CLOTHES_10_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_CLOTHES_11_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_CREWUNLOCK_1_FM_EQUIP"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_CREWUNLOCK_2_FM_EQUIP"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_CREWUNLOCK_3_FM_EQUIP"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_CREWUNLOCK_4_FM_EQUIP"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_HAIR_UNLCK1"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_HAIR_UNLCK2"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_HAIR_UNLCK3"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_HAIR_UNLCK4"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_HAIR_UNLCK5"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_HAIR_UNLCK6"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_HAIR_UNLCK7"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_HAIR_UNLCK8"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_HAIR_UNLCK9"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_HAIR_UNLCK10"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_HAIR_UNLCK11"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_HAIR_UNLCK12"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_HEALTH_1_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_HEALTH_2_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_FM_HAIRCUT_1_UNLCK"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_WEAP_UNLOCKED"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_WEAP_UNLOCKED2"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_HEIST_COMPLETION"), 26, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_HEIST_TOTAL_TIME"), 86400000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_HEISTS_ORGANISED"), 50, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CH_ARC_CAB_CLAW_TROPHY"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CH_ARC_CAB_LOVE_TROPHY"), -1, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_CAR_CLUB_MEM"), 1000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_SPRINTRACER"), 250, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_STREETRACER"), 250, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_PURSUITRACER"), 250, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_TEST_CAR"), 1800000, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_CAR_EXPORT"), 100, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_GROUNDWORK"), 250, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_ROBBERY_CONTRACT"), 100, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_FACES_OF_DEATH"), 250, TRUE);
                            STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_AWD_AUTO_SHOP"), 100, TRUE);
                            break;
                        }
                        case 3:
                        {
                            for (int i = 1; i <= 77; i++) {
                                PLAYER::GIVE_ACHIEVEMENT_TO_PLAYER(i);
                                controller::ShowMessage("All Achievements Unlocked!", true);
                            }
                            break;
                        }
                        case 4:
                        {
                            for (int i = 26932; i <= 27506; i++)
                            {
                                *script_global(262145).at(i).as<int*>() = TRUE;
                            }
                            controller::ShowMessage("Shop Unlocked!", true);
                            break;
                        }
                        case 5:
                        {
                            for (int i = 7378; i <= 7457; i++)
                            {
                                controller::set_packed_bool(i, false, -1);
                            }

                            controller::set_packed_bool(7466, false, -1);

                            for (int i = 7621; i <= 7627; i++)
                            {
                                controller::set_packed_bool(i, false, -1);
                            }

                            for (int i = 15441; i <= 15446; i++)
                            {
                                controller::set_packed_bool(i, false, -1);
                            }

                            for (int i = 18100; i <= 18105; i++)
                            {
                                controller::set_packed_bool(i, false, -1);
                            }

                            controller::set_packed_bool(15995, false, -1);
                            controller::set_packed_bool(15548, false, -1);

                            for (int i = 25241; i <= 25243; i++)
                            {
                                controller::set_packed_bool(i, false, -1);
                            }

                            controller::set_packed_bool(25518, false, -1);
                            controller::set_packed_bool(25519, false, -1);
                            STATS::STAT_SAVE(0, 0, 3, 0);
                            controller::ShowMessage("~b~~o~Done! Check your Weapons.", true);
                            break;
                        }
                        case 6:
                        {
                            for (int i = 30817; i <= 30839; i++)
                            {
                                *script_global(262145).at(i).as<bool*>() = true;
                            }
                            controller::ShowMessage("Tuner Costumes Unlocked!", true);
                            break;
                        }
                    }
                });
            }
            ImGui::SameLine();
            if (ImGui::Button(xorstr("Force Cloud Save"))) {
                g_fiber_pool->queue_job([]
                    {
                        STATS::STAT_SAVE(0, 0, 3, 0);
                        controller::ShowMessage("~bold~~g~Character Saved!", true);
                    });
            }

            ImGui::Text(xorstr("Player Level"));
            ImGui::RadioButton(xorstr("R* Gift Admin"), &level_type, 0);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip(xorstr("This method requires changing session."));
            ImGui::SameLine();

            ImGui::RadioButton(xorstr("Char XP FM"), &level_type, 1);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip(xorstr("This method is instant, you don't need to change session."));

            ImGui::SameLine();
            ImGui::RadioButton(xorstr("Char Rank FM"), &level_type, 2);

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip(xorstr("This method is instant, you don't need to change session."));

            ImGui::SameLine();
            ImGui::RadioButton(xorstr("Crew Level"), &level_type, 3);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip(xorstr("This method is instant, you don't need to change session."));

            ImGui::PushItemWidth(100);
            ImGui::InputScalar(xorstr("Rank 1-8000"), ImGuiDataType_U32, &LevelArray);
            ImGui::PopItemWidth();

            if (ImGui::Button(xorstr("Set Level")))
            {
                g_fiber_pool->queue_job([] {
                    const auto mpx = std::to_string(gta_util::get_character());
                    switch (level_type)
                    {
                    case 0:
                        STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_SET_RP_GIFT_ADMIN"), *script_global(g_global.level_array).at(LevelArray, 1).as<int*>(), TRUE);
                        break;
                    case 1:
                        *script_global(g_global.player_exp).at(g_local.character).as<int*>() = *script_global(g_global.level_array).at(LevelArray, 1).as<int*>();//STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_XP_FM"), *script_global(g_global.level_array).at(LevelArray, 1).as<int*>(), TRUE);
                        gta_util::get_local_playerinfo()->m_wanted_level = 1;
                        script::get_current()->yield(100ms);
                        gta_util::get_local_playerinfo()->m_wanted_level = 0;
                        STATS::STAT_SAVE(0, 0, 3, 0);
                        break;
                    case 2:
                        STATS::STAT_SET_INT(rage::joaat("MP" + mpx + "_CHAR_RANK_FM_SA"), LevelArray, TRUE);
                        break;
                    case 3:
                        STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_CREW_LOCAL_XP_0"), *script_global(g_global.level_array).at(LevelArray, 1).as<int*>(), TRUE);
                        STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_CREW_LOCAL_XP_1"), *script_global(g_global.level_array).at(LevelArray, 1).as<int*>(), TRUE);
                        STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_CREW_LOCAL_XP_2"), *script_global(g_global.level_array).at(LevelArray, 1).as<int*>(), TRUE);
                        STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_CREW_LOCAL_XP_3"), *script_global(g_global.level_array).at(LevelArray, 1).as<int*>(), TRUE);
                        STATS::STAT_SET_INT(RAGE_JOAAT("MPPLY_CREW_LOCAL_XP_4"), *script_global(g_global.level_array).at(LevelArray, 1).as<int*>(), TRUE);
                        gta_util::get_local_playerinfo()->m_wanted_level = 1;
                        script::get_current()->yield(100ms);
                        gta_util::get_local_playerinfo()->m_wanted_level = 0;
                        STATS::STAT_SAVE(0, 0, 3, 0);
                        break;
                    }
                });
            }
        ImGui::EndTabItem();
        }
    }
}