// =====================================================================
// CORE: Info — hold overlay, contextual hints, program cards
//
// Two features on k13:
//   Hold >300ms    → full info overlay (replaces viz while held)
//   Double-press   → toggle contextual hints (pot tooltips, cards)
// =====================================================================

// ─── Global pot labels (p0–p3, same across all programs) ────────────
static const char* const globalPotLabels[] = {
  "Palette", "Speed", "Trail", "Intensity"
};

// ─── Pot movement tracking ──────────────────────────────────────────
static int infoPrevPots[16];
static int infoLastMovedPot = -1;
static unsigned long infoLastPotMoveTime = 0;
static bool infoPotTrackReady = false;

// ─── Toast / HUD state ─────────────────────────────────────────────
static int infoPrevProgram = -1;
static int infoPrevPreset = -1;

#define TOAST_NONE    0
#define TOAST_POT     1
#define TOAST_MODE    2
#define TOAST_FX      3
#define TOAST_FXMODE  4
static uint8_t infoToastType = TOAST_NONE;
static unsigned long infoToastExpiry = 0;
static int infoToastPotIdx = -1;
static int infoToastFxId = -1;
static bool infoToastFxOn = false;

static bool infoProgramCardActive = false;
static unsigned long infoProgramCardExpiry = 0;
static unsigned long infoProgramCardShownAt = 0;  // grace period: ignore key/pot for 500ms
static int infoCardPots[16];

// ─── FX toast triggers (called by core_input when FX state changes) ─
static bool infoPrevFxMode = false;
static uint16_t infoPrevFxActive = 0;

void infoCheckFxChanges() {
  unsigned long now = millis();

  if (g_fxMode != infoPrevFxMode) {
    infoToastType = TOAST_FXMODE;
    infoToastExpiry = now + 1500;
    infoPrevFxMode = g_fxMode;
  }

  if (g_fxActive != infoPrevFxActive) {
    uint16_t changed = g_fxActive ^ infoPrevFxActive;
    for (int i = 0; i < FX_COUNT; i++) {
      if (changed & (1 << i)) {
        infoToastFxId = i;
        infoToastFxOn = (g_fxActive & (1 << i)) != 0;
        infoToastType = TOAST_FX;
        infoToastExpiry = now + 1500;
        break;
      }
    }
    infoPrevFxActive = g_fxActive;
  }
}

// ─── Program info routing (table-driven via prog_registry) ──────────

static const char* infoGetProgName() {
  ProgEntry* p = progForSlot(activeProgram);
  return p ? p->name() : "Unknown";
}

static const char* infoGetProgCharacter() {
  ProgEntry* p = progForSlot(activeProgram);
  return p ? p->character() : "";
}

static const char* infoGetPresetName(int preset) {
  ProgEntry* p = progForSlot(activeProgram);
  return p ? p->presetName(preset) : NULL;
}

static const char* infoGetPotLabel(int preset, int pot) {
  if (pot < 4) {
    ProgEntry* p = progForSlot(activeProgram);
    if (p) {
      const char* label = p->potLabel(preset, pot);
      if (label && label[0]) return label;
    }
    return globalPotLabels[pot];
  }
  ProgEntry* p = progForSlot(activeProgram);
  return p ? p->potLabel(preset, pot) : "";
}

// ─── Helpers ────────────────────────────────────────────────────────

static void infoPrintPadded(const char* s, int width) {
  int len = 0;
  if (s) { while (s[len]) len++; }
  int printed = 0;
  if (s) { display.print(s); printed = len; }
  while (printed < width) { display.print(' '); printed++; }
}

static void infoPrintRightAligned(int value, int x, int y) {
  char buf[6];
  snprintf(buf, sizeof(buf), "%4d", value);
  display.setCursor(x, y);
  display.print(buf);
}

static void infoDrawPotRow(int col, int y, int potIdx, bool highlight) {
  int x = (col == 0) ? 4 : 168;
  const char* label = infoGetPotLabel(activePreset, potIdx);

  if (highlight) {
    display.fillRect(x - 2, y - 1, 154, 10, 255);
    display.setTextColor(0);
  } else {
    display.setTextColor(255);
  }

  display.setCursor(x, y);
  char prefix[5];
  snprintf(prefix, sizeof(prefix), "p%-2d ", potIdx);
  display.print(prefix);
  infoPrintPadded(label, 10);
  infoPrintRightAligned(pots[potIdx], x + 90, y);

  display.setTextColor(255);
}

