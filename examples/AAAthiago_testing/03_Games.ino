// Pac-Man and interactive game implementations.

void resetDinoGame() {
  dinoY = 10.0f;
  dinoVelocity = 0.0f;
  dinoObstacleX[0] = 34.0f;
  dinoObstacleX[1] = 49.0f;
  dinoObstacleX[2] = 66.0f;
  for (uint8_t i = 0; i < 3; i++) {
    dinoObstacleHeight[i] = random(3, 6);
    dinoObstaclePassed[i] = false;
  }
  dinoScore = 0;
  dinoDistance = 0;
  dinoGameOver = false;
  dinoLastFrame = millis();
}

void jumpDino() {
  if (dinoGameOver) {
    resetDinoGame();
    return;
  }
  if (dinoY >= 9.8f) dinoVelocity = -1.55f;
}

void drawDinoSprite(int x, int y, bool running) {
  uint16_t body = matrix->color565(230, 230, 230);
  // Seven pixels wide by five high: tail, body, long neck, head and two legs.
  drawPixelMapped(x + 3, y, body);
  drawPixelMapped(x + 4, y, body);
  drawPixelMapped(x + 5, y, body);
  drawPixelMapped(x + 6, y, body);
  drawPixelMapped(x + 3, y + 1, body);
  drawPixelMapped(x + 4, y + 1, body);
  drawPixelMapped(x + 5, y + 1, body);
  drawPixelMapped(x + 6, y + 1, body);
  drawPixelMapped(x + 1, y + 2, body);
  drawPixelMapped(x + 2, y + 2, body);
  drawPixelMapped(x + 3, y + 2, body);
  drawPixelMapped(x + 4, y + 2, body);
  drawPixelMapped(x, y + 3, body);
  drawPixelMapped(x + 1, y + 3, body);
  drawPixelMapped(x + 2, y + 3, body);
  drawPixelMapped(x + 3, y + 3, body);
  drawPixelMapped(x + 1, y + 4, body);
  drawPixelMapped(x + (running ? 3 : 2), y + 4, body);
  drawPixelMapped(x + 5, y, BLACK); // eye
}

void drawDinoFrame(uint32_t t) {
  if (t - dinoLastFrame < 55) return;
  dinoLastFrame = t;

  const float groundY = 15.0f;
  if (!dinoGameOver) {
    dinoVelocity += 0.15f;
    dinoY += dinoVelocity;
    if (dinoY >= 10.0f) {
      dinoY = 10.0f;
      dinoVelocity = 0.0f;
    }

    float speed = 0.48f + min(0.42f, dinoDistance / 2200.0f);
    float rightmost = dinoObstacleX[0];
    for (uint8_t i = 1; i < 3; i++) rightmost = max(rightmost, dinoObstacleX[i]);
    for (uint8_t i = 0; i < 3; i++) {
      dinoObstacleX[i] -= speed;
      if (dinoObstacleX[i] < -2.0f) {
        dinoObstacleX[i] = rightmost + random(16, 24);
        rightmost = dinoObstacleX[i];
        dinoObstacleHeight[i] = random(3, 6);
        dinoObstaclePassed[i] = false;
      }
      int ox = (int)dinoObstacleX[i];
      if (!dinoObstaclePassed[i] && ox + 1 < 4) {
        dinoObstaclePassed[i] = true;
        dinoScore++;
      }
      int dinoBottom = (int)dinoY + 4;
      int cactusTop = (int)groundY - dinoObstacleHeight[i] + 1;
      // Use the dinosaur's solid torso for collision; the tail and snout are visual detail.
      if (ox <= 9 && ox + 1 >= 5 && dinoBottom >= cactusTop) dinoGameOver = true;
    }
    dinoDistance++;
  }

  matrix->fillScreen(BLACK);
  uint16_t ground = matrix->color565(120, 120, 120);
  for (int x = 0; x < PANEL_RES_X; x++) {
    if (((x + dinoDistance / 2) % 5) != 0) drawPixelMapped(x, (int)groundY, ground);
  }
  for (uint8_t i = 0; i < 3; i++) {
    int ox = (int)dinoObstacleX[i];
    for (uint8_t h = 0; h < dinoObstacleHeight[i]; h++) drawPixelMapped(ox, (int)groundY - h, GREEN);
    drawPixelMapped(ox - 1, (int)groundY - 1, GREEN);
    drawPixelMapped(ox + 1, (int)groundY - 2, GREEN);
  }
  drawDinoSprite(4, (int)dinoY, ((dinoDistance / 3) & 1) != 0);

  uint8_t shownScore = min((uint16_t)99, dinoScore);
  if (shownScore >= 10) drawWallpaperDigit(24, 0, '0' + shownScore / 10, YELLOW);
  drawWallpaperDigit(28, 0, '0' + shownScore % 10, YELLOW);
  if (dinoGameOver) drawMappedText(9, 4, "HIT", RED);
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
    drawPixelMapped(x - 2, y + ey, color);
    drawPixelMapped(x - 1, y + ey, color);
    drawPixelMapped(x, y + ey, color);
    drawPixelMapped(x + 1, y + ey, color);
  }
  uint16_t pupil = WHITE;
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

