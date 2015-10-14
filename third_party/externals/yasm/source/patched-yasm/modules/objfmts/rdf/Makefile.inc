libyasm_a_SOURCES += modules/objfmts/rdf/rdf-objfmt.c

YASM_MODULES += objfmt_rdf

EXTRA_DIST += modules/objfmts/rdf/tests/Makefile.inc

include modules/objfmts/rdf/tests/Makefile.inc
