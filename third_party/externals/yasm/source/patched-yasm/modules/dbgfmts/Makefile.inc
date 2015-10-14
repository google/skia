EXTRA_DIST += modules/dbgfmts/codeview/Makefile.inc
EXTRA_DIST += modules/dbgfmts/dwarf2/Makefile.inc
EXTRA_DIST += modules/dbgfmts/null/Makefile.inc
EXTRA_DIST += modules/dbgfmts/stabs/Makefile.inc

include modules/dbgfmts/codeview/Makefile.inc
include modules/dbgfmts/dwarf2/Makefile.inc
include modules/dbgfmts/null/Makefile.inc
include modules/dbgfmts/stabs/Makefile.inc

notrans_dist_man_MANS += yasm_dbgfmts.7

if BUILD_MAN
yasm_dbgfmts.7: modules/dbgfmts/yasm_dbgfmts.xml
	$(XMLTO) -o $(top_builddir) man $(srcdir)/modules/dbgfmts/yasm_dbgfmts.xml
endif

EXTRA_DIST += modules/dbgfmts/yasm_dbgfmts.xml
