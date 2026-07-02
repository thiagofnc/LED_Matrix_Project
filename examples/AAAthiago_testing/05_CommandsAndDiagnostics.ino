// Serial command routing, help output, and panel diagnostic patterns.
void printHelp() {
  Serial.println();
  Serial.println("Calibration commands:");
  Serial.println("  h            -> show this help");
  Serial.println("  corners      -> show corner test once");
  Serial.println("  row          -> step through full rows, Enter advances");
  Serial.println("  col <y>      -> step through columns on row y, Enter advances");
  Serial.println("  split <y>    -> show row y with x=0..15 RED and x=16..31 GREEN");
  Serial.println("  chunks <y>   -> show row y with 8-pixel color-coded chunks");
  Serial.println("  chunk <y> <n>   -> show one 8-pixel chunk n=0..3 on row y");
  Serial.println("  chunkpx <y> <n> <o> -> show one pixel in chunk n at offset o=0..7");
  Serial.println("  px <x> <y>   -> show one logical pixel");
  Serial.println("  sweep <ms>   -> sweep all pixels with delay per pixel in ms");
  Serial.println("  aurora       -> run remapped aurora animation");
  Serial.println("  life         -> run rainbow Conway's Game of Life");
  Serial.println("  rainbow      -> run rainbow wave pattern");
  Serial.println("  matrix       -> run green matrix-rain pattern");
  Serial.println("  fire         -> run animated fire effect");
  Serial.println("  plasma       -> run colorful plasma effect");
  Serial.println("  pclock       -> plasma with the current clock time");
  Serial.println("  rainclock    -> colorful rain clock wallpaper");
  Serial.println("  starclock    -> drifting stars and comet clock wallpaper");
  Serial.println("  aquarium     -> fish and bubbles clock wallpaper");
  Serial.println("  matrixclock  -> green digital-rain clock wallpaper");
  Serial.println("  tetrisclock  -> falling-block clock wallpaper");
  Serial.println("  orbitclock   -> orbiting-particle clock wallpaper");
  Serial.println("  settime <hh:mm[:ss]> -> set the device clock manually");
  Serial.println("  twinkle      -> run neon starfield twinkle");
  Serial.println("  soundbar     -> run rainbow equalizer bars");
  Serial.println("  fluid        -> run fluid-like rainbow motion");
  Serial.println("  fireworks    -> run rainbow fireworks");
  Serial.println("  supernova    -> run the cinematic plasma + fireworks showpiece");
  Serial.println("  vortex       -> run a neon spiral tunnel");
  Serial.println("  lava         -> run a molten lava field");
  Serial.println("  meteor       -> run an upgraded comet-storm sky with impacts");
  Serial.println("  storm        -> run an electric storm with lightning");
  Serial.println("  galaxy       -> run a colorful bright spiral galaxy");
  Serial.println("  catrun       -> run a looping cat animation left to right");
  Serial.println("  squareburst  -> run colorful squares that pop and explode");
  Serial.println("  smoke        -> run color smoke / ink in water");
  Serial.println("  blobs        -> run lava-lamp blobs that merge and split");
  Serial.println("  windfire     -> run a windy fire effect");
  Serial.println("  ripples      -> run water ripples from a moving point");
  Serial.println("  circuit      -> run an energy pulse through circuits");
  Serial.println("  radar        -> run a radar sweep with glowing targets");
  Serial.println("  stylerain    -> run stylized digital rain");
  Serial.println("  neural       -> run neural-network light paths");
  Serial.println("  tunnel       -> run an infinite tunnel zoom");
  Serial.println("  cube         -> run a rotating wireframe cube");
  Serial.println("  kaleido      -> run kaleidoscope symmetry");
  Serial.println("  spiral       -> run a spiral vortex");
  Serial.println("  interfere    -> run wave interference patterns");
  Serial.println("  tessellate   -> run morphing tessellations");
  Serial.println("  beatburst    -> run particle explosions on beats");
  Serial.println("  sunrise      -> run a sunrise over silhouettes");
  Serial.println("  pacman       -> run a Pac-Man style chase loop");
  Serial.println("  dino         -> play infinite Dino runner; any input jumps, c exits");
  Serial.println("  dinoclock    -> watch an infinite auto-jumping Dino with a clock");
  Serial.println("  flappy       -> play Flappy Bird; any input jumps, c returns here");
  Serial.println("  pong         -> watch neon auto-Pong; c returns here");
  Serial.println("  breakout [1-10] -> play Breakout at the selected speed");
  Serial.println("  snake        -> play Snake; w/a/s/d move, c returns here");
  Serial.println("  invaders     -> play Invaders; a/d move, other keys fire, c exits");
  Serial.println("  rhythm [1-15] -> play four lanes at the selected speed");
  Serial.println("  sphere       -> run a hypnotic shaded energy orb");
  Serial.println("  parallax     -> run a parallax starfield");
  Serial.println("  moire        -> run moire-like shifting lines");
  Serial.println("  hypnosis     -> run pulsing hypnotic rings");
  Serial.println("  cafewall     -> run the drifting cafe-wall illusion");
  Serial.println("  pinwheel     -> run a rotating chromatic pinwheel illusion");
  Serial.println("  petalwarp    -> run a zooming flower-fold illusion");
  Serial.println("  twistsquare  -> run a twisting nested-square tunnel");
  Serial.println("  eventhorizon -> run the chromatic gravity-well showpiece");
  Serial.println("  morph        -> run a morphing symbol animation");
  Serial.println("  dvd          -> run a bouncing DVD logo screensaver");
  Serial.println("  tvbars       -> run animated TV color bars with static");
  Serial.println("  tvstatic     -> run full-screen colorful analog TV static");
  Serial.println("  sinewave     -> run layered colorful sine waves");
  Serial.println("  noiseflow <0-10> -> run adjustable electric noise currents");
  Serial.println("  phasebeat    -> run two drifting waves with beat highlights");
  Serial.println("  lissajous    -> run a colorful Lissajous curve");
  Serial.println("  harmonics    -> run stacked harmonic wave traces");
  Serial.println("  silk         -> run flowing iridescent silk wallpaper");
  Serial.println("  silkclock    -> iridescent silk with a clock overlay");
  Serial.println("  deepsea      -> bioluminescent deep-sea clock wallpaper");
  Serial.println("  text <msg>   -> display remapped text using built-in 5x7 font");
  Serial.println("  scroll <msg> -> continuously scroll text from right to left");
  Serial.println("  draw <msg>   -> draw each character one pixel at a time");
  Serial.println("  emoji [ms]   -> cycle through emojis with an optional delay");
  Serial.println("  rchar <c>    -> display one large char on rotated 16x32 view");
  Serial.println("  rchartest <ms> -> cycle through all supported rotated chars");
  Serial.println("  stop         -> stop animation / stepped mode");
  Serial.println("  next         -> advance current stepped test");
  Serial.println("  clear        -> blank the panel");
  Serial.println();
  Serial.println("Suggested order:");
  Serial.println("  1. row");
  Serial.println("  2. split 0, split 4, split 8, split 12");
  Serial.println("  3. col 0, col 4, col 8, col 12");
  Serial.println("  4. corners");
  Serial.println();
}

