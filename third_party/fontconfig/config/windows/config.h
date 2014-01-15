/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.in by autoheader.  */

#include <direct.h>
#ifndef _S_IFDIR
#define	_S_IFDIR	0x4000
#endif
#ifndef S_IFDIR
#define	S_IFDIR _S_IFDIR
#endif
#ifndef _S_IFMT
#define	_S_IFMT		0xF000
#endif
#ifndef S_IFMT
#define	S_IFMT _S_IFMT
#endif

#ifndef S_ISDIR
#   ifdef S_IFDIR
#       define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#   else
#       define S_ISDIR(m) 0
#   endif
#endif /* !S_ISDIR */
#ifndef F_OK
#    define F_OK 0x00
#endif
#ifndef X_OK
#    define X_OK 0x01
#endif
#ifndef W_OK
#    define W_OK 0x02
#endif

#ifdef __WIN64
#define SIZEOF_VOID_P 4
#else
#define SIZEOF_VOID_P 8
#endif

#define snprintf _snprintf
/* Font configuration directory */
#define CONFDIR "c:/windows/etc"

/* Use libxml2 instead of Expat */
/* #undef ENABLE_LIBXML2 */

/* Additional font directories */
#define FC_ADD_FONTS "yes"

/* System font directory */
#define FC_DEFAULT_FONTS "c:/windows/fonts"

#define FC_CACHEDIR "c:/temp"

/* Define to 1 if you have the `chsize' function. */
#define HAVE_CHSIZE 1

/* Define to 1 if you have the <dirent.h> header file, and it defines `DIR'.
   */
#define HAVE_DIRENT_H 1

/* Define to 1 if you have the <dlfcn.h> header file. */
/* #undef HAVE_DLFCN_H */

/* Define to 1 if you don't have `vprintf' but do have `_doprnt.' */
/* #undef HAVE_DOPRNT */

/* Found a useable expat library */
#define HAVE_EXPAT 1

#define FLEXIBLE_ARRAY_MEMBER 1

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H 1

/* Define to 1 if you have the `ftruncate' function. */
/*#define HAVE_FTRUNCATE 1*/

/* FT_Bitmap_Size structure includes y_ppem field */
#define HAVE_FT_BITMAP_SIZE_Y_PPEM 1

/* Define to 1 if you have the `FT_Get_BDF_Property' function. */
#define HAVE_FT_GET_BDF_PROPERTY 1

/* Define to 1 if you have the `FT_Get_Next_Char' function. */
#define HAVE_FT_GET_NEXT_CHAR 1

/* Define to 1 if you have the `FT_Get_PS_Font_Info' function. */
#define HAVE_FT_GET_PS_FONT_INFO 1

/* Define to 1 if you have the `FT_Get_X11_Font_Format' function. */
#define HAVE_FT_GET_X11_FONT_FORMAT 1

/* Define to 1 if you have the `FT_Has_PS_Glyph_Names' function. */
#define HAVE_FT_HAS_PS_GLYPH_NAMES 1

/* Define to 1 if you have the `FT_Select_Size' function. */
#define HAVE_FT_SELECT_SIZE 1

/* Define to 1 if you have the `geteuid' function. */
/* #undef HAVE_GETEUID */

/* Define to 1 if you have the `getopt' function. */
/*#define HAVE_GETOPT 1*/

/* Define to 1 if you have the `getopt_long' function. */
/*#define HAVE_GETOPT_LONG 1*/

/* Define to 1 if you have the `getpagesize' function. */
/*#define HAVE_GETPAGESIZE 1*/

/* Define to 1 if you have the `getuid' function. */
/* #undef HAVE_GETUID */

/* Define to 1 if you have the <inttypes.h> header file. */
/*#define HAVE_INTTYPES_H 1*/

/* Define to 1 if you have the `link' function. */
/* #undef HAVE_LINK */

/* Define to 1 if you have the `lrand48' function. */
/* #undef HAVE_LRAND48 */

/* Define to 1 if you have the `memmove' function. */
#define HAVE_MEMMOVE 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the `memset' function. */
#define HAVE_MEMSET 1

/* Define to 1 if you have the `mkstemp' function. */
/* #undef HAVE_MKSTEMP */

/* Define to 1 if you have a working `mmap' system call. */
/* #undef HAVE_MMAP */

/* Define to 1 if you have the <ndir.h> header file, and it defines `DIR'. */
/* #undef HAVE_NDIR_H */

/* Define to 1 if you have the `rand' function. */
#define HAVE_RAND 1

/* Define to 1 if you have the `random' function. */
/* #undef HAVE_RANDOM */

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `strchr' function. */
#define HAVE_STRCHR 1

/* Define to 1 if you have the <strings.h> header file. */
/*#define HAVE_STRINGS_H 1*/

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the `strrchr' function. */
#define HAVE_STRRCHR 1

/* Define to 1 if you have the `strtol' function. */
#define HAVE_STRTOL 1

/* Define to 1 if you have the `sysconf' function. */
/* #undef HAVE_SYSCONF */

/* Define to 1 if you have the <sys/dir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_DIR_H */

/* Define to 1 if you have the <sys/ndir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_NDIR_H */

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
/*#define HAVE_UNISTD_H 1*/

/* Define to 1 if you have the `vprintf' function. */
#define HAVE_VPRINTF 1

/* Can use #warning in C files */
#define HAVE_WARNING_CPP_DIRECTIVE 1

/* Use xmlparse.h instead of expat.h */
/* #undef HAVE_XMLPARSE_H */

/* Define to 1 if you have the `XML_SetDoctypeDeclHandler' function. */
#define HAVE_XML_SETDOCTYPEDECLHANDLER 1

/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
#define LT_OBJDIR ".libs/"

/* Name of package */
#define PACKAGE "fontconfig"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME ""

/* Define to the full name and version of this package. */
#define PACKAGE_STRING ""

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME ""

/* Define to the version of this package. */
#define PACKAGE_VERSION ""

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Use iconv. */
/*#define USE_ICONV 1*/

/* Version number of package */
#define VERSION "2.10.1"

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef inline */
#endif

/* Define to `int' if <sys/types.h> does not define. */
/* #undef pid_t */
