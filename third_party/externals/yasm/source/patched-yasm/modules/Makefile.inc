EXTRA_DIST += modules/arch/Makefile.inc
EXTRA_DIST += modules/listfmts/Makefile.inc
EXTRA_DIST += modules/parsers/Makefile.inc
EXTRA_DIST += modules/preprocs/Makefile.inc
EXTRA_DIST += modules/objfmts/Makefile.inc

include modules/arch/Makefile.inc
include modules/listfmts/Makefile.inc
include modules/parsers/Makefile.inc
include modules/preprocs/Makefile.inc
include modules/dbgfmts/Makefile.inc
include modules/objfmts/Makefile.inc
