/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#ifndef SKC_ONCE_DEVICE_CL_12_AVX2_H
#define SKC_ONCE_DEVICE_CL_12_AVX2_H

//
//
//

#define SKC_DEVICE_BLOCK_WORDS_LOG2         6
#define SKC_DEVICE_SUBBLOCK_WORDS_LOG2      4

//
//
//

#define SKC_DEVICE_BLOCK_WORDS              (1u << SKC_DEVICE_BLOCK_WORDS_LOG2)
#define SKC_DEVICE_SUBBLOCK_WORDS           (1u << SKC_DEVICE_SUBBLOCK_WORDS_LOG2)

//
//
//

#define SKC_DEVICE_SUBBLOCKS_PER_BLOCK      (SKC_DEVICE_BLOCK_WORDS / SKC_DEVICE_SUBBLOCK_WORDS)

//
//
//

#define SKC_COPY_PATHS_THREADS_PER_BLOCK    SKC_DEVICE_SUBBLOCK_WORDS
#define SKC_COPY_PATHS_ELEM_WORDS           1

//
//
//

#define SKC_EXPAND_FILLS_THREADS_PER_BLOCK  SKC_DEVICE_SUBBLOCK_WORDS
#define SKC_EXPAND_FILLS_ELEM_WORDS         1

//
//
//

#define SKC_RASTERIZE_THREADS_PER_BLOCK     SKC_DEVICE_SUBBLOCK_WORDS

//
//
//

#endif

//
//
//
