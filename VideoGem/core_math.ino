// =====================================================================
// CORE: Math — sin table, fast angle, fixed-point helpers
// =====================================================================

uint8_t sinTab[SIN_TAB];

void initSinTable() {
  for (int i = 0; i < SIN_TAB; i++) {
    sinTab[i] = (uint8_t)(128.0f + 127.0f * sinf(i * TWO_PI / SIN_TAB));
  }
}

uint8_t fastAngle(int dx, int dy) {
  if (dx == 0 && dy == 0) return 0;
  int adx = abs(dx), ady = abs(dy);
  uint8_t oct;
  if (adx >= ady) {
    oct = (uint8_t)((uint32_t)ady * 32 / (adx + 1));
  } else {
    oct = 64 - (uint8_t)((uint32_t)adx * 32 / (ady + 1));
  }
  if (dx >= 0 && dy >= 0) return oct;
  if (dx < 0  && dy >= 0) return 128 - oct;
  if (dx < 0  && dy <  0) return 128 + oct;
  return (uint8_t)(0 - oct);
}
