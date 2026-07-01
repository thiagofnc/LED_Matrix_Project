// Standalone animation effects: aurora through sunrise.
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

