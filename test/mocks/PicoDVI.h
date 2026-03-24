#ifndef PICODVI_H_MOCK
#define PICODVI_H_MOCK

#include "Arduino.h"

#define DVI_RES_320x240p60 0

struct _dvi_cfg_t {};
static _dvi_cfg_t adafruit_feather_dvi_cfg;

class DVIGFX8 {
public:
  DVIGFX8(int, bool, _dvi_cfg_t) { memset(_buf, 0, sizeof(_buf)); }

  bool begin() { return true; }
  void swap(bool, bool) {}

  uint8_t* getBuffer() { return _buf; }

  uint8_t getPixel(int x, int y) const {
    if (x >= 0 && x < 320 && y >= 0 && y < 240) return _buf[y * 320 + x];
    return 0;
  }

  void setColor(int idx, uint16_t c565) {
    if (idx >= 0 && idx < 256) {
      _pal[idx][0] = (c565 >> 11) & 0x1F;
      _pal[idx][1] = (c565 >> 5) & 0x3F;
      _pal[idx][2] = c565 & 0x1F;
    }
  }

  void setColor(int idx, uint8_t r, uint8_t g, uint8_t b) {
    if (idx >= 0 && idx < 256) { _pal[idx][0] = r; _pal[idx][1] = g; _pal[idx][2] = b; }
  }

  uint16_t getColor(int idx) const {
    if (idx >= 0 && idx < 256) {
      return ((uint16_t)_pal[idx][0] << 11) | ((uint16_t)_pal[idx][1] << 5) | _pal[idx][2];
    }
    return 0;
  }

  void fillScreen(uint8_t c) { memset(_buf, c, sizeof(_buf)); }
  void drawPixel(int x, int y, uint8_t c) {
    if (x >= 0 && x < 320 && y >= 0 && y < 240) _buf[y * 320 + x] = c;
  }
  void drawLine(int, int, int, int, uint8_t)  { _lineCnt++; }
  void drawCircle(int, int, int, uint8_t)     { _circleCnt++; }
  void fillCircle(int, int, int, uint8_t)          {}
  void fillEllipse(int, int, int, int, uint8_t)   {}
  void drawRect(int, int, int, int, uint8_t)       {}
  void fillRect(int, int, int, int, uint8_t)  {}
  void drawFastHLine(int x, int y, int w, uint8_t c) {
    for (int i = 0; i < w && x + i < 320; i++)
      drawPixel(x + i, y, c);
  }
  void drawFastVLine(int x, int y, int h, uint8_t c) {
    for (int i = 0; i < h && y + i < 240; i++)
      drawPixel(x, y + i, c);
  }
  void drawCircleHelper(int, int, int, uint8_t, uint8_t) {}
  void drawTriangle(int, int, int, int, int, int, uint8_t) {}
  void fillTriangle(int, int, int, int, int, int, uint8_t) {}
  void setCursor(int, int) {}
  void setTextColor(uint8_t) {}
  void setTextSize(int) {}
  void print(const char*) {}
  void print(int) {}
  void print(char) {}

  // --- Test helpers --------------------------------------------------------
  int  lineCount()  const { return _lineCnt; }
  int  circleCount() const { return _circleCnt; }
  void resetCounts() { _lineCnt = 0; _circleCnt = 0; }
  void getColor(int idx, uint8_t &r, uint8_t &g, uint8_t &b) const {
    r = _pal[idx][0]; g = _pal[idx][1]; b = _pal[idx][2];
  }

private:
  uint8_t _buf[320 * 240];
  uint8_t _pal[256][3] = {};
  int _lineCnt = 0;
  int _circleCnt = 0;
};

#endif
