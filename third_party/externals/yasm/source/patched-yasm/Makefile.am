SUBDIRS = po .

ACLOCAL_AMFLAGS = -I m4
AM_YFLAGS = -d
AM_CFLAGS = @MORE_CFLAGS@

bin_PROGRAMS =
dist_man_MANS =
notrans_dist_man_MANS =
TESTS =
TESTS_ENVIRONMENT =
noinst_PROGRAMS = genstring

check_PROGRAMS = test_hd

test_hd_SOURCES = test_hd.c

include_HEADERS = libyasm.h
nodist_include_HEADERS = libyasm-stdint.h

noinst_HEADERS = util.h

BUILT_SOURCES =
MAINTAINERCLEANFILES =
DISTCLEANFILES =
SUFFIXES =

# configure.lineno doesn't clean up after itself?
CLEANFILES = configure.lineno
EXTRA_DIST = config/config.rpath

# libyasm-stdint.h doesn't clean up after itself?
CONFIG_CLEAN_FILES = libyasm-stdint.h
CONFIG_CLEAN_FILES += YASM-VERSION-FILE
CONFIG_CLEAN_FILES += YASM-VERSION.h

EXTRA_DIST += tools/Makefile.inc
EXTRA_DIST += libyasm/Makefile.inc
EXTRA_DIST += modules/Makefile.inc
EXTRA_DIST += frontends/Makefile.inc

include tools/Makefile.inc

YASM_MODULES = 
lib_LIBRARIES = libyasm.a
libyasm_a_SOURCES =
nodist_libyasm_a_SOURCES =
include modules/Makefile.inc

include libyasm/Makefile.inc
include frontends/Makefile.inc
include m4/Makefile.inc

