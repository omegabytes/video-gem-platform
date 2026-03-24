# Video Gem Platform — Capabilities & Conformance

Describes the platform's control model, capabilities, and the interface programs must implement. No specific program ideas; the platform is agnostic to visualizer content.

---

## 1. Control Model

### Physical Layout

```
        ┌────┬────┬────┬────┬────┬────┬────┬────┐
        │ p0 │ p1 │ p2 │ p3 │ p4 │ p5 │ p6 │ p7 │  ← pots 0–7
        ├────┼────┼────┼────┼────┼────┼────┼────┤
        │ k0 │ k1 │ k2 │ k3 │ p8 │ p9 │p10 │p11 │  ← keys 0–3, pots 8–11
        ├────┼────┼────┼────┼────┼────┼────┼────┤
        │ k4 │ k5 │ k6 │ k7 │p12 │p13 │p14 │p15 │  ← keys 4–7, pots 12–15
        ├────┼────┼────┼────┼────┼────┼────┼────┤
        │ k8 │ k9 │k10 │k11 │k12 │k13 │k14 │k15 │  ← keys 8–15
        └────┴────┴────┴────┴────┴────┴────┴────┘
```

### Key Roles

| Key(s) | Role | Behavior |
|--------|------|----------|
| **k0–k11** | Preset select | Primary 12 modes within the current program. In FX mode: k0–k11 toggle FX on/off |
| **k12** | Modifier A | Program-defined (e.g. freeze, hold effects). Ctrl+k12 = toggle FX mode. Shift+k12 (in FX mode) = clear all FX |
| **k13** | Info | **System-level.** Hold = full info overlay. Double-tap = toggle contextual hints |
| **k14** | Control | Program switcher. Hold with k15 + k0–k11 to load one of 12 programs. Hold + k12 = toggle FX mode |
| **k15** | Shift | Extends k0–k11 to slots 12–23 (secondary presets within same program) |

### Control vs Shift

- **Shift (k15):** "More of the same" — doubles preset slots. Same program, different modes.
- **Control (k14):** "Change context" — system-level actions. Ctrl+Shift+k0–k11 = program switch. Ctrl+k12 = FX mode toggle.

### k13 — System Info Key

k13 is system-level, not program-defined:

| Gesture | Behavior |
|---------|----------|
| **Hold** (>300ms) | Full info overlay — program name, preset name, all 16 pot labels with live values |
| **Double-press** (<400ms) | Toggle contextual hints — pot tooltips, program card on switch, preset name toast |

### Pot Roles

| Range | Role | Platform behavior |
|-------|------|-------------------|
| **p0–p3** | Global | Same semantics across all programs. Platform suggests: palette, speed, trail, intensity |
| **p4–p15** | Per-preset | Each preset defines its own mapping. Fully custom per program |

---

## 2. Required Conformance (Platform Contract)

Programs must:

- Expose: `draw()`, `renderHint()`, `name()`, `character()`, `presetName()`, `potLabel()`
- Use p0–p3 for globals; p4–p15 per preset
- Honor k14+k15 chord for program switch (platform handles this)
- Define k12 behavior (or NOP if unused)

Fixed platform behavior (not configurable):

- k0–k11 select presets
- k15 = Shift for slots 12–23
- k14+k15+k0..k11 = program switch (up to 12 loadable programs)
- 320×240, 60 fps, 8-bit indexed color

---

## 3. Allowed Variation

| Aspect | Flexibility |
|--------|-------------|
| Preset count | 1–24 (slots may be empty) |
| p4–p15 mapping | Fully custom per preset |
| k12 | Program-defined modifier or NOP |
| p0 semantics | Palette index, hue offset, theme — program chooses |
| Per-preset state | Independent parameters and animation state |

---

## 4. Optional Recommendations

- Graceful empty slots (simple fallback for unused k0–k11)
- Smooth preset transitions
- Per-preset parameter recall

---

## 5. Global FX Layer — Proposal

### 5.1 Overview

The legacy Video_Gem_FX sketch contained 11 frame-manipulation effects that were tied to a single program. This proposal promotes FX to a **global platform feature**: effects that can be layered on top of *any* program's output, managed entirely by the platform core.

Programs should remain unaware of FX. FX are applied by the platform's render pipeline, not by individual programs. This means every program—current and future—benefits from FX without any code changes.