void resetFlappyGame() {
  flappyY = 7.0f;
  flappyVelocity = 0.0f;
  flappyPipeX[0] = 24.0f;
  flappyPipeX[1] = 42.0f;
  flappyGapY[0] = random(4, 10);
  flappyGapY[1] = random(4, 10);
  flappyScore = 0;
  flappyGameOver = false;
  flappyLastFrame = millis();
}

void flapBird() {
  if (flappyGameOver) {
    resetFlappyGame();
  } else {
    flappyVelocity = -1.35f;
  }
}

void drawFlappyFrame(uint32_t t) {
  if (t - flappyLastFrame < 55) return;
  flappyLastFrame = t;
  matrix->fillScreen(matrix->color565(8, 35, 80));

  if (!flappyGameOver) {
    flappyVelocity += 0.18f;
    if (flappyVelocity > 1.15f) flappyVelocity = 1.15f;
    flappyY += flappyVelocity;

    for (uint8_t i = 0; i < 2; i++) {
      flappyPipeX[i] -= 0.48f;
      if (flappyPipeX[i] < -3.0f) {
        flappyPipeX[i] += 36.0f;
        flappyGapY[i] = random(4, 10);
        flappyScore++;
      }
    }
  }

  uint16_t pipeColor = matrix->color565(30, 230, 70);
  for (uint8_t i = 0; i < 2; i++) {
    int pipeX = (int)flappyPipeX[i];
    int gapTop = flappyGapY[i] - 3;
    int gapBottom = flappyGapY[i] + 3;
    for (int x = pipeX; x < pipeX + 3; x++) {
      for (int y = 0; y < PANEL_RES_Y; y++) {
        if (y < gapTop || y > gapBottom) drawPixelMapped(x, y, pipeColor);
      }
    }
    if (!flappyGameOver && pipeX <= 7 && pipeX + 2 >= 5) {
      int birdY = (int)flappyY;
      if (birdY < gapTop || birdY + 1 > gapBottom) flappyGameOver = true;
    }
  }

  int birdY = (int)flappyY;
  if (birdY < 0 || birdY + 1 >= PANEL_RES_Y) flappyGameOver = true;
  uint16_t birdColor = flappyGameOver ? RED : YELLOW;
  drawPixelMapped(5, birdY, birdColor);
  drawPixelMapped(6, birdY, birdColor);
  drawPixelMapped(5, birdY + 1, birdColor);
  drawPixelMapped(6, birdY + 1, WHITE);

  for (uint8_t i = 0; i < min((uint16_t)10, flappyScore); i++) {
    drawPixelMapped(1 + i, 1, WHITE);
  }
  if (flappyGameOver) {
    drawMappedText(7, 5, "HIT", WHITE);
  }
}

void resetPongGame() {
  pongX = 15.0f;
  pongY = 7.0f;
  pongVX = random(2) ? 0.55f : -0.55f;
  pongVY = random(2) ? 0.34f : -0.34f;
  pongLeftY = pongRightY = 5.0f;
  pongLastFrame = millis();
}

