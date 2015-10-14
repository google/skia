/**
 * \file libyasm/valparam.h
 * \brief YASM value/parameter interface.
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
#ifndef YASM_VALPARAM_H
#define YASM_VALPARAM_H

#ifndef YASM_LIB_DECL
#define YASM_LIB_DECL
#endif

/** Value/parameter pair.  \internal */
struct yasm_valparam {
    /*@reldef@*/ STAILQ_ENTRY(yasm_valparam) link;  /**< Next pair in list */
    /*@owned@*/ /*@null@*/ char *val;           /**< Value */

    /** Parameter type. */
    enum yasm_param_type {
        YASM_PARAM_ID,                          /**< Identifier */
        YASM_PARAM_STRING,                      /**< String */
        YASM_PARAM_EXPR                         /**< Expression */
    } type;                                     /**< Parameter type */

    /** Parameter value. */
    union yasm_param {
        /*@owned@*/ char *id;                   /**< Identifier */
        /*@owned@*/ char *str;                  /**< String */
        /*@owned@*/ yasm_expr *e;               /**< Expression */
    } param;                                    /**< Parameter */

    /** Prefix character that indicates a raw identifier.  When
     * yasm_vp_string() is called on a #YASM_PARAM_ID, all characters are
     * returned.  When yasm_vp_id() is called on a #YASM_PARAM_ID, if the
     * identifier begins with this character, this character is stripped
     * from the returned value.
     */
    char id_prefix;
};

/** Linked list of value/parameter pairs.  \internal */
/*@reldef@*/ STAILQ_HEAD(yasm_valparamhead, yasm_valparam);

/** Directive list entry structure. */
struct yasm_directive {
    /** Directive name.  GAS directives should include the ".", NASM
     * directives should just be the raw name (not including the []).
     * NULL entry required to terminate list of directives.
     */
    /*@null@*/ const char *name;

    const char *parser;                     /**< Parser keyword */

    /** Handler callback function for the directive.
     * \param object            object 
     * \param valparams         value/parameters
     * \param objext_valparams  object format-specific value/parameters
     * \param line              virtual line (from yasm_linemap)
     */
    void (*handler) (yasm_object *object, yasm_valparamhead *valparams,
                     yasm_valparamhead *objext_valparams, unsigned long line);

    /** Flags for pre-handler parameter checking. */
    enum yasm_directive_flags {
        YASM_DIR_ANY = 0,           /**< Any valparams accepted */
        YASM_DIR_ARG_REQUIRED = 1,  /**< Require at least 1 valparam */
        YASM_DIR_ID_REQUIRED = 2    /**< First valparam must be ID */
    } flags;
};

/** Call a directive.  Performs any valparam checks asked for by the
 * directive prior to call.  Note that for a variety of reasons, a directive
 * can generate an error.
 * \param directive             directive
 * \param object                object 
 * \param valparams             value/parameters
 * \param objext_valparams      object format-specific value/parameters
 * \param line                  virtual line (from yasm_linemap)
 */
YASM_LIB_DECL
void yasm_call_directive(const yasm_directive *directive, yasm_object *object,
                         yasm_valparamhead *valparams,
                         yasm_valparamhead *objext_valparams,
                         unsigned long line);

/** Create a new valparam with identifier parameter.
 * \param v             value
 * \param p             parameter
 * \param id_prefix     identifier prefix for raw identifiers
 * \return Newly allocated valparam.
 */
YASM_LIB_DECL
yasm_valparam *yasm_vp_create_id(/*@keep@*/ char *v, /*@keep@*/ char *p,
                                 int id_prefix);

/** Create a new valparam with string parameter.
 * \param v     value
 * \param p     parameter
 * \return Newly allocated valparam.
 */
YASM_LIB_DECL
yasm_valparam *yasm_vp_create_string(/*@keep@*/ char *v, /*@keep@*/ char *p);

/** Create a new valparam with expression parameter.
 * \param v     value
 * \param p     parameter
 * \return Newly allocated valparam.
 */
YASM_LIB_DECL
yasm_valparam *yasm_vp_create_expr(/*@keep@*/ char *v,
                                   /*@keep@*/ yasm_expr *p);

/** Get a valparam parameter as an expr.  If the parameter is an identifier,
 * it's treated as a symbol (yasm_symtab_use() is called to convert it).
 * \param vp            valparam
 * \param symtab        symbol table
 * \param line          virtual line
 * \return Expression, or NULL if vp is NULL or the parameter cannot be
 *         converted to an expression.
 */
YASM_LIB_DECL
/*@null@*/ /*@only@*/ yasm_expr *yasm_vp_expr
    (const yasm_valparam *vp, yasm_symtab *symtab, unsigned long line);

/** Get a valparam parameter as a string.  If the parameter is an identifier,
 * it's treated as a string.
 * \param vp            valparam
 * \return String, or NULL if vp is NULL or the parameter cannot be realized
 *         as a string.
 */
