/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#ifndef SKC_ONCE_TILE
#define SKC_ONCE_TILE

//
//
//

#include "macros.h"

//
// Hard requirements:
//
//   - A TTXB "block pool" extent that is at least 1GB.
//
//   - A virtual surface of at least 8K x 8K
//
//   - A physical surface of __don't really care__ because it's
//     advantageous to tile the physical surface since it's likely
//     to shrink the post-place TTCK sorting step.
//
//
//      EXTENT                 TTXB BITS
//     SIZE (MB) +------------------------------------+
//               |  22    23    24    25    26    27  |
//          +----+------------------------------------+
//          |  8 |  128   256   512  1024  2048  4096 |
//     TTXB | 16 |  256   512  1024  2048  4096  8192 |
//    WORDS | 32 |  512  1024  2048  4096  8192 16384 |
//          | 64 | 1024  2048  4096  8192 16384 32768 |
//          +----+------------------------------------+
//
//
//         SURF                        X/Y BITS
//         TILE  +------------------------------------------------------+
//               |   5     6     7     8     9    10    11    12    13  |
//          +----+------------------------------------------------------+
//          |  3 |  256   512  1024  2048  4096  8192 16384 32768 65536 |
//     TILE |  4 |  512  1024  2048  4096  8192 16384 32768 65536  128K |
//     SIDE |  5 | 1024  2048  4096  8192 16384 32768 65536  128K  256K |
//     BITS |  6 | 2048  4096  8192 16384 32768 65536  128K  256K  512K |
//          |  7 | 4096  8192 16384 32768 65536  128K  256K  512K 1024K |
//          +----+------------------------------------------------------+
//      TILES^2  | 1024  4096 16384 65536  256K    1M    4M   16M   64M |
//               +------------------------------------------------------+
//
// The following values should be pretty future-proof across all GPUs:
//
//   - The minimum addressable subblock size is 16 words (64 bytes) to
//     ensure there is enough space for a path or raster header and
//     its payload.
//
//   - Blocks are power-of-2 multiples of subblocks. Larger blocks can
//     reduce allocation activity (fewer atomic adds).
//
//   - 27 bits of TTXB_ID space implies a max of 4GB-32GB of
//     rasterized paths depending on the size of the TTXB block.
//     This could enable interesting use cases.
//
//   - A virtual rasterization surface that's from +/-16K to +/-128K
//     depending on the size of the TTXB block.
//
//   - Keys that (optionally) only require a 32-bit high word
//     comparison.
//
//   - Support for a minimum of 256K layers. This can be practically
//     raised to 1m or 2m layers.
//

//
// TTRK (64-bit COMPARE)
//
//  0                                         63
//  | TTSB ID |   X  |   Y  | RASTER COHORT ID |
//  +---------+------+------+------------------+
//  |    27   |  12  |  12  |        13        |
//
//
// TTRK (32-BIT COMPARE)
//
//  0                                               63
//  | TTSB ID | N/A |   X  |   Y  | RASTER COHORT ID |
//  +---------+-----+------+------+------------------+
//  |    27   |  5  |  12  |  12  |        8         |
//
//
// TTSK v2:
//
//  0                                     63
//  | TTSB ID | IS_PREFIX |  N/A |  X |  Y |
//  +---------+-----------+------+----+----+
//  |    27   |   1 (=0)  |  12  | 12 | 12 |
//
//
// TTPK v2:
//
//  0                                       63
//  | TTPB ID | IS_PREFIX | SPAN |  X  |  Y  |
//  +---------+-----------+------+-----+-----+
//  |    27   |   1 (=1)  |  12  | 12  | 12  |
//
//
// TTCK (32-BIT COMPARE) v1:
//
//  0                                                           63
//  | PAYLOAD/TTSB/TTPB ID | PREFIX | ESCAPE | LAYER |  X  |  Y  |
//  +----------------------+--------+--------+-------+-----+-----+
//  |          30          |    1   |    1   |   18  |  7  |  7  |
//
//
// TTCK (32-BIT COMPARE) v2:
//
//  0                                                           63
//  | PAYLOAD/TTSB/TTPB ID | PREFIX | ESCAPE | LAYER |  X  |  Y  |
//  +----------------------+--------+--------+-------+-----+-----+
//  |          30          |    1   |    1   |   15  |  9  |  8  |
//
//
// TTCK (64-BIT COMPARE) -- achieves 4K x 4K with an 8x16 tile:
//
//  0                                                           63
//  | PAYLOAD/TTSB/TTPB ID | PREFIX | ESCAPE | LAYER |  X  |  Y  |
//  +----------------------+--------+--------+-------+-----+-----+
//  |          27          |    1   |    1   |   18  |  9  |  8  |
//