### 5.2 FX Catalog

FX are grouped into three categories based on *what they modify*. Each category corresponds to a different technique and a different point in the render pipeline.

#### Canvas FX (draw operations that add/remove content)

These use drawing primitives (`drawFastHLine`, `drawPixel`) to paint or erase content on the framebuffer surface. They run **pre-draw** (before the program draws), creating decay effects that interact with the program's trail behavior.

| ID | Name | Description | Pipeline stage |
|----|------|-------------|----------------|
| 0 | **cutFade** | Erase random horizontal lines (scanline decay) | Pre-draw |
| 1 | **dotFade** | Erase random individual pixels (pixel decay) | Pre-draw |

Canvas FX are complementary to the existing render hint system (RENDER_CLEAR / RENDER_TRAIL / RENDER_PERPIXEL). When a program uses RENDER_TRAIL, a canvas FX adds a second layer of frame decay on top of the trail fade.

#### Framebuffer FX (read-modify-write pixel data)

These use `getPixel()` / `drawPixel()` to read and transform existing pixel values in the buffer. They split across two pipeline stages depending on purpose:

- **Pre-draw** variants shift palette indices across the entire frame (decay/color-cycling effects).
- **Post-draw** variants rearrange spatial layout (symmetry/mirror effects) after the program has finished drawing.

| ID | Name | Description | Pipeline stage |
|----|------|-------------|----------------|
| 2 | **descend** | Decrement every pixel's palette index by 1 (palette shift down) | Pre-draw |
| 3 | **ascend** | Increment every pixel's palette index by 1 (palette shift up) | Pre-draw |
| 4 | **mirrorH** | Copy left half to right half (horizontal mirror) | Post-draw |
| 5 | **mirrorV** | Copy top half to bottom half (vertical mirror) | Post-draw |
| 6 | **mirrorQuad** | Copy top-left quadrant to all four quadrants | Post-draw |
| 7 | **echo** | Blend current frame with previous frame (feedback trail) | Post-draw |

Post-draw framebuffer FX are powerful because they work with any program's output. A simple generative pattern becomes a complex kaleidoscopic composition.

**Note on echo:** Requires a ~77KB shadow buffer (320×240×1 byte) to store the previous frame for blending. With 264KB total SRAM and PicoDVI's double-buffer already consuming ~154KB, this is tight but feasible. The blend ratio is a natural parameter for pot control in FX mode.

#### Palette FX (modify the color lookup table)

These use `getColor()` / `setColor()` to transform the 256-entry color palette each frame, creating color-space effects that apply to everything on screen. They run in their own **palette stage** after the platform builds and applies the palette from p0/p3.

| ID | Name | Description | Pipeline stage |
|----|------|-------------|----------------|
| 8 | **invert** | Bitwise-NOT all 256 palette entries (color negative) | Palette |
| 9 | **reverse** | Reverse palette order (swap index i with 255−i) | Palette |
| 10 | **wipe** | Overwrite a sliding window of palette entries with the last color | Palette |
| 11 | **dropouts** | Zero out every non-4th palette entry with a cycling offset | Palette |

### 5.3 UX: FX Mode Layer (Ctrl + k12)

A dedicated mode toggle that temporarily repurposes the key grid and pot bank for FX control. This cleanly separates "performance mode" (normal operation) from "FX mode" (configuring the global effects layer) without breaking any existing key or pot assignments.

| Action | Gesture |
|--------|---------|
| Enter/exit FX mode | Ctrl + k12 (toggle) |
| In FX mode: toggle FX on/off | k0–k11 (toggles individual FX) |
| In FX mode: clear all | Shift + k12 |
| In FX mode: FX parameters | p4–p15 remap to FX parameters |

#### Behavior

- **FX selection:** While in FX mode, k0–k11 become FX toggle switches (ordered by mechanism: canvas 0–1, framebuffer 2–7, palette 8–11). All 12 keys map to the 12 FX. Multiple FX stack. Press a key to toggle that FX on; press again to toggle off.
- **Clear all:** Shift + k12 while in FX mode clears the entire FX bitmask and resets all FX parameters to defaults.
- **FX parameters:** In FX mode, p4–p15 remap to FX-specific parameters. Each active FX can claim 2–3 pots. The info overlay (k13 hold) shows FX-specific pot labels when in FX mode.
- **Visual indicator:** An "FX" badge appears in the corner of the display while FX mode is entered. A toast shows each FX name as it is toggled on/off.
- **Persistence:** Active FX and their parameter values persist across preset changes and program switches. Entering/exiting FX mode does not clear them — only Shift+k12 (clear all) or individually toggling each FX off does.
- **Programs are unaffected:** k12 retains its program-defined Modifier A role in normal mode. k13, Shift, and preset select are unchanged. FX mode only intercepts k0–k11 and p4–p15 while active.

