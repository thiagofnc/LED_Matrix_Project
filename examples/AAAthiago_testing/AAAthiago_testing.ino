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

bool parseBuildTime(tm& outTm) {
  const char* months = "JanFebMarAprMayJunJulAugSepOctNovDec";
  char monthStr[4];
  int day;
  int year;
  int hour;
  int minute;
  int second;

  if (sscanf(__DATE__, "%3s %d %d", monthStr, &day, &year) != 3) {
    return false;
  }
  if (sscanf(__TIME__, "%d:%d:%d", &hour, &minute, &second) != 3) {
    return false;
  }

  char* monthPtr = strstr(months, monthStr);
  if (monthPtr == nullptr) {
    return false;
  }

  memset(&outTm, 0, sizeof(outTm));
  outTm.tm_year = year - 1900;
  outTm.tm_mon = (monthPtr - months) / 3;
  outTm.tm_mday = day;
  outTm.tm_hour = hour;
  outTm.tm_min = minute;
  outTm.tm_sec = second;
  outTm.tm_isdst = -1;
  return true;
}

void initClockTime() {
  setenv("TZ", PANEL_TIMEZONE, 1);
  tzset();

  tm buildTm;
  if (!parseBuildTime(buildTm)) {
    return;
  }

  time_t buildEpoch = mktime(&buildTm);
  if (buildEpoch <= 0) {
    return;
  }

  timeval now = { buildEpoch, 0 };
  settimeofday(&now, nullptr);
}

bool setClockFromString(const String& value) {
  int h = 0;
  int m = 0;
  int s = 0;
  int parts = 0;

  if (sscanf(value.c_str(), "%d:%d:%d%n", &h, &m, &s, &parts) >= 2 && parts == value.length()) {
    // Parsed HH:MM:SS
  } else {
    s = 0;
    if (sscanf(value.c_str(), "%d:%d%n", &h, &m, &parts) < 2 || parts != value.length()) {
      return false;
    }
  }

  if (h < 0 || h > 23 || m < 0 || m > 59 || s < 0 || s > 59) {
    return false;
  }

  time_t nowSecs = time(nullptr);
  tm localNow;
  localtime_r(&nowSecs, &localNow);
  localNow.tm_hour = h;
  localNow.tm_min = m;
  localNow.tm_sec = s;
  localNow.tm_isdst = -1;

  time_t newEpoch = mktime(&localNow);
  if (newEpoch <= 0) {
    return false;
  }

  timeval now = { newEpoch, 0 };
  settimeofday(&now, nullptr);
  return true;
}

String getCurrentClockText() {
  time_t nowSecs = time(nullptr);
  tm localNow;
  localtime_r(&nowSecs, &localNow);

  char buf[6];
  snprintf(buf, sizeof(buf), "%02d:%02d", localNow.tm_hour, localNow.tm_min);
  return String(buf);
}

RemapPoint builtInMapPrediction(int x, int y) {
  int chunk = x / 8;
  int off = x % 8;

  // Original derived mapping kept here for reference:
  // int xBase = (y & 0x04) ? 16 : 0;
  // int yBase = (y & 0x08) ? 0 : 8;
  // int row = y & 0x03;
  // RemapPoint pt;
  // pt.x = xBase + ((chunk < 2) ? 8 : 0) + ((chunk & 1) ? off : (7 - off));
  // pt.y = yBase + ((chunk & 1) ? (7 - row) : (3 - row));
  // return pt;

  int xBase = (y & 0x04) ? 16 : 0;
  int yBase = (y & 0x08) ? 0 : 8;
  int row = y & 0x03;

  RemapPoint pt;
  pt.x = xBase + ((chunk < 2) ? 8 : 0) + ((chunk & 1) ? off : (7 - off));
  pt.y = yBase + ((chunk & 1) ? (7 - row) : (3 - row));
  return pt;
}

void swapInverseMapEntries(int ax, int ay, int bx, int by) {
  RemapPoint temp = inverseMap[ay][ax];
  inverseMap[ay][ax] = inverseMap[by][bx];
  inverseMap[by][bx] = temp;
}

void buildInverseMap() {
  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      inverseMap[y][x] = {0, 0};
    }
  }

  for (int logicalY = 0; logicalY < PANEL_RES_Y; logicalY++) {
    for (int logicalX = 0; logicalX < PANEL_RES_X; logicalX++) {
      RemapPoint phys = builtInMapPrediction(logicalX, logicalY);
      if (phys.x < PANEL_RES_X && phys.y < PANEL_RES_Y) {
        inverseMap[phys.y][phys.x] = {(uint8_t)logicalX, (uint8_t)logicalY};
      }
    }
  }
}

void drawPixelMapped(int x, int y, uint16_t color) {
  if (x < 0 || x >= PANEL_RES_X || y < 0 || y >= PANEL_RES_Y) {
    return;
  }

  RemapPoint input = inverseMap[y][x];
  matrix->drawPixel(input.x, input.y, color);
}

void drawPixelMappedRotated(int x, int y, uint16_t color) {
  if (x < 0 || x >= ROTATED_RES_X || y < 0 || y >= ROTATED_RES_Y) {
    return;
  }

  // Treat the display as a portrait 16x32 surface by rotating logical
  // coordinates clockwise into the native 32x16 landscape space.
  int nativeX = y;
  int nativeY = PANEL_RES_Y - 1 - x;
  drawPixelMapped(nativeX, nativeY, color);
}
 
void showSolidColor(uint16_t color, uint16_t msDelay) {
  matrix->fillScreen(color);
  delay(msDelay);
  matrix->fillScreen(BLACK);
}
 
void testCorners(uint16_t msDelay) {
  matrix->fillScreen(BLACK);
 
  drawPixelMapped(0, 0, RED);                         // top-left
  drawPixelMapped(PANEL_RES_X - 1, 0, GREEN);        // top-right
  drawPixelMapped(0, PANEL_RES_Y - 1, BLUE);         // bottom-left
  drawPixelMapped(PANEL_RES_X - 1, PANEL_RES_Y - 1, WHITE); // bottom-right
 
  delay(msDelay);
  matrix->fillScreen(BLACK);
}

void testRowsSlow() {
  for (int y = 0; y < PANEL_RES_Y; y++) {
    matrix->fillScreen(BLACK);
    for (int x = 0; x < PANEL_RES_X; x++) {
      drawPixelMapped(x, y, RED);
    }
    Serial.printf("Logical row %d\n", y);
    delay(2000);
  }
  matrix->fillScreen(BLACK);
}

void testColumnSweepOnRow(int y) {
  for (int x = 0; x < PANEL_RES_X; x++) {
    matrix->fillScreen(BLACK);
    drawPixelMapped(x, y, GREEN);
    // Serial.printf("Logical pixel (%d,%d)\n", x, y);
    delay(1000);
  }
  matrix->fillScreen(BLACK);
}

void showRowStep(int y) {
  matrix->fillScreen(BLACK);
  for (int x = 0; x < PANEL_RES_X; x++) {
    drawPixelMapped(x, y, RED);
  }
  Serial.printf("ROW logical y=%d. Record the physical row(s) that light.\n", y);
}

void showColumnStep(int y, int x) {
  matrix->fillScreen(BLACK);
  drawPixelMapped(x, y, GREEN);
  Serial.printf("COL logical (%d,%d). Record where this pixel appears.\n", x, y);
}

void showSplitRowStep(int y) {
  matrix->fillScreen(BLACK);
  for (int x = 0; x < 16; x++) {
    drawPixelMapped(x, y, RED);
  }
  for (int x = 16; x < PANEL_RES_X; x++) {
    drawPixelMapped(x, y, GREEN);
  }
  Serial.printf("SPLIT logical row y=%d. RED is x=0..15, GREEN is x=16..31.\n", y);
}

void showChunkRowStep(int y) {
  matrix->fillScreen(BLACK);

  for (int x = 0; x < 8; x++) {
    drawPixelMapped(x, y, RED);
  }
  for (int x = 8; x < 16; x++) {
    drawPixelMapped(x, y, GREEN);
  }
  for (int x = 16; x < 24; x++) {
    drawPixelMapped(x, y, BLUE);
  }
  for (int x = 24; x < 32; x++) {
    drawPixelMapped(x, y, WHITE);
  }

  Serial.printf("CHUNKS logical row y=%d. RED=0..7 GREEN=8..15 BLUE=16..23 WHITE=24..31.\n", y);
}

void showSingleChunk(int y, int chunkIndex) {
  if (chunkIndex < 0 || chunkIndex > 3) {
    return;
  }

  matrix->fillScreen(BLACK);

  int startX = chunkIndex * 8;
  uint16_t color = RED;
  if (chunkIndex == 1) color = GREEN;
  else if (chunkIndex == 2) color = BLUE;
  else if (chunkIndex == 3) color = WHITE;

  for (int x = startX; x < startX + 8; x++) {
    drawPixelMapped(x, y, color);
  }

  Serial.printf("CHUNK logical row y=%d chunk=%d covering x=%d..%d.\n",
                y, chunkIndex, startX, startX + 7);
}

void showSingleChunkPixel(int y, int chunkIndex, int offset) {
  if (chunkIndex < 0 || chunkIndex > 3 || offset < 0 || offset > 7) {
    return;
  }

  matrix->fillScreen(BLACK);
  int x = chunkIndex * 8 + offset;
  drawPixelMapped(x, y, MAGENTA);
  Serial.printf("CHUNKPX logical row y=%d chunk=%d offset=%d logical x=%d.\n",
                y, chunkIndex, offset, x);
}

void showSinglePixel(int x, int y) {
  matrix->fillScreen(BLACK);
  drawPixelMapped(x, y, WHITE);
  Serial.printf("PIXEL logical (%d,%d). Record the physical location.\n", x, y);
}

uint16_t colorWheel(uint8_t pos) {
  pos = 255 - pos;
  if (pos < 85) {
    return matrix->color565(255 - pos * 3, 0, pos * 3);
  }
  if (pos < 170) {
    pos -= 85;
    return matrix->color565(0, pos * 3, 255 - pos * 3);
  }
  pos -= 170;
  return matrix->color565(pos * 3, 255 - pos * 3, 0);
}

uint16_t heatColor(uint8_t heat) {
  if (heat > 200) {
    return matrix->color565(255, 255, (heat - 200) * 4);
  }
  if (heat > 120) {
    return matrix->color565(255, (heat - 120) * 3, 0);
  }
  return matrix->color565(heat * 2, 0, 0);
}

const uint8_t* getGlyphRows(char c) {
  if (c >= 'a' && c <= 'z') {
    c = c - 'a' + 'A';
  }

  for (size_t i = 0; i < (sizeof(simpleFont) / sizeof(simpleFont[0])); i++) {
    if (simpleFont[i].c == c) {
      return simpleFont[i].rows;
    }
  }

  for (size_t i = 0; i < (sizeof(simpleFont) / sizeof(simpleFont[0])); i++) {
    if (simpleFont[i].c == '?') {
      return simpleFont[i].rows;
    }
  }

  return simpleFont[0].rows;
}

void drawMappedChar(int x, int y, char c, uint16_t color, uint8_t scale = 1) {
  const uint8_t* rows = getGlyphRows(c);

  for (int row = 0; row < 7; row++) {
    for (int col = 0; col < 5; col++) {
      if (rows[row] & (1 << (4 - col))) {
        for (int sy = 0; sy < scale; sy++) {
          for (int sx = 0; sx < scale; sx++) {
            drawPixelMapped(x + col * scale + sx, y + row * scale + sy, color);
          }
        }
      }
    }
  }
}

void drawMappedText(int x, int y, const String& text, uint16_t color, uint8_t scale = 1) {
  int cursorX = x;
  for (size_t i = 0; i < text.length(); i++) {
    drawMappedChar(cursorX, y, text[i], color, scale);
    cursorX += 6 * scale;
  }
}

void drawMappedCharRotated(int x, int y, char c, uint16_t color, uint8_t scale = 1) {
  const uint8_t* rows = getGlyphRows(c);

  for (int row = 0; row < 7; row++) {
    for (int col = 0; col < 5; col++) {
      if (rows[row] & (1 << (4 - col))) {
        for (int sy = 0; sy < scale; sy++) {
          for (int sx = 0; sx < scale; sx++) {
            drawPixelMappedRotated(x + col * scale + sx, y + row * scale + sy, color);
          }
        }
      }
    }
  }

  // Manual corrections based on chunk tests. The base mapping gets almost
  // everything right, but this panel cross-couples a few right-half edge
  // pixels between neighboring 8x4 regions. Swapping the ownership of these
  // physical pixels cleans up the visible glitches without disturbing the
  // rest of the map.
  swapInverseMapEntries(31, 4, 24, 0);
  swapInverseMapEntries(31, 12, 16, 8);
}

void showTextMessage(const String& message) {
  matrix->fillScreen(BLACK);

  uint8_t scale = 1;
  int textWidth = message.length() * 6 * scale - 1;
  int startX = 0;
  if (textWidth < PANEL_RES_X) {
    startX = (PANEL_RES_X - textWidth) / 2;
  }

  int startY = (PANEL_RES_Y - (7 * scale)) / 2;
  if (startY < 0) {
    startY = 0;
  }

  drawMappedText(startX, startY, message, WHITE, scale);
}

void drawScrollTextFrame(uint32_t now) {
  if (now - scrollTextLastStep < 55) return;
  scrollTextLastStep = now;

  matrix->fillScreen(BLACK);
  int startY = (PANEL_RES_Y - 7) / 2;
  drawMappedText(scrollTextX, startY, textMessage, colorWheel((uint8_t)(now / 18)));

  scrollTextX--;
  int textWidth = (int)textMessage.length() * 6 - 1;
  if (scrollTextX < -textWidth) scrollTextX = PANEL_RES_X;
}

void drawTextWritingFrame(uint32_t now) {
  if (now - drawTextLastStep < 45) return;
  drawTextLastStep = now;

  if (textMessage.length() == 0) textMessage = " ";
  char c = textMessage[drawTextCharIndex];
  const uint8_t* rows = getGlyphRows(c);
  const uint8_t scale = 2;
  const int startX = (PANEL_RES_X - 5 * scale) / 2;
  const int startY = (PANEL_RES_Y - 7 * scale) / 2;

  if (drawTextPixelIndex == 0) matrix->fillScreen(BLACK);

  int scanIndex = drawTextPixelIndex;
  int row = scanIndex / 5;
  int col = scanIndex % 5;
  if (row < 7 && (rows[row] & (1 << (4 - col)))) {
    uint16_t color = colorWheel((uint8_t)(drawTextCharIndex * 47 + scanIndex * 5));
    for (int sy = 0; sy < scale; sy++) {
      for (int sx = 0; sx < scale; sx++) {
        drawPixelMapped(startX + col * scale + sx, startY + row * scale + sy, color);
      }
    }
  }

  drawTextPixelIndex++;
  if (drawTextPixelIndex >= 35) {
    drawTextPixelIndex = 0;
    drawTextCharIndex = (drawTextCharIndex + 1) % textMessage.length();
  }
}

