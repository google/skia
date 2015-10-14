libyasm_a_SOURCES += modules/objfmts/bin/bin-objfmt.c

YASM_MODULES += objfmt_bin objfmt_dosexe

EXTRA_DIST += modules/objfmts/bin/tests/Makefile.inc

include modules/objfmts/bin/tests/Makefile.inc
