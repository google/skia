#include "YASM-VERSION.h"
#define yasm__splitpath(path, tail)     yasm__splitpath_win(path, tail)
#define yasm__abspath(path)             yasm__abspath_win(path)
#define yasm__combpath(from, to)        yasm__combpath_win(from, to)

/* Command name to run C preprocessor */
#define CPP_PROG "cpp"

/* */
/* #undef ENABLE_NLS */

/* Define if you have the `abort' function. */
#define HAVE_ABORT 1

/* */
/* #undef HAVE_CATGETS */

/* Define to 1 if you have the MacOS X function CFLocaleCopyCurrent in the
   CoreFoundation framework. */
/* #undef HAVE_CFLOCALECOPYCURRENT */

/* Define to 1 if you have the MacOS X function CFPreferencesCopyAppValue in
   the CoreFoundation framework. */
/* #undef HAVE_CFPREFERENCESCOPYAPPVALUE */

/* Define if the GNU dcgettext() function is already present or preinstalled.
   */
/* #undef HAVE_DCGETTEXT */

/* Define to 1 if you have the <direct.h> header file. */
#define HAVE_DIRECT_H 1

/* Define to 1 if you have the `getcwd' function. */
#define HAVE_GETCWD 1

/* */
/* #undef HAVE_GETTEXT */

/* Define if you have the GNU C Library */
/* #undef HAVE_GNU_C_LIBRARY */

/* Define if you have the iconv() function. */
/* #undef HAVE_ICONV */

/* Define if you have the <inttypes.h> header file. */
/* #undef HAVE_INTTYPES_H */

/* */
/* #undef HAVE_LC_MESSAGES */

/* Define to 1 if you have the <libgen.h> header file. */
/* #undef HAVE_LIBGEN_H */

/* Define if you have the <memory.h> header file. */
/* #undef HAVE_MEMORY_H */

/* Define if you have the `mergesort' function. */
/* #undef HAVE_MERGESORT */

/* Define to 1 if you have the `popen' function. */
/* #undef HAVE_POPEN */

/* Define if you have the <stdint.h> header file. */
/* #undef HAVE_STDINT_H */

/* Define if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* */
/* #undef HAVE_STPCPY */

/* Define if you have the `strcasecmp' function. */
/* #undef HAVE_STRCASECMP */

/* Define if you have the `strcmpi' function. */
/* #undef HAVE_STRCMPI */

/* Define if you have the `stricmp' function. */
/* #undefine HAVE_STRICMP */

/* Define if you have the <strings.h> header file. */
/* #undef HAVE_STRINGS_H */

/* Define if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define if you have the `strncasecmp' function. */
/* #undef HAVE_STRNCASECMP */

/* Define if you have the `strsep' function. */
/* #undef HAVE_STRSEP */

/* Define if you have the <sys/stat.h> header file. */
/* #undef HAVE_SYS_STAT_H */

/* Define if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define if you have the `toascii' function. */
#define HAVE_TOASCII 1
#define toascii __toascii

/* Define if you have the <unistd.h> header file. */
/* #undef HAVE_UNISTD_H */

/* Define to 1 if you have the `vsnprintf' function. */
/* #undef HAVE_VSNPRINTF */

/* Define to 1 if you have the `_stricmp' function. */
#define HAVE__STRICMP 1

/* Name of package */
#define PACKAGE "yasm"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "bug-yasm@tortall.net"

/* Define to the full name of this package. */
#define PACKAGE_NAME "yasm"

/* Define to the full name and version of this package. */
/*#define PACKAGE_STRING "yasm HEAD"*/

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "yasm"

/* Define to the version of this package. */
/*#define PACKAGE_VERSION "HEAD"*/

/* Define if the C compiler supports function prototypes. */
#define PROTOTYPES 1

/* The size of a `char', as computed by sizeof. */
/* #undef SIZEOF_CHAR */

/* The size of a `int', as computed by sizeof. */
/* #undef SIZEOF_INT */

/* The size of a `long', as computed by sizeof. */
/* #undef SIZEOF_LONG */

/* The size of a `short', as computed by sizeof. */
/* #undef SIZEOF_SHORT */

/* The size of a `void*', as computed by sizeof. */
/* #undef SIZEOF_VOIDP */

/* Define if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Version number of package */
#define VERSION "HEAD"

/* Define if using the dmalloc debugging malloc package */
/* #undef WITH_DMALLOC */

/* Define like PROTOTYPES; this can be used by system headers. */
#define __PROTOTYPES 1

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define as `__inline' if that's what the C compiler calls it, or to nothing
   if it is not supported. */
#ifndef __cplusplus
/* #undef inline */
#endif

/* Define to `unsigned' if <sys/types.h> doesn't define. */
/* #undef size_t */
