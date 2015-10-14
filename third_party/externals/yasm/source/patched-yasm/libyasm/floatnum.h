/**
 * \file libyasm/floatnum.h
 * \brief YASM floating point (IEEE) interface.
 *
 * \license
 *  Copyright (C) 2001-2007  Peter Johnson
 *
 *  Based on public-domain x86 assembly code by Randall Hyde (8/28/91).
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
#ifndef YASM_FLOATNUM_H
#define YASM_FLOATNUM_H

#ifndef YASM_LIB_DECL
#define YASM_LIB_DECL
#endif

/** Initialize floatnum internal data structures. */
YASM_LIB_DECL
void yasm_floatnum_initialize(void);

/** Clean up internal floatnum allocations. */
YASM_LIB_DECL
void yasm_floatnum_cleanup(void);

/** Create a new floatnum from a decimal string.  The input string must be in
 * standard C representation ([+-]123.456e[-+]789).
 * \param str   floating point decimal string
 * \return Newly allocated floatnum.
 */
YASM_LIB_DECL
/*@only@*/ yasm_floatnum *yasm_floatnum_create(const char *str);

/** Duplicate a floatnum.
 * \param flt   floatnum
 * \return Newly allocated floatnum with the same value as flt.
 */
YASM_LIB_DECL
/*@only@*/ yasm_floatnum *yasm_floatnum_copy(const yasm_floatnum *flt);

/** Destroy (free allocated memory for) a floatnum.
 * \param flt   floatnum
 */
YASM_LIB_DECL
void yasm_floatnum_destroy(/*@only@*/ yasm_floatnum *flt);

/** Floating point calculation function: acc = acc op operand.
 * \note Not all operations in yasm_expr_op may be supported; unsupported
 *       operations will result in an error.
 * \param acc       floatnum accumulator
 * \param op        operation
 * \param operand   floatnum operand
 * \return Nonzero on error.
 */
YASM_LIB_DECL
int yasm_floatnum_calc(yasm_floatnum *acc, yasm_expr_op op,
                       yasm_floatnum *operand);

/** Convert a floatnum to single-precision and return as 32-bit value.
 * The 32-bit value is a "standard" C value (eg, of unknown endian).
 * \param flt       floatnum
 * \param ret_val   pointer to storage for 32-bit output
 * \return Nonzero if flt can't fit into single precision: -1 if underflow
 *         occurred, 1 if overflow occurred.
 */
YASM_LIB_DECL
int yasm_floatnum_get_int(const yasm_floatnum *flt,
                          /*@out@*/ unsigned long *ret_val);

/** Output a #yasm_floatnum to buffer in little-endian or big-endian.  Puts the
 * value into the least significant bits of the destination, or may be shifted
 * into more significant bits by the shift parameter.  The destination bits are
 * cleared before being set.  [0] should be the first byte output to the file.
 * \note Not all sizes are valid.  Currently, only 32 (single-precision), 64
 *       (double-precision), and 80 (extended-precision) are valid sizes.
 *       Use yasm_floatnum_check_size() to check for supported sizes.
 * \param flt       floatnum
 * \param ptr       pointer to storage for size bytes of output
 * \param destsize  destination size (in bytes)
 * \param valsize   size (in bits)
 * \param shift     left shift (in bits)
 * \param bigendian endianness (nonzero=big, zero=little)
 * \param warn      enables standard overflow/underflow warnings
 * \return Nonzero if flt can't fit into the specified precision: -1 if
 *         underflow occurred, 1 if overflow occurred.
 */
YASM_LIB_DECL
int yasm_floatnum_get_sized(const yasm_floatnum *flt, unsigned char *ptr,
                            size_t destsize, size_t valsize, size_t shift,
                            int bigendian, int warn);

/** Basic check to see if size is valid for flt conversion (using
 * yasm_floatnum_get_sized()).  Doesn't actually check for underflow/overflow
 * but rather checks for size=32,64,80
 * (at present).
 * \param flt       floatnum
 * \param size      number of bits of output space
 * \return 1 if valid size, 0 if invalid size.
 */
YASM_LIB_DECL
int yasm_floatnum_check_size(const yasm_floatnum *flt, size_t size);

/** Print various representations of a floatnum.  For debugging purposes only.
 * \param f         file
 * \param flt       floatnum
 */
YASM_LIB_DECL
void yasm_floatnum_print(const yasm_floatnum *flt, FILE *f);

#endif
