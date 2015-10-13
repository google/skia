AC_PREREQ([2.64])
AC_INIT([HarfBuzz],
        [0.9.35],
        [http://bugs.freedesktop.org/enter_bug.cgi?product=harfbuzz],
        [harfbuzz],
        [http://harfbuzz.org/])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/harfbuzz.pc.in])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([1.11.1 gnits tar-pax dist-bzip2 no-dist-gzip -Wall no-define color-tests -Wno-portability])
AM_CONDITIONAL(AUTOMAKE_OLDER_THAN_1_13, test $am__api_version = 1.11 -o $am__api_version = 1.12)
AM_SILENT_RULES([yes])

# Initialize libtool
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
LT_PREREQ([2.2])
LT_INIT([disable-static])

# Check for programs
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX
PKG_PROG_PKG_CONFIG([0.20])
AM_MISSING_PROG([RAGEL], [ragel])
AM_MISSING_PROG([GIT], [git])

# Version
m4_define(hb_version_triplet,m4_split(AC_PACKAGE_VERSION,[[.]]))
m4_define(hb_version_major,m4_argn(1,hb_version_triplet))
m4_define(hb_version_minor,m4_argn(2,hb_version_triplet))
m4_define(hb_version_micro,m4_argn(3,hb_version_triplet))
HB_VERSION_MAJOR=hb_version_major
HB_VERSION_MINOR=hb_version_minor
HB_VERSION_MICRO=hb_version_micro
HB_VERSION=AC_PACKAGE_VERSION
AC_SUBST(HB_VERSION_MAJOR)
AC_SUBST(HB_VERSION_MINOR)
AC_SUBST(HB_VERSION_MICRO)
AC_SUBST(HB_VERSION)

# Libtool version
m4_define([hb_version_int],
	  m4_eval(hb_version_major*10000 + hb_version_minor*100 + hb_version_micro))
m4_if(m4_eval(hb_version_minor % 2), [1],
      dnl for unstable releases
      [m4_define([hb_libtool_revision], 0)],
      dnl for stable releases
      [m4_define([hb_libtool_revision], hb_version_micro)])
m4_define([hb_libtool_age],
	  m4_eval(hb_version_int - hb_libtool_revision))
m4_define([hb_libtool_current],
	  m4_eval(hb_version_major + hb_libtool_age))
HB_LIBTOOL_VERSION_INFO=hb_libtool_current:hb_libtool_revision:hb_libtool_age
AC_SUBST(HB_LIBTOOL_VERSION_INFO)

# Documentation
have_gtk_doc=false
m4_ifdef([GTK_DOC_CHECK], [
GTK_DOC_CHECK([1.15],[--flavour no-tmpl])
	if test "x$enable_gtk_doc" = xyes; then
		have_gtk_doc=true
	fi
], [
	AM_CONDITIONAL([ENABLE_GTK_DOC], false)
])

# Functions and headers
AC_CHECK_FUNCS(atexit mprotect sysconf getpagesize mmap isatty)
AC_CHECK_HEADERS(unistd.h sys/mman.h)

# Compiler flags
AC_CANONICAL_HOST
AC_CHECK_ALIGNOF([struct{char;}])
if test "x$GCC" = "xyes"; then

	# Make symbols link locally
	LDFLAGS="$LDFLAGS -Bsymbolic-functions"

	# Make sure we don't link to libstdc++
	CXXFLAGS="$CXXFLAGS -fno-rtti -fno-exceptions"

	# Assorted warnings
	CXXFLAGS="$CXXFLAGS -Wcast-align"

	case "$host" in
		*-*-mingw*)
		;;
		*)
			# Hide inline methods
			CXXFLAGS="$CXXFLAGS -fvisibility-inlines-hidden"
		;;
	esac

	case "$host" in
		arm-*-*)
			if test "x$ac_cv_alignof_struct_char__" != x1; then
				# Request byte alignment
				CXXFLAGS="$CXXFLAGS -mstructure-size-boundary=8"
			fi
		;;
	esac
fi

AM_CONDITIONAL(HAVE_GCC, test "x$GCC" = "xyes")

hb_os_win32=no
AC_MSG_CHECKING([for native Win32])
case "$host" in
  *-*-mingw*)
    hb_os_win32=yes
    ;;
esac
AC_MSG_RESULT([$hb_os_win32])
AM_CONDITIONAL(OS_WIN32, test "$hb_os_win32" = "yes")

have_pthread=false
if test "$hb_os_win32" = no; then
	AX_PTHREAD([have_pthread=true])
fi
if $have_pthread; then
	AC_DEFINE(HAVE_PTHREAD, 1, [Have POSIX threads])
fi
AM_CONDITIONAL(HAVE_PTHREAD, $have_pthread)

dnl ==========================================================================

