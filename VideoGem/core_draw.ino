// =====================================================================
// CORE: Drawing — polygon, star, fadeScreen, shared primitives
// =====================================================================

void fadeScreen(uint8_t amount) {
  uint8_t* buf = display.getBuffer();
#ifdef UNIT_TEST
  int total = W * H;
  for (int i = 0; i < total; i++) {
    uint8_t v = buf[i];
    buf[i] = (v > amount) ? v - amount : 0;
  }
#else
  uint32_t* buf32 = (uint32_t*)buf;
  int total32 = W * H / 4;
  uint32_t sub4 = (uint32_t)amount * 0x01010101u;
  for (int i = 0; i < total32; i++) {
    uint32_t v = buf32[i];
    uint32_t diff = v - sub4;
    uint32_t borrow = (~v & sub4) | ((~v | sub4) & diff);
    uint32_t mask = ((borrow >> 7) & 0x01010101u) * 0xFFu;
    buf32[i] = diff & ~mask;
  }
#endif
}

void drawPoly(int cx, int cy, int r, int sides, float rot, uint8_t c) {
  float inc = TWO_PI / sides;
  float ci = cosf(inc), si = sinf(inc);
  float vx = cosf(rot), vy = sinf(rot);
  int x0 = cx + (int)(r * vx);
  int y0 = cy + (int)(r * vy);
  for (int i = 0; i < sides; i++) {
    float nx = vx * ci - vy * si;
    float ny = vx * si + vy * ci;
    vx = nx; vy = ny;
    int x1 = cx + (int)(r * vx);
    int y1 = cy + (int)(r * vy);
    display.drawLine(x0, y0, x1, y1, c);
    x0 = x1; y0 = y1;
  }
}

void drawStarShape(int cx, int cy, int r1, int r2, int pts, float rot, uint8_t c) {
  int total = pts * 2;
  float inc = TWO_PI / total;
  float ci = cosf(inc), si = sinf(inc);
  float vx = cosf(rot), vy = sinf(rot);
  for (int i = 0; i < total; i++) {
    int rc = (i & 1) ? r2 : r1;
    int x0 = cx + (int)(rc * vx);
    int y0 = cy + (int)(rc * vy);
    float nx = vx * ci - vy * si;
    float ny = vx * si + vy * ci;
    vx = nx; vy = ny;
    int rn = ((i + 1) & 1) ? r2 : r1;
    int x1 = cx + (int)(rn * vx);
    int y1 = cy + (int)(rn * vy);
    display.drawLine(x0, y0, x1, y1, c);
  }
}