void drawPongClock(uint32_t t) {
  String timeText = getCurrentClockText();
  drawMappedText(3, 4, timeText.substring(0, 2), CYAN);
  drawMappedText(18, 4, timeText.substring(3, 5), MAGENTA);

  // A gently blinking colon makes the clock feel alive without distracting.
  if ((t / 500) % 2 == 0) {
    drawPixelMapped(16, 6, WHITE);
    drawPixelMapped(16, 9, WHITE);
  }
}

void drawPongFrame(uint32_t t) {
  if (t - pongLastFrame < 45) return;
  pongLastFrame = t;
  matrix->fillScreen(BLACK);
  for (int y = 0; y < PANEL_RES_Y; y += 2) drawPixelMapped(PANEL_RES_X / 2, y, matrix->color565(45, 45, 70));

  pongLeftY += ((pongY - 2.0f) - pongLeftY) * 0.22f;
  pongRightY += ((pongY - 2.0f) - pongRightY) * 0.18f;
  pongX += pongVX;
  pongY += pongVY;
  if (pongY <= 0 || pongY >= PANEL_RES_Y - 1) pongVY = -pongVY;
  pongY = constrain(pongY, 0.0f, (float)PANEL_RES_Y - 1.0f);
  pongLeftY = constrain(pongLeftY, 0.0f, (float)PANEL_RES_Y - 5.0f);
  pongRightY = constrain(pongRightY, 0.0f, (float)PANEL_RES_Y - 5.0f);
  if (pongX <= 2 && pongX >= 1 && pongVX < 0 && pongY >= pongLeftY && pongY <= pongLeftY + 4) pongVX = -pongVX;
  if (pongX >= PANEL_RES_X - 3 && pongX <= PANEL_RES_X - 2 && pongVX > 0 && pongY >= pongRightY && pongY <= pongRightY + 4) pongVX = -pongVX;
  if (pongX < 0 || pongX >= PANEL_RES_X) resetPongGame();

  for (int i = 0; i < 5; i++) {
    drawPixelMapped(1, (int)pongLeftY + i, CYAN);
    drawPixelMapped(PANEL_RES_X - 2, (int)pongRightY + i, MAGENTA);
  }
  drawPongClock(t);
  drawPixelMapped((int)pongX, (int)pongY, WHITE);
}

void resetBreakoutGame() {
  for (uint8_t row = 0; row < 3; row++) {
    for (uint8_t col = 0; col < 8; col++) breakoutBricks[row][col] = true;
  }
  breakoutX = 15.0f;
  breakoutY = 11.0f;
  breakoutVX = random(2) ? 0.45f : -0.45f;
  breakoutVY = -0.38f;
  breakoutPaddleX = 13.0f;
  breakoutLastFrame = millis();
}

void moveBreakoutPaddle(int8_t direction) {
  breakoutPaddleX += direction * 3;
  if (breakoutPaddleX < 0) breakoutPaddleX = 0;
  if (breakoutPaddleX > PANEL_RES_X - 5) breakoutPaddleX = PANEL_RES_X - 5;
}

