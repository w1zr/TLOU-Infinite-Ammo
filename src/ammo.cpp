#include "ammo.h"

#include "game.h"
#include "log.h"

#include <windows.h>

namespace ammo {

namespace {

Config g_cfg;
game::Flags g_flags;
HANDLE g_worker = nullptr;
DWORD g_hookThreadId = 0;
volatile LONG g_stop = 0;
volatile LONG g_toggleInfinite = 0; // set by the keyboard hook, consumed by the worker
volatile LONG g_toggleNoReload = 0;
bool g_wantInfinite = false;
bool g_wantNoReload = false;

bool GameHasFocus() {
    HWND fg = GetForegroundWindow();
    if (!fg) return false;
    DWORD pid = 0;
    GetWindowThreadProcessId(fg, &pid);
    return pid == GetCurrentProcessId();
}

// The game reads the keyboard only through raw input, so neither
// GetAsyncKeyState nor a low-level keyboard hook sees key presses while it has
// focus (both verified in-game). Windows hotkeys are delivered regardless.
// While the game runs the two keys are reserved system-wide; presses made
// while another window is focused are ignored.
enum : int { kHotkeyInfinite = 1, kHotkeyNoReload = 2 };

void RegisterKey(int id, int vk, const char* what) {
    if (vk <= 0) return;
    if (RegisterHotKey(nullptr, id, MOD_NOREPEAT, static_cast<UINT>(vk)))
        logx::Write("hotkey for %s registered (vk 0x%X)", what, vk);
    else
        logx::Write("hotkey for %s unavailable: key 0x%X is taken by another program (%lu)", what, vk, GetLastError());
}

DWORD WINAPI HookThread(LPVOID) {
    RegisterKey(kHotkeyInfinite, g_cfg.toggleInfiniteKey, "InfiniteAmmo");
    RegisterKey(kHotkeyNoReload, g_cfg.toggleNoReloadKey, "NoReload");
    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        if (msg.message != WM_HOTKEY || !GameHasFocus()) continue;
        if (msg.wParam == kHotkeyInfinite) InterlockedExchange(&g_toggleInfinite, 1);
        if (msg.wParam == kHotkeyNoReload) InterlockedExchange(&g_toggleNoReload, 1);
    }
    UnregisterHotKey(nullptr, kHotkeyInfinite);
    UnregisterHotKey(nullptr, kHotkeyNoReload);
    return 0;
}

// The flags are private bytes only the mod writes (see game::Resolve). If the
// player turned on the official modifier themselves, mirror it so its HUD and
// behaviour keep working; that is why this runs every tick.
void Apply() {
    const bool official = game::OfficialModifierOn(g_flags);
    *g_flags.infiniteAmmo = (g_wantInfinite || official) ? 1 : 0;
    *g_flags.bottomless = g_wantInfinite ? 1 : 0;
    if (g_flags.noReload) *g_flags.noReload = g_wantNoReload ? 1 : 0;
}

void Feedback(bool on) {
    if (g_cfg.beepOnToggle) MessageBeep(on ? MB_OK : MB_ICONHAND);
}

DWORD WINAPI Worker(LPVOID) {
    if (!game::Resolve(g_flags)) return 0;
    if (g_cfg.showInfinitySign) game::PatchHud(g_flags);

    g_wantInfinite = g_cfg.infiniteAmmo;
    g_wantNoReload = g_cfg.noReload;
    Apply();
    logx::Write("active: InfiniteAmmo=%d NoReload=%d", g_wantInfinite, g_wantNoReload);

    if (g_cfg.toggleInfiniteKey > 0 || g_cfg.toggleNoReloadKey > 0) {
        HANDLE t = CreateThread(nullptr, 0, HookThread, nullptr, 0, &g_hookThreadId);
        if (t) CloseHandle(t);
    }

    while (!InterlockedCompareExchange(&g_stop, 0, 0)) {
        if (InterlockedExchange(&g_toggleInfinite, 0)) {
            g_wantInfinite = !g_wantInfinite;
            logx::Write("hotkey: InfiniteAmmo=%d", g_wantInfinite);
            Feedback(g_wantInfinite);
        }
        if (InterlockedExchange(&g_toggleNoReload, 0)) {
            g_wantNoReload = !g_wantNoReload;
            logx::Write("hotkey: NoReload=%d", g_wantNoReload);
            Feedback(g_wantNoReload);
        }
        Apply();
        Sleep(100);
    }
    return 0;
}

} // namespace

void Start(const Config& cfg) {
    if (g_worker) return; // never start twice
    g_cfg = cfg;
    g_worker = CreateThread(nullptr, 0, Worker, nullptr, 0, nullptr);
    if (!g_worker) logx::Write("ERROR: could not create worker thread (%lu)", GetLastError());
}

void Stop() {
    // Called from DLL_PROCESS_DETACH (loader lock held): signal only, never wait.
    InterlockedExchange(&g_stop, 1);
    if (g_hookThreadId) PostThreadMessageW(g_hookThreadId, WM_QUIT, 0, 0);
}

} // namespace ammo
