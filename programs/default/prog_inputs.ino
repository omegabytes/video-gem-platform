// =====================================================================
// PROGRAM: INPUTS (Video_Gem_Inputs by RMNA)
// 12 presets: pot- and time-driven patterns from Gray Area workshop.
// Demonstrates mapping pots[] to visuals. Original by Ramona Sharples (RMNA).
// =====================================================================

// Map legacy p0-p3 style to per-preset p4-p15
static int inp_pot(int idx, int lo, int hi) {
  return potMap(4 + idx, lo, hi);
}

// ─── Patterns (from Video_Gem_Inputs layers) ─────────────────────────

static void inp_doodle() {
  display.fillScreen(0);
  int x = inp_pot(0, 0, W);
  int y = inp_pot(1, 0, H);
  int r = inp_pot(2, 1, 40);
  int c = inp_pot(3, 1, 254);
  display.fillCircle(x, y, r, (uint8_t)c);
}

static void inp_stretchyRectangles() {
  display.fillScreen(0);
  int rw0 = inp_pot(0, 0, W);
  int rw1 = W - rw0;
  int rh0 = inp_pot(1, 0, H);
  int rh1 = H - rh0;
  int c0 = inp_pot(2, 1, 254);
  int c1 = inp_pot(3, 1, 254);
  display.fillRect(HALFW - rw0 / 2, HALFH - rh0 / 2, rw0, rh0, (uint8_t)c0);
  display.fillRect(HALFW - rw1 / 2, HALFH - rh1 / 2, rw1, rh1, (uint8_t)c1);
}

static void inp_regularShapes() {
  display.fillScreen(0);
  int r = inp_pot(0, 1, 120);
  int sides = inp_pot(1, 3, 12);
  float rotation = (float)inp_pot(2, 0, 359) / 360.0f;
  int spacing = inp_pot(3, 50, HALFW);
  int rows = H / spacing, cols = W / spacing;
  int c = inp_pot(4, 60, 254);
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      int cx = HALFW + (j - cols / 2) * spacing;
      int cy = HALFH + (i - rows / 2) * spacing;
      drawPoly(cx, cy, r, sides, rotation * TWO_PI, (uint8_t)c);
    }
  }
}

static void inp_star() {
  display.fillScreen(0);
  int r1 = inp_pot(0, 10, 100);
  int r2 = inp_pot(1, 10, 100);
  int pts = inp_pot(2, 3, 16);
  float rotation = (float)inp_pot(3, 0, 359) / 360.0f;
  int c = inp_pot(4, 60, 254);
  drawStarShape(HALFW, HALFH, r1, r2, pts, rotation * TWO_PI, (uint8_t)c);
}

static void inp_fourTriangles() {
  display.fillScreen(0);
  int cx = HALFW, cy = HALFH, size = 80;
  display.fillTriangle(cx, cy, cx - size, cy, cx, cy - size, (uint8_t)inp_pot(0, 1, 254));
  display.fillTriangle(cx, cy, cx + size, cy, cx, cy - size, (uint8_t)inp_pot(1, 1, 254));
  display.fillTriangle(cx, cy, cx + size, cy, cx, cy + size, (uint8_t)inp_pot(2, 1, 254));
  display.fillTriangle(cx, cy, cx - size, cy, cx, cy + size, (uint8_t)inp_pot(3, 1, 254));
}

static void inp_slidingCircles() {
  display.fillScreen(0);
  int r0 = inp_pot(0, 5, 35);
  int r1 = inp_pot(1, 5, 35);
  int r2 = inp_pot(2, 5, 35);
  int c = inp_pot(3, 1, 254);
  int speedA = 500 + inp_pot(4, 0, 2000);
  int speedB = 500 + inp_pot(5, 0, 2000);
  int speedC = 500 + inp_pot(6, 0, 2000);
  float t0 = normalizedTime(speedA, 0);
  float t1 = normalizedTime(speedB, 0.33f);
  float t2 = normalizedTime(speedC, 0.66f);
  int x0 = (int)(t0 * (W + r0 * 2)) - r0;
  int x1 = (int)(t1 * (W + r1 * 2)) - r1;
  int x2 = (int)(t2 * (W + r2 * 2)) - r2;
  display.drawCircle(x0, 60, r0, (uint8_t)c);
  display.drawCircle(x1, 120, r1, (uint8_t)c);
  display.drawCircle(x2, 180, r2, (uint8_t)c);
}

static void inp_randomCircle() {
  display.fillScreen(0);
  int periodMS = 500 + inp_pot(0, 0, 3000);
  randomSeed(millis() / periodMS);
  int x = random(W);
  int y = random(H);
  int r = inp_pot(1, 20, 80);
  int c = inp_pot(2, 1, 254);
  display.fillCircle(x, y, r, (uint8_t)c);
}

static void inp_growingCircle() {
  display.fillScreen(0);
  int periodMS = 1000 + inp_pot(0, 0, 5000);
  float t = normalizedTime(periodMS, 0);
  int r = (int)(t * (H / 2));
  int c = inp_pot(1, 1, 254);
  display.drawCircle(HALFW, HALFH, r, (uint8_t)c);
}

