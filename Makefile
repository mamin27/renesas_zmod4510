# Build libzmod4510.so and no2_o3-example without CMake.
# Same sources and flags as CMakeLists.txt. Steps and warnings are in
# doc/skills/build-manual.md (section "Choosing the compiler").

# ---- Compiler: change this number to change the gcc version -----------------
# The compiler is gcc-$(GCC_VERSION). Edit the number below, or give it per call
# ("make GCC_VERSION=6") or in the environment. Precedence: command line, then
# environment, then this line. "make GCC_VERSION=" (empty) uses the plain "gcc".
# CC=... on the command line or in the environment replaces gcc-$(GCC_VERSION).
GCC_VERSION ?= 14
# -----------------------------------------------------------------------------

## make                    library + example, linked with lib/lib_*.a (CMake default)
## make USE_REIMPL=1       library + example built from src/reimpl instead
## make GCC_VERSION=6      compile with gcc-6 (the default number is set in this file)
## make CC=/path/to/gcc    any other compiler, replaces GCC_VERSION
## make check              no-I2C checks of the build, shows the compiler of the binary
## make run                start the example on the real sensor (read the notice)
## make compare            compare src/reimpl with the archives (mock only)
## make clean              remove the output of the current compiler
## make clean-all          remove the output of every compiler
## make help               this text and the compiler that would be used
#
# Output goes to $(BUILD_DIR)/<compiler>/archive or .../reimpl. Every compiler
# and variant has its own directory: make does not notice a changed compiler,
# so a shared directory would silently keep the objects of the old one. The
# CMake directory build/ is not touched. Configure CMake out of source
# ("cmake -S . -B build", see doc/skills/build-sample.md), never with
# "cmake ." in this directory: an in-source CMake build writes its own
# Makefile here.

THIS_MAKEFILE := $(lastword $(MAKEFILE_LIST))
ROOT := $(abspath $(dir $(THIS_MAKEFILE)))

# GCC_VERSION is pasted into a command name, so it may hold digits and dots only.
GCC_VERSION_JUNK := $(subst .,,$(GCC_VERSION))
$(foreach d,0 1 2 3 4 5 6 7 8 9,$(eval GCC_VERSION_JUNK := $$(subst $(d),,$$(GCC_VERSION_JUNK))))
ifneq ($(strip $(GCC_VERSION_JUNK)),)
$(error GCC_VERSION must contain digits and dots only (for example 14 or 6), got "$(GCC_VERSION)")
endif
ifneq ($(words $(GCC_VERSION)),$(if $(strip $(GCC_VERSION)),1,0))
$(error GCC_VERSION must be a single number (for example 14 or 6), got "$(GCC_VERSION)")
endif

# make defines CC as "cc" by default. Replace that default with gcc-N. A CC given
# by the user (command line or environment) is kept and GCC_VERSION is not used.
ifeq ($(origin CC),default)
CC := gcc$(if $(strip $(GCC_VERSION)),-$(strip $(GCC_VERSION)))
CC_SOURCE := GCC_VERSION=$(GCC_VERSION)
else
CC_SOURCE := CC set by the user, GCC_VERSION is ignored
ifeq ($(origin GCC_VERSION),command line)
$(warning CC is set, so GCC_VERSION=$(GCC_VERSION) is ignored)
endif
endif

ifeq ($(strip $(CC)),)
$(error CC must not be empty)
endif

# Directory name for this compiler: CC with "/" and blanks replaced.
empty :=
space := $(empty) $(empty)
TAG := $(subst /,_,$(subst $(space),_,$(strip $(CC))))

# Only looked up when needed (recursive variables).
CC_PATH       = $(shell command -v '$(firstword $(CC))' 2>/dev/null)
INSTALLED_GCC = $(notdir $(wildcard /usr/bin/gcc-[0-9]*))

