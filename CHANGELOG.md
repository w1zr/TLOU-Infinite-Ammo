# Changelog

## 1.0.0 — 2026-10-28

First release.

- Infinite ammo with normal reloads, using the engine's own infinite-ammo
  logic; works without finishing the game.
- Every reload fills the full magazine, including from zero ammo.
- HUD shows ∞ for reserve ammo (`ShowInfinitySign`).
- Optional player-only No Reload mode (`NoReload`, F8).
- F7/F8 hotkeys via Windows hotkeys (the game's raw keyboard input hides key
  presses from other methods).
- Loads as a `version.dll` proxy; no third-party loader.
- Signature-based; disables itself with a logged reason on unknown game builds.
- Fluffy Mod Manager package with `modinfo.ini` and preview image.

### Fixed during development

- Writing the engine's debug flags did not stick (the game resets that block);
  flag reads are now redirected to private bytes.
- The engine's own "No Reload" debug flag does not stop magazine use; replaced
  with a player-only detour.
- Log file is opened shared so it can be read while the game runs.
