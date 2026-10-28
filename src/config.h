// Settings read from TLOUInfiniteAmmo.ini next to the game executable.
// A missing file or key falls back to the defaults below.
#pragma once

struct Config {
    bool infiniteAmmo = true;   // reserve never runs out, reloads stay normal
    bool noReload = false;      // optional: engine "No Reload" debug behaviour
    int toggleInfiniteKey = 0x76; // VK_F7, 0 = no hotkey
    int toggleNoReloadKey = 0x77; // VK_F8, 0 = no hotkey
    bool beepOnToggle = true;
    bool showInfinitySign = true; // HUD reserve count shows the infinity sign
    bool logging = true;
};

Config LoadConfig(const wchar_t* iniPath);