//
//
//

#define SKC_SUBPIXEL_RESL_X_LOG2  5
#define SKC_SUBPIXEL_RESL_Y_LOG2  5

//
// FIXME -- COMMON -- HOIST ELSEWHERE
//

#define SKC_TILE_WIDTH            (1 << SKC_TILE_WIDTH_LOG2)
#define SKC_TILE_HEIGHT           (1 << SKC_TILE_HEIGHT_LOG2)

#define SKC_SUBPIXEL_RESL_X       (1 << SKC_SUBPIXEL_RESL_X_LOG2)
#define SKC_SUBPIXEL_RESL_Y       (1 << SKC_SUBPIXEL_RESL_Y_LOG2)

//
// PLATFORM SURFACE TILE SIZE
//

#define SKC_TILE_WIDTH_MASK       SKC_BITS_TO_MASK(SKC_TILE_WIDTH_LOG2)
#define SKC_TILE_HEIGHT_MASK      SKC_BITS_TO_MASK(SKC_TILE_HEIGHT_LOG2)

//
// TILE SUBPIXEL RESOLUTION
//

#define SKC_SUBPIXEL_RESL_X       (1 << SKC_SUBPIXEL_RESL_X_LOG2)
#define SKC_SUBPIXEL_RESL_Y       (1 << SKC_SUBPIXEL_RESL_Y_LOG2)

#define SKC_SUBPIXEL_MASK_X       SKC_BITS_TO_MASK(SKC_SUBPIXEL_RESL_X_LOG2)
#define SKC_SUBPIXEL_MASK_Y       SKC_BITS_TO_MASK(SKC_SUBPIXEL_RESL_Y_LOG2)

#define SKC_SUBPIXEL_RESL_X_F32   ((float)(SKC_SUBPIXEL_RESL_X))
#define SKC_SUBPIXEL_RESL_Y_F32   ((float)(SKC_SUBPIXEL_RESL_Y))

#define SKC_SUBPIXEL_X_SCALE_UP   SKC_SUBPIXEL_RESL_X_F32
#define SKC_SUBPIXEL_Y_SCALE_UP   SKC_SUBPIXEL_RESL_Y_F32

#define SKC_SUBPIXEL_X_SCALE_DOWN (1.0f / SKC_SUBPIXEL_RESL_X_F32)
#define SKC_SUBPIXEL_Y_SCALE_DOWN (1.0f / SKC_SUBPIXEL_RESL_Y_F32)

//
// SUBTILE RESOLUTION
//

#define SKC_SUBTILE_RESL_X_LOG2   (SKC_TILE_WIDTH_LOG2  + SKC_SUBPIXEL_RESL_X_LOG2)
#define SKC_SUBTILE_RESL_Y_LOG2   (SKC_TILE_HEIGHT_LOG2 + SKC_SUBPIXEL_RESL_Y_LOG2)

#define SKC_SUBTILE_RESL_X        (1 << SKC_SUBTILE_RESL_X_LOG2)
#define SKC_SUBTILE_RESL_Y        (1 << SKC_SUBTILE_RESL_Y_LOG2)

