/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

//
//
//

#include "path_builder.h"
#include "context.h"

//
//
//

skc_err
skc_path_builder_retain(skc_path_builder_t path_builder)
{
  ++path_builder->refcount;

  return SKC_ERR_SUCCESS;
}

skc_err
skc_path_builder_release(skc_path_builder_t path_builder)
{
  SKC_ASSERT_STATE_ASSERT(SKC_PATH_BUILDER_STATE_READY,path_builder);

  path_builder->release(path_builder->impl);

  return SKC_ERR_SUCCESS;
}

//
// PATH BODY
//

skc_err
skc_path_begin(skc_path_builder_t path_builder)
{
  SKC_ASSERT_STATE_TRANSITION(SKC_PATH_BUILDER_STATE_READY,
                              SKC_PATH_BUILDER_STATE_BUILDING,
                              path_builder);

  // init path builder counters
  path_builder->line .rem  = 0;
  path_builder->quad .rem  = 0;
  path_builder->cubic.rem  = 0;

  // begin the path
  path_builder->begin(path_builder->impl);

  return SKC_ERR_SUCCESS;
}

skc_err
skc_path_end(skc_path_builder_t path_builder, skc_path_t * path)
{
  SKC_ASSERT_STATE_TRANSITION(SKC_PATH_BUILDER_STATE_BUILDING,
                              SKC_PATH_BUILDER_STATE_READY,
                              path_builder);

  // update path header with proper counts
  path_builder->end(path_builder->impl,path);

  return SKC_ERR_SUCCESS;
}

//
// PATH SEGMENT OPS
//

static
void
skc_path_move_to_1(skc_path_builder_t path_builder,
                   float x0, float y0)
{
  path_builder->curr[0].x = x0;
  path_builder->curr[0].y = y0;
  path_builder->curr[1].x = x0;
  path_builder->curr[1].y = y0;
}

static
void
skc_path_move_to_2(skc_path_builder_t path_builder,
                   float x0, float y0,
                   float x1, float y1)
{
  path_builder->curr[0].x = x0;
  path_builder->curr[0].y = y0;
  path_builder->curr[1].x = x1;
  path_builder->curr[1].y = y1;
}

skc_err
skc_path_move_to(skc_path_builder_t path_builder,
                 float x0, float y0)
{
  skc_path_move_to_1(path_builder,x0,y0);

  return SKC_ERR_SUCCESS;
}

skc_err
skc_path_close(skc_path_builder_t path_builder)
{
  //
  // FIXME -- CLARIFY WHY SUBTLE AUTO-CLOSE BEHAVIORS _DON'T BELONG_
  // IN THE SKIA COMPUTE LAYER
  //
  // OR, BETTER YET, GET RID OF THIS FUNC ENTIRELY
  //
  return SKC_ERR_NOT_IMPLEMENTED;
}

skc_err
skc_path_line_to(skc_path_builder_t path_builder,
                 float x1, float y1)
{
  if (path_builder->line.rem == 0) {
    path_builder->new_line(path_builder->impl);
  }

  --path_builder->line.rem;

  *path_builder->line.coords[0]++ = path_builder->curr[0].x;
  *path_builder->line.coords[1]++ = path_builder->curr[0].y;
  *path_builder->line.coords[2]++ = x1;
  *path_builder->line.coords[3]++ = y1;

  skc_path_move_to_1(path_builder,x1,y1);

  return SKC_ERR_SUCCESS;
}

skc_err
skc_path_quad_to(skc_path_builder_t path_builder,
                 float x1, float y1,
                 float x2, float y2)
{
  if (path_builder->quad.rem == 0) {
    path_builder->new_quad(path_builder->impl);
  }

  --path_builder->quad.rem;

  *path_builder->quad.coords[0]++ = path_builder->curr[0].x;
  *path_builder->quad.coords[1]++ = path_builder->curr[0].y;
  *path_builder->quad.coords[2]++ = x1;
  *path_builder->quad.coords[3]++ = y1;
  *path_builder->quad.coords[4]++ = x2;
  *path_builder->quad.coords[5]++ = y2;

  skc_path_move_to_2(path_builder,x2,y2,x1,y1);

  return SKC_ERR_SUCCESS;
}

skc_err
skc_path_quad_smooth_to(skc_path_builder_t path_builder,
                        float x2, float y2)
{
  float const x1 = path_builder->curr[0].x * 2.0f - path_builder->curr[1].x;
  float const y1 = path_builder->curr[0].y * 2.0f - path_builder->curr[1].y;

  return skc_path_quad_to(path_builder,x1,y1,x2,y2);
}

skc_err
skc_path_cubic_to(skc_path_builder_t path_builder,
                  float x1, float y1,
                  float x2, float y2,
                  float x3, float y3)
{
  if (path_builder->cubic.rem == 0) {
    path_builder->new_cubic(path_builder->impl);
  }

  --path_builder->cubic.rem;

  *path_builder->cubic.coords[0]++ = path_builder->curr[0].x;
  *path_builder->cubic.coords[1]++ = path_builder->curr[0].y;
  *path_builder->cubic.coords[2]++ = x1;
  *path_builder->cubic.coords[3]++ = y1;
  *path_builder->cubic.coords[4]++ = x2;
  *path_builder->cubic.coords[5]++ = y2;
  *path_builder->cubic.coords[6]++ = x3;
  *path_builder->cubic.coords[7]++ = y3;

  skc_path_move_to_2(path_builder,x3,y3,x2,y2);

  return SKC_ERR_SUCCESS;
}

skc_err
skc_path_cubic_smooth_to(skc_path_builder_t path_builder,
                         float x2, float y2,
                         float x3, float y3)
{
  float const x1 = path_builder->curr[0].x * 2.0f - path_builder->curr[1].x;
  float const y1 = path_builder->curr[0].y * 2.0f - path_builder->curr[1].y;

  return skc_path_cubic_to(path_builder,x1,y1,x2,y2,x3,y3);
}

//
// FIXME -- add rational quad and cubic support and move primitives
// like ellipse into an adapter. They do *not* belong in the core API.
//

skc_err
skc_path_ellipse(skc_path_builder_t path_builder,
                 float cx, float cy,
                 float rx, float ry)
{
  //
  // FIXME -- we can implement this with rationals later...
  //

  //
  // Approximate a circle with 4 cubics:
  //
  // http://en.wikipedia.org/wiki/B%C3%A9zier_spline#Approximating_circular_arcs
  //
  skc_path_move_to_1(path_builder, cx, cy + ry);

#define KAPPA_FLOAT 0.55228474983079339840f // moar digits!

  float const kx = rx * KAPPA_FLOAT;
  float const ky = ry * KAPPA_FLOAT;

  skc_err err;

  err = skc_path_cubic_to(path_builder,
                          cx + kx, cy + ry,
                          cx + rx, cy + ky,
                          cx + rx, cy);

  if (err)
    return err;

  err = skc_path_cubic_to(path_builder,
                          cx + rx, cy - ky,
                          cx + kx, cy - ry,
                          cx,      cy - ry);

  if (err)
    return err;

  err = skc_path_cubic_to(path_builder,
                          cx - kx, cy - ry,
                          cx - rx, cy - ky,
                          cx - rx, cy);

  if (err)
    return err;

  err = skc_path_cubic_to(path_builder,
                          cx - rx, cy + ky,
                          cx - kx, cy + ry,
                          cx,      cy + ry);
  return err;
}

//
//
//
