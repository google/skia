/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#include "transform_stack.h"

//
//
//

#include <stdlib.h>
#include <math.h>

//
//
//

#undef NDEBUG
#include <assert.h>

//
//
//

#define TS_TRANSFORM_SUFFIX_EVAL(a)  a

#define TS_TRANSFORM_SUFFIX_CONCAT(func)                                \
  TS_TRANSFORM_SUFFIX_EVAL(func)##TS_TRANSFORM_SUFFIX_EVAL(TS_TRANSFORM_FLOAT_SUFFIX)

//
//
//

#define TS_TRANSFORM_SIN(x)  TS_TRANSFORM_SUFFIX_CONCAT(sin)(x)
#define TS_TRANSFORM_COS(x)  TS_TRANSFORM_SUFFIX_CONCAT(cos)(x)
#define TS_TRANSFORM_TAN(x)  TS_TRANSFORM_SUFFIX_CONCAT(tan)(x)

//
//
//
#define TS_TRANSFORM_ZERO    ((ts_transform_float_t)0.0)
#define TS_TRANSFORM_ONE     ((ts_transform_float_t)1.0)
#define TS_TRANSFORM_RCP(f)  (TS_TRANSFORM_ONE / (f))


//
//
//

union ts_transform_stack_3x3_u
{
  ts_transform_float_t   a8[8];

  struct {
    ts_transform_float_t sx;
    ts_transform_float_t shx;
    ts_transform_float_t tx;

    ts_transform_float_t shy;
    ts_transform_float_t sy;
    ts_transform_float_t ty;

    ts_transform_float_t w0;
    ts_transform_float_t w1;
    // w2 is always 1.0
  };

  struct {
    ts_transform_float_t a;
    ts_transform_float_t b;
    ts_transform_float_t c;

    ts_transform_float_t d;
    ts_transform_float_t e;
    ts_transform_float_t f;

    ts_transform_float_t g;
    ts_transform_float_t h;
    // i is always 1.0
  };
};

//
//
//

struct ts_transform_stack
{
  uint32_t                         size;
  uint32_t                         count;

  ts_transform_weakref_t         * weakrefs;
  union ts_transform_stack_3x3_u * transforms;
};

//
//
//

static
void
ts_transform_stack_resize(struct ts_transform_stack * const ts, uint32_t const size)
{
  ts->size       = size;
  ts->weakrefs   = realloc(ts->weakrefs,  size * sizeof(*ts->weakrefs));
  ts->transforms = realloc(ts->transforms,size * sizeof(*ts->transforms));
}

static
void
ts_transform_stack_ensure(struct ts_transform_stack * const ts)
{
  if (ts->count < ts->size)
    return;

  // increase by 50% and by at least 8
  ts_transform_stack_resize(ts,ts->size + max(ts->size/2,8));
}

//
//
//

struct ts_transform_stack *
ts_transform_stack_create(uint32_t const size)
{
  struct ts_transform_stack * ts = malloc(sizeof(*ts));

  ts->size       = size;
  ts->count      = 0;

  ts->transforms = NULL;
  ts->weakrefs   = NULL;

  ts_transform_stack_resize(ts,size);

  return ts;
}

void
ts_transform_stack_release(struct ts_transform_stack * const ts)
{
  free(ts->transforms);
  free(ts->weakrefs);

  free(ts);
}

//
//
//

uint32_t
ts_transform_stack_save(struct ts_transform_stack * const ts)
{
  return ts->count;
}

void
ts_transform_stack_restore(struct ts_transform_stack * const ts,
                           uint32_t                    const restore)
{
  ts->count = restore;
}

//
//
//

static
union ts_transform_stack_3x3_u *
ts_transform_stack_tos(struct ts_transform_stack * const ts)
{
  return ts->transforms + ts->count - 1;
}

//
//
//

static
void
ts_transform_stack_3x3_u_copy(union ts_transform_stack_3x3_u       * const __restrict dst,
                              union ts_transform_stack_3x3_u const * const __restrict src)
{
  *dst = *src;
}

//
// C = A * B
//
// FIXME -- can save affine vs. projective flags and save a few ops
//

