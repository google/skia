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

#define ARRAY_LENGTH(x) (sizeof(x)/sizeof(x[0]))

//
//
//

#define MAX_MACRO(a,b)  (((a) > (b)) ? (a) : (b))
#define MIN_MACRO(a,b)  (((a) < (b)) ? (a) : (b))
#define GTE_MACRO(a,b)  ((a) >= (b))
#define LT_MACRO(a,b)   ((a) <  (b))

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

#if defined(_MSC_VER)
    #define ALLOCA(n)  _alloca(n)
#else
    #define ALLOCA(n) alloca(n)
#endif
//
//
//