have_ot=true
if $have_ot; then
	AC_DEFINE(HAVE_OT, 1, [Have native OpenType Layout backend])
fi
AM_CONDITIONAL(HAVE_OT, $have_ot)

have_fallback=true
if $have_fallback; then
	AC_DEFINE(HAVE_FALLBACK, 1, [Have simple TrueType Layout backend])
fi
AM_CONDITIONAL(HAVE_FALLBACK, $have_fallback)

dnl ===========================================================================

AC_ARG_WITH(glib,
	[AS_HELP_STRING([--with-glib=@<:@yes/no/auto@:>@],
			[Use glib @<:@default=auto@:>@])],,
	[with_glib=auto])
have_glib=false
if test "x$with_glib" = "xyes" -o "x$with_glib" = "xauto"; then
	PKG_CHECK_MODULES(GLIB, glib-2.0 >= 2.16, have_glib=true, :)
fi
if test "x$with_glib" = "xyes" -a "x$have_glib" != "xtrue"; then
	AC_MSG_ERROR([glib support requested but glib-2.0 not found])
fi
if $have_glib; then
	AC_DEFINE(HAVE_GLIB, 1, [Have glib2 library])
fi
AM_CONDITIONAL(HAVE_GLIB, $have_glib)

dnl ===========================================================================

AC_ARG_WITH(gobject,
	[AS_HELP_STRING([--with-gobject=@<:@yes/no/auto@:>@],
			[Use gobject @<:@default=auto@:>@])],,
	[with_gobject=no])
have_gobject=false
if test "x$with_gobject" = "xyes" -o "x$with_gobject" = "xauto"; then
	PKG_CHECK_MODULES(GOBJECT, gobject-2.0 glib-2.0, have_gobject=true, :)
fi
if test "x$with_gobject" = "xyes" -a "x$have_gobject" != "xtrue"; then
	AC_MSG_ERROR([gobject support requested but gobject-2.0 / glib-2.0 not found])
fi
if $have_gobject; then
	AC_DEFINE(HAVE_GOBJECT, 1, [Have gobject2 library])
	GLIB_MKENUMS=`$PKG_CONFIG --variable=glib_mkenums glib-2.0`
	AC_SUBST(GLIB_MKENUMS)
fi
AM_CONDITIONAL(HAVE_GOBJECT, $have_gobject)

dnl ===========================================================================


dnl ===========================================================================
# Gobject-Introspection
have_introspection=false
m4_ifdef([GOBJECT_INTROSPECTION_CHECK], [
	if $have_gobject; then
		GOBJECT_INTROSPECTION_CHECK([1.34.0])
		if test "x$found_introspection" = xyes; then
			have_introspection=true
		fi
	else
		AM_CONDITIONAL([HAVE_INTROSPECTION], false)
	fi
], [
	AM_CONDITIONAL([HAVE_INTROSPECTION], false)
])

dnl ===========================================================================

have_ucdn=true
if $have_glib; then
	have_ucdn=false
fi
if $have_ucdn; then
	AC_DEFINE(HAVE_UCDN, 1, [Have UCDN Unicode functions])
fi
AM_CONDITIONAL(HAVE_UCDN, $have_ucdn)

dnl ==========================================================================

AC_ARG_WITH(cairo,
	[AS_HELP_STRING([--with-cairo=@<:@yes/no/auto@:>@],
			[Use cairo @<:@default=auto@:>@])],,
	[with_cairo=auto])
have_cairo=false
if test "x$with_cairo" = "xyes" -o "x$with_cairo" = "xauto"; then
	PKG_CHECK_MODULES(CAIRO, cairo >= 1.8.0, have_cairo=true, :)
fi
if test "x$with_cairo" = "xyes" -a "x$have_cairo" != "xtrue"; then
	AC_MSG_ERROR([cairo support requested but not found])
fi
if $have_cairo; then
	AC_DEFINE(HAVE_CAIRO, 1, [Have cairo graphics library])
fi
AM_CONDITIONAL(HAVE_CAIRO, $have_cairo)

have_cairo_ft=false
if $have_cairo; then
	PKG_CHECK_MODULES(CAIRO_FT, cairo-ft, have_cairo_ft=true, :)
fi
if $have_cairo_ft; then
	AC_DEFINE(HAVE_CAIRO_FT, 1, [Have cairo-ft support in cairo graphics library])
fi
AM_CONDITIONAL(HAVE_CAIRO_FT, $have_cairo_ft)

dnl ==========================================================================

AC_ARG_WITH(icu,
	[AS_HELP_STRING([--with-icu=@<:@yes/no/auto@:>@],
			[Use ICU @<:@default=auto@:>@])],,
	[with_icu=auto])