#define SKC_SUBTILE_MASK_X        SKC_BITS_TO_MASK(SKC_SUBTILE_RESL_X_LOG2)
#define SKC_SUBTILE_MASK_Y        SKC_BITS_TO_MASK(SKC_SUBTILE_RESL_Y_LOG2)

#define SKC_SUBTILE_RESL_X_F32    ((float)(SKC_SUBTILE_RESL_X))
#define SKC_SUBTILE_RESL_Y_F32    ((float)(SKC_SUBTILE_RESL_Y))

#define SKC_SUBTILE_X_SCALE_DOWN  (1.0f / SKC_SUBTILE_RESL_X_F32)
#define SKC_SUBTILE_Y_SCALE_DOWN  (1.0f / SKC_SUBTILE_RESL_Y_F32)

//
//
//

#define SKC_TILE_X_OFFSET_U32     (1 << (SKC_TTSK_BITS_X-1))
#define SKC_TILE_X_SPAN_U32       (1 << (SKC_TTSK_BITS_X))   // exclusive

#define SKC_TILE_Y_OFFSET_U32     (1 << (SKC_TTSK_BITS_Y-1))
#define SKC_TILE_Y_SPAN_U32       (1 << (SKC_TTSK_BITS_Y))   // exclusive

#define SKC_TILE_X_OFFSET_F32     0 // ((float)SKC_TILE_X_OFFSET_U32)
#define SKC_TILE_X_SPAN_F32       ((float)SKC_TILE_X_SPAN_U32)

#define SKC_TILE_Y_OFFSET_F32     0 // ((float)SKC_TILE_Y_OFFSET_U32)
#define SKC_TILE_Y_SPAN_F32       ((float)SKC_TILE_Y_SPAN_U32)

//
// TILE TRACE SUBPIXEL, PREFIX & COMPOSITION KEYS
//
// These keys are are purposefully 64-bits so they can be sorted with
// Hotsort's 32:32 or 64-bit implementation.
//
// Tiles are 32x32 on CUDA but can be made rectangular or smaller to
// fit other architectures.
//
//   TW   : tile width
//   TH   : tile height
//
//   TTS  : tile trace subpixel
//   TTSB : tile trace subpixel block
//   TTRK : tile trace subpixel key while in raster cohort
//   TTSK : tile trace subpixel key
//
//   TTP  : tile trace prefix
//   TTPB : tile trace prefix block
//   TTPK : tile trace prefix key
//
//   TTCK : tile trace composition key
//

//
// TILE TRACE SUBPIXEL
//
// The subpixels are encoded with either absolute tile coordinates
// (32-bits) or packed in delta-encoded form form.
//
// For 32-bit subpixel packing of a 32x32 or smaller tile:
//
// A tile X is encoded as:
//
//   TX : 10 : unsigned min(x0,x1) tile subpixel coordinate.
//
//   SX :  6 : unsigned subpixel span from min to max x with range
//             [0,32]. The original direction is not captured. Would
//             be nice to capture dx but not necessary right now but
//             could be in the future. <--- SPARE VALUES AVAILABLE
//
// A tile Y is encoded as:
//
//   TY : 10 : unsigned min(y0,y1) tile subpixel coordinate.
//
//   DY :  6 : signed subpixel delta y1-y0. The range of delta is
//             [-32,32] but horizontal lines are not encoded so [1,32]
//             is mapped to [0,31]. The resulting range [-32,31] fits
//             in 6 bits.
//
// TTS:
//
//  0                        31
//  |  TX |  SX  |  TY |  DY  |
//  +-----+------+-----+------+
//  |  10 |   6  |  10 |   6  |
//

#define SKC_TTS_BITS_TX           10
#define SKC_TTS_BITS_SX           6
#define SKC_TTS_BITS_TY           10
#define SKC_TTS_BITS_SY           6

//
//
//

#define SKC_TTS_INVALID           ( SKC_UINT_MAX ) // relies on limited range of dx

//
//
//