#define TS_TRANSFORM_MULTIPLY(A,B)                                    \
  A->sx  * B->sx   +  A->shx * B->shy  +  A->tx * B->w0,              \
  A->sx  * B->shx  +  A->shx * B->sy   +  A->tx * B->w1,              \
  A->sx  * B->tx   +  A->shx * B->ty   +  A->tx,                      \
  A->shy * B->sx   +  A->sy  * B->shy  +  A->ty * B->w0,              \
  A->shy * B->shx  +  A->sy  * B->sy   +  A->ty * B->w1,              \
  A->shy * B->tx   +  A->sy  * B->ty   +  A->ty,                      \
  A->w0  * B->sx   +  A->w1  * B->shy  +          B->w0,              \
  A->w0  * B->shx  +  A->w1  * B->sy   +          B->w1,              \
  A->w0  * B->tx   +  A->w1  * B->ty   +          TS_TRANSFORM_ONE

//
//
//

#define TS_IS_AFFINE(t) ((t->w0 == TS_TRANSFORM_ZERO) && (t->w1 == TS_TRANSFORM_ZERO))

static
ts_transform_type_e
ts_transform_stack_classify(struct ts_transform_stack * const ts)
{
  union ts_transform_stack_3x3_u const * const t = ts_transform_stack_tos(ts);

  if (TS_IS_AFFINE(t))
    return TS_TRANSFORM_TYPE_AFFINE;
  else
    return TS_TRANSFORM_TYPE_PROJECTIVE;
}

//
//
//

ts_transform_float_t *
ts_transform_stack_top_transform(struct ts_transform_stack * const ts)
{
  return ts_transform_stack_tos(ts)->a8;
}

ts_transform_weakref_t *
ts_transform_stack_top_weakref(struct ts_transform_stack * const ts)
{
  return ts->weakrefs + ts->count - 1;
}

//
//
//

void
ts_transform_stack_dup(struct ts_transform_stack * const ts)
{
  ts_transform_stack_ensure(ts);

  union ts_transform_stack_3x3_u * const tos = ts_transform_stack_tos(ts);

  ts_transform_stack_3x3_u_copy(tos+1,tos);

  ts->weakrefs[ts->count] = ts->weakrefs[ts->count-1];

  ts->count += 1;
}

void
ts_transform_stack_drop(struct ts_transform_stack * const ts)
{
  assert(ts->count >= 1);

  ts->count -= 1;
}

//
//
//

static
void
ts_transform_stack_swap_drop(struct ts_transform_stack * const ts)
{
  assert(ts->count >= 2);

  union ts_transform_stack_3x3_u * const tos = ts_transform_stack_tos(ts);

  ts_transform_stack_3x3_u_copy(tos-1,tos);

  ts->weakrefs[ts->count-2] = ts->weakrefs[ts->count-1];

  ts->count -= 1;
}

//
//
//

static
void
ts_transform_stack_store_matrix_8(struct ts_transform_stack * const ts,
                                  uint32_t                    const idx,
                                  ts_transform_float_t        const sx,
                                  ts_transform_float_t        const shx,
                                  ts_transform_float_t        const tx,
                                  ts_transform_float_t        const shy,
                                  ts_transform_float_t        const sy,
                                  ts_transform_float_t        const ty,
                                  ts_transform_float_t        const w0,
                                  ts_transform_float_t        const w1)
{
  union ts_transform_stack_3x3_u * t = ts->transforms + idx;

  t->sx  = sx;
  t->shx = shx;
  t->tx  = tx;

  t->shy = shy;
  t->sy  = sy;
  t->ty  = ty;

  t->w0  = w0;
  t->w1  = w1;

  ts->weakrefs[idx] = TS_TRANSFORM_WEAKREF_INVALID;
}

//
//
//

