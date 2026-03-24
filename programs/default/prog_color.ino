// =====================================================================
// PROGRAM 3: COLOR LAB (Video_Gem_Color by RMNA)
// 12 presets + 4 palettes. Lab mode: disables global palette/speed/trail.
// Palette mode from p4; pattern params from p0-p3; palette params p5-p15.
// =====================================================================

static float col_normalizedTime(int periodMS, float phase) {
  return ((millis() + (int)(phase * periodMS)) % periodMS) / (float)periodMS;
}

static float col_sineTime(int periodMS, float phase) {
  return sinf(col_normalizedTime(periodMS, phase) * TWO_PI);
}

static int col_cyclicalPotMap(int potIdx, float multiple, float phase, int lo, int hi) {
  int diff = hi - lo;
  float cycle = sinf((pots[potIdx] / 1023.0f) * multiple * TWO_PI + (phase * TWO_PI)) * 0.5f + 0.5f;
  return lo + (int)(diff * cycle);
}

static int col_lerp(int a, int b, float t) {
  return a + (int)((b - a) * t);
}

static void col_drawPolygon(int cx, int cy, int r, int sides, float rot01, uint8_t c) {
  drawPoly(cx, cy, r, sides, rot01 * TWO_PI, c);
}

// ─── Palettes (program owns color when active) ────────────────────────

static void col_artisanalPalette() {
  for (int i = 0; i < 50; i++) display.setColor(i, i, 0, 0);
  for (int i = 51; i < 100; i++) display.setColor(i, 255 - i, 0, i);
  for (int i = 101; i < 150; i++) display.setColor(i, i / 2, (i * i) % 255, 37);
  for (int i = 151; i < 200; i++) display.setColor(i, (i * 17) % 255, i, 0);
  for (int i = 200; i < 255; i++) {
    randomSeed(millis() / 1000);
    display.setColor(i, random(256), random(256), random(256));
  }
  display.setColor(0, 0, 0, 0);
  display.setColor(255, 255, 255, 255);
}

static void col_fourColorPalette() {
  for (int i = 0; i < 4; i++) {
    int r = potMap(5 + i, 0, 255);
    int g = potMap(9 + i, 0, 255);
    int b = potMap(13 + i, 0, 255);
    for (int j = 0; j < 64; j++)
      display.setColor(i * 64 + j, r, g, b);
  }
  display.setColor(0, 0, 0, 0);
  display.setColor(255, 255, 255, 255);
}

static void col_gradientPalette() {
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 85; j++) {
      display.setColor(
        i * 85 + j,
        col_lerp(potMap(5 + i, 0, 255), potMap(6 + i, 0, 255), j / 85.0f),
        col_lerp(potMap(9 + i, 0, 255), potMap(10 + i, 0, 255), j / 85.0f),
        col_lerp(potMap(13 + i, 0, 255), potMap(14 + i, 0, 255), j / 85.0f)
      );
    }
  }
  display.setColor(255, potMap(8, 0, 255), potMap(12, 0, 255), potMap(15, 0, 255));
  display.setColor(0, 0, 0, 0);
}

static void col_wavePalette() {
  float rFreq = pots[5] / 1023.0f, rAmp = pots[6] / 1023.0f, rPh = pots[7] / 1023.0f, rOff = pots[8] / 1023.0f;
  float gFreq = pots[9] / 1023.0f, gAmp = pots[10] / 1023.0f, gPh = pots[11] / 1023.0f, gOff = pots[12] / 1023.0f;
  float bFreq = pots[13] / 1023.0f, bAmp = pots[14] / 1023.0f, bPh = pots[15] / 1023.0f, bOff = 0.5f;
  for (int i = 0; i < 256; i++) {
    int r = constrain((int)(255.0f * (rOff + rAmp * sinf(rFreq * i / 8.0f + rPh * TWO_PI))), 0, 255);
    int g = constrain((int)(255.0f * (gOff + gAmp * sinf(gFreq * i / 8.0f + gPh * TWO_PI))), 0, 255);
    int b = constrain((int)(255.0f * (bOff + bAmp * sinf(bFreq * i / 8.0f + bPh * TWO_PI))), 0, 255);
    display.setColor(i, r, g, b);
  }
  display.setColor(0, 0, 0, 0);
  display.setColor(255, 255, 255, 255);
}