void drawBreakoutFrame(uint32_t t) {
  uint16_t frameInterval = 70 - breakoutSpeed * 5;
  if (t - breakoutLastFrame < frameInterval) return;
  breakoutLastFrame = t;
  matrix->fillScreen(BLACK);
  uint16_t brickColors[3] = {MAGENTA, matrix->color565(255, 100, 20), CYAN};
  bool bricksRemain = false;
  for (uint8_t row = 0; row < 3; row++) {
    for (uint8_t col = 0; col < 8; col++) {
      if (!breakoutBricks[row][col]) continue;
      bricksRemain = true;
      for (uint8_t px = 0; px < 3; px++) drawPixelMapped(col * 4 + px, row + 1, brickColors[row]);
    }
  }
  if (!bricksRemain) resetBreakoutGame();

  breakoutX += breakoutVX;
  breakoutY += breakoutVY;
  if (breakoutX <= 0 || breakoutX >= PANEL_RES_X - 1) breakoutVX = -breakoutVX;
  if (breakoutY <= 0) breakoutVY = -breakoutVY;

  int ballRow = (int)breakoutY - 1;
  int ballCol = (int)breakoutX / 4;
  if (ballRow >= 0 && ballRow < 3 && ballCol >= 0 && ballCol < 8 && breakoutBricks[ballRow][ballCol]) {
    breakoutBricks[ballRow][ballCol] = false;
    breakoutVY = fabsf(breakoutVY);
  }
  if (breakoutY >= PANEL_RES_Y - 3 && breakoutY <= PANEL_RES_Y - 2 && breakoutVY > 0 &&
      breakoutX >= breakoutPaddleX && breakoutX <= breakoutPaddleX + 4) {
    breakoutVY = -breakoutVY;
    breakoutVX += (breakoutX - (breakoutPaddleX + 2.0f)) * 0.04f;
  }
  if (breakoutY >= PANEL_RES_Y) resetBreakoutGame();

  for (int i = 0; i < 5; i++) drawPixelMapped((int)breakoutPaddleX + i, PANEL_RES_Y - 2, YELLOW);
  drawPixelMapped((int)breakoutX, (int)breakoutY, WHITE);
}

void placeSnakeFood() {
  bool onSnake;
  do {
    onSnake = false;
    snakeFoodX = random(0, 16);
    snakeFoodY = random(0, 8);
    for (uint8_t i = 0; i < snakeLength; i++) {
      if (snakeX[i] == snakeFoodX && snakeY[i] == snakeFoodY) onSnake = true;
    }
  } while (onSnake);
}

void resetSnakeGame() {
  snakeLength = 4;
  for (uint8_t i = 0; i < snakeLength; i++) {
    snakeX[i] = 7 - i;
    snakeY[i] = 4;
  }
  snakeDX = 1;
  snakeDY = 0;
  snakeGameOver = false;
  snakeLastStep = millis();
  placeSnakeFood();
}

void steerSnake(int8_t dx, int8_t dy) {
  if (snakeGameOver) {
    resetSnakeGame();
    return;
  }
  if (dx == -snakeDX && dy == -snakeDY) return;
  snakeDX = dx;
  snakeDY = dy;
}

void drawSnakeFrame(uint32_t t) {
  if (!snakeGameOver && t - snakeLastStep >= 170) {
    snakeLastStep = t;
    int nextX = snakeX[0] + snakeDX;
    int nextY = snakeY[0] + snakeDY;
    if (nextX < 0 || nextX >= 16 || nextY < 0 || nextY >= 8) snakeGameOver = true;
    for (uint8_t i = 0; i < snakeLength && !snakeGameOver; i++) {
      if (snakeX[i] == nextX && snakeY[i] == nextY) snakeGameOver = true;
    }
    if (!snakeGameOver) {
      bool ate = nextX == snakeFoodX && nextY == snakeFoodY;
      if (ate && snakeLength < 128) snakeLength++;
      for (int i = snakeLength - 1; i > 0; i--) {
        snakeX[i] = snakeX[i - 1];
        snakeY[i] = snakeY[i - 1];
      }
      snakeX[0] = nextX;
      snakeY[0] = nextY;
      if (ate) placeSnakeFood();
    }
  }

  matrix->fillScreen(BLACK);
  uint16_t foodColor = matrix->color565(255, 40, 80);
  for (uint8_t oy = 0; oy < 2; oy++) for (uint8_t ox = 0; ox < 2; ox++)
    drawPixelMapped(snakeFoodX * 2 + ox, snakeFoodY * 2 + oy, foodColor);
  for (uint8_t i = 0; i < snakeLength; i++) {
    uint16_t segmentColor = i == 0 ? YELLOW : matrix->color565(20, 255 - min(180, i * 5), 80);
    for (uint8_t oy = 0; oy < 2; oy++) for (uint8_t ox = 0; ox < 2; ox++)
      drawPixelMapped(snakeX[i] * 2 + ox, snakeY[i] * 2 + oy, segmentColor);
  }
  if (snakeGameOver) drawMappedText(7, 5, "HIT", WHITE);
}

