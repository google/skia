/**
 * \file libyasm/file.h
 * \brief YASM file helpers.
 *
 * \license
 *  Copyright (C) 2001-2007  Peter Johnson
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND OTHER CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR OTHER CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 * \endlicense
 */
#ifndef YASM_FILE_H
#define YASM_FILE_H

#ifndef YASM_LIB_DECL
#define YASM_LIB_DECL
#endif

/** Re2c scanner state. */
typedef struct yasm_scanner {
    unsigned char *bot;     /**< Bottom of scan buffer */
    unsigned char *tok;     /**< Start of token */
    unsigned char *ptr;     /**< Scan marker */
    unsigned char *cur;     /**< Cursor (1 past end of token) */
    unsigned char *lim;     /**< Limit of good data */
    unsigned char *top;     /**< Top of scan buffer */
    unsigned char *eof;     /**< End of file */
} yasm_scanner;

/** Initialize scanner state.
 * \param scanner   Re2c scanner state
 */
YASM_LIB_DECL
void yasm_scanner_initialize(yasm_scanner *scanner);

/** Frees any memory used by scanner state; does not free state itself.
 * \param scanner   Re2c scanner state
 */
YASM_LIB_DECL
void yasm_scanner_delete(yasm_scanner *scanner);

/** Fill a scanner state structure with data coming from an input function.
 * \param scanner       Re2c scanner state
 * \param cursor        Re2c scan cursor
 * \param input_func    Input function to read data; takes buffer and maximum
 *                      number of bytes, returns number of bytes read.
 * \param input_func_data   Data to pass as the first parameter to input_func
 * \return 1 if this was the first time this function was called on this
 *         scanner state, 0 otherwise.
 */
YASM_LIB_DECL
int yasm_fill_helper
    (yasm_scanner *scanner, unsigned char **cursor,
     size_t (*input_func) (void *d, unsigned char *buf, size_t max),
     void *input_func_data);

/** Unescape a string with C-style escapes.  Handles b, f, n, r, t, and hex
 * and octal escapes.  String is updated in-place.
 * Edge cases:
 * - hex escapes: reads as many hex digits as possible, takes last 2 as value.
 * - oct escapes: takes up to 3 digits 0-9 and scales appropriately, with
 *                warning.
 * \param str           C-style string (updated in place)
 * \param len           length of string (updated with new length)
 */
YASM_LIB_DECL
void yasm_unescape_cstring(unsigned char *str, size_t *len);

/** Split a UNIX pathname into head (directory) and tail (base filename)
 * portions.
 * \internal
 * \param path  pathname
 * \param tail  (returned) base filename
 * \return Length of head (directory).
 */
YASM_LIB_DECL
size_t yasm__splitpath_unix(const char *path, /*@out@*/ const char **tail);

/** Split a Windows pathname into head (directory) and tail (base filename)
 * portions.
 * \internal
 * \param path  pathname
 * \param tail  (returned) base filename
 * \return Length of head (directory).
 */
YASM_LIB_DECL
size_t yasm__splitpath_win(const char *path, /*@out@*/ const char **tail);

/** Split a pathname into head (directory) and tail (base filename) portions.
 * Unless otherwise defined, defaults to yasm__splitpath_unix().
 * \internal
 * \param path  pathname
 * \param tail  (returned) base filename
 * \return Length of head (directory).
 */
#ifndef yasm__splitpath
# if defined (_WIN32) || defined (WIN32) || defined (__MSDOS__) || \
 defined (__DJGPP__) || defined (__OS2__)
#  define yasm__splitpath(path, tail)   yasm__splitpath_win(path, tail)
# else
#  define yasm__splitpath(path, tail)   yasm__splitpath_unix(path, tail)
# endif
#endif

/** Get the current working directory.
 * \internal
 * \return Current working directory pathname (newly allocated).
 */
YASM_LIB_DECL
/*@only@*/ char *yasm__getcwd(void);

