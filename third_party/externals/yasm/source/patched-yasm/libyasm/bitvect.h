#ifndef YASM_BITVECT_H
#define YASM_BITVECT_H
/*****************************************************************************/
/*  MODULE NAME:  BitVector.h                           MODULE TYPE:  (adt)  */
/*****************************************************************************/
/*  MODULE IMPORTS:                                                          */
/*****************************************************************************/

/* ToolBox.h */
/*****************************************************************************/
/*  NOTE: The type names that have been chosen here are somewhat weird on    */
/*        purpose, in order to avoid name clashes with system header files   */
/*        and your own application(s) which might - directly or indirectly - */
/*        include this definitions file.                                     */
/*****************************************************************************/
#ifndef YASM_LIB_DECL
#define YASM_LIB_DECL
#endif

typedef  unsigned   char    N_char;
typedef  unsigned   char    N_byte;
typedef  unsigned   short   N_short;
typedef  unsigned   short   N_shortword;
typedef  unsigned   int     N_int;
typedef  unsigned   int     N_word;
typedef  unsigned   long    N_long;
typedef  unsigned   long    N_longword;

/*  Mnemonic 1:  The natural numbers,  N = { 0, 1, 2, 3, ... }               */
/*  Mnemonic 2:  Nnnn = u_N_signed,  _N_ot signed                            */

typedef  signed     char    Z_char;
typedef  signed     char    Z_byte;
typedef  signed     short   Z_short;
typedef  signed     short   Z_shortword;
typedef  signed     int     Z_int;
typedef  signed     int     Z_word;
typedef  signed     long    Z_long;
typedef  signed     long    Z_longword;

/*  Mnemonic 1:  The whole numbers,  Z = { 0, -1, 1, -2, 2, -3, 3, ... }     */
/*  Mnemonic 2:  Zzzz = Ssss_igned                                           */

typedef  void               *voidptr;
typedef  N_char             *charptr;
typedef  N_byte             *byteptr;
typedef  N_short            *shortptr;
typedef  N_shortword        *shortwordptr;
typedef  N_int              *intptr;
typedef  N_word             *wordptr;
typedef  N_long             *longptr;
typedef  N_longword         *longwordptr;

typedef  N_char             *N_charptr;
typedef  N_byte             *N_byteptr;
typedef  N_short            *N_shortptr;
typedef  N_shortword        *N_shortwordptr;
typedef  N_int              *N_intptr;
typedef  N_word             *N_wordptr;
typedef  N_long             *N_longptr;
typedef  N_longword         *N_longwordptr;

typedef  Z_char             *Z_charptr;
typedef  Z_byte             *Z_byteptr;
typedef  Z_short            *Z_shortptr;
typedef  Z_shortword        *Z_shortwordptr;
typedef  Z_int              *Z_intptr;
typedef  Z_word             *Z_wordptr;
typedef  Z_long             *Z_longptr;
typedef  Z_longword         *Z_longwordptr;

#ifndef FALSE
#define FALSE       (0!=0)
#endif

#ifndef TRUE
#define TRUE        (0==0)
#endif

#ifdef __cplusplus
    typedef bool boolean;
#else
    #ifdef MACOS_TRADITIONAL
        #define boolean Boolean
    #else
        typedef enum boolean { false = FALSE, true = TRUE } boolean;
    #endif
#endif

/*****************************************************************************/
/*  MODULE INTERFACE:                                                        */
/*****************************************************************************/

typedef enum ErrCode
    {
        ErrCode_Ok = 0,    /* everything went allright                       */

        ErrCode_Type,      /* types word and size_t have incompatible sizes  */
        ErrCode_Bits,      /* bits of word and sizeof(word) are inconsistent */
        ErrCode_Word,      /* size of word is less than 16 bits              */
        ErrCode_Long,      /* size of word is greater than size of long      */
        ErrCode_Powr,      /* number of bits of word is not a power of two   */
        ErrCode_Loga,      /* error in calculation of logarithm              */

        ErrCode_Null,      /* unable to allocate memory                      */

        ErrCode_Indx,      /* index out of range                             */
        ErrCode_Ordr,      /* minimum > maximum index                        */
        ErrCode_Size,      /* bit vector size mismatch                       */
        ErrCode_Pars,      /* input string syntax error                      */
        ErrCode_Ovfl,      /* numeric overflow error                         */
        ErrCode_Same,      /* operands must be distinct                      */
        ErrCode_Expo,      /* exponent must be positive                      */
        ErrCode_Zero       /* division by zero error                         */
    } ErrCode;

