#include "test_framework.h"
#include "PicoDVI.h"
#include "types.h"

DVIGFX8 display(DVI_RES_320x240p60, true, adafruit_feather_dvi_cfg);

#include "core_palette.ino"

// === buildPalette ==========================================================

void test_buildPalette_rainbow_first_stop_red() {
  buildPalette(0);
  // First entry should be near the first color stop (255,0,0)
  TEST_ASSERT(palData[0].r > 200);
  TEST_ASSERT(palData[0].g < 30);
  TEST_ASSERT(palData[0].b < 80);
}

void test_buildPalette_entries_nonzero() {
  buildPalette(0);
  int nonzero = 0;
  for (int i = 0; i < PAL_ENTRIES; i++) {
    if (palData[i].r || palData[i].g || palData[i].b) nonzero++;
  }
  TEST_ASSERT(nonzero > PAL_ENTRIES / 2);
}

void test_buildPalette_all_types_populate() {
  for (int p = 0; p < NUM_PALETTES; p++) {
    buildPalette(p);
    int nonzero = 0;
    for (int i = 0; i < PAL_ENTRIES; i++) {
      if (palData[i].r || palData[i].g || palData[i].b) nonzero++;
    }
    TEST_ASSERT(nonzero > PAL_ENTRIES / 4);
  }
}

void test_buildPalette_smooth_gradient() {
  buildPalette(0);
  int bigJumps = 0;
  for (int i = 1; i < PAL_ENTRIES; i++) {
    int dr = abs((int)palData[i].r - (int)palData[i-1].r);
    int dg = abs((int)palData[i].g - (int)palData[i-1].g);
    int db = abs((int)palData[i].b - (int)palData[i-1].b);
    if (dr > 30 || dg > 30 || db > 30) bigJumps++;
  }
  // Allow a few jumps at stop boundaries, but most should be smooth
  TEST_ASSERT(bigJumps < PAL_ENTRIES / 8);
}

// === applyPalette ==========================================================

void test_applyPalette_zero_offset() {
  buildPalette(0);
  applyPalette(0);

  uint8_t r, g, b;
  // Color index 1 should match palData[0]
  display.getColor(1, r, g, b);
  TEST_ASSERT_EQUAL_INT(palData[0].r, r);
  TEST_ASSERT_EQUAL_INT(palData[0].g, g);
  TEST_ASSERT_EQUAL_INT(palData[0].b, b);
}

void test_applyPalette_with_offset() {
  buildPalette(0);
  int offset = 50;
  applyPalette(offset);

  uint8_t r, g, b;
  // Color index 1 should match palData[offset]
  display.getColor(1, r, g, b);
  TEST_ASSERT_EQUAL_INT(palData[offset].r, r);
  TEST_ASSERT_EQUAL_INT(palData[offset].g, g);
  TEST_ASSERT_EQUAL_INT(palData[offset].b, b);
}

void test_applyPalette_index0_always_black() {
  buildPalette(0);
  applyPalette(0);

  uint8_t r, g, b;
  display.getColor(0, r, g, b);
  TEST_ASSERT_EQUAL_INT(0, r);
  TEST_ASSERT_EQUAL_INT(0, g);
  TEST_ASSERT_EQUAL_INT(0, b);
}

void test_applyPalette_index255_always_white() {
  buildPalette(0);
  applyPalette(0);

  uint8_t r, g, b;
  display.getColor(255, r, g, b);
  TEST_ASSERT_EQUAL_INT(255, r);
  TEST_ASSERT_EQUAL_INT(255, g);
  TEST_ASSERT_EQUAL_INT(255, b);
}

void test_applyPalette_offset_wraps() {
  buildPalette(0);
  // Large offset wraps around via modulo
  applyPalette(PAL_ENTRIES + 10);

  uint8_t r, g, b;
  display.getColor(1, r, g, b);
  TEST_ASSERT_EQUAL_INT(palData[10].r, r);
  TEST_ASSERT_EQUAL_INT(palData[10].g, g);
  TEST_ASSERT_EQUAL_INT(palData[10].b, b);
}

void test_buildPalette_out_of_range_is_noop() {
  buildPalette(0);
  RGB saved = palData[0];
  buildPalette(-1);
  TEST_ASSERT_EQUAL_INT(saved.r, palData[0].r);
  TEST_ASSERT_EQUAL_INT(saved.g, palData[0].g);
  buildPalette(NUM_PALETTES);
  TEST_ASSERT_EQUAL_INT(saved.r, palData[0].r);
  TEST_ASSERT_EQUAL_INT(saved.g, palData[0].g);
}

// ===========================================================================

TEST_SUITE_BEGIN("core_palette")
  RUN_TEST(test_buildPalette_rainbow_first_stop_red);
  RUN_TEST(test_buildPalette_entries_nonzero);
  RUN_TEST(test_buildPalette_all_types_populate);
  RUN_TEST(test_buildPalette_smooth_gradient);
  RUN_TEST(test_buildPalette_out_of_range_is_noop);
  RUN_TEST(test_applyPalette_zero_offset);
  RUN_TEST(test_applyPalette_with_offset);
  RUN_TEST(test_applyPalette_index0_always_black);
  RUN_TEST(test_applyPalette_index255_always_white);
  RUN_TEST(test_applyPalette_offset_wraps);
TEST_SUITE_END
