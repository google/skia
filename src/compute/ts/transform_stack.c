/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#include <stdlib.h>
#include <assert.h>
#include <math.h>

//
//
//

#include "transform_stack.h"

//
//
//

#define SKC_TRANSFORM_SUFFIX_EVAL(a)      a

#define SKC_TRANSFORM_SUFFIX_CONCAT(func)                               \
  SKC_TRANSFORM_SUFFIX_EVAL(func)##SKC_TRANSFORM_SUFFIX_EVAL(SKC_TRANSFORM_FLOAT_SUFFIX)

//
//
//

#define SKC_TRANSFORM_SIN(x)  SKC_TRANSFORM_SUFFIX_CONCAT(sin)(x)
#define SKC_TRANSFORM_COS(x)  SKC_TRANSFORM_SUFFIX_CONCAT(cos)(x)
#define SKC_TRANSFORM_TAN(x)  SKC_TRANSFORM_SUFFIX_CONCAT(tan)(x)
#define SKC_TRANSFORM_RCP(x)  (SKC_TRANSFORM_SUFFIX_CONCAT(1.0) / (x))

//
//
//

union skc_transform_stack_3x3_u
{
  skc_transform_float_t   a9[9];

  struct {
    skc_transform_float_t sx;
    skc_transform_float_t shx;
    skc_transform_float_t tx;

    skc_transform_float_t shy;
    skc_transform_float_t sy;
    skc_transform_float_t ty;

    skc_transform_float_t w0;
    skc_transform_float_t w1;
    skc_transform_float_t w2;
  };
};


struct skc_transform_stack
{
  uint32_t                          size;
  uint32_t                          count;

  skc_transform_weakref_t         * weakrefs;
  union skc_transform_stack_3x3_u * transforms;
};

//
//
//

static
void
skc_transform_stack_3x3_u_copy(union skc_transform_stack_3x3_u       * const __restrict dst,
                               union skc_transform_stack_3x3_u const * const __restrict src)
{
  for (int32_t ii=0; ii<9; ii++)
    dst->a9[ii] = src->a9[ii];
}

//
// C = A * B
//

static
void
skc_transform_stack_3x3_u_multiply(union skc_transform_stack_3x3_u       * const __restrict C,
                                   union skc_transform_stack_3x3_u const * const __restrict A,
                                   union skc_transform_stack_3x3_u const * const __restrict B)
{
  C->sx  = A->sx  * B->sx   +  A->shx * B->shy  +  A->tx * B->w0;
  C->shx = A->sx  * B->shx  +  A->shx * B->sy   +  A->tx * B->w1;
  C->tx  = A->sx  * B->tx   +  A->shx * B->ty   +  A->tx * B->w2;

  C->shy = A->shy * B->sx   +  A->sy  * B->shy  +  A->ty * B->w0;
  C->sy  = A->shy * B->shx  +  A->sy  * B->sy   +  A->ty * B->w1;
  C->ty  = A->shy * B->tx   +  A->sy  * B->ty   +  A->ty * B->w2;

  C->w0  = A->w0  * B->sx   +  A->w1  * B->shy  +  A->w2 * B->w0;
  C->w1  = A->w0  * B->shx  +  A->w1  * B->sy   +  A->w2 * B->w1;
  C->w2  = A->w0  * B->tx   +  A->w1  * B->ty   +  A->w2 * B->w2;
}

//
//
//

static
void
skc_transform_stack_resize(struct skc_transform_stack * const ts, uint32_t const size)
{
  ts->size       = size;
  ts->weakrefs   = realloc(ts->weakrefs,  size * sizeof(*ts->weakrefs));
  ts->transforms = realloc(ts->transforms,size * sizeof(*ts->transforms));
}

static
void
skc_transform_stack_ensure(struct skc_transform_stack * const ts)
{
  if (ts->count < ts->size)
    return;

  // increase by 50% and by at least 8
  skc_transform_stack_resize(ts,ts->size + max(ts->size/2,8));
}

//
//
//

struct skc_transform_stack *
skc_transform_stack_create(uint32_t const size)
{
  struct skc_transform_stack * ts = malloc(sizeof(*ts));

  ts->size       = size;
  ts->count      = 0;

  ts->transforms = NULL;
  ts->weakrefs   = NULL;

  skc_transform_stack_resize(ts,size);

  return ts;
}

void
skc_transform_stack_release(struct skc_transform_stack * const ts)
{
  free(ts->transforms);
  free(ts->weakrefs);

  free(ts);
}

