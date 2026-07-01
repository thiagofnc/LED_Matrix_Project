# RGB Matrix Wall Art Prototype

This project is an ESP32-driven HUB75 LED matrix prototype built around a `32x16` `1/4-scan` P10 panel. The immediate goal is to make one difficult outdoor-style panel behave like a normal display. The longer-term goal is to use several of these panels together as animated wall decor.

The active sketch is:

- [examples/AAAthiago_testing/AAAthiago_testing.ino](/c:/Users/costatf/Desktop/RGBMatrixPanel-for-1-4-scan-main/RGB_matrix_Panel/examples/AAAthiago_testing/AAAthiago_testing.ino)

## Library Stack

The current sketch uses these libraries:

- `ESP32-HUB75-MatrixPanel-I2S-DMA`
- `ESP32-VirtualMatrixPanel-I2S-DMA`

Why this stack is used:

- it supports ESP32 DMA output for HUB75 panels
- it can drive panels that need virtual remapping
- it provides a way to handle non-standard scan patterns such as `32x16 1/4-scan`

This repository still contains the original Adafruit `RGBmatrixPanel` sources, but the active sketch is not using that API as the main rendering path.

## Hardware

Current hardware target:

- ESP32 controller
- HUB75 `32x16` P10 RGB panel
- `1/4 scan`

Current pin mapping in the sketch:

- `R0=35`
- `G0=21`
- `B0=41`
- `R1=36`
- `G1=12`
- `B1=39`
- `A=48`
- `B=19`
- `C=5`
- `D=17`
- `E=-1`
- `LAT=4`
- `OE=15`
- `CLK=16`

## Core Panel Setup

For this panel, the DMA layer is configured as:

- width `64`
- height `8`

even though the physical panel is:

- width `32`
- height `16`

That `64x8` DMA configuration is required for this style of `32x16 1/4-scan` panel. The sketch then uses `VirtualMatrixPanel` and `FOUR_SCAN_16PX_HIGH` to translate back toward normal `32x16` coordinates.

## What Was Changed

Most of the project-specific work has been done inside the test sketch.

Major changes made so far:

- corrected the DMA geometry for a single `32x16 1/4-scan` panel
- changed the virtual panel configuration to `1x1`
- enabled `FOUR_SCAN_16PX_HIGH`
- removed an earlier manual remap that was fighting with the virtual panel remap
- added serial-driven diagnostics to inspect rows, columns, chunks, and single pixels
- measured the panel’s actual scan behavior from observed output
- derived a custom mapping model for the panel’s non-linear layout
- added an inverse lookup table so drawing can happen in normal screen coordinates
- routed most custom drawing through `drawPixelMapped()`
- added a rotated portrait-style `16x32` character mode
- added a custom `5x7` font renderer for text and numbers
- exposed panel timing settings in the sketch with:
  - `PANEL_I2S_SPEED`
  - `PANEL_CLK_PHASE`

## Mapping Investigation

This panel did not behave like a normal `32x16` display. Early tests showed:

- colors appeared
- pixels lit up
- the sweep covered the whole panel
- but pixel order looked scrambled

To diagnose that, the sketch was extended with tools to test:

- corners
- full rows
- individual column sweeps
- split rows
- color-coded `8-pixel` chunks
- individual chunks
- individual pixels inside a chunk

From those tests, the panel was found to behave in a structured but unusual way:

- it operates in grouped scan regions rather than straightforward row order
- `8-pixel` chunks do not all map the same way
- the right half of the panel shows different behavior from the left half

### Mapping Work That Was Attempted

The project currently includes:

- a derived mapping function in `builtInMapPrediction()`
- an inverse map built by `buildInverseMap()`
- a drawing helper `drawPixelMapped()`

The sketch also keeps an earlier derived mapping formula commented in the source so it is easy to compare revisions while debugging.

### Current Status Of The Mapping

The mapping is significantly better than the initial scrambled output, but it is not yet perfect.

What is working well:

- most pixel-based effects render in a coherent orientation
- custom text rendering works through the remapped path
- portrait single-character mode works well enough to be useful for testing

What is still imperfect:

- the right half of the panel still shows visible glitches
- some chunk tests show overlap or cross-coupling on edge pixels
- some artifacts appear more like timing or ghosting than pure XY remap errors

Because of that, the sketch now also exposes timing-related settings to tune the panel:

- `PANEL_I2S_SPEED`
- `PANEL_CLK_PHASE`