#### Trade-offs

| Pros | Cons |
|------|------|
| Full parameter control over every FX | Modal — preset selection disabled in FX mode |
| Multiple simultaneous FX with individual parameters | Ctrl+k12 chord is less discoverable |
| Clean separation: "FX mode" vs "performance mode" | Users must remember to exit FX mode |
| Doesn't break k12, k13, Shift, or preset select | Higher implementation complexity |
| Scalable to many more FX in the future | |

### 5.5 Render Pipeline Integration

The updated render pipeline with FX insertion points:

```
 1. Input scan (keypad + pots)
 2. Program lookup + init on switch
 3. Global parameter application (palette, speed, trail, intensity)
 4. ► Palette FX (invert, reverse, wipe, dropouts — modify color LUT)
 5. Info overlay / hints (may replace frame)
 6. Render hint application (clear / fade / none)
 7. ► Canvas + Framebuffer pre-draw FX (cutFade, dotFade, descend, ascend)
 8. Program draw
 9. ► Framebuffer post-draw FX (mirrorH, mirrorV, mirrorQuad, echo)
10. Hint overlay (bottom bar / indicator)
11. Framebuffer swap
```

#### Insertion rationale

- **Palette FX (step 4):** Must run after `buildPalette()` + `applyPalette()` so they layer on top of the user's palette selection. Must run before any drawing so all subsequent `drawPixel` / `fillScreen` calls use the modified palette.

- **Canvas + Framebuffer pre-draw FX (step 7):** Run between render-hint and program draw. The render hint controls the baseline frame treatment (clear, fade, leave). Pre-draw FX apply *additional* transformations to whatever the render hint left behind. Canvas FX (cutFade, dotFade) erase content; framebuffer FX (descend, ascend) shift pixel values. For RENDER_CLEAR programs, these have no visible effect (the screen was just wiped). For RENDER_TRAIL programs, they create compound decay effects. This interaction is a feature — it makes FX behavior program-aware without programs needing to know about FX.

- **Framebuffer post-draw FX (step 9):** Run after the program has finished drawing but before hint overlays. This ensures the mirror/symmetry effects transform the program's output, and the hint bar remains readable (not mirrored).

### 5.6 Composability

All FX are composable — the bitmask allows any combination of the 12 FX to be active simultaneously. Within each pipeline stage, active FX apply in ID order.

#### Cross-category composition

FX from different categories compose cleanly because they run at different pipeline stages and operate on different things:

| Combination | Behavior |
|-------------|----------|
| cutFade + mirrorQuad | Scanline decay (pre-draw) followed by symmetry (post-draw) — the mirrored image shows the decay pattern |
| dotFade + invert | Pixel decay (pre-draw) with inverted colors (palette) — decay happens in color-negative |
| ascend + echo + reverse | Index shift (pre-draw), frame blending (post-draw), palette reversal (palette) — all three layers interact |

Cross-category stacking is where the system becomes expressive. A performer can build complex layered effects from simple primitives.

#### Within-category composition

FX in the same category apply sequentially in ID order. Some combinations are useful, some are redundant, and some cancel out:

| Combination | Result |
|-------------|--------|
| cutFade + dotFade | Additive erasure — both remove content, compound decay |
| descend + ascend | Cancel out (decrement then increment = no net change) |
| mirrorH + mirrorV | Effectively mirrorQuad (left→right, then top→bottom) |
| mirrorH + mirrorQuad | Redundant — mirrorQuad overwrites mirrorH's result |
| mirror* + echo | Mirror first, then echo blends with the previous (unmirrored) frame — creates an asymmetric feedback loop |
| invert + reverse | Both transform the LUT sequentially — order-dependent compound effect |
| wipe + dropouts | Both destroy palette entries — compound color reduction |

