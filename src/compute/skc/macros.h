/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#ifndef SKC_ONCE_MACROS
#define SKC_ONCE_MACROS

//
//
//

#include "types.h"

//
//
//

#define SKC_RESTRICT   __restrict

//
//
//

#define SKC_CONSTANT   __constant
#define SKC_GLOBAL     __global

//
//
//

#define SKC_CALLBACK   __stdcall

//
//
//

#define SKC_TRUE   1
#define SKC_FALSE  0

//
//
//

#define SKC_EMPTY
#define SKC_COMMA  ,

//
// INDEX, SUFFIX, COMPONENT, PUNCTUATION, RGBA
//

#define SKC_EXPAND_1()                          \
  SKC_EXPAND_X(0,SKC_EMPTY,SKC_EMPTY,SKC_EMPTY,SKC_EMPTY)

#define SKC_EXPAND_2()                          \
  SKC_EXPAND_X(0, 0,.s0,SKC_COMMA,.even)        \
  SKC_EXPAND_X(1, 1,.s1,SKC_EMPTY,.odd)

#define SKC_EXPAND_4()                          \
  SKC_EXPAND_X(0, 0,.s0,SKC_COMMA,.even.even)   \
  SKC_EXPAND_X(1, 1,.s1,SKC_COMMA,.odd.even)    \
  SKC_EXPAND_X(2, 2,.s2,SKC_COMMA,.even.odd)    \
  SKC_EXPAND_X(3, 3,.s3,SKC_EMPTY,.odd.odd)

#define SKC_EXPAND_8()                          \
  SKC_EXPAND_X(0, 0,.s0,SKC_COMMA,SKC_EMPTY)    \
  SKC_EXPAND_X(1, 1,.s1,SKC_COMMA,SKC_EMPTY)    \
  SKC_EXPAND_X(2, 2,.s2,SKC_COMMA,SKC_EMPTY)    \
  SKC_EXPAND_X(3, 3,.s3,SKC_COMMA,SKC_EMPTY)    \
  SKC_EXPAND_X(4, 4,.s4,SKC_COMMA,SKC_EMPTY)    \
  SKC_EXPAND_X(5, 5,.s5,SKC_COMMA,SKC_EMPTY)    \
  SKC_EXPAND_X(6, 6,.s6,SKC_COMMA,SKC_EMPTY)    \
  SKC_EXPAND_X(7, 7,.s7,SKC_EMPTY,SKC_EMPTY)

#define SKC_EXPAND_16()                         \
  SKC_EXPAND_X(0, 0,.s0,SKC_COMMA,SKC_EMPTY)    \
  SKC_EXPAND_X(1, 1,.s1,SKC_COMMA,SKC_EMPTY)    \
  SKC_EXPAND_X(2, 2,.s2,SKC_COMMA,SKC_EMPTY)    \
  SKC_EXPAND_X(3, 3,.s3,SKC_COMMA,SKC_EMPTY)    \
  SKC_EXPAND_X(4, 4,.s4,SKC_COMMA,SKC_EMPTY)    \
  SKC_EXPAND_X(5, 5,.s5,SKC_COMMA,SKC_EMPTY)    \
  SKC_EXPAND_X(6, 6,.s6,SKC_COMMA,SKC_EMPTY)    \
  SKC_EXPAND_X(7, 7,.s7,SKC_COMMA,SKC_EMPTY)    \
  SKC_EXPAND_X(8, 8,.s8,SKC_COMMA,SKC_EMPTY)    \
  SKC_EXPAND_X(9, 9,.s9,SKC_COMMA,SKC_EMPTY)    \
  SKC_EXPAND_X(10,A,.sA,SKC_COMMA,SKC_EMPTY)    \
  SKC_EXPAND_X(11,B,.sB,SKC_COMMA,SKC_EMPTY)    \
  SKC_EXPAND_X(12,C,.sC,SKC_COMMA,SKC_EMPTY)    \
  SKC_EXPAND_X(13,D,.sD,SKC_COMMA,SKC_EMPTY)    \
  SKC_EXPAND_X(14,E,.sE,SKC_COMMA,SKC_EMPTY)    \
  SKC_EXPAND_X(15,F,.sF,SKC_EMPTY,SKC_EMPTY)