void drawEmojiFace(uint8_t expression) {
  const int cx = PANEL_RES_X / 2;
  const int cy = PANEL_RES_Y / 2;
  uint16_t yellow = matrix->color565(255, 205, 20);

  for (int y = -7; y <= 7; y++) {
    for (int x = -7; x <= 7; x++) {
      if (x * x + y * y <= 49) drawPixelMapped(cx + x, cy + y, yellow);
    }
  }

  drawPixelMapped(cx - 3, cy - 2, BLACK);
  if (expression == 1) {
    drawPixelMapped(cx + 2, cy - 2, BLACK);
    drawPixelMapped(cx + 3, cy - 2, BLACK);
  } else {
    drawPixelMapped(cx + 3, cy - 2, BLACK);
  }

  if (expression == 2) {
    drawPixelMapped(cx, cy + 2, BLACK);
    drawPixelMapped(cx - 1, cy + 3, BLACK);
    drawPixelMapped(cx, cy + 3, BLACK);
    drawPixelMapped(cx + 1, cy + 3, BLACK);
    drawPixelMapped(cx, cy + 4, BLACK);
  } else {
    drawPixelMapped(cx - 3, cy + 2, BLACK);
    drawPixelMapped(cx - 2, cy + 3, BLACK);
    drawPixelMapped(cx - 1, cy + 4, BLACK);
    drawPixelMapped(cx, cy + 4, BLACK);
    drawPixelMapped(cx + 1, cy + 4, BLACK);
    drawPixelMapped(cx + 2, cy + 3, BLACK);
    drawPixelMapped(cx + 3, cy + 2, BLACK);
  }
}

void drawHeartEmoji() {
  static const uint16_t heartRows[12] = {
    0x0000, 0x0C60, 0x1EF0, 0x3FF8, 0x3FF8, 0x1FF0,
    0x0FE0, 0x07C0, 0x0380, 0x0100, 0x0000, 0x0000
  };
  uint16_t red = matrix->color565(255, 25, 70);
  int startX = (PANEL_RES_X - 14) / 2;
  int startY = 2;
  for (int y = 0; y < 12; y++) {
    for (int x = 0; x < 14; x++) {
      if (heartRows[y] & (1 << (13 - x))) drawPixelMapped(startX + x, startY + y, red);
    }
  }
}

void drawEmojiFrame(uint32_t now) {
  if (emojiLastChange != 0 && now - emojiLastChange < emojiDelayMs) return;
  emojiLastChange = now;
  matrix->fillScreen(BLACK);

  if (emojiIndex == 1) drawHeartEmoji();
  else drawEmojiFace(emojiIndex == 2 ? 1 : (emojiIndex == 3 ? 2 : 0));

  emojiIndex = (emojiIndex + 1) % 4;
}

void showRotatedSingleChar(char c) {
  matrix->fillScreen(BLACK);

  uint8_t scale = 3;
  int charWidth = 5 * scale;
  int charHeight = 7 * scale;
  int startX = (ROTATED_RES_X - charWidth) / 2;
  int startY = (ROTATED_RES_Y - charHeight) / 2;

  if (startX < 0) {
    startX = 0;
  }
  if (startY < 0) {
    startY = 0;
  }

  drawMappedCharRotated(startX, startY, c, WHITE, scale);
}

void testRotatedCharSet(uint16_t delayMs) {
  for (size_t i = 0; i < sizeof(supportedChars) - 1; i++) {
    char c = supportedChars[i];
    showRotatedSingleChar(c);
    Serial.print("RCHAR test: '");
    Serial.print(c);
    Serial.println("'");
    delay(delayMs);
  }

  matrix->fillScreen(BLACK);
}

uint16_t scaleColor565(uint16_t color, uint8_t brightness) {
  uint8_t r = (((color >> 11) & 0x1F) << 3);
  uint8_t g = (((color >> 5) & 0x3F) << 2);
  uint8_t b = ((color & 0x1F) << 3);

  r = (uint8_t)((r * brightness) / 255);
  g = (uint8_t)((g * brightness) / 255);
  b = (uint8_t)((b * brightness) / 255);
  return matrix->color565(r, g, b);
}

void drawSoftGlow(int cx, int cy, float radius, uint16_t color, uint8_t brightness) {
  if (radius <= 0.0f || brightness == 0) {
    return;
  }

  int minX = max(0, (int)(cx - radius - 1));
  int maxX = min(PANEL_RES_X - 1, (int)(cx + radius + 1));
  int minY = max(0, (int)(cy - radius - 1));
  int maxY = min(PANEL_RES_Y - 1, (int)(cy + radius + 1));

  for (int y = minY; y <= maxY; y++) {
    for (int x = minX; x <= maxX; x++) {
      float dx = x - cx;
      float dy = y - cy;
      float dist = sqrtf(dx * dx + dy * dy);
      if (dist > radius) {
        continue;
      }

      float falloff = 1.0f - (dist / radius);
      uint8_t localBrightness = (uint8_t)(brightness * falloff);
      if (localBrightness < 12) {
        continue;
      }
      drawPixelMapped(x, y, scaleColor565(color, localBrightness));
    }
  }
}

void drawLineMapped(int x0, int y0, int x1, int y1, uint16_t color) {
  int dx = abs(x1 - x0);
  int sx = (x0 < x1) ? 1 : -1;
  int dy = -abs(y1 - y0);
  int sy = (y0 < y1) ? 1 : -1;
  int err = dx + dy;

  while (true) {
    drawPixelMapped(x0, y0, color);
    if (x0 == x1 && y0 == y1) {
      break;
    }

    int e2 = err * 2;
    if (e2 >= dy) {
      err += dy;
      x0 += sx;
    }
    if (e2 <= dx) {
      err += dx;
      y0 += sy;
    }
  }
}

void drawCatFrame(int originX, int originY, uint8_t frameIndex) {
  uint8_t frame = frameIndex & 0x03;
  uint16_t orange = matrix->color565(255, 150, 40);
  uint16_t cream = matrix->color565(255, 245, 220);
  uint16_t pink = matrix->color565(255, 120, 170);
  uint16_t outline = matrix->color565(40, 20, 10);

  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 12; x++) {
      char pixel = catFrames[frame][y][x];
      if (pixel == '.') {
        continue;
      }

      uint16_t color = orange;
      if (pixel == 'W') {
        color = cream;
      } else if (pixel == 'P') {
        color = pink;
      } else if (pixel == 'B') {
        color = outline;
      }

      drawPixelMapped(originX + x, originY + y, color);
    }
  }
}

void drawAuroraFrame(uint32_t t) {
  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      float v1 = sinf((x * 0.35f) + (t * 0.0025f));
      float v2 = sinf((y * 0.50f) - (t * 0.0018f));
      float v3 = sinf(((x + y) * 0.28f) + (t * 0.0012f));
      float mix = (v1 + v2 + v3) / 3.0f;

      uint8_t r = 0;
      uint8_t g = (uint8_t)(80 + 100 * (0.5f + 0.5f * mix));
      uint8_t b = (uint8_t)(40 + 180 * (0.5f + 0.5f * sinf(mix * 3.14f + t * 0.001f)));

      drawPixelMapped(x, y, matrix->color565(r, g, b));
    }
  }
}

void seedLife() {
  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      lifeCurrent[y][x] = (random(100) < 35);
      lifeNext[y][x] = false;
    }
  }
  lifeGeneration = 0;
}

int countLifeNeighbors(int x, int y) {
  int count = 0;
  for (int dy = -1; dy <= 1; dy++) {
    for (int dx = -1; dx <= 1; dx++) {
      if (dx == 0 && dy == 0) {
        continue;
      }
      int nx = (x + dx + PANEL_RES_X) % PANEL_RES_X;
      int ny = (y + dy + PANEL_RES_Y) % PANEL_RES_Y;
      if (lifeCurrent[ny][nx]) {
        count++;
      }
    }
  }
  return count;
}

void drawLifeFrame() {
  bool anyAlive = false;

  matrix->fillScreen(BLACK);
  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      if (lifeCurrent[y][x]) {
        uint8_t hue = (uint8_t)((x * 7 + y * 11 + lifeGeneration * 5) & 0xFF);
        drawPixelMapped(x, y, colorWheel(hue));
        anyAlive = true;
      }
    }
  }

  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      int neighbors = countLifeNeighbors(x, y);
      bool alive = lifeCurrent[y][x];
      lifeNext[y][x] = (alive && (neighbors == 2 || neighbors == 3)) || (!alive && neighbors == 3);
    }
  }

  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      lifeCurrent[y][x] = lifeNext[y][x];
      lifeNext[y][x] = false;
    }
  }

  lifeGeneration++;

  if (!anyAlive || (lifeGeneration % 128 == 0)) {
    seedLife();
  }
}

void drawRainbowFrame(uint32_t t) {
  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      float waveA = sinf((x * 0.40f) + (t * 0.0030f));
      float waveB = cosf((y * 0.55f) - (t * 0.0020f));
      float waveC = sinf(((x + y) * 0.22f) + (t * 0.0015f));
      int hue = (int)(128.0f + 70.0f * waveA + 40.0f * waveB + 18.0f * waveC);
      drawPixelMapped(x, y, colorWheel((uint8_t)hue));
    }
  }
}

void seedMatrixRain() {
  for (int x = 0; x < PANEL_RES_X; x++) {
    rainHead[x] = random(-PANEL_RES_Y, PANEL_RES_Y);
    rainLength[x] = 3 + random(6);
    rainSpeed[x] = 1 + random(3);
  }
  rainTick = 0;
}

void drawMatrixRainFrame() {
  matrix->fillScreen(BLACK);
  rainTick++;

  for (int x = 0; x < PANEL_RES_X; x++) {
    if ((rainTick % rainSpeed[x]) == 0) {
      rainHead[x]++;
      if (rainHead[x] - rainLength[x] > PANEL_RES_Y) {
        rainHead[x] = -random(PANEL_RES_Y);
        rainLength[x] = 3 + random(6);
        rainSpeed[x] = 1 + random(3);
      }
    }

    for (int i = 0; i < rainLength[x]; i++) {
      int y = rainHead[x] - i;
      if (y < 0 || y >= PANEL_RES_Y) {
        continue;
      }

      uint16_t color;
      if (i == 0) {
        color = matrix->color565(180, 255, 180);
      } else if (i == 1) {
        color = matrix->color565(40, 255, 80);
      } else {
        uint8_t g = 180 - (i * 20);
        if (g < 20) {
          g = 20;
        }
        color = matrix->color565(0, g, 0);
      }

      drawPixelMapped(x, y, color);
    }
  }
}

void seedFire() {
  for (int x = 0; x < PANEL_RES_X; x++) {
    for (int y = 0; y < PANEL_RES_Y; y++) {
      fireHeat[x][y] = 0;
    }
  }
}

void drawFireFrame() {
  for (int x = 0; x < PANEL_RES_X; x++) {
    fireHeat[x][PANEL_RES_Y - 1] = 160 + random(96);
  }

  for (int y = 0; y < PANEL_RES_Y - 1; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      int below = y + 1;
      int left = (x - 1 + PANEL_RES_X) % PANEL_RES_X;
      int right = (x + 1) % PANEL_RES_X;

      int heat = fireHeat[x][below];
      heat += fireHeat[left][below];
      heat += fireHeat[right][below];
      if (below + 1 < PANEL_RES_Y) {
        heat += fireHeat[x][below + 1];
      }

      heat /= 4;
      heat -= random(0, 18);
      if (heat < 0) {
        heat = 0;
      }
      fireHeat[x][y] = heat;
    }
  }

  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      drawPixelMapped(x, y, heatColor(fireHeat[x][y]));
    }
  }
}

void drawPlasmaFrame(uint32_t t) {
  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      float cx = x - (PANEL_RES_X / 2.0f);
      float cy = y - (PANEL_RES_Y / 2.0f);
      float dist = sqrtf(cx * cx + cy * cy);

      float v =
        sinf(dist * 0.55f - t * 0.0040f) +
        sinf(x * 0.45f + t * 0.0030f) +
        cosf(y * 0.60f - t * 0.0022f) +
        sinf((x + y) * 0.25f + t * 0.0018f);

      uint8_t hue = (uint8_t)(128 + 40 * v);
      drawPixelMapped(x, y, colorWheel(hue));
    }
  }
}

void drawTwinkleFrame(uint32_t t) {
  twinklePhase++;
  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      float v1 = sinf((x * 1.73f) + (y * 0.91f) + (t * 0.004f));
      float v2 = cosf((x * 0.63f) - (y * 1.27f) - (t * 0.003f));
      float sparkle = (v1 + v2) * 0.5f;

      if (sparkle > 0.72f) {
        uint8_t hue = (uint8_t)((x * 9 + y * 13 + twinklePhase * 3) & 0xFF);
        drawPixelMapped(x, y, colorWheel(hue));
      } else if (sparkle > 0.45f) {
        uint8_t glow = (uint8_t)(20 + 40 * sparkle);
        drawPixelMapped(x, y, matrix->color565(0, glow / 2, glow));
      } else {
        uint8_t bg = (uint8_t)(4 + 8 * (0.5f + 0.5f * sinf((x + y) * 0.2f + t * 0.001f)));
        drawPixelMapped(x, y, matrix->color565(0, 0, bg));
      }
    }
  }
}

