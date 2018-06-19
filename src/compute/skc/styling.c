/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#include <memory.h>

#include "styling.h"
#include "styling_types.h"
#include "skc.h"

//
// FIXME -- x86'isms are temporary
//

#include <immintrin.h>

//
//
//

skc_err
skc_styling_retain(skc_styling_t styling)
{
  styling->ref_count += 1;

  return SKC_ERR_SUCCESS;
}

skc_err
skc_styling_release(skc_styling_t styling)
{
  //
  // blocks and waits if grid is active
  //
  styling->release(styling->impl);

  return SKC_ERR_SUCCESS;
}

skc_err
skc_styling_seal(skc_styling_t styling)
{
  //
  // no-op if sealed
  //
  styling->seal(styling->impl);

  return SKC_ERR_SUCCESS;
}

skc_err
skc_styling_unseal(skc_styling_t styling)
{
  //
  // no-op if unsealed
  // blocks and waits if sealed and grid is active
  //
  styling->unseal(styling->impl,false);

  return SKC_ERR_SUCCESS;
}

skc_err
skc_styling_reset(skc_styling_t styling)
{
  styling->unseal(styling->impl,true);

  styling->layers.count = 0;
  styling->groups.count = 0;
  styling->extras.count = 0;

  return SKC_ERR_SUCCESS;
}

//
// FIXME -- various robustifications can be made to this builder but
// we don't want to make this heavyweight too soon
//
// - out of range layer_id is an error
// - extras[] overflow is an error
//

skc_err
skc_styling_group_alloc(skc_styling_t  styling,
                        skc_group_id * group_id)
{
  styling->unseal(styling->impl,true);

  *group_id = styling->groups.count++;

  return SKC_ERR_SUCCESS;
}

skc_err
skc_styling_group_enter(skc_styling_t             styling,
                        skc_group_id              group_id,
                        uint32_t                  n,
                        skc_styling_cmd_t const * cmds)
{
  styling->unseal(styling->impl,true);

  styling->groups.extent[group_id].cmds.enter = styling->extras.count;

  memcpy(styling->extras.extent + styling->extras.count,cmds,n * sizeof(*cmds));

  styling->extras.count += n;

  return SKC_ERR_SUCCESS;
}

skc_err
skc_styling_group_leave(skc_styling_t             styling,
                        skc_group_id              group_id,
                        uint32_t                  n,
                        skc_styling_cmd_t const * cmds)
{
  styling->unseal(styling->impl,true);

  styling->groups.extent[group_id].cmds.leave = styling->extras.count;

  memcpy(styling->extras.extent + styling->extras.count,cmds,n * sizeof(*cmds));

  styling->extras.count += n;

  return SKC_ERR_SUCCESS;
}

skc_err
skc_styling_group_parents(skc_styling_t        styling,
                          skc_group_id         group_id,
                          uint32_t             depth,
                          skc_group_id const * parents)
{
  styling->unseal(styling->impl,true);

  styling->groups.extent[group_id].parents = (union skc_group_parents)
    {
      .depth = depth,
      .base  = styling->extras.count
    };

  memcpy(styling->extras.extent + styling->extras.count,parents,depth * sizeof(*parents));

  styling->extras.count += depth;

  return SKC_ERR_SUCCESS;
}

skc_err
skc_styling_group_range_lo(skc_styling_t styling,
                           skc_group_id  group_id,
                           skc_layer_id  layer_lo)
{
  styling->unseal(styling->impl,true);

  styling->groups.extent[group_id].range.lo = layer_lo;

  return SKC_ERR_SUCCESS;
}

skc_err
skc_styling_group_range_hi(skc_styling_t styling,
                           skc_group_id  group_id,
                           skc_layer_id  layer_hi)
{
  styling->unseal(styling->impl,true);

  styling->groups.extent[group_id].range.hi = layer_hi;

  return SKC_ERR_SUCCESS;
}

skc_err
skc_styling_group_layer(skc_styling_t             styling,
                        skc_group_id              group_id,
                        skc_layer_id              layer_id,
                        uint32_t                  n,
                        skc_styling_cmd_t const * cmds)
{
  styling->unseal(styling->impl,true);

  styling->layers.extent[layer_id] = (union skc_layer_node)
    {
      .cmds   = styling->extras.count,
      .parent = group_id
    };

  memcpy(styling->extras.extent + styling->extras.count,cmds,n * sizeof(*cmds));

  styling->extras.count += n;

  return SKC_ERR_SUCCESS;
}

//
// FIXME -- get rid of these x86'isms ASAP -- let compiler figure it
// out with a vector type
//

static
__m128i
skc_convert_colors_4(float const * const colors)
{
  __m128i c;

  c = _mm_cvtps_ph(*(__m128*)colors,0);

  return c;
}

static
__m128i
skc_convert_colors_8(float const * const colors)
{
  __m128i c;

  c = _mm256_cvtps_ph(*(__m256*)colors,0);

  return c;
}

//
//
//

static
void
skc_styling_layer_cmd_rgba_encoder(skc_styling_cmd_t  * const cmds,
                                   skc_styling_opcode_e const opcode,
                                   float                const rgba[4])
{
  __m128i const c = skc_convert_colors_4(rgba);

  cmds[0] = opcode;
  cmds[1] = c.m128i_u32[0];
  cmds[2] = c.m128i_u32[1];
}