void resetInvadersGame() {
  for (uint8_t i = 0; i < 12; i++) invaderAlive[i] = true;
  invaderOffsetX = 1;
  invaderOffsetY = 1;
  invaderDirection = 1;
  invaderPlayerX = 15;
  invaderBulletX = -1;
  invaderBulletY = -1;
  invaderGameOver = false;
  invaderWon = false;
  invaderLastFrame = millis();
  invaderLastMove = millis();
}

void controlInvaders(int8_t direction, bool fire) {
  if (invaderGameOver || invaderWon) {
    resetInvadersGame();
    return;
  }
  if (direction != 0) {
    invaderPlayerX += direction * 2;
    if (invaderPlayerX < 1) invaderPlayerX = 1;
    if (invaderPlayerX > PANEL_RES_X - 2) invaderPlayerX = PANEL_RES_X - 2;
  }
  if (fire && invaderBulletY < 0) {
    invaderBulletX = invaderPlayerX;
    invaderBulletY = PANEL_RES_Y - 3;
  }
}

void drawInvadersFrame(uint32_t t) {
  if (t - invaderLastFrame >= 70 && !invaderGameOver && !invaderWon) {
    invaderLastFrame = t;
    if (invaderBulletY >= 0) {
      invaderBulletY--;
      for (uint8_t i = 0; i < 12; i++) {
        if (!invaderAlive[i]) continue;
        int alienX = invaderOffsetX + (i % 6) * 5;
        int alienY = invaderOffsetY + (i / 6) * 3;
        if (invaderBulletX >= alienX && invaderBulletX <= alienX + 1 && invaderBulletY == alienY) {
          invaderAlive[i] = false;
          invaderBulletY = -1;
          break;
        }
      }
      if (invaderBulletY < 0) invaderBulletX = -1;
    }
  }

  if (t - invaderLastMove >= 420 && !invaderGameOver && !invaderWon) {
    invaderLastMove = t;
    int nextOffset = invaderOffsetX + invaderDirection;
    if (nextOffset < 0 || nextOffset + 27 >= PANEL_RES_X) {
      invaderDirection = -invaderDirection;
      invaderOffsetY++;
    } else {
      invaderOffsetX = nextOffset;
    }
    if (invaderOffsetY + 3 >= PANEL_RES_Y - 3) invaderGameOver = true;
  }

  bool anyAlive = false;
  matrix->fillScreen(BLACK);
  for (uint8_t i = 0; i < 12; i++) {
    if (!invaderAlive[i]) continue;
    anyAlive = true;
    int alienX = invaderOffsetX + (i % 6) * 5;
    int alienY = invaderOffsetY + (i / 6) * 3;
    uint16_t alienColor = (i / 6) == 0 ? MAGENTA : CYAN;
    drawPixelMapped(alienX, alienY, alienColor);
    drawPixelMapped(alienX + 1, alienY, alienColor);
    drawPixelMapped(alienX, alienY + 1, alienColor);
    drawPixelMapped(alienX + 1, alienY + 1, alienColor);
  }
  if (!anyAlive) invaderWon = true;
  drawPixelMapped(invaderPlayerX, PANEL_RES_Y - 2, YELLOW);
  drawPixelMapped(invaderPlayerX - 1, PANEL_RES_Y - 1, YELLOW);
  drawPixelMapped(invaderPlayerX, PANEL_RES_Y - 1, YELLOW);
  drawPixelMapped(invaderPlayerX + 1, PANEL_RES_Y - 1, YELLOW);
  if (invaderBulletY >= 0) drawPixelMapped(invaderBulletX, invaderBulletY, WHITE);
  if (invaderGameOver) drawMappedText(7, 5, "HIT", WHITE);
  if (invaderWon) drawMappedText(7, 5, "WIN", GREEN);
}

