libyasm_a_SOURCES += modules/dbgfmts/stabs/stabs-dbgfmt.c

YASM_MODULES += dbgfmt_stabs

EXTRA_DIST += modules/dbgfmts/stabs/tests/Makefile.inc
include modules/dbgfmts/stabs/tests/Makefile.inc
