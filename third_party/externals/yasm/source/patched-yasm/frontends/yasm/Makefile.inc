bin_PROGRAMS += yasm
dist_man_MANS += yasm.1

if BUILD_MAN
yasm.1: frontends/yasm/yasm.xml
	$(XMLTO) -o $(top_builddir) man $(srcdir)/frontends/yasm/yasm.xml
endif

yasm_SOURCES  = frontends/yasm/yasm.c
yasm_SOURCES += frontends/yasm/yasm-options.c
yasm_SOURCES += frontends/yasm/yasm-options.h

$(srcdir)/frontends/yasm/yasm.c: license.c

license.c: $(srcdir)/COPYING genstring$(EXEEXT)
	$(top_builddir)/genstring$(EXEEXT) license_msg $@ $(srcdir)/COPYING

BUILT_SOURCES += license.c
CLEANFILES += license.c

yasm_LDADD = libyasm.a $(INTLLIBS)

EXTRA_DIST += frontends/yasm/yasm.xml
