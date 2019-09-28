/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_matrix_DEFINED
#define sk_matrix_DEFINED

#include "include/c/sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD


SK_C_API bool sk_matrix_try_invert (sk_matrix_t *matrix, sk_matrix_t *result);
SK_C_API void sk_matrix_concat (sk_matrix_t *result, sk_matrix_t *first, sk_matrix_t *second);
SK_C_API void sk_matrix_pre_concat (sk_matrix_t *result, sk_matrix_t *matrix);
SK_C_API void sk_matrix_post_concat (sk_matrix_t *result, sk_matrix_t *matrix);
SK_C_API void sk_matrix_map_rect (sk_matrix_t *matrix, sk_rect_t *dest, sk_rect_t *source);
SK_C_API void sk_matrix_map_points (sk_matrix_t *matrix, sk_point_t *dst, sk_point_t *src, int count);
SK_C_API void sk_matrix_map_vectors (sk_matrix_t *matrix, sk_point_t *dst, sk_point_t *src, int count);
SK_C_API void sk_matrix_map_xy (sk_matrix_t *matrix, float x, float y, sk_point_t* result);
SK_C_API void sk_matrix_map_vector (sk_matrix_t *matrix, float x, float y, sk_point_t* result);
SK_C_API float sk_matrix_map_radius (sk_matrix_t *matrix, float radius);


SK_C_API sk_3dview_t* sk_3dview_new (void);
SK_C_API void sk_3dview_destroy (sk_3dview_t* cview);
SK_C_API void sk_3dview_save (sk_3dview_t* cview);
SK_C_API void sk_3dview_restore (sk_3dview_t* cview);
SK_C_API void sk_3dview_translate (sk_3dview_t* cview, float x, float y, float z);
SK_C_API void sk_3dview_rotate_x_degrees (sk_3dview_t* cview, float degrees);
SK_C_API void sk_3dview_rotate_y_degrees (sk_3dview_t* cview, float degrees);
SK_C_API void sk_3dview_rotate_z_degrees (sk_3dview_t* cview, float degrees);
SK_C_API void sk_3dview_rotate_x_radians (sk_3dview_t* cview, float radians);
SK_C_API void sk_3dview_rotate_y_radians (sk_3dview_t* cview, float radians);
SK_C_API void sk_3dview_rotate_z_radians (sk_3dview_t* cview, float radians);
SK_C_API void sk_3dview_get_matrix (sk_3dview_t* cview, sk_matrix_t* cmatrix);
SK_C_API void sk_3dview_apply_to_canvas (sk_3dview_t* cview, sk_canvas_t* ccanvas);
SK_C_API float sk_3dview_dot_with_normal (sk_3dview_t* cview, float dx, float dy, float dz);


SK_C_API void sk_matrix44_destroy (sk_matrix44_t* matrix);
SK_C_API sk_matrix44_t* sk_matrix44_new (void);
SK_C_API sk_matrix44_t* sk_matrix44_new_identity (void);
SK_C_API sk_matrix44_t* sk_matrix44_new_copy (const sk_matrix44_t* src);
SK_C_API sk_matrix44_t* sk_matrix44_new_concat (const sk_matrix44_t* a, const sk_matrix44_t* b);
SK_C_API sk_matrix44_t* sk_matrix44_new_matrix (const sk_matrix_t* src);
SK_C_API bool sk_matrix44_equals (sk_matrix44_t* matrix, const sk_matrix44_t* other);
SK_C_API void sk_matrix44_to_matrix (sk_matrix44_t* matrix, sk_matrix_t* dst);
SK_C_API sk_matrix44_type_mask_t sk_matrix44_get_type (sk_matrix44_t* matrix);
SK_C_API void sk_matrix44_set_identity (sk_matrix44_t* matrix);
SK_C_API float sk_matrix44_get (sk_matrix44_t* matrix, int row, int col);
SK_C_API void sk_matrix44_set (sk_matrix44_t* matrix, int row, int col, float value);
SK_C_API void sk_matrix44_as_col_major (sk_matrix44_t* matrix, float* dst);
SK_C_API void sk_matrix44_as_row_major (sk_matrix44_t* matrix, float* dst);
SK_C_API void sk_matrix44_set_col_major (sk_matrix44_t* matrix, float* dst);
SK_C_API void sk_matrix44_set_row_major (sk_matrix44_t* matrix, float* dst);
SK_C_API void sk_matrix44_set_translate (sk_matrix44_t* matrix, float dx, float dy, float dz);
SK_C_API void sk_matrix44_pre_translate (sk_matrix44_t* matrix, float dx, float dy, float dz);
SK_C_API void sk_matrix44_post_translate (sk_matrix44_t* matrix, float dx, float dy, float dz);
SK_C_API void sk_matrix44_set_scale (sk_matrix44_t* matrix, float sx, float sy, float sz);
SK_C_API void sk_matrix44_pre_scale (sk_matrix44_t* matrix, float sx, float sy, float sz);
SK_C_API void sk_matrix44_post_scale (sk_matrix44_t* matrix, float sx, float sy, float sz);
SK_C_API void sk_matrix44_set_rotate_about_degrees (sk_matrix44_t* matrix, float x, float y, float z, float degrees);
SK_C_API void sk_matrix44_set_rotate_about_radians (sk_matrix44_t* matrix, float x, float y, float z, float radians);
SK_C_API void sk_matrix44_set_rotate_about_radians_unit (sk_matrix44_t* matrix, float x, float y, float z, float radians);
SK_C_API void sk_matrix44_set_concat (sk_matrix44_t* matrix, const sk_matrix44_t* a, const sk_matrix44_t* b);
SK_C_API void sk_matrix44_pre_concat (sk_matrix44_t* matrix, const sk_matrix44_t* m);
SK_C_API void sk_matrix44_post_concat (sk_matrix44_t* matrix, const sk_matrix44_t* m);
SK_C_API bool sk_matrix44_invert (sk_matrix44_t* matrix, sk_matrix44_t* inverse);
SK_C_API void sk_matrix44_transpose (sk_matrix44_t* matrix);
SK_C_API void sk_matrix44_map_scalars (sk_matrix44_t* matrix, const float* src, float* dst);
SK_C_API void sk_matrix44_map2 (sk_matrix44_t* matrix, const float* src2, int count, float* dst4);
SK_C_API bool sk_matrix44_preserves_2d_axis_alignment (sk_matrix44_t* matrix, float epsilon);
SK_C_API double sk_matrix44_determinant (sk_matrix44_t* matrix);


SK_C_PLUS_PLUS_END_GUARD

#endif
