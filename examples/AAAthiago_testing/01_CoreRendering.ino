// Clock, remapping, drawing, text, emoji, and shared rendering helpers.
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

