/**
 * \file libyasm/errwarn.h
 * \brief YASM error and warning reporting interface.
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
#ifndef YASM_ERRWARN_H
#define YASM_ERRWARN_H

#ifndef YASM_LIB_DECL
#define YASM_LIB_DECL
#endif

/** Warning classes (that may be enabled/disabled). */
typedef enum yasm_warn_class {
    YASM_WARN_NONE = 0,     /**< No warning */
    YASM_WARN_GENERAL,      /**< Non-specific warnings */
    YASM_WARN_UNREC_CHAR,   /**< Unrecognized characters (while tokenizing) */
    YASM_WARN_PREPROC,      /**< Preprocessor warnings */
    YASM_WARN_ORPHAN_LABEL, /**< Label alone on a line without a colon */
    YASM_WARN_UNINIT_CONTENTS, /**< Uninitialized space in code/data section */
    YASM_WARN_SIZE_OVERRIDE,/**< Double size override */
    YASM_WARN_IMPLICIT_SIZE_OVERRIDE /**< Implicit size override */
} yasm_warn_class;

/** Error classes.  Bitmask-based to support limited subclassing. */
typedef enum yasm_error_class {
    YASM_ERROR_NONE             = 0x0000, /**< No error */
    YASM_ERROR_GENERAL          = 0xFFFF, /**< Non-specific */
    YASM_ERROR_ARITHMETIC       = 0x0001, /**< Arithmetic error (general) */
    YASM_ERROR_OVERFLOW         = 0x8001, /**< Arithmetic overflow */
    YASM_ERROR_FLOATING_POINT   = 0x4001, /**< Floating point error */
    YASM_ERROR_ZERO_DIVISION    = 0x2001, /**< Divide-by-zero */
    YASM_ERROR_ASSERTION        = 0x0002, /**< Assertion error */
    YASM_ERROR_VALUE            = 0x0004, /**< Value inappropriate
                                           *   (e.g. not in range) */
    YASM_ERROR_NOT_ABSOLUTE     = 0x8004, /**< Absolute expression required */
    YASM_ERROR_TOO_COMPLEX      = 0x4004, /**< Expression too complex */
    YASM_ERROR_NOT_CONSTANT     = 0x2004, /**< Constant expression required */
    YASM_ERROR_IO               = 0x0008, /**< I/O error */
    YASM_ERROR_NOT_IMPLEMENTED  = 0x0010, /**< Not implemented error */
    YASM_ERROR_TYPE             = 0x0020, /**< Type error */
    YASM_ERROR_SYNTAX           = 0x0040, /**< Syntax error */
    YASM_ERROR_PARSE            = 0x8040  /**< Parser error */
} yasm_error_class;

/** Initialize any internal data structures. */
YASM_LIB_DECL
void yasm_errwarn_initialize(void);

/** Clean up any memory allocated by yasm_errwarn_initialize() or other
 * functions.
 */
YASM_LIB_DECL
void yasm_errwarn_cleanup(void);

/** Reporting point of internal errors.  These are usually due to sanity
 * check failures in the code.
 * \warning This function must NOT return to calling code; exit or longjmp
 *          instead.
 * \param file      source file (ala __FILE__)
 * \param line      source line (ala __LINE__)
 * \param message   internal error message
 */
YASM_LIB_DECL
extern /*@exits@*/ void (*yasm_internal_error_)
    (const char *file, unsigned int line, const char *message);

/** Easily-callable version of yasm_internal_error_().  Automatically uses
 * __FILE__ and __LINE__ as the file and line.
 * \param message   internal error message
 */
#define yasm_internal_error(message) \
    yasm_internal_error_(__FILE__, __LINE__, message)

/** Reporting point of fatal errors.
 * \warning This function must NOT return to calling code; exit or longjmp
 *          instead.
 * \param message   fatal error message
 * \param va        va_list argument list for message
 */
YASM_LIB_DECL
extern /*@exits@*/ void (*yasm_fatal) (const char *message, va_list va);

/** Reporting point of fatal errors, with variable arguments (internal only).
 * \warning This function calls #yasm_fatal, and thus does not return to the
 *          calling code.
 * \param message   fatal error message
 * \param ...       argument list for message
 */
YASM_LIB_DECL
/*@exits@*/ void yasm__fatal(const char *message, ...);