typedef wordptr *listptr;

/* ===> MISCELLANEOUS BASIC FUNCTIONS: <=== */

YASM_LIB_DECL
const char * BitVector_Error      (ErrCode error);  /* return string for err code */

YASM_LIB_DECL
ErrCode BitVector_Boot       (void);                 /* 0 = ok, 1..7 = error */
YASM_LIB_DECL
void    BitVector_Shutdown   (void);                            /* undo Boot */

YASM_LIB_DECL
N_word  BitVector_Size       (N_int bits);  /* bit vector size (# of words)  */
YASM_LIB_DECL
N_word  BitVector_Mask       (N_int bits);  /* bit vector mask (unused bits) */

/* ===> CLASS METHODS: <=== */

YASM_LIB_DECL
const char * BitVector_Version    (void);          /* returns version string */

YASM_LIB_DECL
N_int   BitVector_Word_Bits  (void);     /* return # of bits in machine word */
YASM_LIB_DECL
N_int   BitVector_Long_Bits  (void);    /* return # of bits in unsigned long */

/* ===> CONSTRUCTOR METHODS: <=== */

YASM_LIB_DECL
/*@only@*/ wordptr BitVector_Create     (N_int bits, boolean clear);          /* malloc */
YASM_LIB_DECL
listptr BitVector_Create_List(N_int bits, boolean clear, N_int count);

YASM_LIB_DECL
wordptr BitVector_Resize     (wordptr oldaddr, N_int bits);       /* realloc */

YASM_LIB_DECL
wordptr BitVector_Shadow     (wordptr addr); /* make new same size but empty */
YASM_LIB_DECL
wordptr BitVector_Clone      (wordptr addr);         /* make exact duplicate */

YASM_LIB_DECL
wordptr BitVector_Concat     (wordptr X, wordptr Y); /* return concatenation */

/* ===> DESTRUCTOR METHODS: <=== */

YASM_LIB_DECL
void    BitVector_Dispose            (/*@only@*/ /*@out@*/ charptr string);             /* string */
YASM_LIB_DECL
void    BitVector_Destroy            (/*@only@*/ wordptr addr);               /* bitvec */
YASM_LIB_DECL
void    BitVector_Destroy_List       (listptr list, N_int count);  /* list   */

/* ===> OBJECT METHODS: <=== */

/* ===> bit vector copy function: */

YASM_LIB_DECL
void    BitVector_Copy       (wordptr X, wordptr Y);              /* X = Y   */

/* ===> bit vector initialization: */

YASM_LIB_DECL
void    BitVector_Empty      (wordptr addr);                      /* X = {}  */
YASM_LIB_DECL
void    BitVector_Fill       (wordptr addr);                      /* X = ~{} */
YASM_LIB_DECL
void    BitVector_Flip       (wordptr addr);                      /* X = ~X  */

YASM_LIB_DECL
void    BitVector_Primes     (wordptr addr);

/* ===> miscellaneous functions: */

YASM_LIB_DECL
void    BitVector_Reverse    (wordptr X, wordptr Y);

/* ===> bit vector interval operations and functions: */

YASM_LIB_DECL
void    BitVector_Interval_Empty     (/*@out@*/ wordptr addr, N_int lower, N_int upper);
YASM_LIB_DECL
void    BitVector_Interval_Fill      (/*@out@*/ wordptr addr, N_int lower, N_int upper);
YASM_LIB_DECL
void    BitVector_Interval_Flip      (/*@out@*/ wordptr addr, N_int lower, N_int upper);
YASM_LIB_DECL
void    BitVector_Interval_Reverse   (/*@out@*/ wordptr addr, N_int lower, N_int upper);

