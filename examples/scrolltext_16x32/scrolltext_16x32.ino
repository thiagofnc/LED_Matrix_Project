#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <math.h>

// ---------------- Panel settings ----------------
#define PANEL_RES_X 32
#define PANEL_RES_Y 16
#define PANEL_CHAIN 1

// ---------------- Your exact pin mapping ----------------
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
  -1, // E (not used for 32x16)
  4,  // LAT
  15, // OE
  16  // CLK
};

HUB75_I2S_CFG mxconfig(
  PANEL_RES_X,
  PANEL_RES_Y,
  PANEL_CHAIN,
  _pins
);

MatrixPanel_I2S_DMA *matrix = nullptr;

inline void drawPixelMapped(int x, int y, uint16_t color) {
  int mappedY = (y % 4) * 4 + (y / 4);
  matrix->drawPixel(x, mappedY, color);
}

// Same sine table from the original demo
static const int8_t sinetab[256] = {
     0,   2,   5,   8,  11,  15,  18,  21,
    24,  27,  30,  33,  36,  39,  42,  45,
    48,  51,  54,  56,  59,  62,  65,  67,
    70,  72,  75,  77,  80,  82,  85,  87,
    89,  91,  93,  96,  98, 100, 101, 103,
   105, 107, 108, 110, 111, 113, 114, 116,
   117, 118, 119, 120, 121, 122, 123, 123,
   124, 125, 125, 126, 126, 126, 126, 126,
   127, 126, 126, 126, 126, 126, 125, 125,
   124, 123, 123, 122, 121, 120, 119, 118,
   117, 116, 114, 113, 111, 110, 108, 107,
   105, 103, 101, 100,  98,  96,  93,  91,
    89,  87,  85,  82,  80,  77,  75,  72,
    70,  67,  65,  62,  59,  56,  54,  51,
    48,  45,  42,  39,  36,  33,  30,  27,
    24,  21,  18,  15,  11,   8,   5,   2,
     0,  -3,  -6,  -9, -12, -16, -19, -22,
   -25, -28, -31, -34, -37, -40, -43, -46,
   -49, -52, -55, -57, -60, -63, -66, -68,
   -71, -73, -76, -78, -81, -83, -86, -88,
   -90, -92, -94, -97, -99,-101,-102,-104,
  -106,-108,-109,-111,-112,-114,-115,-117,
  -118,-119,-120,-121,-122,-123,-124,-124,
  -125,-126,-126,-127,-127,-127,-127,-127,
  -128,-127,-127,-127,-127,-127,-126,-126,
  -125,-124,-124,-123,-122,-121,-120,-119,
  -118,-117,-115,-114,-112,-111,-109,-108,
  -106,-104,-102,-101, -99, -97, -94, -92,
   -90, -88, -86, -83, -81, -78, -76, -73,
   -71, -68, -66, -63, -60, -57, -55, -52,
   -49, -46, -43, -40, -37, -34, -31, -28,
   -25, -22, -19, -16, -12,  -9,  -6,  -3
};

const float radius1 = 65.2, radius2 = 92.0, radius3 = 163.2, radius4 = 176.8;
const float centerx1 = 64.4, centerx2 = 46.4, centerx3 = 93.6, centerx4 = 16.4;
const float centery1 = 34.8, centery2 = 26.0, centery3 = 56.0, centery4 = -11.6;

float angle1 = 0.0, angle2 = 0.0, angle3 = 0.0, angle4 = 0.0;
long hueShift = 0;

#define FPS 15
uint32_t prevTime = 0;

// HSV (0-1535-ish hue range works fine here) -> RGB565
uint16_t ColorHSV565(long hue, uint8_t sat, uint8_t val) {
  uint8_t r, g, b;

  hue %= 1536;
  if (hue < 0) hue += 1536;

  uint8_t region = hue / 256;
  uint8_t remainder = hue % 256;

  uint8_t p = (val * (255 - sat)) / 255;
  uint8_t q = (val * (255 - ((sat * remainder) >> 8))) / 255;
  uint8_t t = (val * (255 - ((sat * (255 - remainder)) >> 8))) / 255;

  switch (region) {
    case 0: r = val; g = t;   b = p;   break;
    case 1: r = q;   g = val; b = p;   break;
    case 2: r = p;   g = val; b = t;   break;
    case 3: r = p;   g = q;   b = val; break;
    case 4: r = t;   g = p;   b = val; break;
    default:r = val; g = p;   b = q;   break;
  }

  return matrix->color565(r, g, b);
}

void setup() {
  matrix = new MatrixPanel_I2S_DMA(mxconfig);
  matrix->begin();
  matrix->setBrightness8(96);
  matrix->clearScreen();
}

void loop() {
  int x1, x2, x3, x4, y1, y2, y3, y4, sx1, sx2, sx3, sx4;
  uint8_t x, y;
  long value;

  sx1 = (int)(cos(angle1) * radius1 + centerx1);
  sx2 = (int)(cos(angle2) * radius2 + centerx2);
  sx3 = (int)(cos(angle3) * radius3 + centerx3);
  sx4 = (int)(cos(angle4) * radius4 + centerx4);
  y1  = (int)(sin(angle1) * radius1 + centery1);
  y2  = (int)(sin(angle2) * radius2 + centery2);
  y3  = (int)(sin(angle3) * radius3 + centery3);
  y4  = (int)(sin(angle4) * radius4 + centery4);

  for (y = 0; y < PANEL_RES_Y; y++) {
    x1 = sx1; x2 = sx2; x3 = sx3; x4 = sx4;

    for (x = 0; x < PANEL_RES_X; x++) {
      value = hueShift
        + sinetab[(uint8_t)((x1 * x1 + y1 * y1) >> 4)]
        + sinetab[(uint8_t)((x2 * x2 + y2 * y2) >> 4)]
        + sinetab[(uint8_t)((x3 * x3 + y3 * y3) >> 5)]
        + sinetab[(uint8_t)((x4 * x4 + y4 * y4) >> 5)];

      drawPixelMapped(x, y, ColorHSV565(value * 3, 255, 255));

      x1--;
      x2--;
      x3--;
      x4--;
    }

    y1--;
    y2--;
    y3--;
    y4--;
  }

  angle1   += 0.03;
  angle2   -= 0.07;
  angle3   += 0.13;
  angle4   -= 0.15;
  hueShift += 2;

  uint32_t t;
  while (((t = millis()) - prevTime) < (1000 / FPS)) {}
  prevTime = t;
}