/** Unconditionally clear the error indicator, freeing any associated data.
 * Has no effect if the error indicator is not set.
 */
YASM_LIB_DECL
void yasm_error_clear(void);

/** Get the error indicator.  YASM_ERROR_NONE is returned if no error has
 * been set.  Note that as YASM_ERROR_NONE is 0, the return value can also
 * be treated as a boolean value.
 * \return Current error indicator.
 */
yasm_error_class yasm_error_occurred(void);

/** Check the error indicator against an error class.  To check if any error
 * has been set, check against the YASM_ERROR_GENERAL class.  This function
 * properly checks error subclasses.
 * \param eclass    base error class to check against
 * \return Nonzero if error indicator is set and a subclass of eclass, 0
 *         otherwise.
 */
YASM_LIB_DECL
int yasm_error_matches(yasm_error_class eclass);

#ifndef YASM_DOXYGEN
YASM_LIB_DECL
extern yasm_error_class yasm_eclass;
#define yasm_error_occurred()       yasm_eclass
#endif

/** Set the error indicator (va_list version).  Has no effect if the error
 * indicator is already set.
 * \param eclass    error class
 * \param format    printf format string
 * \param va        argument list for format
 */
YASM_LIB_DECL
void yasm_error_set_va(yasm_error_class eclass, const char *format, va_list va);

/** Set the error indicator.  Has no effect if the error indicator is already
 * set.
 * \param eclass    error class
 * \param format    printf format string
 * \param ...       argument list for format
 */
YASM_LIB_DECL
void yasm_error_set(yasm_error_class eclass, const char *format, ...)
    /*@printflike@*/;

/** Set a cross-reference for a new error (va_list version).  Has no effect
 * if the error indicator is already set (e.g. with yasm_error_set()).  This
 * function must be called prior to its corresponding yasm_error_set() call.
 * \param xrefline  virtual line to cross-reference to (should not be 0)
 * \param format    printf format string
 * \param va        argument list for format
 */
YASM_LIB_DECL
void yasm_error_set_xref_va(unsigned long xrefline, const char *format,
                            va_list va);

/** Set a cross-reference for a new error.  Has no effect if the error
 * indicator is already set (e.g. with yasm_error_set()).  This function
 * must be called prior to its corresponding yasm_error_set() call.
 * \param xrefline  virtual line to cross-reference to (should not be 0)
 * \param format    printf format string
 * \param ...       argument list for format
 */
YASM_LIB_DECL
void yasm_error_set_xref(unsigned long xrefline, const char *format, ...)
    /*@printflike@*/;

/** Fetch the error indicator and all associated data.  If the error
 * indicator is set, the output pointers are set to the current error
 * indicator values, and the error indicator is cleared.
 * The code using this function is then responsible for yasm_xfree()'ing
 * str and xrefstr (if non-NULL).  If the error indicator is not set,
 * all output values are set to 0 (including eclass, which is set to
 * YASM_ERROR_NONE).
 * \param eclass    error class (output)
 * \param str       error message
 * \param xrefline  virtual line used for cross-referencing (0 if no xref)
 * \param xrefstr   cross-reference error message (NULL if no xref)
 */
YASM_LIB_DECL
void yasm_error_fetch(/*@out@*/ yasm_error_class *eclass,
                      /*@out@*/ /*@only@*/ /*@null@*/ char **str,
                      /*@out@*/ unsigned long *xrefline,
                      /*@out@*/ /*@only@*/ /*@null@*/ char **xrefstr);

/** Unconditionally clear all warning indicators, freeing any associated data.
 * Has no effect if no warning indicators have been set.
 */
YASM_LIB_DECL
void yasm_warn_clear(void);

/** Get the first warning indicator.  YASM_WARN_NONE is returned if no warning
 * has been set.  Note that as YASM_WARN_NONE is 0, the return value can also
 * be treated as a boolean value.
 * \return First warning indicator.
 */
YASM_LIB_DECL
yasm_warn_class yasm_warn_occurred(void);

/** Add a warning indicator (va_list version).
 * \param wclass    warning class
 * \param format    printf format string
 * \param va        argument list for format
 */
YASM_LIB_DECL
void yasm_warn_set_va(yasm_warn_class wclass, const char *format, va_list va);

/** Add a warning indicator.
 * \param wclass    warning class
 * \param format    printf format string
 * \param ...       argument list for format
 */
