#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <ESP32-VirtualMatrixPanel-I2S-DMA.h>
 
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
  MODE_TWINKLE
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
char rotatedChar = 'A';
int rainHead[PANEL_RES_X];
uint8_t rainLength[PANEL_RES_X];
uint8_t rainSpeed[PANEL_RES_X];
uint8_t rainTick = 0;
uint8_t fireHeat[PANEL_RES_X][PANEL_RES_Y];
uint16_t twinklePhase = 0;

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
  Serial.println("  twinkle      -> run neon starfield twinkle");
  Serial.println("  text <msg>   -> display remapped text using built-in 5x7 font");
  Serial.println("  rchar <c>    -> display one large char on rotated 16x32 view");
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
    matrix->fillScreen(BLACK);
    Serial.println("Panel cleared.");
    return;
  }

  if (input == "stop") {
    currentMode = MODE_HELP;
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
    currentMode = MODE_PLASMA;
    Serial.println("Plasma effect started. Send 'stop' to return to command mode.");
    return;
  }

  if (input == "twinkle") {
    currentMode = MODE_TWINKLE;
    Serial.println("Twinkle effect started. Send 'stop' to return to command mode.");
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

  if (currentMode == MODE_TWINKLE) {
    drawTwinkleFrame(millis());
    delay(30);
  }
}
