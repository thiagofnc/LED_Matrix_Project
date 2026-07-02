#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <ESP32-VirtualMatrixPanel-I2S-DMA.h>
#include <time.h>
#include <sys/time.h>
 
// ================= Panel settings =================
#define PANEL_RES_X 32
#define PANEL_RES_Y 16
#define PANEL_CHAIN 1

// A single 32x16 1/4-scan panel must be configured to the DMA engine
// as 64x8. VirtualMatrixPanel then remaps that back to normal 32x16 XY.
#define DMA_PANEL_RES_X (PANEL_RES_X * 2)
#define DMA_PANEL_RES_Y (PANEL_RES_Y / 2)
#define VIRTUAL_PANEL_ROWS 1
#define VIRTUAL_PANEL_COLS 1

// These right-edge artifacts may be signal-timing related rather than pure
// XY mapping. Keep these explicit so they are easy to tune.
#define PANEL_I2S_SPEED HUB75_I2S_CFG::HZ_10M
#define PANEL_CLK_PHASE false
#define PANEL_TIMEZONE "EST5EDT,M3.2.0/2,M11.1.0/2"
 
// ================= Your exact pin mapping =================
HUB75_I2S_CFG::i2s_pins _pins = {
  35, // R0
  21, // G0
  41, // B0
  36, // R1
  12, // G1
  39, // B1
  48, // A
  19, // B
  5,  // C
  17, // D
  -1, // E
  4,  // LAT
  15, // OE
  16  // CLK
};
 
HUB75_I2S_CFG mxconfig(
  DMA_PANEL_RES_X,
  DMA_PANEL_RES_Y,
  PANEL_CHAIN,
  _pins
);
 
MatrixPanel_I2S_DMA *dma_display = nullptr;
VirtualMatrixPanel *matrix = nullptr;
 
uint16_t RED, GREEN, BLUE, WHITE, BLACK, YELLOW, CYAN, MAGENTA;

#define ROTATED_RES_X PANEL_RES_Y
#define ROTATED_RES_Y PANEL_RES_X

enum TestMode {
  MODE_HELP,
  MODE_CORNERS,
  MODE_ROW_STEP,
  MODE_COL_STEP,
  MODE_SPLIT_ROW_STEP,
  MODE_SINGLE_PIXEL,
  MODE_AURORA,
  MODE_LIFE,
  MODE_RAINBOW,
  MODE_MATRIX_RAIN,
  MODE_FIRE,
  MODE_PLASMA,
  MODE_TWINKLE,
  MODE_SOUNDBAR,
  MODE_FLUID,
  MODE_FIREWORKS,
  MODE_PLASMA_CLOCK,
  MODE_RAIN_CLOCK,
  MODE_STAR_CLOCK,
  MODE_AQUARIUM_CLOCK,
  MODE_MATRIX_CLOCK,
  MODE_TETRIS_CLOCK,
  MODE_ORBIT_CLOCK,
  MODE_DINO_CLOCK,
  MODE_SUPERNOVA,
  MODE_VORTEX,
  MODE_LAVA,
  MODE_METEOR,
  MODE_STORM,
  MODE_GALAXY,
  MODE_CATRUN,
  MODE_SQUAREBURST,
  MODE_SMOKE,
  MODE_BLOBS,
  MODE_WINDFIRE,
  MODE_RIPPLES,
  MODE_CIRCUIT,
  MODE_RADAR,
  MODE_STYLERAIN,
  MODE_NEURAL,
  MODE_TUNNEL,
  MODE_CUBE,
  MODE_KALEIDO,
  MODE_SPIRAL,
  MODE_INTERFERENCE,
  MODE_TESSELLATE,
  MODE_BEATBURST,
  MODE_SUNRISE,
  MODE_PACMAN,
  MODE_DINO,
  MODE_FLAPPY,
  MODE_PONG,
  MODE_BREAKOUT,
  MODE_SNAKE,
  MODE_INVADERS,
  MODE_RHYTHM,
  MODE_SPHERE,
  MODE_PARALLAX,
  MODE_MOIRE,
  MODE_HYPNOSIS,
  MODE_CAFEWALL,
  MODE_PINWHEEL,
  MODE_PETALWARP,
  MODE_TWISTSQUARE,
  MODE_EVENTHORIZON,
  MODE_MORPH,
  MODE_DVD,
  MODE_TVBARS,
  MODE_TVSTATIC,
  MODE_SINEWAVE,
  MODE_NOISEFLOW,
  MODE_PHASEBEAT,
  MODE_LISSAJOUS,
  MODE_HARMONICS,
  MODE_SILK,
  MODE_SILK_CLOCK,
  MODE_DEEPSEA,
  MODE_SCROLL_TEXT,
  MODE_DRAW_TEXT,
  MODE_EMOJI
};

