// =====================================================================
// CORE: FX — global effects layer
//
// FX are composable frame-manipulation effects applied by the platform
// pipeline, independent of the active program. Programs are unaware
// of FX. Activated via FX mode (Ctrl+k12 toggle).
//
// Three pipeline insertion points:
//   applyPaletteFX()  — after palette build (step 4)
//   applyPreDrawFX()  — after render hint, before program draw (step 7)
//   applyPostDrawFX() — after program draw, before hint overlay (step 9)
// =====================================================================

// ─── State ──────────────────────────────────────────────────────────
bool     g_fxMode   = false;
uint16_t g_fxActive = 0;     // bitmask: bit N = FX N is active

static const char* const fxNames[FX_COUNT] = {
  "CutFade", "DotFade", "Descend", "Ascend",
  "MirrorH", "MirrorV", "MirrorQuad", "Echo",
  "Invert", "Reverse", "Wipe", "Dropouts"
};

// ─── Toggle / query ─────────────────────────────────────────────────

void fxToggle(int fxId) {
  if (fxId >= 0 && fxId < FX_COUNT) {
    g_fxActive ^= (1 << fxId);
  }
}

void fxClearAll() {
  g_fxActive = 0;
}

bool fxIsActive(int fxId) {
  return (g_fxActive & (1 << fxId)) != 0;
}

int fxActiveCount() {
  int count = 0;
  uint16_t bits = g_fxActive;
  while (bits) { count += (bits & 1); bits >>= 1; }
  return count;
}

const char* fxGetName(int fxId) {
  if (fxId >= 0 && fxId < FX_COUNT) return fxNames[fxId];
  return NULL;
}

// ─── Canvas FX ──────────────────────────────────────────────────────

static void fxCutFade() {
  int num = 40;
  for (int i = 0; i < num; i++) {
    int y = random(H);
    display.drawFastHLine(0, y, W, 0);
  }
}

static void fxDotFade() {
  int num = 300;
  for (int i = 0; i < num; i++) {
    int x = random(W);
    int y = random(H);
    display.drawPixel(x, y, 0);
  }
}

// ─── Framebuffer FX — pre-draw ──────────────────────────────────────

static void fxDescend() {
  uint8_t* buf = display.getBuffer();
  int total = W * H;
  for (int i = 0; i < total; i++) {
    if (buf[i] > 0) buf[i]--;
  }
}

static void fxAscend() {
  uint8_t* buf = display.getBuffer();
  int total = W * H;
  for (int i = 0; i < total; i++) {
    if (buf[i] < 255) buf[i]++;
  }
}

// ─── Framebuffer FX — post-draw ─────────────────────────────────────

static void fxMirrorH() {
  for (int x = 0; x < HALFW; x++) {
    for (int y = 0; y < H; y++) {
      int c = display.getPixel(x, y);
      display.drawPixel((W - 1) - x, y, c);
    }
  }
}

static void fxMirrorV() {
  for (int x = 0; x < W; x++) {
    for (int y = 0; y < HALFH; y++) {
      int c = display.getPixel(x, y);
      display.drawPixel(x, (H - 1) - y, c);
    }
  }
}

static void fxMirrorQuad() {
  for (int x = 0; x < HALFW; x++) {
    for (int y = 0; y < HALFH; y++) {
      int c = display.getPixel(x, y);
      display.drawPixel((W - 1) - x, y, c);
      display.drawPixel(x, (H - 1) - y, c);
      display.drawPixel((W - 1) - x, (H - 1) - y, c);
    }
  }
}

// Echo: blend current frame toward previous frame.
// Full 320×240 shadow (~77KB) plus private program .bss exceeds RP2040 SRAM once
// PicoDVI allocates double buffers — display.begin() fails. Use 2×2 subsampled
// history (~19KB): same temporal smear, slightly blocky (acceptable tradeoff).
#define FX_ECHO_BW (W / 2)
#define FX_ECHO_BH (H / 2)
static uint8_t fxEchoBuf[FX_ECHO_BW * FX_ECHO_BH];
static bool fxEchoReady = false;

static void fxEcho() {
  uint8_t* buf = display.getBuffer();

  if (!fxEchoReady) {
    for (int by = 0; by < FX_ECHO_BH; by++) {
      int row = (by * 2) * W;
      for (int bx = 0; bx < FX_ECHO_BW; bx++) {
        fxEchoBuf[by * FX_ECHO_BW + bx] = buf[row + bx * 2];
      }
    }
    fxEchoReady = true;
    return;
  }

  for (int by = 0; by < FX_ECHO_BH; by++) {
    int row = (by * 2) * W;
    for (int bx = 0; bx < FX_ECHO_BW; bx++) {
      int i = row + bx * 2;
      uint8_t prev = fxEchoBuf[by * FX_ECHO_BW + bx];
      uint8_t blended = ((uint16_t)buf[i] + (uint16_t)prev) >> 1;
      fxEchoBuf[by * FX_ECHO_BW + bx] = buf[i];
      buf[i]         = blended;
      buf[i + 1]     = blended;
      buf[i + W]     = blended;
      buf[i + W + 1] = blended;
    }
  }
}

// ─── Palette FX ─────────────────────────────────────────────────────

static void fxInvert() {
  for (int i = 0; i < 256; i++) {
    uint16_t c = display.getColor(i);
    display.setColor(i, ~c);
  }
}

static void fxReverse() {
  for (int i = 0; i < 128; i++) {
    uint16_t c1 = display.getColor(i);
    uint16_t c2 = display.getColor(255 - i);
    display.setColor(i, c2);
    display.setColor(255 - i, c1);
  }
}

static void fxWipe() {
  int width = 20;
  int index = (int)(normalizedTime(1000, 0) * 255.0f);
  uint16_t c = display.getColor(255);
  for (int i = 0; i < width; i++) {
    display.setColor((index + i) & 255, c);
  }
}

static void fxDropouts() {
  int offset = (int)(normalizedTime(1000, 0) * 255.0f);
  for (int i = 0; i < 256; i++) {
    if (i % 4 != 0) {
      display.setColor((i + offset) & 255, 0);
    }
  }
}

// ─── Pipeline apply functions ───────────────────────────────────────

void applyPaletteFX() {
  if (!g_fxActive) return;
  if (fxIsActive(FX_INVERT))   fxInvert();
  if (fxIsActive(FX_REVERSE))  fxReverse();
  if (fxIsActive(FX_WIPE))     fxWipe();
  if (fxIsActive(FX_DROPOUTS)) fxDropouts();
}

void applyPreDrawFX() {
  if (!g_fxActive) return;
  if (fxIsActive(FX_CUTFADE))  fxCutFade();
  if (fxIsActive(FX_DOTFADE))  fxDotFade();
  if (fxIsActive(FX_DESCEND))  fxDescend();
  if (fxIsActive(FX_ASCEND))   fxAscend();
}

void applyPostDrawFX() {
  if (!g_fxActive) return;
  if (fxIsActive(FX_MIRROR_H))    fxMirrorH();
  if (fxIsActive(FX_MIRROR_V))    fxMirrorV();
  if (fxIsActive(FX_MIRROR_QUAD)) fxMirrorQuad();
  if (fxIsActive(FX_ECHO))        fxEcho();

  // Reset echo state when echo is deactivated
  if (!fxIsActive(FX_ECHO) && fxEchoReady) {
    fxEchoReady = false;
  }
}