static void infoUpdatePotHighlight() {
  if (!infoPotTrackReady) {
    for (int i = 0; i < 16; i++) infoPrevPots[i] = pots[i];
    infoPotTrackReady = true;
    return;
  }
  int maxDelta = 0;
  int best = -1;
  for (int i = 0; i < 16; i++) {
    int d = abs(pots[i] - infoPrevPots[i]);
    if (d > maxDelta) { maxDelta = d; best = i; }
    infoPrevPots[i] = pots[i];
  }
  if (maxDelta > 8) {
    infoLastMovedPot = best;
    infoLastPotMoveTime = millis();
  } else if (millis() - infoLastPotMoveTime > 1500) {
    infoLastMovedPot = -1;
  }
}

// ─── Full info overlay (k13 hold) ──────────────────────────────────

void drawInfoOverlay() {
  display.fillScreen(0);
  infoUpdatePotHighlight();

  // Program name (text size 2)
  display.setTextSize(2);
  display.setTextColor(255);
  display.setCursor(8, 4);
  display.print(infoGetProgName());

  // Preset name (text size 1)
  display.setTextSize(1);
  display.setCursor(8, 24);
  const char* presetName = infoGetPresetName(activePreset);
  display.print(presetName ? presetName : "---");

  // Pot rows: two columns, p0-p7 left, p8-p15 right
  int rowY = 44;
  for (int r = 0; r < 8; r++) {
    int leftPot = r;
    int rightPot = r + 8;
    bool hlL = (infoLastMovedPot == leftPot);
    bool hlR = (infoLastMovedPot == rightPot);

    infoDrawPotRow(0, rowY, leftPot, hlL);
    infoDrawPotRow(1, rowY, rightPot, hlR);

    // Separator between globals (p3) and per-mode (p4)
    if (r == 3) {
      rowY += 4;
      rowY += 6;
    } else {
      rowY += 12;
    }
  }

  // Active FX list
  if (g_fxActive) {
    rowY += 4;
    display.setTextColor(200);
    display.setCursor(4, rowY);
    display.print("FX: ");
    for (int i = 0; i < FX_COUNT; i++) {
      if (fxIsActive(i)) {
        const char* n = fxGetName(i);
        if (n) { display.print(n); display.print(' '); }
      }
    }
    rowY += 12;
  }

  // Key legend
  rowY += 8;
  display.setTextColor(128);
  display.setCursor(4, rowY);
  display.print("Ctrl+k12=FX  k13=INFO  Ctrl+Shft+key=PROG");

  // Hints & FX mode status
  rowY += 12;
  display.setCursor(4, rowY);
  display.print("HINTS: ");
  display.setTextColor(hintsEnabled ? 255 : 80);
  display.print(hintsEnabled ? "ON" : "OFF");
  display.setTextColor(128);
  display.print("  FX MODE: ");
  display.setTextColor(g_fxMode ? 255 : 80);
  display.print(g_fxMode ? "ON" : "OFF");

  display.setTextSize(1);
  display.setTextColor(255);
}

// ─── Program card (shown on program switch when hints enabled) ─────

static void infoDrawProgramCard() {
  display.fillScreen(0);
  display.setTextColor(255);

  // "PROGRAM N" header
  display.setTextSize(2);
  const char* pname = infoGetProgName();
  int nameLen = 0;
  if (pname) { while (pname[nameLen]) nameLen++; }

  display.setCursor(20, 10);
  display.print("PROGRAM ");
  display.print(activeProgram + 1);

  // Program name large
  display.setTextSize(3);
  int nameW = nameLen * 18;
  int nameX = (W - nameW) / 2;
  if (nameX < 4) nameX = 4;
  display.setCursor(nameX, 36);
  display.print(pname);

  // Character tagline
  display.setTextSize(1);
  display.setTextColor(180);
  display.setCursor(20, 68);
  display.print(infoGetProgCharacter());

  // Primary preset list (k0–k11), two columns
  display.setTextColor(255);
  int listY = 88;
  for (int i = 0; i < 12; i++) {
    const char* name = infoGetPresetName(i);
    if (!name || name[0] == '\0') continue;

    int col = (i < 6) ? 0 : 1;
    int row = (i < 6) ? i : i - 6;
    int x = (col == 0) ? 20 : 170;
    int y = listY + row * 12;

    display.setCursor(x, y);
    char kLabel[5];
    snprintf(kLabel, sizeof(kLabel), "k%-2d ", i);
    display.print(kLabel);
    display.print(name);
  }

  // Secondary presets (shift+k0–k11), if any exist
  const char* firstShift = infoGetPresetName(12);
  if (firstShift && firstShift[0] != '\0') {
    int shiftY = listY + 78;
    display.setTextColor(128);
    display.setCursor(20, shiftY);
    display.print("SHIFT MODES");
    display.setTextColor(255);
    shiftY += 14;

    for (int i = 0; i < 12; i++) {
      const char* name = infoGetPresetName(12 + i);
      if (!name || name[0] == '\0') continue;

      int col = (i < 6) ? 0 : 1;
      int row = (i < 6) ? i : i - 6;
      int x = (col == 0) ? 20 : 170;
      int y = shiftY + row * 12;

      display.setCursor(x, y);
      char kLabel[5];
      snprintf(kLabel, sizeof(kLabel), "+k%-1d ", i);
      display.print(kLabel);
      display.print(name);
    }
  }

  display.setTextSize(1);
}