# Stop early, with the list of installed versions, when the compiler is missing.
NEEDS_CC := $(filter-out help clean clean-all,$(or $(MAKECMDGOALS),all))
ifneq ($(NEEDS_CC),)
ifeq ($(CC_PATH),)
$(error Compiler "$(CC)" not found in PATH ($(CC_SOURCE)). Installed in /usr/bin: $(or $(INSTALLED_GCC),none). Try "make GCC_VERSION=<n>" or "make CC=/path/to/gcc")
endif
endif

CFLAGS     ?= -O3 -DNDEBUG
LDFLAGS    ?=
USE_REIMPL ?= 0
BUILD_DIR  ?= $(ROOT)/build-make

ifneq ($(filter-out 0 1,$(USE_REIMPL)),)
$(error USE_REIMPL must be 0 or 1, got "$(USE_REIMPL)")
endif

ifeq ($(USE_REIMPL),1)
VARIANT := reimpl
else
VARIANT := archive
endif

BUILD := $(abspath $(BUILD_DIR))
OUT   := $(BUILD)/$(TAG)/$(VARIANT)
# Marks a directory as made by this Makefile. clean only removes marked ones.
STAMP := $(BUILD)/$(TAG)/.zmod4510-make

INC := -I$(ROOT)/src -I$(ROOT)/src/algos -I$(ROOT)/src/sensors \
       -I$(ROOT)/src/hal -I$(ROOT)/src/reimpl

DRIVER_SRCS := src/sensors/zmod4xxx.c src/hal/zmod4xxx_hal.c src/hal/hal.c \
               src/hal/raspi/rpi.c src/sensor_interface.c
REIMPL_SRCS := src/reimpl/no2_o3.c src/reimpl/no2_o3_nnet.c \
               src/reimpl/no2_o3_data.c src/reimpl/no2_o3_nnet_data.c \
               src/reimpl/zmod4xxx_cleaning.c
ARCHIVES    := $(ROOT)/lib/lib_no2_o3.a $(ROOT)/lib/lib_zmod4xxx_cleaning.a

DRIVER_OBJS := $(DRIVER_SRCS:%.c=$(OUT)/obj/%.o)
REIMPL_OBJS := $(REIMPL_SRCS:%.c=$(OUT)/obj/%.o)

ifeq ($(USE_REIMPL),1)
LIB_OBJS := $(DRIVER_OBJS) $(REIMPL_OBJS)
LIB_DEPS :=
LIB_LINK :=
else
LIB_OBJS := $(DRIVER_OBJS)
LIB_DEPS := $(ARCHIVES)
LIB_LINK := $(ARCHIVES)
endif

LIB := $(OUT)/libzmod4510.so
EXE := $(OUT)/no2_o3-example

.DEFAULT_GOAL := all
.PHONY: all lib example check run compare clean clean-all help
.DELETE_ON_ERROR:

all: $(LIB) $(EXE)
lib: $(LIB)
example: $(EXE)

# Keep the evaluation order of the original. No extra FMA contraction.
$(REIMPL_OBJS): FPFLAGS := -ffp-contract=off

$(STAMP):
	@mkdir -p $(@D) && touch $@

$(OUT)/obj/%.o: $(ROOT)/%.c | $(STAMP)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -std=gnu99 -fPIC $(INC) $(FPFLAGS) -MMD -MP -c $< -o $@

$(ARCHIVES):
	@echo "Missing $@. The archives are not in git (see .gitignore)." >&2
	@echo "Use USE_REIMPL=1 or put the two files into lib/." >&2
	@exit 1

# A missing archive stops the link with a clear message. With a serial make
# that happens before anything is compiled; with -j the compile may already run.
$(LIB): $(LIB_DEPS) $(LIB_OBJS)
	$(CC) -shared -Wl,-soname,libzmod4510.so $(LDFLAGS) -o $@ $(LIB_OBJS) $(LIB_LINK) -lm

# The rpath is the absolute output directory, so no LD_LIBRARY_PATH is needed.
$(EXE): $(ROOT)/src/main.c $(LIB)
	$(CC) $(CFLAGS) -std=gnu99 -I$(ROOT)/src/algos -I$(ROOT)/src/sensors \
	    -MMD -MP -MF $(OUT)/obj/main.d $(LDFLAGS) $< -o $@ \
	    -L$(OUT) -Wl,-rpath,$(OUT) -lzmod4510

