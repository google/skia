libyasm_a_SOURCES += modules/arch/lc3b/lc3barch.c
libyasm_a_SOURCES += modules/arch/lc3b/lc3barch.h
libyasm_a_SOURCES += modules/arch/lc3b/lc3bbc.c
nodist_libyasm_a_SOURCES += lc3bid.c

YASM_MODULES += arch_lc3b

lc3bid.c: $(srcdir)/modules/arch/lc3b/lc3bid.re re2c$(EXEEXT)
	$(top_builddir)/re2c$(EXEEXT) -s -o $@ $(srcdir)/modules/arch/lc3b/lc3bid.re

CLEANFILES += lc3bid.c

EXTRA_DIST += modules/arch/lc3b/tests/Makefile.inc
EXTRA_DIST += modules/arch/lc3b/lc3bid.re

include modules/arch/lc3b/tests/Makefile.inc