/** Convert a relative or absolute pathname into an absolute pathname.
 * \internal
 * \param path  pathname
 * \return Absolute version of path (newly allocated).
 */
YASM_LIB_DECL
/*@only@*/ char *yasm__abspath(const char *path);

/** Build a UNIX pathname that is equivalent to accessing the "to" pathname
 * when you're in the directory containing "from".  Result is relative if both
 * from and to are relative.
 * \internal
 * \param from  from pathname
 * \param to    to pathname
 * \return Combined path (newly allocated).
 */
YASM_LIB_DECL
char *yasm__combpath_unix(const char *from, const char *to);

/** Build a Windows pathname that is equivalent to accessing the "to" pathname
 * when you're in the directory containing "from".  Result is relative if both
 * from and to are relative.
 * \internal
 * \param from  from pathname
 * \param to    to pathname
 * \return Combined path (newly allocated).
 */
YASM_LIB_DECL
char *yasm__combpath_win(const char *from, const char *to);

/** Build a pathname that is equivalent to accessing the "to" pathname
 * when you're in the directory containing "from".  Result is relative if both
 * from and to are relative.
 * Unless otherwise defined, defaults to yasm__combpath_unix().
 * \internal
 * \param from  from pathname
 * \param to    to pathname
 * \return Combined path (newly allocated).
 */
#ifndef yasm__combpath
# if defined (_WIN32) || defined (WIN32) || defined (__MSDOS__) || \
 defined (__DJGPP__) || defined (__OS2__)
#  define yasm__combpath(from, to)      yasm__combpath_win(from, to)
# else
#  define yasm__combpath(from, to)      yasm__combpath_unix(from, to)
# endif
#endif

/** Recursively create tree of directories needed for pathname.
 * \internal
 * \param path  pathname
 * \param win   handle windows paths
 * \return Length of directory portion of pathname.
 */
YASM_LIB_DECL
size_t yasm__createpath_common(const char *path, int win);

/** Recursively create tree of directories needed for pathname.
 * Unless otherwise defined, defaults to yasm__createpath_unix().
 * \internal
 * \param path  pathname
 * \return Length of directory portion of pathname.
 */
#ifndef yasm__createpath
# if defined (_WIN32) || defined (WIN32) || defined (__MSDOS__) || \
 defined (__DJGPP__) || defined (__OS2__)
#  define yasm__createpath(path)    yasm__createpath_common(path, 1)
# else
#  define yasm__createpath(path)    yasm__createpath_common(path, 0)
# endif
#endif

/** Try to find and open an include file, searching through include paths.
 * First iname is looked for relative to the directory containing "from", then
 * it's looked for relative to each of the include paths.
 *
 * All pathnames may be either absolute or relative; from, oname, and
 * include paths, if relative, are relative from the current working directory.
 *
 * First match wins; the full pathname (newly allocated) to the opened file
 * is saved into oname, and the fopen'ed FILE * is returned.  If not found,
 * NULL is returned.
 *
 * \param iname     file to include
 * \param from      file doing the including
 * \param mode      fopen mode string
 * \param oname     full pathname of included file (may be relative). NULL
 *                  may be passed if this is unwanted.
 * \return fopen'ed include file, or NULL if not found.
 */
YASM_LIB_DECL
/*@null@*/ FILE *yasm_fopen_include
    (const char *iname, const char *from, const char *mode,
     /*@null@*/ /*@out@*/ /*@only@*/ char **oname);

/** Delete any stored include paths added by yasm_add_include_path().
 */
YASM_LIB_DECL
void yasm_delete_include_paths(void);

/** Iterate through include paths.
*/
YASM_LIB_DECL
const char * yasm_get_include_dir(void **iter);

/** Add an include path for use by yasm_fopen_include().
 * If path is relative, it is treated by yasm_fopen_include() as relative to
 * the current working directory.
 *
 * \param path      path to add
 */
YASM_LIB_DECL
void yasm_add_include_path(const char *path);

