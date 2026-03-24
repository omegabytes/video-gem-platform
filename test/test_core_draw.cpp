#include "test_framework.h"
#include "PicoDVI.h"
#include "types.h"

DVIGFX8 display(DVI_RES_320x240p60, true, adafruit_feather_dvi_cfg);

#include "core_draw.ino"

// === fadeScreen =============================================================

void test_fadeScreen_decreases_values() {
  uint8_t* buf = display.getBuffer();
  for (int i = 0; i < W * H; i++) buf[i] = 100;

  fadeScreen(10);
  for (int i = 0; i < W * H; i++) {
    TEST_ASSERT_EQUAL_INT(90, buf[i]);
  }
}

void test_fadeScreen_clamps_to_zero() {
  uint8_t* buf = display.getBuffer();
  for (int i = 0; i < W * H; i++) buf[i] = 5;

  fadeScreen(10);
  for (int i = 0; i < W * H; i++) {
    TEST_ASSERT_EQUAL_INT(0, buf[i]);
  }
}

void test_fadeScreen_zero_amount_noop() {
  uint8_t* buf = display.getBuffer();
  for (int i = 0; i < W * H; i++) buf[i] = 42;

  fadeScreen(0);
  for (int i = 0; i < W * H; i++) {
    TEST_ASSERT_EQUAL_INT(42, buf[i]);
  }
}

void test_fadeScreen_mixed_values() {
  uint8_t* buf = display.getBuffer();
  buf[0] = 200;
  buf[1] = 3;
  buf[2] = 0;
  buf[3] = 255;

  fadeScreen(5);

  TEST_ASSERT_EQUAL_INT(195, buf[0]);
  TEST_ASSERT_EQUAL_INT(0,   buf[1]);
  TEST_ASSERT_EQUAL_INT(0,   buf[2]);
  TEST_ASSERT_EQUAL_INT(250, buf[3]);
}

void test_fadeScreen_repeated_converges_to_zero() {
  uint8_t* buf = display.getBuffer();
  for (int i = 0; i < W * H; i++) buf[i] = 200;

  for (int rep = 0; rep < 50; rep++) fadeScreen(10);

  for (int i = 0; i < W * H; i++) {
    TEST_ASSERT_EQUAL_INT(0, buf[i]);
  }
}

// === drawPoly ==============================================================

void test_drawPoly_triangle_draws_3_lines() {
  display.resetCounts();
  drawPoly(HALFW, HALFH, 50, 3, 0.0f, 100);
  TEST_ASSERT_EQUAL_INT(3, display.lineCount());
}

void test_drawPoly_square_draws_4_lines() {
  display.resetCounts();
  drawPoly(HALFW, HALFH, 50, 4, 0.0f, 100);
  TEST_ASSERT_EQUAL_INT(4, display.lineCount());
}

void test_drawPoly_hexagon_draws_6_lines() {
  display.resetCounts();
  drawPoly(HALFW, HALFH, 50, 6, 0.0f, 100);
  TEST_ASSERT_EQUAL_INT(6, display.lineCount());
}

// === drawStarShape =========================================================

void test_drawStar_5pt() {
  display.resetCounts();
  drawStarShape(HALFW, HALFH, 60, 30, 5, 0.0f, 200);
  // 5-point star: total segments = 2 * pts = 10
  TEST_ASSERT_EQUAL_INT(10, display.lineCount());
}

void test_drawStar_8pt() {
  display.resetCounts();
  drawStarShape(HALFW, HALFH, 60, 30, 8, 0.0f, 200);
  TEST_ASSERT_EQUAL_INT(16, display.lineCount());
}

// ===========================================================================

TEST_SUITE_BEGIN("core_draw")
  RUN_TEST(test_fadeScreen_decreases_values);
  RUN_TEST(test_fadeScreen_clamps_to_zero);
  RUN_TEST(test_fadeScreen_zero_amount_noop);
  RUN_TEST(test_fadeScreen_mixed_values);
  RUN_TEST(test_fadeScreen_repeated_converges_to_zero);
  RUN_TEST(test_drawPoly_triangle_draws_3_lines);
  RUN_TEST(test_drawPoly_square_draws_4_lines);
  RUN_TEST(test_drawPoly_hexagon_draws_6_lines);
  RUN_TEST(test_drawStar_5pt);
  RUN_TEST(test_drawStar_8pt);
TEST_SUITE_END
