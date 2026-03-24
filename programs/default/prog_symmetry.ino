// =====================================================================
// PROGRAM 1: SYMMETRY (Video_Gem_Symmetry by RMNA)
// 12 presets: translation, reflection, rotation symmetry patterns
// Uses p4–p7 per mode; p8–p15 vary by preset.
// =====================================================================

static float sym_normalizedTime(int periodMS, float phase) {
  return ((millis() + (int)(phase * periodMS)) % periodMS) / (float)periodMS;
}

static float sym_sineTime(int periodMS, float phase) {
  return sinf(sym_normalizedTime(periodMS, phase) * TWO_PI);
}

static int sym_cyclicalPotMap(int potIdx, float multiple, float phase, int lo, int hi) {
  int diff = hi - lo;
  float cycle = sinf((pots[potIdx] / 1023.0f) * multiple * TWO_PI + (phase * TWO_PI)) * 0.5f + 0.5f;
  return lo + (int)(diff * cycle);
}

static void sym_drawPolygon(int cx, int cy, int r, int sides, float rotation01, uint8_t c) {
  drawPoly(cx, cy, r, sides, rotation01 * TWO_PI, c);
}

// ─── Preset 0: Circle Translation ────────────────────────────────────
static void sym_circleTranslation() {
  display.fillScreen(0);
  int copies = potMap(4, 1, 33);
  int gap = potMap(5, 1, 11);
  int rIncrement = potMap(6, 0, 50);
  int cIncrement = potMap(7, 0, 255);

  int rBase = 10, rMax = 200, cBase = 128;
  for (int i = 0; i < copies; i++) {
    int x = i * gap;
    int r = (rBase + (i * rIncrement)) % rMax;
    int c = (cBase + (i * cIncrement)) % 256;
    if (c < 1) c = 1;
    display.drawCircle(x, HALFH, r, c);
  }
}

// ─── Preset 1: Polygon Grid ──────────────────────────────────────────
static void sym_polygonGrid() {
  display.fillScreen(0);
  int r = potMap(4, 1, 150);
  int sides = potMap(5, 3, 20);
  float rotation = (float)potMap(6, 0, 359) / 360.0f;
  int rowSpacing = potMap(7, 40, HALFH);
  int colSpacing = potMap(8, 40, HALFW);
  int rows = H / rowSpacing;
  int cols = W / colSpacing;

  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      int x = j * colSpacing, y = i * rowSpacing;
      sym_drawPolygon(HALFW + x, HALFH + y, r, sides, rotation, 255);
      if (i > 0) sym_drawPolygon(HALFW + x, HALFH - y, r, sides, rotation, 255);
      if (j > 0) sym_drawPolygon(HALFW - x, HALFH + y, r, sides, rotation, 255);
      if (i > 0 && j > 0) sym_drawPolygon(HALFW - x, HALFH - y, r, sides, rotation, 255);
    }
  }
}

// ─── Preset 2: Wobbly Lines ──────────────────────────────────────────
static void sym_wobblyLines() {
  display.fillScreen(0);
  int copies = potMap(4, 1, 7);
  float phaseOffset = potMap(5, 0, 360) / 360.0f;
  int period = potMap(6, 50, 500) * 10;
  int c = potMap(7, 1, 255);

  int xRange = 15, xWidth = 30;
  int x0 = HALFW + (int)(sinf(sym_normalizedTime(period, 0) * TWO_PI) * xRange);
  int x1 = HALFW + (int)(sinf(sym_normalizedTime(period, phaseOffset) * TWO_PI) * xRange);
  display.drawLine(x0, 0, x1, H, c);

  for (int i = 1; i < copies; i++) {
    x0 = HALFW + (i * xWidth) + (int)(sinf(sym_normalizedTime(period, i * phaseOffset) * TWO_PI) * xRange);
    x1 = HALFW + (i * xWidth) + (int)(sinf(sym_normalizedTime(period, (i + 1) * phaseOffset) * TWO_PI) * xRange);
    display.drawLine(x0, 0, x1, H, c);
    x0 = HALFW - (i * xWidth) + (int)(sinf(sym_normalizedTime(period, i * phaseOffset) * TWO_PI) * xRange);
    x1 = HALFW - (i * xWidth) + (int)(sinf(sym_normalizedTime(period, (i + 1) * phaseOffset) * TWO_PI) * xRange);
    display.drawLine(x0, 0, x1, H, c);
  }
}