void resetRhythmGame() {
  for (uint8_t i = 0; i < RHYTHM_NOTE_COUNT; i++) rhythmNoteActive[i] = false;
  rhythmScore = 0;
  rhythmCombo = 0;
  rhythmFlashLane = 255;
  rhythmFlashFrames = 0;
  rhythmHitSuccess = false;
  rhythmLastStep = millis();
  rhythmLastSpawn = millis() - (680 - rhythmSpeed * 40);
}

void spawnRhythmNote() {
  for (uint8_t i = 0; i < RHYTHM_NOTE_COUNT; i++) {
    if (rhythmNoteActive[i]) continue;
    rhythmNoteActive[i] = true;
    rhythmNoteLane[i] = random(0, 4);
    rhythmNoteY[i] = 0;
    return;
  }
}

void hitRhythmLane(uint8_t lane) {
  int bestNote = -1;
  int bestDistance = 99;
  for (uint8_t i = 0; i < RHYTHM_NOTE_COUNT; i++) {
    if (!rhythmNoteActive[i] || rhythmNoteLane[i] != lane) continue;
    int distance = abs(13 - rhythmNoteY[i]);
    if (distance <= 2 && distance < bestDistance) {
      bestDistance = distance;
      bestNote = i;
    }
  }

  rhythmFlashLane = lane;
  rhythmFlashFrames = 4;
  rhythmHitSuccess = bestNote >= 0;
  if (bestNote >= 0) {
    rhythmNoteActive[bestNote] = false;
    rhythmCombo++;
    rhythmScore += bestDistance == 0 ? 3 : (bestDistance == 1 ? 2 : 1);
  } else {
    rhythmCombo = 0;
  }
}

void drawRhythmFrame(uint32_t t) {
  uint16_t spawnInterval = 680 - rhythmSpeed * 40;
  uint16_t stepInterval = 110 - rhythmSpeed * 5;
  if (t - rhythmLastSpawn >= spawnInterval) {
    rhythmLastSpawn = t;
    spawnRhythmNote();
  }
  if (t - rhythmLastStep >= stepInterval) {
    rhythmLastStep = t;
    for (uint8_t i = 0; i < RHYTHM_NOTE_COUNT; i++) {
      if (!rhythmNoteActive[i]) continue;
      rhythmNoteY[i]++;
      if (rhythmNoteY[i] > 14) {
        rhythmNoteActive[i] = false;
        rhythmCombo = 0;
      }
    }
    if (rhythmFlashFrames > 0) rhythmFlashFrames--;
  }

  matrix->fillScreen(BLACK);
  uint16_t laneColors[4] = {CYAN, GREEN, MAGENTA, matrix->color565(255, 110, 20)};
  for (uint8_t lane = 0; lane < 4; lane++) {
    int laneLeft = lane * 8;
    for (int y = 0; y < PANEL_RES_Y; y += 2) drawPixelMapped(laneLeft, y, matrix->color565(18, 18, 35));
    for (uint8_t x = 1; x <= 6; x++) drawPixelMapped(laneLeft + x, 14, scaleColor565(laneColors[lane], 100));
  }
  for (uint8_t i = 0; i < RHYTHM_NOTE_COUNT; i++) {
    if (!rhythmNoteActive[i]) continue;
    int noteX = rhythmNoteLane[i] * 8 + 2;
    for (uint8_t ox = 0; ox < 4; ox++) {
      drawPixelMapped(noteX + ox, rhythmNoteY[i], laneColors[rhythmNoteLane[i]]);
      drawPixelMapped(noteX + ox, rhythmNoteY[i] + 1, laneColors[rhythmNoteLane[i]]);
    }
  }
  if (rhythmFlashFrames > 0 && rhythmFlashLane < 4) {
    uint16_t flashColor = rhythmHitSuccess ? WHITE : RED;
    int flashX = rhythmFlashLane * 8 + 1;
    for (uint8_t x = 0; x < 6; x++) {
      drawPixelMapped(flashX + x, 13, flashColor);
      drawPixelMapped(flashX + x, 14, flashColor);
    }
  }
  uint8_t meter = min((uint16_t)8, rhythmCombo);
  for (uint8_t i = 0; i < meter; i++) drawPixelMapped(12 + i, 0, YELLOW);
}

