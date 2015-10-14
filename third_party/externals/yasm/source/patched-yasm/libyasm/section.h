/**
 * \file libyasm/section.h
 * \brief YASM section interface.
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
#ifndef YASM_SECTION_H
#define YASM_SECTION_H

#ifndef YASM_LIB_DECL
#define YASM_LIB_DECL
#endif

/** Basic YASM relocation.  Object formats will need to extend this
 * structure with additional fields for relocation type, etc.
 */
typedef struct yasm_reloc yasm_reloc;

struct yasm_reloc {
    /*@reldef@*/ STAILQ_ENTRY(yasm_reloc) link; /**< Link to next reloc */
    yasm_intnum *addr;          /**< Offset (address) within section */
    /*@dependent@*/ yasm_symrec *sym;       /**< Relocated symbol */
};

/** An object.  This is the internal representation of an object file. */
struct yasm_object {
    /*@owned@*/ char *src_filename;     /**< Source filename */
    /*@owned@*/ char *obj_filename;     /**< Object filename */

    /*@owned@*/ yasm_symtab *symtab;    /**< Symbol table */
    /*@owned@*/ yasm_arch *arch;        /**< Target architecture */
    /*@owned@*/ yasm_objfmt *objfmt;    /**< Object format */
    /*@owned@*/ yasm_dbgfmt *dbgfmt;    /**< Debug format */

    /** Currently active section.  Used by some directives.  NULL if no
     * section active.
     */
    /*@dependent@*/ /*@null@*/ yasm_section *cur_section;

    /** Linked list of sections. */
    /*@reldef@*/ STAILQ_HEAD(yasm_sectionhead, yasm_section) sections;

    /** Directives, organized as two level HAMT; first level is parser,
     * second level is directive name.
     */
    /*@owned@*/ struct HAMT *directives;

    /** Prefix prepended to externally-visible symbols (empty string if none) */
    /*@owned@*/ char *global_prefix;

    /** Suffix appended to externally-visible symbols (empty string if none) */
    /*@owned@*/ char *global_suffix;
};

/** Create a new object.  A default section is created as the first section.
 * An empty symbol table (yasm_symtab) and line mapping (yasm_linemap) are
 * automatically created.
 * \param src_filename  source filename (e.g. "file.asm")
 * \param obj_filename  object filename (e.g. "file.o")
 * \param arch          architecture
 * \param objfmt_module object format module
 * \param dbgfmt_module debug format module
 * \return Newly allocated object, or NULL on error.
 */
YASM_LIB_DECL
/*@null@*/ /*@only@*/ yasm_object *yasm_object_create
    (const char *src_filename, const char *obj_filename,
     /*@kept@*/ yasm_arch *arch,
     const yasm_objfmt_module *objfmt_module,
     const yasm_dbgfmt_module *dbgfmt_module);

/** Create a new, or continue an existing, general section.  The section is
 * added to the object if there's not already a section by that name.
 * \param object    object
 * \param name      section name
 * \param align     alignment in bytes (0 if none)
 * \param code      if nonzero, section is intended to contain code
 *                  (e.g. alignment should be made with NOP instructions, not 0)
 * \param res_only  if nonzero, only space-reserving bytecodes are allowed in
 *                  the section (ignored if section already exists)
 * \param isnew     output; set to nonzero if section did not already exist
 * \param line      virtual line of section declaration (ignored if section
 *                  already exists)
 * \return New section.
 */
YASM_LIB_DECL
/*@dependent@*/ yasm_section *yasm_object_get_general
    (yasm_object *object, const char *name, unsigned long align, int code,
     int res_only, /*@out@*/ int *isnew, unsigned long line);

/** Handle a directive.  Passed down to object format, debug format, or
 * architecture as appropriate.
 * \param object                object
 * \param name                  directive name
 * \param parser                parser keyword
 * \param valparams             value/parameters
 * \param objext_valparams      "object format-specific" value/parameters
 * \param line                  virtual line (from yasm_linemap)
 * \return 0 if directive recognized, nonzero if unrecognized.
 */
YASM_LIB_DECL
int yasm_object_directive(yasm_object *object, const char *name,
                          const char *parser, yasm_valparamhead *valparams,
                          yasm_valparamhead *objext_valparams,
                          unsigned long line);

/** Delete (free allocated memory for) an object.  All sections in the
 * object and all bytecodes within those sections are also deleted.
 * \param object        object
 */