EXTRA_DIST += out_test.sh
EXTRA_DIST += Artistic.txt
EXTRA_DIST += BSD.txt
EXTRA_DIST += GNU_GPL-2.0
EXTRA_DIST += GNU_LGPL-2.0
EXTRA_DIST += splint.sh
EXTRA_DIST += YASM-VERSION-GEN.sh
EXTRA_DIST += Mkfiles/Makefile.flat
EXTRA_DIST += Mkfiles/Makefile.dj
EXTRA_DIST += Mkfiles/dj/config.h
EXTRA_DIST += Mkfiles/dj/libyasm-stdint.h
EXTRA_DIST += Mkfiles/vc9/config.h
EXTRA_DIST += Mkfiles/vc9/crt_secure_no_deprecate.vsprops
EXTRA_DIST += Mkfiles/vc9/libyasm-stdint.h
EXTRA_DIST += Mkfiles/vc9/readme.vc9.txt
EXTRA_DIST += Mkfiles/vc9/vc98_swap.py
EXTRA_DIST += Mkfiles/vc9/vsyasm.vcproj
EXTRA_DIST += Mkfiles/vc9/yasm.sln
EXTRA_DIST += Mkfiles/vc9/yasm.vcproj
EXTRA_DIST += Mkfiles/vc9/ytasm.vcproj
EXTRA_DIST += Mkfiles/vc9/yasm.rules
EXTRA_DIST += Mkfiles/vc9/genmacro/genmacro.vcproj
EXTRA_DIST += Mkfiles/vc9/genmacro/run.bat
EXTRA_DIST += Mkfiles/vc9/genmodule/genmodule.vcproj
EXTRA_DIST += Mkfiles/vc9/genmodule/run.bat
EXTRA_DIST += Mkfiles/vc9/genperf/genperf.vcproj
EXTRA_DIST += Mkfiles/vc9/genperf/run.bat
EXTRA_DIST += Mkfiles/vc9/genstring/genstring.vcproj
EXTRA_DIST += Mkfiles/vc9/genstring/run.bat
EXTRA_DIST += Mkfiles/vc9/genversion/genversion.vcproj
EXTRA_DIST += Mkfiles/vc9/genversion/run.bat
EXTRA_DIST += Mkfiles/vc9/libyasm/libyasm.vcproj
EXTRA_DIST += Mkfiles/vc9/modules/modules.vcproj
EXTRA_DIST += Mkfiles/vc9/re2c/re2c.vcproj
EXTRA_DIST += Mkfiles/vc9/re2c/run.bat
EXTRA_DIST += Mkfiles/vc10/config.h
EXTRA_DIST += Mkfiles/vc10/crt_secure_no_deprecate.props
EXTRA_DIST += Mkfiles/vc10/crt_secure_no_deprecate.vsprops
EXTRA_DIST += Mkfiles/vc10/libyasm-stdint.h
EXTRA_DIST += Mkfiles/vc10/out_copy_rename.bat
EXTRA_DIST += Mkfiles/vc10/readme.vc10.txt
EXTRA_DIST += Mkfiles/vc10/vsyasm.props
EXTRA_DIST += Mkfiles/vc10/vsyasm.targets
EXTRA_DIST += Mkfiles/vc10/vsyasm.vcxproj
EXTRA_DIST += Mkfiles/vc10/vsyasm.xml
EXTRA_DIST += Mkfiles/vc10/yasm.sln
EXTRA_DIST += Mkfiles/vc10/yasm.vcxproj
EXTRA_DIST += Mkfiles/vc10/yasm.vcxproj.filters
EXTRA_DIST += Mkfiles/vc10/ytasm.vcxproj
EXTRA_DIST += Mkfiles/vc10/genmacro/genmacro.vcxproj
EXTRA_DIST += Mkfiles/vc10/genmacro/genmacro.vcxproj.filters
EXTRA_DIST += Mkfiles/vc10/genmacro/run.bat
EXTRA_DIST += Mkfiles/vc10/genmodule/genmodule.vcxproj
EXTRA_DIST += Mkfiles/vc10/genmodule/genmodule.vcxproj.filters
EXTRA_DIST += Mkfiles/vc10/genmodule/run.bat
EXTRA_DIST += Mkfiles/vc10/genperf/genperf.vcxproj
EXTRA_DIST += Mkfiles/vc10/genperf/genperf.vcxproj.filters
EXTRA_DIST += Mkfiles/vc10/genperf/run.bat
EXTRA_DIST += Mkfiles/vc10/genstring/genstring.vcxproj
EXTRA_DIST += Mkfiles/vc10/genstring/genstring.vcxproj.filters
EXTRA_DIST += Mkfiles/vc10/genstring/run.bat
EXTRA_DIST += Mkfiles/vc10/genversion/genversion.vcxproj
EXTRA_DIST += Mkfiles/vc10/genversion/genversion.vcxproj.filters
EXTRA_DIST += Mkfiles/vc10/genversion/run.bat
EXTRA_DIST += Mkfiles/vc10/libyasm/libyasm.vcxproj
EXTRA_DIST += Mkfiles/vc10/libyasm/libyasm.vcxproj.filters
EXTRA_DIST += Mkfiles/vc10/modules/modules.vcxproj
EXTRA_DIST += Mkfiles/vc10/modules/modules.vcxproj.filters
EXTRA_DIST += Mkfiles/vc10/re2c/re2c.vcxproj
EXTRA_DIST += Mkfiles/vc10/re2c/re2c.vcxproj.filters
EXTRA_DIST += Mkfiles/vc10/re2c/run.bat

# Until this gets fixed in automake
DISTCLEANFILES += libyasm/stamp-h libyasm/stamp-h[0-9]*

dist-hook: YASM-VERSION-FILE YASM-VERSION.h
	cp YASM-VERSION-FILE $(distdir)/version
	cp YASM-VERSION.h $(distdir)/Mkfiles/dj/
	cp YASM-VERSION.h $(distdir)/Mkfiles/vc9/
	cp YASM-VERSION.h $(distdir)/Mkfiles/vc10/

YASM-VERSION-FILE: $(top_srcdir)/YASM-VERSION-GEN.sh
	$(top_srcdir)/YASM-VERSION-GEN.sh

distclean-local:
	-rm -rf results
if HAVE_PYTHON
	-rm -rf build
endif

# Until gets fixed libyasm cross-build for target system
# use fixinstall-yasm-XXX make targets
all-local: python-build
install-exec-hook: python-install
uninstall-hook: python-uninstall

if BUILD_MAN
MAINTAINERCLEANFILES += $(dist_man_MANS) $(notrans_dist_man_MANS)
endif

# genstring build
genstring_SOURCES =
EXTRA_DIST += genstring.c
genstring_LDADD = genstring.$(OBJEXT)
genstring_LINK = $(CCLD_FOR_BUILD) -o $@

genstring.$(OBJEXT): genstring.c
	$(CC_FOR_BUILD) $(DEFAULT_INCLUDES) $(INCLUDES) -c -o $@ `test -f genstring.c || echo '$(srcdir)/'`genstring.c