/** Write an 8-bit value to a buffer, incrementing buffer pointer.
 * \note Only works properly if ptr is an (unsigned char *).
 * \param ptr   buffer
 * \param val   8-bit value
 */
#define YASM_WRITE_8(ptr, val)                  \
        *((ptr)++) = (unsigned char)((val) & 0xFF)

/** Write a 16-bit value to a buffer in little endian, incrementing buffer
 * pointer.
 * \note Only works properly if ptr is an (unsigned char *).
 * \param ptr   buffer
 * \param val   16-bit value
 */
#define YASM_WRITE_16_L(ptr, val)               \
        do {                                    \
            *((ptr)++) = (unsigned char)((val) & 0xFF);         \
            *((ptr)++) = (unsigned char)(((val) >> 8) & 0xFF);  \
        } while (0)

/** Write a 32-bit value to a buffer in little endian, incrementing buffer
 * pointer.
 * \note Only works properly if ptr is an (unsigned char *).
 * \param ptr   buffer
 * \param val   32-bit value
 */
#define YASM_WRITE_32_L(ptr, val)               \
        do {                                    \
            *((ptr)++) = (unsigned char)((val) & 0xFF);         \
            *((ptr)++) = (unsigned char)(((val) >> 8) & 0xFF);  \
            *((ptr)++) = (unsigned char)(((val) >> 16) & 0xFF); \
            *((ptr)++) = (unsigned char)(((val) >> 24) & 0xFF); \
        } while (0)

/** Write a 16-bit value to a buffer in big endian, incrementing buffer
 * pointer.
 * \note Only works properly if ptr is an (unsigned char *).
 * \param ptr   buffer
 * \param val   16-bit value
 */
#define YASM_WRITE_16_B(ptr, val)               \
        do {                                    \
            *((ptr)++) = (unsigned char)(((val) >> 8) & 0xFF);  \
            *((ptr)++) = (unsigned char)((val) & 0xFF);         \
        } while (0)

/** Write a 32-bit value to a buffer in big endian, incrementing buffer
 * pointer.
 * \note Only works properly if ptr is an (unsigned char *).
 * \param ptr   buffer
 * \param val   32-bit value
 */
#define YASM_WRITE_32_B(ptr, val)               \
        do {                                    \
            *((ptr)++) = (unsigned char)(((val) >> 24) & 0xFF); \
            *((ptr)++) = (unsigned char)(((val) >> 16) & 0xFF); \
            *((ptr)++) = (unsigned char)(((val) >> 8) & 0xFF);  \
            *((ptr)++) = (unsigned char)((val) & 0xFF);         \
        } while (0)


/** Write an 8-bit value to a buffer.  Does not increment buffer pointer.
 * \note Only works properly if ptr is an (unsigned char *).
 * \param ptr   buffer
 * \param val   8-bit value
 */
#define YASM_SAVE_8(ptr, val)                   \
        *(ptr) = (unsigned char)((val) & 0xFF)

/** Write a 16-bit value to a buffer in little endian.  Does not increment
 * buffer pointer.
 * \note Only works properly if ptr is an (unsigned char *).
 * \param ptr   buffer
 * \param val   16-bit value
 */
#define YASM_SAVE_16_L(ptr, val)                \
        do {                                    \
            *(ptr) = (unsigned char)((val) & 0xFF);             \
            *((ptr)+1) = (unsigned char)(((val) >> 8) & 0xFF);  \
        } while (0)

/** Write a 32-bit value to a buffer in little endian.  Does not increment
 * buffer pointer.
 * \note Only works properly if ptr is an (unsigned char *).
 * \param ptr   buffer
 * \param val   32-bit value
 */
#define YASM_SAVE_32_L(ptr, val)                \
        do {                                    \
            *(ptr) = (unsigned char)((val) & 0xFF);             \
            *((ptr)+1) = (unsigned char)(((val) >> 8) & 0xFF);  \
            *((ptr)+2) = (unsigned char)(((val) >> 16) & 0xFF); \
            *((ptr)+3) = (unsigned char)(((val) >> 24) & 0xFF); \
        } while (0)

