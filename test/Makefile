# Platform core tests — run against platform/VideoGem (no merge)

CXX        := g++
SKETCH_DIR := $(CURDIR)/../VideoGem
# Enable gcov coverage info for host-based tests
COVERAGE_FLAGS := --coverage
CXXFLAGS   := -std=c++17 -Wall -Wextra -Wpedantic -g -I mocks -I $(SKETCH_DIR) -DUNIT_TEST $(COVERAGE_FLAGS)
LDFLAGS    := -lm $(COVERAGE_FLAGS)

SRCS := $(wildcard test_core_*.cpp)
BINS := $(patsubst %.cpp,build/%,$(SRCS))

.PHONY: all clean coverage coverage-html

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

# Coverage reports (requires gcovr to be installed)
coverage: all
	@gcovr -r .. --exclude '.*/test/.*' --exclude '.*/mocks/.*'

coverage-html: all
	@gcovr -r .. --exclude '.*/test/.*' --exclude '.*/mocks/.*' --html --html-details -o coverage.html