The platform does not prevent "useless" combinations like descend+ascend. This is intentional — the performer decides what's interesting, and accidental combinations can produce unexpected results worth exploring.

### 5.7 Platform Interface

#### New file: `core_fx.ino`

Contains FX mode state, FX function implementations, per-FX parameter storage, and the three apply functions called by the main loop.

```
// State
bool     g_fxMode;             // true while FX mode is active (Ctrl+k12 toggle)
uint16_t g_fxActive;           // bitmask: bit N = FX N is active
                                // bits 0–1 = canvas, 2–3 = framebuffer pre-draw,
                                // 4–7 = framebuffer post-draw, 8–11 = palette
int      g_fxParams[FX_COUNT]; // per-FX parameter value (set via pots in FX mode)

// Called by main loop at the appropriate pipeline stages
void applyPaletteFX();          // step 4: palette FX (modify color LUT)
void applyPreDrawFX();          // step 7: canvas + framebuffer pre-draw FX
void applyPostDrawFX();         // step 9: framebuffer post-draw FX

// Called by core_input keypad handler
void fxToggle(int fxId);        // toggle FX fxId on/off
void fxClearAll();              // clear all active FX
bool fxIsActive(int fxId);      // query state (for info overlay)
int  fxActiveCount();           // number of active FX (for indicator)

// FX-mode pot labels (shown in info overlay while in FX mode)
const char* fxPotLabel(int potIdx);
```

#### New defines in `types.h`

```
// FX identifiers — grouped by mechanism (match section 5.2)
// Canvas FX (draw primitives)
#define FX_CUTFADE    0
#define FX_DOTFADE    1
// Framebuffer FX — pre-draw (index shift)
#define FX_DESCEND    2
#define FX_ASCEND     3
// Framebuffer FX — post-draw (spatial transform + temporal blend)
#define FX_MIRROR_H   4
#define FX_MIRROR_V   5
#define FX_MIRROR_QUAD 6
#define FX_ECHO       7
// Palette FX (color LUT transform)
#define FX_INVERT     8
#define FX_REVERSE    9
#define FX_WIPE       10
#define FX_DROPOUTS   11
#define FX_COUNT      12

// FX state (declared in core_fx.ino)
extern bool     g_fxMode;
extern uint16_t g_fxActive;
```

#### Changes to `core_input.ino`

Add FX mode toggle and FX-mode key/pot handling:

```
// Inside handleKeypad(), in the KEY_JUST_PRESSED branch:

// Ctrl + k12: toggle FX mode
if (keynum == 12 && controlHeld) {
  g_fxMode = !g_fxMode;
  // toast "FX MODE ON" / "FX MODE OFF"
}

// Shift + k12: clear all FX (works in FX mode)
if (keynum == 12 && shiftHeld && g_fxMode) {
  fxClearAll();
  // toast "FX CLEARED"
}

if (keynum <= 11) {
  if (controlHeld && shiftHeld) {
    activeProgram = keynum;       // existing: program switch
    activePreset = 0;
  } else if (g_fxMode) {
    fxToggle(keynum);             // FX mode: k0–k11 = toggle FX
  } else if (shiftHeld) {
    activePreset = keynum + 12;   // existing: secondary presets
  } else {
    activePreset = keynum;        // existing: primary presets
  }
}

// In pot scanning: when g_fxMode is true, p4–p15 write to
// g_fxParams[] instead of the program's per-preset parameters.
```

#### Changes to `VideoGem.ino` (main loop)

```
// After applyPalette(paletteOffset):
applyPaletteFX();                   // step 4

// After render hint, before prog->draw():
applyPreDrawFX();                   // step 7

// After prog->draw(), before drawHints():
applyPostDrawFX();                  // step 9
```

#### Changes to `core_info.ino`

- When FX mode is active, display an "FX" badge in the corner of the display
- When FX are active (regardless of mode), display a small "FX" indicator in the top-right corner (next to the "H" hints indicator)
- In the full info overlay (k13 hold), show FX-specific pot labels (via `fxPotLabel()`) when in FX mode; show "Active FX" section listing enabled effects by name
- Toast notification when an FX is toggled: "FX: mirrorQuad ON" / "FX: mirrorQuad OFF"
- Toast notification on mode entry/exit: "FX MODE ON" / "FX MODE OFF"

### 5.8 Migration from prog_fx.ino