//
//
//

uint32_t
skc_transform_stack_save(struct skc_transform_stack * const ts)
{
  return ts->count;
}

void
skc_transform_stack_restore(struct skc_transform_stack * const ts,
                            uint32_t                     const restore)
{
  ts->count = restore;
}

//
//
//

static
union skc_transform_stack_3x3_u *
skc_transform_stack_tos(struct skc_transform_stack * const ts)
{
  return ts->transforms + ts->count - 1;
}

//
//
//

#if 0

void
skc_transform_to_f32a8(float f32a8[8], skc_transform_float_t const a9[9])
{
  if (a9[8] == 1.0)
    {
      f32a8[0] = (float)a9[0];
      f32a8[1] = (float)a9[1];
      f32a8[2] = (float)a9[2];
      f32a8[3] = (float)a9[3];
      f32a8[4] = (float)a9[4];
      f32a8[5] = (float)a9[5];
      f32a8[6] = (float)a9[6];
      f32a8[7] = (float)a9[7];
    }
  else
    {
      skc_transform_float_t const rcp = 1.0 / a9[8];

      f32a8[0] = (float)(a9[0] * rcp);
      f32a8[1] = (float)(a9[1] * rcp);
      f32a8[2] = (float)(a9[2] * rcp);
      f32a8[3] = (float)(a9[3] * rcp);
      f32a8[4] = (float)(a9[4] * rcp);
      f32a8[5] = (float)(a9[5] * rcp);
      f32a8[6] = (float)(a9[6] * rcp);
      f32a8[7] = (float)(a9[7] * rcp);
    }
}

#endif

//
//
//

skc_transform_float_t *
skc_transform_stack_top_transform(struct skc_transform_stack * const ts)
{
  return skc_transform_stack_tos(ts)->a9;
}

skc_weakref_t *
skc_transform_stack_top_weakref(struct skc_transform_stack * const ts)
{
  return ts->weakrefs + ts->count - 1;
}

//
//
//

void
skc_transform_stack_drop(struct skc_transform_stack * const ts)
{
  assert(ts->count >= 1);

  ts->count -= 1;
}

void
skc_transform_stack_dup(struct skc_transform_stack * const ts)
{
  skc_transform_stack_ensure(ts);

  union skc_transform_stack_3x3_u * const tos = skc_transform_stack_tos(ts);

  skc_transform_stack_3x3_u_copy(tos+1,tos);

  ts->weakrefs[ts->count] = ts->weakrefs[ts->count-1];

  ts->count += 1;
}

//
//
//

void
skc_transform_stack_push_matrix(struct skc_transform_stack * const ts,
                                skc_transform_float_t        const sx,
                                skc_transform_float_t        const shx,
                                skc_transform_float_t        const tx,
                                skc_transform_float_t        const shy,
                                skc_transform_float_t        const sy,
                                skc_transform_float_t        const ty,
                                skc_transform_float_t        const w0,
                                skc_transform_float_t        const w1,
                                skc_transform_float_t        const w2)
{
  skc_transform_stack_ensure(ts);

  union skc_transform_stack_3x3_u * t = ts->transforms + ts->count;

  t->sx  = sx;
  t->shx = shx;
  t->tx  = tx;

  t->shy = shy;
  t->sy  = sy;
  t->ty  = ty;

  t->w0  = w0;
  t->w1  = w1;
  t->w2  = w2;

  ts->weakrefs[ts->count++] = SKC_WEAKREF_INVALID;
}

void
skc_transform_stack_push_identity(struct skc_transform_stack * const ts)
{
  skc_transform_stack_push_matrix(ts,
                                  1.0, 0.0, 0.0,
                                  0.0, 1.0, 0.0,
                                  0.0, 0.0, 1.0);
}

void
skc_transform_stack_push_affine(struct skc_transform_stack * const ts,
                                skc_transform_float_t        const sx,
                                skc_transform_float_t        const shx,
                                skc_transform_float_t        const tx,
                                skc_transform_float_t        const shy,
                                skc_transform_float_t        const sy,
                                skc_transform_float_t        const ty)
{
  skc_transform_stack_push_matrix(ts,
                                  sx,  shx, tx,
                                  shy, sy,  ty,
                                  0.0, 0.0, 1.0);
}

