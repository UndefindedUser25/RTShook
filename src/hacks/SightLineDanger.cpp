/*
 * SightLineDanger.cpp
 *
 *  Created on: Dec 6, 2022 9:52 PM
 *      Author: Spice
 */

#include "common.hpp"
#include <PlayerTools.hpp>
#include <settings/String.hpp>
#include <settings/Int.hpp>

namespace sight_danger
{
static settings::Boolean dangercheck{ "danger.enable", "true" };
static settings::Int draw_string_x{ "danger.draw-info.x", "1500" };
static settings::Int draw_string_y{ "danger.draw-info.y", "600" };
static settings::Int fov1{ "danger.fov-looking-at", "15" };
static settings::Int fov2{ "danger.fov-can-see", "90" };
static settings::Int fov3{ "danger.fov-visible", "180" };
#if ENABLE_VISUALS
static std::array<std::string, 32> danger_strings;
static size_t danger_strings_count{ 0 };
static std::array<rgba_t, 32> danger_strings_colors{ colors::empty };
void AddDangerString(const std::string &string, const rgba_t &color)
{
    danger_strings[danger_strings_count]        = string;
    danger_strings_colors[danger_strings_count] = color;
    ++danger_strings_count;
}
void DrawDangerStrings()
{
    float x = *draw_string_x;
    float y = *draw_string_y;
    for (size_t i = 0; i < danger_strings_count; ++i)
    {
        float sx, sy;
        fonts::menu->stringSize(danger_strings[i], &sx, &sy);
        draw::String(x, y, danger_strings_colors[i], danger_strings[i].c_str(), *fonts::center_screen);
        y += fonts::center_screen->size + 1;
    }
    danger_strings_count = 0;
}
static void FOVCheck()
    {
    if (!dangercheck)
        return;
    if (!g_IEngine->IsInGame())
        return;
    if (CE_BAD(LOCAL_E))
        return;
    if (!LOCAL_E->m_bAlivePlayer())
        return;
    for (auto &ent : entity_cache::valid_ents)
        {
            if (CE_BAD(ent))
                continue;
            if (!ent->m_bAlivePlayer() || !ent->m_bEnemy())
                continue;
            if (!player_tools::shouldTarget(ent))
                continue;
                player_info_s info{};
            if (!GetPlayerInfo(ent->m_IDX, &info)/* || !info.friendsID*/)
                continue;
            else
            {
                rgba_t color = colors::green;
                std::string name(info.name);
                auto target = ent->m_vecOrigin();
                auto local_origin = g_pLocalPlayer->v_Origin;
                GetHitbox(ent, head, target);
                GetHitbox(LOCAL_E, head, local_origin);
                if (IsVectorVisible(local_origin, target, true))
                {
                    if (GetFov(ent->m_vecAngle(), ent->hitboxes.GetHitbox(head)->center, LOCAL_E->hitboxes.GetHitbox(head)->center) < *fov1)
                    {
                        color = colors::red;
                        AddDangerString((name) + " is looking at you!", color);
                    }
                    else if (GetFov(ent->m_vecAngle(), ent->hitboxes.GetHitbox(head)->center, LOCAL_E->hitboxes.GetHitbox(head)->center) < *fov2)
                    {
                        color = colors::yellow;
                        AddDangerString((name) + " can see you", color);
                    }
                    else if  (GetFov(ent->m_vecAngle(), ent->hitboxes.GetHitbox(head)->center, LOCAL_E->hitboxes.GetHitbox(head)->center) < *fov3)
                    {
                        color = colors::green;
                        AddDangerString((name) + " is visible", color);
                    }
                }
            }
        }
        DrawDangerStrings();
    }
static InitRoutine EC(
    []()
    {
        EC::Register(EC::Draw, FOVCheck, "DRAW_Danger", EC::average);
    });
#endif
} // namespace sight_danger