void clearSerialInput() {
  while (Serial.available()) {
    Serial.read();
  }
}

void advanceCurrentMode() {
  switch (currentMode) {
    case MODE_ROW_STEP:
      showRowStep(currentRow);
      currentRow++;
      if (currentRow >= PANEL_RES_Y) {
        currentMode = MODE_HELP;
        currentRow = 0;
        Serial.println("Row stepping complete.");
      }
      break;

    case MODE_COL_STEP:
      showColumnStep(currentRow, currentCol);
      currentCol++;
      if (currentCol >= PANEL_RES_X) {
        currentMode = MODE_HELP;
        currentCol = 0;
        Serial.println("Column stepping complete.");
      }
      break;

    default:
      Serial.println("No stepped test is active. Use 'row' or 'col <y>'.");
      break;
  }
}

void handleCommand(String input) {
  input.trim();
  input.toLowerCase();

  if (input.length() == 0 || input == "next") {
    advanceCurrentMode();
    return;
  }

  if (input == "h" || input == "help") {
    currentMode = MODE_HELP;
    printHelp();
    return;
  }

  if (input == "clear") {
    currentMode = MODE_HELP;
    plasmaClockInitialized = false;
    lastClockText = "";
    matrix->fillScreen(BLACK);
    Serial.println("Panel cleared.");
    return;
  }

  if (input == "stop") {
    currentMode = MODE_HELP;
    plasmaClockInitialized = false;
    lastClockText = "";
    matrix->fillScreen(BLACK);
    Serial.println("Stopped active mode.");
    return;
  }

  if (input == "corners") {
    currentMode = MODE_CORNERS;
    testCorners(5000);
    Serial.println("Corner test shown.");
    return;
  }

  if (input == "row") {
    currentMode = MODE_ROW_STEP;
    currentRow = 0;
    Serial.println("Row stepping started. Press Enter or send 'next' to advance.");
    advanceCurrentMode();
    return;
  }

  if (input.startsWith("col ")) {
    int y = input.substring(4).toInt();
    if (y < 0 || y >= PANEL_RES_Y) {
      Serial.println("Invalid row for col test. Use 0..15.");
      return;
    }
    currentMode = MODE_COL_STEP;
    currentRow = y;
    currentCol = 0;
    Serial.printf("Column stepping started for logical row %d. Press Enter or send 'next' to advance.\n", y);
    advanceCurrentMode();
    return;
  }

  if (input.startsWith("split ")) {
    int y = input.substring(6).toInt();
    if (y < 0 || y >= PANEL_RES_Y) {
      Serial.println("Invalid row for split test. Use 0..15.");
      return;
    }
    currentMode = MODE_SPLIT_ROW_STEP;
    showSplitRowStep(y);
    return;
  }

  if (input.startsWith("chunks ")) {
    int y = input.substring(7).toInt();
    if (y < 0 || y >= PANEL_RES_Y) {
      Serial.println("Invalid row for chunk test. Use 0..15.");
      return;
    }
    currentMode = MODE_HELP;
    showChunkRowStep(y);
    return;
  }

  if (input.startsWith("chunk ")) {
    int firstSpace = input.indexOf(' ', 6);
    if (firstSpace == -1) {
      Serial.println("Use: chunk <row> <chunk>");
      return;
    }

    int y = input.substring(6, firstSpace).toInt();
    int chunkIndex = input.substring(firstSpace + 1).toInt();
    if (y < 0 || y >= PANEL_RES_Y || chunkIndex < 0 || chunkIndex > 3) {
      Serial.println("Invalid chunk command. Use row=0..15 and chunk=0..3.");
      return;
    }

    currentMode = MODE_HELP;
    showSingleChunk(y, chunkIndex);
    return;
  }

  if (input.startsWith("chunkpx ")) {
    int firstSpace = input.indexOf(' ', 8);
    int secondSpace = (firstSpace == -1) ? -1 : input.indexOf(' ', firstSpace + 1);
    if (firstSpace == -1 || secondSpace == -1) {
      Serial.println("Use: chunkpx <row> <chunk> <offset>");
      return;
    }

    int y = input.substring(8, firstSpace).toInt();
    int chunkIndex = input.substring(firstSpace + 1, secondSpace).toInt();
    int offset = input.substring(secondSpace + 1).toInt();
    if (y < 0 || y >= PANEL_RES_Y || chunkIndex < 0 || chunkIndex > 3 || offset < 0 || offset > 7) {
      Serial.println("Invalid chunkpx command. Use row=0..15 chunk=0..3 offset=0..7.");
      return;
    }

    currentMode = MODE_HELP;
    showSingleChunkPixel(y, chunkIndex, offset);
    return;
  }

  if (input.startsWith("sweep ")) {
    int delayMs = input.substring(6).toInt();
    if (delayMs < 0) {
      Serial.println("Invalid sweep delay.");
      return;
    }

    currentMode = MODE_HELP;
    Serial.printf("Starting full pixel sweep with %d ms delay.\n", delayMs);
    testPixelSweepSingle((uint16_t)delayMs);
    Serial.println("Pixel sweep complete.");
    return;
  }

  if (input.startsWith("px ")) {
    int spaceIndex = input.indexOf(' ', 3);
    if (spaceIndex == -1) {
      Serial.println("Invalid pixel command. Use: px <x> <y>");
      return;
    }

    int x = input.substring(3, spaceIndex).toInt();
    int y = input.substring(spaceIndex + 1).toInt();
    if (x < 0 || x >= PANEL_RES_X || y < 0 || y >= PANEL_RES_Y) {
      Serial.println("Invalid pixel coordinates. Use x=0..31 and y=0..15.");
      return;
    }

    currentMode = MODE_SINGLE_PIXEL;
    showSinglePixel(x, y);
    return;
  }

  if (input == "aurora") {
    currentMode = MODE_AURORA;
    Serial.println("Aurora animation started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "life") {
    seedLife();
    currentMode = MODE_LIFE;
    Serial.println("Rainbow Game of Life started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "rainbow") {
    currentMode = MODE_RAINBOW;
    Serial.println("Rainbow pattern started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "matrix") {
    seedMatrixRain();
    currentMode = MODE_MATRIX_RAIN;
    Serial.println("Matrix rain started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "fire") {
    seedFire();
    currentMode = MODE_FIRE;
    Serial.println("Fire effect started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "plasma") {
    plasmaClockInitialized = false;
    lastClockText = "";
    currentMode = MODE_PLASMA;
    Serial.println("Plasma effect started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "pclock") {
    plasmaClockInitialized = false;
    lastClockText = "";
    currentMode = MODE_PLASMA_CLOCK;
    Serial.print("Plasma clock started at ");
    Serial.print(getCurrentClockText());
    Serial.println(". Send 'stop' to return to command mode.");
    return;
  }

  if (input == "rainclock") {
    currentMode = MODE_RAIN_CLOCK;
    Serial.println("Rain clock started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "starclock") {
    currentMode = MODE_STAR_CLOCK;
    Serial.println("Star clock started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "aquarium") {
    currentMode = MODE_AQUARIUM_CLOCK;
    Serial.println("Aquarium clock started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "matrixclock") {
    currentMode = MODE_MATRIX_CLOCK;
    Serial.println("Matrix clock started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "tetrisclock") {
    currentMode = MODE_TETRIS_CLOCK;
    Serial.println("Tetris clock started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "orbitclock") {
    currentMode = MODE_ORBIT_CLOCK;
    Serial.println("Orbit clock started. Send 'stop' to return to command mode.");
    return;
  }

  if (input.startsWith("settime ")) {
    String value = input.substring(8);
    value.trim();
    if (!setClockFromString(value)) {
      Serial.println("Use: settime <hh:mm> or settime <hh:mm:ss>");
      return;
    }
    clockText = getCurrentClockText();
    plasmaClockInitialized = false;
    lastClockText = "";
    Serial.print("Clock set to ");
    Serial.println(clockText);
    return;
  }

  if (input == "twinkle") {
    currentMode = MODE_TWINKLE;
    Serial.println("Twinkle effect started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "soundbar") {
    seedSoundbar();
    currentMode = MODE_SOUNDBAR;
    Serial.println("Soundbar effect started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "fluid") {
    seedFluid();
    currentMode = MODE_FLUID;
    Serial.println("Fluid effect started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "fireworks") {
    clearFireworks();
    currentMode = MODE_FIREWORKS;
    Serial.println("Fireworks effect started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "supernova") {
    clearSupernova();
    currentMode = MODE_SUPERNOVA;
    Serial.println("Supernova effect started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "vortex") {
    currentMode = MODE_VORTEX;
    Serial.println("Vortex effect started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "lava") {
    currentMode = MODE_LAVA;
    Serial.println("Lava effect started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "meteor") {
    seedMeteors();
    currentMode = MODE_METEOR;
    Serial.println("Meteor effect started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "storm") {
    stormBoltLife = 0;
    currentMode = MODE_STORM;
    Serial.println("Storm effect started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "galaxy") {
    currentMode = MODE_GALAXY;
    Serial.println("Galaxy effect started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "catrun") {
    catRunOffsetX = -12;
    catRunFrame = 0;
    currentMode = MODE_CATRUN;
    Serial.println("Cat run started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "squareburst") {
    seedSquareBursts();
    currentMode = MODE_SQUAREBURST;
    Serial.println("Square burst effect started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "smoke") {
    currentMode = MODE_SMOKE;
    Serial.println("Smoke effect started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "blobs") {
    currentMode = MODE_BLOBS;
    Serial.println("Blob effect started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "windfire") {
    seedFire();
    currentMode = MODE_WINDFIRE;
    Serial.println("Wind fire effect started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "ripples") {
    currentMode = MODE_RIPPLES;
    Serial.println("Ripple effect started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "circuit") {
    currentMode = MODE_CIRCUIT;
    Serial.println("Circuit effect started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "radar") {
    currentMode = MODE_RADAR;
    Serial.println("Radar effect started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "stylerain") {
    currentMode = MODE_STYLERAIN;
    Serial.println("Stylized rain effect started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "neural") {
    currentMode = MODE_NEURAL;
    Serial.println("Neural effect started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "tunnel") {
    currentMode = MODE_TUNNEL;
    Serial.println("Tunnel effect started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "cube") {
    currentMode = MODE_CUBE;
    Serial.println("Cube effect started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "kaleido") {
    currentMode = MODE_KALEIDO;
    Serial.println("Kaleidoscope effect started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "spiral") {
    currentMode = MODE_SPIRAL;
    Serial.println("Spiral effect started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "interfere") {
    currentMode = MODE_INTERFERENCE;
    Serial.println("Interference effect started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "tessellate") {
    currentMode = MODE_TESSELLATE;
    Serial.println("Tessellation effect started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "beatburst") {
    currentMode = MODE_BEATBURST;
    Serial.println("Beat burst effect started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "sunrise") {
    currentMode = MODE_SUNRISE;
    Serial.println("Sunrise effect started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "pacman") {
    resetPacmanGame();
    currentMode = MODE_PACMAN;
    Serial.println("Pac-Man maze chase started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "dino") {
    currentMode = MODE_DINO;
    resetDinoGame();
    Serial.println("Infinite Dino started. Any key jumps; after a crash, any key restarts; c exits.");
    return;
  }

  if (input == "dinoclock") {
    currentMode = MODE_DINO_CLOCK;
    Serial.println("Dino clock started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "flappy") {
    resetFlappyGame();
    currentMode = MODE_FLAPPY;
    Serial.println("Flappy Bird started. Send anything to jump; send 'c' to return to the menu.");
    return;
  }

  if (input == "pong") {
    resetPongGame();
    currentMode = MODE_PONG;
    Serial.println("Neon auto-Pong started. Send 'c' to return to the menu.");
    return;
  }

  if (input == "breakout" || input.startsWith("breakout ")) {
    uint8_t requestedSpeed = 5;
    if (input.length() > 8) {
      int parsedSpeed = input.substring(9).toInt();
      if (parsedSpeed < 1 || parsedSpeed > 10) {
        Serial.println("Invalid Breakout speed. Use: breakout <1-10>");
        return;
      }
      requestedSpeed = parsedSpeed;
    }
    breakoutSpeed = requestedSpeed;
    resetBreakoutGame();
    currentMode = MODE_BREAKOUT;
    Serial.printf("Breakout started at speed %u. Use a/d to move; c returns to the menu.\n", breakoutSpeed);
    return;
  }

  if (input == "snake") {
    resetSnakeGame();
    currentMode = MODE_SNAKE;
    Serial.println("Snake started. Send w/a/s/d to steer; 'c' returns to the menu.");
    return;
  }

  if (input == "invaders") {
    resetInvadersGame();
    currentMode = MODE_INVADERS;
    Serial.println("Invaders started. Send a/d to move, any other key to fire, or c to exit.");
    return;
  }

  if (input == "rhythm" || input.startsWith("rhythm ")) {
    uint8_t requestedSpeed = 5;
    if (input.length() > 6) {
      int parsedSpeed = input.substring(7).toInt();
      if (parsedSpeed < 1 || parsedSpeed > 15) {
        Serial.println("Invalid rhythm speed. Use: rhythm <1-15>");
        return;
      }
      requestedSpeed = parsedSpeed;
    }
    rhythmSpeed = requestedSpeed;
    resetRhythmGame();
    currentMode = MODE_RHYTHM;
    Serial.printf("Rhythm game started at speed %u. Use a/s/k/l to hit notes; c exits.\n", rhythmSpeed);
    return;
  }

  if (input == "sphere") {
    currentMode = MODE_SPHERE;
    Serial.println("Sphere effect started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "parallax") {
    currentMode = MODE_PARALLAX;
    Serial.println("Parallax effect started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "moire") {
    currentMode = MODE_MOIRE;
    Serial.println("Moire effect started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "hypnosis") {
    currentMode = MODE_HYPNOSIS;
    Serial.println("Hypnotic rings started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "cafewall") {
    currentMode = MODE_CAFEWALL;
    Serial.println("Cafe-wall illusion started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "pinwheel") {
    currentMode = MODE_PINWHEEL;
    Serial.println("Chromatic pinwheel started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "petalwarp") {
    currentMode = MODE_PETALWARP;
    Serial.println("Petal warp started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "twistsquare") {
    currentMode = MODE_TWISTSQUARE;
    Serial.println("Twisting square tunnel started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "eventhorizon") {
    currentMode = MODE_EVENTHORIZON;
    Serial.println("Event horizon showpiece started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "morph") {
    currentMode = MODE_MORPH;
    Serial.println("Morph effect started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "dvd") {
    seedDvdBounce();
    currentMode = MODE_DVD;
    Serial.println("DVD bounce started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "tvbars") {
    currentMode = MODE_TVBARS;
    Serial.println("TV color bars started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "tvstatic") {
    currentMode = MODE_TVSTATIC;
    Serial.println("Color TV static started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "sinewave") {
    currentMode = MODE_SINEWAVE;
    Serial.println("Colorful sine wave started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "noiseflow") {
    currentMode = MODE_NOISEFLOW;
    Serial.printf("Noise flow started at level %u. Use 'noiseflow <0-10>' to adjust.\n", noiseFlowAmount);
    return;
  }

  if (input.startsWith("noiseflow ")) {
    String value = input.substring(10);
    value.trim();
    bool validLevel = value.length() > 0 && value.length() <= 2;
    for (unsigned int i = 0; i < value.length() && validLevel; i++) {
      validLevel = value[i] >= '0' && value[i] <= '9';
    }
    int level = validLevel ? value.toInt() : -1;
    if (level < 0 || level > 10) {
      Serial.println("Use: noiseflow <0-10>");
      return;
    }
    noiseFlowAmount = (uint8_t)level;
    currentMode = MODE_NOISEFLOW;
    Serial.printf("Noise flow started at level %u. Send 'stop' to return to command mode.\n", noiseFlowAmount);
    return;
  }

  if (input == "phasebeat") {
    currentMode = MODE_PHASEBEAT;
    Serial.println("Phase beat waves started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "lissajous") {
    currentMode = MODE_LISSAJOUS;
    Serial.println("Lissajous curve started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "harmonics") {
    currentMode = MODE_HARMONICS;
    Serial.println("Harmonic wave stack started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "silk") {
    currentMode = MODE_SILK;
    Serial.println("Iridescent silk wallpaper started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "silkclock") {
    currentMode = MODE_SILK_CLOCK;
    Serial.println("Iridescent silk clock started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "deepsea") {
    currentMode = MODE_DEEPSEA;
    Serial.println("Deep-sea clock wallpaper started. Send 'stop' to return to command mode.");
    return;
  }

  if (input.startsWith("text ")) {
    textMessage = input.substring(5);
    if (textMessage.length() == 0) {
      textMessage = " ";
    }
    currentMode = MODE_HELP;
    showTextMessage(textMessage);
    Serial.print("Displayed text: ");
    Serial.println(textMessage);
    return;
  }

  if (input.startsWith("scroll ")) {
    textMessage = input.substring(7);
    if (textMessage.length() == 0) textMessage = " ";
    scrollTextX = PANEL_RES_X;
    scrollTextLastStep = 0;
    currentMode = MODE_SCROLL_TEXT;
    Serial.print("Scrolling text: ");
    Serial.println(textMessage);
    return;
  }

  if (input.startsWith("draw ")) {
    textMessage = input.substring(5);
    if (textMessage.length() == 0) textMessage = " ";
    drawTextCharIndex = 0;
    drawTextPixelIndex = 0;
    drawTextLastStep = 0;
    matrix->fillScreen(BLACK);
    currentMode = MODE_DRAW_TEXT;
    Serial.print("Drawing text one character at a time: ");
    Serial.println(textMessage);
    return;
  }

  if (input == "emoji" || input.startsWith("emoji ")) {
    if (input.length() > 5) {
      int requestedDelay = input.substring(6).toInt();
      if (requestedDelay < 100 || requestedDelay > 60000) {
        Serial.println("Use: emoji [delay-ms], where delay is 100..60000.");
        return;
      }
      emojiDelayMs = (uint16_t)requestedDelay;
    }
    emojiIndex = 0;
    emojiLastChange = 0;
    currentMode = MODE_EMOJI;
    Serial.printf("Emoji cycle started with %u ms delay. Send 'stop' to return.\n", emojiDelayMs);
    return;
  }

  if (input.startsWith("rchar ")) {
    String value = input.substring(6);
    value.trim();
    if (value.length() == 0) {
      Serial.println("Use: rchar <character>");
      return;
    }

    rotatedChar = value[0];
    currentMode = MODE_HELP;
    showRotatedSingleChar(rotatedChar);
    Serial.print("Displayed rotated character: ");
    Serial.println(rotatedChar);
    return;
  }

  if (input.startsWith("rchartest ")) {
    int delayMs = input.substring(10).toInt();
    if (delayMs < 0) {
      Serial.println("Invalid rchartest delay.");
      return;
    }

    currentMode = MODE_HELP;
    Serial.printf("Starting rotated character test with %d ms delay.\n", delayMs);
    testRotatedCharSet((uint16_t)delayMs);
    Serial.println("Rotated character test complete.");
    return;
  }

  if (input.startsWith("sweep ")) {
    int delayMs = input.substring(6).toInt();
    if (delayMs < 0) {
      Serial.println("Invalid sweep delay.");
      return;
    }

    currentMode = MODE_HELP;
    Serial.printf("Starting full pixel sweep with %d ms delay.\n", delayMs);
    testPixelSweepSingle((uint16_t)delayMs);
    Serial.println("Pixel sweep complete.");
    return;
  }

  Serial.println("Unknown command.");
  printHelp();
}

 
void testBorder(uint16_t msDelay) {
  matrix->fillScreen(BLACK);
 
  for (int x = 0; x < PANEL_RES_X; x++) {
    drawPixelMapped(x, 0, YELLOW);
    drawPixelMapped(x, PANEL_RES_Y - 1, YELLOW);
  }
 
  for (int y = 0; y < PANEL_RES_Y; y++) {
    drawPixelMapped(0, y, YELLOW);
    drawPixelMapped(PANEL_RES_X - 1, y, YELLOW);
  }
 
  delay(msDelay);
  matrix->fillScreen(BLACK);
}
 
void testRows(uint16_t msDelayPerRow) {
  for (int y = 0; y < PANEL_RES_Y; y++) {
    matrix->fillScreen(BLACK);
 
    uint16_t color;
    if (y % 3 == 0) color = RED;
    else if (y % 3 == 1) color = GREEN;
    else color = BLUE;
 
    for (int x = 0; x < PANEL_RES_X; x++) {
      drawPixelMapped(x, y, color);
    }
 
    delay(msDelayPerRow);
  }
 
  matrix->fillScreen(BLACK);
}
 
void testColumns(uint16_t msDelayPerCol) {
  for (int x = 0; x < PANEL_RES_X; x++) {
    matrix->fillScreen(BLACK);
 
    uint16_t color;
    if (x % 3 == 0) color = RED;
    else if (x % 3 == 1) color = GREEN;
    else color = BLUE;
 
    for (int y = 0; y < PANEL_RES_Y; y++) {
      drawPixelMapped(x, y, color);
    }
 
    delay(msDelayPerCol);
  }
 
  matrix->fillScreen(BLACK);
}
 
void testCheckerboard(uint16_t msDelay) {
  matrix->fillScreen(BLACK);
 
  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      if ((x + y) & 1) drawPixelMapped(x, y, WHITE);
    }
  }
 
  delay(msDelay);
  matrix->fillScreen(BLACK);
}
 
void testQuadrants(uint16_t msDelay) {
  matrix->fillScreen(BLACK);
 
  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      if (x < 16 && y < 8) drawPixelMapped(x, y, RED);
      else if (x >= 16 && y < 8) drawPixelMapped(x, y, GREEN);
      else if (x < 16 && y >= 8) drawPixelMapped(x, y, BLUE);
      else drawPixelMapped(x, y, WHITE);
    }
  }
 
  delay(msDelay);
  matrix->fillScreen(BLACK);
}
 
void testText14(uint16_t msDelay) {
  matrix->fillScreen(BLACK);
 
  matrix->setTextWrap(false);
  matrix->setTextSize(2);
  matrix->setTextColor(WHITE);
  matrix->setCursor(6, 1);
  matrix->print("14");
 
  delay(msDelay);
  matrix->fillScreen(BLACK);
}
 
