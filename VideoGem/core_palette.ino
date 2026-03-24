// =====================================================================
// CORE: Palette — build, apply, cycle, palette definitions
// =====================================================================

RGB palData[PAL_ENTRIES];
static int palAppliedOffset = -1;

static void lerpColor(RGB* out, const RGB* a, const RGB* b, float t) {
  out->r = a->r + (int)((b->r - a->r) * t);
  out->g = a->g + (int)((b->g - a->g) * t);
  out->b = a->b + (int)((b->b - a->b) * t);
}

void buildPalette(int palType) {
  if (palType < 0 || palType >= NUM_PALETTES) return;
  palAppliedOffset = -1;
  static const RGB stops[NUM_PALETTES][8] = {
    // 0: RAINBOW PLASMA
    {{255,0,0}, {255,160,0}, {255,255,0}, {0,255,60},
     {0,200,255}, {0,40,255}, {160,0,255}, {255,0,120}},
    // 1: DEEP OCEAN
    {{0,2,20}, {0,20,80}, {0,70,140}, {0,140,180},
     {0,200,200}, {0,120,160}, {10,40,120}, {0,5,40}},
    // 2: HELLFIRE
    {{10,0,0}, {100,0,0}, {200,20,0}, {255,100,0},
     {255,200,0}, {255,255,60}, {255,255,180}, {255,255,255}},
    // 3: ACID TRIP
    {{0,15,0}, {0,100,20}, {40,220,0}, {160,255,0},
     {220,255,40}, {120,255,0}, {0,200,80}, {0,80,40}},
    // 4: NEON STRIPES
    {{0,0,0}, {255,0,120}, {0,0,0}, {0,180,255},
     {0,0,0}, {180,255,0}, {0,0,0}, {255,80,220}},
    // 5: COSMIC VOID
    {{5,0,15}, {40,0,100}, {100,0,200}, {180,20,220},
     {220,80,255}, {255,180,255}, {180,0,220}, {60,0,120}}
  };

  int numStops = 8;
  for (int i = 0; i < PAL_ENTRIES; i++) {
    float pos = (float)i / PAL_ENTRIES * numStops;
    int s0 = (int)pos % numStops;
    int s1 = (s0 + 1) % numStops;
    float t = pos - (int)pos;
    lerpColor(&palData[i], &stops[palType][s0], &stops[palType][s1], t);
  }
}

void applyPalette(int offset) {
  if (offset == palAppliedOffset) return;
  palAppliedOffset = offset;
  for (int i = 0; i < PAL_ENTRIES; i++) {
    int j = (i + offset) % PAL_ENTRIES;
    display.setColor(i + 1, palData[j].r, palData[j].g, palData[j].b);
  }
  display.setColor(0, 0, 0, 0);
  display.setColor(255, 255, 255, 255);
}
