# Video Gem Platform
#
# Clone this repo and run from the platform root:
#   make setup      # one-time: Arduino CLI, RP2040 core, libraries
#   make compile    # build the sketch
#   make test       # run unit tests
#   make coverage   # line coverage for VideoGem/ (needs gcovr: make install-gcovr)
#   make coverage-html   # HTML report → test/coverage.html
#   make lint       # arduino-lint + cppcheck (needs: make install-tools)
#   make list-boards
#   ARDUINO_PORT=/dev/cu.usbmodem101 make upload
#   ARDUINO_PORT=/dev/cu.usbmodem101 make monitor
#

BINDIR      := $(CURDIR)/bin
SKETCH_DIR  := $(CURDIR)/build/VideoGem
VIDEOGEM    := $(CURDIR)/VideoGem
DEFAULT_PROGS := $(CURDIR)/programs/default
FQBN        := rp2040:rp2040:adafruit_feather_dvi
RP2040_URL  := https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json

.PHONY: setup install-arduino-cli setup-arduino install-gcovr merge compile test \
	coverage coverage-all coverage-html coverage-html-all coverage-xml
.PHONY: upload list-boards monitor
.PHONY: list-video-devices preview
.PHONY: install-tools arduino-lint cppcheck lint

# ─── Merge VideoGem + default programs ─────────────────────────────────
merge:
	@mkdir -p $(SKETCH_DIR)
	@rm -rf $(SKETCH_DIR)/*
	cp -r $(VIDEOGEM)/* $(SKETCH_DIR)/
	cp $(DEFAULT_PROGS)/*.ino $(SKETCH_DIR)/
	@echo "Merged sketch → $(SKETCH_DIR)"

# ─── One-time setup ─────────────────────────────────────────────────────
setup: install-arduino-cli setup-arduino install-gcovr

install-arduino-cli:
	@command -v arduino-cli >/dev/null 2>&1 || { \
		echo "Installing Arduino CLI (brew)..."; \
		brew install arduino-cli; \
	}
	@arduino-cli version

setup-arduino: install-arduino-cli
	@echo "Adding RP2040 board index..."
	@arduino-cli config add board_manager.additional_urls $(RP2040_URL) 2>/dev/null || true
	@arduino-cli core update-index
	@echo "Installing RP2040 core..."
	@arduino-cli core install rp2040:rp2040
	@echo "Installing libraries..."
	@arduino-cli lib install "PicoDVI - Adafruit Fork"
	@arduino-cli lib install "Adafruit Keypad"
	@echo "Setup done. Run: make compile"

install-gcovr:
	@command -v gcovr >/dev/null 2>&1 || { \
		echo "Installing gcovr (pip)..."; \
		pip3 install gcovr; \
	}

# ─── Build & test ───────────────────────────────────────────────────────
compile: merge
	arduino-cli compile -b $(FQBN) $(SKETCH_DIR)

test:
	$(MAKE) -C test all

coverage:
	$(MAKE) -C test coverage

coverage-all:
	$(MAKE) -C test coverage-all

coverage-html:
	$(MAKE) -C test coverage-html

coverage-html-all:
	$(MAKE) -C test coverage-html-all

coverage-xml:
	$(MAKE) -C test coverage-xml

# ─── Upload & monitor ───────────────────────────────────────────────────
upload:
	@if [ -z "$$ARDUINO_PORT" ]; then \
		echo "Set ARDUINO_PORT. Run: make list-boards"; \
		arduino-cli board list; \
		exit 1; \
	fi
	arduino-cli upload -b $(FQBN) -p $$ARDUINO_PORT $(SKETCH_DIR)

list-boards:
	arduino-cli board list

monitor:
	@if [ -z "$$ARDUINO_PORT" ]; then \
		echo "Set ARDUINO_PORT. Run: make list-boards"; \
		exit 1; \
	fi
	arduino-cli monitor -p $$ARDUINO_PORT -c baudrate=115200

# ─── Video preview (macOS, optional) ────────────────────────────────────
list-video-devices:
	@ffmpeg -hide_banner -f avfoundation -list_devices true -i "" 2>&1 || true

preview:
	@if [ -z "$$VIDEO_DEVICE" ]; then \
		echo "Set VIDEO_DEVICE. Run: make list-video-devices"; \
		exit 1; \
	fi
	ffplay -hide_banner -f avfoundation -framerate 60 -video_size 640x480 -i "$$VIDEO_DEVICE:none"

# ─── Linting ────────────────────────────────────────────────────────────
install-tools:
	@command -v cppcheck >/dev/null 2>&1 || brew install cppcheck
	@mkdir -p $(BINDIR)
	@command -v arduino-lint >/dev/null 2>&1 || { \
		echo "Installing arduino-lint to $(BINDIR)..."; \
		curl -fsSL https://raw.githubusercontent.com/arduino/arduino-lint/main/etc/install.sh | BINDIR=$(BINDIR) sh; \
		echo "Add $(BINDIR) to PATH, or: PATH=\"$(BINDIR):$$PATH make arduino-lint\""; \
	}

arduino-lint: merge install-tools
	@PATH="$(BINDIR):$$PATH" arduino-lint $(SKETCH_DIR)

cppcheck: merge
	cppcheck --enable=warning,style,performance --inline-suppr -q $(SKETCH_DIR)/*.ino

lint: arduino-lint cppcheck
