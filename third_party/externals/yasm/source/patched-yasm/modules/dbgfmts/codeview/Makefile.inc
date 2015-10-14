libyasm_a_SOURCES += modules/dbgfmts/codeview/cv-dbgfmt.h
libyasm_a_SOURCES += modules/dbgfmts/codeview/cv-dbgfmt.c
libyasm_a_SOURCES += modules/dbgfmts/codeview/cv-symline.c
libyasm_a_SOURCES += modules/dbgfmts/codeview/cv-type.c

YASM_MODULES += dbgfmt_cv8

EXTRA_DIST += modules/dbgfmts/codeview/cv8.txt

#EXTRA_DIST += modules/dbgfmts/codeview/tests/Makefile.inc
#include modules/dbgfmts/codeview/tests/Makefile.inc