static
void
ts_transform_stack_store_matrix(struct ts_transform_stack * const ts,
                                uint32_t                    const idx,
                                ts_transform_float_t        const sx,
                                ts_transform_float_t        const shx,
                                ts_transform_float_t        const tx,
                                ts_transform_float_t        const shy,
                                ts_transform_float_t        const sy,
                                ts_transform_float_t        const ty,
                                ts_transform_float_t        const w0,
                                ts_transform_float_t        const w1,
                                ts_transform_float_t        const w2)
{
  if (w2 == TS_TRANSFORM_ONE)
    {
      ts_transform_stack_store_matrix_8(ts,idx,
                                        sx, shx,tx,
                                        shy,sy, ty,
                                        w0, w1);
    }
  else
    {
      // normalize
      ts_transform_float_t d = TS_TRANSFORM_RCP(w2);

      ts_transform_stack_store_matrix_8(ts,idx,
                                        sx  * d, shx * d, tx * d,
                                        shy * d, sy  * d, ty * d,
                                        w0  * d, w1  * d);
    }
}

//
//
//

static
void
ts_transform_stack_push_matrix_8(struct ts_transform_stack * const ts,
                                 ts_transform_float_t        const sx,
                                 ts_transform_float_t        const shx,
                                 ts_transform_float_t        const tx,
                                 ts_transform_float_t        const shy,
                                 ts_transform_float_t        const sy,
                                 ts_transform_float_t        const ty,
                                 ts_transform_float_t        const w0,
                                 ts_transform_float_t        const w1)
{
  ts_transform_stack_ensure(ts);

  ts_transform_stack_store_matrix_8(ts,ts->count++,
                                    sx, shx,tx,
                                    shy,sy, ty,
                                    w0, w1);
}

//
//
//

void
ts_transform_stack_push_matrix(struct ts_transform_stack * const ts,
                               ts_transform_float_t        const sx,
                               ts_transform_float_t        const shx,
                               ts_transform_float_t        const tx,
                               ts_transform_float_t        const shy,
                               ts_transform_float_t        const sy,
                               ts_transform_float_t        const ty,
                               ts_transform_float_t        const w0,
                               ts_transform_float_t        const w1,
                               ts_transform_float_t        const w2)
{
  if (w2 == TS_TRANSFORM_ONE)
    {
      ts_transform_stack_push_matrix_8(ts,
                                       sx, shx,tx,
                                       shy,sy, ty,
                                       w0, w1);
    }
  else
    {
      // normalize
      ts_transform_float_t d = TS_TRANSFORM_RCP(w2);

      ts_transform_stack_push_matrix_8(ts,
                                       sx  * d, shx * d, tx * d,
                                       shy * d, sy  * d, ty * d,
                                       w0  * d, w1  * d);
    }
}

//
//
//

void
ts_transform_stack_push_identity(struct ts_transform_stack * const ts)
{
  ts_transform_stack_push_matrix_8(ts,
                                   1.0, 0.0, 0.0,
                                   0.0, 1.0, 0.0,
                                   0.0, 0.0);
}

void
ts_transform_stack_push_affine(struct ts_transform_stack * const ts,
                               ts_transform_float_t        const sx,
                               ts_transform_float_t        const shx,
                               ts_transform_float_t        const tx,
                               ts_transform_float_t        const shy,
                               ts_transform_float_t        const sy,
                               ts_transform_float_t        const ty)
{
  ts_transform_stack_push_matrix_8(ts,
                                   sx,  shx, tx,
                                   shy, sy,  ty,
                                   0.0, 0.0);
}

void
ts_transform_stack_push_translate(struct ts_transform_stack * const ts,
                                  ts_transform_float_t        const tx,
                                  ts_transform_float_t        const ty)
{
  ts_transform_stack_push_matrix_8(ts,
                                   1.0, 0.0, tx,
                                   0.0, 1.0, ty,
                                   0.0, 0.0);
}

void
ts_transform_stack_push_scale(struct ts_transform_stack * const ts,
                              ts_transform_float_t        const sx,
                              ts_transform_float_t        const sy)
{
  ts_transform_stack_push_matrix_8(ts,
                                   sx,  0.0, 0.0,
                                   0.0, sy,  0.0,
                                   0.0, 0.0);
}

