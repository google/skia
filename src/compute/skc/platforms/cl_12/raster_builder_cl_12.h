/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#ifndef SKC_RASTER_BUILDER_CL_12_ONCE
#define SKC_RASTER_BUILDER_CL_12_ONCE

//
//
//

#include "types.h"
#include "macros.h"
#include "common.h"

//
// FIXME -- these magic numbers will be replaced with tile.h constants
// although they're probably universal across all devices
//
// FIXME -- NEED TO EVALUATE IF THIS DISTRIBUTION OF BITS IS GOING TO
// BE TOO SMALL -- plenty of room to jiggle these bits
//

#define SKC_CMD_RASTERIZE_BITS_TRANSFORM  12
#define SKC_CMD_RASTERIZE_BITS_CLIP       12
#define SKC_CMD_RASTERIZE_BITS_COHORT      8

SKC_STATIC_ASSERT(SKC_CMD_RASTERIZE_BITS_TRANSFORM == SKC_CMD_FILL_BITS_TRANSFORM);
SKC_STATIC_ASSERT(SKC_CMD_RASTERIZE_BITS_CLIP      == SKC_CMD_FILL_BITS_CLIP);
SKC_STATIC_ASSERT(SKC_CMD_RASTERIZE_BITS_COHORT    == SKC_CMD_FILL_BITS_COHORT);

//
// device-side rasterization cmd
//

union skc_cmd_rasterize
{
  skc_ulong    u64;

  skc_uint2    u32v2;

  struct {
    //
    // Unlike anywhere else in the pipeline, the nodeword index points
    // "inside" of a path node (with word resolution). This means
    // there is up to 16 GB of 32-bit word addressing in a unified
    // block pool:
    //
    // "16GB ought to be enough for anyone" -- ASM 5/30/17
    //
    skc_uint   nodeword;
#if defined(__OPENCL_C_VERSION__)
    skc_uint   tcc;
#else
    skc_uint   transform : SKC_CMD_RASTERIZE_BITS_TRANSFORM;
    skc_uint   clip      : SKC_CMD_RASTERIZE_BITS_CLIP;
    skc_uint   cohort    : SKC_CMD_RASTERIZE_BITS_COHORT;
#endif
  };
};

SKC_STATIC_ASSERT(sizeof(union skc_cmd_rasterize) == sizeof(skc_uint2));

//
//
//

#define SKC_CMD_RASTERIZE_HI_OFFSET_COHORT  (SKC_CMD_RASTERIZE_BITS_TRANSFORM + SKC_CMD_RASTERIZE_BITS_CLIP)
#define SKC_CMD_RASTERIZE_MASK_COHORT(c)    ((c).u32v2.hi & SKC_BITS_TO_MASK_AT(SKC_CMD_RASTERIZE_BITS_COHORT,SKC_CMD_RASTERIZE_HI_OFFSET_COHORT))

#define SKC_CMD_RASTERIZE_GET_TRANSFORM(c)  ((c).u32v2.hi & SKC_BITS_TO_MASK(SKC_CMD_RASTERIZE_BITS_TRANSFORM))
#define SKC_CMD_RASTERIZE_GET_CLIP(c)       SKC_BFE((c).tcc,SKC_CMD_RASTERIZE_BITS_CLIP,SKC_CMD_RASTERIZE_BITS_TRANSFORM)
#define SKC_CMD_RASTERIZE_GET_COHORT(c)     ((c).u32v2.hi >> SKC_CMD_RASTERIZE_HI_OFFSET_COHORT)
// SKC_BFE((c).tcc,SKC_CMD_RASTERIZE_BITS_COHORT,SKC_CMD_RASTERIZE_HI_OFFSET_COHORT)

//
//
//

#define SKC_TTSK_SIZE_COHORT                (1 << SKC_CMD_RASTERIZE_BITS_COHORT)

//
// COHORT META DATA
//

union skc_raster_cohort_meta_in
{
  skc_uint4  u32v4;

  struct {
    skc_uint blocks; // # of rk blocks
    skc_uint offset; // start of rk span
    skc_uint pk;     // # of pk keys
    skc_uint rk;     // # of rk keys
  };
};

union skc_raster_cohort_meta_out
{
  skc_uint4  u32v4;

  struct {
    skc_uint blocks; // # of blocks in raster -- initially just rk blocks
    skc_uint offset; // start of rk span
    skc_uint nodes;  // # of nodes in raster  -- necessary for walking
    skc_uint keys;   // # of rk & pk keys     -- initially just rk
  };
};

union skc_raster_cohort_meta_inout
{
  union skc_raster_cohort_meta_in  in;
  union skc_raster_cohort_meta_out out;
};

//
// followed by one word for the offset
//

struct skc_raster_cohort_meta
{
  union skc_raster_cohort_meta_inout inout[SKC_TTSK_SIZE_COHORT];
  skc_uint                           reads[SKC_TTSK_SIZE_COHORT]; // starting ring reads  -- [0] is raster head
};

#define SKC_RASTER_COHORT_META_OFFSET_READS (SKC_OFFSET_OF(struct skc_raster_cohort_meta,reads) / sizeof(skc_uint))

//
// COHORT ATOMICS
//

struct skc_raster_cohort_atomic
{
  // rasterization input
  skc_uint cmds;

  // rasterization output
  skc_uint keys;

  // block pool base -- idea here is to perform one atomic allocation
  // skc_uint bp_base;
};

#define SKC_RASTER_COHORT_ATOMIC_OFFSET_CMDS      0
#define SKC_RASTER_COHORT_ATOMIC_OFFSET_KEYS      1

#define SKC_RASTER_COHORT_ATOMIC_OFFSET_CMDS_CALC (SKC_OFFSET_OF(struct skc_raster_cohort_atomic,cmds) / sizeof(skc_uint))
#define SKC_RASTER_COHORT_ATOMIC_OFFSET_KEYS_CALC (SKC_OFFSET_OF(struct skc_raster_cohort_atomic,keys) / sizeof(skc_uint))

SKC_STATIC_ASSERT(SKC_RASTER_COHORT_ATOMIC_OFFSET_CMDS == SKC_RASTER_COHORT_ATOMIC_OFFSET_CMDS_CALC); // verify
SKC_STATIC_ASSERT(SKC_RASTER_COHORT_ATOMIC_OFFSET_KEYS == SKC_RASTER_COHORT_ATOMIC_OFFSET_KEYS_CALC); // verify

//
//
//

#endif

//
//
//
