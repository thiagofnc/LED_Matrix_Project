# AAAthiago testing sketch

This Arduino sketch is split into tabs by responsibility. Arduino compiles all
of the `.ino` files in this directory as one sketch, so this organization does
not change the command interface or runtime behavior.

- `AAAthiago_testing.ino` — hardware configuration, shared state, `setup()`, and
  `loop()`
- `01_CoreRendering.ino` — clock helpers, panel remapping, drawing primitives,
  fonts, text, emoji, and common visual helpers
- `02_VisualEffects.ino` — the first group of standalone animation effects
- `03_Games.ino` — Pac-Man and the interactive games
- `04_MoreEffects.ino` — additional geometric, television, audio, and cat effects
- `05_CommandsAndDiagnostics.ino` — help text, Serial command dispatch, and
  hardware diagnostic patterns

When adding a mode, keep its state near the shared declarations, put its frame
renderer in the closest feature tab, and connect it through the command handler
and main `loop()` dispatch.
