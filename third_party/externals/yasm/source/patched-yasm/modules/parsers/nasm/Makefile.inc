libyasm_a_SOURCES += modules/parsers/nasm/nasm-parser.c
libyasm_a_SOURCES += modules/parsers/nasm/nasm-parser.h
libyasm_a_SOURCES += modules/parsers/nasm/nasm-parser-struct.h
libyasm_a_SOURCES += modules/parsers/nasm/nasm-parse.c
nodist_libyasm_a_SOURCES += nasm-token.c

YASM_MODULES += parser_nasm parser_tasm

nasm-token.c: $(srcdir)/modules/parsers/nasm/nasm-token.re re2c$(EXEEXT)
	$(top_builddir)/re2c$(EXEEXT) -b -o $@ $(srcdir)/modules/parsers/nasm/nasm-token.re

BUILT_SOURCES += nasm-token.c

CLEANFILES += nasm-token.c

EXTRA_DIST += modules/parsers/nasm/nasm-token.re

$(top_srcdir)/modules/parsers/nasm/nasm-parser.c: nasm-macros.c

nasm-macros.c: $(srcdir)/modules/parsers/nasm/nasm-std.mac genmacro$(EXEEXT)
	$(top_builddir)/genmacro$(EXEEXT) $@ nasm_standard_mac $(srcdir)/modules/parsers/nasm/nasm-std.mac

BUILT_SOURCES += nasm-macros.c
CLEANFILES += nasm-macros.c
EXTRA_DIST += modules/parsers/nasm/nasm-std.mac

EXTRA_DIST += modules/parsers/nasm/tests/Makefile.inc

include modules/parsers/nasm/tests/Makefile.inc