void
skc_transform_stack_push_translate(struct skc_transform_stack * const ts,
                                   skc_transform_float_t        const tx,
                                   skc_transform_float_t        const ty)
{
  skc_transform_stack_push_matrix(ts,
                                  1.0, 0.0, tx,
                                  0.0, 1.0, ty,
                                  0.0, 0.0, 1.0);
}

void
skc_transform_stack_push_scale(struct skc_transform_stack * const ts,
                               skc_transform_float_t        const sx,
                               skc_transform_float_t        const sy)
{
  skc_transform_stack_push_matrix(ts,
                                  sx,  0.0, 0.0,
                                  0.0, sy,  0.0,
                                  0.0, 0.0, 1.0);
}

void
skc_transform_stack_push_shear(struct skc_transform_stack * const ts,
                               skc_transform_float_t        const shx,
                               skc_transform_float_t        const shy)
{
  skc_transform_stack_push_matrix(ts,
                                  1.0, shx, 0.0,
                                  shy, 1.0, 0.0,
                                  0.0, 0.0, 1.0);
}

void
skc_transform_stack_push_skew_x(struct skc_transform_stack * const ts,
                                skc_transform_float_t        const theta)
{
  skc_transform_float_t        const tan_theta = SKC_TRANSFORM_TAN(theta); // replace with tanpi if available

  skc_transform_stack_push_matrix(ts,
                                  1.0, tan_theta,0.0,
                                  0.0, 1.0,      0.0,
                                  0.0, 0.0,      1.0);
}

void
skc_transform_stack_push_skew_y(struct skc_transform_stack * const ts,
                                skc_transform_float_t        const theta)
{
  skc_transform_float_t const tan_theta = SKC_TRANSFORM_TAN(theta); // replace with tanpi if available

  skc_transform_stack_push_matrix(ts,
                                  1.0,       0.0, 0.0,
                                  tan_theta, 1.0, 0.0,
                                  0.0,       0.0, 1.0);
}

void
skc_transform_stack_push_rotate(struct skc_transform_stack * const ts,
                                skc_transform_float_t        const theta)
{
  skc_transform_float_t const cos_theta = SKC_TRANSFORM_COS(theta); // replace with cospi if available
  skc_transform_float_t const sin_theta = SKC_TRANSFORM_SIN(theta); // replace with sinpi if available

  skc_transform_stack_push_matrix(ts,
                                  cos_theta,-sin_theta, 0.0,
                                  sin_theta, cos_theta, 0.0,
                                  0.0,       0.0,       1.0);
}

void
skc_transform_stack_push_rotate_xy2(struct skc_transform_stack * const ts,
                                    skc_transform_float_t        const theta,
                                    skc_transform_float_t        const cx,
                                    skc_transform_float_t        const cy,
                                    skc_transform_float_t        const tx,
                                    skc_transform_float_t        const ty)
{
  skc_transform_float_t const cos_theta = SKC_TRANSFORM_COS(theta); // replace with cospi if available
  skc_transform_float_t const sin_theta = SKC_TRANSFORM_SIN(theta); // replace with sinpi if available

  skc_transform_stack_push_matrix(ts,
                                  cos_theta,-sin_theta, tx - (cx * cos_theta) + (cy * sin_theta),
                                  sin_theta, cos_theta, ty - (cx * sin_theta) - (cy * cos_theta),
                                  0.0,       0.0,       1.0);
}

void
skc_transform_stack_push_rotate_xy(struct skc_transform_stack * const ts,
                                   skc_transform_float_t        const theta,
                                   skc_transform_float_t        const cx,
                                   skc_transform_float_t        const cy)
{
  skc_transform_stack_push_rotate_xy2(ts,theta,cx,cy,cx,cy);
}

void
skc_transform_stack_push_rotate_scale_xy(struct skc_transform_stack * const ts,
                                         skc_transform_float_t        const theta,
                                         skc_transform_float_t        const sx,
                                         skc_transform_float_t        const sy,
                                         skc_transform_float_t        const cx,
                                         skc_transform_float_t        const cy)
{
  skc_transform_float_t const cos_theta = SKC_TRANSFORM_COS(theta); // replace with cospi if available
  skc_transform_float_t const sin_theta = SKC_TRANSFORM_SIN(theta); // replace with sinpi if available

  skc_transform_stack_push_matrix(ts,
                                  sx*cos_theta,-sx*sin_theta, cx - cx*sx*cos_theta + cy*sy*sin_theta,
                                  sy*sin_theta, sy*cos_theta, cy - cy*sy*cos_theta - cx*sx*sin_theta,
                                  0.0,          0.0,          1.0);
}