At this point the project is dealing with two overlapping problems:

- non-standard logical pixel mapping
- possible right-edge timing or ghosting behavior

## Current Features

The sketch now supports both diagnostics and visual demos from the Serial Monitor.

Open the Serial Monitor at:

- `115200`

Menu commands are submitted as full lines. Once a game starts, the sketch reads each serial byte immediately. Use a raw serial terminal such as PuTTY or Tera Term for one-key controls; Arduino IDE's Serial Monitor still buffers typed input until Enter is pressed.

### Diagnostic Commands

- `h`
- `help`
- `corners`
- `row`
- `col <y>`
- `split <y>`
- `chunks <y>`
- `chunk <y> <n>`
- `chunkpx <y> <n> <o>`
- `px <x> <y>`
- `sweep <ms>`
- `clear`
- `stop`
- `next`

What these are for:

- `corners` checks display orientation
- `row` steps one logical row at a time
- `col <y>` steps across one row pixel by pixel
- `split <y>` shows the left half and right half of a row in different colors
- `chunks <y>` color-codes four `8-pixel` row chunks
- `chunk <y> <n>` isolates one `8-pixel` chunk
- `chunkpx <y> <n> <o>` isolates one pixel inside a chunk
- `px <x> <y>` shows one logical pixel
- `sweep <ms>` sweeps all pixels with a configurable delay

### Text And Character Commands

- `text <msg>`
- `scroll <msg>`
- `draw <msg>`
- `emoji [delay-ms]`
- `rchar <c>`
- `rchartest <ms>`

What they do:

- `text <msg>` renders remapped text using a built-in `5x7` font
- `scroll <msg>` continuously scrolls the message from right to left
- `draw <msg>` draws each message character one pixel at a time
- `emoji [delay-ms]` cycles through smile, heart, wink, and surprised emojis (default `1000` ms)
- `rchar <c>` draws one large character on a rotated portrait-style `16x32` view
- `rchartest <ms>` cycles through all supported rotated characters with a delay

Supported glyph set for the built-in font:

- space
- `! - . :`
- `0-9`
- `?`
- `A-Z`

Lowercase letters are converted to uppercase.

### Clock Commands

- `pclock`
- `settime <hh:mm>`
- `settime <hh:mm:ss>`

How the clock currently works:

- the sketch uses the ESP32 system clock
- on startup, the clock is initialized from the sketch build time
- after that, the ESP32 keeps time internally while it remains powered
- if the displayed time is wrong, it can be corrected manually with `settime`
- `pclock` shows the current device time on top of the plasma background

Important limitation:

- this is not yet synchronized from Wi-Fi, NTP, or an RTC module
- after a reset or new upload, the clock starts from the build/upload time until it is corrected

### Animation Commands

- `aurora`
- `life`
- `rainbow`
- `matrix`
- `fire`
- `plasma`
- `twinkle`
- `supernova`
- `vortex`
- `lava`
- `meteor`
- `storm`
- `galaxy`
- `catrun`
- `squareburst`
- `smoke`
- `blobs`
- `windfire`
- `ripples`
- `circuit`
- `radar`
- `stylerain`
- `neural`
- `tunnel`
- `cube`
- `kaleido`
- `spiral`
- `interfere`
- `tessellate`
- `beatburst`
- `sunrise`
- `pacman`
- `sphere`
- `parallax`
- `moire`
- `hypnosis`
- `cafewall`
- `pinwheel`
- `petalwarp`
- `twistsquare`
- `eventhorizon`
- `noiseflow <0-10>`
- `tvstatic`
- `morph`

What they do:

- `aurora` runs an aurora-style wave effect
- `life` runs rainbow Conway's Game of Life
- `rainbow` runs a flowing rainbow pattern
- `matrix` runs a green matrix-rain effect
- `fire` runs an animated flame effect
- `plasma` runs a colorful plasma effect
- `twinkle` runs a starfield-style sparkle effect
- `supernova` runs a cinematic plasma-sky fireworks showpiece
- `vortex` runs a neon spiral tunnel effect
- `lava` runs a molten lava field effect
- `meteor` runs an upgraded comet-storm sky with impacts
- `storm` runs an electric storm with lightning
- `galaxy` runs a colorful bright spiral galaxy effect
- `catrun` runs a looping cat animation from left to right
- `squareburst` runs colorful squares that pop and explode
- `smoke` runs color smoke / ink in water
- `blobs` runs lava-lamp blobs that merge and split
- `windfire` runs a windy fire effect
- `ripples` runs water ripples from a moving point
- `circuit` runs an energy pulse through circuits
- `radar` runs a radar sweep with glowing targets
- `stylerain` runs stylized digital rain
- `neural` runs neural-network light paths firing across nodes
- `tunnel` runs an infinite tunnel zoom
- `cube` runs a rotating wireframe cube
- `kaleido` runs kaleidoscope symmetry
- `spiral` runs a spiral vortex
- `interfere` runs wave interference patterns
- `tessellate` runs tessellations that shift and morph
- `beatburst` runs particle explosions on beats
- `sunrise` runs a sunrise gradient over silhouettes
- `pacman` runs Pac-Man and three spaced ghosts through a pellet-filled maze; eaten pellets remain cleared until the round resets
- `flappy` starts Flappy Bird; send any Serial Monitor input to jump, or send `c` to exit back to the command menu
- `pong` runs neon auto-Pong; send `c` to return to the command menu
- `breakout [1-10]` starts playable Breakout at the selected speed (default `5`); send `a` or `d` to move the paddle and `c` to return to the command menu
- `snake` starts playable Snake; steer with `w`, `a`, `s`, and `d`, or send `c` to exit
- `invaders` starts playable Space Invaders; use `a`/`d` to move, any other input to fire, and `c` to exit
- `rhythm [1-10]` starts a four-lane Guitar Hero-style game at the selected speed (default `5`); hit falling notes with `a`, `s`, `k`, and `l`, or send `c` to exit
- `sphere` runs a fake 3D sphere made of particles
- `parallax` runs a parallax starfield
- `moire` runs moire-like shifting lines
- `hypnosis` runs pulsing, wandering concentric rings
- `cafewall` runs offset tile rows that make straight dividers appear slanted
- `pinwheel` runs a rotating chromatic spiral with a breathing depth effect
- `petalwarp` runs a zooming radial flower made from folding light bands
- `twistsquare` runs a black-and-white nested-square spiral tunnel
- `eventhorizon` runs a chromatic gravity-well showpiece with a warped grid and glowing lens ring
- `noiseflow <0-10>` runs an adjustable electric-liquid noise field with warped color currents
- `tvstatic` runs full-screen colorful analog television snow with scanlines and horizontal tearing
- `morph` runs morphing one symbol into another

All of these custom effects draw through `drawPixelMapped()`.

## Using The Sketch

Basic workflow:

1. Open [examples/AAAthiago_testing/AAAthiago_testing.ino](/c:/Users/costatf/Desktop/RGBMatrixPanel-for-1-4-scan-main/RGB_matrix_Panel/examples/AAAthiago_testing/AAAthiago_testing.ino).
2. Upload it to the ESP32.
3. Open Serial Monitor at `115200`.
4. Start with diagnostics such as `corners`, `row`, `chunks 0`, or `px 0 0`.
5. Then try display modes such as `supernova`, `galaxy`, `squareburst`, `meteor`, `catrun`, or `storm`.
6. Use `stop` to leave an animated mode.

Clock example:

1. Upload the sketch.
2. Open Serial Monitor.
3. If needed, run `settime 14:37`.
4. Run `pclock`.

## Project Goal: LED Panels As Wall Decor

The bigger goal is to turn this from a one-panel testbed into a multi-panel wall installation.

Planned use:

- mount several LED matrices on a wall
- use them as modular animated art pieces
- display ambient loops, text, symbols, and low-resolution motion graphics
- explore portrait and landscape layouts depending on placement

Why this single-panel work matters first:

- one panel needs reliable mapping before multiple panels can be chained
- the drawing code needs to work in normal logical coordinates
- the visual effects need to look good at low resolution
- text and single-character display need to work for wall labels, numbers, or icon-like designs

Once the single-panel mapping is stable enough, the next step is to expand into a larger virtual display made from several panels.

## Known Limitations

- the right half of the panel still has unresolved artifacts
- mapping is improved but not fully solved
- some of the remaining issues may be timing-related rather than purely geometric
- the sketch has grown into a capable testbed, but it is still a debugging and prototyping environment rather than a polished library

## Notes

- the active sketch contains commented mapping history to preserve earlier findings while tuning
- the project currently favors explicit test commands over abstraction because visibility and debugging speed are more important right now than cleanup
- later, once the hardware behavior is fully understood, the sketch should be cleaned up and the reusable pieces should be separated from the test harness
