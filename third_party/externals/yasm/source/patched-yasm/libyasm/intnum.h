/**
 * \file libyasm/intnum.h
 * \brief YASM integer number interface.
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
#ifndef YASM_INTNUM_H
#define YASM_INTNUM_H

#ifndef YASM_LIB_DECL
#define YASM_LIB_DECL
#endif

/** Initialize intnum internal data structures. */
YASM_LIB_DECL
void yasm_intnum_initialize(void);

/** Clean up internal intnum allocations. */
YASM_LIB_DECL
void yasm_intnum_cleanup(void);

/** Create a new intnum from a decimal string.
 * \param str       decimal string
 * \return Newly allocated intnum.
 */
YASM_LIB_DECL
/*@only@*/ yasm_intnum *yasm_intnum_create_dec(char *str);

/** Create a new intnum from a binary string.
 * \param str       binary string
 * \return Newly allocated intnum.
 */
YASM_LIB_DECL
/*@only@*/ yasm_intnum *yasm_intnum_create_bin(char *str);

/** Create a new intnum from an octal string.
 * \param str       octal string
 * \return Newly allocated intnum.
 */
YASM_LIB_DECL
/*@only@*/ yasm_intnum *yasm_intnum_create_oct(char *str);

/** Create a new intnum from a hexidecimal string.
 * \param str       hexidecimal string
 * \return Newly allocated intnum.
 */
YASM_LIB_DECL
/*@only@*/ yasm_intnum *yasm_intnum_create_hex(char *str);

/** Convert character constant to integer value, using NASM rules.  NASM syntax
 * supports automatic conversion from strings such as 'abcd' to a 32-bit
 * integer value (little endian order).  This function performs those conversions.
 * \param str       character constant string
 * \return Newly allocated intnum.
 */
YASM_LIB_DECL
/*@only@*/ yasm_intnum *yasm_intnum_create_charconst_nasm(const char *str);

/** Convert character constant to integer value, using TASM rules.  TASM syntax
 * supports automatic conversion from strings such as 'abcd' to a 32-bit
 * integer value (big endian order).  This function performs those conversions.
 * \param str       character constant string
 * \return Newly allocated intnum.
 */
YASM_LIB_DECL
/*@only@*/ yasm_intnum *yasm_intnum_create_charconst_tasm(const char *str);

/** Create a new intnum from an unsigned integer value.
 * \param i         unsigned integer value
 * \return Newly allocated intnum.
 */
YASM_LIB_DECL
/*@only@*/ yasm_intnum *yasm_intnum_create_uint(unsigned long i);

/** Create a new intnum from an signed integer value.
 * \param i         signed integer value
 * \return Newly allocated intnum.
 */
YASM_LIB_DECL
/*@only@*/ yasm_intnum *yasm_intnum_create_int(long i);

/** Create a new intnum from LEB128-encoded form.
 * \param ptr   pointer to start of LEB128 encoded form
 * \param sign  signed (1) or unsigned (0) LEB128 format
 * \param size  number of bytes read from ptr (output)
 * \return Newly allocated intnum.  Number of bytes read returned into
 *         bytes_read parameter.
 */
YASM_LIB_DECL
/*@only@*/ yasm_intnum *yasm_intnum_create_leb128
    (const unsigned char *ptr, int sign, /*@out@*/ unsigned long *size);

/** Create a new intnum from a little-endian or big-endian buffer.
 * In little endian, the LSB is in ptr[0].
 * \param ptr       pointer to start of buffer
 * \param sign      signed (1) or unsigned (0) source
 * \param srcsize   source buffer size (in bytes)
 * \param bigendian endianness (nonzero=big, zero=little)
 */
YASM_LIB_DECL
/*@only@*/ yasm_intnum *yasm_intnum_create_sized
    (unsigned char *ptr, int sign, size_t srcsize, int bigendian);

/** Duplicate an intnum.
 * \param intn  intnum
 * \return Newly allocated intnum with the same value as intn.
 */
