libyasm_a_SOURCES += modules/parsers/gas/gas-parser.c
libyasm_a_SOURCES += modules/parsers/gas/gas-parser.h
libyasm_a_SOURCES += modules/parsers/gas/gas-parse.c
libyasm_a_SOURCES += modules/parsers/gas/gas-parse-intel.c
nodist_libyasm_a_SOURCES += gas-token.c

YASM_MODULES += parser_gas parser_gnu

gas-token.c: $(srcdir)/modules/parsers/gas/gas-token.re re2c$(EXEEXT)
	$(top_builddir)/re2c$(EXEEXT) -b -o $@ $(srcdir)/modules/parsers/gas/gas-token.re

BUILT_SOURCES += gas-token.c

CLEANFILES += gas-token.c

EXTRA_DIST += modules/parsers/gas/tests/Makefile.inc
EXTRA_DIST += modules/parsers/gas/gas-token.re

include modules/parsers/gas/tests/Makefile.inc