void
ts_transform_stack_push_shear(struct ts_transform_stack * const ts,
                              ts_transform_float_t        const shx,
                              ts_transform_float_t        const shy)
{
  ts_transform_stack_push_matrix_8(ts,
                                   1.0, shx, 0.0,
                                   shy, 1.0, 0.0,
                                   0.0, 0.0);
}

void
ts_transform_stack_push_skew_x(struct ts_transform_stack * const ts,
                               ts_transform_float_t        const theta)
{
  ts_transform_float_t        const tan_theta = TS_TRANSFORM_TAN(theta); // replace with tanpi if available

  ts_transform_stack_push_matrix_8(ts,
                                   1.0, tan_theta,0.0,
                                   0.0, 1.0,      0.0,
                                   0.0, 0.0);
}

void
ts_transform_stack_push_skew_y(struct ts_transform_stack * const ts,
                               ts_transform_float_t        const theta)
{
  ts_transform_float_t const tan_theta = TS_TRANSFORM_TAN(theta); // replace with tanpi if available

  ts_transform_stack_push_matrix_8(ts,
                                   1.0,       0.0, 0.0,
                                   tan_theta, 1.0, 0.0,
                                   0.0,       0.0);
}

void
ts_transform_stack_push_rotate(struct ts_transform_stack * const ts,
                               ts_transform_float_t        const theta)
{
  ts_transform_float_t const cos_theta = TS_TRANSFORM_COS(theta); // replace with cospi if available
  ts_transform_float_t const sin_theta = TS_TRANSFORM_SIN(theta); // replace with sinpi if available

  ts_transform_stack_push_matrix_8(ts,
                                   cos_theta,-sin_theta, 0.0,
                                   sin_theta, cos_theta, 0.0,
                                   0.0,       0.0);
}

void
ts_transform_stack_push_rotate_xy2(struct ts_transform_stack * const ts,
                                   ts_transform_float_t        const theta,
                                   ts_transform_float_t        const cx,
                                   ts_transform_float_t        const cy,
                                   ts_transform_float_t        const tx,
                                   ts_transform_float_t        const ty)
{
  ts_transform_float_t const cos_theta = TS_TRANSFORM_COS(theta); // replace with cospi if available
  ts_transform_float_t const sin_theta = TS_TRANSFORM_SIN(theta); // replace with sinpi if available

  ts_transform_stack_push_matrix_8(ts,
                                   cos_theta,-sin_theta, tx - (cx * cos_theta) + (cy * sin_theta),
                                   sin_theta, cos_theta, ty - (cx * sin_theta) - (cy * cos_theta),
                                   0.0,       0.0);
}

void
ts_transform_stack_push_rotate_xy(struct ts_transform_stack * const ts,
                                  ts_transform_float_t        const theta,
                                  ts_transform_float_t        const cx,
                                  ts_transform_float_t        const cy)
{
  ts_transform_stack_push_rotate_xy2(ts,theta,cx,cy,cx,cy);
}

void
ts_transform_stack_push_rotate_scale_xy(struct ts_transform_stack * const ts,
                                        ts_transform_float_t        const theta,
                                        ts_transform_float_t        const sx,
                                        ts_transform_float_t        const sy,
                                        ts_transform_float_t        const cx,
                                        ts_transform_float_t        const cy)
{
  ts_transform_float_t const cos_theta = TS_TRANSFORM_COS(theta); // replace with cospi if available
  ts_transform_float_t const sin_theta = TS_TRANSFORM_SIN(theta); // replace with sinpi if available

  ts_transform_stack_push_matrix_8(ts,
                                   sx*cos_theta,-sx*sin_theta, cx - cx*sx*cos_theta + cy*sy*sin_theta,
                                   sy*sin_theta, sy*cos_theta, cy - cy*sy*cos_theta - cx*sx*sin_theta,
                                   0.0,          0.0);
}

//
// See: "Fundamentals of Texture Mapping and Image Warping" by Paul S. Heckbert (1989)
//

#define DET(a,b,c,d)  (a * d - b * c)

#define X(v,i)        v[i*2]
#define Y(v,i)        v[i*2+1]

//
//
//