void drawPlasmaClockFrame(uint32_t t) {
  clockText = getCurrentClockText();
  int scale = 1;
  int textWidth = clockText.length() * 6 * scale - 1;
  int startX = (PANEL_RES_X - textWidth) / 2;
  if (startX < 0) {
    startX = 0;
  }

  int startY = 1;
  int textBandTop = 0;
  int textBandBottom = 8;

  if (!plasmaClockInitialized || clockText != lastClockText) {
    for (int y = textBandTop; y <= textBandBottom && y < PANEL_RES_Y; y++) {
      for (int x = 0; x < PANEL_RES_X; x++) {
        drawPixelMapped(x, y, BLACK);
      }
    }
    drawMappedText(startX, startY, clockText, WHITE, scale);
    plasmaClockInitialized = true;
    lastClockText = clockText;
  }

  for (int y = textBandBottom + 1; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      float cx = x - (PANEL_RES_X / 2.0f);
      float cy = y - (PANEL_RES_Y / 2.0f);
      float dist = sqrtf(cx * cx + cy * cy);

      float v =
        sinf(dist * 0.55f - t * 0.0040f) +
        sinf(x * 0.45f + t * 0.0030f) +
        cosf(y * 0.60f - t * 0.0022f) +
        sinf((x + y) * 0.25f + t * 0.0018f);

      uint8_t hue = (uint8_t)(128 + 40 * v);
      drawPixelMapped(x, y, colorWheel(hue));
    }
  }
}

void seedSoundbar() {
  for (int x = 0; x < PANEL_RES_X; x++) {
    soundbarHeights[x] = random(PANEL_RES_Y);
    soundbarTargets[x] = random(PANEL_RES_Y);
  }
}

void drawSoundbarFrame(uint32_t t) {
  matrix->fillScreen(BLACK);

  for (int x = 0; x < PANEL_RES_X; x++) {
    if ((t / 40 + x) % 5 == 0) {
      soundbarTargets[x] = random(2, PANEL_RES_Y + 1);
    }

    if (soundbarHeights[x] < soundbarTargets[x]) {
      soundbarHeights[x]++;
    } else if (soundbarHeights[x] > soundbarTargets[x]) {
      soundbarHeights[x]--;
    }

    for (int y = 0; y < soundbarHeights[x]; y++) {
      int drawY = PANEL_RES_Y - 1 - y;
      uint8_t hue = (uint8_t)((x * 8 + y * 10 + (t / 8)) & 0xFF);
      uint16_t color = colorWheel(hue);
      if (y == soundbarHeights[x] - 1) {
        color = matrix->color565(255, 255, 255);
      }
      drawPixelMapped(x, drawY, color);
    }
  }
}

void seedFluid() {
  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      fluidField[y][x] = 0;
      fluidScratch[y][x] = 0;
    }
  }
  fluidTick = 0;
}

void addFluidDroplets() {
  int drops = 1 + random(3);
  for (int i = 0; i < drops; i++) {
    int x = random(PANEL_RES_X);
    fluidField[0][x] = 220 + random(36);
    if (x > 0 && random(100) < 40) {
      fluidField[0][x - 1] = max(fluidField[0][x - 1], (uint8_t)(140 + random(50)));
    }
    if (x < PANEL_RES_X - 1 && random(100) < 40) {
      fluidField[0][x + 1] = max(fluidField[0][x + 1], (uint8_t)(140 + random(50)));
    }
  }
}

void drawFluidFrame(uint32_t t) {
  fluidTick++;

  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      fluidScratch[y][x] = 0;
    }
  }

  addFluidDroplets();

  for (int y = PANEL_RES_Y - 1; y >= 0; y--) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      uint8_t amount = fluidField[y][x];
      if (amount < 8) {
        continue;
      }

      int remaining = amount;
      int drift = (((int)(t / 120) + y * 3 + x) & 1) ? -1 : 1;
      int down = y + 1;

      if (down < PANEL_RES_Y) {
        int flowDown = min(remaining, 120);
        fluidScratch[down][x] = min(255, fluidScratch[down][x] + flowDown);
        remaining -= flowDown;
      } else {
        fluidScratch[y][x] = min(255, fluidScratch[y][x] + remaining / 3);
        remaining = remaining * 2 / 3;
      }

      int sideA = x + drift;
      int sideB = x - drift;

      if (remaining > 0 && sideA >= 0 && sideA < PANEL_RES_X) {
        int flowSide = min(remaining, 70);
        fluidScratch[y][sideA] = min(255, fluidScratch[y][sideA] + flowSide);
        remaining -= flowSide;
      }

      if (remaining > 0 && sideB >= 0 && sideB < PANEL_RES_X) {
        int flowSide = min(remaining, 45);
        fluidScratch[y][sideB] = min(255, fluidScratch[y][sideB] + flowSide);
        remaining -= flowSide;
      }

      if (remaining > 0) {
        fluidScratch[y][x] = min(255, fluidScratch[y][x] + remaining);
      }
    }
  }

  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      int smoothed = fluidScratch[y][x] * 3;
      int weight = 3;

      if (x > 0) {
        smoothed += fluidScratch[y][x - 1];
        weight++;
      }
      if (x < PANEL_RES_X - 1) {
        smoothed += fluidScratch[y][x + 1];
        weight++;
      }
      if (y > 0) {
        smoothed += fluidScratch[y - 1][x];
        weight++;
      }

      uint8_t amount = smoothed / weight;
      if (amount > 3) {
        amount -= 3;
      }
      fluidField[y][x] = amount;
    }
  }

  matrix->fillScreen(BLACK);
  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      uint8_t amount = fluidField[y][x];
      if (amount < 10) {
        continue;
      }

      uint8_t hue = (uint8_t)(150 + ((amount / 3) + y * 4 + (t / 14)) % 90);
      uint16_t base = colorWheel(hue);
      uint8_t r = (((base >> 11) & 0x1F) << 3);
      uint8_t g = (((base >> 5) & 0x3F) << 2);
      uint8_t b = ((base & 0x1F) << 3);

      uint8_t bright = min(255, amount + 30);
      r = (uint8_t)((r * bright) / 255);
      g = (uint8_t)((g * bright) / 255);
      b = (uint8_t)((b * bright) / 255);

      drawPixelMapped(x, y, matrix->color565(r, g, b));
    }
  }
}

void clearFireworks() {
  for (int i = 0; i < MAX_FIREWORK_PARTICLES; i++) {
    fwLife[i] = 0;
    fwX[i] = 0;
    fwY[i] = 0;
    fwVX[i] = 0;
    fwVY[i] = 0;
    fwHue[i] = 0;
  }
}

void spawnFireworkBurst() {
  float cx = random(4, PANEL_RES_X - 4);
  float cy = random(2, PANEL_RES_Y / 2 + 2);
  uint8_t baseHue = random(256);
  int particles = 18 + (int)random(12);
  int totalParticles = particles;

  for (int i = 0; i < MAX_FIREWORK_PARTICLES && particles > 0; i++) {
    if (fwLife[i] != 0) {
      continue;
    }

    float angle = (6.2831853f * particles) / (float)((totalParticles > 0) ? totalParticles : 1);
    float speed = 0.35f + (random(100) / 100.0f) * 1.3f;

    fwX[i] = cx;
    fwY[i] = cy;
    fwVX[i] = cosf(angle + random(100) * 0.01f) * speed;
    fwVY[i] = sinf(angle + random(100) * 0.01f) * speed;
    fwLife[i] = 12 + random(14);
    fwHue[i] = baseHue + random(50);
    particles--;
  }
}

void drawFireworksFrame(uint32_t t) {
  if (random(100) < 12) {
    spawnFireworkBurst();
  }

  matrix->fillScreen(BLACK);

  for (int i = 0; i < MAX_FIREWORK_PARTICLES; i++) {
    if (fwLife[i] == 0) {
      continue;
    }

    fwX[i] += fwVX[i];
    fwY[i] += fwVY[i];
    fwVY[i] += 0.05f;
    fwVX[i] *= 0.99f;
    fwVY[i] *= 0.99f;

    if (fwLife[i] > 0) {
      fwLife[i]--;
    }

    int px = (int)(fwX[i] + 0.5f);
    int py = (int)(fwY[i] + 0.5f);

    if (px < 0 || px >= PANEL_RES_X || py < 0 || py >= PANEL_RES_Y || fwLife[i] == 0) {
      fwLife[i] = 0;
      continue;
    }

    uint16_t color = colorWheel(fwHue[i] + (uint8_t)(t / 10));
    drawPixelMapped(px, py, color);

    if (fwLife[i] > 8) {
      drawPixelMapped(px, py, WHITE);
    } else if (fwLife[i] > 4 && py + 1 < PANEL_RES_Y) {
      drawPixelMapped(px, py + 1, matrix->color565(30, 30, 30));
    }
  }
}

void clearSupernova() {
  snRocketActive = false;
  snShockwaveLife = 0;
  snShockwaveRadius = 0.0f;

  for (int i = 0; i < MAX_SUPERNOVA_PARTICLES; i++) {
    snLife[i] = 0;
    snMaxLife[i] = 0;
    snHue[i] = 0;
    snX[i] = 0.0f;
    snY[i] = 0.0f;
    snPrevX[i] = 0.0f;
    snPrevY[i] = 0.0f;
    snVX[i] = 0.0f;
    snVY[i] = 0.0f;
  }
}

void launchSupernovaRocket() {
  snRocketActive = true;
  snRocketX = random(4, PANEL_RES_X - 4);
  snRocketY = PANEL_RES_Y - 1;
  snRocketVX = (random(100) / 100.0f - 0.5f) * 0.30f;
  snRocketVY = -(1.15f + (random(100) / 100.0f) * 0.55f);
  snRocketHue = random(256);
  snRocketFuse = 8 + random(7);
}

void spawnSupernovaBurst(float cx, float cy, uint8_t baseHue) {
  snShockwaveX = cx;
  snShockwaveY = cy;
  snShockwaveRadius = 0.0f;
  snShockwaveLife = 9;

  for (int i = 0; i < MAX_SUPERNOVA_PARTICLES; i++) {
    if (snLife[i] != 0) {
      continue;
    }

    float ratio = (float)i / (float)MAX_SUPERNOVA_PARTICLES;
    float angle = ratio * 6.2831853f;
    float ringBias = (i % 3 == 0) ? 1.45f : ((i % 3 == 1) ? 0.95f : 0.55f);
    float speed = ringBias + (random(100) / 100.0f) * 0.55f;

    snX[i] = cx;
    snY[i] = cy;
    snPrevX[i] = cx;
    snPrevY[i] = cy;
    snVX[i] = cosf(angle) * speed;
    snVY[i] = sinf(angle) * speed - 0.12f;
    snMaxLife[i] = 10 + random(10);
    snLife[i] = snMaxLife[i];
    snHue[i] = baseHue + (uint8_t)(ratio * 70.0f) + random(18);
  }
}

void drawSupernovaFrame(uint32_t t) {
  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      float cx = x - (PANEL_RES_X / 2.0f);
      float cy = y - (PANEL_RES_Y / 2.0f);
      float dist = sqrtf(cx * cx + cy * cy);

      float waveA = sinf(dist * 0.72f - t * 0.0050f);
      float waveB = sinf((x * 0.48f) + (t * 0.0014f));
      float waveC = cosf((y * 0.85f) - (t * 0.0026f));
      float nebula = (waveA * 0.55f) + (waveB * 0.25f) + (waveC * 0.20f);

      uint8_t r = (uint8_t)(8 + 16 * (0.5f + 0.5f * waveB));
      uint8_t g = (uint8_t)(4 + 24 * (0.5f + 0.5f * waveC));
      uint8_t b = (uint8_t)(16 + 70 * (0.5f + 0.5f * nebula));

      if ((((x * 11) + (y * 17) + (t / 40)) & 0x1F) == 0) {
        r = min(255, r + 18);
        g = min(255, g + 18);
        b = min(255, b + 40);
      }

      drawPixelMapped(x, y, matrix->color565(r, g, b));
    }
  }

  if (!snRocketActive && random(100) < 18) {
    launchSupernovaRocket();
  }

  if (snRocketActive) {
    float prevX = snRocketX;
    float prevY = snRocketY;

    snRocketX += snRocketVX;
    snRocketY += snRocketVY;
    snRocketVY += 0.02f;

    if (snRocketFuse > 0) {
      snRocketFuse--;
    }

    int tailX = (int)(prevX + 0.5f);
    int tailY = (int)(prevY + 0.5f);
    int rocketX = (int)(snRocketX + 0.5f);
    int rocketY = (int)(snRocketY + 0.5f);

    if (tailX >= 0 && tailX < PANEL_RES_X && tailY >= 0 && tailY < PANEL_RES_Y) {
      drawPixelMapped(tailX, tailY, matrix->color565(80, 80, 120));
    }
    if (rocketX >= 0 && rocketX < PANEL_RES_X && rocketY >= 0 && rocketY < PANEL_RES_Y) {
      drawPixelMapped(rocketX, rocketY, WHITE);
      if (rocketY + 1 < PANEL_RES_Y) {
        drawPixelMapped(rocketX, rocketY + 1, matrix->color565(255, 120, 40));
      }
    }

    if (snRocketFuse == 0 || snRocketY < 4) {
      spawnSupernovaBurst(snRocketX, snRocketY, snRocketHue);
      snRocketActive = false;
    }
  }

  if (snShockwaveLife > 0) {
    float radius = snShockwaveRadius;
    for (int y = 0; y < PANEL_RES_Y; y++) {
      for (int x = 0; x < PANEL_RES_X; x++) {
        float dx = x - snShockwaveX;
        float dy = y - snShockwaveY;
        float dist = sqrtf(dx * dx + dy * dy);
        if (fabsf(dist - radius) < 0.85f) {
          uint8_t glow = 120 + snShockwaveLife * 12;
          drawPixelMapped(x, y, matrix->color565(glow, glow, 255));
        }
      }
    }

    snShockwaveRadius += 1.15f;
    snShockwaveLife--;
  }

  for (int i = 0; i < MAX_SUPERNOVA_PARTICLES; i++) {
    if (snLife[i] == 0) {
      continue;
    }

    snPrevX[i] = snX[i];
    snPrevY[i] = snY[i];
    snX[i] += snVX[i];
    snY[i] += snVY[i];
    snVX[i] *= 0.985f;
    snVY[i] *= 0.985f;
    snVY[i] += 0.035f;

    int tailX = (int)(snPrevX[i] + 0.5f);
    int tailY = (int)(snPrevY[i] + 0.5f);
    int px = (int)(snX[i] + 0.5f);
    int py = (int)(snY[i] + 0.5f);

    if (px < 0 || px >= PANEL_RES_X || py < 0 || py >= PANEL_RES_Y) {
      snLife[i] = 0;
      continue;
    }

    int fadeDenominator = (snMaxLife[i] > 0) ? snMaxLife[i] : 1;
    uint8_t fade = (uint8_t)((snLife[i] * 255) / fadeDenominator);
    uint16_t base = colorWheel(snHue[i] + (uint8_t)(t / 18));
    uint8_t r = (((base >> 11) & 0x1F) << 3);
    uint8_t g = (((base >> 5) & 0x3F) << 2);
    uint8_t b = ((base & 0x1F) << 3);

    uint8_t coreR = min(255, (r * fade) / 160 + 35);
    uint8_t coreG = min(255, (g * fade) / 160 + 35);
    uint8_t coreB = min(255, (b * fade) / 160 + 50);
    uint8_t trailR = max(8, (int)(r * fade) / 510);
    uint8_t trailG = max(8, (int)(g * fade) / 510);
    uint8_t trailB = max(10, (int)(b * fade) / 420);

    if (tailX >= 0 && tailX < PANEL_RES_X && tailY >= 0 && tailY < PANEL_RES_Y) {
      drawPixelMapped(tailX, tailY, matrix->color565(trailR, trailG, trailB));
    }

    drawPixelMapped(px, py, matrix->color565(coreR, coreG, coreB));
    if (snLife[i] > (snMaxLife[i] / 2) && px + 1 < PANEL_RES_X) {
      drawPixelMapped(px + 1, py, matrix->color565(trailR, trailG, trailB));
    }

    snLife[i]--;
  }
}