static void col_applyPalette(int mode) {
  switch (mode) {
    case 0: col_artisanalPalette(); break;
    case 1: col_fourColorPalette(); break;
    case 2: col_gradientPalette(); break;
    case 3: col_wavePalette(); break;
    default: col_artisanalPalette(); break;
  }
}

// ─── Presets (use p0–p3 for pattern params, same as legacy) ───────────

static void col_paletteTest() {
  display.fillScreen(0);
  for (int i = 0; i < 256; i++)
    display.drawFastVLine(32 + i, 0, H, i);
}

static void col_polygonGrid() {
  display.fillScreen(0);
  int r = potMap(0, 1, 150);
  int sides = potMap(1, 3, 20);
  float rotation = (float)potMap(2, 0, 359) / 360.0f;
  int rowSpacing = potMap(3, 40, HALFH);
  int colSpacing = potMap(3, 40, HALFW);
  int rows = H / rowSpacing, cols = W / colSpacing;
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      int x = j * colSpacing, y = i * rowSpacing;
      col_drawPolygon(HALFW + x, HALFH + y, r, sides, rotation, 255);
      if (i > 0) col_drawPolygon(HALFW + x, HALFH - y, r, sides, rotation, 255);
      if (j > 0) col_drawPolygon(HALFW - x, HALFH + y, r, sides, rotation, 255);
      if (i > 0 && j > 0) col_drawPolygon(HALFW - x, HALFH - y, r, sides, rotation, 255);
    }
  }
}

static void col_wobblyLines() {
  display.fillScreen(0);
  int copies = potMap(0, 1, 7);
  float phaseOffset = potMap(1, 0, 360) / 360.0f;
  int period = potMap(2, 50, 500) * 10;
  int c = potMap(3, 1, 255);
  int xRange = 15, xWidth = 30;
  int x0 = HALFW + (int)(sinf(col_normalizedTime(period, 0) * TWO_PI) * xRange);
  int x1 = HALFW + (int)(sinf(col_normalizedTime(period, phaseOffset) * TWO_PI) * xRange);
  display.drawLine(x0, 0, x1, H, c);
  for (int i = 1; i < copies; i++) {
    x0 = HALFW + (i * xWidth) + (int)(sinf(col_normalizedTime(period, i * phaseOffset) * TWO_PI) * xRange);
    x1 = HALFW + (i * xWidth) + (int)(sinf(col_normalizedTime(period, (i + 1) * phaseOffset) * TWO_PI) * xRange);
    display.drawLine(x0, 0, x1, H, c);
    x0 = HALFW - (i * xWidth) + (int)(sinf(col_normalizedTime(period, i * phaseOffset) * TWO_PI) * xRange);
    x1 = HALFW - (i * xWidth) + (int)(sinf(col_normalizedTime(period, (i + 1) * phaseOffset) * TWO_PI) * xRange);
    display.drawLine(x0, 0, x1, H, c);
  }
}

static void col_gradientGrid() {
  int numRows = potMap(0, 1, 64);
  int numCols = potMap(1, 1, 64);
  int rw = (W + numCols - 1) / numCols, rh = (H + numRows - 1) / numRows;
  int iGap = potMap(2, 0, 255), jGap = potMap(3, 0, 255);
  int firstC = (millis() / 10) % 254 + 1;
  for (int i = 0; i < numCols; i++) {
    for (int j = 0; j < numRows; j++) {
      int c = (firstC + (i * iGap) + (j * jGap)) % 254 + 1;
      display.fillRect(i * rw, j * rh, rw, rh, c);
    }
  }
}