YASM_LIB_DECL
boolean BitVector_interval_scan_inc  (wordptr addr, N_int start,
                                      N_intptr min, N_intptr max);
YASM_LIB_DECL
boolean BitVector_interval_scan_dec  (wordptr addr, N_int start,
                                      N_intptr min, N_intptr max);

YASM_LIB_DECL
void    BitVector_Interval_Copy      (/*@out@*/ wordptr X, wordptr Y, N_int Xoffset,
                                      N_int Yoffset, N_int length);

YASM_LIB_DECL
wordptr BitVector_Interval_Substitute(/*@out@*/ wordptr X, wordptr Y,
                                      N_int Xoffset, N_int Xlength,
                                      N_int Yoffset, N_int Ylength);

/* ===> bit vector test functions: */

YASM_LIB_DECL
boolean BitVector_is_empty   (wordptr addr);                  /* X == {} ?   */
YASM_LIB_DECL
boolean BitVector_is_full    (wordptr addr);                  /* X == ~{} ?  */

YASM_LIB_DECL
boolean BitVector_equal      (wordptr X, wordptr Y);          /* X == Y ?    */
YASM_LIB_DECL
Z_int   BitVector_Lexicompare(wordptr X, wordptr Y);          /* X <,=,> Y ? */
YASM_LIB_DECL
Z_int   BitVector_Compare    (wordptr X, wordptr Y);          /* X <,=,> Y ? */

/* ===> bit vector string conversion functions: */

YASM_LIB_DECL
/*@only@*/ charptr BitVector_to_Hex     (wordptr addr);
YASM_LIB_DECL
ErrCode BitVector_from_Hex   (/*@out@*/wordptr addr, charptr string);

YASM_LIB_DECL
ErrCode BitVector_from_Oct(/*@out@*/ wordptr addr, charptr string);

YASM_LIB_DECL
/*@only@*/ charptr BitVector_to_Bin     (wordptr addr);
YASM_LIB_DECL
ErrCode BitVector_from_Bin   (/*@out@*/ wordptr addr, charptr string);

YASM_LIB_DECL
/*@only@*/ charptr BitVector_to_Dec     (wordptr addr);
YASM_LIB_DECL
ErrCode BitVector_from_Dec   (/*@out@*/ wordptr addr, charptr string);

typedef struct BitVector_from_Dec_static_data BitVector_from_Dec_static_data;
YASM_LIB_DECL
BitVector_from_Dec_static_data *BitVector_from_Dec_static_Boot(N_word bits);
YASM_LIB_DECL
void BitVector_from_Dec_static_Shutdown(/*@null@*/ BitVector_from_Dec_static_data *data);
YASM_LIB_DECL
ErrCode BitVector_from_Dec_static(BitVector_from_Dec_static_data *data,
                                  /*@out@*/ wordptr addr, charptr string);

YASM_LIB_DECL
/*@only@*/ charptr BitVector_to_Enum    (wordptr addr);
YASM_LIB_DECL
ErrCode BitVector_from_Enum  (/*@out@*/ wordptr addr, charptr string);

/* ===> bit vector bit operations, functions & tests: */

YASM_LIB_DECL
void    BitVector_Bit_Off    (/*@out@*/ wordptr addr, N_int indx); /*  X = X \ {x}    */
YASM_LIB_DECL
void    BitVector_Bit_On     (/*@out@*/ wordptr addr, N_int indx); /*  X = X + {x}    */
YASM_LIB_DECL
boolean BitVector_bit_flip   (/*@out@*/ wordptr addr, N_int indx); /* (X+{x})\(X*{x}) */

YASM_LIB_DECL
boolean BitVector_bit_test   (wordptr addr, N_int indx); /*  {x} in X ?     */

YASM_LIB_DECL
void    BitVector_Bit_Copy   (/*@out@*/ wordptr addr, N_int indx, boolean bit);

/* ===> bit vector bit shift & rotate functions: */

