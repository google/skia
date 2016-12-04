/*
 * Copyright 2016 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL
// DO NOT USE -- FOR INTERNAL TESTING ONLY

#ifndef sk_patheffect_DEFINED
#define sk_patheffect_DEFINED

#include "sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

SK_API void sk_path_effect_unref(sk_path_effect_t* t); 
SK_API sk_path_effect_t* sk_path_effect_create_compose(sk_path_effect_t* outer, sk_path_effect_t* inner);
SK_API sk_path_effect_t* sk_path_effect_create_sum(sk_path_effect_t* first, sk_path_effect_t* second);
SK_API sk_path_effect_t* sk_path_effect_create_discrete(float segLength, float deviation, uint32_t seedAssist /*0*/);
SK_API sk_path_effect_t* sk_path_effect_create_corner(float radius);
SK_API sk_path_effect_t* sk_path_effect_create_1d_path(const sk_path_t* path, float advance, float phase, sk_path_effect_1d_style_t style);
SK_API sk_path_effect_t* sk_path_effect_create_2d_line(float width, const sk_matrix_t* matrix);
SK_API sk_path_effect_t* sk_path_effect_create_2d_path(const sk_matrix_t* matrix, const sk_path_t* path);
SK_API sk_path_effect_t* sk_path_effect_create_dash(const float intervals[], int count, float phase);

SK_C_PLUS_PLUS_END_GUARD

#endif