TestMode currentMode = MODE_HELP;
int currentRow = 0;
int currentCol = 0;

struct RemapPoint {
  uint8_t x;
  uint8_t y;
};

RemapPoint inverseMap[PANEL_RES_Y][PANEL_RES_X];
bool lifeCurrent[PANEL_RES_Y][PANEL_RES_X];
bool lifeNext[PANEL_RES_Y][PANEL_RES_X];
uint16_t lifeGeneration = 0;
String textMessage = "HELLO";
int scrollTextX = PANEL_RES_X;
uint32_t scrollTextLastStep = 0;
size_t drawTextCharIndex = 0;
uint8_t drawTextPixelIndex = 0;
uint32_t drawTextLastStep = 0;
uint8_t emojiIndex = 0;
uint16_t emojiDelayMs = 1000;
uint32_t emojiLastChange = 0;
const uint8_t PACMAN_ROUTE_LENGTH = 68;
bool pacmanPellets[PACMAN_ROUTE_LENGTH];
uint8_t pacmanRouteIndex = 0;
uint32_t pacmanLastStep = 0;
uint32_t pacmanRoundCompleteAt = 0;
float dinoY = 10.0f;
float dinoVelocity = 0.0f;
float dinoObstacleX[3] = {34.0f, 49.0f, 66.0f};
uint8_t dinoObstacleHeight[3] = {3, 4, 3};
bool dinoObstaclePassed[3] = {false, false, false};
uint16_t dinoScore = 0;
uint32_t dinoDistance = 0;
uint32_t dinoLastFrame = 0;
bool dinoGameOver = false;
float flappyY = 7.0f;
float flappyVelocity = 0.0f;
float flappyPipeX[2] = {24.0f, 42.0f};
uint8_t flappyGapY[2] = {5, 9};
uint16_t flappyScore = 0;
bool flappyGameOver = false;
uint32_t flappyLastFrame = 0;
float pongX = 15.0f, pongY = 7.0f, pongVX = 0.55f, pongVY = 0.34f;
float pongLeftY = 5.0f, pongRightY = 5.0f;
uint32_t pongLastFrame = 0;
float breakoutX = 15.0f, breakoutY = 11.0f, breakoutVX = 0.45f, breakoutVY = -0.38f;
float breakoutPaddleX = 13.0f;
bool breakoutBricks[3][8];
uint32_t breakoutLastFrame = 0;
uint8_t breakoutSpeed = 5;
uint8_t snakeX[128], snakeY[128], snakeLength = 4;
int8_t snakeDX = 1, snakeDY = 0;
uint8_t snakeFoodX = 12, snakeFoodY = 4;
bool snakeGameOver = false;
uint32_t snakeLastStep = 0;
bool invaderAlive[12];
int8_t invaderOffsetX = 1, invaderOffsetY = 1, invaderDirection = 1;
int8_t invaderPlayerX = 15, invaderBulletX = -1, invaderBulletY = -1;
bool invaderGameOver = false, invaderWon = false;
uint32_t invaderLastFrame = 0, invaderLastMove = 0;
const uint8_t RHYTHM_NOTE_COUNT = 16;
int8_t rhythmNoteY[RHYTHM_NOTE_COUNT];
uint8_t rhythmNoteLane[RHYTHM_NOTE_COUNT];
bool rhythmNoteActive[RHYTHM_NOTE_COUNT];
uint16_t rhythmScore = 0, rhythmCombo = 0;
uint8_t rhythmFlashLane = 255, rhythmFlashFrames = 0;
bool rhythmHitSuccess = false;
uint32_t rhythmLastStep = 0, rhythmLastSpawn = 0;
uint8_t rhythmSpeed = 5;
String clockText = "12:34";
String lastClockText = "";
char rotatedChar = 'A';
bool plasmaClockInitialized = false;
int rainHead[PANEL_RES_X];
uint8_t rainLength[PANEL_RES_X];
uint8_t rainSpeed[PANEL_RES_X];
uint8_t rainTick = 0;
uint8_t fireHeat[PANEL_RES_X][PANEL_RES_Y];
uint16_t twinklePhase = 0;
uint8_t soundbarHeights[PANEL_RES_X];
uint8_t soundbarTargets[PANEL_RES_X];
uint8_t fluidField[PANEL_RES_Y][PANEL_RES_X];
uint8_t fluidScratch[PANEL_RES_Y][PANEL_RES_X];
uint32_t fluidTick = 0;
float dvdX = 2.0f;
float dvdY = 2.0f;
float dvdVX = 0.45f;
float dvdVY = 0.32f;
uint8_t dvdHue = 0;
uint8_t noiseFlowAmount = 5;
const int MAX_FIREWORK_PARTICLES = 48;
float fwX[MAX_FIREWORK_PARTICLES];
float fwY[MAX_FIREWORK_PARTICLES];
float fwVX[MAX_FIREWORK_PARTICLES];
float fwVY[MAX_FIREWORK_PARTICLES];
uint8_t fwLife[MAX_FIREWORK_PARTICLES];
uint8_t fwHue[MAX_FIREWORK_PARTICLES];
const int MAX_SUPERNOVA_PARTICLES = 72;
float snX[MAX_SUPERNOVA_PARTICLES];
float snY[MAX_SUPERNOVA_PARTICLES];
float snPrevX[MAX_SUPERNOVA_PARTICLES];
float snPrevY[MAX_SUPERNOVA_PARTICLES];
float snVX[MAX_SUPERNOVA_PARTICLES];
float snVY[MAX_SUPERNOVA_PARTICLES];
uint8_t snLife[MAX_SUPERNOVA_PARTICLES];
uint8_t snMaxLife[MAX_SUPERNOVA_PARTICLES];
uint8_t snHue[MAX_SUPERNOVA_PARTICLES];
bool snRocketActive = false;
float snRocketX = 0.0f;
float snRocketY = 0.0f;
float snRocketVX = 0.0f;
float snRocketVY = 0.0f;
uint8_t snRocketHue = 0;
uint8_t snRocketFuse = 0;
float snShockwaveX = 0.0f;
float snShockwaveY = 0.0f;
float snShockwaveRadius = 0.0f;
uint8_t snShockwaveLife = 0;
const int MAX_METEORS = 16;
const int MAX_METEOR_IMPACTS = 6;
float meteorX[MAX_METEORS];
float meteorY[MAX_METEORS];
float meteorVX[MAX_METEORS];
float meteorVY[MAX_METEORS];
uint8_t meteorHue[MAX_METEORS];
uint8_t meteorLength[MAX_METEORS];
float meteorImpactX[MAX_METEOR_IMPACTS];
float meteorImpactY[MAX_METEOR_IMPACTS];
float meteorImpactRadius[MAX_METEOR_IMPACTS];
uint8_t meteorImpactLife[MAX_METEOR_IMPACTS];
uint8_t stormBoltMask[PANEL_RES_Y];
uint8_t stormBoltLife = 0;
uint8_t stormBoltHue = 0;
int catRunOffsetX = -12;
uint8_t catRunFrame = 0;
const int MAX_SQUARE_BURSTS = 10;
float squareBurstX[MAX_SQUARE_BURSTS];
float squareBurstY[MAX_SQUARE_BURSTS];
float squareBurstRadius[MAX_SQUARE_BURSTS];
uint8_t squareBurstLife[MAX_SQUARE_BURSTS];
uint8_t squareBurstHue[MAX_SQUARE_BURSTS];