static void col_basicReflections() {
  display.fillScreen(0);
  int xRange = potMap(0, 0, 80), yRange = potMap(1, 0, 60);
  int r = potMap(2, 1, 100), mode = potMap(3, 0, 5);
  float t = col_normalizedTime(2000, 0);
  int cx = 80 + (int)(sinf(t * TWO_PI) * xRange);
  int cy = 60 + (int)(cosf(t * TWO_PI) * yRange);
  display.fillCircle(cx, cy, r, 255);
  if (mode == 1) display.fillCircle(W - cx, cy, r, 255);
  else if (mode == 2) display.fillCircle(cx, H - cy, r, 255);
  else if (mode == 3) display.fillCircle(W - cx, H - cy, r, 255);
  else if (mode == 4) {
    display.fillCircle(W - cx, cy, r, 255);
    display.fillCircle(cx, H - cy, r, 255);
    display.fillCircle(W - cx, H - cy, r, 255);
  }
}

static void col_reflectedRects() {
  display.fillScreen(0);
  randomSeed(millis() / 1000);
  int widthLimit = potMap(0, 1, HALFW / 2);
  int heightLimit = potMap(1, 1, H / 2);
  int quantity = potMap(2, 1, 10);
  int c = potMap(3, 0, 235);
  for (int i = 0; i < quantity; i++) {
    int rw = random(1, widthLimit + 1), rh = random(1, heightLimit + 1);
    int x = random(W - rw + 1), y = random(H - rh + 1);
    uint8_t col = (uint8_t)(c + random(20));
    display.fillRect(x, y, rw, rh, col);
    display.fillRect(W - x - rw, H - y - rh, rw, rh, col);
    display.fillRect(W - x - rw, y, rw, rh, col);
    display.fillRect(x, H - y - rh, rw, rh, col);
  }
}

static void col_expandingCircles() {
  display.fillScreen(0);
  int xOffset = col_cyclicalPotMap(0, 3, 0, 0, HALFW);
  int yOffset = col_cyclicalPotMap(1, 3, 0, 0, HALFH);
  int period0 = potMap(2, 10, 100) * 100;
  int period1 = potMap(3, 10, 100) * 100;
  int rMax0 = col_cyclicalPotMap(0, 4, 0, 40, HALFW);
  int rMax1 = col_cyclicalPotMap(1, 4, 0.5f, 40, HALFW);
  int c = (int)(col_normalizedTime(period1, 0) * 255);
  if (c < 1) c = 1;
  int r0 = (int)(col_normalizedTime(period0, 0) * rMax0) + 1;
  int r1 = (int)(col_normalizedTime(period0, 0.5f) * rMax1) + 1;
  display.drawCircle(HALFW + xOffset, HALFH - yOffset, r0, c);
  display.drawCircle(HALFW - xOffset, HALFH + yOffset, r0, c);
  display.drawCircle(HALFW + xOffset, HALFH + yOffset, r1, c);
  display.drawCircle(HALFW - xOffset, HALFH - yOffset, r1, c);
}

static void col_reflectedLines() {
  display.fillScreen(0);
  int copies = potMap(0, 1, 20);
  int cIncrement = potMap(1, 0, 255);
  int period = potMap(2, 5, 50) * 100;
  int wiggleRange = potMap(3, 0, 100);
  int wiggle = (int)(col_normalizedTime(5000, 0) * wiggleRange);
  randomSeed(millis() / period);
  for (int i = 0; i < copies; i++) {
    int x0 = random(0, HALFW) - wiggle, y0 = HALFH - 1;
    int x1 = HALFW - 1, y1 = random(0, HALFH) - wiggle;
    int x2 = HALFW - 1, y2 = HALFH - 1;
    int x3 = random(0, HALFW) - wiggle, y3 = random(0, HALFH) - wiggle;
    int c = (255 + (i * cIncrement)) % 256;
    if (c < 1) c = 1;
    display.drawLine(x0, y0, x1, y1, c);
    display.drawLine(x2, y2, x3, y3, c);
    display.drawLine(W - x0, y0, W - x1, y1, c);
    display.drawLine(W - x2, y2, W - x3, y3, c);
    display.drawLine(x0, H - y0, x1, H - y1, c);
    display.drawLine(x2, H - y2, x3, H - y3, c);
    display.drawLine(W - x0, H - y0, W - x1, H - y1, c);
    display.drawLine(W - x2, H - y2, W - x3, H - y3, c);
  }
}

