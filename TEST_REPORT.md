# Test report — TLOU Infinite Ammo 1.0.0

**Environment:** Windows 11 Home 10.0.26300, The Last of Us Part I Steam build
18995818 (`tlou-i.exe` file version 1.1.4.0, image timestamp 0x685BEBE1),
Fluffy Mod Manager 3.016 with three other mods installed (Ellie Clothing
collection, Ellie Left Behind Shorts Outfit, Must Install First).
Built with MSVC v143 (VS 2022 Build Tools). Date: 2026-10-28.

In-game tests used **Encounters → Pittsburgh → Underpass** (Joel, Very Light),
which writes no story saves. "Not tested" means the test was not performed.

## Automated (offline, `scan_test.exe`)

Run against `tlou-i.exe` and `tlou-i-l.exe` from the install above.

| Check | tlou-i.exe | tlou-i-l.exe |
|---|---|---|
| Infinite-ammo check and debug flags located; match manual disassembly | Pass | Pass |
| Bottomless flag adjacent; options pointer and modifier offset (0x4968) | Pass | Pass |
| 5 HUD sites found; rewrites same length and target the flag | Pass | Pass |
| Flag reads to redirect: InfiniteAmmo 2, Bottomless 5 | Pass | Pass |
| Magazine store found; fire routine calls the same check | Pass | Pass |
| No Reload stub/jump encodings land on the right addresses | Pass | Pass |
| Damaged flag layout rejected | Pass | Pass |
| Missing signature fails safely | Pass | Pass |

## In-game

| # | Test | Expected | Result |
|---|---|---|---|
| 1 | Fire with reserve ammo | Magazine goes down | **Pass** (8 → 6 → 4) |
| 2 | Fire until magazine empty | Magazine reaches 0 | **Pass** |
| 3 | Reload empty weapon | Original animation, full magazine | **Pass** (0 → 8) |
| 4 | Fire again after reload | Works | **Pass** |
| 5 | Several reload cycles | Never runs out | **Pass** |
| 6 | Switch weapons and back | Normal | **Pass** |
| 7 | Reload with zero total ammo | Full magazine | **Pass** (0 \| ∞ → 8) |
| 8 | Pistol, revolver, hunting rifle, shotgun | Infinite with reloads | **Pass** (user confirmed all guns) |
| 9 | Bow, flamethrower | — | **Not tested** (not available in unlocked Encounters) |
| 10 | Ellie as playable character | — | **Not tested** (no later save/encounter used) |
| 11 | Throwables / crafting unchanged | Normal consumption | **Not tested** |
| 12 | HUD shows ∞ | ∞ for reserve | **Pass** |
| 13 | F7 off / on | Real reserve shown and used; ∞ returns | **Pass** (4 \| 15, then 7 \| ∞) |
| 14 | F8 No Reload on / off | Magazine stays; then drops again | **Pass** (12 → 12 after 5 shots; 12 → 10) |
| 15 | Enemies unaffected by No Reload | Enemies reload | **Not observed directly** (follows from the player check in code) |
| 16 | Death and checkpoint restart | Mod keeps working | **Pass** |
| 17 | Story save / load | — | **Not tested** (by choice, to protect the user's saves) |
| 18 | Damage, recoil, fire rate unchanged | Unchanged | **Pass by design** — no damage/recoil/rate code touched; not measured |
| 19 | Starting magazine size with vs without mod | Same | **Pass** (6 and 6, fresh Encounter) — see known issues |
| 20 | Alongside 3 Fluffy asset mods | No conflict | **Pass** |

## Fluffy Mod Manager

| Step | Result |
|---|---|
| Zip recognised; name, description, author, version, preview shown | **Pass** |
| Tick to install | **Pass** — "Copied 2 files", files next to `tlou-i.exe`, recorded in `installed.ini` |
| Game loads the Fluffy-installed mod | **Pass** |
| Untick / uninstall removes files | **Pass** — "Deleted 2 files", game folder clean, record removed from `installed.ini` |
| Re-enable after disable | **Not tested** |

The test build's zip did not contain the log file, so the log was left behind
after uninstall and removed by hand. The release zip ships an empty
`TLOUInfiniteAmmo.log` and `TLOUInfiniteAmmo-README.txt`, so Fluffy tracks and
removes all four files; the release zip itself has not been through a Fluffy
install/uninstall cycle yet.

## Known issues / limitations

- During testing the pistol's magazine size was seen at 8 and 12 in some
  Encounter runs. A clean run measured 6 with and without the mod, and the
  user later reported everything working. Cause not identified; the mod has no
  code path that changes magazine capacity.
- The HUD redraws ammo only when it changes, so F7/F8 show on the next shot.
- F7/F8 are reserved system-wide while the game runs (Windows hotkeys).
- Epic Games Store build not tested.
