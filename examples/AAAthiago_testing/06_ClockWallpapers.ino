// Animated clock wallpapers designed for the logical 32x16 panel.
uint8_t clockWallpaperHash(uint8_t x, uint8_t y, uint8_t seed) {
  uint16_t value = x * 73u + y * 151u + seed * 199u;
  value ^= value << 7;
  value ^= value >> 5;
  return (uint8_t)value;
}

void drawWallpaperDigit(int x, int y, char digit, uint16_t color) {
  static const uint8_t rows[10][5] = {
    { 7, 5, 5, 5, 7 }, { 2, 6, 2, 2, 7 }, { 7, 1, 7, 4, 7 },
    { 7, 1, 7, 1, 7 }, { 5, 5, 7, 1, 1 }, { 7, 4, 7, 1, 7 },
    { 7, 4, 7, 5, 7 }, { 7, 1, 2, 2, 2 }, { 7, 5, 7, 5, 7 },
    { 7, 5, 7, 1, 7 }
  };
  if (digit < '0' || digit > '9') return;
  uint8_t n = digit - '0';
  for (uint8_t row = 0; row < 5; row++) {
    for (uint8_t col = 0; col < 3; col++) {
      if (rows[n][row] & (1 << (2 - col))) drawPixelMapped(x + col, y + row, color);
    }
  }
}

void drawWallpaperClock(uint32_t t, uint16_t color) {
  String value = getCurrentClockText();
  // A dark one-pixel halo keeps the clock legible over every wallpaper.
  for (int y = 4; y <= 10; y++) {
    drawPixelMapped(6, y, BLACK);
    drawPixelMapped(25, y, BLACK);
  }
  drawWallpaperDigit(7, 5, value[0], color);
  drawWallpaperDigit(11, 5, value[1], color);
  drawWallpaperDigit(18, 5, value[3], color);
  drawWallpaperDigit(22, 5, value[4], color);
  if ((t / 500) & 1) {
    drawPixelMapped(16, 6, color);
    drawPixelMapped(16, 8, color);
  }
}

void drawRainClockFrame(uint32_t t) {
  matrix->fillScreen(matrix->color565(0, 2, 8));
  uint8_t step = t / 90;
  for (uint8_t x = 0; x < PANEL_RES_X; x += 2) {
    int head = (step + clockWallpaperHash(x, 0, 11)) % 22 - 3;
    uint16_t color = colorWheel(x * 8 + step * 2);
    for (uint8_t tail = 0; tail < 4; tail++) {
      int y = head - tail;
      if (y >= 0 && y < PANEL_RES_Y) drawPixelMapped(x, y, tail == 0 ? WHITE : scaleColor565(color, 170 - tail * 35));
    }
  }
  drawWallpaperClock(t, WHITE);
}

void drawStarClockFrame(uint32_t t) {
  matrix->fillScreen(matrix->color565(0, 0, 12));
  uint8_t drift = t / 180;
  for (uint8_t i = 0; i < 28; i++) {
    uint8_t x = (clockWallpaperHash(i, 3, 21) + drift * (1 + i % 3)) % PANEL_RES_X;
    uint8_t y = clockWallpaperHash(i, 9, 17) % PANEL_RES_Y;
    uint8_t glow = 80 + ((t / 25 + i * 37) % 176);
    drawPixelMapped(x, y, matrix->color565(glow, glow, min(255, glow + 45)));
  }
  int cometX = (t / 110) % 42 - 5;
  for (uint8_t i = 0; i < 4; i++) drawPixelMapped(cometX - i, 2 + i, scaleColor565(WHITE, 255 - i * 50));
  drawWallpaperClock(t, matrix->color565(210, 225, 255));
}

