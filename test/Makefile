# Platform core tests — run against VideoGem/ (no merge)
#
# Coverage uses gcc --coverage + gcovr (pip install gcovr; root Makefile: install-gcovr).
# Aligned with video-gem parent repo: text + HTML + optional Cobertura XML.

CXX        := g++
SKETCH_DIR := $(CURDIR)/../VideoGem
COVERAGE_FLAGS := --coverage
CXXFLAGS   := -std=c++17 -Wall -Wextra -Wpedantic -g -I mocks -I $(SKETCH_DIR) -DUNIT_TEST $(COVERAGE_FLAGS)
LDFLAGS    := -lm $(COVERAGE_FLAGS)

SRCS := $(wildcard test_core_*.cpp)
BINS := $(patsubst %.cpp,build/%,$(SRCS))

# gcovr: repo root (parent of test/)
GCOVR       := gcovr
GCOVR_ROOT  := $(abspath ..)
GCOVR_BASE  := -r $(GCOVR_ROOT)
# Exclude mock headers (regex must match paths like test/mocks/…)
GCOVR_EX    := --exclude 'mocks'
# Optional: totals only for merged VideoGem firmware (excludes test/*.cpp harness)
GCOVR_FW    := --filter '.*/VideoGem/.*'

.PHONY: all clean \
	coverage coverage-all coverage-html coverage-html-all coverage-xml

all: $(BINS)
	@pass=0; fail=0; \
	for t in $(BINS); do \
		printf "\n"; \
		if ./$$t; then pass=$$((pass+1)); else fail=$$((fail+1)); fi; \
	done; \
	printf "\n\033[1m══ platform: %d/%d suites passed ══\033[0m\n\n" $$pass $$(( pass + fail )); \
	[ $$fail -eq 0 ]

build/%: %.cpp test_framework.h $(wildcard mocks/*.h) | build
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

build:
	@mkdir -p build

clean:
	rm -rf build
	rm -f coverage.html coverage-all.html coverage.xml

# Line coverage, VideoGem/*.ino only (primary metric for firmware)
coverage: all
	@$(GCOVR) $(GCOVR_BASE) $(GCOVR_EX) $(GCOVR_FW)

# Include test harness sources in the report
coverage-all: all
	@$(GCOVR) $(GCOVR_BASE) $(GCOVR_EX)

coverage-html: all
	@$(GCOVR) $(GCOVR_BASE) $(GCOVR_EX) $(GCOVR_FW) --html --html-details -o coverage.html
	@echo "Wrote test/coverage.html"

coverage-html-all: all
	@$(GCOVR) $(GCOVR_BASE) $(GCOVR_EX) --html --html-details -o coverage-all.html
	@echo "Wrote test/coverage-all.html"

# CI / Cobertura (e.g. GitHub Actions, Codecov)
coverage-xml: all
	@$(GCOVR) $(GCOVR_BASE) $(GCOVR_EX) $(GCOVR_FW) --xml -o coverage.xml
	@echo "Wrote test/coverage.xml"