const char catFrames[4][8][13] = {
  {
    "..OO....OO..",
    ".OOOOOOOOOO.",
    "OOOWOOOOWOOO",
    "OOOOOOOOOOOO",
    ".OOOOPOOOOO.",
    "..OO....OO..",
    ".O..O..O..O.",
    "O....OO....O"
  },
  {
    "..OO....OO..",
    ".OOOOOOOOOO.",
    "OOOWOOOOWOOO",
    "OOOOOOOOOOOO",
    ".OOOOPOOOOO.",
    "...OO..OO...",
    ".OO..OO..OO.",
    "O..........O"
  },
  {
    "..OO....OO..",
    ".OOOOOOOOOO.",
    "OOOWOOOOWOOO",
    "OOOOOOOOOOOO",
    ".OOOOPOOOOO.",
    "..OO....OO..",
    "OO...OO...OO",
    "...OO..OO..."
  },
  {
    "..OO....OO..",
    ".OOOOOOOOOO.",
    "OOOWOOOOWOOO",
    "OOOOOOOOOOOO",
    ".OOOOPOOOOO.",
    "...OO..OO...",
    "..O.OOOO.O..",
    ".O........O."
  }
};

struct GlyphDef {
  char c;
  uint8_t rows[7];
};

const GlyphDef simpleFont[] = {
  {' ', {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
  {'!', {0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04}},
  {'-', {0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00}},
  {'.', {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04}},
  {':', {0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00}},
  {'0', {0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E}},
  {'1', {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E}},
  {'2', {0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F}},
  {'3', {0x1E, 0x01, 0x01, 0x0E, 0x01, 0x01, 0x1E}},
  {'4', {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02}},
  {'5', {0x1F, 0x10, 0x10, 0x1E, 0x01, 0x01, 0x1E}},
  {'6', {0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E}},
  {'7', {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08}},
  {'8', {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E}},
  {'9', {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C}},
  {'?', {0x0E, 0x11, 0x01, 0x02, 0x04, 0x00, 0x04}},
  {'A', {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}},
  {'B', {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E}},
  {'C', {0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E}},
  {'D', {0x1C, 0x12, 0x11, 0x11, 0x11, 0x12, 0x1C}},
  {'E', {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F}},
  {'F', {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10}},
  {'G', {0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0F}},
  {'H', {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}},
  {'I', {0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E}},
  {'J', {0x01, 0x01, 0x01, 0x01, 0x11, 0x11, 0x0E}},
  {'K', {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11}},
  {'L', {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F}},
  {'M', {0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11}},
  {'N', {0x11, 0x11, 0x19, 0x15, 0x13, 0x11, 0x11}},
  {'O', {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}},
  {'P', {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10}},
  {'Q', {0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D}},
  {'R', {0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11}},
  {'S', {0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E}},
  {'T', {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04}},
  {'U', {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}},
  {'V', {0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04}},
  {'W', {0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0A}},
  {'X', {0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11}},
  {'Y', {0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04}},
  {'Z', {0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F}}
};

const char supportedChars[] = " !-.:0123456789?ABCDEFGHIJKLMNOPQRSTUVWXYZ";


// Feature implementations are organized into the numbered Arduino tabs.
// See README.md in this sketch directory for the tab responsibility map.

void setup() {
  Serial.begin(115200);
  delay(300);
  initClockTime();
  clockText = getCurrentClockText();
 
  mxconfig.i2sspeed = PANEL_I2S_SPEED;
  mxconfig.clkphase = PANEL_CLK_PHASE;
 
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  if (!dma_display->begin()) {
    Serial.println("DMA display begin() failed");
    while (1) delay(100);
  }
 
  dma_display->setBrightness8(80);
  dma_display->clearScreen();
 
  matrix = new VirtualMatrixPanel(*dma_display,
                                  VIRTUAL_PANEL_ROWS,
                                  VIRTUAL_PANEL_COLS,
                                  PANEL_RES_X,
                                  PANEL_RES_Y);
 
  // Important for 32x16 1/4-scan panels
  matrix->setPhysicalPanelScanRate(FOUR_SCAN_16PX_HIGH);
 
  RED     = matrix->color565(255, 0, 0);
  GREEN   = matrix->color565(0, 255, 0);
  BLUE    = matrix->color565(0, 0, 255);
  WHITE   = matrix->color565(255, 255, 255);
  BLACK   = matrix->color565(0, 0, 0);
  YELLOW  = matrix->color565(255, 255, 0);
  CYAN    = matrix->color565(0, 255, 255);
  MAGENTA = matrix->color565(255, 0, 255);
 
  buildInverseMap();
  matrix->fillScreen(BLACK);
  Serial.println("Custom panel remap enabled from measured scan pattern.");
  Serial.println("Note: text rendering still uses the library's native mapping.");
  printHelp();
}

void testPixelSweepSingle(uint16_t delayMs) {
  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      matrix->fillScreen(BLACK);
      drawPixelMapped(x, y, RED);

      Serial.print("Pixel: ");
      Serial.print(x);
      Serial.print(", ");
      Serial.println(y);

      delay(delayMs);
    }
  }

  matrix->fillScreen(BLACK);
}
 
