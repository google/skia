# Process this file with automake to produce Makefile.in

NULL =
SUBDIRS =
DIST_SUBDIRS =
BUILT_SOURCES =
EXTRA_DIST =
CLEANFILES =
DISTCLEANFILES =
MAINTAINERCLEANFILES =
DISTCHECK_CONFIGURE_FLAGS = --enable-introspection

# The following warning options are useful for debugging: -Wpadded
#AM_CXXFLAGS =

lib_LTLIBRARIES = libharfbuzz.la

HBCFLAGS =
HBLIBS =
HBSOURCES =  \
	hb-atomic-private.hh \
	hb-blob.cc \
	hb-buffer-deserialize-json.hh \
	hb-buffer-deserialize-text.hh \
	hb-buffer-private.hh \
	hb-buffer-serialize.cc \
	hb-buffer.cc \
	hb-cache-private.hh \
	hb-common.cc \
	hb-face-private.hh \
	hb-face.cc \
	hb-font-private.hh \
	hb-font.cc \
	hb-mutex-private.hh \
	hb-object-private.hh \
	hb-open-file-private.hh \
	hb-open-type-private.hh \
	hb-ot-cmap-table.hh \
	hb-ot-head-table.hh \
	hb-ot-hhea-table.hh \
	hb-ot-hmtx-table.hh \
	hb-ot-maxp-table.hh \
	hb-ot-name-table.hh \
	hb-ot-tag.cc \
	hb-private.hh \
	hb-set-private.hh \
	hb-set.cc \
	hb-shape.cc \
	hb-shape-plan-private.hh \
	hb-shape-plan.cc \
	hb-shaper-list.hh \
	hb-shaper-impl-private.hh \
	hb-shaper-private.hh \
	hb-shaper.cc \
	hb-unicode-private.hh \
	hb-unicode.cc \
	hb-utf-private.hh \
	hb-warning.cc \
	$(NULL)
HBHEADERS = \
	hb.h \
	hb-blob.h \
	hb-buffer.h \
	hb-common.h \
	hb-deprecated.h \
	hb-face.h \
	hb-font.h \
	hb-set.h \
	hb-shape.h \
	hb-shape-plan.h \
	hb-unicode.h \
	$(NULL)
HBNODISTHEADERS = \
	hb-version.h \
	$(NULL)

if HAVE_OT
HBSOURCES += \
	hb-ot-font.cc \
	hb-ot-layout.cc \
	hb-ot-layout-common-private.hh \
	hb-ot-layout-gdef-table.hh \
	hb-ot-layout-gpos-table.hh \
	hb-ot-layout-gsubgpos-private.hh \
	hb-ot-layout-gsub-table.hh \
	hb-ot-layout-jstf-table.hh \
	hb-ot-layout-private.hh \
	hb-ot-map.cc \
	hb-ot-map-private.hh \
	hb-ot-shape.cc \
	hb-ot-shape-complex-arabic.cc \
	hb-ot-shape-complex-arabic-fallback.hh \
	hb-ot-shape-complex-arabic-table.hh \
	hb-ot-shape-complex-arabic-win1256.hh \
	hb-ot-shape-complex-default.cc \
	hb-ot-shape-complex-hangul.cc \
	hb-ot-shape-complex-hebrew.cc \
	hb-ot-shape-complex-indic.cc \
	hb-ot-shape-complex-indic-machine.hh \
	hb-ot-shape-complex-indic-private.hh \
	hb-ot-shape-complex-indic-table.cc \
	hb-ot-shape-complex-myanmar.cc \
	hb-ot-shape-complex-myanmar-machine.hh \
	hb-ot-shape-complex-sea.cc \
	hb-ot-shape-complex-sea-machine.hh \
	hb-ot-shape-complex-thai.cc \
	hb-ot-shape-complex-tibetan.cc \
	hb-ot-shape-complex-private.hh \
	hb-ot-shape-normalize-private.hh \
	hb-ot-shape-normalize.cc \
	hb-ot-shape-fallback-private.hh \
	hb-ot-shape-fallback.cc \
	hb-ot-shape-private.hh \
	$(NULL)
