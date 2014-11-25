/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL
// DO NOT USE -- FOR INTERNAL TESTING ONLY

#ifndef sk_shader_DEFINED
#define sk_shader_DEFINED

#include "sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

void sk_shader_ref(sk_shader_t*);
void sk_shader_unref(sk_shader_t*);

typedef enum {
    CLAMP_SK_SHADER_TILEMODE,
    REPEAT_SK_SHADER_TILEMODE,
    MIRROR_SK_SHADER_TILEMODE,
} sk_shader_tilemode_t;

sk_shader_t* sk_shader_new_linear_gradient(const sk_point_t pts[2],
                                           const sk_color_t colors[],
                                           const float colorPos[],
                                           int colorCount,
                                           sk_shader_tilemode_t tileMode,
                                           const sk_matrix_t* localMatrix);


SK_C_PLUS_PLUS_END_GUARD

#endif
