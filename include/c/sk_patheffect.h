/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_patheffect_DEFINED
#define sk_patheffect_DEFINED

#include "include/c/sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

SK_C_API void sk_path_effect_unref(sk_path_effect_t* t); 
SK_C_API sk_path_effect_t* sk_path_effect_create_compose(sk_path_effect_t* outer, sk_path_effect_t* inner);
SK_C_API sk_path_effect_t* sk_path_effect_create_sum(sk_path_effect_t* first, sk_path_effect_t* second);
SK_C_API sk_path_effect_t* sk_path_effect_create_discrete(float segLength, float deviation, uint32_t seedAssist /*0*/);
SK_C_API sk_path_effect_t* sk_path_effect_create_corner(float radius);
SK_C_API sk_path_effect_t* sk_path_effect_create_1d_path(const sk_path_t* path, float advance, float phase, sk_path_effect_1d_style_t style);
SK_C_API sk_path_effect_t* sk_path_effect_create_2d_line(float width, const sk_matrix_t* matrix);
SK_C_API sk_path_effect_t* sk_path_effect_create_2d_path(const sk_matrix_t* matrix, const sk_path_t* path);
SK_C_API sk_path_effect_t* sk_path_effect_create_dash(const float intervals[], int count, float phase);
SK_C_API sk_path_effect_t* sk_path_effect_create_trim(float start, float stop, sk_path_effect_trim_mode_t mode);

SK_C_PLUS_PLUS_END_GUARD

#endif