-include $(DRIVER_OBJS:.o=.d) $(REIMPL_OBJS:.o=.d) $(OUT)/obj/main.d

# Never opens an I2C bus: the smoke run uses an empty HOME, so sensor_init()
# fails on the missing ~/.comet/config.yaml and the example exits with 1.
check: all
	@set -e; \
	ldd $(EXE) | grep -F "$(LIB)" >/dev/null || { echo "FAIL: $(EXE) does not load $(LIB)"; exit 1; }; \
	for s in init_no2_o3 calc_no2_o3 zmod4xxx_cleaning_run sensor_init; do \
	  nm -D --defined-only $(LIB) | grep -Eq " T $$s\$$" || { echo "FAIL: $$s is not exported by $(LIB)"; exit 1; }; \
	done; \
	mkdir -p $(OUT)/smoke-home; \
	rc=0; HOME=$(OUT)/smoke-home $(EXE) >$(OUT)/smoke.out 2>&1 || rc=$$?; \
	if [ $$rc -ne 1 ]; then echo "FAIL: smoke run exited with $$rc, expected 1"; cat $(OUT)/smoke.out; exit 1; fi; \
	cc_line=$$(readelf -p .comment $(EXE) 2>/dev/null | grep -m1 'GCC:' | sed 's/^ *\[ *[0-9a-f]*\] *//'); \
	echo "OK: $(VARIANT) variant, compiler $(CC), the binary says: $${cc_line:-no GCC string in .comment}"; \
	echo "    library loads, symbols exported, smoke run exited 1 as expected"

# Talks to the real sensor. The first start on a module that was never cleaned
# runs the one-time cleaning, which writes to its non-volatile memory.
run: all
	@echo "Starting $(EXE) ($(VARIANT) variant) on the real sensor." >&2
	@echo "  - Bus: i2c.smb from $$HOME/.comet/config.yaml. Stop with Ctrl+C." >&2
	@echo "  - A module that was never cleaned gets its one lifetime cleaning now." >&2
ifeq ($(USE_REIMPL),1)
	@echo "  - The cleaning here is the C version from src/reimpl, never tried on a real module." >&2
	@echo "    If the module may be uncleaned, use 'make run USE_REIMPL=0' for the first start." >&2
endif
	$(EXE)

# Mock input and mock I2C bus only. Needs lib/lib_*.a and writes to /tmp.
compare:
	CC="$(CC)" sh $(ROOT)/test_script/reimpl_compare.sh
	CC="$(CC)" sh $(ROOT)/test_script/reimpl_compare_cleaning.sh

# Removes only directories that carry the marker file, never $(BUILD_DIR) itself.
clean:
	@if [ -f '$(STAMP)' ]; then echo "rm -rf $(BUILD)/$(TAG)"; rm -rf '$(BUILD)/$(TAG)'; fi; \
	rmdir '$(BUILD)' 2>/dev/null || true

clean-all:
	@for d in '$(BUILD)'/*/; do \
	  if [ -f "$${d}.zmod4510-make" ]; then echo "rm -rf $$d"; rm -rf "$$d"; fi; \
	done; \
	rmdir '$(BUILD)' 2>/dev/null || true

help:
	@grep '^## ' $(THIS_MAKEFILE) | sed 's/^## \{0,1\}/  /'
	@echo
	@echo "Compiler:  $(CC)   ($(CC_SOURCE))"
	@echo "Found at:  $(or $(CC_PATH),NOT FOUND in PATH)"
	@echo "Version:   $(or $(shell $(CC) --version 2>/dev/null | head -n 1),unknown)"
	@echo "Installed: $(or $(INSTALLED_GCC),none found in /usr/bin)"
	@echo "Variables: CFLAGS='$(CFLAGS)' USE_REIMPL=$(USE_REIMPL) BUILD_DIR=$(BUILD_DIR)"
	@echo "Output of this build: $(OUT)"