void drawVortexFrame(uint32_t t) {
  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      float cx = x - (PANEL_RES_X / 2.0f) + 0.5f;
      float cy = y - (PANEL_RES_Y / 2.0f) + 0.5f;
      float dist = sqrtf(cx * cx + cy * cy);
      float angle = atan2f(cy, cx);
      float spiral = angle * 3.0f - dist * 1.55f + t * 0.0060f;
      float rings = sinf(dist * 1.65f - t * 0.0080f);
      float glow = 0.5f + 0.5f * sinf(spiral);
      float pulse = 0.5f + 0.5f * rings;

      uint8_t hue = (uint8_t)(180 + 55 * sinf(spiral * 0.7f) + dist * 9.0f);
      uint16_t base = colorWheel(hue);
      uint8_t r = (((base >> 11) & 0x1F) << 3);
      uint8_t g = (((base >> 5) & 0x3F) << 2);
      uint8_t b = ((base & 0x1F) << 3);

      float brightness = 0.18f + glow * 0.52f + pulse * 0.30f;
      if (dist < 2.5f) {
        brightness += (2.5f - dist) * 0.20f;
      }

      r = (uint8_t)min(255, (int)(r * brightness));
      g = (uint8_t)min(255, (int)(g * brightness));
      b = (uint8_t)min(255, (int)(b * brightness + 18 * glow));

      drawPixelMapped(x, y, matrix->color565(r, g, b));
    }
  }
}

void drawLavaFrame(uint32_t t) {
  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      float nx = x * 0.33f;
      float ny = y * 0.52f;
      float flow =
        sinf(nx + t * 0.0026f) +
        sinf((nx + ny) * 1.3f - t * 0.0018f) +
        cosf(ny * 1.7f + t * 0.0021f) +
        sinf(sqrtf((x - 16.0f) * (x - 16.0f) + (y - 8.0f) * (y - 8.0f)) * 0.9f - t * 0.0038f);

      float heat = 0.5f + 0.5f * (flow / 4.0f);
      float crack = sinf((x * 1.9f) - (y * 0.8f) + t * 0.0042f);

      uint8_t r = (uint8_t)(35 + heat * 220);
      uint8_t g = (uint8_t)(heat * heat * 170);
      uint8_t b = (uint8_t)(heat > 0.78f ? (heat - 0.78f) * 300 : 0);

      if (heat < 0.24f && crack > 0.35f) {
        r = 18;
        g = 0;
        b = 0;
      } else if (heat > 0.86f) {
        r = 255;
        g = min(255, 180 + (int)((heat - 0.86f) * 420));
        b = min(255, 40 + (int)((heat - 0.86f) * 360));
      }

      drawPixelMapped(x, y, matrix->color565(r, g, b));
    }
  }
}

void seedMeteors() {
  for (int i = 0; i < MAX_METEORS; i++) {
    meteorX[i] = PANEL_RES_X + random(20);
    meteorY[i] = random(PANEL_RES_Y);
    meteorVX[i] = -(1.1f + (random(100) / 100.0f) * 2.2f);
    meteorVY[i] = 0.15f + (random(100) / 100.0f) * 0.95f;
    meteorHue[i] = random(256);
    meteorLength[i] = 5 + random(6);
  }

  for (int i = 0; i < MAX_METEOR_IMPACTS; i++) {
    meteorImpactX[i] = 0.0f;
    meteorImpactY[i] = 0.0f;
    meteorImpactRadius[i] = 0.0f;
    meteorImpactLife[i] = 0;
  }
}

void respawnMeteor(int i) {
  meteorX[i] = PANEL_RES_X - 1 + random(18);
  meteorY[i] = random(PANEL_RES_Y / 2 + 2);
  meteorVX[i] = -(1.2f + (random(100) / 100.0f) * 2.6f);
  meteorVY[i] = 0.25f + (random(100) / 100.0f) * 1.10f;
  meteorHue[i] = random(256);
  meteorLength[i] = 6 + random(6);
}

void spawnMeteorImpact(float x, float y) {
  for (int i = 0; i < MAX_METEOR_IMPACTS; i++) {
    if (meteorImpactLife[i] != 0) {
      continue;
    }

    meteorImpactX[i] = x;
    meteorImpactY[i] = y;
    meteorImpactRadius[i] = 0.5f;
    meteorImpactLife[i] = 8;
    return;
  }
}

void drawMeteorFrame(uint32_t t) {
  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      float cloud = 0.5f + 0.5f * sinf((x * 0.28f) + (y * 0.46f) + t * 0.0010f);
      uint8_t r = (uint8_t)(2 + cloud * 8);
      uint8_t g = (uint8_t)(4 + cloud * 10);
      uint8_t b = (uint8_t)(10 + cloud * 22);
      drawPixelMapped(x, y, matrix->color565(r, g, b));
    }
  }

  for (int s = 0; s < 18; s++) {
    int sx = (s * 9 + (int)(t / (45 + (s % 4) * 10))) % PANEL_RES_X;
    int sy = (s * 5 + (int)(t / (90 + (s % 3) * 20))) % PANEL_RES_Y;
    drawPixelMapped(sx, sy, matrix->color565(25, 25, 40));
    if ((s & 3) == 0 && sy + 1 < PANEL_RES_Y) {
      drawPixelMapped(sx, sy + 1, matrix->color565(10, 10, 18));
    }
  }

  if ((t / 180) % 2 == 0) {
    int horizon = PANEL_RES_Y - 2;
    for (int x = 0; x < PANEL_RES_X; x++) {
      uint8_t glow = (uint8_t)(18 + 22 * (0.5f + 0.5f * sinf((x * 0.55f) + t * 0.0025f)));
      drawPixelMapped(x, horizon, matrix->color565(glow, glow / 3, 0));
    }
  }

  for (int i = 0; i < MAX_METEOR_IMPACTS; i++) {
    if (meteorImpactLife[i] == 0) {
      continue;
    }

    drawSoftGlow((int)(meteorImpactX[i] + 0.5f),
                 (int)(meteorImpactY[i] + 0.5f),
                 meteorImpactRadius[i],
                 matrix->color565(255, 170, 80),
                 (uint8_t)(meteorImpactLife[i] * 28));
    meteorImpactRadius[i] += 0.9f;
    meteorImpactLife[i]--;
  }

  for (int i = 0; i < MAX_METEORS; i++) {
    meteorX[i] += meteorVX[i];
    meteorY[i] += meteorVY[i];

    if (meteorX[i] < -meteorLength[i] || meteorY[i] >= PANEL_RES_Y + meteorLength[i]) {
      if (meteorY[i] >= PANEL_RES_Y - 1) {
        spawnMeteorImpact(meteorX[i], PANEL_RES_Y - 2);
      }
      respawnMeteor(i);
    }

    for (int step = 0; step < meteorLength[i]; step++) {
      int px = (int)(meteorX[i] - meteorVX[i] * step + 0.5f);
      int py = (int)(meteorY[i] - meteorVY[i] * step + 0.5f);
      if (px < 0 || px >= PANEL_RES_X || py < 0 || py >= PANEL_RES_Y) {
        continue;
      }

      uint16_t base = colorWheel(meteorHue[i] + step * 6 + (uint8_t)(t / 14));
      uint8_t r = (((base >> 11) & 0x1F) << 3);
      uint8_t g = (((base >> 5) & 0x3F) << 2);
      uint8_t b = ((base & 0x1F) << 3);
      int lengthDivisor = (meteorLength[i] > 0) ? meteorLength[i] : 1;
      int scale = 255 - step * (190 / lengthDivisor);
      if (step == 0) {
        r = 255;
        g = 255;
        b = 255;
      } else if (step == 1) {
        r = 255;
        g = min(255, 180 + scale / 3);
        b = min(255, 120 + scale / 4);
      } else {
        r = (uint8_t)((r * scale) / 255);
        g = (uint8_t)((g * scale) / 255);
        b = (uint8_t)((b * scale) / 255);
      }

      drawPixelMapped(px, py, matrix->color565(r, g, b));
    }
  }
}

void drawStormFrame(uint32_t t) {
  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      float cloudA = sinf((x * 0.32f) + t * 0.0015f);
      float cloudB = cosf((y * 0.75f) - t * 0.0020f);
      float cloudC = sinf(((x + y) * 0.22f) + t * 0.0011f);
      float storm = (cloudA * 0.45f) + (cloudB * 0.35f) + (cloudC * 0.20f);
      uint8_t r = (uint8_t)(6 + 12 * (0.5f + 0.5f * storm));
      uint8_t g = (uint8_t)(10 + 24 * (0.5f + 0.5f * storm));
      uint8_t b = (uint8_t)(18 + 44 * (0.5f + 0.5f * storm));
      drawPixelMapped(x, y, matrix->color565(r, g, b));
    }
  }

  for (int x = 0; x < PANEL_RES_X; x++) {
    int rainY = (int)((t / 25 + x * 3) % (PANEL_RES_Y + 6)) - 3;
    if (rainY >= 0 && rainY < PANEL_RES_Y) {
      drawPixelMapped(x, rainY, matrix->color565(80, 120, 255));
    }
    if (rainY + 1 >= 0 && rainY + 1 < PANEL_RES_Y && (x & 1) == 0) {
      drawPixelMapped(x, rainY + 1, matrix->color565(30, 60, 120));
    }
  }

  if (stormBoltLife == 0 && random(100) < 7) {
    stormBoltLife = 4 + random(4);
    stormBoltHue = 180 + random(30);
    int currentX = random(4, PANEL_RES_X - 4);
    for (int y = 0; y < PANEL_RES_Y; y++) {
      currentX += random(3) - 1;
      if (currentX < 1) {
        currentX = 1;
      } else if (currentX > PANEL_RES_X - 2) {
        currentX = PANEL_RES_X - 2;
      }
      stormBoltMask[y] = (uint8_t)currentX;
    }
  }

  if (stormBoltLife > 0) {
    uint16_t boltColor = colorWheel(stormBoltHue + (uint8_t)(t / 8));
    for (int y = 0; y < PANEL_RES_Y; y++) {
      int x = stormBoltMask[y];
      drawPixelMapped(x, y, WHITE);
      if (x > 0) {
        drawPixelMapped(x - 1, y, scaleColor565(boltColor, 120));
      }
      if (x + 1 < PANEL_RES_X) {
        drawPixelMapped(x + 1, y, scaleColor565(boltColor, 120));
      }
    }

    drawSoftGlow(stormBoltMask[PANEL_RES_Y - 1], PANEL_RES_Y - 1, 3.0f, WHITE, 180);
    stormBoltLife--;
  }
}

void drawGalaxyFrame(uint32_t t) {
  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      float cx = x - (PANEL_RES_X / 2.0f);
      float cy = y - (PANEL_RES_Y / 2.0f);
      float dist = sqrtf(cx * cx + cy * cy);
      float angle = atan2f(cy, cx);
      float spin = t * 0.0038f;
      float armA = 0.5f + 0.5f * sinf(angle * 3.0f + dist * 0.95f - spin);
      float armB = 0.5f + 0.5f * cosf(angle * 2.0f - dist * 1.15f + spin * 0.7f);
      float dust = 0.5f + 0.5f * sinf((x * 0.52f) - (y * 0.31f) + t * 0.0019f);
      float core = (dist < 3.8f) ? (3.8f - dist) / 3.8f : 0.0f;
      float nebula = armA * 0.45f + armB * 0.35f + dust * 0.20f;

      uint8_t hue = (uint8_t)(dist * 18.0f + angle * 28.0f + t / 9 + armA * 80.0f);
      uint16_t base = colorWheel(hue);
      uint8_t r = (((base >> 11) & 0x1F) << 3);
      uint8_t g = (((base >> 5) & 0x3F) << 2);
      uint8_t b = ((base & 0x1F) << 3);

      float brightness = 0.18f + nebula * 0.95f + core * 0.90f;
      if (dist > 8.5f) {
        brightness *= 0.62f;
      }

      r = (uint8_t)min(255, (int)(r * brightness + core * 180.0f));
      g = (uint8_t)min(255, (int)(g * brightness + core * 160.0f));
      b = (uint8_t)min(255, (int)(b * brightness + core * 220.0f));

      if (armA > 0.86f || armB > 0.88f) {
        r = min(255, (int)r + 55);
        g = min(255, (int)g + 35);
        b = min(255, (int)b + 65);
      }

      drawPixelMapped(x, y, matrix->color565(r, g, b));
    }
  }

  for (int s = 0; s < 24; s++) {
    int sx = (s * 13 + (int)(t / (24 + (s % 5) * 5))) % PANEL_RES_X;
    int sy = (s * 7 + (int)(t / (48 + (s % 4) * 9))) % PANEL_RES_Y;
    uint16_t starColor = ((s % 4) == 0) ? WHITE : colorWheel((uint8_t)(s * 23 + t / 12));
    drawPixelMapped(sx, sy, starColor);
    if ((s % 6) == 0 && sx + 1 < PANEL_RES_X) {
      drawPixelMapped(sx + 1, sy, scaleColor565(starColor, 120));
    }
  }
}

void seedSquareBursts() {
  for (int i = 0; i < MAX_SQUARE_BURSTS; i++) {
    squareBurstLife[i] = 0;
    squareBurstRadius[i] = 0.0f;
    squareBurstHue[i] = 0;
    squareBurstX[i] = 0.0f;
    squareBurstY[i] = 0.0f;
  }
}

