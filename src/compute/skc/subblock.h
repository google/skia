/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#ifndef SKC_ONCE_SUBBLOCK
#define SKC_ONCE_SUBBLOCK

//
//
//

#include "block.h"

//
//
//

#if 0

#define SKC_BLOCK_ID_BITS_BLOCK       (SKC_BLOCK_ID_BITS_ID - SKC_BLOCK_ID_BITS_SUBLOCK)
#define SKC_BLOCK_ID_BITS_SUBLOCK     (SKC_DEVICE_BLOCK_WORDS_LOG2 - SKC_DEVICE_SUBBLOCK_WORDS_LOG2)

#define SKC_BLOCK_ID_MASK_SUBBLOCK    SKC_BITS_TO_MASK_AT(SKC_BLOCK_ID_BITS_SUBLOCK,SKC_BLOCK_ID_BITS_TAG)

#define SKC_BLOCK_ID_GET_BLOCK(b)     ((b) >> (SKC_BLOCK_ID_BITS_SUBLOCK + SKC_BLOCK_ID_BITS_TAG))
#define SKC_BLOCK_ID_GET_SUBBLOCK(b)  (((b) & SKC_BLOCK_ID_MASK_SUBBLOCK) >> SKC_BLOCK_ID_BITS_TAG)

#endif

//
//
//

#endif

//
//
//
