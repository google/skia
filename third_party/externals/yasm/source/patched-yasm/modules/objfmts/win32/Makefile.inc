# Assume objfmt_coff is included

YASM_MODULES += objfmt_win32

EXTRA_DIST += modules/objfmts/win32/tests/Makefile.inc

include modules/objfmts/win32/tests/Makefile.inc