YASM_LIB_DECL
void    BitVector_LSB                (/*@out@*/ wordptr addr, boolean bit);
YASM_LIB_DECL
void    BitVector_MSB                (/*@out@*/ wordptr addr, boolean bit);
YASM_LIB_DECL
boolean BitVector_lsb_               (wordptr addr);
YASM_LIB_DECL
boolean BitVector_msb_               (wordptr addr);
YASM_LIB_DECL
boolean /*@alt void@*/ BitVector_rotate_left        (wordptr addr);
YASM_LIB_DECL
boolean /*@alt void@*/ BitVector_rotate_right       (wordptr addr);
YASM_LIB_DECL
boolean /*@alt void@*/ BitVector_shift_left         (wordptr addr, boolean carry_in);
YASM_LIB_DECL
boolean /*@alt void@*/ BitVector_shift_right        (wordptr addr, boolean carry_in);
YASM_LIB_DECL
void    BitVector_Move_Left          (wordptr addr, N_int bits);
YASM_LIB_DECL
void    BitVector_Move_Right         (wordptr addr, N_int bits);

/* ===> bit vector insert/delete bits: */

YASM_LIB_DECL
void    BitVector_Insert     (wordptr addr, N_int offset, N_int count,
                              boolean clear);
YASM_LIB_DECL
void    BitVector_Delete     (wordptr addr, N_int offset, N_int count,
                              boolean clear);

/* ===> bit vector arithmetic: */

YASM_LIB_DECL
boolean /*@alt void@*/ BitVector_increment  (wordptr addr);                        /*  X++  */
YASM_LIB_DECL
boolean /*@alt void@*/ BitVector_decrement  (wordptr addr);                        /*  X--  */

YASM_LIB_DECL
boolean /*@alt void@*/ BitVector_compute    (wordptr X, wordptr Y, wordptr Z, boolean minus,
                                                               boolean *carry);
YASM_LIB_DECL
boolean /*@alt void@*/ BitVector_add        (wordptr X, wordptr Y, wordptr Z, boolean *carry);
YASM_LIB_DECL
boolean /*@alt void@*/ BitVector_sub        (wordptr X, wordptr Y, wordptr Z, boolean *carry);
YASM_LIB_DECL
boolean /*@alt void@*/ BitVector_inc        (wordptr X, wordptr Y);
YASM_LIB_DECL
boolean /*@alt void@*/ BitVector_dec        (wordptr X, wordptr Y);

YASM_LIB_DECL
void    BitVector_Negate     (wordptr X, wordptr Y);
YASM_LIB_DECL
void    BitVector_Absolute   (wordptr X, wordptr Y);
YASM_LIB_DECL
Z_int   BitVector_Sign       (wordptr addr);
YASM_LIB_DECL
ErrCode BitVector_Mul_Pos    (wordptr X, wordptr Y, wordptr Z, boolean strict);
YASM_LIB_DECL
ErrCode BitVector_Multiply   (wordptr X, wordptr Y, wordptr Z);
YASM_LIB_DECL
ErrCode BitVector_Div_Pos    (wordptr Q, wordptr X, wordptr Y, wordptr R);
YASM_LIB_DECL
ErrCode BitVector_Divide     (wordptr Q, wordptr X, wordptr Y, wordptr R);
YASM_LIB_DECL
ErrCode BitVector_GCD        (wordptr X, wordptr Y, wordptr Z);
YASM_LIB_DECL
ErrCode BitVector_GCD2       (wordptr U, wordptr V, wordptr W,      /*   O   */
                                         wordptr X, wordptr Y);     /*   I   */
YASM_LIB_DECL
ErrCode BitVector_Power      (wordptr X, wordptr Y, wordptr Z);

/* ===> direct memory access functions: */

YASM_LIB_DECL
void    BitVector_Block_Store(wordptr addr, charptr buffer, N_int length);
YASM_LIB_DECL
charptr BitVector_Block_Read (wordptr addr, /*@out@*/ N_intptr length);

/* ===> word array functions: */

YASM_LIB_DECL
void    BitVector_Word_Store (wordptr addr, N_int offset, N_int value);
YASM_LIB_DECL
N_int   BitVector_Word_Read  (wordptr addr, N_int offset);

YASM_LIB_DECL
void    BitVector_Word_Insert(wordptr addr, N_int offset, N_int count,
                              boolean clear);
