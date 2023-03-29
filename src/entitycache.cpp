/*
 * entitycache.cpp
 *
 *  Created on: Nov 7, 2016
 *      Author: nullifiedcat
 */

#include "common.hpp"

#include <time.h>
#include <settings/Float.hpp>
#include "soundcache.hpp"

bool IsProjectileACrit(CachedEntity *ent)
{
    if (ent->m_bGrenadeProjectile())
        return CE_BYTE(ent, netvar.Grenade_bCritical);
    return CE_BYTE(ent, netvar.Rocket_bCritical);
}
// This method of const'ing the index is weird.
CachedEntity::CachedEntity() : m_IDX(int(((unsigned) this - (unsigned) &entity_cache::array) / sizeof(CachedEntity))), hitboxes(hitbox_cache::Get(unsigned(m_IDX)))
{
#if PROXY_ENTITY != true
    m_pEntity = nullptr;
#endif
    m_fLastUpdate = 0.0f;
}

void CachedEntity::Reset()
{
    m_bAnyHitboxVisible = false;
    m_bVisCheckComplete = false;
    m_lLastSeen         = 0;
    m_lSeenTicks        = 0;
    memset(&player_info, 0, sizeof(player_info_s));
    m_vecAcceleration.Zero();
    m_vecVOrigin.Zero();
    m_vecVelocity.Zero();
    m_fLastUpdate = 0;
}

CachedEntity::~CachedEntity()
{
}

static settings::Float ve_window{ "debug.ve.window", "0" };
static settings::Boolean ve_smooth{ "debug.ve.smooth", "true" };
static settings::Int ve_averager_size{ "debug.ve.averaging", "0" };

void CachedEntity::Update()
{
#if PROXY_ENTITY != true
    m_pEntity = g_IEntityList->GetClientEntity(idx);
    if (!m_pEntity)
    {
        return;
    }
#endif
    m_lLastSeen = 0;
    hitboxes.InvalidateCache();
    m_bVisCheckComplete = false;
}

bool CachedEntity::IsVisible()
{
    PROF_SECTION(CE_IsVisible)
    if (m_bVisCheckComplete)
        return m_bAnyHitboxVisible;

    auto hitbox = hitboxes.GetHitbox(std::max(0, (hitboxes.GetNumHitboxes() >> 1) - 1));
    Vector result;
    if (!hitbox)
        result = m_vecOrigin();
    else
        result = hitbox->center;
    // Just check a centered hitbox. This is mostly used for ESP anyway
    if (IsEntityVectorVisible(this, result, true, MASK_SHOT_HULL, nullptr))
    {
        m_bAnyHitboxVisible = true;
        m_bVisCheckComplete = true;
        return true;
    }

    m_bAnyHitboxVisible = false;
    m_bVisCheckComplete = true;

    return false;
}

std::optional<Vector> CachedEntity::m_vecDormantOrigin()
{
    if (!RAW_ENT(this)->IsDormant())
        return m_vecOrigin();
    auto vec = soundcache::GetSoundLocation(this->m_IDX);
    if (vec)
        return *vec;
    return std::nullopt;
}

namespace entity_cache
{

CachedEntity array[MAX_ENTITIES]{};
std::vector<CachedEntity *> valid_ents;

void Update()
{
    max = g_IEntityList->GetHighestEntityIndex();
    valid_ents.clear(); // Reserving isn't necessary as this doesn't reallocate it
    if (max >= MAX_ENTITIES)
        max = MAX_ENTITIES - 1;
    
    // pre-allocate memory
    valid_ents.reserve(max);
    
    for (int i = 0; i <= max; i++)
    {
        array[i].Update();
        if (CE_GOOD((&array[i])))
        {
            array[i].hitboxes.UpdateBones();
            valid_ents.push_back(&array[i]);
        }
    }
}

void Invalidate()
{
    for (auto &ent : array)
    {
        // pMuch useless line!
        // ent.m_pEntity = nullptr;
        ent.Reset();
    }
}

void Shutdown()
{
    for (auto &ent : array)
    {
        ent.Reset();
        ent.hitboxes.Reset();
    }
}

int max = 0;
} // namespace entity_cache