YASM_LIB_DECL
/*@null@*/ /*@dependent@*/ const char *yasm_vp_string(const yasm_valparam *vp);

/** Get a valparam parameter as an identifier.
 * \param vp            valparam
 * \return Identifier (string), or NULL if vp is NULL or the parameter is not
 *         an identifier.
 */
YASM_LIB_DECL
/*@null@*/ /*@dependent@*/ const char *yasm_vp_id(const yasm_valparam *vp);

/** Create a new linked list of valparams.
 * \return Newly allocated valparam list.
 */
YASM_LIB_DECL
yasm_valparamhead *yasm_vps_create(void);

/** Destroy a list of valparams (created with yasm_vps_create).
 * \param headp         list of valparams
 */
YASM_LIB_DECL
void yasm_vps_destroy(yasm_valparamhead *headp);

/** Initialize linked list of valparams.
 * \param headp linked list
 */
void yasm_vps_initialize(/*@out@*/ yasm_valparamhead *headp);
#ifndef YASM_DOXYGEN
#define yasm_vps_initialize(headp)      STAILQ_INIT(headp)
#endif

/** Destroy (free allocated memory for) linked list of valparams (created with
 * yasm_vps_initialize).
 * \warning Deletes val/params.
 * \param headp linked list
 */
YASM_LIB_DECL
void yasm_vps_delete(yasm_valparamhead *headp);

/** Append valparam to tail of linked list.
 * \param headp linked list
 * \param vp    valparam
 */
void yasm_vps_append(yasm_valparamhead *headp, /*@keep@*/ yasm_valparam *vp);
#ifndef YASM_DOXYGEN
#define yasm_vps_append(headp, vp)      do {        \
        if (vp)                                     \
            STAILQ_INSERT_TAIL(headp, vp, link);    \
    } while(0)
#endif

/** Get first valparam in linked list.
 * \param headp linked list
 * \return First valparam in linked list.
 */
/*@null@*/ /*@dependent@*/ yasm_valparam *yasm_vps_first
    (yasm_valparamhead *headp);
#ifndef YASM_DOXYGEN
#define yasm_vps_first(headp)       STAILQ_FIRST(headp)
#endif

/** Get next valparam in linked list.
 * \param cur   previous valparam in linked list
 * \return Next valparam in linked list.
 */
/*@null@*/ /*@dependent@*/ yasm_valparam *yasm_vps_next(yasm_valparam *cur);
#ifndef YASM_DOXYGEN
#define yasm_vps_next(cur)          STAILQ_NEXT(cur, link)
#endif

/** Iterate through linked list of valparams.
 * \internal
 * \param iter      iterator variable
 * \param headp     linked list
 */
#ifndef YASM_DOXYGEN
#define yasm_vps_foreach(iter, headp)   STAILQ_FOREACH(iter, headp, link)
#endif

/** Print linked list of valparams.  For debugging purposes.
 * \param f     file
 * \param headp linked list
 */
YASM_LIB_DECL
void yasm_vps_print(/*@null@*/ const yasm_valparamhead *headp, FILE *f);

/** Directive valparam parse helper structure. */
typedef struct yasm_dir_help {
    /** Value portion of val=param (if needsparam=1), or standalone identifier
     * (if needsparam=0).
     */
    const char *name;

    /** 1 if value requires parameter, 0 if it must not have a parameter. */
    int needsparam;

    /** Helper callback function if name and parameter existence match.
     * \param obj       obj passed into yasm_dir_helper()
     * \param vp        value/parameter
     * \param line      line passed into yasm_dir_helper()
     * \param data      data passed into yasm_dir_helper() plus
                        #yasm_dir_help.off offset
     * \param arg       #yasm_dir_help.arg argument
     * \return -1 on error, 0 otherwise.
     */
    int (*helper) (void *obj, yasm_valparam *vp, unsigned long line,
                   void *data, uintptr_t arg);

    /** Offset added to data pointer passed into yasm_dir_helper() before
     * data pointer is given to #yasm_dir_help.helper().  This is so that
     * a structure can be passed into yasm_dir_helper() and this can be an
     * offsetof() to point the helper function to a specific structure
     * member.
     */
    size_t off;

    /** Argument to pass in as the arg parameter to #yasm_dir_help.helper().
     */
    uintptr_t arg;
} yasm_dir_help;

/** Help parse a list of directive value/parameters.  Takes an array of
 * #yasm_dir_help structures and tries to match val=param (or just val)
 * against the passed value/parameters.  When no match is found in the
 * array of help structures, calls helper_valparam.
 * \param obj       object to be passed to yasm_dir_help.helper() or
 *                  helper_valparam() callback
 * \param vp_first  first value/parameter to examine
 * \param line      virtual line number; passed down to helper callback
 * \param help      array of #yasm_dir_help structures
 * \param nhelp     number of array elements
 * \param data      base data pointer; if a match is found,
 *                  the respective #yasm_dir_help.off is added to this
 *                  prior to it being passed to the helper callback
 * \param helper_valparam   catch-all callback; should return -1 on error,
 *                          0 if not matched, 1 if matched.
 * \return -1 on error, 1 if any arguments matched (including via
 *         catch-all callback), 0 if no match.
 */
