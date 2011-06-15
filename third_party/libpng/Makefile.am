# Makefile.am:
#   Source file for Makefile.in (and hence Makefile)
#

PNGLIB_BASENAME= libpng@PNGLIB_MAJOR@@PNGLIB_MINOR@

# libpng does not follow GNU file name conventions

# "color-tests" requires automake 1.11.1 or later.  Enable it if you like,
# to get red "FAIL" and green "PASS" notations during tests.
# AUTOMAKE_OPTIONS = foreign color-tests
AUTOMAKE_OPTIONS = foreign

# test programs - run on make check, make distcheck
check_PROGRAMS= pngtest pngvalid
pngtest_SOURCES = pngtest.c
pngtest_LDADD = libpng@PNGLIB_MAJOR@@PNGLIB_MINOR@.la
pngvalid_SOURCES = pngvalid.c
pngvalid_LDADD = libpng@PNGLIB_MAJOR@@PNGLIB_MINOR@.la
TESTS = test-pngtest.sh test-pngvalid-simple.sh test-pngvalid-full.sh
TESTS_ENVIRONMENT= srcdir=$(srcdir) 

# man pages
dist_man_MANS= libpng.3 libpngpf.3 png.5

# generate the -config scripts if required
binconfigs= libpng@PNGLIB_MAJOR@@PNGLIB_MINOR@-config 
EXTRA_SCRIPTS= libpng-config libpng@PNGLIB_MAJOR@@PNGLIB_MINOR@-config
bin_SCRIPTS= @binconfigs@

# rules to build libpng, only build the old library on request
lib_LTLIBRARIES=libpng@PNGLIB_MAJOR@@PNGLIB_MINOR@.la
# EXTRA_LTLIBRARIES= libpng.la
libpng@PNGLIB_MAJOR@@PNGLIB_MINOR@_la_SOURCES = png.c pngerror.c\
	pngget.c pngmem.c pngpread.c pngread.c pngrio.c pngrtran.c pngrutil.c\
	pngset.c pngtrans.c pngwio.c pngwrite.c pngwtran.c pngwutil.c\
	png.h pngconf.h pngdebug.h pnginfo.h pngpriv.h pngstruct.h

nodist_libpng@PNGLIB_MAJOR@@PNGLIB_MINOR@_la_SOURCES = pnglibconf.h

libpng@PNGLIB_MAJOR@@PNGLIB_MINOR@_la_CPPFLAGS = @LIBPNG_DEFINES@

libpng@PNGLIB_MAJOR@@PNGLIB_MINOR@_la_LDFLAGS = -no-undefined -export-dynamic \
	-version-number @PNGLIB_MAJOR@@PNGLIB_MINOR@:@PNGLIB_RELEASE@:0

if HAVE_LD_VERSION_SCRIPT
  # Versioned symbols and restricted exports
  libpng@PNGLIB_MAJOR@@PNGLIB_MINOR@_la_LDFLAGS += -Wl,--version-script=libpng.vers
  libpng@PNGLIB_MAJOR@@PNGLIB_MINOR@_la_DEPENDENCIES = libpng.vers
else
  # Only restricted exports when possible
  libpng@PNGLIB_MAJOR@@PNGLIB_MINOR@_la_LDFLAGS += -export-symbols libpng.sym
  libpng@PNGLIB_MAJOR@@PNGLIB_MINOR@_la_DEPENDENCIES = libpng.sym
endif

#distribute headers in /usr/include/libpng/*
pkgincludedir= $(includedir)/$(PNGLIB_BASENAME)
pkginclude_HEADERS= png.h pngconf.h
nodist_pkginclude_HEADERS= pnglibconf.h

# pkg-config stuff, note that libpng.pc is always required in order
# to get the correct library
pkgconfigdir = @pkgconfigdir@
pkgconfig_DATA = libpng@PNGLIB_MAJOR@@PNGLIB_MINOR@.pc