void drawAquariumClockFrame(uint32_t t) {
  matrix->fillScreen(matrix->color565(0, 12, 30));
  for (uint8_t y = 0; y < PANEL_RES_Y; y += 3) {
    for (uint8_t x = (y + t / 180) % 5; x < PANEL_RES_X; x += 5) drawPixelMapped(x, y, matrix->color565(0, 35 + y * 3, 70 + y * 5));
  }
  for (uint8_t i = 0; i < 6; i++) {
    int bx = 2 + i * 5;
    int by = 15 - ((t / (130 + i * 17) + i * 3) % 18);
    drawPixelMapped(bx, by, matrix->color565(90, 180, 220));
  }
  int fish1 = (t / 100) % 38 - 3;
  drawPixelMapped(fish1, 2, YELLOW); drawPixelMapped(fish1 + 1, 2, YELLOW);
  drawPixelMapped(fish1 - 1, 1, matrix->color565(255, 100, 20)); drawPixelMapped(fish1 - 1, 3, matrix->color565(255, 100, 20));
  int fish2 = 34 - ((t / 145) % 40);
  drawPixelMapped(fish2, 13, MAGENTA); drawPixelMapped(fish2 - 1, 13, MAGENTA);
  drawPixelMapped(fish2 + 1, 12, CYAN); drawPixelMapped(fish2 + 1, 14, CYAN);
  drawWallpaperClock(t, WHITE);
}

void drawMatrixClockFrame(uint32_t t) {
  matrix->fillScreen(BLACK);
  uint8_t step = t / 75;
  for (uint8_t x = 0; x < PANEL_RES_X; x += 2) {
    int head = (step * (1 + x % 3) + clockWallpaperHash(x, 2, 5)) % 23 - 4;
    for (uint8_t tail = 0; tail < 6; tail++) {
      int y = head - tail;
      if (y >= 0 && y < PANEL_RES_Y && ((x + y + step) & 1)) {
        uint8_t green = tail == 0 ? 255 : 150 - tail * 22;
        drawPixelMapped(x, y, matrix->color565(tail == 0 ? 150 : 0, green, tail == 0 ? 150 : 15));
      }
    }
  }
  drawWallpaperClock(t, matrix->color565(190, 255, 190));
}

void drawTetrisClockFrame(uint32_t t) {
  matrix->fillScreen(matrix->color565(3, 3, 16));
  static const uint8_t heights[16] = { 2, 4, 3, 1, 5, 2, 4, 3, 2, 5, 1, 4, 3, 2, 4, 1 };
  uint8_t phase = t / 260;
  for (uint8_t col = 0; col < 16; col++) {
    uint8_t height = heights[(col + phase / 8) % 16];
    for (uint8_t row = 0; row < height; row++) {
      uint16_t color = colorWheel(col * 16 + row * 25);
      drawPixelMapped(col * 2, 15 - row * 2, color);
      drawPixelMapped(col * 2 + 1, 15 - row * 2, color);
      drawPixelMapped(col * 2, 14 - row * 2, scaleColor565(color, 180));
      drawPixelMapped(col * 2 + 1, 14 - row * 2, scaleColor565(color, 180));
    }
  }
  int fallingX = (phase * 7) % 30;
  int fallingY = (phase % 9) - 2;
  uint16_t fallingColor = colorWheel(phase * 19);
  drawPixelMapped(fallingX, fallingY, fallingColor); drawPixelMapped(fallingX + 1, fallingY, fallingColor);
  drawPixelMapped(fallingX + 1, fallingY + 1, fallingColor); drawPixelMapped(fallingX + 2, fallingY + 1, fallingColor);
  drawWallpaperClock(t, WHITE);
}

void drawOrbitClockFrame(uint32_t t) {
  matrix->fillScreen(matrix->color565(2, 0, 15));
  float angle = t * 0.0032f;
  for (uint8_t i = 0; i < 12; i++) {
    float a = angle * (i % 2 ? -1.0f : 1.0f) + i * 0.524f;
    float radiusX = 6.0f + (i % 3) * 4.0f;
    float radiusY = 3.0f + (i % 3) * 1.8f;
    int x = 16 + (int)(cos(a) * radiusX);
    int y = 7 + (int)(sin(a) * radiusY);
    drawPixelMapped(x, y, colorWheel(i * 21 + t / 18));
  }
  drawWallpaperClock(t, WHITE);
}
