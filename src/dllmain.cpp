#include "ammo.h"
#include "config.h"
#include "game.h"
#include "log.h"

#include <windows.h>

#define MOD_VERSION "1.0.0"

namespace {

// Builds "<game folder>\<file>" from the host executable's path.
bool GamePath(const wchar_t* file, wchar_t* out, DWORD cap) {
    const DWORD n = GetModuleFileNameW(nullptr, out, cap);
    if (n == 0 || n >= cap) return false;
    wchar_t* slash = wcsrchr(out, L'\\');
    if (!slash) return false;
    slash[1] = 0;
    return wcscat_s(out, cap, file) == 0;
}

void Init() {
    // version.dll is loaded by other processes too (launchers, crash handler).
    if (!game::IsSupportedHost()) return;

    wchar_t ini[MAX_PATH], log[MAX_PATH];
    if (!GamePath(L"TLOUInfiniteAmmo.ini", ini, MAX_PATH) || !GamePath(L"TLOUInfiniteAmmo.log", log, MAX_PATH)) return;

    const Config cfg = LoadConfig(ini);
    logx::Open(log, cfg.logging);
    logx::Write("TLOU Infinite Ammo " MOD_VERSION " loaded");
    logx::Write("config: InfiniteAmmo=%d NoReload=%d ToggleInfiniteAmmoKey=0x%X ToggleNoReloadKey=0x%X",
                cfg.infiniteAmmo, cfg.noReload, cfg.toggleInfiniteKey, cfg.toggleNoReloadKey);
    ammo::Start(cfg);
}

} // namespace

BOOL WINAPI DllMain(HINSTANCE inst, DWORD reason, LPVOID) {
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(inst);
        Init();
        break;
    case DLL_PROCESS_DETACH:
        ammo::Stop();
        break;
    }
    return TRUE;
}
