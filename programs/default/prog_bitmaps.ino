// =====================================================================
// PROGRAM: BITMAPS (Video_Gem_Bitmaps by RMNA)
//
// Bitmap and text modes; per-preset parameters use pots p4–p7.
// =====================================================================

#include "bitmaps.h"

// Local indices 0..3 map to p4..p7 (p0–p3 are globals unless PROG_FLAG_OWNS_GLOBALS).
static inline int bm_pot(int localIdx, int lo, int hi) {
  return potMap(4 + localIdx, lo, hi);
}

static float bm_sineTime(int periodMS, float phase) {
  return sinf(normalizedTime(periodMS, phase) * TWO_PI);
}

static void bm_drawScaledBitmap(int x, int y,
                                 const unsigned char* bitmap,
                                 int w, int h,
                                 int scale,
                                 uint8_t color) {
  // Bit access matches the image2cpp output used by bitmaps.h
  for (int row = 0; row < h; row++) {
    for (int col = 0; col < w; col++) {
      int bitIndex = row * w + col;
      uint8_t byte = pgm_read_byte(&bitmap[bitIndex / 8]);
      bool lit = (byte >> (7 - (bitIndex % 8))) & 1;
      if (lit) display.fillRect(x + col * scale, y + row * scale, scale, scale, color);
    }
  }
}

static void bm_logoSmall() {
  display.fillScreen(0);
  display.drawBitmap(144, 108, epd_bitmap_Video_gem_logo_small, 32, 24, 255);
}

static void bm_logoBig() {
  display.fillScreen(0);
  const int scale = 3;
  const int w = 32, h = 24;
  int x = HALFW - (w * scale) / 2;
  int y = HALFH - (h * scale) / 2;
  bm_drawScaledBitmap(x, y, epd_bitmap_Video_gem_logo_small, w, h, scale, 255);
}

static void bm_randomSparks() {
  display.fillScreen(0);
  int bitmap = bm_pot(0, 0, 5);
  int xRange = bm_pot(1, 1, W);
  int yRange = bm_pot(2, 1, H);
  uint8_t c = (uint8_t)bm_pot(3, 1, 254);

  for (int i = 0; i < 10; i++) {
    int x = HALFW + random(xRange) - (xRange / 2);
    int y = HALFH + random(yRange) - (yRange / 2);
    display.drawBitmap(x, y, spark_bitmap_array[bitmap], 17, 17, c);
  }
}

static void bm_animatedSpark() {
  display.fillScreen(0);
  // Phase increments from pots[4..7]
  float xPhaseIncrement = bm_pot(0, 0, 21) / 20.0f;
  float yPhaseIncrement = bm_pot(1, 0, 21) / 20.0f;
  int xColorIncrement = bm_pot(2, 0, 256);
  int yColorIncrement = bm_pot(3, 0, 256);

  for (int i = 0; i < 16; i++) {
    for (int j = 0; j < 12; j++) {
      int x = i * 20;
      int y = j * 20;
      float phaseOffset = i * xPhaseIncrement + j * yPhaseIncrement;
      int bitmapIndex = (int)(pingPongTime(1000, phaseOffset) * (spark_bitmap_array_LEN - 1));
      int c = (i * xColorIncrement + j * yColorIncrement) % 255;
      if (c < 1) c = 1;
      display.drawBitmap(x, y, spark_bitmap_array[bitmapIndex], 17, 17, (uint8_t)c);
    }
  }
}

static void bm_sparkBrush() {
  display.fillScreen(0);
  int xPeriod = bm_pot(0, 100, 4000);
  int yPeriod = bm_pot(1, 100, 4000);
  int bitmap = bm_pot(2, 0, 5);
  uint8_t c = (uint8_t)bm_pot(3, 1, 254);

  int x = (int)(bm_sineTime(xPeriod, 0.5f) * HALFW) + HALFW;
  int y = (int)(bm_sineTime(yPeriod, 0.0f) * HALFH) + HALFH;
  display.drawBitmap(x - 8, y - 8, spark_bitmap_array[bitmap], 17, 17, c);
}

static void bm_simpleCharacters() {
  display.fillScreen(0);
  display.drawChar(100, 100, 'R', 128, 0, 1);
  display.drawChar(150, 120, 'M', 170, 0, 2);
  display.drawChar(200, 180, 'N', 212, 0, 3);
  display.drawChar(250, 40,  'A', 255, 0, 4);
}

static void bm_randomGlyphs() {
  display.fillScreen(0);
  int glyph = bm_pot(0, 1, 255);
  int num = bm_pot(1, 1, 50);
  int size = bm_pot(2, 1, 12);
  uint8_t color = (uint8_t)bm_pot(3, 1, 254);

  for (int i = 0; i < num; i++) {
    int x = random(W) / (6 * size) * (6 * size);
    int y = random(H) / (8 * size) * (8 * size);
    display.drawChar(x, y, (unsigned char)glyph, color, 0, size);
  }
}

