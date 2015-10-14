libyasm_a_SOURCES += modules/preprocs/nasm/nasm-preproc.c
libyasm_a_SOURCES += modules/preprocs/nasm/nasm-pp.h
libyasm_a_SOURCES += modules/preprocs/nasm/nasm-pp.c
libyasm_a_SOURCES += modules/preprocs/nasm/nasm.h
libyasm_a_SOURCES += modules/preprocs/nasm/nasmlib.h
libyasm_a_SOURCES += modules/preprocs/nasm/nasmlib.c
libyasm_a_SOURCES += modules/preprocs/nasm/nasm-eval.h
libyasm_a_SOURCES += modules/preprocs/nasm/nasm-eval.c

YASM_MODULES += preproc_nasm preproc_tasm

$(top_srcdir)/modules/preprocs/nasm/nasm-preproc.c: nasm-version.c

nasm-version.c: version.mac genmacro$(EXEEXT)
	$(top_builddir)/genmacro$(EXEEXT) $@ nasm_version_mac version.mac

BUILT_SOURCES += nasm-version.c
CLEANFILES += nasm-version.c

version.mac: genversion$(EXEEXT)
	$(top_builddir)/genversion$(EXEEXT) $@

BUILT_SOURCES += version.mac
CLEANFILES += version.mac

noinst_PROGRAMS += genversion

genversion_SOURCES =
EXTRA_DIST += modules/preprocs/nasm/genversion.c
genversion_LDADD = genversion.$(OBJEXT)
genversion_LINK = $(CCLD_FOR_BUILD) -o $@

genversion.$(OBJEXT): modules/preprocs/nasm/genversion.c
	$(CC_FOR_BUILD) $(DEFAULT_INCLUDES) $(INCLUDES) -c -o $@ `test -f modules/preprocs/nasm/genversion.c || echo '$(srcdir)/'`modules/preprocs/nasm/genversion.c

EXTRA_DIST += modules/preprocs/nasm/tests/Makefile.inc

include modules/preprocs/nasm/tests/Makefile.inc