YASM_LIB_DECL
void    BitVector_Word_Delete(wordptr addr, N_int offset, N_int count,
                              boolean clear);

/* ===> arbitrary size chunk functions: */

YASM_LIB_DECL
void    BitVector_Chunk_Store(wordptr addr, N_int chunksize,
                              N_int offset, N_long value);
YASM_LIB_DECL
N_long  BitVector_Chunk_Read (wordptr addr, N_int chunksize,
                              N_int offset);

/* ===> set operations: */

YASM_LIB_DECL
void    Set_Union            (wordptr X, wordptr Y, wordptr Z); /* X = Y + Z */
YASM_LIB_DECL
void    Set_Intersection     (wordptr X, wordptr Y, wordptr Z); /* X = Y * Z */
YASM_LIB_DECL
void    Set_Difference       (wordptr X, wordptr Y, wordptr Z); /* X = Y \ Z */
YASM_LIB_DECL
void    Set_ExclusiveOr      (wordptr X, wordptr Y, wordptr Z); /*(Y+Z)\(Y*Z)*/
YASM_LIB_DECL
void    Set_Complement       (wordptr X, wordptr Y);            /* X = ~Y    */

/* ===> set functions: */

YASM_LIB_DECL
boolean Set_subset           (wordptr X, wordptr Y);            /* X in Y ?  */

YASM_LIB_DECL
N_int   Set_Norm             (wordptr addr);                    /* = | X |   */
YASM_LIB_DECL
N_int   Set_Norm2            (wordptr addr);                    /* = | X |   */
YASM_LIB_DECL
N_int   Set_Norm3            (wordptr addr);                    /* = | X |   */
YASM_LIB_DECL
Z_long  Set_Min              (wordptr addr);                    /* = min(X)  */
YASM_LIB_DECL
Z_long  Set_Max              (wordptr addr);                    /* = max(X)  */

/* ===> matrix-of-booleans operations: */

YASM_LIB_DECL
void    Matrix_Multiplication(wordptr X, N_int rowsX, N_int colsX,
                              wordptr Y, N_int rowsY, N_int colsY,
                              wordptr Z, N_int rowsZ, N_int colsZ);

YASM_LIB_DECL
void    Matrix_Product       (wordptr X, N_int rowsX, N_int colsX,
                              wordptr Y, N_int rowsY, N_int colsY,
                              wordptr Z, N_int rowsZ, N_int colsZ);

YASM_LIB_DECL
void    Matrix_Closure       (wordptr addr, N_int rows, N_int cols);

YASM_LIB_DECL
void    Matrix_Transpose     (wordptr X, N_int rowsX, N_int colsX,
                              wordptr Y, N_int rowsY, N_int colsY);