#define SKC_TTS_OFFSET_SX         (SKC_TTS_BITS_TX)
#define SKC_TTS_OFFSET_TY         (SKC_TTS_BITS_TX + SKC_TTS_BITS_SX)
#define SKC_TTS_OFFSET_DY         (SKC_TTS_BITS_TX + SKC_TTS_BITS_SX + SKC_TTS_BITS_TY)

#define SKC_TTS_MASK_TX           SKC_BITS_TO_MASK(SKC_TTS_BITS_TX)
#define SKC_TTS_MASK_SX           SKC_BITS_TO_MASK_AT(SKC_TTS_BITS_SX,SKC_TTS_OFFSET_SX)
#define SKC_TTS_MASK_TY           SKC_BITS_TO_MASK_AT(SKC_TTS_BITS_TY,SKC_TTS_OFFSET_TY)

#define SKC_TTS_MASK_TX_PIXEL     SKC_BITS_TO_MASK_AT(SKC_TTS_BITS_TX-SKC_SUBPIXEL_RESL_X_LOG2, \
                                                      SKC_SUBPIXEL_RESL_X_LOG2)
#define SKC_TTS_MASK_TY_PIXEL     SKC_BITS_TO_MASK_AT(SKC_TTS_BITS_TY-SKC_SUBPIXEL_RESL_Y_LOG2, \
                                                      SKC_TTS_OFFSET_TY+SKC_SUBPIXEL_RESL_Y_LOG2)

//
// TTRK (64-BIT COMPARE)
//
//    0                                  63
//    | TTSB ID |   X  |   Y  | COHORT ID |
//    +---------+------+------+-----------+
//    |    27   |  12  |  12  |     13    |
//
//
// TTRK (32-BIT COMPARE)
//
//    0                                        63
//    | TTSB ID | N/A |   X  |   Y  | COHORT ID |
//    +---------+-----+------+------+-----------+
//    |    27   |  5  |  12  |  12  |     8     |
//

//
// TTRK is sortable intermediate key format for TTSK
//
// We're going to use the 32-bit comparison version for now
//

//
// TTSK v2:
//
//    0                                  63
//    | TTSB ID | PREFIX |  N/A |  X |  Y |
//    +---------+--------+------+----+----+
//    |    27   | 1 (=0) |  12  | 12 | 12 |
//
//
// TTPK v2:
//
//    0                                    63
//    | TTPB ID | PREFIX | SPAN |  X  |  Y  |
//    +---------+--------+------+-----+-----+
//    |    27   | 1 (=1) |  12  | 12  | 12  |
//

#define SKC_TTXK_LO_BITS_ID          27
#define SKC_TTXK_LO_BITS_PREFIX      1
#define SKC_TTXK_HI_BITS_Y           12
#define SKC_TTXK_HI_BITS_X           12
#define SKC_TTXK_BITS_SPAN           12
#define SKC_TTXK_HI_BITS_YX          (SKC_TTXK_HI_BITS_Y + SKC_TTXK_HI_BITS_X)

#define SKC_TTRK_HI_MASK_X           SKC_BITS_TO_MASK(SKC_TTXK_HI_BITS_X)
#define SKC_TTRK_HI_MASK_YX          SKC_BITS_TO_MASK(SKC_TTXK_HI_BITS_YX)

#define SKC_TTRK_HI_BITS_COHORT      8
#define SKC_TTRK_LO_BITS_NA          (32 - SKC_TTXK_LO_BITS_ID)
#define SKC_TTRK_HI_BITS_COHORT_Y    (SKC_TTRK_HI_BITS_COHORT + SKC_TTXK_HI_BITS_Y)

#define SKC_TTRK_HI_OFFSET_COHORT    (32 - SKC_TTRK_HI_BITS_COHORT)
#define SKC_TTRK_HI_MASK_COHORT      SKC_BITS_TO_MASK_AT(SKC_TTRK_HI_BITS_COHORT,SKC_TTRK_HI_OFFSET_COHORT)