ts_transform_type_e
ts_transform_stack_adjoint(struct ts_transform_stack * const ts)
{
  union ts_transform_stack_3x3_u * const t = ts_transform_stack_tos(ts);

#if 0
  // save for determinant
  ts_transform_float_t const a = t->a;
  ts_transform_float_t const b = t->b;
  ts_transform_float_t const c = t->c;
#endif

  ts_transform_stack_store_matrix(ts,ts->count-1,

                                  +DET(t->e, t->f, t->h,             TS_TRANSFORM_ONE),
                                  -DET(t->b, t->c, t->h,             TS_TRANSFORM_ONE),
                                  +DET(t->b, t->c, t->e,             t->f),

                                  -DET(t->d, t->f, t->g,             TS_TRANSFORM_ONE),
                                  +DET(t->a, t->c, t->g,             TS_TRANSFORM_ONE),
                                  -DET(t->a, t->c, t->d,             t->f),

                                  +DET(t->d, t->e, t->g,             t->h),
                                  -DET(t->a, t->b, t->g,             t->h),
                                  +DET(t->a, t->b, t->d,             t->e));

#if 0
  // determinant of t
  ts_transform_float_t const det = a * t->a + b * t->d + c * t->g;
#endif

  return ts_transform_stack_classify(ts);
}

//
//
//

ts_transform_type_e
ts_transform_stack_push_unit_to_quad(struct ts_transform_stack * const ts,
                                     ts_transform_float_t        const quad[8])
{
  ts_transform_float_t const x0  = X(quad,0);
  ts_transform_float_t const y0  = Y(quad,0);

  ts_transform_float_t const x1  = X(quad,1);
  ts_transform_float_t const y1  = Y(quad,1);

  ts_transform_float_t const x2  = X(quad,2);
  ts_transform_float_t const y2  = Y(quad,2);

  ts_transform_float_t const x3  = X(quad,3);
  ts_transform_float_t const y3  = Y(quad,3);

  ts_transform_float_t       sx  = x1 - x0;
  ts_transform_float_t       shy = y1 - y0;

  ts_transform_float_t const dx2 = x3 - x2;
  ts_transform_float_t const dy2 = y3 - y2;

  ts_transform_float_t const dx3 = -sx  - dx2;
  ts_transform_float_t const dy3 = -shy - dy2;

  // if both zero then quad_dst is a parallelogram and affine
  if ((dx3 == TS_TRANSFORM_ZERO) && (dy3 == TS_TRANSFORM_ZERO))
    {
      ts_transform_float_t const shx = x2 - x1;
      ts_transform_float_t const sy  = y2 - y1;

      ts_transform_stack_push_matrix_8(ts,
                                       sx,  shx, x0,
                                       shy, sy,  y0,
                                       0.0, 0.0);

      return TS_TRANSFORM_TYPE_AFFINE;
    }
  else
    {
      ts_transform_float_t const dx1    = x1 - x2;
      ts_transform_float_t const dy1    = y1 - y2;

      ts_transform_float_t const wx_den = dx1 * dy2 - dx2 * dy1;

      if (wx_den == TS_TRANSFORM_ZERO)
        return TS_TRANSFORM_TYPE_INVALID;

      ts_transform_float_t const w0_num = dx3 * dy2 - dx2 * dy3;
      ts_transform_float_t const w1_num = dx1 * dy3 - dx3 * dy1;

      ts_transform_float_t const w0     = w0_num / wx_den;
      ts_transform_float_t const w1     = w1_num / wx_den;

      sx                 += w0 * x1;
      ts_transform_float_t const shx    = x3 - x0 + w1 * x3;

      shy                += w0 * y1;
      ts_transform_float_t const sy     = y3 - y0 + w1 * y3;

      ts_transform_stack_push_matrix_8(ts,
                                       sx,  shx, x0,
                                       shy, sy,  y0,
                                       w0,  w1);

      return TS_TRANSFORM_TYPE_PROJECTIVE;
    }
}

//
//
//