#extra source distribution files.
EXTRA_DIST= \
	ANNOUNCE CHANGES INSTALL LICENSE README TODO \
	pngtest.png pngbar.png pngnow.png pngbar.jpg autogen.sh \
	${srcdir}/contrib ${srcdir}/projects ${srcdir}/scripts \
	$(TESTS) \
	CMakeLists.txt example.c libpng-manual.txt

SCRIPT_CLEANFILES=scripts/*.out scripts/*.chk scripts/pnglibconf.dfn

CLEANFILES= dfn.c dfn?.out pngout.png libpng@PNGLIB_MAJOR@@PNGLIB_MINOR@.pc \
	libpng@PNGLIB_MAJOR@@PNGLIB_MINOR@-config libpng.vers libpng.sym \
	check.new pnglibconf.* symbols.new pngtest-log.txt \
	$(SCRIPT_CLEANFILES)

MAINTAINERCLEANFILES = Makefile.in aclocal.m4 config.guess config.h.in \
config.sub configure depcomp install-sh ltmain.sh missing

# DFNCPP is normally just CPP - the C preprocessor - but on Solaris and maybe
# other operating systems (NeXT?) the C preprocessor selected by configure
# checks input tokens for validity - effectively it performs part of the ANSI-C
# parsing - and therefore fails with the .df files.  configure.ac has special
# checks for this and sets DFNCPP appropriately.
DFNCPP = @DFNCPP@

SUFFIXES = .chk .dfn .out

$(PNGLIB_BASENAME).pc: libpng.pc
	cp libpng.pc $@

$(PNGLIB_BASENAME)-config: libpng-config
	cp libpng-config $@

scripts/sym.out scripts/vers.out: png.h pngconf.h pnglibconf.h
scripts/symbols.out: png.h pngconf.h scripts/pnglibconf.h.prebuilt

libpng.sym: scripts/sym.out
	rm -f $@
	cp $? $@
libpng.vers: scripts/vers.out
	rm -f $@
	cp $? $@
pnglibconf.h: pnglibconf.out
	rm -f $@
	cp $? $@
scripts/pnglibconf.h.prebuilt:
	@echo "This is a machine generated file, but if you want to make" >&2
	@echo "a new one simply make 'scripts/pnglibconf.out' and copy that" >&2
	@exit 1

# The following is necessary to ensure that the local pnglibconf.h is used, not
# an installed one (this can happen immediately after on a clean system if
# 'make test' is the first thing the user does.)
pngvalid.o pngtest.o: pnglibconf.h

# We must use -DPNG_NO_USE_READ_MACROS here even when the library may actually
# be built with PNG_USE_READ_MACROS; this stops the read macros interfering
# with the symbol file format.
SYMBOL_CFLAGS = -DPNGLIB_LIBNAME='PNG@PNGLIB_MAJOR@@PNGLIB_MINOR@_0'\
		-DPNGLIB_VERSION='@PNGLIB_VERSION@'\
		-DSYMBOL_PREFIX='$(SYMBOL_PREFIX)'\
		-DPNG_NO_USE_READ_MACROS -DPNG_BUILDING_SYMBOL_TABLE

.dfn.out:
	rm -f $@ dfn.c dfn?.out
	test -d scripts || mkdir scripts
	echo '#include "$<"' >dfn.c
	$(DFNCPP) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) @LIBPNG_DEFINES@\
	    $(CPPFLAGS) $(SYMBOL_CFLAGS) dfn.c > dfn1.out
	$(SED) -n -e 's|^.*PNG_DEFN_MAGIC-\(.*\)-PNG_DEFN_END.*$$|\1|p'\
	    dfn1.out >dfn2.out
	$(SED) -e 's| *@@@ *||g' -e 's| *$$||' dfn2.out >dfn3.out
	rm -f dfn.c dfn[12].out
	mv dfn3.out $@

# The .dfn file for pnglibconf.h is machine generated
pnglibconf.dfn: scripts/pnglibconf.dfa scripts/options.awk
	rm -f $@ dfn?.out
	$(AWK) -f ${srcdir}/scripts/options.awk out=dfn1.out\
	    ${srcdir}/scripts/pnglibconf.dfa $(DFA_XTRA) 1>&2
	$(AWK) -f ${srcdir}/scripts/options.awk out=dfn2.out dfn1.out 1>&2
	rm dfn1.out
	mv dfn2.out $@

# Symbol checks (.def and .out files should match)
scripts/symbols.chk: scripts/checksym.awk scripts/symbols.def scripts/symbols.out
.out.chk:
	rm -f $@ symbols.new
	$(AWK) -f ${srcdir}/scripts/checksym.awk ${srcdir}/scripts/${*F}.def\
	    $< >&2
	mv symbols.new $@

# used on demand to regenerate the standard header, CPPFLAGS should
# be empty - no non-standard defines
scripts/pnglibconf.dfn: scripts/pnglibconf.dfa scripts/options.awk
	rm -f $@ dfn?.out
	test -z "$(CPPFLAGS)" 
	echo "com @PNGLIB_VERSION@ STANDARD API DEFINITION" |\
	$(AWK) -f ${srcdir}/scripts/options.awk out=dfn1.out\
	    logunsupported=2 - ${srcdir}/scripts/pnglibconf.dfa 1>&2
	$(AWK) -f ${srcdir}/scripts/options.awk out=dfn2.out dfn1.out 1>&2
	rm dfn1.out
	mv dfn2.out $@

$(libpng@PNGLIB_MAJOR@@PNGLIB_MINOR@_la_OBJECTS): png.h pngconf.h \
	pnglibconf.h pngpriv.h pngdebug.h pnginfo.h pngstruct.h

test: check-am

# Extra checks
check: scripts/symbols.chk

# Don't distribute the generated script files
dist-hook:
	cd '$(top_distdir)'; rm -f $(SCRIPT_CLEANFILES)

# install the .../include headers as links to the new ones
install-data-hook:
	cd $(DESTDIR)$(includedir); rm -f png.h pngconf.h pnglibconf.h
	cd $(DESTDIR)$(includedir); $(LN_S) $(PNGLIB_BASENAME)/png.h png.h
	cd $(DESTDIR)$(includedir); $(LN_S) $(PNGLIB_BASENAME)/pngconf.h \
		pngconf.h
	cd $(DESTDIR)$(includedir); $(LN_S) $(PNGLIB_BASENAME)/pnglibconf.h \
		pnglibconf.h
	cd $(DESTDIR)$(pkgconfigdir); rm -f libpng.pc
	cd $(DESTDIR)$(pkgconfigdir); $(LN_S) $(PNGLIB_BASENAME).pc libpng.pc

# do evil things to libpng to cause libpng@PNGLIB_MAJOR@@PNGLIB_MINOR@ to be used
install-exec-hook:
	cd $(DESTDIR)$(bindir); rm -f libpng-config
	cd $(DESTDIR)$(bindir); $(LN_S) $(PNGLIB_BASENAME)-config libpng-config
	@set -x;\
	cd $(DESTDIR)$(libdir);\
	for ext in a la so so.@PNGLIB_MAJOR@@PNGLIB_MINOR@.@PNGLIB_RELEASE@ sl dylib dll.a; do\
		rm -f libpng.$$ext;\
                if test -f $(PNGLIB_BASENAME).$$ext; then\
                       $(LN_S) $(PNGLIB_BASENAME).$$ext libpng.$$ext;\
                fi;\
	done

uninstall-hook:
	cd $(DESTDIR)$(includedir); rm -f png.h pngconf.h pnglibconf.h
	rm -f $(DESTDIR)$(pkgconfigdir)/libpng.pc
	rm -f $(DESTDIR)$(bindir)/libpng-config
	rm -f $(DESTDIR)$(libdir)/libpng.a
	rm -f $(DESTDIR)$(libdir)/libpng.la
