# Video Gem — Hardware Specifications

Complete hardware reference for the Video Gem platform.

---

## Attribution

- **Hardware** — Adafruit Feather RP2040 DVI; pin layout and control design by Adafruit. See [Adafruit Feather RP2040 DVI](https://learn.adafruit.com/adafruit-feather-rp2040-dvi).
- **Video_Gem\*** default programs — Ramona Sharples (RMNA) for Gray Area workshop.

---

## Overview

The Video Gem is an **Arduino-based video synthesizer** built around the **Adafruit Feather RP2040 DVI**. It outputs real-time generative visuals to HDMI/DVI displays and accepts control from 16 potentiometers and a 4×4 keypad.

---

## Main Board: Adafruit Feather RP2040 DVI

| Spec | Value |
|------|-------|
| **MCU** | Raspberry Pi RP2040 (Cortex-M0+ dual-core, 133 MHz) |
| **Flash** | QSPI Flash (on-board, not user-accessible for storage) |
| **Logic** | 3.3V |
| **Power** | USB-C or 2-pin JST PH LiPoly battery (with charging) |
| **3.3V regulator** | 500 mA peak output |
| **Video output** | DVI (compatible with HDMI via adapter) |

**Reference:** [Adafruit Feather RP2040 DVI](https://learn.adafruit.com/adafruit-feather-rp2040-dvi)

---

## Display Output

| Spec | Value |
|------|-------|
| **Interface** | DVI (digital video) |
| **Resolution** | 320 × 240 pixels |
| **Frame rate** | 60 fps |
| **Color depth** | 8-bit indexed (256 colors) |
| **Buffering** | Double-buffered (flicker-free) |
| **Library** | PicoDVI + Adafruit_GFX |
| **Mode** | `DVI_RES_320x240p60` |
| **Config** | `adafruit_feather_dvi_cfg` |

DVI uses 8 pins: CKP/CKN, D0P/D0N, D1P/D1N, D2P/D2N (dedicated to video, not broken out for GPIO).

---

## Analog Inputs (Potentiometers)

**16 potentiometers** via 4 analog inputs and a 2-line multiplexer (4×4 = 16 channels).

### Potentiometer physical layout

Pots are arranged in three rows: 8 on top, 4 on the right of row 2, 4 on the right of row 3.

```
Pots:  0  1  2  3  4  5  6  7
                   8  9  10 11
                   12 13 14 15
```

### Analog pins

| Pin | Arduino | GPIO | ADC | Notes |
|-----|---------|------|-----|-------|
| POT_A | A0 | 26 | ADC0 | Mux channel 0–3 |
| POT_B | A1 | 27 | ADC1 | Mux channel 0–3 |
| POT_C | A2 | 28 | ADC2 | Mux channel 0–3 |
| POT_D | A3 | 29 | ADC3 | Mux channel 0–3 |

### Multiplexer control

| Pin | Arduino | GPIO | Purpose |
|-----|---------|------|---------|
| MUX_CTRL_0 | 24 | GPIO24 | Mux channel select bit 0 |
| MUX_CTRL_1 | 25 | GPIO25 | Mux channel select bit 1 |

**Mux channel select:** `digitalWrite(MUX_CTRL_0, channel & 1); digitalWrite(MUX_CTRL_1, channel & 2);`

| Channel | CTRL_1 | CTRL_0 | pots[] indices |
|---------|--------|--------|----------------|
| 0 | 0 | 0 | 0, 4, 8, 12 |
| 1 | 0 | 1 | 1, 5, 9, 13 |
| 2 | 1 | 0 | 2, 6, 10, 14 |
| 3 | 1 | 1 | 3, 7, 11, 15 |

### Pot value range

- Raw: 0–1023 (10-bit ADC)
- Inverted in software: `pots[i] = 1023 - analogRead(...)` (handles inverted wiring)
- Typical sampling: 20 samples averaged, ~16 ms update interval

---

## Keypad (4×4 Matrix)

**16 keys** arranged in a 4×4 matrix, scanned via `Adafruit_Keypad`.

### Row pins (outputs)

| Row | Arduino | GPIO |
|-----|---------|------|
| 0 | 13 | GPIO13 |
| 1 | 12 | GPIO12 |
| 2 | 11 | GPIO11 |
| 3 | 0 | GPIO0 (TX) |

### Column pins (inputs)

| Col | Arduino | GPIO |
|-----|---------|------|
| 0 | 10 | GPIO10 |
| 1 | 9 | GPIO9 |
| 2 | 6 | GPIO6 |
| 3 | 5 | GPIO5 |

### Key physical layout

Keys are arranged in a 4×4 matrix. Rows 0–1 and cols 0–3 form the left 4×2 block; rows 2–3 span the full width.

```
Keys:  0  1  2  3
       4  5  6  7
       8  9 10 11 12 13 14 15
```

---

## Physical Control Layout

Keys and pots share a **4×8 grid**. Top row is all pots; bottom row is all keys; middle rows mix keys (left) and pots (right).

```
        ┌────┬────┬────┬────┬────┬────┬────┬────┐
        │ p0 │ p1 │ p2 │ p3 │ p4 │ p5 │ p6 │ p7 │  ← Row 0: pots 0–7
        ├────┼────┼────┼────┼────┼────┼────┼────┤
        │ k0 │ k1 │ k2 │ k3 │ p8 │ p9 │p10 │p11 │  ← Row 1: keys 0–3, pots 8–11
        ├────┼────┼────┼────┼────┼────┼────┼────┤
        │ k4 │ k5 │ k6 │ k7 │p12 │p13 │p14 │p15 │  ← Row 2: keys 4–7, pots 12–15
        ├────┼────┼────┼────┼────┼────┼────┼────┤
        │ k8 │ k9 │k10 │k11 │k12 │k13 │k14 │k15 │  ← Row 3: keys 8–15
        └────┴────┴────┴────┴────┴────┴────┴────┘
```

Compact form:

```
p0  p1  p2  p3  p4  p5  p6  p7
k0  k1  k2  k3  p8  p9 p10 p11
k4  k5  k6  k7 p12 p13 p14 p15
k8  k9 k10 k11 k12 k13 k14 k15
```

---

## Pin Assignment Summary

| Function | Pins used |
|----------|-----------|
| DVI video | CKP, CKN, D0P, D0N, D1P, D1N, D2P, D2N (internal) |
| Pot analog | A0, A1, A2, A3 (GPIO26–29) |
| Mux control | 24, 25 (GPIO24, GPIO25) |
| Keypad rows | 0, 11, 12, 13 (GPIO0, 11, 12, 13) |
| Keypad cols | 5, 6, 9, 10 (GPIO5, 6, 9, 10) |

**Note:** DVI uses dedicated hardware pins. Analog and digital pins above remain available for I/O except where shared with keypad.

---

## Software Dependencies

| Library | Purpose |
|---------|---------|
| **PicoDVI** | DVI output, framebuffer, sync |
| **Adafruit_GFX** | Drawing (setPixel, drawLine, fillCircle, etc.) — v1.11.4+ |
| **Adafruit_Keypad** | 4×4 matrix scanning |
| **Arduino.h** | Core Arduino API |
| **math.h** | sinf, cosf, etc. |

---

## Performance Notes

- **Target:** 30–60 fps (16.6 ms per frame at 60 fps)
- **Display:** Double-buffered 320×240×8-bit ≈ 76,800 bytes per buffer
- **RP2040:** No hardware FPU (Cortex-M0+); floating-point in software
- **Optimization:** Prefer integer/fixed-point math in tight loops; use lookup tables for trig when possible

---

## Power

- **3.3V:** Logic and peripherals
- **5V:** Not exposed (board regulates from USB/battery to 3.3V)
- **USB-C:** Power and programming
- **LiPoly:** Optional via JST PH (charge supported over USB)

---

## On-Board Indicators

| Component | Pin | Notes |
|-----------|-----|-------|
| NeoPixel | GPIO4 | RGB status LED |
| Red LED | D13 / GPIO13 | **Shared with keypad row 0** — may affect keypad if used |
| Boot button | — | Hold on power-up for bootloader |

---

## Connectors

- **USB-C:** Power, programming, bootloader
- **DVI:** Video output (HDMI-compatible with passive adapter)
- **STEMMA QT:** I2C (SCL/SDA) for sensors
- **Battery:** 2-pin JST PH

---

## Troubleshooting: DVI does not start (`display.begin()` fails)

If USB serial shows **“DVI init failure loop”** and the status LED blinks about **twice per second**, the firmware is stuck because **PicoDVI never initialized**. That is almost always **hardware or board selection**, not the sketch registry.

1. **Board identity** — Firmware is built for **Adafruit Feather RP2040 DVI** (`adafruit_feather_dvi`). The plain **Feather RP2040** (no DVI socket) uses different pins; `display.begin()` will **fail** on that board. Confirm the silkscreen / product page matches the DVI variant.

2. **HDMI / DVI** — Use the **DVI add-on for this Feather** (or the correct Adafruit DVI wing), cable fully seated, then **USB power**. Try a **direct** USB port (not a marginal hub). Some monitors need **HDMI first**, then power; try both orders.

3. **Libraries / core** — Use **Earle Philhower’s RP2040 core** and **“PicoDVI - Adafruit Fork”** as in the project `Makefile` (`make setup`). Mismatched library vs core versions can break init.

4. **Serial capture** — Open the monitor **before** reset (`make monitor`, then tap **RESET**) so you do not miss the first lines (`display.begin() FAILED`, etc.).

5. **Static RAM vs. private merge** — The private repo links **extra** `prog_*.ino` files with large `.bss` (simulation buffers, particles, etc.). If **global variables** reported by `arduino-cli compile` are near **50%** of “dynamic memory” (~131KB+) while the platform-only build is ~35% (~94KB), PicoDVI may not get enough SRAM for its framebuffers and **`display.begin()` fails** even on correct hardware. The platform mitigates this (e.g. subsampled echo FX buffer); if you add more huge static arrays, trim counts or share storage.

For authoritative pin and electrical specifications, use [Adafruit’s Feather RP2040 DVI guide](https://learn.adafruit.com/adafruit-feather-rp2040-dvi).
