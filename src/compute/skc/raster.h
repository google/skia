/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#ifndef SKC_ONCE_RASTER
#define SKC_ONCE_RASTER

//
//
//

#include "block.h"

//
// The raster "head" structure aliases a block.
//
// Blocks are guaranteed to be at least 16 words.
//
// Note that the end of the ttsk key sequence and beginning of the
// ttpk key sequence may share a node.
//
// Note that keys are 64-bits and the "next" id is only a 32-bit
// (27-bit tagged block id) so there is room for another next id or a
// key/node countdown. For now we'll leave the second to last word
// empty/unused.
//

typedef skc_uint2 skc_ttsk_t;
typedef skc_uint2 skc_ttpk_t;
typedef skc_uint2 skc_ttxk_t;

union skc_raster_node_next
{
  skc_uint2        u32v2;

  struct {
    skc_block_id_t node; // low word
    skc_uint       na;
  };
};

//
//
//

union skc_raster_node_elem
{
  skc_uint2                  u32v2;
  skc_ttsk_t                 sk;
  skc_ttpk_t                 pk;
  skc_ttxk_t                 xk;
  union skc_raster_node_next next;
};

//
//
//

union skc_raster_header
{
  skc_uint4  u32v4;

  struct {
    skc_uint blocks; // # of blocks -- head+node+skb+pkb    -- uint2.lo
    skc_uint na;     // unused                              -- uint2.hi
    skc_uint nodes;  // # of nodes  -- not including header -- uint2.lo
    skc_uint keys;   // # of sk+pk keys                     -- uint2.hi
  };
};

//
//
//

struct skc_raster_head
{
  union skc_raster_header    header;  // 3 counters and a spare
  skc_int4                   bounds;  // sub-pixel resolution bounds
  union skc_raster_node_elem elems[]; // 64-bit keys and index of next node
};

//
//
//

#define SKC_RASTER_HEAD_WORDS                     8

#define SKC_RASTER_HEAD_OFFSET_COUNTS_BLOCKS      0
#define SKC_RASTER_HEAD_OFFSET_COUNTS_NA          1
#define SKC_RASTER_HEAD_OFFSET_COUNTS_NODES       2
#define SKC_RASTER_HEAD_OFFSET_COUNTS_KEYS        3

#define SKC_RASTER_HEAD_OFFSET_BOUNDS             4
#define SKC_RASTER_HEAD_OFFSET_ELEMS              8

//
//
//

#define SKC_RASTER_HEAD_WORDS_CALC                (sizeof(struct skc_raster_head) / sizeof(skc_uint))

#define SKC_RASTER_HEAD_OFFSET_COUNTS_BLOCKS_CALC (SKC_OFFSET_OF(struct skc_raster_head,header.blocks) / sizeof(skc_uint))
#define SKC_RASTER_HEAD_OFFSET_COUNTS_NODES_CALC  (SKC_OFFSET_OF(struct skc_raster_head,header.nodes)  / sizeof(skc_uint))
#define SKC_RASTER_HEAD_OFFSET_COUNTS_KEYS_CALC   (SKC_OFFSET_OF(struct skc_raster_head,header.keys)   / sizeof(skc_uint))

#define SKC_RASTER_HEAD_OFFSET_BOUNDS_CALC        (SKC_OFFSET_OF(struct skc_raster_head,bounds) / sizeof(skc_uint))
#define SKC_RASTER_HEAD_OFFSET_ELEMS_CALC         (SKC_OFFSET_OF(struct skc_raster_head,elems)  / sizeof(skc_uint))

//
// NOT ALL OPENCL PREPROCESSORS ARE HAPPY WITH CALCULATING OFFSET_OF()
//
// - Intel ioc compiler failed in the past
//

#if !defined(__OPENCL_C_VERSION__)
SKC_STATIC_ASSERT(SKC_RASTER_HEAD_WORDS                == SKC_RASTER_HEAD_WORDS_CALC);
SKC_STATIC_ASSERT(SKC_RASTER_HEAD_OFFSET_COUNTS_BLOCKS == SKC_RASTER_HEAD_OFFSET_COUNTS_BLOCKS_CALC);
SKC_STATIC_ASSERT(SKC_RASTER_HEAD_OFFSET_COUNTS_NODES  == SKC_RASTER_HEAD_OFFSET_COUNTS_NODES_CALC);
SKC_STATIC_ASSERT(SKC_RASTER_HEAD_OFFSET_COUNTS_KEYS   == SKC_RASTER_HEAD_OFFSET_COUNTS_KEYS_CALC);
SKC_STATIC_ASSERT(SKC_RASTER_HEAD_OFFSET_BOUNDS        == SKC_RASTER_HEAD_OFFSET_BOUNDS_CALC);
SKC_STATIC_ASSERT(SKC_RASTER_HEAD_OFFSET_ELEMS         == SKC_RASTER_HEAD_OFFSET_ELEMS_CALC);
#endif

//
//
//

#if 0
#if !defined(__OPENCL_C_VERSION__)
struct skc_raster_node
{
  union skc_raster_node_elem elems[];
};
#endif
#endif

//
//
//

#define SKC_RASTER_HEAD_DWORDS       (SKC_RASTER_HEAD_WORDS / 2)
#define SKC_RASTER_NODE_DWORDS       SKC_DEVICE_BLOCK_DWORDS

//
//

#define SKC_RASTER_NODE_COUNT_KEYS   (SKC_DEVICE_BLOCK_DWORDS - 1)
#define SKC_RASTER_HEAD_COUNT_KEYS   (SKC_RASTER_NODE_COUNT_KEYS - SKC_RASTER_HEAD_DWORDS)

//
// these are the most possible keys that could be produced by a node
//

#define SKC_RASTER_HEAD_MAX_TTSK     SKC_RASTER_HEAD_COUNT_KEYS
#define SKC_RASTER_HEAD_MAX_TTPK     ((SKC_DEVICE_BLOCK_DWORDS - SKC_RASTER_HEAD_DWORDS) / 2)

#define SKC_RASTER_NODE_MAX_TTSK     SKC_RASTER_NODE_COUNT_KEYS   // a node could be all TTSK keys minus next pointer
#define SKC_RASTER_NODE_MAX_TTPK     (SKC_RASTER_NODE_DWORDS / 2) // a node could be at most half TTPK keys and (half-1) TTSK keys

#define SKC_RASTER_HEAD_MIN_TTSK     (SKC_RASTER_HEAD_MAX_TTSK - SKC_RASTER_NODE_MAX_TTPK)
#define SKC_RASTER_HEAD_MIN_TTPK     0

#define SKC_RASTER_NODE_MIN_TTSK     (SKC_RASTER_NODE_MAX_TTSK - SKC_RASTER_NODE_MAX_TTPK)
#define SKC_RASTER_NODE_MIN_TTPK     0

//
//
//

#endif

//
//
//
