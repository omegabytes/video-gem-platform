# Video Gem Platform — Capabilities & Conformance

Describes the platform's control model, capabilities, and the interface programs must implement. No specific program ideas; the platform is agnostic to visualizer content.

---

## 1. Control Model

**Modifiers:** Chord names below use **Control** for **k14** held and **Shift** for **k15** held (e.g. “Control+k12” means hold k14, then press k12). This is not a PC keyboard—those are physical keys on the 4×4 pad.

**Electrical layout and pins:** See [hardware.md](hardware.md).

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
| **k12** | Modifier A | Program-defined in normal mode. **Control+k12:** toggle FX mode. **Shift+k12** (in FX mode): clear all FX |
| **k13** | Info | **System-level.** Hold = full info overlay. Double-tap = toggle contextual hints |
| **k14** | Control | **Control+Shift+k0–k11:** switch program (load slot 0–11). Used with k12/k15 as in the k12 row |
| **k15** | Shift | **Shift+k0–k11:** presets 12–23. Same program, secondary presets |

### Control vs Shift

- **Shift (k15):** "More of the same" — doubles preset slots. Same program, different modes.
- **Control (k14):** "Change context" — system-level actions. **Control+Shift+k0–k11** = program switch. **Control+k12** = FX mode toggle.

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

## 5. Global FX Layer

**Implementation:** `VideoGem/core_fx.ino`, FX identifiers and state in `VideoGem/types.h`, toggles and FX-mode input in `VideoGem/core_input.ino`, main loop order in `VideoGem/VideoGem.ino`. The default `programs/default/prog_fx.ino` program (slot 11) is a separate **demo** that draws its own base scene; platform FX run in the pipeline for every program.

### 5.1 Overview

FX are a **global platform feature**: composable frame and palette effects layered on any program’s output by the core. Programs stay unaware of FX—no changes required in program code to benefit from them.

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

**Note on echo:** Uses a **subsampled** shadow buffer (~19KB, 160×120 cells covering 2×2 pixel blocks) so the full private sketch (default + custom programs) still leaves enough SRAM for PicoDVI’s double framebuffer (~154KB). A full 320×240 history (~77KB) made `display.begin()` fail once all programs were linked. The blend ratio remains a natural FX-mode pot target.

#### Palette FX (modify the color lookup table)

These use `getColor()` / `setColor()` to transform the 256-entry color palette each frame, creating color-space effects that apply to everything on screen. They run in their own **palette stage** after the platform builds and applies the palette from p0/p3.

| ID | Name | Description | Pipeline stage |
|----|------|-------------|----------------|
| 8 | **invert** | Bitwise-NOT all 256 palette entries (color negative) | Palette |
| 9 | **reverse** | Reverse palette order (swap index i with 255−i) | Palette |
| 10 | **wipe** | Overwrite a sliding window of palette entries with the last color | Palette |
| 11 | **dropouts** | Zero out every non-4th palette entry with a cycling offset | Palette |

### 5.3 UX: FX Mode (Control + k12)

Hold **Control (k14)** and press **k12** to toggle FX mode. That temporarily repurposes the key grid and part of the pot bank for FX control, separate from normal performance mapping.

| Action | Gesture |
|--------|---------|
| Enter/exit FX mode | Control held + k12 (toggle) |
| In FX mode: toggle FX on/off | k0–k11 (one FX per key) |
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
| Multiple simultaneous FX with individual parameters | Control+k12 chord is less discoverable |
| Clean separation: "FX mode" vs "performance mode" | Users must remember to exit FX mode |
| Doesn't break k12, k13, Shift, or preset select | Higher implementation complexity |
| Scalable to many more FX in the future | |

### 5.4 Render pipeline

Order in `VideoGem.ino` (simplified; info paths can early-return before draw):

```
 1. Input scan (keypad + pots)
 2. Program lookup + init on switch
 3. Global parameters (palette, speed, trail, intensity) + applyPalette + applyPaletteFX
 4. updateLFOs()
 5. k13 hold → full info overlay (replaces frame) / hints full-screen card if active
 6. Render hint (RENDER_CLEAR / RENDER_TRAIL / RENDER_PERPIXEL)
 7. applyPreDrawFX() — canvas + framebuffer pre-draw
 8. Program draw
 9. applyPostDrawFX() — framebuffer post-draw
10. drawHints() — bottom bar / corner indicators
11. Framebuffer swap
```

#### Insertion rationale

- **Palette FX:** After `buildPalette()` / `applyPalette()` so drawing uses the modified LUT; before program draw.

- **Pre-draw FX:** After render hint (clear / trail fade / none), before `prog->draw()`. Pre-draw FX add effects on top of that baseline. On a full clear, canvas/pre-draw index effects may show little until the program draws; with trails they compound.

- **Post-draw FX:** After the program, before `drawHints()`, so mirrors apply to the program image while the hint bar stays legible.

### 5.5 Composability

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

### 5.6 Source and design notes

- **API:** `applyPaletteFX()`, `applyPreDrawFX()`, `applyPostDrawFX()`, `fxToggle()`, `fxClearAll()`, `fxIsActive()`, `fxGetName()` — see `core_fx.ino`. FX `FX_*` IDs and `g_fxActive` / `g_fxMode` are in `types.h`.
- **Persistence:** Active FX are intended to persist across preset and program changes until cleared or toggled off.
- **Same-stage order:** Multiple FX in one stage run in ascending ID order; some combinations override or cancel (see tables above).
- **Echo:** Uses a full-frame shadow buffer for feedback; SRAM is tight on RP2040 with double-buffered video—see implementation in `core_fx.ino`.

---

## 6. LFO utility

**Implementation:** `VideoGem/core_time.ino` (`updateLFOs()`, `lfoSine` / `lfoTriangle` / … and bipolar variants). Pool and struct are declared in `types.h` (`NUM_LFOS`, `g_lfos[]`).

Unlike the global FX layer (section 5), LFOs are **program-facing**: optional building blocks for motion. The main loop still provides `globalTime`, `normalizedTime`, and `pingPongTime`; LFOs add **multiple independent oscillators** with sine, triangle, square, saw, and ramp waveforms (unipolar 0–255 and bipolar −128…127), fixed-point phase, and integer math suited to tight draw loops.

Programs set each oscillator’s rate via `g_lfos[i].freq` and read values with the `lfo*` accessors after `updateLFOs()` has run for the frame (called from `VideoGem.ino` after palette/FX palette stage).

---

*Platform contract. Programs are free to implement any visualizer content; the platform does not prescribe taxonomy or prioritization.*
