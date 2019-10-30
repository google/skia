/*
 * Copyright 2014 Google Inc.
 * Copyright 2016 Xamarin Inc.
 * Copyright 2018 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_rrect_DEFINED
#define sk_rrect_DEFINED

#include "include/c/sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

SK_C_API sk_rrect_t* sk_rrect_new(void);
SK_C_API sk_rrect_t* sk_rrect_new_copy(const sk_rrect_t* rrect);
SK_C_API void sk_rrect_delete(const sk_rrect_t* rrect);
SK_C_API sk_rrect_type_t sk_rrect_get_type(const sk_rrect_t* rrect);
SK_C_API void sk_rrect_get_rect(const sk_rrect_t* rrect, sk_rect_t* rect);
SK_C_API void sk_rrect_get_radii(const sk_rrect_t* rrect, sk_rrect_corner_t corner, sk_vector_t* radii);
SK_C_API float sk_rrect_get_width(const sk_rrect_t* rrect);
SK_C_API float sk_rrect_get_height(const sk_rrect_t* rrect);
SK_C_API void sk_rrect_set_empty(sk_rrect_t* rrect);
SK_C_API void sk_rrect_set_rect(sk_rrect_t* rrect, const sk_rect_t* rect);
SK_C_API void sk_rrect_set_oval(sk_rrect_t* rrect, const sk_rect_t* rect);
SK_C_API void sk_rrect_set_rect_xy(sk_rrect_t* rrect, const sk_rect_t* rect, float xRad, float yRad);
SK_C_API void sk_rrect_set_nine_patch(sk_rrect_t* rrect, const sk_rect_t* rect, float leftRad, float topRad, float rightRad, float bottomRad);
SK_C_API void sk_rrect_set_rect_radii(sk_rrect_t* rrect, const sk_rect_t* rect, const sk_vector_t* radii);
SK_C_API void sk_rrect_inset(sk_rrect_t* rrect, float dx, float dy);
SK_C_API void sk_rrect_outset(sk_rrect_t* rrect, float dx, float dy);
SK_C_API void sk_rrect_offset(sk_rrect_t* rrect, float dx, float dy);
SK_C_API bool sk_rrect_contains(const sk_rrect_t* rrect, const sk_rect_t* rect);
SK_C_API bool sk_rrect_is_valid(const sk_rrect_t* rrect);
SK_C_API bool sk_rrect_transform(sk_rrect_t* rrect, const sk_matrix_t* matrix, sk_rrect_t* dest);

SK_C_PLUS_PLUS_END_GUARD

#endif
