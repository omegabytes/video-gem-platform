# Video Gem Platform

Core library for the Video Gem video synthesizer: input handling, palette, drawing primitives, info overlay, and table-driven program routing. Targets the **Adafruit Feather RP2040 DVI** with 16 potentiometers and a 4×4 keypad.

## Attribution

- **Hardware** — Adafruit Feather RP2040 DVI and related design by [Adafruit](https://www.adafruit.com/).
- **Default programs** (Video_Gem_Basic_Shapes, Video_Gem_Color, Video_Gem_Inputs, Video_Gem_Symmetry) — Original programs by **Ramona Sharples (RMNA)** (https://ramona.diy/) for the Gray Area "[Programming Lo-fi Hardware Video Synthesizers](https://grayarea.org/course/programming-lo-fi-hardware-video-synthesizers/)" workshop, adapted from Adafruit tutorials.

Ships with **default programs** (Basic Shapes, Color Lab, Inputs, Symmetry). Add more via a parent repo and merge script.

## Requirements

- **macOS** (Makefile uses `brew`; adapt for Linux/Windows)
- **Arduino CLI** — installed by `make setup`
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

- `VideoGem/` — Arduino sketch (core modules + prog_registry)
- `programs/default/` — Default programs (Basic Shapes, Color Lab, Inputs, Symmetry)
- `test/` — Unit tests (`make test`)
- `docs/` — [hardware](docs/hardware.md), [platform capabilities](docs/VISUALIZER_PRODUCT_FRAMEWORK.md)

## Extending

To add programs: use a parent repo with `programs/` (prog_*.ino + prog_registry.ino) and a merge script. See your parent repo's submodule docs.
