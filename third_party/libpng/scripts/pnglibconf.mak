#!/usr/bin/make -f
# pnglibconf.mak - standard make lines for pnglibconf.h
# 
# These lines are copied from Makefile.am, they illustrate
# how to automate the build of pnglibconf.h from scripts/pnglibconf.dfa
# given 'awk' and 'sed'

# Override as appropriate, these definitions can be overridden on
# the make command line (AWK='nawk' for example).
AWK = gawk
AWK = mawk
AWK = nawk
AWK = one-true-awk
AWK = awk  # Crashes on SunOS 5.10 - use 'nawk'
CPP = $(CC) -E # Does not work on SUN OS 5.10 - use /lib/cpp
SED = sed

COPY = cp
DELETE = rm -f
ECHO = echo
DFA_XTRA = # Appended to scripts/options.awk

# CPPFLAGS should contain the options to control the result,
# but DEFS and CFLAGS are also supported here, override
# as appropriate
DFNFLAGS = $(DEFS) $(CPPFLAGS) $(CFLAGS)

# srcdir is a defacto standard for the location of the source
srcdir = .

# The standard pnglibconf.h exists as scripts/pnglibconf.h.prebuilt,
# copy this if the following doesn't work.
pnglibconf.h: pnglibconf.dfn
	$(DELETE) $@ dfn.c dfn1.out dfn2.out dfn3.out
	$(ECHO) '#include "pnglibconf.dfn"' >dfn.c
	$(CPP) $(DFNFLAGS) dfn.c >dfn1.out
	$(ECHO) "If 'cpp -e' crashes try /lib/cpp (e.g. CPP='/lib/cpp')" >&2
	$(SED) -n -e 's|^.*PNG_DEFN_MAGIC-\(.*\)-PNG_DEFN_END.*$$|\1|p'\
	    dfn1.out >dfn2.out
	$(SED) -e 's| *@@@ *||g' -e 's| *$$||' dfn2.out >dfn3.out
	$(COPY) dfn3.out $@
	$(DELETE) dfn.c dfn1.out dfn2.out dfn3.out

pnglibconf.dfn: $(srcdir)/scripts/pnglibconf.dfa $(srcdir)/scripts/options.awk
	$(DELETE) $@ dfn1.out dfn2.out
	$(ECHO) "Calling $(AWK) from scripts/pnglibconf.mak" >&2
	$(ECHO) "If 'awk' crashes try a better awk (e.g. AWK='nawk')" >&2
	$(AWK) -f $(srcdir)/scripts/options.awk out=dfn1.out\
	    $(srcdir)/scripts/pnglibconf.dfa $(DFA_XTRA) 1>&2
	$(AWK) -f $(srcdir)/scripts/options.awk out=dfn2.out dfn1.out 1>&2
	$(COPY) dfn2.out $@
	$(DELETE) dfn1.out dfn2.out

clean-pnglibconf:
	$(DELETE) pnglibconf.h pnglibconf.dfn dfn.c dfn1.out dfn2.out dfn3.out

clean: clean-pnglibconf
