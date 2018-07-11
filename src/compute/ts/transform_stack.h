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

#include <skc/skc.h>

//
//
//

#if 1
typedef float  skc_transform_float_t;
#define SKC_TRANSFORM_FLOAT_SUFFIX f
#else
typedef double skc_transform_float_t;
#define SKC_TRANSFORM_FLOAT_SUFFIX
#endif

//
//
//

struct skc_transform_stack;

//
//
//

struct skc_transform_stack *
skc_transform_stack_create(const uint32_t size);

void
skc_transform_stack_release(struct skc_transform_stack * const ts);

//
//
//

uint32_t
skc_transform_stack_save(struct skc_transform_stack * const ts);

void
skc_transform_stack_restore(struct skc_transform_stack * const ts, uint32_t const restore);

//
//
//

skc_transform_float_t *
skc_transform_stack_top_transform(struct skc_transform_stack * const ts);

skc_transform_weakref_t *
skc_transform_stack_top_weakref(struct skc_transform_stack * const ts);

//
//
//

void
skc_transform_stack_drop(struct skc_transform_stack * const ts);

void
skc_transform_stack_dup(struct skc_transform_stack * const ts);

//
//
//

void
skc_transform_stack_transform_affine(struct skc_transform_stack * const ts,
                                     skc_transform_float_t        const x_pre,
                                     skc_transform_float_t        const y_pre,
                                     skc_transform_float_t      * const x_post,
                                     skc_transform_float_t      * const y_post);

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
                                skc_transform_float_t        const w2);

void
skc_transform_stack_push_identity(struct skc_transform_stack * const ts);

void
skc_transform_stack_push_affine(struct skc_transform_stack * const ts,
                                skc_transform_float_t        const sx,
                                skc_transform_float_t        const shx,
                                skc_transform_float_t        const tx,
                                skc_transform_float_t        const shy,
                                skc_transform_float_t        const sy,
                                skc_transform_float_t        const ty);

void
skc_transform_stack_push_translate(struct skc_transform_stack * const ts,
                                   skc_transform_float_t        const tx,
                                   skc_transform_float_t        const ty);

void
skc_transform_stack_push_scale(struct skc_transform_stack * const ts,
                               skc_transform_float_t        const sx,
                               skc_transform_float_t        const sy);

void
skc_transform_stack_push_shear(struct skc_transform_stack * const ts,
                               skc_transform_float_t        const shx,
                               skc_transform_float_t        const shy);


void
skc_transform_stack_push_skew_x(struct skc_transform_stack * const ts,
                                skc_transform_float_t        const theta);

void
skc_transform_stack_push_skew_y(struct skc_transform_stack * const ts,
                                skc_transform_float_t        const theta);

void
skc_transform_stack_push_rotate(struct skc_transform_stack * const ts,
                                skc_transform_float_t        const theta);

void
skc_transform_stack_push_rotate_xy2(struct skc_transform_stack * const ts,
                                    skc_transform_float_t        const theta,
                                    skc_transform_float_t        const cx,
                                    skc_transform_float_t        const cy,
                                    skc_transform_float_t        const tx,
                                    skc_transform_float_t        const ty);

void
skc_transform_stack_push_rotate_xy(struct skc_transform_stack * const ts,
                                   skc_transform_float_t        const theta,
                                   skc_transform_float_t        const cx,
                                   skc_transform_float_t        const cy);

void
skc_transform_stack_push_rotate_scale_xy(struct skc_transform_stack * const ts,
                                         skc_transform_float_t        const theta,
                                         skc_transform_float_t        const sx,
                                         skc_transform_float_t        const sy,
                                         skc_transform_float_t        const cx,
                                         skc_transform_float_t        const cy);
//
// Quadrilateral coordinates are skc_transform_float_t2 structs:
//
//   float2[4] = { xy0, xy1, xy2, xy3 }
//
// -or-
//
//   float[8]  = { x0, y0, x1, y1, x2, y2, x3, y3 };
//

bool
skc_transform_stack_push_quad_to_unit(struct skc_transform_stack * const ts,
                                      float const quad_src[8]);

bool
skc_transform_stack_push_unit_to_quad(struct skc_transform_stack * const ts,
                                      float const quad_dst[8]);

bool
skc_transform_stack_push_rect_to_quad(struct skc_transform_stack * const ts,
                                      float const x0,
                                      float const y0,
                                      float const x1,
                                      float const y1,
                                      float const quad_dst[8]);

bool
skc_transform_stack_push_quad_to_quad(struct skc_transform_stack * const ts,
                                      float const quad_src[8],
                                      float const quad_dst[8]);

//
//
//

void
skc_transform_stack_concat(struct skc_transform_stack * const ts);

//
//
//

