/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#ifndef SKC_COMMON_ONCE
#define SKC_COMMON_ONCE

#include "types.h"

//
// structures common to both host and device -- placeholder until
// everything shakes out
//

union skc_transform
{
  //
  // Transform is always scaled s.t. w2 is 1.0f
  //
  skc_float   f32a8[8];

  skc_float8  f32v8;

  struct {
    skc_float sx;
    skc_float shx;
    skc_float tx;

    skc_float shy;
    skc_float sy;
    skc_float ty;

    skc_float w0;
    skc_float w1;
  };
};

//
//
//

union skc_path_clip
{
  skc_float   f32a4[4]; // FIXME -- use the SIMD4 representation trick

  skc_float4  f32v4;

  struct {
    skc_float x0;
    skc_float y0;
    skc_float x1;
    skc_float y1;
  };
};

//
// host-side path fill cmd that is expanded into rasterization cmds
//
// FIXME -- these magic numbers will be replaced with tile.h constants
//
// FIXME -- make this command opaque by moving it into the platform impl
//
// FIXME -- NEED TO EVALUATE IF THIS DISTRIBUTION OF BITS IS GOING TO
// BE TOO SMALL -- plenty of room to jiggle these bits
//

#define SKC_CMD_FILL_BITS_TRANSFORM  12  // distinct transforms -- perhaps too generous
#define SKC_CMD_FILL_BITS_CLIP       12  // distinct clips      -- perhaps too generous
#define SKC_CMD_FILL_BITS_COHORT      8  // perhaps too small

//
//
//

typedef skc_uint skc_path_h; // host path handle

union skc_cmd_fill
{
  skc_ulong    u64;

  skc_uint2    u32v2;

  struct {
    skc_path_h path;           // host path id
#if defined(__OPENCL_C_VERSION__)
    skc_uint   tcc;
#else
    skc_uint   transform : SKC_CMD_FILL_BITS_TRANSFORM;
    skc_uint   clip      : SKC_CMD_FILL_BITS_CLIP;
    skc_uint   cohort    : SKC_CMD_FILL_BITS_COHORT;
#endif
  };
};

//
//
//

typedef skc_uint skc_raster_h;

union skc_cmd_place
{
  skc_uint4       u32v4;

  struct {
    skc_raster_h  raster_h;
    skc_uint      layer_id;
    skc_uint      tx;
    skc_uint      ty;
  };
};

SKC_STATIC_ASSERT(sizeof(union skc_cmd_place) == sizeof(skc_uint4));

//
//
//

#endif

//
//
//
