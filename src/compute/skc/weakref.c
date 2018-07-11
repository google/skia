/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#pragma once

//
//
//

#include "skc.h"
#include "weakref.h"
#include "macros.h"

//
// WEAKREF
//

#define SKC_WEAKREF_INDEX_BITS   16 // max bits for a weakref index -- this could/should be 12 bits
#define SKC_WEAKREF_INDEX_COUNT  (1u << SKC_WEAKREF_INDEX_BITS)

#define SKC_WEAKREF_INDEX_MASK   SKC_BITS_TO_MASK(SKC_WEAKREF_INDEX_BITS)
#define SKC_WEAKREF_EPOCH_MASK   SKC_BITS_TO_MASK_AT_64(64-SKC_WEAKREF_INDEX_BITS,SKC_WEAKREF_INDEX_BITS)

#define SKC_WEAKREF_EPOCH_INIT   0ul
#define SKC_WEAKREF_EPOCH_ONE    SKC_BITS_TO_MASK_AT_64(1,SKC_WEAKREF_INDEX_BITS)

//
// FIXME -- ASSUMES LITTLE-ENDIAN
//

union skc_weakref
{
  skc_weakref_t u64; // 64-bits containing refutation epoch and an index

  struct {
    skc_uint    index :      SKC_WEAKREF_INDEX_BITS;
    skc_uint    na_lo : 32 - SKC_WEAKREF_INDEX_BITS;
    skc_uint    na_hi;
  };

  struct {
    skc_ulong         :      SKC_WEAKREF_INDEX_BITS;
    skc_ulong   epoch : 64 - SKC_WEAKREF_INDEX_BITS;
  };
};

SKC_STATIC_ASSERT(sizeof(skc_weakref_t)     == sizeof(skc_ulong));
SKC_STATIC_ASSERT(sizeof(union skc_weakref) == sizeof(skc_ulong));

//
//
//

void
skc_weakref_epoch_init(skc_epoch_t * const epoch)
{
  *epoch = SKC_WEAKREF_EPOCH_INIT;
}

void
skc_weakref_epoch_inc(skc_epoch_t * const epoch)
{
  *epoch += SKC_WEAKREF_EPOCH_ONE;
}

void
skc_weakref_init(skc_weakref_t * const weakref,
                 skc_epoch_t   * const epoch,
                 skc_uint        const index)
{
  *weakref = *epoch | (index & SKC_WEAKREF_INDEX_MASK);
}

bool
skc_weakref_is_invalid(skc_weakref_t const * const weakref,
                       skc_epoch_t   const * const epoch)
{
  return ((*weakref ^ *epoch) & SKC_WEAKREF_EPOCH_MASK) != 0UL;
}

skc_uint
skc_weakref_index(skc_weakref_t const * const weakref)
{
  // truncate to word and mask
  return (skc_uint)*weakref & SKC_WEAKREF_INDEX_MASK;
}

//
//
//
