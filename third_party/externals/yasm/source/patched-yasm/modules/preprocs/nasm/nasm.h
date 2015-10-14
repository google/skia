/* nasm.h   main header file for the Netwide Assembler: inter-module interface
 *
 * The Netwide Assembler is copyright (C) 1996 Simon Tatham and
 * Julian Hall. All rights reserved. The software is
 * redistributable under the licence given in the file "Licence"
 * distributed in the NASM archive.
 *
 * initial version: 27/iii/95 by Simon Tatham
 */
#ifndef YASM_NASM_H
#define YASM_NASM_H

#ifndef NULL
#define NULL 0
#endif

#ifndef FALSE
#define FALSE 0                        /* comes in handy */
#endif
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FILENAME_MAX
#define FILENAME_MAX 256
#endif

#ifndef PREFIX_MAX
#define PREFIX_MAX 10
#endif

#ifndef POSTFIX_MAX
#define POSTFIX_MAX 10
#endif

#define IDLEN_MAX 4096

/*
 * -------------------------
 * Error reporting functions
 * -------------------------
 */

/*
 * An error reporting function should look like this.
 */
typedef void (*efunc) (int severity, const char *fmt, ...);

/*
 * These are the error severity codes which get passed as the first
 * argument to an efunc.
 */

#define ERR_DEBUG       0x00000008      /* put out debugging message */
#define ERR_WARNING     0x00000000      /* warn only: no further action */
#define ERR_NONFATAL    0x00000001      /* terminate assembly after phase */
#define ERR_FATAL       0x00000002      /* instantly fatal: exit with error */
#define ERR_PANIC       0x00000003      /* internal error: panic instantly
                                        * and dump core for reference */
#define ERR_MASK        0x0000000F      /* mask off the above codes */
#define ERR_NOFILE      0x00000010      /* don't give source file name/line */
#define ERR_USAGE       0x00000020      /* print a usage message */
#define ERR_PASS1       0x00000040      /* only print this error on pass one */

/*
 * These codes define specific types of suppressible warning.
 */

#define ERR_WARN_MASK   0x0000FF00      /* the mask for this feature */
#define ERR_WARN_SHR  8                /* how far to shift right */

#define ERR_WARN_MNP    0x00000100      /* macro-num-parameters warning */
#define ERR_WARN_MSR    0x00000200      /* macro self-reference */
#define ERR_WARN_OL     0x00000300      /* orphan label (no colon, and
                                        * alone on line) */
#define ERR_WARN_NOV    0x00000400      /* numeric overflow */
#define ERR_WARN_GNUELF 0x00000500      /* using GNU ELF extensions */
#define ERR_WARN_MAX    5               /* the highest numbered one */

/*
 * -----------------------
 * Other function typedefs
 * -----------------------
 */

/*
 * List-file generators should look like this:
 */
typedef struct {
    /*
     * Called to initialise the listing file generator. Before this
     * is called, the other routines will silently do nothing when
     * called. The `char *' parameter is the file name to write the
     * listing to.
     */
    void (*init) (char *, efunc);

    /*
     * Called to clear stuff up and close the listing file.
     */
    void (*cleanup) (void);

    /*
     * Called to output binary data. Parameters are: the offset;
     * the data; the data type. Data types are similar to the
     * output-format interface, only OUT_ADDRESS will _always_ be
     * displayed as if it's relocatable, so ensure that any non-
     * relocatable address has been converted to OUT_RAWDATA by
     * then. Note that OUT_RAWDATA+0 is a valid data type, and is a
     * dummy call used to give the listing generator an offset to
     * work with when doing things like uplevel(LIST_TIMES) or
     * uplevel(LIST_INCBIN).
     */
    void (*output) (long, const void *, unsigned long);

    /*
     * Called to send a text line to the listing generator. The
     * `int' parameter is LIST_READ or LIST_MACRO depending on
     * whether the line came directly from an input file or is the
     * result of a multi-line macro expansion.
     */
    void (*line) (int, char *);

    /*
     * Called to change one of the various levelled mechanisms in
     * the listing generator. LIST_INCLUDE and LIST_MACRO can be
     * used to increase the nesting level of include files and
     * macro expansions; LIST_TIMES and LIST_INCBIN switch on the
     * two binary-output-suppression mechanisms for large-scale
     * pseudo-instructions.
     *
     * LIST_MACRO_NOLIST is synonymous with LIST_MACRO except that
     * it indicates the beginning of the expansion of a `nolist'
     * macro, so anything under that level won't be expanded unless
     * it includes another file.
     */
    void (*uplevel) (int);

    /*
     * Reverse the effects of uplevel.
     */
    void (*downlevel) (int);
} ListGen;

/*
 * The expression evaluator must be passed a scanner function; a
 * standard scanner is provided as part of nasmlib.c. The
 * preprocessor will use a different one. Scanners, and the
 * token-value structures they return, look like this.
 *
 * The return value from the scanner is always a copy of the
 * `t_type' field in the structure.
 */
