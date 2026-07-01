// Calm, seamless procedural scenes intended to run indefinitely as wallpapers.

uint8_t wallpaperHash(uint8_t x, uint8_t y, uint16_t seed) {
  uint16_t value = (uint16_t)x * 157u + (uint16_t)y * 263u + seed * 37u;
  value = (value ^ (value >> 7)) * 109u;
  return (uint8_t)(value ^ (value >> 8));
}

void drawSilkFrame(uint32_t t) {
  float time = t * 0.00105f;
  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      float px = (x - 15.5f) * 0.19f;
      float py = (y - 7.5f) * 0.27f;
      float foldA = sinf(px * 1.7f + time + sinf(py * 1.3f - time * 0.6f) * 1.8f);
      float foldB = cosf(py * 2.1f - time * 0.8f + sinf(px + time * 0.4f) * 1.2f);
      float fold = foldA * 0.66f + foldB * 0.34f;
      float highlight = powf(fmaxf(0.0f, 1.0f - fabsf(fold - 0.25f) * 1.5f), 4.0f);
      uint8_t hue = (uint8_t)(t / 52 + x * 3 + y * 5 + fold * 38.0f);
      uint8_t brightness = (uint8_t)constrain(42.0f + (fold + 1.0f) * 35.0f + highlight * 150.0f, 12.0f, 255.0f);
      drawPixelMapped(x, y, scaleColor565(colorWheel(hue), brightness));
    }
  }
}

void drawSilkClockFrame(uint32_t t) {
  drawSilkFrame(t);
  drawWallpaperClock(t, WHITE);
}

void drawDeepSeaFrame(uint32_t t) {
  float time = t * 0.00075f;
  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      float caustic = sinf(x * 0.42f + time * 2.1f + sinf(y * 0.55f - time) * 1.5f);
      float depth = 1.0f - y / 15.0f;
      uint8_t blue = (uint8_t)constrain(15.0f + depth * 58.0f + caustic * 11.0f, 3.0f, 90.0f);
      drawPixelMapped(x, y, matrix->color565(0, blue / 3, blue));
    }
  }

  for (uint8_t i = 0; i < 11; i++) {
    float speed = 0.45f + (i % 4) * 0.12f;
    float travel = fmodf(time * speed + i * 1.73f, 1.35f);
    int y = (int)(17.0f - travel * 24.0f);
    int x = (int)(wallpaperHash(i, 5, 31) % 28 + 2 + sinf(time * 1.7f + i * 2.2f) * 2.5f);
    uint16_t glowColor = colorWheel((uint8_t)(118 + i * 8 + t / 80));
    drawSoftGlow(x, y, 1.8f, glowColor, 135);
    drawPixelMapped(x, y, scaleColor565(glowColor, 235));
    if ((i % 3) == 0) drawPixelMapped(x, y + 1, scaleColor565(glowColor, 105));
  }

  for (int x = 0; x < PANEL_RES_X; x += 3) {
    int tip = 12 + (int)(2.0f + sinf(time * 1.4f + x * 0.43f) * 1.5f);
    uint16_t plant = scaleColor565(colorWheel((uint8_t)(105 + x * 3 + t / 100)), 125);
    drawLineMapped(x, PANEL_RES_Y - 1, x + (int)sinf(time + x), tip, plant);
  }
  drawWallpaperClock(t, matrix->color565(180, 255, 255));
}