//
//
//

#define SKC_BITS_TO_MASK(n)         (((skc_uint)1<<(n))-1)
#define SKC_BITS_TO_MASK_64(n)      (((skc_ulong)1<<(n))-1)

#define SKC_BITS_TO_MASK_AT(n,b)    (SKC_BITS_TO_MASK(n)<<(b))
#define SKC_BITS_TO_MASK_AT_64(n,b) (SKC_BITS_TO_MASK_64(n)<<(b))

//
// IF BFE IS SUPPORTED BY THE PLATFORM THEN IMPLEMENT DIFFERENTLY
//

#define SKC_BFE(x,n,i)        (((x) & SKC_BITS_TO_MASK_AT(n,i)) >> (i))

//
// IF BFI IS SUPPORTED BY THE PLATFORM THEN IMPLEMENT DIFFERENTLY
//
// Note this BFI assumes the destination bits were already set to zero
//

#define SKC_BFI(d,n,i,v)      ((((v) & SKC_BITS_TO_MASK(n)) << (i)) | (d))

//
//
//

#define SKC_STRINGIFY2(a)     #a
#define SKC_STRINGIFY(a)      SKC_STRINGIFY2(a)

//
//
//

#define SKC_EVAL(x)           x
#define SKC_CONCAT(a,b)       SKC_EVAL(a)##SKC_EVAL(b)

//
//
//

#define SKC_MAX_MACRO(a,b)    (((a) > (b)) ? (a) : (b))
#define SKC_MIN_MACRO(a,b)    (((a) < (b)) ? (a) : (b))
#define SKC_GTE_MACRO(a,b)    ((a) >= (b))
#define SKC_LT_MACRO(a,b)     ((a) <  (b))

//
//
//

#define SKC_COUNT_OF(x)       (sizeof(x) / sizeof(x)[0])
#define SKC_OFFSET_OF(t,m)    ((size_t)&(((t*)0)->m))
#define SKC_MEMBER_SIZE(t,m)  sizeof(((t*)0)->m)


//
// Returns rounded up next power-of-2 for non-zero -- via bit
// twiddling hacks, etc.
//

#define SKC_P2_OR_RS(n,x)     ((x)|(x)>>n)
#define SKC_POW2_RU_U32(x)    (1+SKC_P2_OR_RS(16,SKC_P2_OR_RS(8,SKC_P2_OR_RS(4,SKC_P2_OR_RS(2,SKC_P2_OR_RS(1,x-1))))))

//
// Round up
//

#define SKC_ROUND_DOWN(v,q)      (((v) / (q)) * (q))
#define SKC_ROUND_UP(v,q)        ((((v) + (q) - 1) / (q)) * (q))

#define SKC_ROUND_DOWN_POW2(v,q) ((v) & ~((q) - 1))
#define SKC_ROUND_UP_POW2(v,q)   SKC_ROUND_DOWN_POW2((v) + (q) - 1,q)

//
//
//

#if !defined(__OPENCL_C_VERSION__)

#define SKC_STATIC_ASSERT(p)  static_assert((p),#p)

#else

#define SKC_STATIC_ASSERT(p)

#endif

//
// Returns 1-based bit position of MSB
//

#define SKC_MSB_4(x,n)        (((x>>(n-0))&1)+((x>>(n-1))&1)+((x>>(n-2))&1)+((x>>(n-3))&1))
#define SKC_MSB_7654(x)       SKC_MSB_4(x,31)+SKC_MSB_4(x,27)+SKC_MSB_4(x,23)+SKC_MSB_4(x,19)
#define SKC_MSB_3210(x)       SKC_MSB_4(x,15)+SKC_MSB_4(x,11)+SKC_MSB_4(x, 7)+SKC_MSB_4(x, 3)
#define SKC_MSB_U32(x)        ( SKC_MSB_7654(((skc_uint)x)) + SKC_MSB_3210(((skc_uint)x)) )

//
// FIXME -- handle other compilers besides MSVC/x86
//

#define SKC_LZCNT_32(v)       __lzcnt(v)

//
//
//

#endif

