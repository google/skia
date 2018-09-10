/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#pragma once

//
//
//

#include <stdint.h>

//
//
//

#define ARRAY_LENGTH_MACRO(x)   (sizeof(x)/sizeof(x[0]))
#define OFFSET_OF_MACRO(t,m)    ((size_t)&(((t*)0)->m))
#define MEMBER_SIZE_MACRO(t,m)  sizeof(((t*)0)->m)


//
//
//

#define MAX_MACRO(a,b)          (((a) > (b)) ? (a) : (b))
#define MIN_MACRO(a,b)          (((a) < (b)) ? (a) : (b))
#define GTE_MACRO(a,b)          ((a) >= (b))
#define LT_MACRO(a,b)           ((a) <  (b))

//
//
//

#if defined( _MSC_VER )
#define ALLOCA_MACRO(n)         _alloca(n)
#else
#include <alloca.h>
#define ALLOCA_MACRO(n)         alloca(n)
#endif

//
//
//

#define BITS_TO_MASK(n)         (((uint32_t)1<<(n))-1)
#define BITS_TO_MASK_64(n)      (((uint64_t)1<<(n))-1)

#define BITS_TO_MASK_AT(n,b)    (BITS_TO_MASK(n)<<(b))
#define BITS_TO_MASK_AT_64(n,b) (BITS_TO_MASK_64(n)<<(b))

//
//
//

#define STRINGIFY_MACRO_2(a)    #a
#define STRINGIFY_MACRO(a)      STRINGIFY_MACRO_2(a)

//
//
//

#define CONCAT_MACRO_2(a,b)     a ## b
#define CONCAT_MACRO(a,b)       CONCAT_MACRO_2(a,b)

//
// Convert byte pointer to a network order 32-bit integer to host
// order.
//

#define NPBTOHL_MACRO(pb4)      ((((pb4)[0])<<24) | (((pb4)[1])<<16) |  \
                                 (((pb4)[2])<< 8) |   (pb4)[3])

//
//
//

#if   defined( _MSC_VER )

#if REG_DWORD == REG_DWORD_LITTLE_ENDIAN
#define NTOHL_MACRO(x)          _byteswap_ulong(x)
#else
#define NTOHL_MACRO(x)          x
#endif

#elif defined( __GNUC__ )

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define NTOHL_MACRO(x)          __builtin_bswap32(x)
#else
#define NTOHL_MACRO(x)          x
#endif

#else
//
// FIXME -- CLANG, etc.
//
#endif

//
//
//

#if   defined( _MSC_VER )

#define STATIC_ASSERT_MACRO(...) static_assert(__VA_ARGS__)

#elif defined( __GNUC__ )

#define STATIC_ASSERT_MACRO(...) _Static_assert(__VA_ARGS__)

#else
//
// FIXME -- CLANG, etc.
//
#endif

//
//
//

#if   defined( _MSC_VER )

#define POPCOUNT_MACRO(...) __popcnt(__VA_ARGS__)

#elif defined( __GNUC__ )

#define POPCOUNT_MACRO(...) __builtin_popcount(__VA_ARGS__)

#else
//
// FIXME -- CLANG, etc.
//
#endif

//
//
//