static void col_orbitingCircles() {
  display.fillScreen(0);
  int numCircles = potMap(0, 1, 16);
  float incAngle = 1.0f / numCircles;
  int orbitR = potMap(1, 4, 100);
  int circleR = potMap(2, 4, 100);
  int period = potMap(3, 1, 20) * 500;
  float rotOffset = col_normalizedTime(period, 0);
  for (int i = 0; i < numCircles; i++) {
    int cx = HALFW + (int)(orbitR * cosf(((i * incAngle) + rotOffset) * TWO_PI));
    int cy = HALFH + (int)(orbitR * sinf(((i * incAngle) + rotOffset) * TWO_PI));
    display.drawCircle(cx, cy, circleR, 255);
  }
}

static void col_twistingLines() {
  display.fillScreen(0);
  int numLines = potMap(0, 1, 8) * 2 + 1;
  float angle = TWO_PI / numLines;
  int r0 = potMap(1, 4, 200), r1 = potMap(2, 4, 200);
  float phaseOffset = col_cyclicalPotMap(3, 2, 0, 0, 360) / 360.0f * TWO_PI;
  float rotOffset = col_normalizedTime(5000, 0) * TWO_PI;
  for (int i = 0; i < numLines; i++) {
    int x0 = HALFW + (int)(r0 * cosf((i * angle) + rotOffset));
    int y0 = HALFH + (int)(r0 * sinf((i * angle) + rotOffset));
    int x1 = HALFW + (int)(r1 * cosf((i * angle) + rotOffset + phaseOffset));
    int y1 = HALFH + (int)(r1 * sinf((i * angle) + rotOffset + phaseOffset));
    display.drawLine(x0, y0, x1, y1, 255);
  }
}

static void col_multiPolygon() {
  display.fillScreen(0);
  int numShapes = potMap(0, 1, 16);
  float angle = TWO_PI / numShapes;
  int r0 = potMap(1, 4, 200), r1 = potMap(2, 4, 200);
  int sides = potMap(3, 3, 9);
  float rotOffset = col_normalizedTime(5000, 0) * TWO_PI;
  for (int i = 0; i < numShapes; i++) {
    float theta = i * angle;
    int cx = HALFW + (int)(r0 * cosf(theta + rotOffset));
    int cy = HALFH + (int)(r0 * sinf(theta + rotOffset));
    col_drawPolygon(cx, cy, r1, sides, (-rotOffset - theta) / TWO_PI, 255);
  }
}

static void col_bloom() {
  display.fillScreen(0);
  int petals = potMap(0, 2, 16);
  int levels = potMap(1, 1, 16);
  int rBase = potMap(2, 2, 24);
  float rGrowth = col_cyclicalPotMap(3, 2, 0, 1, 100) / 10.0f;
  float gap = col_sineTime(4000, 0) * col_cyclicalPotMap(0, 3, 0, 1, 80);
  float spiral = col_sineTime(4500, 0) * (col_cyclicalPotMap(1, 3, 0, 1, 100) / 100.0f);
  float offset = col_sineTime(5700, 0) * (col_cyclicalPotMap(2, 3, 0, 1, 100) / 100.0f);
  int cPeriod = potMap(3, 30, 300) * 100;
  int color = (int)(col_sineTime(cPeriod, 0) * 127.5f + 127.5f) % 256;
  if (color < 1) color = 1;
  for (int i = 0; i < petals; i++) {
    for (int j = 1; j <= levels; j++) {
      int cx = HALFW + (int)(sinf((i / (float)petals) * TWO_PI + (j * spiral + offset) * TWO_PI) * (j * gap));
      int cy = HALFH + (int)(cosf((i / (float)petals) * TWO_PI + (j * spiral + offset) * TWO_PI) * (j * gap));
      int cr = (int)(rBase + (j * rGrowth));
      if (cr < 1) cr = 1;
      display.fillCircle(cx, cy, cr, color);
    }
  }
}