#define SKC_TTRK_HI_BITS_COHORT_YX   (SKC_TTRK_HI_BITS_COHORT + SKC_TTXK_HI_BITS_Y + SKC_TTXK_HI_BITS_X)

#define SKC_TTXK_LO_BITS_ID_PREFIX   (SKC_TTXK_LO_BITS_ID + SKC_TTXK_LO_BITS_PREFIX)

#define SKC_TTXK_LO_OFFSET_PREFIX    SKC_TTXK_LO_BITS_ID
#define SKC_TTXK_LO_OFFSET_SPAN      SKC_TTXK_LO_BITS_ID_PREFIX

#define SKC_TTXK_LO_BITS_SPAN        (32 - SKC_TTXK_LO_BITS_ID_PREFIX)
#define SKC_TTXK_HI_BITS_SPAN        (SKC_TTXK_BITS_SPAN - SKC_TTXK_LO_BITS_SPAN)

#define SKC_TTXK_LO_OFFSET_PREFIX    SKC_TTXK_LO_BITS_ID

#define SKC_TTXK_LO_MASK_ID          SKC_BITS_TO_MASK(SKC_TTXK_LO_BITS_ID)
#define SKC_TTXK_LO_MASK_PREFIX      SKC_BITS_TO_MASK_AT(SKC_TTXK_LO_BITS_PREFIX,SKC_TTXK_LO_OFFSET_PREFIX)
#define SKC_TTXK_LO_MASK_ID_PREFIX   SKC_BITS_TO_MASK(SKC_TTXK_LO_BITS_ID_PREFIX)

#define SKC_TTXK_HI_OFFSET_Y         (32 - SKC_TTXK_HI_BITS_Y)
#define SKC_TTXK_HI_OFFSET_X         (SKC_TTXK_HI_OFFSET_Y - SKC_TTXK_HI_BITS_X)

#define SKC_TTXK_HI_ONE_X            (1u << SKC_TTXK_HI_OFFSET_X)

#define SKC_TTXK_HI_MASK_YX          SKC_BITS_TO_MASK_AT(SKC_TTXK_HI_BITS_YX,SKC_TTXK_HI_OFFSET_X)
#define SKC_TTXK_HI_MASK_Y           SKC_BITS_TO_MASK_AT(SKC_TTXK_HI_BITS_Y ,SKC_TTXK_HI_OFFSET_Y)

#define SKC_TTPK_LO_SHL_YX_SPAN      (SKC_TTXK_LO_OFFSET_SPAN - SKC_TTXK_HI_OFFSET_X)
#define SKC_TTPK_HI_SHR_YX_SPAN      (SKC_TTXK_HI_OFFSET_X + SKC_TTXK_LO_BITS_SPAN)

//
// TTCK (32-BIT COMPARE) v1 -- NOT USED:
//
//  0                                                           63
//  | PAYLOAD/TTSB/TTPB ID | PREFIX | ESCAPE | LAYER |  X  |  Y  |
//  +----------------------+--------+--------+-------+-----+-----+
//  |          30          |    1   |    1   |   18  |  7  |  7  |
//
//
// TTCK (32-BIT COMPARE) v2 -- NOT USED:
//
//  0                                                           63
//  | PAYLOAD/TTSB/TTPB ID | PREFIX | ESCAPE | LAYER |  X  |  Y  |
//  +----------------------+--------+--------+-------+-----+-----+
//  |          30          |    1   |    1   |   15  |  9  |  8  |
//
//
// TTCK (64-BIT COMPARE) -- achieves 4K x 4K with an 8x16 tile:
//
//  0                                                           63
//  | PAYLOAD/TTSB/TTPB ID | PREFIX | ESCAPE | LAYER |  X  |  Y  |
//  +----------------------+--------+--------+-------+-----+-----+
//  |          27          |    1   |    1   |   18  |  9  |  8  |
//

#define SKC_TTCK_BITS_LAYER               18

#define SKC_TTCK_LO_BITS_ID               SKC_TTXK_LO_BITS_ID
#define SKC_TTCK_LO_OFFSET_ID             0