have_icu=false
if test "x$with_icu" = "xyes" -o "x$with_icu" = "xauto"; then
	PKG_CHECK_MODULES(ICU, icu-uc, have_icu=true, :)

	dnl Fallback to icu-config if ICU pkg-config files could not be found
	if test "$have_icu" != "true"; then
		AC_CHECK_TOOL(ICU_CONFIG, icu-config, no)
		AC_MSG_CHECKING([for ICU by using icu-config fallback])
		if test "$ICU_CONFIG" != "no" && "$ICU_CONFIG" --version >/dev/null; then
			have_icu=true
			# We don't use --cflags as this gives us a lot of things that we don't
			# necessarily want, like debugging and optimization flags
			# See man (1) icu-config for more info.
			ICU_CFLAGS=`$ICU_CONFIG --cppflags`
			ICU_LIBS=`$ICU_CONFIG --ldflags-searchpath --ldflags-libsonly`
			AC_SUBST(ICU_CFLAGS)
			AC_SUBST(ICU_LIBS)
			AC_MSG_RESULT([yes])
		else
			AC_MSG_RESULT([no])
		fi
	fi
fi
if test "x$with_icu" = "xyes" -a "x$have_icu" != "xtrue"; then
	AC_MSG_ERROR([icu support requested but icu-uc not found])
fi
if $have_icu; then
	CXXFLAGS="$CXXFLAGS `$PKG_CONFIG --variable=CXXFLAGS icu-uc`"
	AC_DEFINE(HAVE_ICU, 1, [Have ICU library])
fi
AM_CONDITIONAL(HAVE_ICU, $have_icu)

dnl ==========================================================================

AC_ARG_WITH(graphite2,
	[AS_HELP_STRING([--with-graphite2=@<:@yes/no/auto@:>@],
			[Use the graphite2 library @<:@default=no@:>@])],,
	[with_graphite2=no])
have_graphite2=false
if test "x$with_graphite2" = "xyes" -o "x$with_graphite2" = "xauto"; then
	PKG_CHECK_MODULES(GRAPHITE2, graphite2, have_graphite2=true, :)
fi
if test "x$with_graphite2" = "xyes" -a "x$have_graphite2" != "xtrue"; then
	AC_MSG_ERROR([graphite2 support requested but libgraphite2 not found])
fi
if $have_graphite2; then
    AC_DEFINE(HAVE_GRAPHITE2, 1, [Have Graphite2 library])
fi
AM_CONDITIONAL(HAVE_GRAPHITE2, $have_graphite2)

dnl ==========================================================================

AC_ARG_WITH(freetype,
	[AS_HELP_STRING([--with-freetype=@<:@yes/no/auto@:>@],
			[Use the FreeType library @<:@default=auto@:>@])],,
	[with_freetype=auto])
have_freetype=false
if test "x$with_freetype" = "xyes" -o "x$with_freetype" = "xauto"; then
	PKG_CHECK_MODULES(FREETYPE, freetype2 >= 2.3.8, have_freetype=true, :)
fi
if test "x$with_freetype" = "xyes" -a "x$have_freetype" != "xtrue"; then
	AC_MSG_ERROR([FreeType support requested but libfreetype2 not found])
fi
if $have_freetype; then
	AC_DEFINE(HAVE_FREETYPE, 1, [Have FreeType 2 library])
	_save_libs="$LIBS"
	_save_cflags="$CFLAGS"
	LIBS="$LIBS $FREETYPE_LIBS"
	CFLAGS="$CFLAGS $FREETYPE_CFLAGS"
	AC_CHECK_FUNCS(FT_Face_GetCharVariantIndex)
	LIBS="$_save_libs"
	CFLAGS="$_save_cflags"
fi
AM_CONDITIONAL(HAVE_FREETYPE, $have_freetype)

dnl ===========================================================================

AC_ARG_WITH(uniscribe,
	[AS_HELP_STRING([--with-uniscribe=@<:@yes/no/auto@:>@],
			[Use the Uniscribe library @<:@default=no@:>@])],,
	[with_uniscribe=no])
have_uniscribe=false
if test "x$with_uniscribe" = "xyes" -o "x$with_uniscribe" = "xauto"; then
	AC_CHECK_HEADERS(usp10.h windows.h, have_uniscribe=true)
fi
if test "x$with_uniscribe" = "xyes" -a "x$have_uniscribe" != "xtrue"; then
	AC_MSG_ERROR([uniscribe support requested but not found])
fi
if $have_uniscribe; then
	UNISCRIBE_CFLAGS=
	UNISCRIBE_LIBS="-lusp10 -lgdi32 -lrpcrt4"
	AC_SUBST(UNISCRIBE_CFLAGS)
	AC_SUBST(UNISCRIBE_LIBS)
	AC_DEFINE(HAVE_UNISCRIBE, 1, [Have Uniscribe library])
fi
AM_CONDITIONAL(HAVE_UNISCRIBE, $have_uniscribe)