// ─── Public interface ────────────────────────────────────────────────

const char* prog_color_name() { return "COLOR LAB"; }
const char* prog_color_character() {
  return "Color palette lab -- own palette, no global color/speed";
}

static const char* const col_presetNames[] = {
  "Palette Test", "Polygon Grid", "Wobbly Lines", "Gradient Grid",
  "Basic Reflect", "Reflected Rects", "Expanding Circles", "Reflected Lines",
  "Orbiting Circles", "Twisting Lines", "Multi Polygon", "Bloom"
};

const char* prog_color_presetName(int preset) {
  if (preset >= 0 && preset < 12) return col_presetNames[preset];
  return NULL;
}

static const char* const col_potLabels[12][12] = {
  {"", "", "", "", "PalMode", "R/G/B...", "", "", "", "", "", ""},
  {"Radius", "Sides", "Rot", "Spacing", "PalMode", "Pal", "", "", "", "", "", ""},
  {"Copies", "Phase", "Period", "Color", "PalMode", "Pal", "", "", "", "", "", ""},
  {"Rows", "Cols", "I Gap", "J Gap", "PalMode", "Pal", "", "", "", "", "", ""},
  {"X Range", "Y Range", "Radius", "Mode", "PalMode", "Pal", "", "", "", "", "", ""},
  {"W Limit", "H Limit", "Qty", "Color", "PalMode", "Pal", "", "", "", "", "", ""},
  {"X Off", "Y Off", "Per 0", "Per 1", "PalMode", "Pal", "", "", "", "", "", ""},
  {"Copies", "C Incr", "Period", "Wiggle", "PalMode", "Pal", "", "", "", "", "", ""},
  {"Count", "Orbit R", "Circle R", "Period", "PalMode", "Pal", "", "", "", "", "", ""},
  {"Lines", "R0", "R1", "Phase", "PalMode", "Pal", "", "", "", "", "", ""},
  {"Shapes", "R0", "R1", "Sides", "PalMode", "Pal", "", "", "", "", "", ""},
  {"Petals", "Levels", "R Base", "Growth", "PalMode", "Pal", "", "", "", "", "", ""}
};

const char* prog_color_potLabel(int preset, int pot) {
  if (pot < 4 || pot > 15) return "";
  if (preset < 0 || preset >= 12) return "";
  if (pot == 4) return "PalMode";
  const char* label = col_potLabels[preset][pot - 4];
  return (label && label[0]) ? label : "--";
}

uint8_t prog_color_renderHint(int preset) {
  (void)preset;
  return RENDER_CLEAR;
}

void prog_color_draw(int preset) {
  int palMode = potMap(4, 0, 3);
  col_applyPalette(palMode);

  switch (preset) {
    case 0:  col_paletteTest();   break;
    case 1:  col_polygonGrid();   break;
    case 2:  col_wobblyLines(); break;
    case 3:  col_gradientGrid(); break;
    case 4:  col_basicReflections(); break;
    case 5:  col_reflectedRects(); break;
    case 6:  col_expandingCircles(); break;
    case 7:  col_reflectedLines(); break;
    case 8:  col_orbitingCircles(); break;
    case 9:  col_twistingLines(); break;
    case 10: col_multiPolygon(); break;
    case 11: col_bloom(); break;
    default: display.fillScreen(0); break;
  }
}
