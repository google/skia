# Makefile that wraps the Gyp and build steps for Unix and Mac (but not Windows)
# Uses "make" to build on Unix, and "xcodebuild" to build on Mac.
#
# Some usage examples (tested on both Linux and Mac):
#
#   # Clean everything
#   make clean
#
#   # Build and run tests (in Debug mode)
#   make tests
#   out/Debug/tests
#
#   # Build and run tests (in Release mode)
#   make tests BUILDTYPE=Release
#   out/Release/tests
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
# See http://code.google.com/p/skia/wiki/DocRoot for complete documentation.

BUILDTYPE ?= Debug
CWD := $(shell pwd)

uname := $(shell uname)
ifneq (,$(findstring CYGWIN, $(uname)))
  $(error Cannot build using Make on Windows. See http://code.google.com/p/skia/wiki/GettingStartedOnWindows)
endif

# Default target.  This must be listed before all other targets.
.PHONY: default
default: all

.PHONY: all
all: SampleApp bench gm tests tools

.PHONY: clean
clean:
	rm -rf out xcodebuild

# Any targets not defined above...
# Ask gyp to generate the buildfiles as appropriate for this platform,
# and then pass control to those buildfiles.
#
# For the Mac, we create a convenience symlink to the generated binary.
%:
	./gyp_skia
ifneq (,$(findstring Linux, $(uname)))
	$(MAKE) -C out $@ BUILDTYPE=$(BUILDTYPE)
endif
ifneq (,$(findstring Darwin, $(uname)))
	xcodebuild -project out/gyp/$@.xcodeproj -configuration $(BUILDTYPE)
	mkdir -p out/$(BUILDTYPE)
	rm -f out/$(BUILDTYPE)/$@
	ln -s $(CWD)/xcodebuild/$(BUILDTYPE)/$@.app/Contents/MacOS/$@ out/$(BUILDTYPE)/$@
endif

# This repetition is ugly, but necessary.
# If there are any files/directories within the same directory as this Makefile
# which share the same name as a target ("tests", for example), then make
# will refuse to rebuild those targets because the file already exists.
local_filenames := $(shell ls)
.PHONY: $(local_filenames)
$(local_filenames)::
	./gyp_skia
ifneq (,$(findstring Linux, $(uname)))
	$(MAKE) -C out $@ BUILDTYPE=$(BUILDTYPE)
endif
ifneq (,$(findstring Darwin, $(uname)))
	xcodebuild -project out/gyp/$@.xcodeproj -configuration $(BUILDTYPE)
	mkdir -p out/$(BUILDTYPE)
	rm -f out/$(BUILDTYPE)/$@
	ln -s $(CWD)/xcodebuild/$(BUILDTYPE)/$@.app/Contents/MacOS/$@ out/$(BUILDTYPE)/$@
endif