bool isGameMode() {
  return currentMode == MODE_DINO || currentMode == MODE_FLAPPY || currentMode == MODE_PONG || currentMode == MODE_BREAKOUT ||
         currentMode == MODE_SNAKE || currentMode == MODE_INVADERS || currentMode == MODE_RHYTHM;
}

void handleGameKey(char key) {
  if (key == '\r' || key == '\n') return;
  if (key >= 'A' && key <= 'Z') key += 'a' - 'A';
  if (!isGameMode()) return;

  if (key == 'c') {
      currentMode = MODE_HELP;
      matrix->fillScreen(BLACK);
      Serial.println("Game closed.");
      printHelp();
      return;
  }

  if (currentMode == MODE_DINO) {
    jumpDino();
  } else if (currentMode == MODE_FLAPPY) {
    flapBird();
  } else if (currentMode == MODE_BREAKOUT) {
    if (key == 'a') moveBreakoutPaddle(-1);
    if (key == 'd') moveBreakoutPaddle(1);
  } else if (currentMode == MODE_SNAKE) {
    if (key == 'w') steerSnake(0, -1);
    if (key == 'a') steerSnake(-1, 0);
    if (key == 's') steerSnake(0, 1);
    if (key == 'd') steerSnake(1, 0);
  } else if (currentMode == MODE_INVADERS) {
    if (key == 'a') controlInvaders(-1, false);
    else if (key == 'd') controlInvaders(1, false);
    else controlInvaders(0, true);
  } else if (currentMode == MODE_RHYTHM) {
    if (key == 'a') hitRhythmLane(0);
    if (key == 's') hitRhythmLane(1);
    if (key == 'k') hitRhythmLane(2);
    if (key == 'l') hitRhythmLane(3);
  }
}

