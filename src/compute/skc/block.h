/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#ifndef SKC_ONCE_BLOCK
#define SKC_ONCE_BLOCK

//
//
//

#include "types.h"
#include "macros.h"

//
// Hard requirements:
//
//   - A TTXB "block pool" extent that is at least 1GB.
//
//   - A virtual surface of at least 8K x 8K
//
//   - A physical surface of __don't really care__ because it's
//     advantageous to tile the physical surface into sub-surface
//     rectangles so that multiple linear (smaller) TTCK sorts are
//     simultaneously performed.
//
//
//      EXTENT                 TTXB BITS
//     SIZE (MB) +-------------------------------------+
//               |  22    23    24    25    26  * 27 * |
//          +----+-------------------------------------+
//          |  8 |  128   256   512  1024  2048  4096  |
//     TTXB | 16 |  256   512  1024  2048  4096  8192  |
//    WORDS | 32 |  512  1024  2048  4096  8192 16384  |
//          | 64 | 1024  2048  4096  8192 16384 32768  |
//          +----+-------------------------------------+
//
//
//    SUB-SURFACE                        X/Y BITS
//     TILE SIZE +------------------------------------------------------+
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
//   - 27 bits of block id space implies a max of 4GB-32GB of
//     rasterized paths depending on the size of the TTXB block.
//     This could enable interesting use cases.
//
//   - A minimum block size is 16 words (64 bytes) and a maximum of 32
//     words (128 bytes) greatly simplifies portability. It's unlikely
//     that a large GPU can service a tile larger than 32x32 pixels.
//     Additionally, on smaller devices, rectangular tiles will have a
//     minimum height of 16 pixels.  Note that a minimum subblock size
//     is probably 8 words (32 bytes).
//
//   - A virtual rasterization surface that's from +/-32768K to
//     +/-128K depending on the target's rasterization tile size.
//
//   - A physical sub-surface tile (or entire surface) from 4Kx4K to
//     16Kx16K depending on the target's rasterization tile size.
//
//   - Support for a minimum of 256K layers. If necessary, this can
//     convseratively raised to 1m or 2m layers by either implementing
//     surface tiling or when the target supports large raster tiles.
//
//   - Keys that (optionally) only require a 32-bit high word
//     comparison.
//
//
// TAGGED BLOCK ID
//
//  0     5                  31
//  |     |         ID        |
//  | TAG | SUBBLOCK | BLOCK  |
//  +-----+----------+--------+
//  |  5  |     N    | 27 - N |
//
// There are 27 bits of subblocks and 5 bits of tag.
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


#define SKC_TAGGED_BLOCK_ID_BITS_ID   27 // this size is cast in stone
#define SKC_TAGGED_BLOCK_ID_BITS_TAG   5 // which leaves 5 bits of tag

//
//
//

typedef enum skc_block_id_tag {

  SKC_BLOCK_ID_TAG_PATH_LINE,      // 0 -- 4  segments
  SKC_BLOCK_ID_TAG_PATH_QUAD,      // 1 -- 6  segments
  SKC_BLOCK_ID_TAG_PATH_CUBIC,     // 2 -- 8  segments
  SKC_BLOCK_ID_TAG_PATH_RAT_QUAD,  // 3 -- 8  segments : 6 + w0 + na
  SKC_BLOCK_ID_TAG_PATH_RAT_CUBIC, // 4 -- 10 segments : 8 + w0 + w1
  SKC_BLOCK_ID_TAG_PATH_NEXT,      // 5 -- this represents the end of path tags

  //
  // TAGS [6-30] ARE AVAILABLE
  //

  SKC_BLOCK_ID_TAG_INVALID = (1u << SKC_TAGGED_BLOCK_ID_BITS_TAG) - 1, // all 1's
  SKC_BLOCK_ID_TAG_COUNT,

} skc_block_id_tag;

//
//
//

#define SKC_TAGGED_BLOCK_ID_INVALID  SKC_UINT_MAX // all 1's

//
//
//

typedef skc_uint skc_block_id_t;

//
//
//

typedef skc_uint skc_tagged_block_id_t;

union skc_tagged_block_id
{
  skc_uint   u32;

#if !defined(__OPENCL_C_VERSION__)
  struct {
    skc_uint tag : SKC_TAGGED_BLOCK_ID_BITS_TAG;
    skc_uint id  : SKC_TAGGED_BLOCK_ID_BITS_ID;
  };
#else
  //
  // OPENCL BIT-FIELD EXTRACT/INSERT
  //
#endif
};

//
//
//

#define SKC_TAGGED_BLOCK_ID_MASK_TAG     SKC_BITS_TO_MASK(SKC_TAGGED_BLOCK_ID_BITS_TAG)

#define SKC_TAGGED_BLOCK_ID_GET_TAG(bst) ((bst) &  SKC_TAGGED_BLOCK_ID_MASK_TAG)
#define SKC_TAGGED_BLOCK_ID_GET_ID(bst)  ((bst) >> SKC_TAGGED_BLOCK_ID_BITS_TAG)

//
//
//

#endif

//
//
//