// ─── Bottom toast bar ──────────────────────────────────────────────

static void infoDrawToastBar(const char* text) {
  display.fillRect(0, H - 16, W, 16, 0);
  display.drawFastHLine(0, H - 17, W, 128);
  display.setTextSize(1);
  display.setTextColor(255);
  display.setCursor(6, H - 12);
  display.print(text);
}

// ─── Hints system: detect changes and render ────────────────────────

void updateHints() {
  infoUpdatePotHighlight();
  infoCheckFxChanges();
  unsigned long now = millis();

  // Detect program change → show program card
  if (infoPrevProgram != activeProgram) {
    if (infoPrevProgram >= 0) {
      infoProgramCardActive = true;
      infoProgramCardExpiry = now + 4000;
      infoProgramCardShownAt = now;
      for (int i = 0; i < 16; i++) infoCardPots[i] = pots[i];
    }
    infoPrevProgram = activeProgram;
    infoPrevPreset = activePreset;
    return;
  }

  // Detect preset change → show mode name toast
  if (infoPrevPreset != activePreset) {
    infoPrevPreset = activePreset;
    infoToastType = TOAST_MODE;
    infoToastExpiry = now + 2000;
    infoProgramCardActive = false;
    return;
  }

  // Detect pot movement → show pot tooltip
  if (infoLastMovedPot >= 0) {
    infoToastType = TOAST_POT;
    infoToastPotIdx = infoLastMovedPot;
    infoToastExpiry = now + 1500;
  }

  // Expire program card (early dismiss on key press or pot movement after grace period)
  if (infoProgramCardActive) {
    if (now > infoProgramCardExpiry) {
      infoProgramCardActive = false;
    } else {
      unsigned long grace = 500;
      if (now - infoProgramCardShownAt > grace) {
        for (int i = 0; i < 12; i++) {
          if (keysPressed[i]) { infoProgramCardActive = false; break; }
        }
        for (int i = 0; i < 16; i++) {
          if (abs(pots[i] - infoCardPots[i]) > 50) {
            infoProgramCardActive = false; break;
          }
        }
      }
    }
  }

  // Expire toast
  if (infoToastType != TOAST_NONE && now > infoToastExpiry) {
    infoToastType = TOAST_NONE;
  }
}

bool hintsWantFullScreen() {
  return infoProgramCardActive;
}

void drawHints() {
  // Program card takes over the full screen
  if (infoProgramCardActive) {
    infoDrawProgramCard();
    return;
  }

  // Bottom bar toasts
  if (infoToastType == TOAST_POT && infoToastPotIdx >= 0) {
    const char* label = infoGetPotLabel(activePreset, infoToastPotIdx);
    char buf[54];
    // Bottom bar hint: keep it compact (no preset/program name suffix).
    snprintf(buf, sizeof(buf), "p%d: %s  %d",
             infoToastPotIdx, label ? label : "?", pots[infoToastPotIdx]);
    infoDrawToastBar(buf);
  } else if (infoToastType == TOAST_MODE) {
    const char* name = infoGetPresetName(activePreset);
    char buf[40];
    snprintf(buf, sizeof(buf), "Mode: %s", name ? name : "---");
    infoDrawToastBar(buf);
  }

  // FX toast
  if (infoToastType == TOAST_FX && infoToastFxId >= 0) {
    const char* name = fxGetName(infoToastFxId);
    char buf[40];
    snprintf(buf, sizeof(buf), "FX: %s %s",
             name ? name : "?", infoToastFxOn ? "ON" : "OFF");
    infoDrawToastBar(buf);
  } else if (infoToastType == TOAST_FXMODE) {
    infoDrawToastBar(g_fxMode ? "FX MODE ON" : "FX MODE OFF");
  }

  // Indicators in top-right corner
  display.setTextSize(1);
  int ix = W - 8;

  // FX mode badge
  if (g_fxMode) {
    display.setTextColor(255);
    display.fillRect(ix - 16, 0, 24, 10, 200);
    display.setCursor(ix - 14, 2);
    display.print("FX");
    ix -= 22;
  } else if (g_fxActive) {
    display.setTextColor(200);
    display.setCursor(ix - 6, 2);
    display.print("fx");
    ix -= 16;
  }

  // Hints-enabled indicator
  display.setTextColor(100);
  display.setCursor(ix, 2);
  display.print('H');
  display.setTextColor(255);
}
