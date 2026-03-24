// =====================================================================
// CORE: Time — normalized time, ping-pong, LFO pool
// =====================================================================

float normalizedTime(int periodMS, float phase) {
  return ((millis() + (int)(phase * periodMS)) % periodMS) / (float)periodMS;
}

float pingPongTime(int periodMS, float phase) {
  float t = normalizedTime(periodMS, phase);
  return 1.0f - fabsf(2.0f * t - 1.0f);
}

// ─── LFO pool ───────────────────────────────────────────────────────

LFO g_lfos[NUM_LFOS];

static uint32_t _lfoLastMicros = 0;

void updateLFOs() {
  uint32_t now = micros();
  uint32_t dt = now - _lfoLastMicros;
  _lfoLastMicros = now;

  for (int i = 0; i < NUM_LFOS; i++) {
    // Target: inc = freq_hz * 65536 * dt_sec
    // With freq in 8.8 fixed-point: freq_hz = freq/256
    // So: inc = freq * 256 * dt / 1000000 = freq * dt / 3906.25
    // To avoid overflow with large dt: divide dt by 16 first, then by 244 (≈3906/16)
    uint32_t inc = ((uint32_t)g_lfos[i].freq * (dt >> 4)) / 244;
    g_lfos[i].phase = (g_lfos[i].phase + inc) & 0xFFFF;
  }
}

static inline uint8_t _lfoPhase(int idx, uint8_t offset) {
  return (uint8_t)((g_lfos[idx].phase >> 8) + offset);
}

// Unipolar waveforms: 0–255

uint8_t lfoSine(int idx, uint8_t phaseOffset) {
  return sinTab[_lfoPhase(idx, phaseOffset)];
}

uint8_t lfoTriangle(int idx, uint8_t phaseOffset) {
  uint8_t p = _lfoPhase(idx, phaseOffset);
  return (p < 128) ? (p * 2) : (255 - (p - 128) * 2);
}

uint8_t lfoSquare(int idx, uint8_t phaseOffset) {
  return (_lfoPhase(idx, phaseOffset) < 128) ? 255 : 0;
}

uint8_t lfoSaw(int idx, uint8_t phaseOffset) {
  return _lfoPhase(idx, phaseOffset);
}

uint8_t lfoRamp(int idx, uint8_t phaseOffset) {
  return 255 - _lfoPhase(idx, phaseOffset);
}

// Bipolar waveforms: -128–127

int8_t lfoSineBi(int idx, uint8_t phaseOffset) {
  return (int8_t)(sinTab[_lfoPhase(idx, phaseOffset)] - 128);
}

int8_t lfoTriangleBi(int idx, uint8_t phaseOffset) {
  return (int8_t)(lfoTriangle(idx, phaseOffset) - 128);
}

int8_t lfoSquareBi(int idx, uint8_t phaseOffset) {
  return (_lfoPhase(idx, phaseOffset) < 128) ? 127 : -128;
}

int8_t lfoSawBi(int idx, uint8_t phaseOffset) {
  return (int8_t)(_lfoPhase(idx, phaseOffset) - 128);
}

int8_t lfoRampBi(int idx, uint8_t phaseOffset) {
  return (int8_t)(127 - _lfoPhase(idx, phaseOffset));
}
