/**
 * \file libyasm/objfmt.h
 * \brief YASM object format module interface.
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
#ifndef YASM_OBJFMT_H
#define YASM_OBJFMT_H

#ifndef YASM_DOXYGEN
/** Base #yasm_objfmt structure.  Must be present as the first element in any
 * #yasm_objfmt implementation.
 */
typedef struct yasm_objfmt_base {
    /** #yasm_objfmt_module implementation for this object format. */
    const struct yasm_objfmt_module *module;
} yasm_objfmt_base;
#endif

/** Object format module interface. */
struct yasm_objfmt_module {
    /** One-line description of the object format. */
    const char *name;

    /** Keyword used to select object format. */
    const char *keyword;

    /** Default output file extension (without the '.').
     * NULL means no extension, with no '.', while "" includes the '.'.
     */
    /*@null@*/ const char *extension;

    /** Default (starting) x86 BITS setting.  This only appies to the x86
     * architecture; other architectures ignore this setting.
     */
    const unsigned char default_x86_mode_bits;

    /** If @ signs should be legal in identifiers. */
    const unsigned char id_at_ok;

    /** NULL-terminated list of debug format (yasm_dbgfmt) keywords that are
     * valid to use with this object format.  The null debug format
     * (null_dbgfmt, "null") should always be in this list so it's possible to
     * have no debug output.
     */
    const char **dbgfmt_keywords;

    /** Default debug format keyword (set even if there's only one available to
     * use).
     */
    const char *default_dbgfmt_keyword;

    /** NULL-terminated list of directives.  NULL if none. */
    /*@null@*/ const yasm_directive *directives;

    /** NULL-terminated list of standard macro lookups.  NULL if none. */
    const yasm_stdmac *stdmacs;

    /** Create object format.
     * Module-level implementation of yasm_objfmt_create().
     * Call yasm_objfmt_create() instead of calling this function.
     * \param object            object
     * \param a                 architecture in use
     * \return NULL if architecture/machine combination not supported.
     */
    /*@null@*/ /*@only@*/ yasm_objfmt * (*create) (yasm_object *object);

    /** Module-level implementation of yasm_objfmt_output().
     * Call yasm_objfmt_output() instead of calling this function.
     */
    void (*output) (yasm_object *o, FILE *f, int all_syms,
                    yasm_errwarns *errwarns);

    /** Module-level implementation of yasm_objfmt_destroy().
     * Call yasm_objfmt_destroy() instead of calling this function.
     */
    void (*destroy) (/*@only@*/ yasm_objfmt *objfmt);

    /** Module-level implementation of yasm_objfmt_add_default_section().
     * Call yasm_objfmt_add_default_section() instead of calling this function.
     */
    yasm_section * (*add_default_section) (yasm_object *object);

    /** Module-level implementation of yasm_objfmt_init_new_section().
     * Call yasm_objfmt_init_new_section() instead of calling this function.
     */
    void (*init_new_section) (yasm_section *section, unsigned long line);

    /** Module-level implementation of yasm_objfmt_section_switch().
     * Call yasm_objfmt_section_switch() instead of calling this function.
     */
    /*@observer@*/ /*@null@*/ yasm_section *
        (*section_switch)(yasm_object *object, yasm_valparamhead *valparams,
                          /*@null@*/ yasm_valparamhead *objext_valparams,
                          unsigned long line);

    /** Module-level implementation of yasm_objfmt_get_special_sym().
     * Call yasm_objfmt_get_special_sym() instead of calling this function.
     */
    /*@observer@*/ /*@null@*/ yasm_symrec *
        (*get_special_sym)(yasm_object *object, const char *name,
                           const char *parser);
};

/** Create object format.
 * \param module        object format module
 * \param object        object
 * \return NULL if architecture/machine combination not supported.
 */
/*@null@*/ /*@only@*/ yasm_objfmt *yasm_objfmt_create
    (const yasm_objfmt_module *module, yasm_object *object);

/** Write out (post-optimized) sections to the object file.
 * This function may call yasm_symrec_* functions as necessary (including
 * yasm_symrec_traverse()) to retrieve symbolic information.
 * \param object        object
 * \param f             output object file
 * \param all_syms      if nonzero, all symbols should be included in
 *                      the object file
 * \param errwarns      error/warning set
 * \note Errors and warnings are stored into errwarns.
 */
void yasm_objfmt_output(yasm_object *object, FILE *f, int all_syms,
                        yasm_errwarns *errwarns);

/** Cleans up any allocated object format memory.
 * \param objfmt        object format
 */
void yasm_objfmt_destroy(/*@only@*/ yasm_objfmt *objfmt);

/** Add a default section to an object.
 * \param object    object
 * \return Default section.
 */
yasm_section *yasm_objfmt_add_default_section(yasm_object *object);

/** Initialize the object-format specific portion of a section.  Called
 * by yasm_object_get_general(); in general should not be directly called.
 * \param section   section
 * \param line      virtual line (from yasm_linemap)
 */
void yasm_objfmt_init_new_section(yasm_object *object, unsigned long line);

/** Switch object file sections.  The first val of the valparams should
 * be the section name.  Calls yasm_object_get_general() to actually get
 * the section.
 * \param object                object
 * \param valparams             value/parameters
 * \param objext_valparams      object format-specific value/parameters
 * \param line                  virtual line (from yasm_linemap)
 * \return NULL on error, otherwise new section.
 */
/*@observer@*/ /*@null@*/ yasm_section *yasm_objfmt_section_switch
    (yasm_object *object, yasm_valparamhead *valparams,
     /*@null@*/ yasm_valparamhead *objext_valparams, unsigned long line);

/** Get a special symbol.  Special symbols are generally used to generate
 * special relocation types via the WRT mechanism.
 * \param object        object
 * \param name          symbol name (not including any parser-specific prefix)
 * \param parser        parser keyword
 * \return NULL if unrecognized, otherwise special symbol.
 */
/*@observer@*/ /*@null@*/ yasm_symrec *yasm_objfmt_get_special_sym
    (yasm_object *object, const char *name, const char *parser);

#ifndef YASM_DOXYGEN

/* Inline macro implementations for objfmt functions */

#define yasm_objfmt_create(module, object) module->create(object)

#define yasm_objfmt_output(object, f, all_syms, ews) \
    ((yasm_objfmt_base *)((object)->objfmt))->module->output \
        (object, f, all_syms, ews)
#define yasm_objfmt_destroy(objfmt) \
    ((yasm_objfmt_base *)objfmt)->module->destroy(objfmt)
#define yasm_objfmt_section_switch(object, vpms, oe_vpms, line) \
    ((yasm_objfmt_base *)((object)->objfmt))->module->section_switch \
        (object, vpms, oe_vpms, line)
#define yasm_objfmt_add_default_section(object) \
    ((yasm_objfmt_base *)((object)->objfmt))->module->add_default_section \
        (object)
#define yasm_objfmt_init_new_section(section, line) \
    ((yasm_objfmt_base *)((object)->objfmt))->module->init_new_section \
        (section, line)
#define yasm_objfmt_get_special_sym(object, name, parser) \
    ((yasm_objfmt_base *)((object)->objfmt))->module->get_special_sym \
        (object, name, parser)

#endif

#endif