/** Write a 16-bit value to a buffer in big endian.  Does not increment buffer
 * pointer.
 * \note Only works properly if ptr is an (unsigned char *).
 * \param ptr   buffer
 * \param val   16-bit value
 */
#define YASM_SAVE_16_B(ptr, val)                \
        do {                                    \
            *(ptr) = (unsigned char)(((val) >> 8) & 0xFF);      \
            *((ptr)+1) = (unsigned char)((val) & 0xFF);         \
        } while (0)

/** Write a 32-bit value to a buffer in big endian.  Does not increment buffer
 * pointer.
 * \note Only works properly if ptr is an (unsigned char *).
 * \param ptr   buffer
 * \param val   32-bit value
 */
#define YASM_SAVE_32_B(ptr, val)                \
        do {                                    \
            *(ptr) = (unsigned char)(((val) >> 24) & 0xFF);     \
            *((ptr)+1) = (unsigned char)(((val) >> 16) & 0xFF); \
            *((ptr)+2) = (unsigned char)(((val) >> 8) & 0xFF);  \
            *((ptr)+3) = (unsigned char)((val) & 0xFF);         \
        } while (0)

/** Direct-to-file version of YASM_SAVE_16_L().
 * \note Using the macro multiple times with a single fwrite() call will
 *       probably be faster than calling this function many times.
 * \param val   16-bit value
 * \param f     file
 * \return 1 if the write was successful, 0 if not (just like fwrite()).
 */
YASM_LIB_DECL
size_t yasm_fwrite_16_l(unsigned short val, FILE *f);

/** Direct-to-file version of YASM_SAVE_32_L().
 * \note Using the macro multiple times with a single fwrite() call will
 *       probably be faster than calling this function many times.
 * \param val   32-bit value
 * \param f     file
 * \return 1 if the write was successful, 0 if not (just like fwrite()).
 */
YASM_LIB_DECL
size_t yasm_fwrite_32_l(unsigned long val, FILE *f);

/** Direct-to-file version of YASM_SAVE_16_B().
 * \note Using the macro multiple times with a single fwrite() call will
 *       probably be faster than calling this function many times.
 * \param val   16-bit value
 * \param f     file
 * \return 1 if the write was successful, 0 if not (just like fwrite()).
 */
YASM_LIB_DECL
size_t yasm_fwrite_16_b(unsigned short val, FILE *f);

/** Direct-to-file version of YASM_SAVE_32_B().
 * \note Using the macro multiple times with a single fwrite() call will
 *       probably be faster than calling this function many times.
 * \param val   32-bit value
 * \param f     file
 * \return 1 if the write was successful, 0 if not (just like fwrite()).
 */
YASM_LIB_DECL
size_t yasm_fwrite_32_b(unsigned long val, FILE *f);

/** Read an 8-bit value from a buffer, incrementing buffer pointer.
 * \note Only works properly if ptr is an (unsigned char *).
 * \param ptr   buffer
 * \param val   8-bit value
 */
#define YASM_READ_8(val, ptr)                   \
        (val) = *((ptr)++) & 0xFF

/** Read a 16-bit value from a buffer in little endian, incrementing buffer
 * pointer.
 * \note Only works properly if ptr is an (unsigned char *).
 * \param ptr   buffer
 * \param val   16-bit value
 */
#define YASM_READ_16_L(val, ptr)                \
        do {                                    \
            (val) = *((ptr)++) & 0xFF;          \
            (val) |= (*((ptr)++) & 0xFF) << 8;  \
        } while (0)

/** Read a 32-bit value from a buffer in little endian, incrementing buffer
 * pointer.
 * \note Only works properly if ptr is an (unsigned char *).
 * \param ptr   buffer
 * \param val   32-bit value
 */
