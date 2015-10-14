EXTRA_DIST += modules/objfmts/dbg/Makefile.inc
EXTRA_DIST += modules/objfmts/bin/Makefile.inc
EXTRA_DIST += modules/objfmts/elf/Makefile.inc
#!EXTRA_DIST += modules/objfmts/omf/Makefile.inc
EXTRA_DIST += modules/objfmts/coff/Makefile.inc
EXTRA_DIST += modules/objfmts/macho/Makefile.inc
EXTRA_DIST += modules/objfmts/rdf/Makefile.inc
EXTRA_DIST += modules/objfmts/win32/Makefile.inc
EXTRA_DIST += modules/objfmts/win64/Makefile.inc
EXTRA_DIST += modules/objfmts/xdf/Makefile.inc

include modules/objfmts/dbg/Makefile.inc
include modules/objfmts/bin/Makefile.inc
include modules/objfmts/elf/Makefile.inc
#!include modules/objfmts/omf/Makefile.inc
include modules/objfmts/coff/Makefile.inc
include modules/objfmts/macho/Makefile.inc
include modules/objfmts/rdf/Makefile.inc
include modules/objfmts/win32/Makefile.inc
include modules/objfmts/win64/Makefile.inc
include modules/objfmts/xdf/Makefile.inc

notrans_dist_man_MANS += yasm_objfmts.7

if BUILD_MAN
yasm_objfmts.7: modules/objfmts/yasm_objfmts.xml
	$(XMLTO) -o $(top_builddir) man $(srcdir)/modules/objfmts/yasm_objfmts.xml
endif

EXTRA_DIST += modules/objfmts/yasm_objfmts.xml
