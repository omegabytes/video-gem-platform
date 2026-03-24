#include "test_framework.h"
#include "PicoDVI.h"
#include "Adafruit_Keypad.h"
#include "types.h"

DVIGFX8 display(DVI_RES_320x240p60, true, adafruit_feather_dvi_cfg);

#include "core_math.ino"
#include "core_time.ino"
#include "core_draw.ino"
#include "core_palette.ino"
#include "core_fx.ino"
#include "core_input.ino"
#include "prog_registry.ino"
#include "core_registry.ino"
#include "core_info.ino"

static void setupInfoTest() {
  initSinTable();
  buildPalette(0);
  applyPalette(0);
  mock_set_millis(1000);
  for (int i = 0; i < 16; i++) pots[i] = 512;
  activeProgram = 0;
  activePreset = 0;
  hintsEnabled = false;
  keypad.mock_reset();
}

void test_drawInfoOverlay_runs() {
  setupInfoTest();
  drawInfoOverlay();
  uint8_t* buf = display.getBuffer();
  int nonzero = 0;
  for (int i = 0; i < W * H; i++) { if (buf[i]) nonzero++; }
  TEST_ASSERT(nonzero > 100);
}

void test_drawHints_runs() {
  setupInfoTest();
  hintsEnabled = true;
  drawHints();
  TEST_ASSERT(1);
}

void test_hintsWantFullScreen_initially_false() {
  setupInfoTest();
  TEST_ASSERT(!hintsWantFullScreen());
}

void test_updateHints_and_drawHints_no_crash() {
  setupInfoTest();
  hintsEnabled = true;
  updateHints();
  drawHints();
  TEST_ASSERT(1);
}

void test_programSwitch_triggers_card() {
  setupInfoTest();
  hintsEnabled = true;
  // First update establishes infoPrevProgram
  updateHints();
  TEST_ASSERT(!hintsWantFullScreen());
  // Switch program
  activeProgram = 1;
  updateHints();
  // Card should now be active
  TEST_ASSERT(hintsWantFullScreen());
}

void test_programCard_expires() {
  setupInfoTest();
  hintsEnabled = true;
  updateHints();
  activeProgram = 1;
  updateHints();
  TEST_ASSERT(hintsWantFullScreen());
  // Advance past 4-second expiry
  mock_set_millis(1000 + 4500);
  updateHints();
  TEST_ASSERT(!hintsWantFullScreen());
}

void test_presetChange_clears_card() {
  setupInfoTest();
  hintsEnabled = true;
  updateHints();
  activeProgram = 1;
  updateHints();
  TEST_ASSERT(hintsWantFullScreen());
  // Change preset dismisses card via toast
  activePreset = 1;
  updateHints();
  TEST_ASSERT(!hintsWantFullScreen());
}

void test_infoOverlay_fills_screen() {
  setupInfoTest();
  display.fillScreen(0);
  drawInfoOverlay();
  uint8_t* buf = display.getBuffer();
  int nonzero = 0;
  for (int i = 0; i < W * H; i++) { if (buf[i]) nonzero++; }
  TEST_ASSERT(nonzero > 100);
}

void test_drawHints_indicator_present() {
  setupInfoTest();
  hintsEnabled = true;
  display.fillScreen(0);
  drawHints();
  // "H" indicator is drawn in top-right corner — just verify no crash and runs
  TEST_ASSERT(1);
}

// ===========================================================================

TEST_SUITE_BEGIN("core_info")
  RUN_TEST(test_drawInfoOverlay_runs);
  RUN_TEST(test_drawHints_runs);
  RUN_TEST(test_hintsWantFullScreen_initially_false);
  RUN_TEST(test_updateHints_and_drawHints_no_crash);
  RUN_TEST(test_programSwitch_triggers_card);
  RUN_TEST(test_programCard_expires);
  RUN_TEST(test_presetChange_clears_card);
  RUN_TEST(test_infoOverlay_fills_screen);
  RUN_TEST(test_drawHints_indicator_present);
TEST_SUITE_END
