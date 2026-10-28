# The Last of Us Infinite Ammo

**Download:** [Nexus Mods](https://www.nexusmods.com/thelastofuspart1/mods/254) or [GitHub Releases](https://github.com/w1zr/TLOU-Infinite-Ammo/releases) — get `TLOUInfiniteAmmo-v1.0.0.zip`.

Infinite ammo for **The Last of Us Part I** (PC) that keeps normal reloading,
in the style of the Resident Evil 2 "Infinite Ammo" mod. Works without
finishing the game.

- Magazines still empty when you fire, and you reload with the original
  animation, sound and timing.
- Every reload fills the whole magazine, even if you have no ammo at all.
- The HUD shows **∞** for reserve ammo, like the official Gameplay Modifier.
- Throwables (bricks, bottles, molotovs, nail bombs, smoke bombs) and crafting
  are not meant to be affected: the mod only changes the gun fire and reload
  code. This has not been tested in-game yet (see TEST_REPORT.md).
- Optional **No Reload** mode (off by default): your magazine never goes down.
  Player only; enemies still use and reload magazines.
- Hotkeys: **F7** toggles infinite ammo, **F8** toggles No Reload. They only
  react while the game window is focused. The HUD updates on your next shot.

## Supported versions

| Game | Version | Status |
|---|---|---|
| Steam, `tlou-i.exe` | 1.1.4.0 (build 18995818) | Tested in-game |
| Steam, `tlou-i-l.exe` | 1.1.4.0 | Offline signature tests only |
| Epic Games Store | — | Not tested. Should work if the executable matches; the mod disables itself safely if it doesn't |

The mod finds the code it needs by byte pattern when the game starts. If a game
update changes that code, it writes the reason to `TLOUInfiniteAmmo.log` and
does nothing, instead of writing to the wrong place.

## Install with Fluffy Mod Manager (recommended)

1. Copy `TLOUInfiniteAmmo-v1.0.0.zip` into
   `Fluffy Mod Manager\Games\TLOU\Mods\` (or use Fluffy's own import).
2. Open Fluffy, choose The Last of Us Part I, and tick **TLOU Infinite Ammo**.
3. Fluffy copies `version.dll`, `TLOUInfiniteAmmo.ini`,
   `TLOUInfiniteAmmo.log` and `TLOUInfiniteAmmo-README.txt` next to
   `tlou-i.exe` and records them.
4. Start the game. Install while the game is closed.

No third-party loader is needed: the game loads `version.dll` from its own
folder before the Windows copy, and the mod forwards every `version.dll`
function to the real System32 DLL.

## Install manually (without Fluffy)

1. Open `TLOUInfiniteAmmo-v1.0.0.zip`.
2. Copy `version.dll` and `TLOUInfiniteAmmo.ini` into the game folder (the
   folder with `tlou-i.exe`), e.g.
   `C:\Program Files (x86)\Steam\steamapps\common\The Last of Us Part I\`.
3. Start the game.

`modinfo.ini` and `screenshot.png` are only used by Fluffy; you don't need to
copy them. If a `version.dll` already exists in the game folder, it belongs to
another mod: back it up first. Only one of them can be loaded this way.

## Uninstall

- **Fluffy:** untick the mod, or use "Uninstall all mods". Fluffy deletes the
  four files it installed.
- **Manual:** delete `version.dll`, `TLOUInfiniteAmmo.ini` and
  `TLOUInfiniteAmmo.log` from the game folder (and
  `TLOUInfiniteAmmo-README.txt` if you copied it).

The mod never writes to game files or saves, so nothing else needs undoing.

## Settings (`TLOUInfiniteAmmo.ini`)

| Key | Default | Meaning |
|---|---|---|
| `InfiniteAmmo` | 1 | Infinite ammo with normal reloads |
| `ShowInfinitySign` | 1 | HUD shows ∞ instead of the reserve count |
| `NoReload` | 0 | Magazine never goes down (player only) |
| `ToggleInfiniteAmmoKey` | 118 (F7) | Virtual-key code, 0 = off |
| `ToggleNoReloadKey` | 119 (F8) | Virtual-key code, 0 = off |
| `BeepOnToggle` | 1 | Windows sound on hotkey toggle |
| `Logging` | 1 | Write `TLOUInfiniteAmmo.log` |

## Verify it is working

1. Open `TLOUInfiniteAmmo.log` in the game folder. A working install ends with:
   ```
   flag reads found: InfiniteAmmo 2, Bottomless 5
   NoReload hook installed at RVA 0x1DC163F (player only)
   HUD patched at 5 sites: ...
   active: InfiniteAmmo=1 NoReload=0
   ```
2. In-game, the reserve count shows ∞. Fire until the magazine is empty and
   reload: the magazine refills completely.

## Troubleshooting

| Symptom | Cause / fix |
|---|---|
| No `TLOUInfiniteAmmo.log` appears | `version.dll` is not in the folder with `tlou-i.exe`, or another mod's `version.dll` replaced it |
| Log says `signature matched 0 times` or `Mod disabled` | Game version not supported; the mod stayed off on purpose |
| Log says `hotkey ... taken by another program` | Another app uses F7/F8 globally; change the keys in the ini |
| F7/F8 seem to do nothing | The HUD only redraws on the next shot; also the game window must be focused |
| HUD shows numbers instead of ∞ | `ShowInfinitySign=0`, or the log says the HUD sites weren't found (ammo is still infinite) |

## Compatibility

- **Saves:** the mod never writes save files. Saving with it on is safe;
  removing it returns normal ammo rules.
- **Fluffy asset mods** (outfits, textures, audio, `bin/dc1` data mods):
  compatible — this mod changes no game files.
- **Other `version.dll` mods:** cannot be installed at the same time (same
  file name). Fluffy will report the file conflict.
- **"Infinite Ammo - No Reload" (Nexus 224), trainers, Cheat Engine tables
  touching ammo:** not tested together; don't run two ammo mods at once.
- **Dev Menu exe replacements:** untested; the mod locates code by pattern and
  disables itself if the patterns are not found.
- **Official Gameplay Modifier "Infinite Ammo":** works alongside; the mod
  mirrors it.

## How it works (for modders)

The engine already contains infinite-ammo logic (used by the official
modifier). `IsInfiniteAmmoActive()` returns *debug flag OR modifier*, and the
reload code has a second "bottomless reserve" debug flag. The game resets its
debug flag block on its own, so instead of writing those flags the mod
redirects the 2 + 5 compares that read them to private bytes it owns
(same-length disp32 rewrites). The HUD's 5 modifier checks are rewritten the
same way. No Reload is a 7-byte detour on the magazine store in the shared
weapon fire routine that skips the store only when the weapon's owner is the
player (using the engine's own owner/player type check).

## Building from source

Requirements: Windows, Visual Studio 2022 Build Tools with "Desktop
development with C++" (MSVC v143, Windows SDK). No other dependencies.

```bat
build.bat
build\scan_test.exe "C:\...\The Last of Us Part I\tlou-i.exe" "C:\...\tlou-i-l.exe"
```

`build.bat` produces `build\version.dll` (the mod) and `build\scan_test.exe`,
which checks every signature, the HUD rewrites and the No Reload detour
against real game executables without running the game. `package.bat` then
builds `release\TLOUInfiniteAmmo-v1.0.0.zip` from `build\version.dll`,
`dist\TLOUInfiniteAmmo.ini` and the Fluffy files in `package\fluffy`.

Source layout: `src/dllmain.cpp` (startup), `src/proxy_version.cpp` +
`version.def` (loader shim), `src/scan.*` (patterns and patch builders, no
Windows dependencies), `src/game.*` (game integration and patching),
`src/ammo.*` (ammo state and hotkeys), `src/config.*`, `src/log.*`.

## License

MIT — see [LICENSE](LICENSE). Not affiliated with Naughty Dog or Sony
Interactive Entertainment. This repository contains no game files.
