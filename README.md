# RGB Matrix Panel Wall Art Project

This repository started from the `RGBmatrixPanel` codebase, but the active work in this project is focused on driving a `32x16` `1/4-scan` P10 HUB75 panel from an ESP32 using DMA.

The sketch currently being used for development is [examples/AAAthiago_testing/AAAthiago_testing.ino](/c:/Users/costatf/Desktop/RGBMatrixPanel-for-1-4-scan-main/RGB_matrix_Panel/examples/AAAthiago_testing/AAAthiago_testing.ino).

## What library stack this project is using

The panel control code in the active sketch uses:

- `ESP32-HUB75-MatrixPanel-I2S-DMA`
- `ESP32-VirtualMatrixPanel-I2S-DMA`

Those libraries are used to:

- drive HUB75 panels from the ESP32 with DMA
- support non-standard scan layouts such as `32x16 1/4-scan` outdoor-style panels
- provide a virtual pixel layer so the sketch can work in normal `x,y` screen coordinates

This means the sketch is not currently using the original Adafruit `RGBmatrixPanel` API as its main rendering path, even though this repository still contains the original library sources.

## Hardware target

Current target panel:

- `32x16` P10 RGB panel
- `1/4 scan`
- HUB75 interface
- ESP32-based controller

The panel is configured in the DMA layer as `64x8`, then mapped back to `32x16` in the virtual display layer. That is required for this specific style of `1/4-scan` panel.

## What changes were made

The main work so far has been in the development sketch:

- corrected the ESP32 DMA panel configuration for a single `32x16 1/4-scan` panel
- switched the virtual panel setup to a proper `1x1` panel layout
- used `FOUR_SCAN_16PX_HIGH` for the physical scan type
- added serial-driven calibration tools to inspect row and pixel placement
- measured the actual panel behavior and derived a custom remap based on the observed scan pattern
- built an inverse lookup table so logical screen coordinates can be drawn in normal order
- routed custom drawing through `drawPixelMapped()` so effects render correctly on this panel
- added a lightweight remapped `5x7` text renderer for letters and numbers

In practice, the key project-specific fix is the custom mapping layer on top of the DMA and virtual panel libraries. Without that extra remap, the panel lights valid pixels, but the visible order on this particular module is scrambled.

## Functionality available so far

The current sketch supports serial-controlled testing and visual demos.

### Calibration and diagnostics

- `h` or `help`
- `corners`
- `row`
- `col <y>`
- `split <y>`
- `px <x> <y>`
- `sweep <ms>`
- `clear`
- `stop`

These commands are used to:

- verify mapping
- inspect single pixels
- step through rows and columns
- confirm corrected screen orientation

### Display features and demos

- `text <msg>` displays remapped text using the built-in `5x7` font
- `aurora` runs an aurora-style animation
- `life` runs a rainbow Conway's Game of Life
- `rainbow` runs a rainbow wave effect
- `matrix` runs a green matrix-rain effect
- `fire` runs a flame effect
- `plasma` runs a colorful plasma effect
- `twinkle` runs a starfield-style sparkle effect

All of the custom pixel-based effects above render through the corrected mapping layer so they appear in proper screen order on this panel.

## How to use the current sketch

1. Open [examples/AAAthiago_testing/AAAthiago_testing.ino](/c:/Users/costatf/Desktop/RGBMatrixPanel-for-1-4-scan-main/RGB_matrix_Panel/examples/AAAthiago_testing/AAAthiago_testing.ino).
2. Build and upload it to the ESP32.
3. Open the Serial Monitor at `115200`.
4. Send commands such as `text 123`, `aurora`, `matrix`, or `plasma`.
5. Send `stop` to exit an animation mode.

## Project direction

The longer-term goal is to use several of these LED matrices as wall-mounted decor.

The idea is to build a modular animated wall piece that can:

- display ambient motion graphics
- show scrolling text, symbols, or numbers
- cycle through reactive-looking visual effects
- eventually scale from a single test panel to multiple panels arranged together on a wall

The current single-panel work is the foundation for that larger installation. Before chaining multiple panels together, this project first needs:

- reliable pixel mapping on one panel
- reusable remapped drawing helpers
- a small library of visual modes that look good at low resolution

Once that is stable, the next phase is to expand to multiple panels and treat them as a larger decorative display surface.

## Notes

- The original repository contents are still present, but the active sketch is now effectively an ESP32 HUB75 experiment and wall-art prototype.
- The remapped text renderer currently uses a small built-in font rather than the normal Adafruit GFX text path.
- Additional cleanup may be useful later if this project is turned into a dedicated multi-panel display app rather than a testbed.
