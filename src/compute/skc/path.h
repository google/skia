/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#ifndef SKC_ONCE_PATH
#define SKC_ONCE_PATH

//
//
//

#include "block.h"

//
//
//

union skc_path_header
{
  skc_uint4   u32v4;

  struct {
    skc_uint  handle; // host handle
    skc_uint  blocks; // # of S-segment blocks in path
    skc_uint  nodes;  // # of S-segment node blocks -- not including header
    skc_uint  prims;  // # of path elements: lines, quads, cubics, rat-quads, rat-cubics
  };
};

union skc_path_bounds
{
  skc_float4  f32v4;

  struct {
    skc_float x0;
    skc_float y0;
    skc_float x1;
    skc_float y1;
  };
};

//
// This head structure aliases a block.
//
// Blocks are at least 16 words.
//

struct skc_path_head
{
  union skc_path_header     header;    // 4
  union skc_path_bounds     bounds;    // 4
  union skc_tagged_block_id tag_ids[]; // 8+ ids
};

#define SKC_PATH_HEAD_WORDS               8

#define SKC_PATH_HEAD_OFFSET_HANDLE       0
#define SKC_PATH_HEAD_OFFSET_BLOCKS       1
#define SKC_PATH_HEAD_OFFSET_NODES        2
#define SKC_PATH_HEAD_OFFSET_PRIMS        3

#define SKC_PATH_HEAD_OFFSET_IDS          8

#define SKC_PATH_HEAD_WORDS_CALC          (sizeof(struct skc_path_head) / sizeof(skc_uint))
#define SKC_PATH_HEAD_OFFSET_HANDLE_CALC  (SKC_OFFSET_OF(struct skc_path_head,header.handle) / sizeof(skc_uint))
#define SKC_PATH_HEAD_OFFSET_BLOCKS_CALC  (SKC_OFFSET_OF(struct skc_path_head,header.blocks) / sizeof(skc_uint))
#define SKC_PATH_HEAD_OFFSET_NODES_CALC   (SKC_OFFSET_OF(struct skc_path_head,header.nodes)  / sizeof(skc_uint))
#define SKC_PATH_HEAD_OFFSET_PRIMS_CALC   (SKC_OFFSET_OF(struct skc_path_head,header.prims)  / sizeof(skc_uint))
#define SKC_PATH_HEAD_OFFSET_IDS_CALC     (SKC_OFFSET_OF(struct skc_path_head,tag_ids)       / sizeof(skc_uint))

//
// NOT ALL OPENCL PREPROCESSORS ARE HAPPY WITH CALCULATING OFFSET_OF()
//
// - Intel ioc compiler failed in the past
//

#if !defined(__OPENCL_C_VERSION__)
SKC_STATIC_ASSERT(SKC_PATH_HEAD_WORDS         == SKC_PATH_HEAD_WORDS_CALC);
SKC_STATIC_ASSERT(SKC_PATH_HEAD_OFFSET_HANDLE == SKC_PATH_HEAD_OFFSET_HANDLE_CALC);
SKC_STATIC_ASSERT(SKC_PATH_HEAD_OFFSET_BLOCKS == SKC_PATH_HEAD_OFFSET_BLOCKS_CALC);
SKC_STATIC_ASSERT(SKC_PATH_HEAD_OFFSET_NODES  == SKC_PATH_HEAD_OFFSET_NODES_CALC);
SKC_STATIC_ASSERT(SKC_PATH_HEAD_OFFSET_PRIMS  == SKC_PATH_HEAD_OFFSET_PRIMS_CALC);
SKC_STATIC_ASSERT(SKC_PATH_HEAD_OFFSET_IDS    == SKC_PATH_HEAD_OFFSET_IDS_CALC);
#endif

//
// This node structure aliases a block.
//
// Blocks are at least 16 words.
//
// The last word in the block points to the next block
//
// The tag indicating a 'next' pointer is zero.
//

#if !defined(__OPENCL_C_VERSION__)
struct skc_path_node
{
  union skc_tagged_block_id tag_ids[];
};
#endif

//
//
//

#endif

//
//
//