void spawnSquareBurst() {
  for (int i = 0; i < MAX_SQUARE_BURSTS; i++) {
    if (squareBurstLife[i] != 0) {
      continue;
    }

    squareBurstX[i] = random(3, PANEL_RES_X - 3);
    squareBurstY[i] = random(3, PANEL_RES_Y - 3);
    squareBurstRadius[i] = 0.0f;
    squareBurstLife[i] = 10 + random(6);
    squareBurstHue[i] = random(256);
    return;
  }
}

void drawSquareBurstFrame(uint32_t t) {
  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      float bg = 0.5f + 0.5f * sinf((x * 0.3f) + (y * 0.47f) + t * 0.0014f);
      uint8_t r = (uint8_t)(4 + 10 * bg);
      uint8_t g = (uint8_t)(2 + 8 * bg);
      uint8_t b = (uint8_t)(10 + 22 * bg);
      drawPixelMapped(x, y, matrix->color565(r, g, b));
    }
  }

  if (random(100) < 22) {
    spawnSquareBurst();
  }

  for (int i = 0; i < MAX_SQUARE_BURSTS; i++) {
    if (squareBurstLife[i] == 0) {
      continue;
    }

    int cx = (int)(squareBurstX[i] + 0.5f);
    int cy = (int)(squareBurstY[i] + 0.5f);
    int radius = (int)(squareBurstRadius[i] + 0.5f);
    uint16_t color = colorWheel(squareBurstHue[i] + (uint8_t)(t / 10));

    if (squareBurstLife[i] > 8) {
      for (int oy = -1; oy <= 1; oy++) {
        for (int ox = -1; ox <= 1; ox++) {
          drawPixelMapped(cx + ox, cy + oy, color);
        }
      }
      drawPixelMapped(cx, cy, WHITE);
    } else {
      for (int x = cx - radius; x <= cx + radius; x++) {
        drawPixelMapped(x, cy - radius, color);
        drawPixelMapped(x, cy + radius, color);
      }
      for (int y = cy - radius; y <= cy + radius; y++) {
        drawPixelMapped(cx - radius, y, color);
        drawPixelMapped(cx + radius, y, color);
      }

      if (radius > 1) {
        uint16_t inner = scaleColor565(color, 130);
        drawPixelMapped(cx - radius + 1, cy - radius + 1, inner);
        drawPixelMapped(cx + radius - 1, cy - radius + 1, inner);
        drawPixelMapped(cx - radius + 1, cy + radius - 1, inner);
        drawPixelMapped(cx + radius - 1, cy + radius - 1, inner);
      }

      drawSoftGlow(cx, cy, squareBurstRadius[i] + 0.6f, color, (uint8_t)(squareBurstLife[i] * 16));
    }

    squareBurstRadius[i] += 0.75f;
    squareBurstLife[i]--;
  }
}

void drawSmokeFrame(uint32_t t) {
  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      float nx = x * 0.24f;
      float ny = y * 0.36f;
      float swirl =
        sinf(nx + t * 0.0014f) +
        cosf(ny * 1.6f - t * 0.0010f) +
        sinf((nx + ny) * 1.2f + t * 0.0017f) +
        cosf(sqrtf((x - 16.0f) * (x - 16.0f) + (y - 8.0f) * (y - 8.0f)) * 0.8f - t * 0.0019f);
      float density = 0.5f + 0.5f * (swirl / 4.0f);
      uint8_t hue = (uint8_t)(120 + x * 4 + y * 7 + t / 15);
      uint16_t base = colorWheel(hue);
      uint8_t r = (((base >> 11) & 0x1F) << 3);
      uint8_t g = (((base >> 5) & 0x3F) << 2);
      uint8_t b = ((base & 0x1F) << 3);
      float brightness = 0.08f + density * 0.70f;
      r = (uint8_t)(r * brightness);
      g = (uint8_t)(g * brightness);
      b = (uint8_t)(b * brightness);
      drawPixelMapped(x, y, matrix->color565(r, g, b));
    }
  }
}

void drawBlobsFrame(uint32_t t) {
  float centers[4][2] = {
    {9.0f + 6.0f * sinf(t * 0.0012f), 5.5f + 3.0f * cosf(t * 0.0015f)},
    {21.0f + 5.5f * cosf(t * 0.0010f), 6.0f + 4.0f * sinf(t * 0.0016f)},
    {14.0f + 7.0f * sinf(t * 0.0018f), 10.0f + 2.5f * cosf(t * 0.0013f)},
    {24.0f + 4.5f * cosf(t * 0.0014f), 10.5f + 3.0f * sinf(t * 0.0011f)}
  };

  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      float field = 0.0f;
      for (int i = 0; i < 4; i++) {
        float dx = x - centers[i][0];
        float dy = y - centers[i][1];
        field += 34.0f / (1.5f + dx * dx + dy * dy);
      }

      if (field > 2.7f) {
        uint8_t hue = (uint8_t)(180 + field * 30.0f + t / 10 + x * 4);
        uint16_t base = colorWheel(hue);
        drawPixelMapped(x, y, scaleColor565(base, 210));
      } else {
        uint8_t bg = (uint8_t)(4 + 10 * (0.5f + 0.5f * sinf(x * 0.2f + y * 0.35f + t * 0.001f)));
        drawPixelMapped(x, y, matrix->color565(bg / 3, 0, bg));
      }
    }
  }
}

void drawWindFireFrame(uint32_t t) {
  float wind = sinf(t * 0.0018f) * 1.5f + cosf(t * 0.0009f) * 0.7f;

  for (int x = 0; x < PANEL_RES_X; x++) {
    int sourceX = x + (int)wind;
    if (sourceX < 0) {
      sourceX = 0;
    } else if (sourceX >= PANEL_RES_X) {
      sourceX = PANEL_RES_X - 1;
    }
    fireHeat[sourceX][PANEL_RES_Y - 1] = 170 + random(86);
  }

  for (int y = 0; y < PANEL_RES_Y - 1; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      int below = y + 1;
      int drift = (int)(wind + ((y & 1) ? 1 : -1));
      int center = (x + drift + PANEL_RES_X) % PANEL_RES_X;
      int left = (center - 1 + PANEL_RES_X) % PANEL_RES_X;
      int right = (center + 1) % PANEL_RES_X;

      int heat = fireHeat[center][below];
      heat += fireHeat[left][below];
      heat += fireHeat[right][below];
      if (below + 1 < PANEL_RES_Y) {
        heat += fireHeat[center][below + 1];
      }

      heat /= 4;
      heat -= random(0, 12);
      if (heat < 0) {
        heat = 0;
      }
      fireHeat[x][y] = heat;
    }
  }

  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      uint16_t color = heatColor(fireHeat[x][y]);
      if (y < 4 && fireHeat[x][y] > 80) {
        color = scaleColor565(color, 190);
      }
      drawPixelMapped(x, y, color);
    }
  }
}

void drawRipplesFrame(uint32_t t) {
  float px = 16.0f + 10.0f * sinf(t * 0.0013f);
  float py = 8.0f + 5.0f * cosf(t * 0.0018f);

  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      float dx = x - px;
      float dy = y - py;
      float dist = sqrtf(dx * dx + dy * dy);
      float ripple = sinf(dist * 1.4f - t * 0.008f) + 0.55f * sinf(dist * 2.2f - t * 0.005f);
      float water = 0.5f + 0.5f * ripple;
      uint8_t r = (uint8_t)(4 + 18 * water);
      uint8_t g = (uint8_t)(20 + 70 * water);
      uint8_t b = (uint8_t)(40 + 170 * water);
      drawPixelMapped(x, y, matrix->color565(r, g, b));
    }
  }

  drawSoftGlow((int)(px + 0.5f), (int)(py + 0.5f), 1.8f, WHITE, 180);
}

void drawCircuitFrame(uint32_t t) {
  matrix->fillScreen(BLACK);
  for (int x = 2; x < PANEL_RES_X; x += 5) {
    for (int y = 0; y < PANEL_RES_Y; y++) {
      drawPixelMapped(x, y, matrix->color565(0, 22, 8));
    }
  }
  for (int y = 2; y < PANEL_RES_Y; y += 4) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      drawPixelMapped(x, y, matrix->color565(0, 18, 8));
    }
  }

  int phase = (int)((t / 40) % 40);
  int pathX[6] = {2, 12, 12, 22, 22, 30};
  int pathY[6] = {13, 13, 4, 4, 10, 10};
  for (int i = 0; i < 5; i++) {
    drawLineMapped(pathX[i], pathY[i], pathX[i + 1], pathY[i + 1], matrix->color565(10, 40, 20));
  }

  int px = 2;
  int py = 13;
  if (phase < 10) {
    px = 2 + phase;
    py = 13;
  } else if (phase < 19) {
    px = 12;
    py = 13 - (phase - 10);
  } else if (phase < 29) {
    px = 12 + (phase - 19);
    py = 4;
  } else {
    px = 22 + (phase - 29);
    py = 10;
  }

  drawSoftGlow(px, py, 2.4f, colorWheel((uint8_t)(t / 8)), 230);
  drawPixelMapped(px, py, WHITE);
}

void drawRadarFrame(uint32_t t) {
  int cx = PANEL_RES_X / 2;
  int cy = PANEL_RES_Y / 2;
  float sweep = fmodf(t * 0.0045f, 6.2831853f);
  const int targets[5][2] = {{6, 3}, {24, 4}, {27, 11}, {11, 12}, {18, 7}};

  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      float dx = x - cx;
      float dy = y - cy;
      float angle = atan2f(dy, dx);
      float diff = fabsf(angle - sweep);
      if (diff > 3.1415926f) {
        diff = 6.2831853f - diff;
      }
      float ring = sinf(sqrtf(dx * dx + dy * dy) * 1.1f - t * 0.002f);
      uint8_t g = (uint8_t)(8 + 28 * (0.5f + 0.5f * ring));
      if (diff < 0.30f) {
        g = min(255, (int)g + (int)((0.30f - diff) * 420.0f));
      }
      drawPixelMapped(x, y, matrix->color565(0, g, g / 4));
    }
  }

  for (int i = 0; i < 5; i++) {
    float dx = targets[i][0] - cx;
    float dy = targets[i][1] - cy;
    float angle = atan2f(dy, dx);
    float diff = fabsf(angle - sweep);
    if (diff > 3.1415926f) {
      diff = 6.2831853f - diff;
    }
    uint16_t color = (diff < 0.22f) ? WHITE : matrix->color565(0, 120, 40);
    drawSoftGlow(targets[i][0], targets[i][1], (diff < 0.22f) ? 1.8f : 0.8f, color, (diff < 0.22f) ? 220 : 120);
    drawPixelMapped(targets[i][0], targets[i][1], color);
  }
}

void drawStyleRainFrame(uint32_t t) {
  matrix->fillScreen(BLACK);
  for (int x = 0; x < PANEL_RES_X; x += 2) {
    int head = (int)((t / (28 + (x % 6) * 7) + x * 3) % (PANEL_RES_Y + 10)) - 5;
    int len = 3 + (x % 4);
    for (int i = 0; i < len; i++) {
      int y = head - i;
      if (y < 0 || y >= PANEL_RES_Y) {
        continue;
      }
      uint16_t color;
      if (i == 0) {
        color = WHITE;
      } else if ((x / 2 + i) & 1) {
        color = matrix->color565(0, 255 - i * 25, 200);
      } else {
        color = matrix->color565(255 - i * 30, 40, 180);
      }
      drawPixelMapped(x, y, color);
      if (x + 1 < PANEL_RES_X && (i & 1) == 0) {
        drawPixelMapped(x + 1, y, scaleColor565(color, 150));
      }
    }
  }
}

void drawNeuralFrame(uint32_t t) {
  const int nodes[8][2] = {{4, 3}, {12, 4}, {20, 3}, {27, 6}, {8, 11}, {16, 8}, {23, 12}, {29, 9}};
  const int edges[10][2] = {{0,1},{1,2},{2,3},{0,4},{1,5},{4,5},{5,6},{2,5},{5,7},{6,7}};
  matrix->fillScreen(BLACK);

  for (int i = 0; i < 10; i++) {
    int a = edges[i][0];
    int b = edges[i][1];
    drawLineMapped(nodes[a][0], nodes[a][1], nodes[b][0], nodes[b][1], matrix->color565(15, 20, 50));
    float phase = fmodf(t * 0.0018f + i * 0.27f, 1.0f);
    int px = (int)(nodes[a][0] + (nodes[b][0] - nodes[a][0]) * phase + 0.5f);
    int py = (int)(nodes[a][1] + (nodes[b][1] - nodes[a][1]) * phase + 0.5f);
    uint16_t pulse = colorWheel((uint8_t)(t / 9 + i * 20));
    drawSoftGlow(px, py, 1.5f, pulse, 200);
    drawPixelMapped(px, py, WHITE);
  }

  for (int i = 0; i < 8; i++) {
    uint16_t nodeColor = colorWheel((uint8_t)(t / 12 + i * 28));
    drawSoftGlow(nodes[i][0], nodes[i][1], 1.6f, nodeColor, 180);
    drawPixelMapped(nodes[i][0], nodes[i][1], WHITE);
  }
}

void drawTunnelFrame(uint32_t t) {
  int cx = PANEL_RES_X / 2;
  int cy = PANEL_RES_Y / 2;
  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      float dx = x - cx;
      float dy = y - cy;
      float dist = sqrtf(dx * dx + dy * dy);
      float angle = atan2f(dy, dx);
      float rings = sinf(dist * 1.8f - t * 0.012f);
      uint8_t hue = (uint8_t)(dist * 24.0f - angle * 30.0f + t / 6);
      uint16_t base = colorWheel(hue);
      uint8_t brightness = (uint8_t)(40 + 190 * (0.5f + 0.5f * rings));
      drawPixelMapped(x, y, scaleColor565(base, brightness));
    }
  }
}

