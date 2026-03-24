# Video Gem Platform

Open firmware for a **real-time video synthesizer** on the **Adafruit Feather RP2040 DVI**: generative visuals over HDMI/DVI, with sixteen potentiometers and a 4×4 keypad.

The repo is the **platform core**—input, palette, drawing primitives, global FX, LFOs, info overlay, and table-driven program routing—plus default programs you can replace or extend.

## Attribution

- **Hardware** — Adafruit Feather RP2040 DVI and related design by [Adafruit](https://www.adafruit.com/).
- **Default programs** (Video_Gem_Basic_Shapes, Video_Gem_Color, Video_Gem_Inputs, Video_Gem_Symmetry) — Original programs by **Ramona Sharples (RMNA)** (https://ramona.diy/) for the Gray Area "[Programming Lo-fi Hardware Video Synthesizers](https://grayarea.org/course/programming-lo-fi-hardware-video-synthesizers/)" workshop, adapted from Adafruit tutorials.

Ships with **default programs** in `programs/default/` (Basic Shapes, Symmetry, Color Lab, Inputs, Bitmaps, FX demo, and related assets). `make compile` merges `VideoGem/` with those files into `build/VideoGem/` before building.

## Requirements

- **macOS** — `make setup` uses Homebrew for Arduino CLI and tools (simplest path).
- **Linux / Windows** — Install [Arduino CLI](https://arduino.cc/cli) and use the same `arduino-cli` core/library/compile steps as in the root `Makefile` (RP2040 core, PicoDVI, Adafruit Keypad); adjust install commands and serial port paths.
- **Board:** Adafruit Feather RP2040 DVI

## Getting Started

```bash
make setup      # one-time: Arduino CLI, RP2040 core, libraries
make compile    # build the sketch
make test       # run unit tests
make list-boards
ARDUINO_PORT=/dev/cu.usbmodem101 make upload
ARDUINO_PORT=/dev/cu.usbmodem101 make monitor
```

## Make Targets

| Target | Description |
|--------|-------------|
| `make setup` | One-time: Arduino CLI, RP2040 core, libraries |
| `make compile` | Build the sketch |
| `make test` | Run unit tests |
| `make lint` | arduino-lint + cppcheck (`make install-tools` first) |
| `make list-boards` | List connected boards |
| `ARDUINO_PORT=<port> make upload` | Upload to board |
| `ARDUINO_PORT=<port> make monitor` | Serial monitor (115200 baud) |
| `make list-video-devices` | List capture devices (macOS) |
| `VIDEO_DEVICE=0 make preview` | Preview output via capture card |

## Contents

- `VideoGem/` — Core sketch: input, palette, draw, time/LFO, FX, info overlay, registry helpers; includes a zero-program `prog_registry.ino` stub until merge
- `programs/default/` — Default `prog_*.ino` sources and the registry that registers them
- `test/` — Host unit tests (`make test`)
- `docs/` — [Hardware](docs/hardware.md), [control model and platform contract](docs/VISUALIZER_PRODUCT_FRAMEWORK.md)

## Extending

- Implement a program as `prog_<name>.ino` with the callbacks the platform expects (`draw`, `renderHint`, `name`, `character`, `presetName`, `potLabel`, optional `init`).
- Register slots in `prog_registry.ino` with `PROG_ENTRY` / `PROG_ENTRY_EX` (see `programs/default/prog_registry.ino`).
- Merge your `.ino` files into the same sketch folder as the core modules—the default layout is what `make compile` produces from `VideoGem/` + `programs/default/`.
- Ship your own set by forking and changing `programs/default/`, or by copying your registry and program files over the merged sketch before compiling.
