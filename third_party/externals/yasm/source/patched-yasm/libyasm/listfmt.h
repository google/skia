/**
 * \file libyasm/listfmt.h
 * \brief YASM list format interface.
 *
 * \license
 *  Copyright (C) 2004-2007  Peter Johnson
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
#ifndef YASM_LISTFMT_H
#define YASM_LISTFMT_H

#ifndef YASM_DOXYGEN
/** Base #yasm_listfmt structure.  Must be present as the first element in any
 * #yasm_listfmt implementation.
 */
typedef struct yasm_listfmt_base {
    /** #yasm_listfmt_module implementation for this list format. */
    const struct yasm_listfmt_module *module;
} yasm_listfmt_base;
#endif

/** YASM list format module interface. */
typedef struct yasm_listfmt_module {
    /** One-line description of the list format. */
    const char *name;

    /** Keyword used to select list format. */
    const char *keyword;

    /** Create list format.
     * Module-level implementation of yasm_listfmt_create().
     * The filenames are provided solely for informational purposes.
     * \param in_filename   primary input filename
     * \param obj_filename  object filename
     * \return NULL if unable to initialize.
     */
    /*@null@*/ /*@only@*/ yasm_listfmt * (*create)
        (const char *in_filename, const char *obj_filename);

    /** Module-level implementation of yasm_listfmt_destroy().
     * Call yasm_listfmt_destroy() instead of calling this function.
     */
    void (*destroy) (/*@only@*/ yasm_listfmt *listfmt);

    /** Module-level implementation of yasm_listfmt_output().
     * Call yasm_listfmt_output() instead of calling this function.
     */
    void (*output) (yasm_listfmt *listfmt, FILE *f, yasm_linemap *linemap,
                    yasm_arch *arch);
} yasm_listfmt_module;

/** Get the keyword used to select a list format.
 * \param listfmt   list format
 * \return keyword
 */
const char *yasm_listfmt_keyword(const yasm_listfmt *listfmt);

/** Initialize list format for use.  Must call before any other list
 * format functions.  The filenames are provided solely for informational
 * purposes.
 * \param module        list format module
 * \param in_filename   primary input filename
 * \param obj_filename  object filename
 * \return NULL if object format does not provide needed support.
 */
/*@null@*/ /*@only@*/ yasm_listfmt *yasm_listfmt_create
    (const yasm_listfmt_module *module, const char *in_filename,
     const char *obj_filename);

/** Cleans up any allocated list format memory.
 * \param listfmt       list format
 */
void yasm_listfmt_destroy(/*@only@*/ yasm_listfmt *listfmt);

/** Write out list to the list file.
 * This function may call all read-only yasm_* functions as necessary.
 * \param listfmt       list format
 * \param f             output list file
 * \param linemap       line mapping repository
 * \param arch          architecture
 */
void yasm_listfmt_output(yasm_listfmt *listfmt, FILE *f,
                         yasm_linemap *linemap, yasm_arch *arch);

#ifndef YASM_DOXYGEN

/* Inline macro implementations for listfmt functions */

#define yasm_listfmt_keyword(listfmt) \
    (((yasm_listfmt_base *)listfmt)->module->keyword)

#define yasm_listfmt_create(module, in_filename, obj_filename) \
    module->create(in_filename, obj_filename)

#define yasm_listfmt_destroy(listfmt) \
    ((yasm_listfmt_base *)listfmt)->module->destroy(listfmt)

#define yasm_listfmt_output(listfmt, f, linemap, a) \
    ((yasm_listfmt_base *)listfmt)->module->output(listfmt, f, linemap, a)

#endif

#endif
