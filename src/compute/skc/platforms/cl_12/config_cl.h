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

#include "block_pool_cl.h"

//
// FIXME -- define individual structs before defining skc_config
//

struct skc_config
{
  struct {
    struct {
      skc_uint               size;
      skc_uint               subbufs;
    } host;   // alignment determined by compiler
    struct {
      skc_uint               size;
      skc_uint               subbufs;
    } device; // alignment determined by device
  } suballocator;

  struct {
    skc_uint                 size;
  } scheduler;

  struct {
    skc_uint                 bytes;    // bytes per subblock -- pow2
    skc_uint                 words;    // words per subblock -- pow2
    // skc_uint              words_log2;
  } subblock;

  struct {
    skc_uint                 bytes;     // bytes per block     -- pow2
    skc_uint                 words;     // words per block     -- pow2
    skc_uint                 subblocks; // subblocks per block -- block.bytes >= subblock.bytes
    // skc_uint              subblocks_log2;
  } block;

  union skc_block_pool_size  block_pool;

  struct {
    cl_command_queue_properties cq_props;
    skc_uint                    size;
  } cq_pool;

  struct {
    skc_uint                 size;      // a large fraction of block pool size
    skc_uint                 width;     // determines number of launched reclamation subgroups
    skc_uint                 recs;      // how many in-flight width-subgroup reclamation grids
  } handle_pool;

  struct {
    skc_uint                 width;     // tile width  in pixels
    skc_uint                 height;    // tile height in pixels
    skc_uint                 ratio;     // subblocks per TTPB
  } tile;

  struct {
    struct {
      skc_uint               count;     // # of subbufs in buffer
    } buffer;

    struct {
      skc_uint               count;     // # of blocks/commands in subbuf
    } subbuf;

    struct {
      size_t                 buffer;    // block.bytes * subbuf.blocks * subbuf.count
      size_t                 subbuf;    // block.bytes * subbuf.blocks -- multiple of CL_DEVICE_MEM_BASE_ADDR_ALIGN
    } block;

    struct {
      size_t                 buffer;    // sizeof(skc_uint) * subbuf.blocks * subbuf.count
      size_t                 subbuf;    // sizeof(skc_uint) * subbuf.blocks -- multiple of CL_DEVICE_MEM_BASE_ADDR_ALIGN
    } command;
    //
    // skc_uint              paths_lowat;
    //
  } paths_copy;

  struct {
    struct {
      skc_uint               elem_count;
      skc_uint               snap_count;
    } path_ids;

    struct {
      skc_uint               elem_count;
      skc_uint               snap_count;
    } transforms;

    struct {
      skc_uint               elem_count;
      skc_uint               snap_count;
    } clips;

    struct {
      skc_uint               elem_count;
      skc_uint               snap_count;
    } fill;

    struct {
      skc_uint               elem_count;
      skc_uint               snap_count;
    } raster_ids;

    struct {
      skc_uint               cmds;
    } expand;

    struct {
      skc_uint               keys;
    } rasterize;
  } raster_cohort;

  struct {
    struct {
      skc_uint               elem_count;
      skc_uint               snap_count;
    } cmds;

    struct {
      skc_uint               elem_count;
    } raster_ids;

    struct {
      skc_uint               elem_count;
    } keys;
  } composition;
};

//
//
//