void drawCubeFrame(uint32_t t) {
  const float verts[8][3] = {
    {-1.15f,-1.15f,-1.15f},{1.15f,-1.15f,-1.15f},{1.15f,1.15f,-1.15f},{-1.15f,1.15f,-1.15f},
    {-1.15f,-1.15f,1.15f},{1.15f,-1.15f,1.15f},{1.15f,1.15f,1.15f},{-1.15f,1.15f,1.15f}
  };
  const int edges[12][2] = {
    {0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{2,6},{3,7}
  };
  const uint8_t edgeOrder[12] = {0, 3, 1, 2, 8, 11, 9, 10, 4, 7, 5, 6};
  int pts[8][2];
  float depth[8];
  float ay = t * 0.00115f;
  float ax = t * 0.00078f + 0.42f;
  float az = t * 0.00042f;
  float cy = cosf(ay);
  float sy = sinf(ay);
  float cx = cosf(ax);
  float sx = sinf(ax);
  float cz = cosf(az);
  float sz = sinf(az);
  matrix->fillScreen(BLACK);

  for (int i = 0; i < 8; i++) {
    float x = verts[i][0];
    float y = verts[i][1];
    float z = verts[i][2];
    float rz0 = x * sy + z * cy;
    float rx0 = x * cy - z * sy;
    float ry0 = y * cx - rz0 * sx;
    float rz1 = y * sx + rz0 * cx;
    float rx1 = rx0 * cz - ry0 * sz;
    float ry1 = rx0 * sz + ry0 * cz;
    float cameraZ = rz1 + 4.2f;
    depth[i] = cameraZ;
    pts[i][0] = (int)(15.5f + rx1 * 18.0f / cameraZ + 0.5f);
    pts[i][1] = (int)(7.5f + ry1 * 9.2f / cameraZ + 0.5f);
  }

  for (int i = 0; i < 12; i++) {
    int edgeIndex = edgeOrder[i];
    int a = edges[edgeIndex][0];
    int b = edges[edgeIndex][1];
    float avgDepth = (depth[a] + depth[b]) * 0.5f;
    uint8_t brightness = (uint8_t)constrain(285.0f - avgDepth * 42.0f, 70.0f, 255.0f);
    uint16_t color = scaleColor565(colorWheel((uint8_t)(150 + edgeIndex * 16)), brightness);
    drawLineMapped(pts[a][0], pts[a][1], pts[b][0], pts[b][1], color);

    if (brightness > 175) {
      drawLineMapped(pts[a][0], pts[a][1] + 1, pts[b][0], pts[b][1] + 1, scaleColor565(color, 95));
    }
  }

  for (int i = 0; i < 8; i++) {
    uint8_t brightness = (uint8_t)constrain(310.0f - depth[i] * 48.0f, 90.0f, 255.0f);
    drawPixelMapped(pts[i][0], pts[i][1], scaleColor565(WHITE, brightness));
  }
}

void drawKaleidoFrame(uint32_t t) {
  int cx = PANEL_RES_X / 2;
  int cy = PANEL_RES_Y / 2;
  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      float dx = fabsf(x - cx);
      float dy = fabsf(y - cy);
      float ax = (dx > dy) ? dx : dy;
      float ay = (dx > dy) ? dy : dx;
      float wave = sinf(ax * 0.85f + t * 0.0030f) + cosf((ax + ay) * 0.55f - t * 0.0021f);
      uint8_t hue = (uint8_t)(ax * 18.0f + ay * 28.0f + t / 7);
      drawPixelMapped(x, y, scaleColor565(colorWheel(hue), (uint8_t)(50 + 180 * (0.5f + 0.5f * wave))));
    }
  }
}

void drawSpiralFrame(uint32_t t) {
  int cx = PANEL_RES_X / 2;
  int cy = PANEL_RES_Y / 2;
  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      float dx = x - cx;
      float dy = y - cy;
      float angle = atan2f(dy, dx);
      float dist = sqrtf(dx * dx + dy * dy);
      float spiral = sinf(angle * 5.0f + dist * 1.25f - t * 0.008f);
      uint8_t hue = (uint8_t)(angle * 40.0f + dist * 20.0f + t / 8);
      drawPixelMapped(x, y, scaleColor565(colorWheel(hue), (uint8_t)(30 + 210 * (0.5f + 0.5f * spiral))));
    }
  }
}

void drawInterferenceFrame(uint32_t t) {
  float sx1 = 8.0f + 6.0f * sinf(t * 0.0012f);
  float sy1 = 4.0f + 2.5f * cosf(t * 0.0018f);
  float sx2 = 24.0f + 6.0f * cosf(t * 0.0015f);
  float sy2 = 11.0f + 2.5f * sinf(t * 0.0011f);
  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      float d1 = sqrtf((x - sx1) * (x - sx1) + (y - sy1) * (y - sy1));
      float d2 = sqrtf((x - sx2) * (x - sx2) + (y - sy2) * (y - sy2));
      float wave = sinf(d1 * 1.7f - t * 0.008f) + sinf(d2 * 1.7f - t * 0.008f);
      uint8_t hue = (uint8_t)(128 + wave * 55.0f + x * 2 + t / 10);
      drawPixelMapped(x, y, scaleColor565(colorWheel(hue), (uint8_t)(40 + 180 * (0.5f + 0.5f * (wave * 0.5f)))));
    }
  }
}

void drawTessellateFrame(uint32_t t) {
  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      float u = x * 0.36f;
      float v = y * 0.62f;
      float tile = sinf(u + t * 0.0015f) * cosf(v - t * 0.0012f) + sinf((u + v) * 1.3f + t * 0.0010f);
      float bands = fabsf(sinf((x + (y & 1) * 0.5f) * 0.7f + t * 0.0018f));
      uint8_t hue = (uint8_t)(x * 11 + y * 17 + t / 9);
      drawPixelMapped(x, y, scaleColor565(colorWheel(hue), (uint8_t)(30 + 200 * (0.35f + 0.35f * tile + 0.30f * bands))));
    }
  }
}

void drawBeatBurstFrame(uint32_t t) {
  int beat = (int)((t / 220) % 8);
  int centers[8][2] = {{6,4},{16,3},{25,5},{9,10},{20,9},{28,12},{4,12},{15,8}};
  matrix->fillScreen(BLACK);
  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      drawPixelMapped(x, y, matrix->color565(0, 0, 10));
    }
  }
  for (int i = 0; i < 8; i++) {
    float phase = fmodf((t % 220) / 220.0f + i * 0.12f, 1.0f);
    float radius = phase * 7.0f;
    uint16_t color = colorWheel((uint8_t)(i * 30 + t / 6));
    drawSoftGlow(centers[i][0], centers[i][1], radius, color, (uint8_t)(220 * (1.0f - phase)));
  }
  drawPixelMapped(centers[beat][0], centers[beat][1], WHITE);
}

void drawSunriseFrame(uint32_t t) {
  float sunY = 14.0f - 6.0f * (0.5f + 0.5f * sinf(t * 0.0005f));
  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      float grad = (float)y / (float)(PANEL_RES_Y - 1);
      uint8_t r = (uint8_t)(20 + 235 * (1.0f - grad));
      uint8_t g = (uint8_t)(10 + 120 * (1.0f - fabsf(grad - 0.45f)));
      uint8_t b = (uint8_t)(30 + 120 * grad);
      float dx = x - 16.0f;
      float dy = y - sunY;
      float dist = sqrtf(dx * dx + dy * dy);
      if (dist < 4.2f) {
        r = min(255, (int)r + (int)((4.2f - dist) * 35));
        g = min(255, (int)g + (int)((4.2f - dist) * 22));
      }
      drawPixelMapped(x, y, matrix->color565(r, g, b));
    }
  }
  for (int x = 0; x < PANEL_RES_X; x++) {
    int skyline = 10 + (int)(2.2f * sinf(x * 0.42f) + 1.5f * cosf(x * 0.18f + t * 0.0002f));
    for (int y = skyline; y < PANEL_RES_Y; y++) {
      drawPixelMapped(x, y, matrix->color565(8, 6, 12));
    }
  }
}

void getPacmanRoutePoint(uint8_t index, int& x, int& y, uint8_t& direction) {
  index %= PACMAN_ROUTE_LENGTH;
  if (index < 26) {
    x = 3 + index;
    y = 3;
    direction = 0; // right
  } else if (index < 35) {
    x = 28;
    y = 4 + (index - 26);
    direction = 1; // down
  } else if (index < 60) {
    x = 27 - (index - 35);
    y = 12;
    direction = 2; // left
  } else {
    x = 3;
    y = 11 - (index - 60);
    direction = 3; // up
  }
}

void resetPacmanGame() {
  for (uint8_t i = 0; i < PACMAN_ROUTE_LENGTH; i++) {
    pacmanPellets[i] = (i % 4) == 2;
  }
  pacmanRouteIndex = 0;
  pacmanLastStep = 0;
  pacmanRoundCompleteAt = 0;
}

void drawPacmanMaze() {
  uint16_t wall = matrix->color565(20, 45, 255);
  uint16_t edge = matrix->color565(0, 150, 255);

  for (int x = 0; x < PANEL_RES_X; x++) {
    drawPixelMapped(x, 0, wall);
    drawPixelMapped(x, PANEL_RES_Y - 1, wall);
  }
  for (int y = 1; y < PANEL_RES_Y - 1; y++) {
    drawPixelMapped(0, y, wall);
    drawPixelMapped(PANEL_RES_X - 1, y, wall);
  }

  // Compact inner maze and ghost house, leaving a clear loop around it.
  for (int x = 6; x <= 12; x++) {
    drawPixelMapped(x, 6, wall);
    drawPixelMapped(x, 9, wall);
  }
  for (int x = 19; x <= 25; x++) {
    drawPixelMapped(x, 6, wall);
    drawPixelMapped(x, 9, wall);
  }
  for (int x = 14; x <= 17; x++) {
    drawPixelMapped(x, 7, edge);
    drawPixelMapped(x, 9, wall);
  }
  drawPixelMapped(13, 8, wall);
  drawPixelMapped(18, 8, wall);
}

void drawPacmanSprite(int x, int y, uint8_t direction, bool mouthOpen) {
  uint16_t yellow = matrix->color565(255, 225, 0);
  for (int oy = -2; oy <= 2; oy++) {
    for (int ox = -2; ox <= 2; ox++) {
      if (ox * ox + oy * oy > 6) continue;

      int forward = 0;
      int side = 0;
      if (direction == 0) { forward = ox;  side = oy; }
      if (direction == 1) { forward = oy;  side = ox; }
      if (direction == 2) { forward = -ox; side = oy; }
      if (direction == 3) { forward = -oy; side = ox; }
      if (mouthOpen && forward > 0 && abs(side) <= forward) continue;
      drawPixelMapped(x + ox, y + oy, yellow);
    }
  }

  // Rotate the eye with Pac-Man so the sprite faces its direction of travel.
  int eyeX = x;
  int eyeY = y;
  if (direction == 0) { eyeX = x;     eyeY = y - 1; }
  if (direction == 1) { eyeX = x + 1; eyeY = y; }
  if (direction == 2) { eyeX = x;     eyeY = y - 1; }
  if (direction == 3) { eyeX = x - 1; eyeY = y; }
  drawPixelMapped(eyeX, eyeY, BLACK);
}

void drawGhostSprite(int x, int y, uint16_t color, uint8_t direction, uint8_t phase) {
  // Rounded 6x5 dome with a scalloped, animated skirt.
  for (int oy = -2; oy <= 2; oy++) {
    for (int ox = -3; ox <= 2; ox++) {
      bool body = false;
      if (oy == -2) body = ox >= -2 && ox <= 1;
      else if (oy < 2) body = true;
      else body = ((ox + phase) & 1) == 0;
      if (body) drawPixelMapped(x + ox, y + oy, color);
    }
  }

  int pupilOffsetX = direction == 0 ? 1 : 0;
  int pupilOffsetY = direction == 1 ? 1 : 0;
  for (int ey = -1; ey <= 0; ey++) {
    drawPixelMapped(x - 2, y + ey, WHITE);
    drawPixelMapped(x - 1, y + ey, WHITE);
    drawPixelMapped(x, y + ey, WHITE);
    drawPixelMapped(x + 1, y + ey, WHITE);
  }
  uint16_t pupil = matrix->color565(20, 40, 180);
  drawPixelMapped(x - 2 + pupilOffsetX, y - 1 + pupilOffsetY, pupil);
  drawPixelMapped(x + pupilOffsetX, y - 1 + pupilOffsetY, pupil);
}

void drawPacmanFrame(uint32_t t) {
  if (pacmanRoundCompleteAt != 0 && t - pacmanRoundCompleteAt >= 900) {
    resetPacmanGame();
    pacmanLastStep = t;
  }

  if (pacmanRoundCompleteAt == 0 && (pacmanLastStep == 0 || t - pacmanLastStep >= 120)) {
    pacmanLastStep = t;
    pacmanRouteIndex = (pacmanRouteIndex + 1) % PACMAN_ROUTE_LENGTH;
    pacmanPellets[pacmanRouteIndex] = false;

    bool pelletsRemain = false;
    for (uint8_t i = 0; i < PACMAN_ROUTE_LENGTH; i++) {
      if (pacmanPellets[i]) {
        pelletsRemain = true;
        break;
      }
    }
    if (!pelletsRemain) pacmanRoundCompleteAt = t;
  }

  matrix->fillScreen(BLACK);
  drawPacmanMaze();

  uint16_t pelletColor = matrix->color565(255, 190, 120);
  for (uint8_t i = 0; i < PACMAN_ROUTE_LENGTH; i++) {
    if (!pacmanPellets[i]) continue;
    int pelletX, pelletY;
    uint8_t pelletDirection;
    getPacmanRoutePoint(i, pelletX, pelletY, pelletDirection);
    drawPixelMapped(pelletX, pelletY, pelletColor);
  }

  int pacX, pacY;
  uint8_t pacDirection;
  getPacmanRoutePoint(pacmanRouteIndex, pacX, pacY, pacDirection);
  drawPacmanSprite(pacX, pacY, pacDirection, ((t / 120) & 1) == 0);

  const uint8_t ghostSpacing = 14; // 6px sprites plus at least one blank pixel.
  uint16_t ghostColors[3] = {
    matrix->color565(255, 20, 20),
    matrix->color565(255, 105, 190),
    matrix->color565(20, 230, 255)
  };
  for (uint8_t g = 0; g < 3; g++) {
    uint8_t ghostIndex = (pacmanRouteIndex + PACMAN_ROUTE_LENGTH - ghostSpacing * (g + 1)) % PACMAN_ROUTE_LENGTH;
    int ghostX, ghostY;
    uint8_t ghostDirection;
    getPacmanRoutePoint(ghostIndex, ghostX, ghostY, ghostDirection);
    drawGhostSprite(ghostX, ghostY, ghostColors[g], ghostDirection, g + (t / 240));
  }
}