dnl ===========================================================================

AC_ARG_WITH(coretext,
	[AS_HELP_STRING([--with-coretext=@<:@yes/no/auto@:>@],
			[Use CoreText @<:@default=no@:>@])],,
	[with_coretext=no])
have_coretext=false
if test "x$with_coretext" = "xyes" -o "x$with_coretext" = "xauto"; then
	AC_CHECK_TYPE(CTFontRef, have_coretext=true,, [#include <ApplicationServices/ApplicationServices.h>])

	if $have_coretext; then
		CORETEXT_CFLAGS=
		CORETEXT_LIBS="-framework ApplicationServices"
		AC_SUBST(CORETEXT_CFLAGS)
		AC_SUBST(CORETEXT_LIBS)
	else
		# On iOS CoreText and CoreGraphics are stand-alone frameworks
		if test "x$have_coretext" != "xtrue"; then
			AC_CHECK_TYPE(CTFontRef, have_coretext=true,, [#include <CoreText/CoreText.h>])
		fi

		if $have_coretext; then
			CORETEXT_CFLAGS=
			CORETEXT_LIBS="-framework CoreText -framework CoreGraphics"
			AC_SUBST(CORETEXT_CFLAGS)
			AC_SUBST(CORETEXT_LIBS)
		fi
	fi
fi
if test "x$with_coretext" = "xyes" -a "x$have_coretext" != "xtrue"; then
	AC_MSG_ERROR([CoreText support requested but libcoretext not found])
fi
if $have_coretext; then
	AC_DEFINE(HAVE_CORETEXT, 1, [Have Core Text backend])
fi
AM_CONDITIONAL(HAVE_CORETEXT, $have_coretext)

dnl ===========================================================================

AC_CACHE_CHECK([for Intel atomic primitives], hb_cv_have_intel_atomic_primitives, [
	hb_cv_have_intel_atomic_primitives=false
	AC_TRY_LINK([
		void memory_barrier (void) { __sync_synchronize (); }
		int atomic_add (int *i) { return __sync_fetch_and_add (i, 1); }
		int mutex_trylock (int *m) { return __sync_lock_test_and_set (m, 1); }
		void mutex_unlock (int *m) { __sync_lock_release (m); }
		], [], hb_cv_have_intel_atomic_primitives=true
	)
])
if $hb_cv_have_intel_atomic_primitives; then
	AC_DEFINE(HAVE_INTEL_ATOMIC_PRIMITIVES, 1, [Have Intel __sync_* atomic primitives])
fi

dnl ===========================================================================

AC_CACHE_CHECK([for Solaris atomic operations], hb_cv_have_solaris_atomic_ops, [
	hb_cv_have_solaris_atomic_ops=false
	AC_TRY_LINK([
		#include <atomic.h>
		/* This requires Solaris Studio 12.2 or newer: */
		#include <mbarrier.h>
		void memory_barrier (void) { __machine_rw_barrier (); }
		int atomic_add (volatile unsigned *i) { return atomic_add_int_nv (i, 1); }
		void *atomic_ptr_cmpxchg (volatile void **target, void *cmp, void *newval) { return atomic_cas_ptr (target, cmp, newval); }
		], [], hb_cv_have_solaris_atomic_ops=true
	)
])
if $hb_cv_have_solaris_atomic_ops; then
	AC_DEFINE(HAVE_SOLARIS_ATOMIC_OPS, 1, [Have Solaris __machine_*_barrier and atomic_* operations])
fi

if test "$os_win32" = no && ! $have_pthread; then
	AC_CHECK_HEADERS(sched.h)
	AC_SEARCH_LIBS(sched_yield,rt,AC_DEFINE(HAVE_SCHED_YIELD, 1, [Have sched_yield]))
fi

dnl ===========================================================================

AC_CONFIG_FILES([
Makefile
src/Makefile
src/hb-version.h
src/hb-ucdn/Makefile
util/Makefile
test/Makefile
test/api/Makefile
test/shaping/Makefile
docs/Makefile
docs/reference/Makefile
docs/reference/version.xml
])

AC_OUTPUT

AC_MSG_NOTICE([

Build configuration:

Unicode callbacks (you want at least one):
	Glib:			${have_glib}
	ICU:			${have_icu}
	UCDN:			${have_ucdn}

Font callbacks (the more the better):
	FreeType:		${have_freetype}

Tools used for command-line utilities:
	Cairo:			${have_cairo}

Additional shapers (the more the better):
	Graphite2:		${have_graphite2}

Platform shapers (not normally needed):
	CoreText:		${have_coretext}
	Uniscribe:		${have_uniscribe}

Other features:
	Documentation:		${have_gtk_doc}
	GObject bindings:	${have_gobject}
	Introspection:		${have_introspection}
])