void
skc_styling_background_over_encoder(skc_styling_cmd_t * cmds, float const rgba[4])
{
  skc_styling_layer_cmd_rgba_encoder(cmds,SKC_STYLING_OPCODE_BACKGROUND_OVER,rgba);
}

void
skc_styling_layer_fill_rgba_encoder(skc_styling_cmd_t * cmds, float const rgba[4])
{
  // encode a solid fill
  skc_styling_layer_cmd_rgba_encoder(cmds,SKC_STYLING_OPCODE_COLOR_FILL_SOLID,rgba);
}

//
//
//

void
skc_styling_layer_fill_gradient_encoder(skc_styling_cmd_t         * cmds,
                                        float                       x0,
                                        float                       y0,
                                        float                       x1,
                                        float                       y1,
                                        skc_styling_gradient_type_e type,
                                        uint32_t                    n,
                                        float                 const stops[],
                                        float                 const colors[])
{
  union skc_styling_cmd * const cmds_u = (union skc_styling_cmd *)cmds;

  //
  // encode a gradient fill
  //
  cmds_u[0].opcode = SKC_STYLING_OPCODE_COLOR_FILL_GRADIENT_LINEAR;

  float const dx = x1 - x0;
  float const dy = y1 - y0;
  float const c1 = x0 * dx + y0 * dy;
  float const c2 = x1 * dx + y1 * dy;

  cmds_u[1].f32 =  dx;               // dx
  cmds_u[2].f32 = -c1;               // p0
  cmds_u[3].f32 =  dy;               // dy
  cmds_u[4].f32 =  1.0f / (c2 - c1); // denom

  //
  // store type
  //
  cmds_u[5].gradient_type = type;

  //
  // Write out slopes
  //
  // Note: make sure that that the first and last stop pairs don't
  // have a span of zero.  Why?  Because it's meaningless and the
  // zero-span stops can simply be dropped.
  //
  // And obviously the stops need to monotonically increasing.
  //
  // These validations can be perfomed elsewhere.
  //
  // After a pile of simple algebra the slope necessary to map a stop
  // percentage on [0,1] to an INDEX.LERP real number from [0.0,N.0]
  // is simply:
  //
  //                delta_stop_prev
  //   slope_curr = --------------- - 1
  //                delta_stop_curr
  //
  // Each delta stop equal to zero reduces the stop count by 1.
  //
  // Note that color pairs are what's stored so this simplified
  // representation works for both linear gradients with non-zero
  // delta stops and linear gradients that double-up the stops in
  // order to produce "stripes".
  //
  float                         ds_prev = stops[1] - stops[0];
  union skc_styling_cmd * const slopes  = cmds_u + 8;

  slopes[0].f32 = 1.0f / ds_prev;

  uint32_t ds_count = 1;

  for (uint32_t ii=1; ii<n-1; ii++)
    {
      float const ds_curr = stops[ii+1] - stops[ii];

      if (ds_curr > 0.0f)
        {
          slopes[ds_count++].f32 = ds_prev / ds_curr - 1.0f;
          ds_prev                = ds_curr;
        }
    }

  //
  // save a potentially compressed delta slope count
  //
  cmds_u[6].u32 = ds_count;
  cmds_u[7].u32 = n; // REMOVE ME -------------------------------------------- REMOVE

  //
  // FIXME -- encode color pair as a single color diff as noted by HERB @ CHAP <------------- FIXME
  //

  //
  // write out color pairs while skipping delta stops equal to zero
  //
  uint32_t const color_count = ds_count + 1;

  union skc_styling_cmd * color_r = cmds_u + 8 + ds_count;
  union skc_styling_cmd * color_g = color_r + color_count;
  union skc_styling_cmd * color_b = color_r + color_count * 2;
  union skc_styling_cmd * color_a = color_r + color_count * 3;

  for (uint32_t ii=0; ii<n-1; ii++)
    {
      if (stops[ii+1] > stops[ii])
        {
          __m128i const c = skc_convert_colors_8(colors+ii*4);

          color_r->u16v2.lo = c.m128i_u16[0];
          color_r->u16v2.hi = c.m128i_u16[4];
          color_g->u16v2.lo = c.m128i_u16[1];
          color_g->u16v2.hi = c.m128i_u16[5];
          color_b->u16v2.lo = c.m128i_u16[2];
          color_b->u16v2.hi = c.m128i_u16[6];
          color_a->u16v2.lo = c.m128i_u16[3];
          color_a->u16v2.hi = c.m128i_u16[7];

          ++color_r;
          ++color_g;
          ++color_b;
          ++color_a;
        }
    }

  float laststop[8]; // sentinel to lerp against same color

  for (int ii=0; ii<4; ii++)
    laststop[ii+4] = laststop[ii] = colors[(n-1)*4+ii];

  __m128i const c = skc_convert_colors_8(laststop);

  color_r->u16v2.lo = c.m128i_u16[0];
  color_r->u16v2.hi = c.m128i_u16[4];
  color_g->u16v2.lo = c.m128i_u16[1];
  color_g->u16v2.hi = c.m128i_u16[5];
  color_b->u16v2.lo = c.m128i_u16[2];
  color_b->u16v2.hi = c.m128i_u16[6];
  color_a->u16v2.lo = c.m128i_u16[3];
  color_a->u16v2.hi = c.m128i_u16[7];
}

//
//
//