ts_transform_type_e
ts_transform_stack_push_quad_to_unit(struct ts_transform_stack * const ts,
                                     float                        const quad[8])
{
  if (ts_transform_stack_push_unit_to_quad(ts,quad) == TS_TRANSFORM_TYPE_INVALID)
    return TS_TRANSFORM_TYPE_INVALID;

  return ts_transform_stack_adjoint(ts);
}

//
//
//

ts_transform_type_e
ts_transform_stack_push_quad_to_quad(struct ts_transform_stack * const ts,
                                     ts_transform_float_t        const quad_src[8],
                                     ts_transform_float_t        const quad_dst[8])
{
  if (ts_transform_stack_push_unit_to_quad(ts,quad_dst) == TS_TRANSFORM_TYPE_INVALID)
    return TS_TRANSFORM_TYPE_INVALID;

  if (ts_transform_stack_push_quad_to_unit(ts,quad_src) == TS_TRANSFORM_TYPE_INVALID)
    return TS_TRANSFORM_TYPE_INVALID;

  ts_transform_stack_multiply(ts);

  return ts_transform_stack_classify(ts);
}

//
//
//

ts_transform_type_e
ts_transform_stack_push_rect_to_quad(struct ts_transform_stack * const ts,
                                     ts_transform_float_t        const x0,
                                     ts_transform_float_t        const y0,
                                     ts_transform_float_t        const x1,
                                     ts_transform_float_t        const y1,
                                     ts_transform_float_t        const quad_dst[8])
{
  if (ts_transform_stack_push_unit_to_quad(ts,quad_dst) == TS_TRANSFORM_TYPE_INVALID)
    return TS_TRANSFORM_TYPE_INVALID;

  ts_transform_stack_push_matrix_8(ts,
                                   TS_TRANSFORM_RCP(x1-x0),
                                   0.0,
                                   -x0,
                                   0.0,
                                   TS_TRANSFORM_RCP(y1-y0),
                                   -y0,
                                   0.0,
                                   0.0);

  ts_transform_stack_multiply(ts);

  return ts_transform_stack_classify(ts);
}

//
// The second matrix on the stack (TOS[-1]) is post-multiplied by the
// top matrix on the stack (TOS[0]).
//
// The result replaces TOS[0] and TOS[-1] is unmodified.
//
// The stack effect of concat is:
//
//   | B |    | A*B |
//   | A |    |  A  |
//   | . | => |  .  |
//   | . |    |  .  |
//   | . |    |  .  |
//
void
ts_transform_stack_concat(struct ts_transform_stack * const ts)
{
  assert(ts->count >= 2);

  // get A and B
  union ts_transform_stack_3x3_u const * const B = ts_transform_stack_tos(ts);
  union ts_transform_stack_3x3_u const * const A = B - 1;

  ts_transform_stack_store_matrix(ts,ts->count-1,TS_TRANSFORM_MULTIPLY(A,B));
}

//
// The second matrix on the stack (TOS[-1]) is post-multiplied by the
// top matrix on the stack (TOS[0]).
//
// The result replaces both matrices.
//
// The stack effect of multiply is:
//
//   | B |    | A*B |
//   | A |    |  .  |
//   | . | => |  .  |
//   | . |    |  .  |
//   | . |    |  .  |
//
void
ts_transform_stack_multiply(struct ts_transform_stack * const ts)
{
  assert(ts->count >= 2);

  // get A and B
  union ts_transform_stack_3x3_u const * const B = ts_transform_stack_tos(ts);
  union ts_transform_stack_3x3_u const * const A = B - 1;

  ts_transform_stack_store_matrix(ts,ts->count-- - 2,TS_TRANSFORM_MULTIPLY(A,B));
}

//
//
//

void
ts_transform_stack_transform_xy(struct ts_transform_stack * const ts,
                                ts_transform_float_t        const x,
                                ts_transform_float_t        const y,
                                ts_transform_float_t      * const xp,
                                ts_transform_float_t      * const yp)
{
  union ts_transform_stack_3x3_u const * const t = ts_transform_stack_tos(ts);

  *xp = x * t->sx  + y * t->shx + t->tx;
  *yp = x * t->shy + y * t->sy  + t->ty;

  if (!TS_IS_AFFINE(t))
    {
      ts_transform_float_t const d = TS_TRANSFORM_RCP(x * t->w0 + y * t->w1 + TS_TRANSFORM_ONE);

      *xp *= d;
      *yp *= d;
    }
}

