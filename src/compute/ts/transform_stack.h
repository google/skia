/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

//
//
//

#pragma once

//
//
//

#include <stdint.h>

//
//
//

struct ts_transform_stack;

//
//
//

#if 1
typedef float  ts_transform_float_t;
#define TS_TRANSFORM_FLOAT_SUFFIX f
#else
typedef double ts_transform_float_t;
#define TS_TRANSFORM_FLOAT_SUFFIX
#endif

//
//
//

typedef uint64_t ts_transform_weakref_t;

#define TS_TRANSFORM_WEAKREF_INVALID UINT64_MAX;

//
//
//

typedef enum ts_transform_type
{
  TS_TRANSFORM_TYPE_INVALID,
  TS_TRANSFORM_TYPE_AFFINE,
  TS_TRANSFORM_TYPE_PROJECTIVE
} ts_transform_type_e;

//
//
//

struct ts_transform_stack *
ts_transform_stack_create(const uint32_t size);

void
ts_transform_stack_release(struct ts_transform_stack * const ts);

//
//
//

uint32_t
ts_transform_stack_save(struct ts_transform_stack * const ts);

void
ts_transform_stack_restore(struct ts_transform_stack * const ts, uint32_t const restore);

//
//
//

ts_transform_float_t *
ts_transform_stack_top_transform(struct ts_transform_stack * const ts);

ts_transform_weakref_t *
ts_transform_stack_top_weakref(struct ts_transform_stack * const ts);

//
//
//

void
ts_transform_stack_dup(struct ts_transform_stack * const ts);

void
ts_transform_stack_drop(struct ts_transform_stack * const ts);

//
//
//

void
ts_transform_stack_transform_xy(struct ts_transform_stack * const ts,
                                ts_transform_float_t        const x,
                                ts_transform_float_t        const y,
                                ts_transform_float_t      * const xp,
                                ts_transform_float_t      * const yp);

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
                               ts_transform_float_t        const w2);

void
ts_transform_stack_push_identity(struct ts_transform_stack * const ts);

void
ts_transform_stack_push_affine(struct ts_transform_stack * const ts,
                               ts_transform_float_t        const sx,
                               ts_transform_float_t        const shx,
                               ts_transform_float_t        const tx,
                               ts_transform_float_t        const shy,
                               ts_transform_float_t        const sy,
                               ts_transform_float_t        const ty);

void
ts_transform_stack_push_translate(struct ts_transform_stack * const ts,
                                  ts_transform_float_t        const tx,
                                  ts_transform_float_t        const ty);

void
ts_transform_stack_push_scale(struct ts_transform_stack * const ts,
                              ts_transform_float_t        const sx,
                              ts_transform_float_t        const sy);

void
ts_transform_stack_push_shear(struct ts_transform_stack * const ts,
                              ts_transform_float_t        const shx,
                              ts_transform_float_t        const shy);


void
ts_transform_stack_push_skew_x(struct ts_transform_stack * const ts,
                               ts_transform_float_t        const theta);

void
ts_transform_stack_push_skew_y(struct ts_transform_stack * const ts,
                               ts_transform_float_t        const theta);

void
ts_transform_stack_push_rotate(struct ts_transform_stack * const ts,
                               ts_transform_float_t        const theta);

void
ts_transform_stack_push_rotate_xy2(struct ts_transform_stack * const ts,
                                   ts_transform_float_t        const theta,
                                   ts_transform_float_t        const cx,
                                   ts_transform_float_t        const cy,
                                   ts_transform_float_t        const tx,
                                   ts_transform_float_t        const ty);

void
ts_transform_stack_push_rotate_xy(struct ts_transform_stack * const ts,
                                  ts_transform_float_t        const theta,
                                  ts_transform_float_t        const cx,
                                  ts_transform_float_t        const cy);

void
ts_transform_stack_push_rotate_scale_xy(struct ts_transform_stack * const ts,
                                        ts_transform_float_t        const theta,
                                        ts_transform_float_t        const sx,
                                        ts_transform_float_t        const sy,
                                        ts_transform_float_t        const cx,
                                        ts_transform_float_t        const cy);
//
// Quadrilateral coordinates are ts_transform_float_t2 structs:
//
//   float2[4] = { xy0, xy1, xy2, xy3 }
//
// -or-
//
//   float[8]  = { x0, y0, x1, y1, x2, y2, x3, y3 };
//

ts_transform_type_e
ts_transform_stack_push_quad_to_unit(struct ts_transform_stack * const ts,
                                     ts_transform_float_t        const quad[8]);

ts_transform_type_e
ts_transform_stack_push_unit_to_quad(struct ts_transform_stack * const ts,
                                     ts_transform_float_t        const quad[8]);

ts_transform_type_e
ts_transform_stack_push_quad_to_quad(struct ts_transform_stack * const ts,
                                     ts_transform_float_t        const quad_src[8],
                                     ts_transform_float_t        const quad_dst[8]);

ts_transform_type_e
ts_transform_stack_push_rect_to_quad(struct ts_transform_stack * const ts,
                                     ts_transform_float_t        const x0,
                                     ts_transform_float_t        const y0,
                                     ts_transform_float_t        const x1,
                                     ts_transform_float_t        const y1,
                                     ts_transform_float_t        const quad_dst[8]);

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
ts_transform_stack_concat(struct ts_transform_stack * const ts);

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
ts_transform_stack_multiply(struct ts_transform_stack * const ts);

//
//
//