//
// See: https://www.ldv.ei.tum.de/fileadmin/w00bfa/www/content_uploads/Vorlesung_3.2_SpatialTransformations.pdf
//

#define DET4(m,a,b,c,d) (m->a9[a] * m->a9[d] - m->a9[b] * m->a9[c])
#define DET3(m,a,b,c)   (m->a9[a]            - m->a9[b] * m->a9[c])

#define DIAG3(m,a,b,c)  (m->a9[a] * m->a9[b] * m->a9[c])
#define DIAG2(m,a,b)    (m->a9[a] * m->a9[b])

#define X(v,i)          v[i*2]
#define Y(v,i)          v[i*2+1]

//
//
//

bool
skc_transform_stack_push_quad_to_unit(struct skc_transform_stack * const ts,
                                      const float quad_src[8])
{
  if (!skc_transform_stack_push_unit_to_quad(ts,quad_src))
    return false;

  union skc_transform_stack_3x3_u * const T = skc_transform_stack_tos(ts);

#if 0
  //
  // dividing by the determinant (just w2?)
  //
  skc_transform_float_t const det =
    DIAG2(T,0,4)   +
    DIAG3(T,3,7,2) +
    DIAG3(T,6,1,5) -
    DIAG3(T,6,4,2) -
    DIAG2(T,3,1)   -
    DIAG3(T,0,7,5);

  if (det == 0.0)
    return false;

  skc_transform_float_t const rcp = 1.0 / det;

  const union skc_transform_stack_3x3_u A =
    {
      +DET3(T,4,5,7)   * rcp,
      -DET3(T,1,2,7)   * rcp,
      +DET4(T,1,2,4,5) * rcp,

      -DET3(T,3,5,6)   * rcp,
      +DET3(T,0,2,6)   * rcp,
      -DET4(T,0,2,3,5) * rcp,

      +DET4(T,3,4,6,7) * rcp,
      -DET4(T,0,1,6,7) * rcp,
      +DET4(T,0,1,3,4) * rcp
    };

#else // just the adjoint can result in large values

  const union skc_transform_stack_3x3_u A =
    {
      +DET3(T,4,5,7),
      -DET3(T,1,2,7),
      +DET4(T,1,2,4,5),

      -DET3(T,3,5,6),
      +DET3(T,0,2,6),
      -DET4(T,0,2,3,5),

      +DET4(T,3,4,6,7),
      -DET4(T,0,1,6,7),
      +DET4(T,0,1,3,4)
    };

#endif

  skc_transform_stack_3x3_u_copy(T,&A);

  return true;
}

//
//
//

bool
skc_transform_stack_push_unit_to_quad(struct skc_transform_stack * const ts,
                                      const float quad_dst[8])
{
  skc_transform_float_t const x0  = (skc_transform_float_t)X(quad_dst,0);
  skc_transform_float_t const y0  = (skc_transform_float_t)Y(quad_dst,0);

  skc_transform_float_t const x1  = (skc_transform_float_t)X(quad_dst,1);
  skc_transform_float_t const y1  = (skc_transform_float_t)Y(quad_dst,1);

  skc_transform_float_t const x2  = (skc_transform_float_t)X(quad_dst,2);
  skc_transform_float_t const y2  = (skc_transform_float_t)Y(quad_dst,2);

  skc_transform_float_t const x3  = (skc_transform_float_t)X(quad_dst,3);
  skc_transform_float_t const y3  = (skc_transform_float_t)Y(quad_dst,3);

  skc_transform_float_t       sx  = x1 - x0;
  skc_transform_float_t       shy = y1 - y0;

  skc_transform_float_t const dx2 = x3 - x2;
  skc_transform_float_t const dy2 = y3 - y2;

  skc_transform_float_t const dx3 = -sx  - dx2;
  skc_transform_float_t const dy3 = -shy - dy2;

  // if both zero then quad_dst is a parallelogram and affine
  if ((dx3 == 0.0) && (dy3 == 0.0))
    {
      skc_transform_float_t const shx = x2 - x1;
      skc_transform_float_t const sy  = y2 - y1;

      skc_transform_stack_push_matrix(ts,
                                      sx,  shx, x0,
                                      shy, sy,  y0,
                                      0.0, 0.0, 1.0);
    }
  else
    {
      skc_transform_float_t const dx1    = x1 - x2;
      skc_transform_float_t const dy1    = y1 - y2;

      skc_transform_float_t const wx_den = dx1 * dy2 - dx2 * dy1;

      if (wx_den == 0.0)
        return false;

      skc_transform_float_t const w0_num = dx3 * dy2 - dx2 * dy3;
      skc_transform_float_t const w1_num = dx1 * dy3 - dx3 * dy1;

      skc_transform_float_t const w0     = w0_num / wx_den;
      skc_transform_float_t const w1     = w1_num / wx_den;

      sx                 += w0 * x1;
      skc_transform_float_t const shx    = x3 - x0 + w1 * x3;

      shy                += w0 * y1;
      skc_transform_float_t const sy     = y3 - y0 + w1 * y3;

      skc_transform_stack_push_matrix(ts,
                                      sx,  shx, x0,
                                      shy, sy,  y0,
                                      w0,  w1,  1.0);
    }

  return true;
}