HBHEADERS += \
	hb-ot.h \
	hb-ot-font.h \
	hb-ot-layout.h \
	hb-ot-shape.h \
	hb-ot-tag.h \
	$(NULL)
endif

if HAVE_FALLBACK
HBSOURCES += hb-fallback-shape.cc
endif

if HAVE_PTHREAD
HBCFLAGS += $(PTHREAD_CFLAGS)
HBLIBS   += $(PTHREAD_LIBS)
endif

if HAVE_GLIB
HBCFLAGS += $(GLIB_CFLAGS)
HBLIBS   += $(GLIB_LIBS)
HBSOURCES += hb-glib.cc
HBHEADERS += hb-glib.h
endif

if HAVE_FREETYPE
HBCFLAGS += $(FREETYPE_CFLAGS)
HBLIBS   += $(FREETYPE_LIBS)
HBSOURCES += hb-ft.cc
HBHEADERS += hb-ft.h
endif

if HAVE_GRAPHITE2
HBCFLAGS += $(GRAPHITE2_CFLAGS)
HBLIBS   += $(GRAPHITE2_LIBS)
HBSOURCES += hb-graphite2.cc
HBHEADERS += hb-graphite2.h
endif

if HAVE_UNISCRIBE
HBCFLAGS += $(UNISCRIBE_CFLAGS)
HBLIBS   += $(UNISCRIBE_LIBS)
HBSOURCES += hb-uniscribe.cc
HBHEADERS += hb-uniscribe.h
endif

if HAVE_CORETEXT
HBCFLAGS += $(CORETEXT_CFLAGS)
HBLIBS   += $(CORETEXT_LIBS)
HBSOURCES += hb-coretext.cc
HBHEADERS += hb-coretext.h
endif

if HAVE_UCDN
SUBDIRS += hb-ucdn
HBCFLAGS += -I$(srcdir)/hb-ucdn
HBLIBS   += hb-ucdn/libhb-ucdn.la
HBSOURCES += hb-ucdn.cc
endif
DIST_SUBDIRS += hb-ucdn


# Put the library together

if OS_WIN32
export_symbols = -export-symbols harfbuzz.def
harfbuzz_def_dependency = harfbuzz.def
libharfbuzz_la_LINK = $(CXXLINK) $(libharfbuzz_la_LDFLAGS)
else
# Use a C linker for GCC, not C++; Don't link to libstdc++
if HAVE_GCC
libharfbuzz_la_LINK = $(LINK) $(libharfbuzz_la_LDFLAGS)
else
libharfbuzz_la_LINK = $(CXXLINK) $(libharfbuzz_la_LDFLAGS)
endif
endif

libharfbuzz_la_SOURCES = $(HBSOURCES) $(HBHEADERS) $(HBNODISTHEADERS)
libharfbuzz_la_CPPFLAGS = $(HBCFLAGS)
libharfbuzz_la_LDFLAGS = $(AM_LDFLAGS) -version-info $(HB_LIBTOOL_VERSION_INFO) $(export_symbols) -no-undefined
libharfbuzz_la_LIBADD = $(HBLIBS)
EXTRA_libharfbuzz_la_DEPENDENCIES = $(harfbuzz_def_dependency)
pkginclude_HEADERS = $(HBHEADERS)
nodist_pkginclude_HEADERS = $(HBNODISTHEADERS)
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = harfbuzz.pc
EXTRA_DIST += harfbuzz.pc.in

if HAVE_ICU
lib_LTLIBRARIES += libharfbuzz-icu.la
libharfbuzz_icu_la_SOURCES = hb-icu.cc
libharfbuzz_icu_la_CPPFLAGS = $(ICU_CFLAGS)
libharfbuzz_icu_la_LDFLAGS = $(AM_LDFLAGS) -version-info $(HB_LIBTOOL_VERSION_INFO) -no-undefined
libharfbuzz_icu_la_LIBADD = $(ICU_LIBS) libharfbuzz.la
pkginclude_HEADERS += hb-icu.h
pkgconfig_DATA += harfbuzz-icu.pc
endif
EXTRA_DIST += harfbuzz-icu.pc.in

