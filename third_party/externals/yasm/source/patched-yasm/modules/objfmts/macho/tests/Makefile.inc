EXTRA_DIST += modules/objfmts/macho/tests/gas32/Makefile.inc
EXTRA_DIST += modules/objfmts/macho/tests/gas64/Makefile.inc
EXTRA_DIST += modules/objfmts/macho/tests/nasm32/Makefile.inc
EXTRA_DIST += modules/objfmts/macho/tests/nasm64/Makefile.inc

include modules/objfmts/macho/tests/gas32/Makefile.inc
include modules/objfmts/macho/tests/gas64/Makefile.inc
include modules/objfmts/macho/tests/nasm32/Makefile.inc
include modules/objfmts/macho/tests/nasm64/Makefile.inc
