/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "sk_facade.h"

sk_shader_t* sk_shader_new_linear_gradient(const sk_point_t* startPoint,
                                           const sk_point_t* endPoint,
                                           const sk_color_t colors[],
                                           const float colorPos[],
                                           int colorCount,
                                           sk_shader_tilemode_t tileMode,
                                           const sk_matrix_t* localMatrix) {
    sk_point_t points[2];
    points[0] = *startPoint;
    points[1] = *endPoint;
    return sk_shader_new_linear_gradient(
            points, colors, colorPos, colorCount, tileMode, localMatrix);
}

sk_rect_t sk_path_get_bounds(const sk_path_t* this_path) {
    sk_rect_t rect;
    sk_path_get_bounds(this_path, &rect);
    return rect;
}