if HAVE_GOBJECT
lib_LTLIBRARIES += libharfbuzz-gobject.la
libharfbuzz_gobject_la_SOURCES = hb-gobject-structs.cc
nodist_libharfbuzz_gobject_la_SOURCES = hb-gobject-enums.cc
libharfbuzz_gobject_la_CPPFLAGS = $(GOBJECT_CFLAGS)
libharfbuzz_gobject_la_LDFLAGS = $(AM_LDFLAGS) -version-info $(HB_LIBTOOL_VERSION_INFO) -no-undefined
libharfbuzz_gobject_la_LIBADD = $(GOBJECT_LIBS) libharfbuzz.la
pkginclude_HEADERS += hb-gobject.h hb-gobject-structs.h
nodist_pkginclude_HEADERS += hb-gobject-enums.h
pkgconfig_DATA += harfbuzz-gobject.pc

BUILT_SOURCES += \
	hb-gobject-enums.cc \
	hb-gobject-enums.h \
	$(NULL)
DISTCLEANFILES += \
	hb-gobject-enums.cc \
	hb-gobject-enums.h \
	$(NULL)
hb-gobject-enums.%: hb-gobject-enums.%.tmpl $(HBHEADERS)
	$(AM_V_GEN) $(GLIB_MKENUMS) \
		--identifier-prefix hb_ --symbol-prefix hb_gobject \
		--template $^ | \
	sed 's/_t_get_type/_get_type/g; s/_T (/ (/g' > "$@.tmp" && \
	mv "$@.tmp" "$@" || ( $(RM) "@.tmp" && false )
endif
EXTRA_DIST += \
	harfbuzz-gobject.pc.in \
	hb-gobject-enums.cc.tmpl \
	hb-gobject-enums.h.tmpl \
	$(NULL)


%.pc: %.pc.in $(top_builddir)/config.status
	$(AM_V_GEN) \
	$(SED)	-e 's@%prefix%@$(prefix)@g' \
		-e 's@%exec_prefix%@$(exec_prefix)@g' \
		-e 's@%libdir%@$(libdir)@g' \
		-e 's@%includedir%@$(includedir)@g' \
		-e 's@%VERSION%@$(VERSION)@g' \
	"$<" \
	> "$@.tmp" && mv "$@.tmp" "$@" || ( $(RM) "$@.tmp"; false )

CLEANFILES += $(pkgconfig_DATA)


CLEANFILES += harfbuzz.def
harfbuzz.def: $(HBHEADERS) $(HBNODISTHEADERS)
	$(AM_V_GEN) (echo EXPORTS; \
	(cat $^ || echo 'hb_ERROR ()' ) | \
	$(EGREP) '^hb_.* \(' | \
	sed -e 's/ (.*//' | \
	LANG=C sort; \
	echo LIBRARY libharfbuzz-$(HB_VERSION_MAJOR).dll; \
	) >"$@.tmp"
	@ ! grep -q hb_ERROR "$@.tmp" && mv "$@.tmp" "$@" || ($(RM) "$@"; false)


GENERATORS = \
	gen-arabic-table.py \
	gen-indic-table.py \
	$(NULL)
EXTRA_DIST += $(GENERATORS)

unicode-tables: arabic-table indic-table

indic-table: gen-indic-table.py IndicSyllabicCategory.txt IndicMatraCategory.txt Blocks.txt
	$(AM_V_GEN) $(builddir)/$^ > hb-ot-shape-complex-indic-table.cc.tmp && \
	mv hb-ot-shape-complex-indic-table.cc.tmp $(srcdir)/hb-ot-shape-complex-indic-table.cc || \
	($(RM) hb-ot-shape-complex-indic-table.cc.tmp; false)

arabic-table: gen-arabic-table.py ArabicShaping.txt UnicodeData.txt Blocks.txt
	$(AM_V_GEN) $(builddir)/$^ > hb-ot-shape-complex-arabic-table.hh.tmp && \
	mv hb-ot-shape-complex-arabic-table.hh.tmp $(srcdir)/hb-ot-shape-complex-arabic-table.hh || \
	($(RM) hb-ot-shape-complex-arabic-table.hh.tmp; false)

built-sources: $(BUILT_SOURCES)

.PHONY: unicode-tables arabic-table indic-table built-sources

