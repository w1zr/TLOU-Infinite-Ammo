// Ammunition handling. The mod does not touch ammo counts itself: it turns on
// the engine's own infinite-ammo path (the same one the Extras > Gameplay
// Modifiers > Infinite Ammo option uses), which leaves magazine use and
// reloading intact and credits each fired round back to the reserve.
#pragma once
#include "config.h"

namespace ammo {
void Start(const Config& cfg);
void Stop();
} // namespace ammo