YASM_LIB_DECL
void yasm_object_destroy(/*@only@*/ yasm_object *object);

/** Print an object.  For debugging purposes.
 * \param object        object
 * \param f             file
 * \param indent_level  indentation level
 */
YASM_LIB_DECL
void yasm_object_print(const yasm_object *object, FILE *f, int indent_level);

/** Finalize an object after parsing.
 * \param object        object
 * \param errwarns      error/warning set
 * \note Errors/warnings are stored into errwarns.
 */
YASM_LIB_DECL
void yasm_object_finalize(yasm_object *object, yasm_errwarns *errwarns);

/** Traverses all sections in an object, calling a function on each section.
 * \param object        object
 * \param d             data pointer passed to func on each call
 * \param func          function
 * \return Stops early (and returns func's return value) if func returns a
 *         nonzero value; otherwise 0.
 */
YASM_LIB_DECL
int yasm_object_sections_traverse
    (yasm_object *object, /*@null@*/ void *d,
     int (*func) (yasm_section *sect, /*@null@*/ void *d));

/** Find a general section in an object, based on its name.
 * \param object        object
 * \param name          section name
 * \return Section matching name, or NULL if no match found.
 */
YASM_LIB_DECL
/*@dependent@*/ /*@null@*/ yasm_section *yasm_object_find_general
    (yasm_object *object, const char *name);

/** Change the source filename for an object.
 * \param object        object
 * \param src_filename  new source filename (e.g. "file.asm")
 */
YASM_LIB_DECL
void yasm_object_set_source_fn(yasm_object *object, const char *src_filename);

/** Change the prefix used for externally-visible symbols.
 * \param object        object
 * \param prefix        new prefix
 */
YASM_LIB_DECL
void yasm_object_set_global_prefix(yasm_object *object, const char *prefix);

/** Change the suffix used for externally-visible symbols.
 * \param object        object
 * \param suffix        new suffix
 */
YASM_LIB_DECL
void yasm_object_set_global_suffix(yasm_object *object, const char *suffix);

/** Optimize an object.  Takes the unoptimized object and optimizes it.
 * If successful, the object is ready for output to an object file.
 * \param object        object
 * \param errwarns      error/warning set
 * \note Optimization failures are stored into errwarns.
 */
YASM_LIB_DECL
void yasm_object_optimize(yasm_object *object, yasm_errwarns *errwarns);

/** Determine if a section is flagged to contain code.
 * \param sect      section
 * \return Nonzero if section is flagged to contain code.
 */
YASM_LIB_DECL
int yasm_section_is_code(yasm_section *sect);

/** Get yasm_optimizer-specific flags.  For yasm_optimizer use only.
 * \param sect      section
 * \return Optimizer-specific flags.
 */
YASM_LIB_DECL
unsigned long yasm_section_get_opt_flags(const yasm_section *sect);

/** Set yasm_optimizer-specific flags.  For yasm_optimizer use only.
 * \param sect      section
 * \param opt_flags optimizer-specific flags.
 */
YASM_LIB_DECL
void yasm_section_set_opt_flags(yasm_section *sect, unsigned long opt_flags);

/** Determine if a section was declared as the "default" section (e.g. not
 * created through a section directive).
 * \param sect      section
 * \return Nonzero if section was declared as default.
 */
YASM_LIB_DECL
int yasm_section_is_default(const yasm_section *sect);

/** Set section "default" flag to a new value.
 * \param sect      section
 * \param def       new value of default flag
 */
YASM_LIB_DECL
void yasm_section_set_default(yasm_section *sect, int def);

/** Get object owner of a section.
 * \param sect      section
 * \return Object this section is a part of.
 */
YASM_LIB_DECL
yasm_object *yasm_section_get_object(const yasm_section *sect);

/** Get assocated data for a section and data callback.
 * \param sect      section
 * \param callback  callback used when adding data
 * \return Associated data (NULL if none).
 */
YASM_LIB_DECL
/*@dependent@*/ /*@null@*/ void *yasm_section_get_data
    (yasm_section *sect, const yasm_assoc_data_callback *callback);

/** Add associated data to a section.
 * \attention Deletes any existing associated data for that data callback.
 * \param sect      section
 * \param callback  callback
 * \param data      data to associate
 */
YASM_LIB_DECL
void yasm_section_add_data(yasm_section *sect,
                           const yasm_assoc_data_callback *callback,
                           /*@null@*/ /*@only@*/ void *data);