void drawSphereFrame(uint32_t t) {
  matrix->fillScreen(BLACK);
  for (int i = 0; i < 64; i++) {
    float phi = (i / 64.0f) * 6.2831853f * 2.0f;
    float theta = acosf(1.0f - 2.0f * ((i + 0.5f) / 64.0f));
    float x = sinf(theta) * cosf(phi + t * 0.0018f);
    float y = cosf(theta);
    float z = sinf(theta) * sinf(phi + t * 0.0018f);
    float rx = x * cosf(t * 0.0011f) - z * sinf(t * 0.0011f);
    float rz = x * sinf(t * 0.0011f) + z * cosf(t * 0.0011f) + 2.4f;
    int px = (int)(16 + rx * 9.0f / rz + 0.5f);
    int py = (int)(8 + y * 8.0f / rz + 0.5f);
    uint16_t color = colorWheel((uint8_t)(i * 4 + t / 8));
    drawPixelMapped(px, py, color);
  }
}

void drawParallaxFrame(uint32_t t) {
  matrix->fillScreen(BLACK);
  for (int i = 0; i < 18; i++) {
    int x = (PANEL_RES_X - 1) - ((int)(t / (35 + (i % 3) * 20) + i * 7) % (PANEL_RES_X + 6));
    int y = (i * 5) % PANEL_RES_Y;
    drawPixelMapped(x, y, matrix->color565(60, 60, 120));
  }
  for (int i = 0; i < 12; i++) {
    int x = (PANEL_RES_X - 1) - ((int)(t / (18 + (i % 3) * 10) + i * 11) % (PANEL_RES_X + 8));
    int y = (i * 7 + 3) % PANEL_RES_Y;
    drawPixelMapped(x, y, matrix->color565(140, 140, 220));
    if (x + 1 < PANEL_RES_X) {
      drawPixelMapped(x + 1, y, matrix->color565(40, 40, 80));
    }
  }
  for (int i = 0; i < 8; i++) {
    int x = (PANEL_RES_X - 1) - ((int)(t / (8 + (i % 2) * 3) + i * 15) % (PANEL_RES_X + 10));
    int y = (i * 9 + 1) % PANEL_RES_Y;
    drawPixelMapped(x, y, WHITE);
    if (x + 1 < PANEL_RES_X) drawPixelMapped(x + 1, y, matrix->color565(100, 100, 160));
  }
}

void drawMoireFrame(uint32_t t) {
  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      float lineA = sinf(x * 0.65f + t * 0.0030f);
      float lineB = sinf((x + y) * 0.62f - t * 0.0026f);
      float lineC = sinf((x - y) * 0.58f + t * 0.0021f);
      float mix = (lineA + lineB + lineC) / 3.0f;
      uint8_t hue = (uint8_t)(128 + mix * 90.0f + t / 12);
      drawPixelMapped(x, y, scaleColor565(colorWheel(hue), (uint8_t)(30 + 200 * (0.5f + 0.5f * mix))));
    }
  }
}

void drawHypnosisFrame(uint32_t t) {
  float centerX = 15.5f + sinf(t * 0.0013f) * 3.5f;
  float centerY = 7.5f + cosf(t * 0.0017f) * 2.0f;
  float pulse = t * 0.009f;

  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      float dx = (x - centerX) * 0.72f;
      float dy = y - centerY;
      float radius = sqrtf(dx * dx + dy * dy);
      int band = (int)floorf(radius * 1.65f - pulse);
      bool bright = (band & 1) == 0;
      uint8_t hue = (uint8_t)(t / 18 + radius * 15.0f);
      drawPixelMapped(x, y, bright ? colorWheel(hue) : scaleColor565(colorWheel(hue + 128), 18));
    }
  }
}

void drawCafeWallFrame(uint32_t t) {
  const int tileWidth = 4;
  const int tileHeight = 3;
  int drift = (t / 110) % (tileWidth * 2);

  for (int y = 0; y < PANEL_RES_Y; y++) {
    int row = y / tileHeight;
    bool mortar = (y % tileHeight) == tileHeight - 1;
    int rowShift = (row & 1) ? drift : -drift;
    for (int x = 0; x < PANEL_RES_X; x++) {
      if (mortar) {
        drawPixelMapped(x, y, matrix->color565(45, 45, 65));
        continue;
      }
      int shiftedX = x + rowShift + row * 2;
      int tile = shiftedX >= 0 ? shiftedX / tileWidth : (shiftedX - tileWidth + 1) / tileWidth;
      bool lightTile = ((tile + row) & 1) == 0;
      uint8_t shimmer = (uint8_t)(25 + 18 * sinf(t * 0.003f + x * 0.25f));
      drawPixelMapped(x, y, lightTile ? matrix->color565(245, 225, 170)
                                      : matrix->color565(shimmer, 8, 55));
    }
  }
}

void drawPinwheelFrame(uint32_t t) {
  float rotation = t * 0.0022f;
  float breathe = sinf(t * 0.0015f) * 1.8f;

  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      float dx = (x - 15.5f) * 0.55f;
      float dy = y - 7.5f;
      float angle = atan2f(dy, dx);
      float radius = sqrtf(dx * dx + dy * dy);
      float wave = sinf(angle * 8.0f + radius * (1.15f + breathe * 0.08f) - rotation * 7.0f);
      uint8_t hue = wave > 0.0f ? (uint8_t)(t / 14 + radius * 9.0f)
                                : (uint8_t)(t / 14 + 150 + radius * 9.0f);
      uint8_t brightness = (uint8_t)(105 + 145 * fabsf(wave));
      drawPixelMapped(x, y, scaleColor565(colorWheel(hue), brightness));
    }
  }
}

void drawPetalWarpFrame(uint32_t t) {
  float spin = t * 0.0011f;
  float zoom = t * 0.010f;
  float centerX = 15.5f + sinf(t * 0.0007f) * 1.4f;
  float centerY = 7.5f + cosf(t * 0.0009f) * 0.8f;

  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      float dx = (x - centerX) * 0.52f;
      float dy = y - centerY;
      float radius = sqrtf(dx * dx + dy * dy) + 0.18f;
      float angle = atan2f(dy, dx);
      float petals = sinf(angle * 12.0f + spin * 4.0f);
      float folds = sinf(radius * 2.7f - zoom + petals * 2.8f);
      float razor = sinf(angle * 24.0f - radius * 1.25f + spin * 7.0f);
      float light = folds * 0.78f + razor * 0.22f;
      uint8_t hue = (uint8_t)(t / 20 + angle * 35.0f + radius * 13.0f);
      uint8_t brightness = light > 0.0f ? (uint8_t)(120 + light * 135.0f)
                                       : (uint8_t)(10 + (light + 1.0f) * 28.0f);
      drawPixelMapped(x, y, scaleColor565(colorWheel(hue), brightness));
    }
  }
}

void drawTwistSquareFrame(uint32_t t) {
  float travel = t * 0.0075f;
  float globalSpin = t * 0.00065f;

  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      float px = (x - 15.5f) / 15.5f;
      float py = (y - 7.5f) / 7.5f;
      float distance = sqrtf(px * px + py * py) + 0.045f;
      float twist = globalSpin + logf(distance) * 1.18f;
      float cs = cosf(twist);
      float sn = sinf(twist);
      float rx = px * cs - py * sn;
      float ry = px * sn + py * cs;
      float squareRadius = fmaxf(fabsf(rx), fabsf(ry));
      float edgeAngle = atan2f(ry, rx);
      float nested = sinf(24.0f * logf(squareRadius + 0.055f) - travel);
      float spokes = sinf(edgeAngle * 16.0f + logf(distance) * 9.0f - travel * 0.45f);
      bool whiteBand = (nested + spokes * 0.42f) > 0.0f;
      uint8_t hue = (uint8_t)(t / 17 + squareRadius * 130.0f);
      uint8_t brightness = whiteBand ? 255 : 16;
      drawPixelMapped(x, y, scaleColor565(whiteBand ? matrix->color565(255, 245, 220)
                                                    : colorWheel(hue), brightness));
    }
  }
}

void drawEventHorizonFrame(uint32_t t) {
  float orbit = t * 0.0014f;
  float centerX = 15.5f + cosf(orbit) * 2.0f;
  float centerY = 7.5f + sinf(orbit * 1.31f) * 1.0f;

  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      float dx = (x - centerX) * 0.57f;
      float dy = y - centerY;
      float radius = sqrtf(dx * dx + dy * dy) + 0.12f;
      float angle = atan2f(dy, dx);
      float gravityTwist = angle + 4.8f / radius + t * 0.0017f;
      float gridA = sinf(gravityTwist * 9.0f);
      float gridB = sinf(radius * 3.4f - t * 0.012f + sinf(angle * 5.0f));
      float checker = gridA * gridB;
      float ring = 1.0f - fminf(1.0f, fabsf(radius - 3.25f) * 1.8f);
      float jet = powf(fabsf(cosf(angle)), 18.0f) * (0.35f + ring * 0.65f);
      uint8_t hue = (uint8_t)(t / 12 + angle * 42.0f + radius * 17.0f);
      uint8_t brightness;
      if (radius < 1.15f) {
        brightness = (uint8_t)(8 + radius * 18.0f);
      } else {
        float energy = 0.20f + (checker > 0.0f ? 0.45f : 0.05f) + ring * 0.55f + jet * 0.45f;
        brightness = (uint8_t)fminf(255.0f, energy * 255.0f);
      }
      uint16_t color = ring > 0.58f ? matrix->color565(255, 235, 170) : colorWheel(hue);
      drawPixelMapped(x, y, scaleColor565(color, brightness));
    }
  }
}