/*****************************************************************************/
/*  VERSION:  6.4                                                            */
/*****************************************************************************/
/*  VERSION HISTORY:                                                         */
/*****************************************************************************/
/*                                                                           */
/*    Version 6.4  03.10.04  Added C++ comp. directives. Improved "Norm()".  */
/*    Version 6.3  28.09.02  Added "Create_List()" and "GCD2()".             */
/*    Version 6.2  15.09.02  Overhauled error handling. Fixed "GCD()".       */
/*    Version 6.1  08.10.01  Make VMS linker happy: _lsb,_msb => _lsb_,_msb_ */
/*    Version 6.0  08.10.00  Corrected overflow handling.                    */
/*    Version 5.8  14.07.00  Added "Power()". Changed "Copy()".              */
/*    Version 5.7  19.05.99  Quickened "Div_Pos()". Added "Product()".       */
/*    Version 5.6  02.11.98  Leading zeros eliminated in "to_Hex()".         */
/*    Version 5.5  21.09.98  Fixed bug of uninitialized "error" in Multiply. */
/*    Version 5.4  07.09.98  Fixed bug of uninitialized "error" in Divide.   */
/*    Version 5.3  12.05.98  Improved Norm. Completed history.               */
/*    Version 5.2  31.03.98  Improved Norm.                                  */
/*    Version 5.1  09.03.98  No changes.                                     */
/*    Version 5.0  01.03.98  Major additions and rewrite.                    */
/*    Version 4.2  16.07.97  Added is_empty, is_full.                        */
/*    Version 4.1  30.06.97  Added word-ins/del, move-left/right, inc/dec.   */
/*    Version 4.0  23.04.97  Rewrite. Added bit shift and bool. matrix ops.  */
/*    Version 3.2  04.02.97  Added interval methods.                         */
/*    Version 3.1  21.01.97  Fixed bug on 64 bit machines.                   */
/*    Version 3.0  12.01.97  Added flip.                                     */
/*    Version 2.0  14.12.96  Efficiency and consistency improvements.        */
/*    Version 1.1  08.01.96  Added Resize and ExclusiveOr.                   */
/*    Version 1.0  14.12.95  First version under UNIX (with Perl module).    */
/*    Version 0.9  01.11.93  First version of C library under MS-DOS.        */
/*    Version 0.1  ??.??.89  First version in Turbo Pascal under CP/M.       */
/*                                                                           */
/*****************************************************************************/
/*  AUTHOR:                                                                  */
/*****************************************************************************/
/*                                                                           */
/*    Steffen Beyer                                                          */
/*    mailto:sb@engelschall.com                                              */
/*    http://www.engelschall.com/u/sb/download/                              */
/*                                                                           */
/*****************************************************************************/
/*  COPYRIGHT:                                                               */
/*****************************************************************************/
/*                                                                           */
/*    Copyright (c) 1995 - 2004 by Steffen Beyer.                            */
/*    All rights reserved.                                                   */
/*                                                                           */
/*****************************************************************************/
/*  LICENSE:                                                                 */
/*****************************************************************************/
/* This package is free software; you can use, modify and redistribute       */
/* it under the same terms as Perl itself, i.e., under the terms of          */
/* the "Artistic License" or the "GNU General Public License".               */
/*                                                                           */
/* The C library at the core of this Perl module can additionally            */
/* be used, modified and redistributed under the terms of the                */
/* "GNU Library General Public License".                                     */
/*                                                                           */
/*****************************************************************************/
/*  ARTISTIC LICENSE:                                                        */
/*****************************************************************************/
/*
                         The "Artistic License"

                                Preamble

The intent of this document is to state the conditions under which a
Package may be copied, such that the Copyright Holder maintains some
semblance of artistic control over the development of the package,
while giving the users of the package the right to use and distribute
the Package in a more-or-less customary fashion, plus the right to make
reasonable modifications.

Definitions:

        "Package" refers to the collection of files distributed by the
        Copyright Holder, and derivatives of that collection of files
        created through textual modification.

        "Standard Version" refers to such a Package if it has not been
        modified, or has been modified in accordance with the wishes
        of the Copyright Holder as specified below.

        "Copyright Holder" is whoever is named in the copyright or
        copyrights for the package.

        "You" is you, if you're thinking about copying or distributing
        this Package.

        "Reasonable copying fee" is whatever you can justify on the
        basis of media cost, duplication charges, time of people involved,
        and so on.  (You will not be required to justify it to the
        Copyright Holder, but only to the computing community at large
        as a market that must bear the fee.)

        "Freely Available" means that no fee is charged for the item
        itself, though there may be fees involved in handling the item.
        It also means that recipients of the item may redistribute it
        under the same conditions they received it.

1. You may make and give away verbatim copies of the source form of the
Standard Version of this Package without restriction, provided that you
duplicate all of the original copyright notices and associated disclaimers.

2. You may apply bug fixes, portability fixes and other modifications
derived from the Public Domain or from the Copyright Holder.  A Package
modified in such a way shall still be considered the Standard Version.

3. You may otherwise modify your copy of this Package in any way, provided
that you insert a prominent notice in each changed file stating how and
when you changed that file, and provided that you do at least ONE of the
following:

    a) place your modifications in the Public Domain or otherwise make them
    Freely Available, such as by posting said modifications to Usenet or
    an equivalent medium, or placing the modifications on a major archive
    site such as uunet.uu.net, or by allowing the Copyright Holder to include
    your modifications in the Standard Version of the Package.

    b) use the modified Package only within your corporation or organization.

    c) rename any non-standard executables so the names do not conflict
    with standard executables, which must also be provided, and provide
    a separate manual page for each non-standard executable that clearly
    documents how it differs from the Standard Version.

    d) make other distribution arrangements with the Copyright Holder.

4. You may distribute the programs of this Package in object code or
executable form, provided that you do at least ONE of the following:

    a) distribute a Standard Version of the executables and library files,
    together with instructions (in the manual page or equivalent) on where
    to get the Standard Version.

    b) accompany the distribution with the machine-readable source of
    the Package with your modifications.

    c) give non-standard executables non-standard names, and clearly
    document the differences in manual pages (or equivalent), together
    with instructions on where to get the Standard Version.

    d) make other distribution arrangements with the Copyright Holder.

5. You may charge a reasonable copying fee for any distribution of this
Package.  You may charge any fee you choose for support of this
Package.  You may not charge a fee for this Package itself.  However,
you may distribute this Package in aggregate with other (possibly
commercial) programs as part of a larger (possibly commercial) software
distribution provided that you do not advertise this Package as a
product of your own.  You may embed this Package's interpreter within
an executable of yours (by linking); this shall be construed as a mere
form of aggregation, provided that the complete Standard Version of the
interpreter is so embedded.

6. The scripts and library files supplied as input to or produced as
output from the programs of this Package do not automatically fall
under the copyright of this Package, but belong to whoever generated
them, and may be sold commercially, and may be aggregated with this
Package.  If such scripts or library files are aggregated with this
Package via the so-called "undump" or "unexec" methods of producing a
binary executable image, then distribution of such an image shall
neither be construed as a distribution of this Package nor shall it
fall under the restrictions of Paragraphs 3 and 4, provided that you do
not represent such an executable image as a Standard Version of this
Package.

7. C subroutines (or comparably compiled subroutines in other
languages) supplied by you and linked into this Package in order to
emulate subroutines and variables of the language defined by this
Package shall not be considered part of this Package, but are the
equivalent of input as in Paragraph 6, provided these subroutines do
not change the language in any way that would cause it to fail the
regression tests for the language.

8. Aggregation of this Package with a commercial distribution is always
permitted provided that the use of this Package is embedded; that is,
when no overt attempt is made to make this Package's interfaces visible
to the end user of the commercial distribution.  Such use shall not be
construed as a distribution of this Package.

9. The name of the Copyright Holder may not be used to endorse or promote
products derived from this software without specific prior written permission.

10. THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.

                                The End
*/
/*****************************************************************************/
/*  GNU GENERAL PUBLIC LICENSE:                                              */
/*****************************************************************************/
/* This program is free software; you can redistribute it and/or             */
/* modify it under the terms of the GNU General Public License               */
/* as published by the Free Software Foundation; either version 2            */
/* of the License, or (at your option) any later version.                    */
/*                                                                           */
/* This program is distributed in the hope that it will be useful,           */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/* GNU General Public License for more details.                              */
/*                                                                           */
/* You should have received a copy of the GNU General Public License         */
/* along with this program; if not, write to the                             */
/* Free Software Foundation, Inc.,                                           */
/* 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.                 */
/*                                                                           */
/*****************************************************************************/
/*  GNU LIBRARY GENERAL PUBLIC LICENSE:                                      */
/*****************************************************************************/
/*                                                                           */
/*    This library is free software; you can redistribute it and/or          */
/*    modify it under the terms of the GNU Library General Public            */
/*    License as published by the Free Software Foundation; either           */
/*    version 2 of the License, or (at your option) any later version.       */
/*                                                                           */
/*    This library is distributed in the hope that it will be useful,        */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU       */
/*    Library General Public License for more details.                       */
/*                                                                           */
/*    You should have received a copy of the GNU Library General Public      */
/*    License along with this library; if not, write to the                  */
/*    Free Software Foundation, Inc.,                                        */
/*    59 Temple Place, Suite 330, Boston, MA 02111-1307 USA                  */
/*                                                                           */
/*    or download a copy from ftp://ftp.gnu.org/pub/gnu/COPYING.LIB-2.0      */
/*                                                                           */
/*****************************************************************************/
#endif
