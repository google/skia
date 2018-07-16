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

#if defined(_MSC_VER)
#define ALLOCA_MACRO(n)         _alloca(n)
#else
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
// Convert 4 byte pointer to network order dword to a host order.
//

#define NPBTOHL_MACRO(pb4)      ((((pb4)[0])<<24) | (((pb4)[1])<<16) |  \
                                 (((pb4)[2])<< 8) |   (pb4)[3])

#define NTOHL_MACRO(nl)         ntohl(nl)

//
//
//
