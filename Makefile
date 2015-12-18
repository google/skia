# Makefile that wraps the Gyp and build steps for Unix and Mac (but not Windows)
# Uses "ninja" to build the code.
#
# Some usage examples (tested on both Linux and Mac):
#
#   # Clean everything
#   make clean
#
#   # Build and run tests (in Debug mode)
#   make dm
#   out/Debug/dm
#
#   # Build and run tests (in Release mode)
#   make dm BUILDTYPE=Release
#   out/Release/dm
#
#   # Build bench and SampleApp (both in Release mode), and then run them
#   make SampleApp bench BUILDTYPE=Release
#   out/Release/bench -repeat 2
#   out/Release/SampleApp
#
#   # Build all targets (in Debug mode)
#   make
#
# If you want more fine-grained control, you can run gyp and then build the
# gyp-generated projects yourself.
#
# See https://skia.org for complete documentation.

SKIA_OUT ?= out
BUILDTYPE ?= Debug
CWD := $(shell pwd)

# Soon we should be able to get rid of VALID_TARGETS, and just pass control
# to the gyp-generated Makefile for *any* target name.
# But that will be a bit complicated, so let's keep it for a future CL.
# Tracked as https://code.google.com/p/skia/issues/detail?id=947 ('eliminate
# need for VALID_TARGETS in toplevel Makefile')
#
# TODO(epoger): I'm not sure if the above comment is still valid in a ninja
# world.
VALID_TARGETS := \
                 nanobench \
                 debugger \
                 dm \
                 everything \
                 lua_app \
                 lua_pictures \
                 most \
                 pathops_unittest \
                 SampleApp \
                 SampleApp_APK \
                 skhello \
                 skia_lib \
                 skpskgr_test \
                 tools \
                 skpdiff

# Default target.  This must be listed before all other targets.
.PHONY: default
default: most

# As noted in http://code.google.com/p/skia/issues/detail?id=330 , building
# multiple targets in parallel was failing.  The special .NOTPARALLEL target
# tells gnu make not to run targets within this Makefile in parallel.
# Targets that ninja builds at this Makefile's behest should not be affected.
.NOTPARALLEL:

uname := $(shell uname)
ifneq (,$(findstring CYGWIN, $(uname)))
  $(error Cannot build using Make on Windows. See https://skia.org/user/quick/windows)
endif

# If user requests "make all", chain to our explicitly-declared "everything"
# target. See https://code.google.com/p/skia/issues/detail?id=932 ("gyp
# automatically creates "all" target on some build flavors but not others")
.PHONY: all
all: everything

.PHONY: clean
clean:
	rm -rf out xcodebuild
ifneq (out, $(SKIA_OUT))
	rm -rf $(SKIA_OUT)
endif

# Run gyp no matter what.
.PHONY: gyp
gyp:
	$(CWD)/gyp_skia --no-parallel -G config=$(BUILDTYPE)

# For all specific targets: run gyp if necessary, and then pass control to
# the gyp-generated buildfiles.
.PHONY: $(VALID_TARGETS)
$(VALID_TARGETS):: gyp
	ninja -C $(SKIA_OUT)/$(BUILDTYPE) $@