//
// test it!
//

#ifdef TS_DEBUG

#include <stdio.h>

#define TS_DEBUG_SCALE 32.0

//
//
//

void
ts_transform_stack_tos_debug(struct ts_transform_stack * const ts)
{
  union ts_transform_stack_3x3_u const * const t = ts_transform_stack_tos(ts);

  printf("{ { %13.5f, %13.5f, %13.5f },\n"
         "  { %13.5f, %13.5f, %13.5f },\n"
         "  { %13.5f, %13.5f, %13.5f } }\n",
         t->a8[0],
         t->a8[1],
         t->a8[2],
         t->a8[3],
         t->a8[4],
         t->a8[5],
         t->a8[6],
         t->a8[7],
         TS_TRANSFORM_ONE);
}

//
//
//

void
ts_debug(struct ts_transform_stack * const ts,
         ts_transform_float_t        const quad[8])
{
  ts_transform_stack_tos_debug(ts);

  for (int ii=0; ii<8; ii+=2)
    {
      ts_transform_float_t xp,yp;

      ts_transform_stack_transform_xy(ts,
                                      quad[ii],quad[ii+1],
                                      &xp,&yp);

      printf("( %13.2f, %13.2f ) \t-> ( %13.2f, %13.2f )\n",
             xp,yp,xp/TS_DEBUG_SCALE,yp/TS_DEBUG_SCALE);
    }
}

//
//
//

int
main(int argc, char * argv[])
{
  struct ts_transform_stack * const ts = ts_transform_stack_create(32);

  ts_transform_float_t const w = 1000;
  ts_transform_float_t const h = 1000;

#if 1
  ts_transform_stack_push_scale(ts,TS_DEBUG_SCALE,TS_DEBUG_SCALE);

  // OpenGL'ism
  ts_transform_stack_push_affine(ts,
                                 1.0f, 0.0f,0.0f,
                                 0.0f,-1.0f,h);
  // multiply
  ts_transform_stack_concat(ts);
#else
  ts_transform_stack_push_identity(ts);
#endif

  uint32_t const restore = ts_transform_stack_save(ts);

  //
  //
  //
  ts_transform_float_t const quad_src[8] = { 0.0f,0.0f,
                                             w,   0.0f,
                                             w,   h,
                                             0.0f,h };

  ts_transform_float_t const quad_dst[8] = { 300.0f,   0.0f,
                                             w-300.0f, 0.0f,
                                             w,        h,
                                             0.0f,     h };

  ts_transform_float_t const quad_tst[8] = { 50,   50,
                                             1550, 50,
                                             1550, 1550,
                                             50,   1550 };
  //
  // RECT TO QUAD
  //
  printf("type = %d\n",
         ts_transform_stack_push_rect_to_quad(ts,
                                              0.0, 0.0,
                                              w,   h,
                                              quad_dst));
  ts_transform_stack_concat(ts);

  ts_debug(ts,quad_src);

  //
  // QUAD TO QUAD
  //
  ts_transform_stack_restore(ts,restore);

  printf("type = %d\n",
         ts_transform_stack_push_quad_to_quad(ts,
                                              quad_src,
                                              quad_dst));
  ts_transform_stack_concat(ts);

  ts_debug(ts,quad_src);

  //
  // DIRECT
  //
  ts_transform_stack_restore(ts,restore);

  ts_transform_stack_push_matrix(ts,
                                 0.87004626f, -0.35519487f,   72.14745f,
                                 0.0f,         0.2600208f,    86.16314f,
                                 0.0f,        -0.0029599573f, 1.0f);

  ts_transform_stack_concat(ts);

  ts_transform_float_t const quad_foo[8] = { -10,  10,
                                             130,  10,
                                             130,  110,
                                             -10,  110 };

  ts_debug(ts,quad_foo);

  return EXIT_SUCCESS;
}

#endif

//
//
//
