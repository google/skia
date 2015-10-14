/**
 * \file libyasm/linemap.h
 * \brief YASM virtual line mapping interface.
 *
 * \license
 *  Copyright (C) 2002-2007  Peter Johnson
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
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
#ifndef YASM_LINEMAP_H
#define YASM_LINEMAP_H

#ifndef YASM_LIB_DECL
#define YASM_LIB_DECL
#endif

/** Create a new line mapping repository.
 * \return New repository.
 */
YASM_LIB_DECL
yasm_linemap *yasm_linemap_create(void);

/** Clean up any memory allocated for a repository.
 * \param linemap       line mapping repository
 */
YASM_LIB_DECL
void yasm_linemap_destroy(yasm_linemap *linemap);

/** Get the current line position in a repository.
 * \param linemap       line mapping repository
 * \return Current virtual line.
 */
YASM_LIB_DECL
unsigned long yasm_linemap_get_current(yasm_linemap *linemap);

/** Get bytecode and source line information, if any, for a virtual line.
 * \param linemap       line mapping repository
 * \param line          virtual line
 * \param bcp           pointer to return bytecode into
 * \param sourcep       pointer to return source code line pointer into
 * \return Zero if source line information available for line, nonzero if not.
 * \note If source line information is not available, bcp and sourcep targets
 * are set to NULL.
 */
YASM_LIB_DECL
int yasm_linemap_get_source(yasm_linemap *linemap, unsigned long line,
                            /*@null@*/ yasm_bytecode **bcp,
                            const char **sourcep);

/** Add bytecode and source line information to the current virtual line.
 * \attention Deletes any existing bytecode and source line information for
 *            the current virtual line.
 * \param linemap       line mapping repository
 * \param bc            bytecode (if any)
 * \param source        source code line
 * \note The source code line pointer is NOT kept, it is strdup'ed.
 */
YASM_LIB_DECL
void yasm_linemap_add_source(yasm_linemap *linemap,
                             /*@null@*/ yasm_bytecode *bc,
                             const char *source);

/** Go to the next line (increments the current virtual line).
 * \param linemap       line mapping repository
 * \return The current (new) virtual line.
 */
YASM_LIB_DECL
unsigned long yasm_linemap_goto_next(yasm_linemap *linemap);

/** Set a new file/line physical association starting point at the specified
 * virtual line.  line_inc indicates how much the "real" line is incremented
 * by for each virtual line increment (0 is perfectly legal).
 * \param linemap       line mapping repository
 * \param filename      physical file name (if NULL, not changed)
 * \param virtual_line  virtual line number (if 0, linemap->current is used)
 * \param file_line     physical line number
 * \param line_inc      line increment
 */
YASM_LIB_DECL
void yasm_linemap_set(yasm_linemap *linemap, /*@null@*/ const char *filename,
                      unsigned long virtual_line, unsigned long file_line,
                      unsigned long line_inc);

/** Poke a single file/line association, restoring the original physical
 * association starting point.  Caution: increments the current virtual line
 * twice.
 * \param linemap       line mapping repository
 * \param filename      physical file name (if NULL, not changed)
 * \param file_line     physical line number
 * \return The virtual line number of the poked association.
 */
YASM_LIB_DECL
unsigned long yasm_linemap_poke(yasm_linemap *linemap,
                                /*@null@*/ const char *filename,
                                unsigned long file_line);

/** Look up the associated physical file and line for a virtual line.
 * \param linemap       line mapping repository
 * \param line          virtual line
 * \param filename      physical file name (output)
 * \param file_line     physical line number (output)
 */
YASM_LIB_DECL
void yasm_linemap_lookup(yasm_linemap *linemap, unsigned long line,
                         /*@out@*/ const char **filename,
                         /*@out@*/ unsigned long *file_line);

/** Traverses all filenames used in a linemap, calling a function on each
 * filename.
 * \param linemap       line mapping repository
 * \param d             data pointer passed to func on each call
 * \param func          function
 * \return Stops early (and returns func's return value) if func returns a
 *         nonzero value; otherwise 0.
 */
YASM_LIB_DECL
int yasm_linemap_traverse_filenames
    (yasm_linemap *linemap, /*@null@*/ void *d,
     int (*func) (const char *filename, void *d));

#endif