YASM_LIB_DECL
int yasm_dir_helper(void *obj, yasm_valparam *vp_first, unsigned long line,
                    const yasm_dir_help *help, size_t nhelp, void *data,
                    int (*helper_valparam) (void *object,
                                            yasm_valparam *vp,
                                            unsigned long line,
                                            void *data));

/** Standard helper for yasm_dir_helper() that simply sets a flag when called.
 * It does not look at the vp; rather, it uses the value of the arg parameter,
 * and stores an unsigned long value to data.
 * \param obj   unused
 * \param vp    unused
 * \param line  unused
 * \param data  pointer to an unsigned long
 * \param arg   flag to set
 * \return 0
 */
YASM_LIB_DECL
int yasm_dir_helper_flag_set(void *obj, yasm_valparam *vp, unsigned long line,
                             void *data, uintptr_t arg);

/** Standard helper for yasm_dir_helper() that simply ORs a flag when called.
 * It does not look at the vp; rather, it uses the value of the arg parameter,
 * and ORs it with the unsigned long value in data.
 * \param obj   unused
 * \param vp    unused
 * \param line  unused
 * \param data  pointer to an unsigned long
 * \param arg   flag to OR
 * \return 0
 */
YASM_LIB_DECL
int yasm_dir_helper_flag_or(void *obj, yasm_valparam *vp, unsigned long line,
                            void *data, uintptr_t arg);

/** Standard helper for yasm_dir_helper() that simply ANDs a flag when called.
 * It does not look at the vp; rather, it uses the value of the arg parameter,
 * and ANDs its inverse (~) with the unsigned long value in data.
 * \param obj   unused
 * \param vp    unused
 * \param line  unused
 * \param data  pointer to an unsigned long
 * \param arg   flag to AND
 * \return 0
 */
YASM_LIB_DECL
int yasm_dir_helper_flag_and(void *obj, yasm_valparam *vp, unsigned long line,
                             void *data, uintptr_t arg);

/** Standard helper for yasm_dir_helper() that parses an expr parameter.
 * The #yasm_dir_help structure that uses this function should have
 * needsparam=1.  The obj parameter to yasm_dir_helper() when this helper
 * is used MUST point to a #yasm_object.  In addition, the data parameter
 * that is ultimately passed to this function (e.g. yasm_dir_helper() data
 * parameter plus #yasm_dir_help.off) must point to a #yasm_expr *
 * initialized to NULL.
 * \param obj   object; must be #yasm_object
 * \param vp    valparam
 * \param line  virtual line number
 * \param data  pointer to #yasm_expr *
 * \param arg   unused argument
 * \return -1 on error, 0 otherwise.
 */
YASM_LIB_DECL
int yasm_dir_helper_expr(void *obj, yasm_valparam *vp, unsigned long line,
                         void *data, uintptr_t arg);

/** Standard helper for yasm_dir_helper() that parses an intnum parameter.
 * The #yasm_dir_help structure that uses this function should have
 * needsparam=1.  The obj parameter to yasm_dir_helper() when this helper
 * is used MUST point to a #yasm_object.  In addition, the data parameter
 * that is ultimately passed to this function (e.g. yasm_dir_helper() data
 * parameter plus #yasm_dir_help.off) must point to a #yasm_intnum *
 * initialized to NULL.
 * \param obj   object; must be #yasm_object
 * \param vp    valparam
 * \param line  virtual line number
 * \param data  pointer to #yasm_intnum *
 * \param arg   unused argument
 * \return -1 on error, 0 otherwise.
 */
YASM_LIB_DECL
int yasm_dir_helper_intn(void *obj, yasm_valparam *vp, unsigned long line,
                         void *data, uintptr_t arg);

/** Standard helper for yasm_dir_helper() that parses an string (or
 * standalone identifier) parameter.
 * The #yasm_dir_help structure that uses this function should have
 * needsparam=1.  The data parameter that is ultimately passed to this
 * function (e.g. yasm_dir_helper() data parameter plus #yasm_dir_help.off)
 * must point to a char * initialized to NULL.
 * \param obj   unused
 * \param vp    valparam
 * \param line  unused
 * \param data  pointer to char *
 * \param arg   unused
 * \return -1 on error, 0 otherwise.
 */
YASM_LIB_DECL
int yasm_dir_helper_string(void *obj, yasm_valparam *vp, unsigned long line,
                           void *data, uintptr_t arg);

/** Standard catch-all callback fro yasm_dir_helper().  Generates standard
 * warning for all valparams.
 * \param obj   unused
 * \param vp    valparam
 * \param line  unused
 * \param data  unused
 * \return 0
 */
YASM_LIB_DECL
int yasm_dir_helper_valparam_warn(void *obj, yasm_valparam *vp,
                                  unsigned long line, void *data);
#endif