The existing `prog_fx.ino` (program slot 11) should be retained as a standalone FX *demo* program that showcases effects with its own base visualization. The FX function implementations (mirror, descend, invert, etc.) should be **moved to `core_fx.ino`** and made non-static so both the platform pipeline and the FX demo program can call them. The FX demo program becomes a thin wrapper that draws a base pattern and calls the shared FX functions.

### 5.9 Open Questions

1. **FX persistence across program switches:** Should active FX survive a program switch (Ctrl+Shift+k), or should switching programs clear the FX bitmask? Recommendation: persist by default, since FX are a platform-level layer independent of the active program.

2. **FX + RENDER_CLEAR interaction:** Pre-draw FX (cutFade, dotFade, descend, ascend) have no visible effect on programs that use RENDER_CLEAR, because the screen is wiped before draw. Should the platform suppress pre-draw FX for RENDER_CLEAR programs (to avoid wasted CPU), or allow them (in case the program switches render hints per-preset)? Recommendation: always run them — the CPU cost is small and the behavior is correct if the program changes render hints dynamically.

3. **FX stacking order:** When multiple post-draw FX are active (e.g., mirrorH + mirrorQuad), what order do they apply? Recommendation: apply in FX ID order (mirrorH before mirrorQuad), which means mirrorQuad will overwrite the mirrorH result. Document this as "later FX override earlier ones in the same category."

4. **Pot allocation in FX mode:** With 12 remappable pots (p4–p15) and 12 FX, how should pots map to FX parameters? Not all FX need parameters (mirror, invert are inherently parameterless). Parameterized FX (cutFade count, echo blend ratio, wipe width, dropout density) could share pots by category. Suggested layout: p4–p5 = canvas FX params, p6–p9 = framebuffer FX params (including echo blend), p10–p13 = palette FX params, p14–p15 = reserved.

5. **Echo memory budget:** Echo requires a ~77KB shadow buffer (320×240 bytes). PicoDVI's double-buffer uses ~154KB of 264KB SRAM, leaving ~110KB for stack, globals, and the echo buffer. This is feasible but tight — the echo buffer should be statically allocated and only populated while FX_ECHO is active to avoid unnecessary memory pressure.

6. **Future FX expansion:** The bitmask supports up to 16 FX (uint16_t). The current catalog uses 12, leaving room for 4 more. Candidates: scroll/shift (translate the framebuffer), rotate 90°/180°, posterize (reduce palette depth), scanlines (darken every Nth row). New FX can be added to `core_fx.ino` without any program changes.

---

## 6. LFO Utility — Proposal

### 6.1 Overview

The legacy Video_Gem_Comprehensive sketch included an `LFO` class — a low-frequency oscillator that provides multiple waveform shapes, independent frequency control, and phase offsets. This is a standard building block in video synthesis that the platform should offer as a shared utility.

Unlike the Global FX Layer (section 5), LFOs are **program-facing utilities** — tools that programs opt into, not pipeline stages that run automatically. Programs that use `sin(globalTime)` today continue to work unchanged.

### 6.2 Current Platform Gaps

The platform currently provides:

| Utility | Source | Limitation |
|---------|--------|------------|
| `globalTime` | `core_input.ino` | Single time source scaled by p1 (speed). All animation shares one tempo. |
| `normalizedTime(periodMS, phase)` | `core_time.ino` | Sawtooth over a fixed period. Not tied to `globalTime` scaling. |
| `pingPongTime(periodMS, phase)` | `core_time.ino` | Triangle over a fixed period. Same limitation. |

Programs that want different motion characters must manually compute `sin(globalTime * k)`, `fmod()`, or conditional logic. There is no way to have two elements animate at different speeds without hardcoding the math.

### 6.3 What LFOs Add

1. **Multiple independent oscillators.** A pool of LFOs lets different visual elements animate at different frequencies. One circle orbits slowly while another pulses fast — without manual math.

2. **Waveform variety.** Five shapes produce fundamentally different motion:

| Waveform | Character | Use case |
|----------|-----------|----------|
| **sine** | Smooth, organic | Breathing, orbiting, pulsing |
| **triangle** | Linear bounce | Ping-pong motion, scanning |
| **square** | Snappy on/off | Strobe, toggling, hard cuts |
| **saw** | Rising ramp | Scrolling, sweeping, one-directional |
| **ramp** | Falling ramp | Decaying, countdown |

