# Video Gem Platform

Open firmware for a **real-time video synthesizer** on the **Adafruit Feather RP2040 DVI**: generative visuals over HDMI/DVI, with sixteen potentiometers and a 4×4 keypad.

The repo is the **platform core**—input, palette, drawing primitives, global FX, LFOs, info overlay, and table-driven program routing—plus default programs you can replace or extend.

## Attribution

- **Hardware** — Adafruit Feather RP2040 DVI and related design by [Adafruit](https://www.adafruit.com/).
- **Default programs** (Video_Gem_Basic_Shapes, Video_Gem_Color, Video_Gem_Inputs, Video_Gem_Symmetry) — Original programs by **Ramona Sharples (RMNA)** (https://ramona.diy/) for the Gray Area "[Programming Lo-fi Hardware Video Synthesizers](https://grayarea.org/course/programming-lo-fi-hardware-video-synthesizers/)" workshop, adapted from Adafruit tutorials.

Ships with **default programs** in `programs/default/` (Basic Shapes, Symmetry, Color Lab, Inputs, Bitmaps, FX demo, and related assets). `make compile` merges `VideoGem/` with `programs/default/*.ino` and any `programs/default/*.h` (e.g. `bitmaps.h`) into `build/VideoGem/` before building.

## Requirements

- **macOS** — `make setup` uses Homebrew for Arduino CLI and tools (simplest path).
- **Linux / Windows** — Install [Arduino CLI](https://arduino.cc/cli) and use the same `arduino-cli` core/library/compile steps as in the root `Makefile` (RP2040 core, PicoDVI, Adafruit Keypad); adjust install commands and serial port paths.
- **Board:** Adafruit Feather RP2040 DVI

## Getting Started

```bash
make setup      # one-time: Arduino CLI, RP2040 core, libraries, gcovr (for coverage)
make compile    # build the sketch
make test       # run unit tests
make coverage   # line coverage for VideoGem/ (after tests; needs gcovr)
make set-board  # choose USB serial port → saves gitignored local.mk
make upload
make monitor
make set-video  # macOS: choose capture card index → local.mk
make preview
```

`local.mk` stores `ARDUINO_PORT` and `VIDEO_DEVICE` so you do not pass them every time. Override for one command with `make upload ARDUINO_PORT=/dev/other`.

## Make Targets

| Target | Description |
|--------|-------------|
| `make setup` | One-time: Arduino CLI, RP2040 core, libraries |
| `make compile` | Build the sketch |
| `make test` | Run unit tests |
| `make coverage` | Line coverage for `VideoGem/*.ino` (gcc `--coverage` + [gcovr](https://gcovr.com/en/stable/)) |
| `make coverage-all` | Same report, includes `test/*.cpp` harness |
| `make coverage-html` | Detailed HTML report → `test/coverage.html` |
| `make coverage-html-all` | HTML including test sources → `test/coverage-all.html` |
| `make coverage-xml` | Cobertura XML → `test/coverage.xml` (CI) |
| `make lint` | arduino-lint + cppcheck (`make install-tools` first) |
| `make list-boards` | List connected boards (same info `set-board` uses) |
| `make set-board` | Interactive serial port → writes `local.mk` (default `/dev/cu.usbmodem101` on macOS when plausible) |
| `make upload` | Upload (needs `ARDUINO_PORT` in env, `local.mk`, or `make upload ARDUINO_PORT=…`) |
| `make monitor` | Serial monitor at 115200 baud |
| `make list-video-devices` | List capture devices (macOS / ffmpeg) |
| `make set-video` | **macOS:** interactive capture device → `local.mk` |
| `make preview` | Live preview via ffmpeg/ffplay (`VIDEO_DEVICE` in `local.mk` or env) |

## Contents

- `VideoGem/` — Core sketch: input, palette, draw, time/LFO, FX, info overlay, registry helpers; includes a zero-program `prog_registry.ino` stub until merge
- `programs/default/` — Default `prog_*.ino` sources, headers they include (e.g. `bitmaps.h`), and the registry
- `test/` — Host unit tests (`make test`); coverage via `make coverage` / `make coverage-html` (see table)
- `docs/` — [Hardware](docs/hardware.md), [control model and platform contract](docs/VISUALIZER_PRODUCT_FRAMEWORK.md)

## Extending

- Implement a program as `prog_<name>.ino` with the callbacks the platform expects (`draw`, `renderHint`, `name`, `character`, `presetName`, `potLabel`, optional `init`).
- Register slots in `prog_registry.ino` with `PROG_ENTRY` / `PROG_ENTRY_EX` (see `programs/default/prog_registry.ino`).
- Merge your `.ino` files into the same sketch folder as the core modules—the default layout is what `make compile` produces from `VideoGem/` + `programs/default/`.
- Ship your own set by forking and changing `programs/default/`, or by copying your registry and program files over the merged sketch before compiling.