void loop() {
  if (Serial.available()) {
    if (isGameMode()) {
      while (Serial.available()) handleGameKey((char)Serial.read());
    } else {
      String input = Serial.readStringUntil('\n');
      clearSerialInput();
      handleCommand(input);
    }
  }

  if (currentMode == MODE_AURORA) {
    drawAuroraFrame(millis());
    delay(20);
  }

  if (currentMode == MODE_LIFE) {
    drawLifeFrame();
    delay(120);
  }

  if (currentMode == MODE_RAINBOW) {
    drawRainbowFrame(millis());
    delay(20);
  }

  if (currentMode == MODE_MATRIX_RAIN) {
    drawMatrixRainFrame();
    delay(60);
  }

  if (currentMode == MODE_FIRE) {
    drawFireFrame();
    delay(45);
  }

  if (currentMode == MODE_PLASMA) {
    drawPlasmaFrame(millis());
    delay(20);
  }

  if (currentMode == MODE_PLASMA_CLOCK) {
    drawPlasmaClockFrame(millis());
    delay(20);
  }

  if (currentMode == MODE_RAIN_CLOCK) {
    drawRainClockFrame(millis());
    delay(45);
  }

  if (currentMode == MODE_STAR_CLOCK) {
    drawStarClockFrame(millis());
    delay(45);
  }

  if (currentMode == MODE_AQUARIUM_CLOCK) {
    drawAquariumClockFrame(millis());
    delay(55);
  }

  if (currentMode == MODE_MATRIX_CLOCK) {
    drawMatrixClockFrame(millis());
    delay(55);
  }

  if (currentMode == MODE_TETRIS_CLOCK) {
    drawTetrisClockFrame(millis());
    delay(80);
  }

  if (currentMode == MODE_ORBIT_CLOCK) {
    drawOrbitClockFrame(millis());
    delay(35);
  }

  if (currentMode == MODE_DINO_CLOCK) {
    drawDinoClockFrame(millis());
    delay(35);
  }

  if (currentMode == MODE_TWINKLE) {
    drawTwinkleFrame(millis());
    delay(30);
  }

  if (currentMode == MODE_SOUNDBAR) {
    drawSoundbarFrame(millis());
    delay(45);
  }

  if (currentMode == MODE_FLUID) {
    drawFluidFrame(millis());
    delay(25);
  }

  if (currentMode == MODE_FIREWORKS) {
    drawFireworksFrame(millis());
    delay(40);
  }

  if (currentMode == MODE_SUPERNOVA) {
    drawSupernovaFrame(millis());
    delay(35);
  }

  if (currentMode == MODE_VORTEX) {
    drawVortexFrame(millis());
    delay(20);
  }

  if (currentMode == MODE_LAVA) {
    drawLavaFrame(millis());
    delay(30);
  }

  if (currentMode == MODE_METEOR) {
    drawMeteorFrame(millis());
    delay(30);
  }

  if (currentMode == MODE_STORM) {
    drawStormFrame(millis());
    delay(35);
  }

  if (currentMode == MODE_GALAXY) {
    drawGalaxyFrame(millis());
    delay(25);
  }

  if (currentMode == MODE_CATRUN) {
    drawCatRunFrame(millis());
    delay(80);
  }

  if (currentMode == MODE_SQUAREBURST) {
    drawSquareBurstFrame(millis());
    delay(45);
  }

  if (currentMode == MODE_SMOKE) {
    drawSmokeFrame(millis());
    delay(25);
  }

  if (currentMode == MODE_BLOBS) {
    drawBlobsFrame(millis());
    delay(30);
  }

  if (currentMode == MODE_WINDFIRE) {
    drawWindFireFrame(millis());
    delay(35);
  }

  if (currentMode == MODE_RIPPLES) {
    drawRipplesFrame(millis());
    delay(25);
  }

  if (currentMode == MODE_CIRCUIT) {
    drawCircuitFrame(millis());
    delay(25);
  }

  if (currentMode == MODE_RADAR) {
    drawRadarFrame(millis());
    delay(25);
  }

  if (currentMode == MODE_STYLERAIN) {
    drawStyleRainFrame(millis());
    delay(35);
  }

  if (currentMode == MODE_NEURAL) {
    drawNeuralFrame(millis());
    delay(30);
  }

  if (currentMode == MODE_TUNNEL) {
    drawTunnelFrame(millis());
    delay(22);
  }

  if (currentMode == MODE_CUBE) {
    drawCubeFrame(millis());
    delay(35);
  }

  if (currentMode == MODE_KALEIDO) {
    drawKaleidoFrame(millis());
    delay(25);
  }

  if (currentMode == MODE_SPIRAL) {
    drawSpiralFrame(millis());
    delay(22);
  }

  if (currentMode == MODE_INTERFERENCE) {
    drawInterferenceFrame(millis());
    delay(25);
  }

  if (currentMode == MODE_TESSELLATE) {
    drawTessellateFrame(millis());
    delay(28);
  }

  if (currentMode == MODE_BEATBURST) {
    drawBeatBurstFrame(millis());
    delay(28);
  }

  if (currentMode == MODE_SUNRISE) {
    drawSunriseFrame(millis());
    delay(60);
  }

  if (currentMode == MODE_PACMAN) {
    drawPacmanFrame(millis());
    delay(70);
  }

  if (currentMode == MODE_DINO) {
    drawDinoFrame(millis());
    delay(10);
  }

  if (currentMode == MODE_FLAPPY) {
    drawFlappyFrame(millis());
    delay(10);
  }

  if (currentMode == MODE_PONG) {
    drawPongFrame(millis());
    delay(10);
  }

  if (currentMode == MODE_BREAKOUT) {
    drawBreakoutFrame(millis());
    delay(10);
  }

  if (currentMode == MODE_SNAKE) {
    drawSnakeFrame(millis());
    delay(10);
  }

  if (currentMode == MODE_INVADERS) {
    drawInvadersFrame(millis());
    delay(10);
  }

  if (currentMode == MODE_RHYTHM) {
    drawRhythmFrame(millis());
    delay(10);
  }

  if (currentMode == MODE_SPHERE) {
    drawSphereFrame(millis());
    delay(30);
  }

  if (currentMode == MODE_PARALLAX) {
    drawParallaxFrame(millis());
    delay(22);
  }

  if (currentMode == MODE_MOIRE) {
    drawMoireFrame(millis());
    delay(22);
  }

  if (currentMode == MODE_HYPNOSIS) {
    drawHypnosisFrame(millis());
    delay(22);
  }

  if (currentMode == MODE_CAFEWALL) {
    drawCafeWallFrame(millis());
    delay(35);
  }

  if (currentMode == MODE_PINWHEEL) {
    drawPinwheelFrame(millis());
    delay(22);
  }

  if (currentMode == MODE_PETALWARP) {
    drawPetalWarpFrame(millis());
    delay(22);
  }

  if (currentMode == MODE_TWISTSQUARE) {
    drawTwistSquareFrame(millis());
    delay(24);
  }

  if (currentMode == MODE_EVENTHORIZON) {
    drawEventHorizonFrame(millis());
    delay(22);
  }

  if (currentMode == MODE_MORPH) {
    drawMorphFrame(millis());
    delay(40);
  }

  if (currentMode == MODE_DVD) {
    drawDvdFrame(millis());
    delay(40);
  }

  if (currentMode == MODE_TVBARS) {
    drawTvBarsFrame(millis());
    delay(30);
  }

  if (currentMode == MODE_TVSTATIC) {
    drawColorTvStaticFrame(millis());
    delay(24);
  }

  if (currentMode == MODE_SINEWAVE) {
    drawSineWaveFrame(millis());
    delay(25);
  }

  if (currentMode == MODE_NOISEFLOW) {
    drawNoiseFlowFrame(millis());
    delay(25);
  }

  if (currentMode == MODE_PHASEBEAT) {
    drawPhaseBeatFrame(millis());
    delay(25);
  }

  if (currentMode == MODE_LISSAJOUS) {
    drawLissajousFrame(millis());
    delay(25);
  }

  if (currentMode == MODE_HARMONICS) {
    drawHarmonicsFrame(millis());
    delay(25);
  }

  if (currentMode == MODE_SILK) {
    drawSilkFrame(millis());
    delay(25);
  }
  if (currentMode == MODE_SILK_CLOCK) {
    drawSilkClockFrame(millis());
    delay(25);
  }
  if (currentMode == MODE_DEEPSEA) {
    drawDeepSeaFrame(millis());
    delay(25);
  }

  if (currentMode == MODE_SCROLL_TEXT) {
    drawScrollTextFrame(millis());
  }

  if (currentMode == MODE_DRAW_TEXT) {
    drawTextWritingFrame(millis());
  }

  if (currentMode == MODE_EMOJI) {
    drawEmojiFrame(millis());
  }
}
