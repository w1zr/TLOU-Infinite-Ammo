#include "config.h"

#include <windows.h>

namespace {

bool ReadBool(const wchar_t* ini, const wchar_t* key, bool def) {
    return GetPrivateProfileIntW(L"InfiniteAmmo", key, def ? 1 : 0, ini) != 0;
}

int ReadInt(const wchar_t* ini, const wchar_t* key, int def) {
    return static_cast<int>(GetPrivateProfileIntW(L"InfiniteAmmo", key, def, ini));
}

} // namespace

Config LoadConfig(const wchar_t* ini) {
    Config c;
    c.infiniteAmmo = ReadBool(ini, L"InfiniteAmmo", c.infiniteAmmo);
    c.noReload = ReadBool(ini, L"NoReload", c.noReload);
    c.toggleInfiniteKey = ReadInt(ini, L"ToggleInfiniteAmmoKey", c.toggleInfiniteKey);
    c.toggleNoReloadKey = ReadInt(ini, L"ToggleNoReloadKey", c.toggleNoReloadKey);
    c.beepOnToggle = ReadBool(ini, L"BeepOnToggle", c.beepOnToggle);
    c.showInfinitySign = ReadBool(ini, L"ShowInfinitySign", c.showInfinitySign);
    c.logging = ReadBool(ini, L"Logging", c.logging);
    return c;
}
