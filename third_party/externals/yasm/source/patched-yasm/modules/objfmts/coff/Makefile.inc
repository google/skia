libyasm_a_SOURCES += modules/objfmts/coff/coff-objfmt.c
libyasm_a_SOURCES += modules/objfmts/coff/coff-objfmt.h
libyasm_a_SOURCES += modules/objfmts/coff/win64-except.c

YASM_MODULES += objfmt_coff

$(top_srcdir)/modules/objfmts/coff/coff-objfmt.c: win64-nasm.c win64-gas.c

win64-nasm.c: $(srcdir)/modules/objfmts/coff/win64-nasm.mac genmacro$(EXEEXT)
	$(top_builddir)/genmacro$(EXEEXT) $@ win64_nasm_stdmac $(srcdir)/modules/objfmts/coff/win64-nasm.mac

BUILT_SOURCES += win64-nasm.c
CLEANFILES += win64-nasm.c
EXTRA_DIST += modules/objfmts/coff/win64-nasm.mac

win64-gas.c: $(srcdir)/modules/objfmts/coff/win64-gas.mac genmacro$(EXEEXT)
	$(top_builddir)/genmacro$(EXEEXT) $@ win64_gas_stdmac $(srcdir)/modules/objfmts/coff/win64-gas.mac

BUILT_SOURCES += win64-gas.c
CLEANFILES += win64-gas.c
EXTRA_DIST += modules/objfmts/coff/win64-gas.mac

EXTRA_DIST += modules/objfmts/coff/tests/Makefile.inc

include modules/objfmts/coff/tests/Makefile.inc
