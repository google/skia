/* nasmlib.h    header file for nasmlib.c
 *
 * The Netwide Assembler is copyright (C) 1996 Simon Tatham and
 * Julian Hall. All rights reserved. The software is
 * redistributable under the licence given in the file "Licence"
 * distributed in the NASM archive.
 */

#ifndef YASM_NASMLIB_H
#define YASM_NASMLIB_H

/*
 * Wrappers around malloc, realloc and free. nasm_malloc will
 * fatal-error and die rather than return NULL; nasm_realloc will
 * do likewise, and will also guarantee to work right on being
 * passed a NULL pointer; nasm_free will do nothing if it is passed
 * a NULL pointer.
 */
#define nasm_malloc yasm_xmalloc
#define nasm_realloc yasm_xrealloc
#ifdef WITH_DMALLOC
#define nasm_free(p) do { if (p) yasm_xfree(p); } while(0)
#else
#define nasm_free(p) yasm_xfree(p)
#endif
#define nasm_strdup yasm__xstrdup
#define nasm_strndup yasm__xstrndup
#define nasm_stricmp yasm__strcasecmp
#define nasm_strnicmp yasm__strncasecmp

/*
 * Convert a string into a number, using NASM number rules. Sets
 * `*error' to TRUE if an error occurs, and FALSE otherwise.
 */
yasm_intnum *nasm_readnum(char *str, int *error);

/*
 * Convert a character constant into a number. Sets
 * `*warn' to TRUE if an overflow occurs, and FALSE otherwise.
 * str points to and length covers the middle of the string,
 * without the quotes.
 */
yasm_intnum *nasm_readstrnum(char *str, size_t length, int *warn);

char *nasm_src_set_fname(char *newname);
char *nasm_src_get_fname(void);
long nasm_src_set_linnum(long newline);
long nasm_src_get_linnum(void);
/*
 * src_get may be used if you simply want to know the source file and line.
 * It is also used if you maintain private status about the source location
 * It return 0 if the information was the same as the last time you
 * checked, -1 if the name changed and (new-old) if just the line changed.
 */
int nasm_src_get(long *xline, char **xname);

void nasm_quote(char **str);
char *nasm_strcat(const char *one, const char *two);

#endif