YASM_LIB_DECL
/*@only@*/ yasm_intnum *yasm_intnum_copy(const yasm_intnum *intn);

/** Destroy (free allocated memory for) an intnum.
 * \param intn  intnum
 */
YASM_LIB_DECL
void yasm_intnum_destroy(/*@only@*/ yasm_intnum *intn);

/** Floating point calculation function: acc = acc op operand.
 * \note Not all operations in yasm_expr_op may be supported; unsupported
 *       operations will result in an error.
 * \param acc       intnum accumulator
 * \param op        operation
 * \param operand   intnum operand
 * \return Nonzero if error occurred.
 */
YASM_LIB_DECL
int yasm_intnum_calc(yasm_intnum *acc, yasm_expr_op op, yasm_intnum *operand);

/** Compare two intnums.
 * \param intn1     first intnum
 * \param intn2     second intnum
 * \return -1 if intn1 < intn2, 0 if intn1 == intn2, 1 if intn1 > intn2.
 */
YASM_LIB_DECL
int yasm_intnum_compare(const yasm_intnum *intn1, const yasm_intnum *intn2);

/** Zero an intnum.
 * \param intn      intnum
 */
YASM_LIB_DECL
void yasm_intnum_zero(yasm_intnum *intn);

/** Set an intnum to the value of another intnum.
 * \param intn      intnum
 * \param val       intnum to get value from
 */
YASM_LIB_DECL
void yasm_intnum_set(yasm_intnum *intn, const yasm_intnum *val);

/** Set an intnum to an unsigned integer.
 * \param intn      intnum
 * \param val       integer value
 */
YASM_LIB_DECL
void yasm_intnum_set_uint(yasm_intnum *intn, unsigned long val);

/** Set an intnum to an signed integer.
 * \param intn      intnum
 * \param val       integer value
 */
YASM_LIB_DECL
void yasm_intnum_set_int(yasm_intnum *intn, long val);

/** Simple value check for 0.
 * \param acc       intnum
 * \return Nonzero if acc==0.
 */
YASM_LIB_DECL
int yasm_intnum_is_zero(const yasm_intnum *acc);

/** Simple value check for 1.
 * \param acc       intnum
 * \return Nonzero if acc==1.
 */
YASM_LIB_DECL
int yasm_intnum_is_pos1(const yasm_intnum *acc);

/** Simple value check for -1.
 * \param acc       intnum
 * \return Nonzero if acc==-1.
 */
YASM_LIB_DECL
int yasm_intnum_is_neg1(const yasm_intnum *acc);

/** Simple sign check.
 * \param acc       intnum
 * \return -1 if negative, 0 if zero, +1 if positive
 */
YASM_LIB_DECL
int yasm_intnum_sign(const yasm_intnum *acc);

/** Convert an intnum to an unsigned 32-bit value.  The value is in "standard"
 * C format (eg, of unknown endian).
 * \note Parameter intnum is truncated to fit into 32 bits.  Use
 *       intnum_check_size() to check for overflow.
 * \param intn  intnum
 * \return Unsigned 32-bit value of intn.
 */
YASM_LIB_DECL
unsigned long yasm_intnum_get_uint(const yasm_intnum *intn);

/** Convert an intnum to a signed 32-bit value.  The value is in "standard" C
 * format (eg, of unknown endian).
 * \note Parameter intnum is truncated to fit into 32 bits.  Use
 *       intnum_check_size() to check for overflow.
 * \param intn  intnum
 * \return Signed 32-bit value of intn.
 */
YASM_LIB_DECL
long yasm_intnum_get_int(const yasm_intnum *intn);

