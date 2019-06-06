/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef FACADE_H_432432
#define FACADE_H_432432

#include "include/c/sk_path.h"
#include "include/c/sk_shader.h"

// facade for APIs to make them easy to wrap

sk_shader_t* sk_shader_new_linear_gradient(const sk_point_t* startPoint,
                                           const sk_point_t* endPoint,
                                           const sk_color_t colors[],
                                           const float colorPos[],
                                           int colorCount,
                                           sk_shader_tilemode_t tileMode,
                                           const sk_matrix_t* localMatrix);

sk_rect_t sk_path_get_bounds(const sk_path_t* this_path);

#endif  // FACADE_H_432432