Each is available in unipolar (0–1) and bipolar (-1–1) variants.

3. **Phase offsets.** Multiple elements share one LFO at different phase offsets to create staggered animations (e.g., 5 circles at evenly spaced phases produce a wave pattern).

4. **Decoupled frequency.** Each LFO has its own frequency. The global speed pot (p1) can still act as a tempo multiplier, but individual LFOs can subdivide or multiply that tempo.

### 6.4 Platform Interface

#### Addition to `core_time.ino`

```
#define NUM_LFOS 4

struct LFO {
  uint32_t phase;       // 16.16 fixed-point (wraps at 0x10000)
  uint16_t freq;        // 8.8 fixed-point Hz (0 = stopped, 0x0100 = 1 Hz)
};

LFO g_lfos[NUM_LFOS];

// Called once per frame in main loop (after globalTime update)
void updateLFOs();

// Waveform outputs — all return 0–255 (unipolar) or -128–127 (bipolar)
// Uses integer math with a 256-entry sine LUT for real-time performance
uint8_t lfoSine(int idx, uint8_t phaseOffset);
uint8_t lfoTriangle(int idx, uint8_t phaseOffset);
uint8_t lfoSquare(int idx, uint8_t phaseOffset);
uint8_t lfoSaw(int idx, uint8_t phaseOffset);
uint8_t lfoRamp(int idx, uint8_t phaseOffset);

int8_t lfoSineBi(int idx, uint8_t phaseOffset);
int8_t lfoTriangleBi(int idx, uint8_t phaseOffset);
int8_t lfoSquareBi(int idx, uint8_t phaseOffset);
int8_t lfoSawBi(int idx, uint8_t phaseOffset);
int8_t lfoRampBi(int idx, uint8_t phaseOffset);
```

#### Design decisions

- **Fixed-point, not float.** The legacy LFO uses `float` phase, `micros()` division, `fmod()`, and `sin()`. These are acceptable at one call per frame, but programs may call waveform functions dozens of times per frame (e.g., per-pixel modulation). The platform version uses 16.16 fixed-point phase, integer wrapping, and a 256-entry sine lookup table.

- **Integer output, not float.** Returning `uint8_t` (0–255) / `int8_t` (-128–127) maps directly to palette indices and pixel coordinates without conversion. Programs that need float can cast and scale.

- **Central update, program reads.** `updateLFOs()` runs once per frame in the main loop — same pattern as pot scanning. Programs call waveform functions without worrying about timing. LFO phase advances are tied to `globalTime` scaling so the speed pot (p1) acts as a global tempo control.

- **4 LFOs.** Matches the legacy sketch. Each LFO is 6 bytes — the pool costs 24 bytes total. Programs set frequency via `g_lfos[i].freq` and read waveforms via the accessor functions.

#### New extern in `types.h`

```
#define NUM_LFOS 4
struct LFO { uint32_t phase; uint16_t freq; };
extern LFO g_lfos[NUM_LFOS];
```

#### Usage example

```
// In a program's draw function:
g_lfos[0].freq = potMap(4, 0, 512);   // p4 controls LFO 0 frequency
g_lfos[1].freq = potMap(5, 0, 512);   // p5 controls LFO 1 frequency

int x = HALFW + lfoSineBi(0, 0) * HALFW / 128;
int y = HALFH + lfoTriangleBi(1, 0) * HALFH / 128;
display.fillCircle(x, y, 20, 255);

// Staggered: 5 circles with phase offsets
for (int i = 0; i < 5; i++) {
  int cx = HALFW + lfoSineBi(0, i * 51) * HALFW / 128;  // 51 ≈ 255/5
  display.fillCircle(cx, HALFH, 10, 200);
}
```

### 6.5 Migration

The legacy `LFO` class in `Video_Gem_Comprehensive/lfo.h` should not be copied directly — it uses float math throughout. The platform implementation replaces it with an integer-math equivalent that matches the performance constraints documented in section 2. Programs ported from the Comprehensive sketch should replace `lfos[i].sine()` calls with `lfoSine(i, 0)` and adjust for integer output range (0–255 instead of 0.0–1.0).

---

*Platform contract. Programs are free to implement any visualizer content; the platform does not prescribe taxonomy or prioritization.*
