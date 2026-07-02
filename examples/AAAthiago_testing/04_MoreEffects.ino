// Geometric, optical, television, audio, and character effects.
void drawSphereOrbitLayer(uint32_t t, float cx, float cy, float radius, bool front) {
  float orbit = t * 0.00155f;
  uint16_t orbitColor = colorWheel((uint8_t)(t / 22 + 92));

  for (uint8_t i = 0; i < 48; i++) {
    float angle = i * 0.1308997f + orbit;
    float depth = sinf(angle);
    if ((depth >= 0.0f) != front) continue;

    float px = cx + cosf(angle) * (radius + 2.0f);
    float py = cy + sinf(angle) * 2.25f + cosf(angle) * 0.55f;
    uint8_t brightness = (uint8_t)(front ? 135.0f + depth * 120.0f
                                        : 28.0f + (depth + 1.0f) * 42.0f);
    drawPixelMapped((int)(px + 0.5f), (int)(py + 0.5f),
                    scaleColor565(orbitColor, brightness));
  }
}

void drawSphereFrame(uint32_t t) {
  float seconds = t * 0.001f;
  float cx = 15.5f + sinf(seconds * 0.37f) * 0.75f;
  float cy = 7.5f + cosf(seconds * 0.43f) * 0.35f;
  float radius = 5.85f + sinf(seconds * 1.05f) * 0.42f;
  float spin = seconds * 1.18f;
  float tilt = sinf(seconds * 0.31f) * 0.62f;

  // A deep, subtly moving star field gives the orb scale without stealing focus.
  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      uint16_t hash = (uint16_t)(x * 73 + y * 151 + x * y * 17);
      float dx = x - cx;
      float dy = y - cy;
      float distance = sqrtf(dx * dx + dy * dy);
      float halo = fmaxf(0.0f, 1.0f - fabsf(distance - radius) / 3.0f);
      uint8_t blue = (uint8_t)(2.0f + halo * halo * 25.0f);
      uint16_t background = matrix->color565(blue / 5, blue / 3, blue);

      if ((hash % 41) == 0) {
        float twinkle = 0.5f + 0.5f * sinf(seconds * (1.6f + (hash & 3) * 0.31f) + hash);
        background = scaleColor565(colorWheel((uint8_t)(hash + t / 45)),
                                   (uint8_t)(30.0f + twinkle * 105.0f));
      }
      drawPixelMapped(x, y, background);
    }
  }

  // Draw the far half of the energy ring first so the globe properly occludes it.
  drawSphereOrbitLayer(t, cx, cy, radius, false);

  float lightX = -0.48f + sinf(seconds * 0.53f) * 0.18f;
  float lightY = -0.48f + cosf(seconds * 0.41f) * 0.14f;
  float lightZ = sqrtf(fmaxf(0.0f, 1.0f - lightX * lightX - lightY * lightY));
  float ct = cosf(tilt);
  float st = sinf(tilt);

  for (int y = 0; y < PANEL_RES_Y; y++) {
    for (int x = 0; x < PANEL_RES_X; x++) {
      float nx = (x - cx) / radius;
      float ny = (y - cy) / radius;
      float rr = nx * nx + ny * ny;
      if (rr > 1.0f) continue;

      float nz = sqrtf(1.0f - rr);
      float surfaceY = ny * ct - nz * st;
      float surfaceZ = ny * st + nz * ct;
      float longitude = atan2f(nx, surfaceZ) + spin;
      float latitude = asinf(fmaxf(-1.0f, fminf(1.0f, surfaceY)));

      // Two counter-moving waves make the texture continuously fold through itself.
      float ribbons = sinf(longitude * 5.0f + latitude * 3.0f + seconds * 1.35f);
      float filaments = sinf(longitude * 9.0f - latitude * 7.0f - seconds * 0.82f);
      float energy = 0.5f + 0.32f * ribbons + 0.18f * filaments;
      float diffuse = fmaxf(0.0f, nx * lightX + ny * lightY + nz * lightZ);
      float specular = powf(fmaxf(0.0f, nx * lightX + ny * lightY + nz * lightZ), 10.0f);
      float rim = powf(1.0f - nz, 1.7f);

      uint8_t hue = (uint8_t)(t / 28 + longitude * 25.0f + latitude * 34.0f + energy * 52.0f);
      float level = 38.0f + diffuse * 128.0f + energy * 48.0f + rim * 75.0f;
      uint8_t brightness = (uint8_t)fmaxf(22.0f, fminf(255.0f, level));
      uint16_t color = specular > 0.36f
                         ? matrix->color565(255, 245, 230)
                         : scaleColor565(colorWheel(hue), brightness);
      drawPixelMapped(x, y, color);
    }
  }

  // The near arc and traveling pearl sell the depth and keep the eye circling.
  drawSphereOrbitLayer(t, cx, cy, radius, true);
  float pearlAngle = seconds * 1.55f;
  int pearlX = (int)(cx + cosf(pearlAngle) * (radius + 2.0f) + 0.5f);
  int pearlY = (int)(cy + sinf(pearlAngle) * 2.25f + cosf(pearlAngle) * 0.55f + 0.5f);
  if (sinf(pearlAngle) >= 0.0f) drawPixelMapped(pearlX, pearlY, WHITE);
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

