libyasm_a_SOURCES += libyasm/assocdat.c
libyasm_a_SOURCES += libyasm/bitvect.c
libyasm_a_SOURCES += libyasm/bc-align.c
libyasm_a_SOURCES += libyasm/bc-data.c
libyasm_a_SOURCES += libyasm/bc-incbin.c
libyasm_a_SOURCES += libyasm/bc-org.c
libyasm_a_SOURCES += libyasm/bc-reserve.c
libyasm_a_SOURCES += libyasm/bytecode.c
libyasm_a_SOURCES += libyasm/errwarn.c
libyasm_a_SOURCES += libyasm/expr.c
libyasm_a_SOURCES += libyasm/file.c
libyasm_a_SOURCES += libyasm/floatnum.c
libyasm_a_SOURCES += libyasm/hamt.c
libyasm_a_SOURCES += libyasm/insn.c
libyasm_a_SOURCES += libyasm/intnum.c
libyasm_a_SOURCES += libyasm/inttree.c
libyasm_a_SOURCES += libyasm/linemap.c
libyasm_a_SOURCES += libyasm/md5.c
libyasm_a_SOURCES += libyasm/mergesort.c
libyasm_a_SOURCES += libyasm/phash.c
libyasm_a_SOURCES += libyasm/section.c
libyasm_a_SOURCES += libyasm/strcasecmp.c
libyasm_a_SOURCES += libyasm/strsep.c
libyasm_a_SOURCES += libyasm/symrec.c
libyasm_a_SOURCES += libyasm/valparam.c
libyasm_a_SOURCES += libyasm/value.c
libyasm_a_SOURCES += libyasm/xmalloc.c
libyasm_a_SOURCES += libyasm/xstrdup.c
nodist_libyasm_a_SOURCES += module.c

module.c: $(top_srcdir)/libyasm/module.in genmodule$(EXEEXT) Makefile
	$(top_builddir)/genmodule$(EXEEXT) $(top_srcdir)/libyasm/module.in Makefile

CLEANFILES += module.c

noinst_PROGRAMS += genmodule

genmodule_SOURCES =
EXTRA_DIST += libyasm/genmodule.c
genmodule_LDADD = genmodule.$(OBJEXT)
genmodule_LINK = $(CCLD_FOR_BUILD) -o $@

genmodule.$(OBJEXT): libyasm/genmodule.c
	$(CC_FOR_BUILD) $(DEFAULT_INCLUDES) $(INCLUDES) -c -o $@ `test -f libyasm/genmodule.c || echo '$(srcdir)/'`libyasm/genmodule.c

EXTRA_DIST += libyasm/module.in

modincludedir = $(includedir)/libyasm

modinclude_HEADERS  = libyasm/arch.h
modinclude_HEADERS += libyasm/assocdat.h
modinclude_HEADERS += libyasm/bitvect.h
modinclude_HEADERS += libyasm/bytecode.h
modinclude_HEADERS += libyasm/compat-queue.h
modinclude_HEADERS += libyasm/coretype.h
modinclude_HEADERS += libyasm/dbgfmt.h
modinclude_HEADERS += libyasm/errwarn.h
modinclude_HEADERS += libyasm/expr.h
modinclude_HEADERS += libyasm/file.h
modinclude_HEADERS += libyasm/floatnum.h
modinclude_HEADERS += libyasm/hamt.h
modinclude_HEADERS += libyasm/insn.h
modinclude_HEADERS += libyasm/intnum.h
modinclude_HEADERS += libyasm/inttree.h
modinclude_HEADERS += libyasm/linemap.h
modinclude_HEADERS += libyasm/listfmt.h
modinclude_HEADERS += libyasm/md5.h
modinclude_HEADERS += libyasm/module.h
modinclude_HEADERS += libyasm/objfmt.h
modinclude_HEADERS += libyasm/parser.h
modinclude_HEADERS += libyasm/phash.h
modinclude_HEADERS += libyasm/preproc.h
modinclude_HEADERS += libyasm/section.h
modinclude_HEADERS += libyasm/symrec.h
modinclude_HEADERS += libyasm/valparam.h
modinclude_HEADERS += libyasm/value.h

EXTRA_DIST += libyasm/tests/Makefile.inc

include libyasm/tests/Makefile.inc
