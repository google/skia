EXTRA_DIST += tools/re2c/Makefile.inc
EXTRA_DIST += tools/genmacro/Makefile.inc
EXTRA_DIST += tools/genperf/Makefile.inc
EXTRA_DIST += tools/python-yasm/Makefile.inc

include tools/re2c/Makefile.inc
include tools/genmacro/Makefile.inc
include tools/genperf/Makefile.inc
include tools/python-yasm/Makefile.inc
