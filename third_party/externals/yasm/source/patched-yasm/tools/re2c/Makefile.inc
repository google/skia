# These utility programs have to be built for BUILD host in cross-build.
# This makes things rather non-standard automake

noinst_PROGRAMS += re2c

re2c_SOURCES =
EXTRA_DIST += tools/re2c/main.c
EXTRA_DIST += tools/re2c/basics.h
EXTRA_DIST += tools/re2c/globals.h
EXTRA_DIST += tools/re2c/ins.h
EXTRA_DIST += tools/re2c/re.h
EXTRA_DIST += tools/re2c/token.h
EXTRA_DIST += tools/re2c/code.c
EXTRA_DIST += tools/re2c/dfa.h
EXTRA_DIST += tools/re2c/dfa.c
EXTRA_DIST += tools/re2c/parse.h
EXTRA_DIST += tools/re2c/parser.h
EXTRA_DIST += tools/re2c/parser.c
EXTRA_DIST += tools/re2c/actions.c
EXTRA_DIST += tools/re2c/scanner.h
EXTRA_DIST += tools/re2c/scanner.c
EXTRA_DIST += tools/re2c/mbo_getopt.h
EXTRA_DIST += tools/re2c/mbo_getopt.c
EXTRA_DIST += tools/re2c/substr.h
EXTRA_DIST += tools/re2c/substr.c
EXTRA_DIST += tools/re2c/translate.c
re2c_LDADD  = re2c-main.$(OBJEXT)
re2c_LDADD += re2c-code.$(OBJEXT)
re2c_LDADD += re2c-dfa.$(OBJEXT)
re2c_LDADD += re2c-parser.$(OBJEXT)
re2c_LDADD += re2c-actions.$(OBJEXT)
re2c_LDADD += re2c-scanner.$(OBJEXT)
re2c_LDADD += re2c-mbo_getopt.$(OBJEXT)
re2c_LDADD += re2c-substr.$(OBJEXT)
re2c_LDADD += re2c-translate.$(OBJEXT)
re2c_LINK = $(CCLD_FOR_BUILD) -o $@

re2c-main.$(OBJEXT): tools/re2c/main.c
	$(CC_FOR_BUILD) $(CFLAGS_FOR_BUILD) $(DEFAULT_INCLUDES) $(INCLUDES) \
	  -c -o $@ `test -f tools/re2c/main.c || echo '$(srcdir)/'`tools/re2c/main.c

re2c-code.$(OBJEXT): tools/re2c/code.c
	$(CC_FOR_BUILD) $(CFLAGS_FOR_BUILD) $(DEFAULT_INCLUDES) $(INCLUDES) \
	  -c -o $@ `test -f tools/re2c/code.c || echo '$(srcdir)/'`tools/re2c/code.c

re2c-dfa.$(OBJEXT): tools/re2c/dfa.c
	$(CC_FOR_BUILD) $(CFLAGS_FOR_BUILD) $(DEFAULT_INCLUDES) $(INCLUDES) \
	  -c -o $@ `test -f tools/re2c/dfa.c || echo '$(srcdir)/'`tools/re2c/dfa.c

re2c-parser.$(OBJEXT): tools/re2c/parser.c
	$(CC_FOR_BUILD) $(CFLAGS_FOR_BUILD) $(DEFAULT_INCLUDES) $(INCLUDES) \
	  -c -o $@ `test -f tools/re2c/parser.c || echo '$(srcdir)/'`tools/re2c/parser.c

re2c-actions.$(OBJEXT): tools/re2c/actions.c
	$(CC_FOR_BUILD) $(CFLAGS_FOR_BUILD) $(DEFAULT_INCLUDES) $(INCLUDES) \
	  -c -o $@ `test -f tools/re2c/actions.c || echo '$(srcdir)/'`tools/re2c/actions.c

re2c-scanner.$(OBJEXT): tools/re2c/scanner.c
	$(CC_FOR_BUILD) $(CFLAGS_FOR_BUILD) $(DEFAULT_INCLUDES) $(INCLUDES) \
	  -c -o $@ `test -f tools/re2c/scanner.c || echo '$(srcdir)/'`tools/re2c/scanner.c

re2c-mbo_getopt.$(OBJEXT): tools/re2c/mbo_getopt.c
	$(CC_FOR_BUILD) $(CFLAGS_FOR_BUILD) $(DEFAULT_INCLUDES) $(INCLUDES) \
	  -c -o $@ `test -f tools/re2c/mbo_getopt.c || echo '$(srcdir)/'`tools/re2c/mbo_getopt.c

re2c-substr.$(OBJEXT): tools/re2c/substr.c
	$(CC_FOR_BUILD) $(CFLAGS_FOR_BUILD) $(DEFAULT_INCLUDES) $(INCLUDES) \
	  -c -o $@ `test -f tools/re2c/substr.c || echo '$(srcdir)/'`tools/re2c/substr.c

re2c-translate.$(OBJEXT): tools/re2c/translate.c
	$(CC_FOR_BUILD) $(CFLAGS_FOR_BUILD) $(DEFAULT_INCLUDES) $(INCLUDES) \
	  -c -o $@ `test -f tools/re2c/translate.c || echo '$(srcdir)/'`tools/re2c/translate.c

EXTRA_DIST += tools/re2c/CHANGELOG
EXTRA_DIST += tools/re2c/NO_WARRANTY
EXTRA_DIST += tools/re2c/README
EXTRA_DIST += tools/re2c/scanner.re
EXTRA_DIST += tools/re2c/re2c.1
EXTRA_DIST += tools/re2c/bootstrap/scanner.c
EXTRA_DIST += tools/re2c/doc/loplas.ps.gz
EXTRA_DIST += tools/re2c/doc/sample.bib
EXTRA_DIST += tools/re2c/examples/basemmap.c
EXTRA_DIST += tools/re2c/examples/c.re
EXTRA_DIST += tools/re2c/examples/cmmap.re
EXTRA_DIST += tools/re2c/examples/cnokw.re
EXTRA_DIST += tools/re2c/examples/cunroll.re
EXTRA_DIST += tools/re2c/examples/modula.re
EXTRA_DIST += tools/re2c/examples/repeater.re
EXTRA_DIST += tools/re2c/examples/sample.re
EXTRA_DIST += tools/re2c/examples/simple.re
EXTRA_DIST += tools/re2c/examples/rexx/README
EXTRA_DIST += tools/re2c/examples/rexx/rexx.l
EXTRA_DIST += tools/re2c/examples/rexx/scanio.c