void drawMorphFrame(uint32_t t) {
  char sequence[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H'};
  int idx = (t / 1800) % 7;
  float phase = (t % 1800) / 1800.0f;
  const uint8_t* fromRows = getGlyphRows(sequence[idx]);
  const uint8_t* toRows = getGlyphRows(sequence[idx + 1]);
  matrix->fillScreen(BLACK);
  int startX = 1;
  int startY = 4;
  for (int gy = 0; gy < 7; gy++) {
    for (int gx = 0; gx < 5; gx++) {
      bool fromOn = fromRows[gy] & (1 << (4 - gx));
      bool toOn = toRows[gy] & (1 << (4 - gx));
      if (!fromOn && !toOn) continue;
      int x = startX + gx * 6;
      int y = startY + gy;
      if (fromOn && phase < 0.65f) {
        drawPixelMapped(x, y, scaleColor565(colorWheel((uint8_t)(gx * 20 + gy * 18 + t / 14)), (uint8_t)(255 - phase * 255)));
      }
      if (toOn && phase > 0.35f) {
        drawPixelMapped(x, y, scaleColor565(colorWheel((uint8_t)(180 + gx * 20 + gy * 16 + t / 11)), (uint8_t)((phase - 0.35f) * 392.0f)));
      }
    }
  }
}

void seedDvdBounce() {
  dvdX = 2.0f;
  dvdY = 2.0f;
  dvdVX = 0.45f;
  dvdVY = 0.32f;
  dvdHue = 0;
}

void drawMiniDvdLogo(int x, int y, uint16_t color) {
  static const uint16_t rows[] = {
    0b111101000111110,
    0b100011000110001,
    0b100011000110001,
    0b100011000110001,
    0b100010101010001,
    0b100010101010001,
    0b111100010011110
  };

  for (int row = 0; row < 7; row++) {
    for (int col = 0; col < 15; col++) {
      if (rows[row] & (1 << (14 - col))) {
        drawPixelMapped(x + col, y + row, color);
      }
    }
  }
}

void drawDvdFrame(uint32_t t) {
  const int logoWidth = 16;
  const int logoHeight = 8;
  bool bounced = false;

  dvdX += dvdVX;
  dvdY += dvdVY;

  if (dvdX <= 0.0f) {
    dvdX = 0.0f;
    if (dvdVX < 0.0f) dvdVX = -dvdVX;
    bounced = true;
  } else if (dvdX >= PANEL_RES_X - logoWidth) {
    dvdX = PANEL_RES_X - logoWidth;
    if (dvdVX > 0.0f) dvdVX = -dvdVX;
    bounced = true;
  }

  if (dvdY <= 0.0f) {
    dvdY = 0.0f;
    if (dvdVY < 0.0f) dvdVY = -dvdVY;
    bounced = true;
  } else if (dvdY >= PANEL_RES_Y - logoHeight) {
    dvdY = PANEL_RES_Y - logoHeight;
    if (dvdVY > 0.0f) dvdVY = -dvdVY;
    bounced = true;
  }

  if (bounced) {
    dvdHue += 43;
  }

  matrix->fillScreen(BLACK);

  int x = (int)(dvdX + 0.5f);
  int y = (int)(dvdY + 0.5f);
  uint16_t logoColor = colorWheel(dvdHue + (uint8_t)(t / 50));
  uint16_t glowColor = scaleColor565(logoColor, 65);

  drawMiniDvdLogo(x + 1, y + 1, glowColor);
  drawMiniDvdLogo(x, y, logoColor);
}

uint16_t tvBarColor(uint8_t index) {
  switch (index & 0x07) {
    case 0: return matrix->color565(210, 210, 210);
    case 1: return matrix->color565(220, 220, 50);
    case 2: return matrix->color565(40, 210, 210);
    case 3: return matrix->color565(40, 210, 70);
    case 4: return matrix->color565(210, 40, 210);
    case 5: return matrix->color565(210, 40, 40);
    case 6: return matrix->color565(35, 35, 190);
    default: return matrix->color565(90, 90, 90);
  }
}

uint8_t tvNoise(int x, int y, uint32_t frame) {
  uint32_t n = (uint32_t)x * 37UL + (uint32_t)y * 73UL + frame * 151UL;
  n ^= n << 7;
  n ^= n >> 9;
  n *= 23UL;
  return (uint8_t)n;
}

void drawTvBarsFrame(uint32_t t) {
  uint32_t frame = t / 30;
  int topDrift = (int)(t / 210);
  int tearY = 4 + (int)((sinf(t * 0.0021f) + 1.0f) * 3.0f);
  int tearShift = (int)(sinf(t * 0.005f) * 4.0f);

  for (int y = 0; y < PANEL_RES_Y; y++) {
    int rowJitter = (int)(sinf(y * 0.9f + t * 0.004f) * 1.6f);

    for (int x = 0; x < PANEL_RES_X; x++) {
      int sx = x + rowJitter + topDrift;
      if (y >= tearY && y <= tearY + 1) {
        sx += tearShift;
      }

      uint16_t color = BLACK;

      if (y < 9) {
        uint8_t bar = (uint8_t)(((sx % PANEL_RES_X + PANEL_RES_X) % PANEL_RES_X) / 4);
        color = tvBarColor(bar);
        uint8_t scan = (uint8_t)(210 + ((y & 1) ? 0 : 35));
        color = scaleColor565(color, scan);
      } else if (y < 11) {
        int bandX = (x + (int)(frame / 2)) % PANEL_RES_X;
        if (bandX < 4) {
          color = CYAN;
        } else if (bandX < 8) {
          color = YELLOW;
        } else if (bandX > 26) {
          color = BLUE;
        } else {
          color = matrix->color565(230, 230, 230);
        }
      } else if (y < 12) {
        color = ((x + (int)frame) % 11 < 7) ? matrix->color565(0, 90, 18) : matrix->color565(20, 20, 20);
      } else {
        uint8_t n = tvNoise(x, y, frame);
        if (n > 225) {
          color = WHITE;
        } else if (n > 170) {
          color = matrix->color565(95, 95, 95);
        } else if (n < 18) {
          color = tvBarColor((uint8_t)(x + y + frame));
        } else {
          color = BLACK;
        }
      }

      if (tvNoise(x, y, frame / 2) > 250) {
        color = WHITE;
      }

      drawPixelMapped(x, y, color);
    }
  }

  int pulseX = (int)((t / 45) % (PANEL_RES_X + 8)) - 4;
  for (int x = 0; x < 10; x++) {
    int px = pulseX + x;
    if (px >= 0 && px < PANEL_RES_X) {
      drawPixelMapped(px, 10, scaleColor565(WHITE, (uint8_t)(90 + x * 13)));
    }
  }
}

void drawColorTvStaticFrame(uint32_t t) {
  uint32_t frame = t / 24;
  int tearY = (int)((frame / 3) % PANEL_RES_Y);
  int tearShift = (int)(tvNoise(0, tearY, frame) % 9) - 4;

  for (int y = 0; y < PANEL_RES_Y; y++) {
    int rowJitter = (int)(tvNoise(31, y, frame / 2) % 5) - 2;
    if (abs(y - tearY) <= 1) rowJitter += tearShift;

    for (int x = 0; x < PANEL_RES_X; x++) {
      int sampleX = (x + rowJitter + PANEL_RES_X) % PANEL_RES_X;
      uint8_t hueNoise = tvNoise(sampleX, y, frame);
      uint8_t levelNoise = tvNoise(sampleX + 17, y + 9, frame * 3 + 41);
      uint8_t sparkle = tvNoise(sampleX + 7, y + 19, frame * 5 + 113);

      uint16_t color;
      if (sparkle > 247) {
        color = WHITE;
      } else if (sparkle < 9) {
        color = BLACK;
      } else {
        // Fully saturated random chroma with noisy luminance reads as classic
        // analog color snow instead of a smooth rainbow animation.
        uint8_t brightness = (uint8_t)(70 + (levelNoise % 186));
        color = scaleColor565(colorWheel(hueNoise), brightness);
      }

      // Dark alternating scanlines keep the effect television-like.
      if ((y & 1) == 0 && sparkle < 235) color = scaleColor565(color, 190);
      drawPixelMapped(x, y, color);
    }
  }
}

void drawSineWaveFrame(uint32_t t) {
  float time = t * 0.0040f;

  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      float shimmer = sinf(x * 0.38f + y * 0.72f + time) + cosf(x * 0.20f - y * 0.55f - time * 0.7f);
      uint8_t hue = (uint8_t)(x * 5 + y * 12 + t / 22);
      uint8_t brightness = (uint8_t)constrain(16.0f + shimmer * 9.0f, 4.0f, 34.0f);
      drawPixelMapped(x, y, scaleColor565(colorWheel(hue), brightness));
    }
  }

  for (int wave = 0; wave < 3; wave++) {
    float phase = time * (0.82f + wave * 0.18f) + wave * 2.1f;
    float amplitude = 4.5f - wave * 0.75f;
    float frequency = 0.42f + wave * 0.11f;
    uint8_t baseHue = (uint8_t)(t / 12 + wave * 72);
    int lastY = 0;
    bool hasLast = false;

    for (int x = 0; x < PANEL_RES_X; x++) {
      float yFloat = 7.5f + sinf(x * frequency + phase) * amplitude;
      yFloat += sinf(x * 0.18f - time * 1.3f + wave) * 0.9f;
      int y = (int)(yFloat + 0.5f);
      uint16_t color = colorWheel((uint8_t)(baseHue + x * 4));
      uint16_t glow = scaleColor565(color, 80);

      drawPixelMapped(x, y - 1, glow);
      drawPixelMapped(x, y + 1, glow);
      drawPixelMapped(x, y, color);
      if (((x + wave * 3 + (int)(t / 80)) % 9) == 0) {
        drawPixelMapped(x, y, WHITE);
      }

      if (hasLast) {
        drawLineMapped(x - 1, lastY, x, y, color);
      }

      lastY = y;
      hasLast = true;
    }
  }
}

void drawNoiseFlowFrame(uint32_t t) {
  float time = t * 0.0018f;
  float turbulence = noiseFlowAmount / 10.0f;
  uint32_t grainFrame = t / 55;

  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      float px = x * 0.22f;
      float py = y * 0.28f;

      // Two moving noise-like fields bend the coordinates before sampling.
      float warpX = sinf(py * 1.37f + time * 1.9f) + cosf(px * 0.71f - time * 1.1f);
      float warpY = cosf(px * 1.21f - time * 1.4f) + sinf(py * 0.83f + time * 0.8f);
      float qx = px + warpX * (0.18f + turbulence * 0.72f);
      float qy = py + warpY * (0.18f + turbulence * 0.72f);

      float octaveA = sinf(qx * 1.35f + qy * 0.77f + time);
      float octaveB = cosf(qx * 2.63f - qy * 1.91f - time * 1.43f) * 0.50f;
      float octaveC = sinf(qx * 5.17f + qy * 3.41f + time * 2.17f) * 0.25f;
      float field = (octaveA + octaveB * turbulence + octaveC * turbulence) /
                    (1.0f + 0.75f * turbulence);

      // Thin bright contours make the flow resemble electric liquid.
      float contour = 1.0f - fminf(1.0f, fabsf(sinf(field * 7.5f + time * 0.7f)) * 3.2f);
      float glow = 0.20f + 0.46f * (field * 0.5f + 0.5f) + contour * 0.55f;
      uint8_t hue = (uint8_t)(t / 13 + field * 75.0f + warpX * 24.0f + x * 2);
      uint8_t brightness = (uint8_t)fminf(255.0f, glow * 255.0f);

      if (noiseFlowAmount > 0) {
        uint8_t grain = tvNoise(x, y, grainFrame);
        if (grain > 255 - noiseFlowAmount * 3) {
          brightness = 255;
          hue += 96;
        } else {
          int grainOffset = ((int)grain - 127) * noiseFlowAmount / 80;
          brightness = (uint8_t)constrain((int)brightness + grainOffset, 6, 255);
        }
      }
      drawPixelMapped(x, y, scaleColor565(colorWheel(hue), brightness));
    }
  }
}

void drawPhaseBeatFrame(uint32_t t) {
  float time = t * 0.0036f;
  float drift = sinf(time * 0.23f) * 2.7f;

  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      float wash = sinf(x * 0.20f + time) * cosf(y * 0.55f - time * 0.65f);
      uint8_t hue = (uint8_t)(160 + x * 3 + y * 9 + t / 35);
      drawPixelMapped(x, y, scaleColor565(colorWheel(hue), (uint8_t)(8 + 18 * (0.5f + 0.5f * wash))));
    }
  }

  int lastA = 0;
  int lastB = 0;
  bool hasLast = false;
  for (int x = 0; x < PANEL_RES_X; x++) {
    float waveA = sinf(x * 0.48f + time);
    float waveB = sinf(x * 0.48f + time + drift);
    int yA = (int)(7.5f + waveA * 4.8f + 0.5f);
    int yB = (int)(7.5f + waveB * 4.8f + 0.5f);
    uint16_t colorA = colorWheel((uint8_t)(t / 11 + x * 5));
    uint16_t colorB = colorWheel((uint8_t)(96 + t / 13 + x * 5));

    drawPixelMapped(x, yA, colorA);
    drawPixelMapped(x, yA - 1, scaleColor565(colorA, 70));
    drawPixelMapped(x, yB, colorB);
    drawPixelMapped(x, yB + 1, scaleColor565(colorB, 70));

    if (abs(yA - yB) <= 1) {
      drawSoftGlow(x, (yA + yB) / 2, 1.6f, WHITE, 210);
      drawPixelMapped(x, (yA + yB) / 2, WHITE);
    }

    if (hasLast) {
      drawLineMapped(x - 1, lastA, x, yA, colorA);
      drawLineMapped(x - 1, lastB, x, yB, colorB);
    }

    lastA = yA;
    lastB = yB;
    hasLast = true;
  }
}

void drawLissajousFrame(uint32_t t) {
  float time = t * 0.0028f;
  float ratioDrift = 0.35f * sinf(time * 0.31f);
  matrix->fillScreen(BLACK);

  for (int x = 0; x < PANEL_RES_X; x += 4) {
    drawPixelMapped(x, PANEL_RES_Y / 2, matrix->color565(6, 10, 18));
  }
  for (int y = 0; y < PANEL_RES_Y; y += 4) {
    drawPixelMapped(PANEL_RES_X / 2, y, matrix->color565(6, 10, 18));
  }

  int lastX = 0;
  int lastY = 0;
  bool hasLast = false;
  for (int i = 0; i < 96; i++) {
    float p = i * 0.131f;
    float xWave = sinf(p * (2.0f + ratioDrift) + time * 1.35f);
    float yWave = sinf(p * 3.0f + time * 0.93f + sinf(time * 0.19f) * 1.4f);
    int x = (int)(15.5f + xWave * 13.5f + 0.5f);
    int y = (int)(7.5f + yWave * 6.3f + 0.5f);
    uint16_t color = colorWheel((uint8_t)(i * 3 + t / 18));

    drawPixelMapped(x, y, color);
    if ((i & 0x07) == 0) {
      drawPixelMapped(x + 1, y, scaleColor565(color, 100));
      drawPixelMapped(x, y + 1, scaleColor565(color, 100));
    }

    if (hasLast) {
      drawLineMapped(lastX, lastY, x, y, scaleColor565(color, 190));
    }

    lastX = x;
    lastY = y;
    hasLast = true;
  }
}

void drawHarmonicsFrame(uint32_t t) {
  float time = t * 0.0032f;

  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      float carrier = sinf(x * 0.28f + time * 0.7f) + sinf(y * 0.62f - time);
      uint8_t hue = (uint8_t)(x * 7 + y * 11 + t / 28);
      uint8_t brightness = (uint8_t)constrain(14.0f + carrier * 8.0f, 3.0f, 32.0f);
      drawPixelMapped(x, y, scaleColor565(colorWheel(hue), brightness));
    }
  }

  for (int harmonic = 1; harmonic <= 4; harmonic++) {
    int lastY = 0;
    bool hasLast = false;
    uint8_t baseHue = (uint8_t)(harmonic * 48 + t / 14);
    for (int x = 0; x < PANEL_RES_X; x++) {
      float sample = 0.0f;
      for (int n = 1; n <= harmonic; n++) {
        sample += sinf(x * 0.30f * n + time * (1.15f + n * 0.13f)) / n;
      }
      sample /= 1.9f;
      int center = 2 + harmonic * 3;
      int y = (int)(center + sample * 2.2f + 0.5f);
      uint16_t color = colorWheel((uint8_t)(baseHue + x * 3));

      drawPixelMapped(x, y, color);
      if (harmonic == 4) {
        drawPixelMapped(x, y - 1, scaleColor565(color, 80));
        drawPixelMapped(x, y + 1, scaleColor565(color, 80));
      }

      if (hasLast) {
        drawLineMapped(x - 1, lastY, x, y, color);
      }

      lastY = y;
      hasLast = true;
    }
  }
}

void drawCatRunFrame(uint32_t t) {
  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      float sky = 0.5f + 0.5f * sinf((x * 0.25f) - (t * 0.0018f));
      uint8_t r = (uint8_t)(30 + 30 * sky);
      uint8_t g = (uint8_t)(10 + 16 * sky);
      uint8_t b = (uint8_t)(60 + 70 * sky);
      if (y > PANEL_RES_Y / 2) {
        r = (uint8_t)(8 + y * 4);
        g = (uint8_t)(20 + y * 6);
        b = (uint8_t)(20 + y * 2);
      }
      drawPixelMapped(x, y, matrix->color565(r, g, b));
    }
  }

  int groundY = PANEL_RES_Y - 2;
  for (int x = 0; x < PANEL_RES_X; x++) {
    uint8_t stripe = (uint8_t)((x * 14 + (t / 8)) & 0x3F);
    drawPixelMapped(x, groundY, matrix->color565(20 + stripe, 180, 90));
    if (groundY + 1 < PANEL_RES_Y) {
      drawPixelMapped(x, groundY + 1, matrix->color565(8, 70, 35));
    }
  }

  for (int s = 0; s < 8; s++) {
    int sx = (s * 19 + (int)(t / 32)) % PANEL_RES_X;
    int sy = 1 + ((s * 3) % 5);
    drawPixelMapped(sx, sy, matrix->color565(255, 240, 180));
  }

  catRunOffsetX += 1;
  if (catRunOffsetX > PANEL_RES_X) {
    catRunOffsetX = -12;
  }
  catRunFrame = (uint8_t)((t / 90) & 0x03);

  drawCatFrame(catRunOffsetX, PANEL_RES_Y - 10, catRunFrame);

  if (catRunOffsetX > -10 && catRunOffsetX < PANEL_RES_X) {
    int dustX = catRunOffsetX - 1;
    int dustY = PANEL_RES_Y - 2;
    if (dustX >= 0 && dustX < PANEL_RES_X) {
      drawPixelMapped(dustX, dustY, matrix->color565(180, 180, 120));
    }
    if (dustX - 1 >= 0 && (catRunFrame & 1)) {
      drawPixelMapped(dustX - 1, dustY - 1, matrix->color565(120, 120, 80));
    }
  }
}

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
  Serial.println("  sphere       -> run a fake 3D particle sphere");
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
 
void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    clearSerialInput();
    handleCommand(input);
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
