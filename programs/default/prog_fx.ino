// =====================================================================
// PROGRAM: FX Demo (Video_Gem_FX by RMNA)
//
// Standalone FX demo program — draws a base pattern, then applies one
// effect per preset to showcase the global FX catalog.
//
// FX implementations live in core_fx.ino. This program is a thin
// wrapper that calls them directly for demonstration purposes.
// =====================================================================

#include "bitmaps.h"

static inline int fx_pot(int localIdx, int lo, int hi) {
  return potMap(4 + localIdx, lo, hi);
}

static void fx_base() {
  int yOffset = fx_pot(1, 0, 255);
  for (int y = 0; y < H; y++) {
    uint8_t c = (uint8_t)(((y * 255) / (H - 1) + yOffset) & 255);
    display.drawFastHLine(0, y, W, c);
  }

  int bitmap = fx_pot(0, 0, 5);
  int count = fx_pot(2, 6, 60);
  int spread = fx_pot(3, 0, 100);

  int seedPeriodMS = 50 + spread * 10;
  randomSeed(millis() / seedPeriodMS);

  uint8_t fg = (uint8_t)fx_pot(2, 1, 254);
  for (int i = 0; i < count; i++) {
    int x = random(W);
    int y = random(H);
    display.drawBitmap(x - 8, y - 8, spark_bitmap_array[bitmap], 17, 17, fg);
  }
}

// Each preset temporarily enables one FX, draws the base, then applies it.
// This does NOT modify g_fxActive — it calls the FX functions directly.

static void fx_demo_apply(int fxId) {
  // Pre-draw FX
  if (fxId == FX_CUTFADE || fxId == FX_DOTFADE ||
      fxId == FX_DESCEND || fxId == FX_ASCEND) {
    uint16_t saved = g_fxActive;
    g_fxActive = (1 << fxId);
    applyPreDrawFX();
    g_fxActive = saved;
  }
  // Post-draw FX
  else if (fxId >= FX_MIRROR_H && fxId <= FX_ECHO) {
    uint16_t saved = g_fxActive;
    g_fxActive = (1 << fxId);
    applyPostDrawFX();
    g_fxActive = saved;
  }
  // Palette FX
  else if (fxId >= FX_INVERT && fxId <= FX_DROPOUTS) {
    uint16_t saved = g_fxActive;
    g_fxActive = (1 << fxId);
    applyPaletteFX();
    g_fxActive = saved;
  }
}

// ─── Public interface ────────────────────────────────────────────────

const char* prog_fx_name() { return "FX Demo"; }

const char* prog_fx_character() {
  return "Effects showcase — Video_Gem_FX by RMNA, adapted for Video Gem";
}

static const char* const fx_presetNames[] = {
  "CutFade", "DotFade", "Descend", "Ascend",
  "MirrorH", "MirrorV", "MirrorQuad", "Echo",
  "Invert", "Reverse", "Wipe", "Dropouts"
};

const char* prog_fx_presetName(int preset) {
  if (preset >= 0 && preset < 12) return fx_presetNames[preset];
  return NULL;
}

const char* prog_fx_potLabel(int preset, int pot) {
  (void)preset;
  if (pot == 4) return "Bitmap";
  if (pot == 5) return "YOff";
  if (pot == 6) return "Count";
  if (pot == 7) return "Spread";
  return "";
}

uint8_t prog_fx_renderHint(int preset) {
  (void)preset;
  return RENDER_CLEAR;
}

void prog_fx_draw(int preset) {
  fx_base();
  if (preset >= 0 && preset < FX_COUNT) {
    fx_demo_apply(preset);
  }
}