/** Output #yasm_intnum to buffer in little-endian or big-endian.  Puts the
 * value into the least significant bits of the destination, or may be shifted
 * into more significant bits by the shift parameter.  The destination bits are
 * cleared before being set.  [0] should be the first byte output to the file.
 * \param intn      intnum
 * \param ptr       pointer to storage for size bytes of output
 * \param destsize  destination size (in bytes)
 * \param valsize   size (in bits)
 * \param shift     left shift (in bits); may be negative to specify right
 *                  shift (standard warnings include truncation to boundary)
 * \param bigendian endianness (nonzero=big, zero=little)
 * \param warn      enables standard warnings (value doesn't fit into valsize
 *                  bits): <0=signed warnings, >0=unsigned warnings, 0=no warn
 */
YASM_LIB_DECL
void yasm_intnum_get_sized(const yasm_intnum *intn, unsigned char *ptr,
                           size_t destsize, size_t valsize, int shift,
                           int bigendian, int warn);

/** Check to see if intnum will fit without overflow into size bits.
 * \param intn      intnum
 * \param size      number of bits of output space
 * \param rshift    right shift
 * \param rangetype signed/unsigned range selection:
 *                  0 => (0, unsigned max);
 *                  1 => (signed min, signed max);
 *                  2 => (signed min, unsigned max)
 * \return Nonzero if intnum will fit.
 */
YASM_LIB_DECL
int yasm_intnum_check_size(const yasm_intnum *intn, size_t size,
                           size_t rshift, int rangetype);

/** Check to see if intnum will fit into a particular numeric range.
 * \param intn      intnum
 * \param low       low end of range (inclusive)
 * \param high      high end of range (inclusive)
 * \return Nonzero if intnum is within range.
 */
YASM_LIB_DECL
int yasm_intnum_in_range(const yasm_intnum *intn, long low, long high);

/** Output #yasm_intnum to buffer in LEB128-encoded form.
 * \param intn      intnum
 * \param ptr       pointer to storage for output bytes
 * \param sign      signedness of LEB128 encoding (0=unsigned, 1=signed)
 * \return Number of bytes generated.
 */
YASM_LIB_DECL
unsigned long yasm_intnum_get_leb128(const yasm_intnum *intn,
                                     unsigned char *ptr, int sign);

/** Calculate number of bytes LEB128-encoded form of #yasm_intnum will take.
 * \param intn      intnum
 * \param sign      signedness of LEB128 encoding (0=unsigned, 1=signed)
 * \return Number of bytes.
 */
YASM_LIB_DECL
unsigned long yasm_intnum_size_leb128(const yasm_intnum *intn, int sign);

/** Output integer to buffer in signed LEB128-encoded form.
 * \param v         integer
 * \param ptr       pointer to storage for output bytes
 * \return Number of bytes generated.
 */
YASM_LIB_DECL
unsigned long yasm_get_sleb128(long v, unsigned char *ptr);

/** Calculate number of bytes signed LEB128-encoded form of integer will take.
 * \param v         integer
 * \return Number of bytes.
 */
YASM_LIB_DECL
unsigned long yasm_size_sleb128(long v);

/** Output integer to buffer in unsigned LEB128-encoded form.
 * \param v         integer
 * \param ptr       pointer to storage for output bytes
 * \return Number of bytes generated.
 */
YASM_LIB_DECL
unsigned long yasm_get_uleb128(unsigned long v, unsigned char *ptr);

/** Calculate number of bytes unsigned LEB128-encoded form of integer will take.
 * \param v         integer
 * \return Number of bytes.
 */
YASM_LIB_DECL
unsigned long yasm_size_uleb128(unsigned long v);

/** Get an intnum as a signed decimal string.  The returned string will
 * contain a leading '-' if the intnum is negative.
 * \param intn  intnum
 * \return Newly allocated string containing the decimal representation of
 *         the intnum.
 */
YASM_LIB_DECL
/*@only@*/ char *yasm_intnum_get_str(const yasm_intnum *intn);

/** Print an intnum.  For debugging purposes.
 * \param f     file
 * \param intn  intnum
 */
YASM_LIB_DECL
void yasm_intnum_print(const yasm_intnum *intn, FILE *f);

#endif