YASM_LIB_DECL
void yasm_warn_set(yasm_warn_class wclass, const char *format, ...)
    /*@printflike@*/;

/** Fetch the first warning indicator and all associated data.  If there
 * is at least one warning indicator, the output pointers are set to the
 * first warning indicator values, and first warning indicator is removed.
 * The code using this function is then responsible for yasm_xfree()'ing
 * str and xrefstr (if non-NULL).  If there is no warning indicator set,
 * all output values are set to 0 (including wclass, which is set to
 * YASM_WARN_NONE).
 * \param wclass    warning class (output)
 * \param str       warning message
 */
YASM_LIB_DECL
void yasm_warn_fetch(/*@out@*/ yasm_warn_class *wclass,
                     /*@out@*/ /*@only@*/ char **str);

/** Enable a class of warnings.
 * \param wclass    warning class
 */
YASM_LIB_DECL
void yasm_warn_enable(yasm_warn_class wclass);

/** Disable a class of warnings.
 * \param wclass    warning class
 */
YASM_LIB_DECL
void yasm_warn_disable(yasm_warn_class wclass);

/** Disable all classes of warnings. */
YASM_LIB_DECL
void yasm_warn_disable_all(void);

/** Create an error/warning set for collection of multiple error/warnings.
 * \return Newly allocated set.
 */
YASM_LIB_DECL
/*@only@*/ yasm_errwarns *yasm_errwarns_create(void);

/** Destroy an error/warning set.
 * \param errwarns  error/warning set
 */
YASM_LIB_DECL
void yasm_errwarns_destroy(/*@only@*/ yasm_errwarns *errwarns);

/** Propagate error indicator and warning indicator(s) to an error/warning set.
 * Has no effect if the error indicator and warning indicator are not set.
 * Does not print immediately; yasm_errwarn_output_all() outputs
 * accumulated errors and warnings.
 * Generally multiple errors on the same line will be reported, but errors
 * of class YASM_ERROR_PARSE will get overwritten by any other class on the
 * same line.
 * \param errwarns  error/warning set
 * \param line      virtual line
 */
YASM_LIB_DECL
void yasm_errwarn_propagate(yasm_errwarns *errwarns, unsigned long line);

/** Get total number of errors logged.
 * \param errwarns          error/warning set
 * \param warning_as_error  if nonzero, warnings are treated as errors.
 * \return Number of errors.
 */
YASM_LIB_DECL
unsigned int yasm_errwarns_num_errors(yasm_errwarns *errwarns,
                                      int warning_as_error);

/** Print out an error.
 * \param fn            filename of source file
 * \param line          line number
 * \param msg           error message
 * \param xref_fn       cross-referenced source filename
 * \param xref_line     cross-referenced line number
 * \param xref_msg      cross-referenced error message
 */
typedef void (*yasm_print_error_func)
    (const char *fn, unsigned long line, const char *msg,
     /*@null@*/ const char *xref_fn, unsigned long xref_line,
     /*@null@*/ const char *xref_msg);

/** Print out a warning.
 * \param fn    filename of source file
 * \param line  line number
 * \param msg   warning message
 */
typedef void (*yasm_print_warning_func)
    (const char *fn, unsigned long line, const char *msg);

/** Outputs error/warning set in sorted order (sorted by virtual line number).
 * \param errwarns          error/warning set
 * \param lm    line map (to convert virtual lines into filename/line pairs)
 * \param warning_as_error  if nonzero, treat warnings as errors.
 * \param print_error       function called to print out errors
 * \param print_warning     function called to print out warnings
 */
YASM_LIB_DECL
void yasm_errwarns_output_all
    (yasm_errwarns *errwarns, yasm_linemap *lm, int warning_as_error,
     yasm_print_error_func print_error, yasm_print_warning_func print_warning);

/** Convert a possibly unprintable character into a printable string.
 * \internal
 * \param ch    possibly unprintable character
 * \return Printable string representation (static buffer).
 */
YASM_LIB_DECL
char *yasm__conv_unprint(int ch);

/** Hook for library users to map to gettext() if GNU gettext is being used.
 * \param msgid     message catalog identifier
 * \return Translated message.
 */
YASM_LIB_DECL
extern const char * (*yasm_gettext_hook) (const char *msgid);

#endif
