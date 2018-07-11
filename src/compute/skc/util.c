/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include "util.h"

//
//
//

#include <intrin.h>

//
//
//

skc_bool
skc_is_pow2_uint(skc_uint n)
{
  return (n & (n-1)) == 0;
}

//
// ASSUMES NON-ZERO
//

skc_uint
skc_msb_idx_uint(skc_uint n)
{

#ifdef _MSC_VER

  skc_uint index;

  _BitScanReverse(&index,n);

  return index;

#elif defined(__GNUC__)

#error "BUSTED msb_index()"
  return 31 - __builtin_clz(mask);

#else

#error "No msb_index()"

#endif

}

//
//
//

skc_uint
skc_pow2_rd_uint(skc_uint n)
{
  return 1u << skc_msb_idx_uint(n);
}

//
//
//

skc_uint
skc_pow2_ru_uint(skc_uint n)
{
  n--;
  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;
  n++;

  return n;
}

//
//
//
