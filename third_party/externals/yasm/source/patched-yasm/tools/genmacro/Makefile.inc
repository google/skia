# These utility programs have to be built for BUILD host in cross-build.
# This makes things rather non-standard automake

noinst_PROGRAMS += genmacro

genmacro_SOURCES =
EXTRA_DIST += tools/genmacro/genmacro.c
genmacro_LDADD = genmacro.$(OBJEXT)
genmacro_LINK = $(CCLD_FOR_BUILD) -o $@

genmacro.$(OBJEXT): tools/genmacro/genmacro.c
	$(CC_FOR_BUILD) $(CFLAGS_FOR_BUILD) $(DEFAULT_INCLUDES) $(INCLUDES) \
	  -c -o $@ `test -f tools/genmacro/genmacro.c || echo '$(srcdir)/'`tools/genmacro/genmacro.c