static void inp_lissajous() {
  display.fillScreen(0);
  int a = inp_pot(0, 1, 5);
  int b = inp_pot(1, 1, 5);
  float phase = (float)inp_pot(2, 0, 360) / 360.0f * TWO_PI;
  float t = globalTime;
  int prevX = HALFW + (int)(80 * sinf(a * t));
  int prevY = HALFH + (int)(80 * sinf(b * t + phase));
  int c = inp_pot(3, 60, 254);
  for (int i = 1; i <= 100; i++) {
    float s = t + i * 0.05f;
    int x = HALFW + (int)(80 * sinf(a * s));
    int y = HALFH + (int)(80 * sinf(b * s + phase));
    display.drawLine(prevX, prevY, x, y, (uint8_t)c);
    prevX = x;
    prevY = y;
  }
}

static void inp_crosshairs() {
  display.fillScreen(0);
  int x = inp_pot(0, 0, W);
  int y = inp_pot(1, 0, H);
  int c = inp_pot(2, 1, 254);
  display.drawFastHLine(0, y, W, (uint8_t)c);
  display.drawFastVLine(x, 0, H, (uint8_t)c);
}

static void inp_checkerboard() {
  display.fillScreen(0);
  int cellW = inp_pot(0, 4, 40);
  int cellH = inp_pot(1, 4, 40);
  int c1 = inp_pot(2, 1, 200);
  int c2 = inp_pot(3, 1, 200);
  for (int i = 0; i < H / cellH + 1; i++) {
    for (int j = 0; j < W / cellW + 1; j++) {
      int x = j * cellW, y = i * cellH;
      uint8_t col = ((i + j) & 1) ? (uint8_t)c1 : (uint8_t)c2;
      display.fillRect(x, y, cellW, cellH, col);
    }
  }
}

static void inp_breathingRings() {
  display.fillScreen(0);
  float t = sinf(globalTime * 2) * 0.5f + 0.5f;
  int rBase = inp_pot(0, 20, 80);
  int rMax = inp_pot(1, 60, 150);
  int r = (int)(rBase + t * (rMax - rBase));
  int c = inp_pot(2, 60, 254);
  for (int i = 1; i <= 5; i++) {
    display.drawCircle(HALFW, HALFH, r * i, (uint8_t)c);
  }
}

// ─── Public interface ────────────────────────────────────────────────

const char* prog_inputs_name() { return "INPUTS"; }
const char* prog_inputs_character() {
  return "Pot & time-driven patterns (Video_Gem_Inputs by RMNA)";
}

static const char* const inp_presetNames[] = {
  "Doodle", "Stretchy Rects", "Regular Shapes", "Star", "Four Triangles",
  "Sliding Circles", "Random Circle", "Growing Circle", "Lissajous",
  "Crosshairs", "Checkerboard", "Breathing Rings"
};

const char* prog_inputs_presetName(int preset) {
  if (preset >= 0 && preset < 12) return inp_presetNames[preset];
  return NULL;
}

static const char* const inp_potLabels[12][12] = {
  {"X", "Y", "Radius", "Color", "", "", "", "", "", "", "", ""},
  {"W0", "H0", "Color 0", "Color 1", "", "", "", "", "", "", "", ""},
  {"Radius", "Sides", "Rot", "Spacing", "Color", "", "", "", "", "", "", ""},
  {"R1", "R2", "Points", "Rot", "Color", "", "", "", "", "", "", ""},
  {"C0", "C1", "C2", "C3", "", "", "", "", "", "", "", ""},
  {"R0", "R1", "R2", "Color", "Speed A", "Speed B", "Speed C", "", "", "", "", ""},
  {"Period", "Radius", "Color", "", "", "", "", "", "", "", "", ""},
  {"Period", "Color", "", "", "", "", "", "", "", "", "", ""},
  {"A", "B", "Phase", "Color", "", "", "", "", "", "", "", ""},
  {"X", "Y", "Color", "", "", "", "", "", "", "", "", ""},
  {"CellW", "CellH", "Color1", "Color2", "", "", "", "", "", "", "", ""},
  {"R Base", "R Max", "Color", "", "", "", "", "", "", "", "", ""}
};

const char* prog_inputs_potLabel(int preset, int pot) {
  if (pot < 4 || pot > 15) return "";
  if (preset < 0 || preset >= 12) return "";
  const char* label = inp_potLabels[preset][pot - 4];
  return (label && label[0]) ? label : "--";
}

uint8_t prog_inputs_renderHint(int preset) {
  (void)preset;
  return RENDER_CLEAR;
}

void prog_inputs_draw(int preset) {
  switch (preset) {
    case 0:  inp_doodle();           break;
    case 1:  inp_stretchyRectangles(); break;
    case 2:  inp_regularShapes();    break;
    case 3:  inp_star();             break;
    case 4:  inp_fourTriangles();    break;
    case 5:  inp_slidingCircles();   break;
    case 6:  inp_randomCircle();     break;
    case 7:  inp_growingCircle();    break;
    case 8:  inp_lissajous();        break;
    case 9:  inp_crosshairs();       break;
    case 10: inp_checkerboard();     break;
    case 11: inp_breathingRings();   break;
    default: display.fillScreen(0);  break;
  }
}