#define SKC_TTCK_LO_MASK_ID               SKC_BITS_TO_MASK(SKC_TTCK_LO_BITS_ID)

#define SKC_TTCK_LO_BITS_PREFIX           1
#define SKC_TTCK_LO_OFFSET_PREFIX         SKC_TTCK_LO_BITS_ID
#define SKC_TTCK_LO_MASK_PREFIX           SKC_BITS_TO_MASK_AT(SKC_TTCK_LO_BITS_PREFIX,SKC_TTCK_LO_OFFSET_PREFIX)

#define SKC_TTCK_LO_BITS_ID_PREFIX        (SKC_TTCK_LO_BITS_ID + SKC_TTCK_LO_BITS_PREFIX)
#define SKC_TTCK_LO_MASK_ID_PREFIX        SKC_BITS_TO_MASK(SKC_TTCK_LO_BITS_ID_PREFIX)

#define SKC_TTCK_LO_BITS_ESCAPE           1
#define SKC_TTCK_LO_OFFSET_ESCAPE         SKC_TTCK_LO_BITS_ID_PREFIX
#define SKC_TTCK_LO_MASK_ESCAPE           SKC_BITS_TO_MASK_AT(SKC_TTCK_LO_BITS_ESCAPE,SKC_TTCK_LO_OFFSET_ESCAPE)

#define SKC_TTCK_LO_BITS_ID_PREFIX_ESCAPE (SKC_TTCK_LO_BITS_ID_PREFIX + SKC_TTCK_LO_BITS_ESCAPE)

#define SKC_TTCK_HI_OFFSET_Y              24
#define SKC_TTCK_HI_OFFSET_X              15

#define SKC_TTCK_HI_BITS_Y                8
#define SKC_TTCK_HI_BITS_X                9
#define SKC_TTCK_HI_BITS_YX               (SKC_TTCK_HI_BITS_X + SKC_TTCK_HI_BITS_Y)
#define SKC_TTCK_HI_MASK_YX               SKC_BITS_TO_MASK_AT(SKC_TTCK_HI_BITS_YX,SKC_TTCK_HI_OFFSET_X)

#define SKC_TTCK_HI_BITS_LAYER            (32 - SKC_TTCK_HI_BITS_YX)
#define SKC_TTCK_HI_MASK_LAYER            SKC_BITS_TO_MASK(SKC_TTCK_HI_BITS_LAYER)
#define SKC_TTCK_HI_SHR_LAYER             (SKC_TTCK_HI_BITS_Y + SKC_TTCK_HI_BITS_X + SKC_TTCK_BITS_LAYER - 32)

#define SKC_TTCK_LO_BITS_LAYER            (SKC_TTCK_BITS_LAYER - SKC_TTCK_HI_BITS_LAYER)

//
// TILE COORD
//
//  0                32
//  | N/A |  X  |  Y  |
//  +-----+-----+-----+
//  |  8  | 12  | 12  |
//
//
// This simplifies the clip test in the place kernel.
//

union skc_tile_coord
{
  skc_uint   u32;

  struct {
#if defined(__OPENCL_C_VERSION__)
    skc_uint xy;
#else
    skc_uint na0 : 32 - SKC_TTXK_HI_BITS_YX;  // 8
    skc_uint x   : SKC_TTXK_HI_BITS_X;        // 12
    skc_uint y   : SKC_TTXK_HI_BITS_Y;        // 12
#endif
  };
};

SKC_STATIC_ASSERT(sizeof(union skc_tile_coord) == sizeof(skc_uint));

//
//
//

union skc_tile_clip
{
  skc_uint               u32a2[2];

  skc_uint2              u32v2;

  struct {
    union skc_tile_coord xy0; // lower left
    union skc_tile_coord xy1; // upper right
  };
};

SKC_STATIC_ASSERT(sizeof(union skc_tile_clip) == sizeof(skc_uint2));

//
//
//

#endif

//
//
//