static void bm_simpleWords() {
  display.fillScreen(0);
  int x = bm_pot(0, 0, W);
  int y = bm_pot(1, 0, H);
  int sizeMultiple = bm_pot(2, 1, 5);
  uint8_t c = (uint8_t)bm_pot(3, 1, 254);

  display.setCursor(x, y);
  display.setTextColor(c);
  display.setTextSize(sizeMultiple);
  display.print("RMNA");
}

static void bm_movingWords() {
  display.fillScreen(0);
  float t = globalTime;

  int ampX = bm_pot(0, 10, 140);
  int ampY = bm_pot(1, 10, 90);
  int sizeMultiple = bm_pot(2, 1, 5);
  uint8_t c = (uint8_t)bm_pot(3, 1, 254);

  int x = HALFW + (int)(sinf(t * 2.0f) * ampX);
  int y = HALFH + (int)(cosf(t * 1.6f) * ampY);

  display.setCursor(x, y);
  display.setTextColor(c);
  display.setTextSize(sizeMultiple);
  display.print("RMNA");
}

static void bm_simpleSentence() {
  display.fillScreen(0);
  int x = bm_pot(0, 0, W);
  int y = bm_pot(1, 0, H);
  int sizeMultiple = bm_pot(2, 1, 3);
  uint8_t c = (uint8_t)bm_pot(3, 1, 254);

  display.setCursor(x, y);
  display.setTextColor(c);
  display.setTextSize(sizeMultiple);
  display.print("Made with love by RMNA!");
}

static void bm_pixelGrid() {
  display.fillScreen(0);
  int cols = bm_pot(0, 5, 40);
  int rows = bm_pot(1, 3, 20);

  if (cols < 1) cols = 1;
  if (rows < 1) rows = 1;

  int cellW = W / cols;
  int cellH = H / rows;
  if (cellW < 1) cellW = 1;
  if (cellH < 1) cellH = 1;

  int c = bm_pot(2, 0, 255);
  (void)c;

  for (int r = 0; r < rows; r++) {
    for (int col = 0; col < cols; col++) {
      int x = col * cellW;
      int y = r * cellH;
      uint8_t color = (uint8_t)((r * cols + col) * 255 / (rows * cols));
      display.drawPixel(x, y, color);
    }
  }
}

static void bm_bitmapOrbit() {
  display.fillScreen(0);
  int count = bm_pot(0, 3, 20);
  if (count < 1) count = 1;
  int orbitR = bm_pot(1, 10, 100);
  int color = bm_pot(2, 1, 254);
  int speed = bm_pot(3, 1, 30);

  for (int i = 0; i < count; i++) {
    float a = (i / (float)count) * TWO_PI + globalTime * (speed * 0.07f);
    int x = HALFW + (int)(cosf(a) * orbitR);
    int y = HALFH + (int)(sinf(a) * orbitR);
    int bitmapIndex = i % spark_bitmap_array_LEN;
    display.drawBitmap(x - 8, y - 8, spark_bitmap_array[bitmapIndex], 17, 17, (uint8_t)color);
  }
}

// ─── Public interface ────────────────────────────────────────────────

const char* prog_bitmaps_name() { return "BITMAPS"; }

const char* prog_bitmaps_character() {
  return "Bitmaps & text — Video_Gem_Bitmaps by RMNA";
}

static const char* const bm_presetNames[] = {
  "Logo Small", "Logo Big", "Random Sparks", "Animated Spark Grid",
  "Spark Brush", "Simple Chars", "Random Glyphs", "Simple Words",
  "Moving Words", "Simple Sentence", "Pixel Grid", "Bitmap Orbit"
};

const char* prog_bitmaps_presetName(int preset) {
  if (preset >= 0 && preset < 12) return bm_presetNames[preset];
  return NULL;
}

const char* prog_bitmaps_potLabel(int preset, int pot) {
  if (pot == 4) return "BMP";
  if (pot == 5) return "ParamA";
  if (pot == 6) return "ParamB";
  if (pot == 7) return "Clr";
  return "";
}

uint8_t prog_bitmaps_renderHint(int preset) {
  (void)preset;
  return RENDER_CLEAR;
}

void prog_bitmaps_draw(int preset) {
  switch (preset) {
    case 0:  bm_logoSmall();         break;
    case 1:  bm_logoBig();           break;
    case 2:  bm_randomSparks();      break;
    case 3:  bm_animatedSpark();     break;
    case 4:  bm_sparkBrush();        break;
    case 5:  bm_simpleCharacters(); break;
    case 6:  bm_randomGlyphs();      break;
    case 7:  bm_simpleWords();       break;
    case 8:  bm_movingWords();       break;
    case 9:  bm_simpleSentence();    break;
    case 10: bm_pixelGrid();         break;
    case 11: bm_bitmapOrbit();       break;
    default: display.fillScreen(0);  break;
  }
}