#define YASM_READ_32_L(val, ptr)                \
        do {                                    \
            (val) = *((ptr)++) & 0xFF;          \
            (val) |= (*((ptr)++) & 0xFF) << 8;  \
            (val) |= (*((ptr)++) & 0xFF) << 16; \
            (val) |= (*((ptr)++) & 0xFF) << 24; \
        } while (0)

/** Read a 16-bit value from a buffer in big endian, incrementing buffer
 * pointer.
 * \note Only works properly if ptr is an (unsigned char *).
 * \param ptr   buffer
 * \param val   16-bit value
 */
#define YASM_READ_16_B(val, ptr)                \
        do {                                    \
            (val) = (*((ptr)++) & 0xFF) << 8;   \
            (val) |= *((ptr)++) & 0xFF;         \
        } while (0)

/** Read a 32-bit value from a buffer in big endian, incrementing buffer
 * pointer.
 * \note Only works properly if ptr is an (unsigned char *).
 * \param ptr   buffer
 * \param val   32-bit value
 */
#define YASM_READ_32_B(val, ptr)                \
        do {                                    \
            (val) = (*((ptr)++) & 0xFF) << 24;  \
            (val) |= (*((ptr)++) & 0xFF) << 16; \
            (val) |= (*((ptr)++) & 0xFF) << 8;  \
            (val) |= *((ptr)++) & 0xFF;         \
        } while (0)

/** Read an 8-bit value from a buffer.  Does not increment buffer pointer.
 * \note Only works properly if ptr is an (unsigned char *).
 * \param ptr   buffer
 * \param val   8-bit value
 */
#define YASM_LOAD_8(val, ptr)                   \
        (val) = *(ptr) & 0xFF

/** Read a 16-bit value from a buffer in little endian.  Does not increment
 * buffer pointer.
 * \note Only works properly if ptr is an (unsigned char *).
 * \param ptr   buffer
 * \param val   16-bit value
 */
#define YASM_LOAD_16_L(val, ptr)                \
        do {                                    \
            (val) = *(ptr) & 0xFF;              \
            (val) |= (*((ptr)+1) & 0xFF) << 8;  \
        } while (0)

/** Read a 32-bit value from a buffer in little endian.  Does not increment
 * buffer pointer.
 * \note Only works properly if ptr is an (unsigned char *).
 * \param ptr   buffer
 * \param val   32-bit value
 */
#define YASM_LOAD_32_L(val, ptr)                \
        do {                                    \
            (val) = (unsigned long)(*(ptr) & 0xFF);                 \
            (val) |= (unsigned long)((*((ptr)+1) & 0xFF) << 8);     \
            (val) |= (unsigned long)((*((ptr)+2) & 0xFF) << 16);    \
            (val) |= (unsigned long)((*((ptr)+3) & 0xFF) << 24);    \
        } while (0)

/** Read a 16-bit value from a buffer in big endian.  Does not increment buffer
 * pointer.
 * \note Only works properly if ptr is an (unsigned char *).
 * \param ptr   buffer
 * \param val   16-bit value
 */
#define YASM_LOAD_16_B(val, ptr)                \
        do {                                    \
            (val) = (*(ptr) & 0xFF) << 8;       \
            (val) |= *((ptr)+1) & 0xFF;         \
        } while (0)

/** Read a 32-bit value from a buffer in big endian.  Does not increment buffer
 * pointer.
 * \note Only works properly if ptr is an (unsigned char *).
 * \param ptr   buffer
 * \param val   32-bit value
 */
#define YASM_LOAD_32_B(val, ptr)                \
        do {                                    \
            (val) = (unsigned long)((*(ptr) & 0xFF) << 24);         \
            (val) |= (unsigned long)((*((ptr)+1) & 0xFF) << 16);    \
            (val) |= (unsigned long)((*((ptr)+2) & 0xFF) << 8);     \
            (val) |= (unsigned long)(*((ptr)+3) & 0xFF);            \
        } while (0)

#endif
