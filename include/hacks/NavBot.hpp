#pragma once

#include <array>
#include <stdint.h>

namespace hacks::tf2::NavBot
{
extern bool isVisible;
std::pair<CachedEntity *, float> getNearestPlayerDistance();
} // namespace hacks::tf2::NavBot