// ─── Preset 3: Gradient Grid ─────────────────────────────────────────
static void sym_gradientGrid() {
  int numRows = potMap(4, 1, 64);
  int numCols = potMap(5, 1, 64);
  int rw = (W + numCols - 1) / numCols;
  int rh = (H + numRows - 1) / numRows;
  int iGap = potMap(6, 0, 255);
  int jGap = potMap(7, 0, 255);
  int firstC = (millis() / 10) % 254 + 1;

  for (int i = 0; i < numCols; i++) {
    for (int j = 0; j < numRows; j++) {
      int c = (firstC + (i * iGap) + (j * jGap)) % 254 + 1;
      display.fillRect(i * rw, j * rh, rw, rh, c);
    }
  }
}

// ─── Preset 4: Basic Reflections ─────────────────────────────────────
static void sym_basicReflections() {
  display.fillScreen(0);
  int xBase = 80, yBase = 60;
  int xRange = potMap(4, 0, 80);
  int yRange = potMap(5, 0, 60);
  int r = potMap(6, 1, 100);
  int mode = potMap(7, 0, 5);

  float t = sym_normalizedTime(2000, 0);
  int cx = xBase + (int)(sinf(t * TWO_PI) * xRange);
  int cy = yBase + (int)(cosf(t * TWO_PI) * yRange);

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

// ─── Preset 5: Reflected Rects ───────────────────────────────────────
static void sym_reflectedRects() {
  display.fillScreen(0);
  randomSeed(millis() / 1000);
  int widthLimit = potMap(4, 1, HALFW / 2);
  int heightLimit = potMap(5, 1, H / 2);
  int quantity = potMap(6, 1, 10);
  int c = potMap(7, 0, 235);

  for (int i = 0; i < quantity; i++) {
    int rw = random(1, widthLimit + 1);
    int rh = random(1, heightLimit + 1);
    int x = random(W - rw + 1);
    int y = random(H - rh + 1);
    uint8_t col = (uint8_t)(c + random(20));
    display.fillRect(x, y, rw, rh, col);
    display.fillRect(W - x - rw, H - y - rh, rw, rh, col);
    display.fillRect(W - x - rw, y, rw, rh, col);
    display.fillRect(x, H - y - rh, rw, rh, col);
  }
}

// ─── Preset 6: Expanding Circles ─────────────────────────────────────
static void sym_expandingCircles() {
  display.fillScreen(0);
  int xOffset = sym_cyclicalPotMap(4, 3, 0, 0, HALFW);
  int yOffset = sym_cyclicalPotMap(5, 3, 0, 0, HALFH);
  int period0 = potMap(6, 10, 100) * 100;
  int period1 = potMap(7, 10, 100) * 100;
  int rMax0 = sym_cyclicalPotMap(4, 4, 0, 40, HALFW);
  int rMax1 = sym_cyclicalPotMap(5, 4, 0.5f, 40, HALFW);

  int c = (int)(sym_normalizedTime(period1, 0) * 255);
  if (c < 1) c = 1;
  int r0 = (int)(sym_normalizedTime(period0, 0) * rMax0) + 1;
  int r1 = (int)(sym_normalizedTime(period0, 0.5f) * rMax1) + 1;

  display.drawCircle(HALFW + xOffset, HALFH - yOffset, r0, c);
  display.drawCircle(HALFW - xOffset, HALFH + yOffset, r0, c);
  display.drawCircle(HALFW + xOffset, HALFH + yOffset, r1, c);
  display.drawCircle(HALFW - xOffset, HALFH - yOffset, r1, c);
}

// ─── Preset 7: Reflected Lines ───────────────────────────────────────
static void sym_reflectedLines() {
  display.fillScreen(0);
  int copies = potMap(4, 1, 20);
  int cIncrement = potMap(5, 0, 255);
  int period = potMap(6, 5, 50) * 100;
  int wiggleRange = potMap(7, 0, 100);
  int wiggle = (int)(sym_normalizedTime(5000, 0) * wiggleRange);

  randomSeed(millis() / period);
  for (int i = 0; i < copies; i++) {
    int x0 = random(0, HALFW) - wiggle;
    int y0 = HALFH - 1;
    int x1 = HALFW - 1;
    int y1 = random(0, HALFH) - wiggle;
    int x2 = HALFW - 1, y2 = HALFH - 1;
    int x3 = random(0, HALFW) - wiggle;
    int y3 = random(0, HALFH) - wiggle;
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

// ─── Preset 8: Orbiting Circles ──────────────────────────────────────
static void sym_orbitingCircles() {
  display.fillScreen(0);
  int numCircles = potMap(4, 1, 16);
  float incAngle = 1.0f / numCircles;
  int orbitR = potMap(5, 4, 100);
  int circleR = potMap(6, 4, 100);
  int period = potMap(7, 1, 20) * 500;
  float rotOffset = sym_normalizedTime(period, 0);

  for (int i = 0; i < numCircles; i++) {
    int cx = HALFW + (int)(orbitR * cosf(((i * incAngle) + rotOffset) * TWO_PI));
    int cy = HALFH + (int)(orbitR * sinf(((i * incAngle) + rotOffset) * TWO_PI));
    display.drawCircle(cx, cy, circleR, 255);
  }
}

// ─── Preset 9: Twisting Lines ────────────────────────────────────────
static void sym_twistingLines() {
  display.fillScreen(0);
  int numLines = potMap(4, 1, 8) * 2 + 1;
  float angle = TWO_PI / numLines;
  int r0 = potMap(5, 4, 200);
  int r1 = potMap(6, 4, 200);
  float phaseOffset = sym_cyclicalPotMap(7, 2, 0, 0, 360) / 360.0f * TWO_PI;
  float rotOffset = sym_normalizedTime(5000, 0) * TWO_PI;

  for (int i = 0; i < numLines; i++) {
    int x0 = HALFW + (int)(r0 * cosf((i * angle) + rotOffset));
    int y0 = HALFH + (int)(r0 * sinf((i * angle) + rotOffset));
    int x1 = HALFW + (int)(r1 * cosf((i * angle) + rotOffset + phaseOffset));
    int y1 = HALFH + (int)(r1 * sinf((i * angle) + rotOffset + phaseOffset));
    display.drawLine(x0, y0, x1, y1, 255);
  }
}

// ─── Preset 10: Multi Polygon ────────────────────────────────────────
static void sym_multiPolygon() {
  display.fillScreen(0);
  int numShapes = potMap(4, 1, 16);
  float angle = TWO_PI / numShapes;
  int r0 = potMap(5, 4, 200);
  int r1 = potMap(6, 4, 200);
  int sides = potMap(7, 3, 9);
  float rotOffset = sym_normalizedTime(5000, 0) * TWO_PI;

  for (int i = 0; i < numShapes; i++) {
    float theta = i * angle;
    int cx = HALFW + (int)(r0 * cosf(theta + rotOffset));
    int cy = HALFH + (int)(r0 * sinf(theta + rotOffset));
    sym_drawPolygon(cx, cy, r1, sides, (-rotOffset - theta) / TWO_PI, 255);
  }
}

// ─── Preset 11: Bloom ────────────────────────────────────────────────
static void sym_bloom() {
  display.fillScreen(0);
  int petals = potMap(4, 2, 16);
  int levels = potMap(5, 1, 16);
  int rBase = potMap(6, 2, 24);
  float rGrowth = sym_cyclicalPotMap(7, 2, 0, 1, 100) / 10.0f;
  float gap = sym_sineTime(4000, 0) * sym_cyclicalPotMap(4, 3, 0, 1, 80);
  float spiral = sym_sineTime(4500, 0) * (sym_cyclicalPotMap(5, 3, 0, 1, 100) / 100.0f);
  float offset = sym_sineTime(5700, 0) * (sym_cyclicalPotMap(6, 3, 0, 1, 100) / 100.0f);
  int cPeriod = potMap(7, 30, 300) * 100;
  int color = (int)(sym_sineTime(cPeriod, 0) * 127.5f + 127.5f) % 256;
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

const char* prog_symmetry_name() { return "SYMMETRY"; }
const char* prog_symmetry_character() {
  return "Translation, reflection, rotation -- RMNA symmetry workshop";
}

static const char* const sym_presetNames[] = {
  "Circle Trans", "Polygon Grid", "Wobbly Lines", "Gradient Grid",
  "Basic Reflect", "Reflected Rects", "Expanding Circles", "Reflected Lines",
  "Orbiting Circles", "Twisting Lines", "Multi Polygon", "Bloom"
};

const char* prog_symmetry_presetName(int preset) {
  if (preset >= 0 && preset < 12) return sym_presetNames[preset];
  return NULL;
}

static const char* const sym_potLabels[12][12] = {
  {"Copies", "Gap", "R Incr", "C Incr", "", "", "", "", "", "", "", ""},
  {"Radius", "Sides", "Rot", "RowSp", "ColSp", "", "", "", "", "", "", ""},
  {"Copies", "Phase", "Period", "Color", "", "", "", "", "", "", "", ""},
  {"Rows", "Cols", "I Gap", "J Gap", "", "", "", "", "", "", "", ""},
  {"X Range", "Y Range", "Radius", "Mode", "", "", "", "", "", "", "", ""},
  {"W Limit", "H Limit", "Quantity", "Color", "", "", "", "", "", "", "", ""},
  {"X Off", "Y Off", "Per 0", "Per 1", "", "", "", "", "", "", "", ""},
  {"Copies", "C Incr", "Period", "Wiggle", "", "", "", "", "", "", "", ""},
  {"Count", "Orbit R", "Circle R", "Period", "", "", "", "", "", "", "", ""},
  {"Lines", "R0", "R1", "Phase", "", "", "", "", "", "", "", ""},
  {"Shapes", "R0", "R1", "Sides", "", "", "", "", "", "", "", ""},
  {"Petals", "Levels", "R Base", "Growth", "", "", "", "", "", "", "", ""}
};

const char* prog_symmetry_potLabel(int preset, int pot) {
  if (pot < 4 || pot > 15) return "";
  if (preset < 0 || preset >= 12) return "";
  const char* label = sym_potLabels[preset][pot - 4];
  return (label && label[0]) ? label : "--";
}

uint8_t prog_symmetry_renderHint(int preset) {
  (void)preset;
  return RENDER_CLEAR;
}

void prog_symmetry_draw(int preset) {
  switch (preset) {
    case 0:  sym_circleTranslation(); break;
    case 1:  sym_polygonGrid(); break;
    case 2:  sym_wobblyLines(); break;
    case 3:  sym_gradientGrid(); break;
    case 4:  sym_basicReflections(); break;
    case 5:  sym_reflectedRects(); break;
    case 6:  sym_expandingCircles(); break;
    case 7:  sym_reflectedLines(); break;
    case 8:  sym_orbitingCircles(); break;
    case 9:  sym_twistingLines(); break;
    case 10: sym_multiPolygon(); break;
    case 11: sym_bloom(); break;
    default: display.fillScreen(0); break;
  }
}
