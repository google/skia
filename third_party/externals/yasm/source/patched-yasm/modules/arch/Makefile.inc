EXTRA_DIST += modules/arch/x86/Makefile.inc
EXTRA_DIST += modules/arch/lc3b/Makefile.inc

include modules/arch/x86/Makefile.inc
include modules/arch/lc3b/Makefile.inc

notrans_dist_man_MANS += yasm_arch.7

if BUILD_MAN
yasm_arch.7: modules/arch/yasm_arch.xml
	$(XMLTO) -o $(top_builddir) man $(srcdir)/modules/arch/yasm_arch.xml
endif

EXTRA_DIST += modules/arch/yasm_arch.xml