//
//
//

bool
skc_transform_stack_push_quad_to_quad(struct skc_transform_stack * const ts,
                                      const float quad_src[8],
                                      const float quad_dst[8])
{
  if (skc_transform_stack_push_quad_to_unit(ts,quad_src) == false)
    return false;

  if (skc_transform_stack_push_unit_to_quad(ts,quad_dst) == false)
    return false;

  union skc_transform_stack_3x3_u * const U2Q = skc_transform_stack_tos(ts);
  union skc_transform_stack_3x3_u * const Q2U = U2Q - 1;
  union skc_transform_stack_3x3_u         Q2Q;

  // pre-multiply
  skc_transform_stack_3x3_u_multiply(&Q2Q,U2Q,Q2U);

  // drop TOS
  skc_transform_stack_drop(ts);

  // overwrite TOS
  skc_transform_stack_3x3_u_copy(Q2U,&Q2Q);

  return true;
}

//
//
//

bool
skc_transform_stack_push_rect_to_quad(struct skc_transform_stack * const ts,
                                      const float x0,
                                      const float y0,
                                      const float x1,
                                      const float y1,
                                      const float quad_dst[8])
{
  if (skc_transform_stack_push_unit_to_quad(ts,quad_dst) == false)
    return false;

  const union skc_transform_stack_3x3_u R2U =
    {
      SKC_TRANSFORM_RCP((skc_transform_float_t)(x1-x0)),
      0.0f,
      (skc_transform_float_t)-x0,

      0.0f,
      SKC_TRANSFORM_RCP((skc_transform_float_t)(y1-y0)),
      (skc_transform_float_t)-y0,

      0.0f,
      0.0f,
      1.0,
    };

  union skc_transform_stack_3x3_u * const U2Q = skc_transform_stack_tos(ts);
  union skc_transform_stack_3x3_u         Q2Q;

  // pre-multiply
  skc_transform_stack_3x3_u_multiply(&Q2Q,U2Q,&R2U);

  // overwrite TOS
  skc_transform_stack_3x3_u_copy(U2Q,&Q2Q);

  return true;
}

//
//
//

void
skc_transform_stack_concat(struct skc_transform_stack * const ts)
{
  //
  // Transformation stack matrices are _post-multiplied_ and the top
  // of stack is replaced with the product:
  //
  //   TOS' = TOS[-1] * TOS
  //
  // -or-
  //
  //   C = A * B
  //   B = C
  //
  if (ts->count <= 1)
    return;

  // get A and B
  union       skc_transform_stack_3x3_u * const B = skc_transform_stack_tos(ts);
  const union skc_transform_stack_3x3_u * const A = B - 1;

  union skc_transform_stack_3x3_u C;

  // post multiply
  skc_transform_stack_3x3_u_multiply(&C,A,B);

  // overwrite TOS
  skc_transform_stack_3x3_u_copy(B,&C);

  // invalidate TOS
  *skc_transform_stack_top_weakref(ts) = SKC_WEAKREF_INVALID;
}

//
//
//

void
skc_transform_stack_transform_affine(struct skc_transform_stack * const ts,
                                     skc_transform_float_t        const x_pre,
                                     skc_transform_float_t        const y_pre,
                                     skc_transform_float_t      * const x_post,
                                     skc_transform_float_t      * const y_post)
{
  union skc_transform_stack_3x3_u const * const t = skc_transform_stack_tos(ts);

  *x_post = x_pre * t->sx  + y_pre * t->shx + t->tx;
  *y_post = x_pre * t->shy + y_pre * t->sy  + t->ty;
}

//
//
//