struct tokenval {
    int t_type;
    yasm_intnum *t_integer, *t_inttwo;
    char *t_charptr;
};
typedef int (*scanner) (void *private_data, struct tokenval *tv);

/*
 * Token types returned by the scanner, in addition to ordinary
 * ASCII character values, and zero for end-of-string.
 */
enum {                                 /* token types, other than chars */
    TOKEN_INVALID = -1,                /* a placeholder value */
    TOKEN_EOS = 0,                     /* end of string */
    TOKEN_EQ = '=', TOKEN_GT = '>', TOKEN_LT = '<',   /* aliases */
    TOKEN_ID = 256, TOKEN_NUM, TOKEN_REG, TOKEN_INSN,  /* major token types */
    TOKEN_ERRNUM,                      /* numeric constant with error in */
    TOKEN_HERE, TOKEN_BASE,            /* $ and $$ */
    TOKEN_SPECIAL,                     /* BYTE, WORD, DWORD, FAR, NEAR, etc */
    TOKEN_PREFIX,                      /* A32, O16, LOCK, REPNZ, TIMES, etc */
    TOKEN_SHL, TOKEN_SHR,              /* << and >> */
    TOKEN_SDIV, TOKEN_SMOD,            /* // and %% */
    TOKEN_GE, TOKEN_LE, TOKEN_NE,      /* >=, <= and <> (!= is same as <>) */
    TOKEN_DBL_AND, TOKEN_DBL_OR, TOKEN_DBL_XOR,   /* &&, || and ^^ */
    TOKEN_SEG, TOKEN_WRT,              /* SEG and WRT */
    TOKEN_FLOAT                        /* floating-point constant */
};

/*
 * The actual expression evaluator function looks like this. When
 * called, it expects the first token of its expression to already
 * be in `*tv'; if it is not, set tv->t_type to TOKEN_INVALID and
 * it will start by calling the scanner.
 *
 * `critical' is non-zero if the expression may not contain forward
 * references. The evaluator will report its own error if this
 * occurs; if `critical' is 1, the error will be "symbol not
 * defined before use", whereas if `critical' is 2, the error will
 * be "symbol undefined".
 *
 * If `critical' has bit 8 set (in addition to its main value: 0x101
 * and 0x102 correspond to 1 and 2) then an extended expression
 * syntax is recognised, in which relational operators such as =, <
 * and >= are accepted, as well as low-precedence logical operators
 * &&, ^^ and ||.
 */
#define CRITICAL 0x100
typedef yasm_expr *(*evalfunc) (scanner sc, void *scprivate, struct tokenval *tv,
                           int critical, efunc error);

/*
 * Preprocessors ought to look like this:
 */
typedef struct {
    /*
     * Called at the start of a pass; given a file name, the number
     * of the pass, an error reporting function, an evaluator
     * function, and a listing generator to talk to.
     */
    void (*reset) (FILE *, const char *, int, efunc, evalfunc, ListGen *);

    /*
     * Called to fetch a line of preprocessed source. The line
     * returned has been malloc'ed, and so should be freed after
     * use.
     */
    char *(*getline) (void);

    /*
     * Called at the end of a pass.
     */
    void (*cleanup) (int);
} Preproc;

/*
 * ----------------------------------------------------------------
 * Some lexical properties of the NASM source language, included
 * here because they are shared between the parser and preprocessor
 * ----------------------------------------------------------------
 */

/*
 * isidstart matches any character that may start an identifier, and isidchar
 * matches any character that may appear at places other than the start of an
 * identifier. E.g. a period may only appear at the start of an identifier
 * (for local labels), whereas a number may appear anywhere *but* at the
 * start. 
 */

#define isidstart(c) ( isalpha(c) || (c)=='_' || (c)=='.' || (c)=='?' \
                                  || (c)=='@' )
#define isidchar(c)  ( isidstart(c) || isdigit(c) || (c)=='$' || (c)=='#' \
                                                  || (c)=='~' )

/* Ditto for numeric constants. */

#define isnumstart(c)  ( isdigit(c) || (c)=='$' )
#define isnumchar(c)   ( isalnum(c) )

/* This returns the numeric value of a given 'digit'. */

#define numvalue(c)  ((c)>='a' ? (c)-'a'+10 : (c)>='A' ? (c)-'A'+10 : (c)-'0')

/*
 * Data-type flags that get passed to listing-file routines.
 */
enum {
    LIST_READ, LIST_MACRO, LIST_MACRO_NOLIST, LIST_INCLUDE,
    LIST_INCBIN, LIST_TIMES
};

/*
 * -----
 * Other
 * -----
 */

/*
 * This is a useful #define which I keep meaning to use more often:
 * the number of elements of a statically defined array.
 */

#define elements(x)     ( sizeof(x) / sizeof(*(x)) )

extern int tasm_compatible_mode;
extern int tasm_locals;
extern const char *tasm_segment;
const char *tasm_get_segment_register(const char *segment);

#endif