/** Add a relocation to a section.
 * \param sect          section
 * \param reloc         relocation
 * \param destroy_func  function that can destroy the relocation
 * \note Does not make a copy of reloc.  The same destroy_func must be
 * used for all relocations in a section or an internal error will occur.
 * The section will destroy the relocation address; it is the caller's
 * responsibility to destroy any other allocated data.
 */
YASM_LIB_DECL
void yasm_section_add_reloc(yasm_section *sect, yasm_reloc *reloc,
    void (*destroy_func) (/*@only@*/ void *reloc));

/** Get the first relocation for a section.
 * \param sect          section
 * \return First relocation for section.  NULL if no relocations.
 */
YASM_LIB_DECL
/*@null@*/ yasm_reloc *yasm_section_relocs_first(yasm_section *sect);

/** Get the next relocation for a section.
 * \param reloc         previous relocation
 * \return Next relocation for section.  NULL if no more relocations.
 */
/*@null@*/ yasm_reloc *yasm_section_reloc_next(yasm_reloc *reloc);
#ifndef YASM_DOXYGEN
#define yasm_section_reloc_next(x)      STAILQ_NEXT((x), link)
#endif

/** Get the basic relocation information for a relocation.
 * \param reloc         relocation
 * \param addrp         address of relocation within section (returned)
 * \param symp          relocated symbol (returned)
 */
YASM_LIB_DECL
void yasm_reloc_get(yasm_reloc *reloc, yasm_intnum **addrp,
                    /*@dependent@*/ yasm_symrec **symp);

/** Get the first bytecode in a section.
 * \param sect          section
 * \return First bytecode in section (at least one empty bytecode is always
 *         present).
 */
YASM_LIB_DECL
yasm_bytecode *yasm_section_bcs_first(yasm_section *sect);

/** Get the last bytecode in a section.
 * \param sect          section
 * \return Last bytecode in section (at least one empty bytecode is always
 *         present).
 */
YASM_LIB_DECL
yasm_bytecode *yasm_section_bcs_last(yasm_section *sect);

/** Add bytecode to the end of a section.
 * \note Does not make a copy of bc; so don't pass this function static or
 *       local variables, and discard the bc pointer after calling this
 *       function.
 * \param sect          section
 * \param bc            bytecode (may be NULL)
 * \return If bytecode was actually appended (it wasn't NULL or empty), the
 *         bytecode; otherwise NULL.
 */
YASM_LIB_DECL
/*@only@*/ /*@null@*/ yasm_bytecode *yasm_section_bcs_append
    (yasm_section *sect,
     /*@returned@*/ /*@only@*/ /*@null@*/ yasm_bytecode *bc);

/** Traverses all bytecodes in a section, calling a function on each bytecode.
 * \param sect      section
 * \param errwarns  error/warning set (may be NULL)
 * \param d         data pointer passed to func on each call (may be NULL)
 * \param func      function
 * \return Stops early (and returns func's return value) if func returns a
 *         nonzero value; otherwise 0.
 * \note If errwarns is non-NULL, yasm_errwarn_propagate() is called after
 *       each call to func (with the bytecode's line number).
 */
YASM_LIB_DECL
int yasm_section_bcs_traverse
    (yasm_section *sect, /*@null@*/ yasm_errwarns *errwarns,
     /*@null@*/ void *d, int (*func) (yasm_bytecode *bc, /*@null@*/ void *d));

/** Get name of a section.
 * \param   sect    section
 * \return Section name.
 */
YASM_LIB_DECL
/*@observer@*/ const char *yasm_section_get_name(const yasm_section *sect);

/** Change alignment of a section.
 * \param sect      section
 * \param align     alignment in bytes
 * \param line      virtual line
 */
YASM_LIB_DECL
void yasm_section_set_align(yasm_section *sect, unsigned long align,
                            unsigned long line);

/** Get alignment of a section.
 * \param sect      section
 * \return Alignment in bytes (0 if none).
 */
YASM_LIB_DECL
unsigned long yasm_section_get_align(const yasm_section *sect);

/** Print a section.  For debugging purposes.
 * \param f             file
 * \param indent_level  indentation level
 * \param sect          section
 * \param print_bcs     if nonzero, print bytecodes within section
 */
YASM_LIB_DECL
void yasm_section_print(/*@null@*/ const yasm_section *sect, FILE *f,
                        int indent_level, int print_bcs);

#endif
