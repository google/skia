# Process this file with automake to produce Makefile.in

NULL =
EXTRA_DIST =
CLEANFILES =
DISTCLEANFILES =
MAINTAINERCLEANFILES =

if HAVE_GLIB
AM_CPPFLAGS = -DSRCDIR="\"$(srcdir)\"" -I$(top_srcdir)/src/ -I$(top_builddir)/src/ $(GLIB_CFLAGS)
LDADD = $(top_builddir)/src/libharfbuzz.la $(GLIB_LIBS)

EXTRA_DIST += hb-test.h

check_PROGRAMS = $(TEST_PROGS)
noinst_PROGRAMS = $(TEST_PROGS)

TEST_PROGS = \
	test-blob \
	test-buffer \
	test-common \
	test-font \
	test-object \
	test-set \
	test-shape \
	test-unicode \
	test-version \
	$(NULL)

test_unicode_CPPFLAGS = $(AM_CPPFLAGS)
test_unicode_LDADD = $(LDADD)
if HAVE_GLIB
test_unicode_CPPFLAGS += $(GLIB_CFLAGS)
endif
if HAVE_ICU
test_unicode_CPPFLAGS += $(ICU_CFLAGS)
test_unicode_LDADD += $(top_builddir)/src/libharfbuzz-icu.la
endif


if HAVE_OT
TEST_PROGS += \
	test-ot-tag \
	$(NULL)
endif

# Tests for header compilation
TEST_PROGS += \
	test-c \
	test-cplusplus \
	$(NULL)
test_cplusplus_SOURCES = test-cplusplus.cc
test_c_CPPFLAGS = $(AM_CPPFLAGS)
test_cplusplus_CPPFLAGS = $(AM_CPPFLAGS)
if HAVE_ICU
test_c_CPPFLAGS += $(ICU_CFLAGS)
test_cplusplus_CPPFLAGS += $(ICU_CFLAGS)
endif
if HAVE_FREETYPE
test_c_CPPFLAGS += $(FREETYPE_CFLAGS)
test_cplusplus_CPPFLAGS += $(FREETYPE_CFLAGS)
endif

# Default test running environment
TESTS = $(TEST_PROGS)
TESTS_ENVIRONMENT = \
	MALLOC_CHECK_=2 \
	MALLOC_PERTURB_=$$(($${RANDOM:-256} % 256)) \
	G_DEBUG=gc-friendly \
	G_SLICE=always-malloc \
	srcdir=$(srcdir) \
	$(NULL)


# check-tool: Run tests under $(TOOL)
check-tool:
	$(AM_V_at)$(MAKE) $(AM_MAKEFLAGS) check \
	TESTS_ENVIRONMENT='$(TESTS_ENVIRONMENT) $(top_builddir)/libtool --mode=execute \
	env $(TOOL)'
# check-tool-raw: Run tests under $(TOOL), but don't run under libtool
check-tool-raw:
	$(AM_V_at)$(MAKE) $(AM_MAKEFLAGS) check \
	TESTS_ENVIRONMENT='$(TESTS_ENVIRONMENT) \
	env $(TOOL)'

# check-gtester: Run tests under gtester
GTESTER = gtester
check-gtester:
	$(AM_V_at)$(MAKE) $(AM_MAKEFLGS) check-tool-raw TOOL="$(GTESTER) --verbose --keep-going"


# Check tests under valgrind.  Saves log to log-valgrind.txt
VALGRIND_FLAGS = \
	--tool=memcheck --suppressions=$(srcdir)/.valgrind-suppressions \
	--track-origins=yes \
	--leak-check=yes
	$(EXTRA_VALGRIND_FLAGS)
#	Can't do for now: --show-reachable=yes
CLEANFILES +=  log-valgrind.txt
valgrind_verbose = $(valgrind_verbose_@AM_V@)
valgrind_verbose_ = $(valgrind_verbose_@AM_DEFAULT_V@)
valgrind_verbose_0 = | \
	grep '\(^[^=]\|ERROR SUMMARY\|definitely lost\|indirectly lost\)' | grep -v ': 0'
# TODO: The following check does not fail if valgrind finds error.  It should.
check-valgrind:
	$(AM_V_at)$(MAKE) $(AM_MAKEFLGS) check-tool TOOL="valgrind $(VALGRIND_FLAGS)" \
	2>&1 | tee log-valgrind.txt $(valgrind_verbose)


# check-symbols: Finds untested API symbols
symbols-tested.txt: $(TEST_PROGS)
	$(AM_V_GEN)$(top_builddir)/libtool --mode=execute nm $^ \
	| grep ' U hb_' | sed 's/.* U hb_/hb_/' \
	| sort | uniq > $@.tmp && mv $@.tmp $@
symbols-exported.txt: $(top_builddir)/src/.libs/libharfbuzz.so
	$(AM_V_GEN)$(top_builddir)/libtool --mode=execute nm $^ \
	| grep ' T ' | sed 's/.* T //' | grep -v '^\(_init\|_fini\)$$' \
	| sort | uniq > $@.tmp && mv $@.tmp $@
symbols-untested.txt: symbols-tested.txt symbols-exported.txt
	$(AM_V_GEN)diff $^ > $@.tmp; mv $@.tmp $@
CLEANFILES += symbols-tested.txt symbols-exported.txt symbols-untested.txt
check-symbols: symbols-untested.txt
	@! cat $^ | grep .



else
check-am: err-glib
err-glib:
	@echo "You need to have glib support enabled to run the tests"
	@exit 77
endif

.PHONY: check-symbols check-tool check-valgrind

-include $(top_srcdir)/git.mk
