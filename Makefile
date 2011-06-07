# Makefile that redirects almost all make directives to the gyp-generated Makefile.
#
# Note that this method of building works only on Unix (not Mac or Windows).
# See http://code.google.com/p/skia/wiki/DocRoot for complete documentation.

# Directory within which we want all generated files to be written.
outdir := out

# GYP-generated Makefiles only work on Linux/Unix (not Mac or Windows).
uname := $(shell uname)
ifneq (,$(findstring Darwin, $(uname)))
  $(error Cannot build using Make on Mac. See http://code.google.com/p/skia/wiki/GettingStartedOnMac)
endif
ifneq (,$(findstring CYGWIN, $(uname)))
  $(error Cannot build using Make on Windows. See http://code.google.com/p/skia/wiki/GettingStartedOnWindows)
endif

# Default target.  This must be listed before all other targets.
.PHONY: default
default: all

.PHONY: clean
clean:
	rm -rf $(outdir)

# Any target OTHER than clean...
# Ask gyp to create the real Makefile, and then pass control to it.
%:
	./gyp_skia -f make
	make -C $(outdir) $@

# Unfortunately, this is a bit ugly, but necessary.
# If there are any files/directories within the same directory as this Makefile
# which share the same name as a target ("tests", for example), then make
# will refuse to rebuild those targets because the file already exists.
local_filenames := $(shell ls)
.PHONY: $(local_filenames)
$(local_filenames)::
	./gyp_skia -f make
	make -C $(outdir) $@
