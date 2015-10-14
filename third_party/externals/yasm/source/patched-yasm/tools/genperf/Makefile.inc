# These utility programs have to be built for BUILD host in cross-build.
# This makes things rather non-standard automake

noinst_PROGRAMS += genperf

# Suffix rule for genperf
SUFFIXES += .gperf
.gperf.c: genperf$(EXEEXT)
	$(top_builddir)/genperf$(EXEEXT) $< $@

genperf_SOURCES =
EXTRA_DIST += tools/genperf/genperf.c
EXTRA_DIST += tools/genperf/perfect.c
EXTRA_DIST += tools/genperf/perfect.h
EXTRA_DIST += tools/genperf/standard.h
genperf_LDADD  = genperf.$(OBJEXT)
genperf_LDADD += gp-perfect.$(OBJEXT)
genperf_LDADD += gp-phash.$(OBJEXT)
genperf_LDADD += gp-xmalloc.$(OBJEXT)
genperf_LDADD += gp-xstrdup.$(OBJEXT)
genperf_LINK = $(CCLD_FOR_BUILD) -o $@

genperf.$(OBJEXT): tools/genperf/genperf.c
	$(CC_FOR_BUILD) $(CFLAGS_FOR_BUILD) $(DEFAULT_INCLUDES) $(INCLUDES) \
	  -c -o $@ `test -f tools/genperf/genperf.c || echo '$(srcdir)/'`tools/genperf/genperf.c

gp-perfect.$(OBJEXT): tools/genperf/perfect.c
	$(CC_FOR_BUILD) $(CFLAGS_FOR_BUILD) $(DEFAULT_INCLUDES) $(INCLUDES) \
	  -c -o $@ `test -f tools/genperf/perfect.c || echo '$(srcdir)/'`tools/genperf/perfect.c

gp-phash.$(OBJEXT): libyasm/phash.c
	$(CC_FOR_BUILD) $(CFLAGS_FOR_BUILD) $(DEFAULT_INCLUDES) $(INCLUDES) \
	  -c -o $@ `test -f libyasm/phash.c || echo '$(srcdir)/'`libyasm/phash.c

gp-xmalloc.$(OBJEXT): libyasm/xmalloc.c
	$(CC_FOR_BUILD) $(CFLAGS_FOR_BUILD) $(DEFAULT_INCLUDES) $(INCLUDES) \
	  -c -o $@ `test -f libyasm/xmalloc.c || echo '$(srcdir)/'`libyasm/xmalloc.c

gp-xstrdup.$(OBJEXT): libyasm/xstrdup.c
	$(CC_FOR_BUILD) $(CFLAGS_FOR_BUILD) $(DEFAULT_INCLUDES) $(INCLUDES) \
	  -c -o $@ `test -f libyasm/xstrdup.c || echo '$(srcdir)/'`libyasm/xstrdup.c