BUILT_SOURCES += \
	hb-buffer-deserialize-json.hh \
	hb-buffer-deserialize-text.hh \
	hb-ot-shape-complex-indic-machine.hh \
	hb-ot-shape-complex-myanmar-machine.hh \
	hb-ot-shape-complex-sea-machine.hh \
	$(NULL)
EXTRA_DIST += \
	hb-buffer-deserialize-json.rl \
	hb-buffer-deserialize-text.rl \
	hb-ot-shape-complex-indic-machine.rl \
	hb-ot-shape-complex-myanmar-machine.rl \
	hb-ot-shape-complex-sea-machine.rl \
	$(NULL)
.rl.hh:
	$(AM_V_GEN)$(RAGEL) -e -F1 -o "$@.tmp" "$<" && \
	mv "$@.tmp" "$@" || ( $(RM) "$@.tmp" && false )

noinst_PROGRAMS = \
	main \
	test \
	test-buffer-serialize \
	test-size-params \
	test-would-substitute \
	$(NULL)
bin_PROGRAMS =

main_SOURCES = main.cc
main_CPPFLAGS = $(HBCFLAGS)
main_LDADD = libharfbuzz.la $(HBLIBS)

test_SOURCES = test.cc
test_CPPFLAGS = $(HBCFLAGS) $(FREETYPE_CFLAGS)
test_LDADD = libharfbuzz.la $(HBLIBS) $(FREETYPE_LIBS)

test_would_substitute_SOURCES = test-would-substitute.cc
test_would_substitute_CPPFLAGS = $(HBCFLAGS) $(FREETYPE_CFLAGS)
test_would_substitute_LDADD = libharfbuzz.la $(HBLIBS) $(FREETYPE_LIBS)

test_size_params_SOURCES = test-size-params.cc
test_size_params_CPPFLAGS = $(HBCFLAGS)
test_size_params_LDADD = libharfbuzz.la $(HBLIBS)

test_buffer_serialize_SOURCES = test-buffer-serialize.cc
test_buffer_serialize_CPPFLAGS = $(HBCFLAGS)
test_buffer_serialize_LDADD = libharfbuzz.la $(HBLIBS)

dist_check_SCRIPTS = \
	check-c-linkage-decls.sh \
	check-defs.sh \
	check-header-guards.sh \
	check-includes.sh \
	check-libstdc++.sh \
	check-static-inits.sh \
	check-symbols.sh \
	$(NULL)

TESTS = $(dist_check_SCRIPTS)
TESTS_ENVIRONMENT = \
	srcdir="$(srcdir)" \
	MAKE="$(MAKE) $(AM_MAKEFLAGS)" \
	HBSOURCES="$(HBSOURCES)" \
	HBHEADERS="$(HBHEADERS) $(HBNODISTHEADERS)" \
	$(NULL)

if HAVE_INTROSPECTION

-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS = HarfBuzz-$(HB_VERSION_MAJOR).0.gir # What does the 0 mean anyway?!
INTROSPECTION_SCANNER_ARGS = -I$(srcdir) -n hb --identifier-prefix=hb_ --warn-all
INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir)
INTROSPECTION_SCANNER_ENV = CC="$(CC)"

HarfBuzz-0.0.gir: libharfbuzz.la libharfbuzz-gobject.la
HarfBuzz_0_0_gir_INCLUDES = GObject-2.0
HarfBuzz_0_0_gir_CFLAGS = \
	$(INCLUDES) \
	$(HBCFLAGS) \
	-DHB_H \
	-DHB_H_IN \
	-DHB_OT_H \
	-DHB_OT_H_IN \
	-DHB_GOBJECT_H \
	-DHB_GOBJECT_H_IN \
	$(NULL)
HarfBuzz_0_0_gir_LIBS = \
	libharfbuzz.la \
	libharfbuzz-gobject.la \
	$(NULL)
HarfBuzz_0_0_gir_FILES = \
	$(HBHEADERS) \
	$(HBNODISTHEADERS) \
	$(HBSOURCES) \
	hb-gobject-enums.cc \
	hb-gobject-enums.h \
	hb-gobject-structs.cc \
	hb-gobject-structs.h \
	$(NULL)

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(gir_DATA) $(typelib_DATA)

endif

-include $(top_srcdir)/git.mk
