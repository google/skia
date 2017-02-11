/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL
// DO NOT USE -- FOR INTERNAL TESTING ONLY

#ifndef sk_matrix_DEFINED
#define sk_matrix_DEFINED

#include "sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

/**
    Set the matrix to identity
*/
SK_C_API void sk_matrix_set_identity (sk_matrix_t*);

/**
    Set the matrix to translate by (tx, ty).
*/
SK_C_API void sk_matrix_set_translate (sk_matrix_t*, float tx, float ty);

/**
    Preconcats the matrix with the specified translation.
        M' = M * T(dx, dy)
*/
SK_C_API void sk_matrix_pre_translate (sk_matrix_t*, float tx, float ty);

/**
    Postconcats the matrix with the specified translation.
        M' = T(dx, dy) * M
*/
SK_C_API void sk_matrix_post_translate (sk_matrix_t*, float tx, float ty);

/** 
    Set the matrix to scale by sx and sy. 
*/
SK_C_API void sk_matrix_set_scale (sk_matrix_t*, float sx, float sy);

/**
    Preconcats the matrix with the specified scale.
        M' = M * S(sx, sy)
*/
SK_C_API void sk_matrix_pre_scale (sk_matrix_t*, float sx, float sy);

/**
    Postconcats the matrix with the specified scale.
        M' = S(sx, sy) * M
*/
SK_C_API void sk_matrix_post_scale (sk_matrix_t*, float sx, float sy);

/**
    Returns the matrix type
 */
SK_C_API int sk_matrix_try_invert (sk_matrix_t *matrix, sk_matrix_t *result);

/**
    Sets a matrix to the concatenation of the other two.
 */
SK_C_API void sk_matrix_concat (sk_matrix_t *result, sk_matrix_t *first, sk_matrix_t *second);

/**
    Preconcatenates the matrix
 */
SK_C_API void sk_matrix_pre_concat (sk_matrix_t *result, sk_matrix_t *matrix);
/**
    Sets a matrix to the concatenation of the other two.
 */
SK_C_API void sk_matrix_post_concat (sk_matrix_t *result, sk_matrix_t *matrix);

/**
    Apply the @matrix to the coordinates in rectangle @source using the matrix definition into @dest
*/
SK_C_API void sk_matrix_map_rect (sk_matrix_t *matrix, sk_rect_t *dest, sk_rect_t *source);

/**
    Apply the @matrix to the array of points @src containing @count points into @dst
*/
SK_C_API void sk_matrix_map_points (sk_matrix_t *matrix, sk_point_t *dst, sk_point_t *src, int count);

/**
    Apply this matrix to the array of vectors specified by src, and write
        the transformed vectors into the array of vectors specified by dst.
        This is similar to mapPoints, but ignores any translation in the matrix.
        @param dst  Where the transformed coordinates are written. It must
                    contain at least count entries
        @param src  The original coordinates that are to be transformed. It
                    must contain at least count entries
        @param count The number of vectors in src to read, and then transform
                     into dst.
*/
SK_C_API void sk_matrix_map_vectors (sk_matrix_t *matrix, sk_point_t *dst, sk_point_t *src, int count);

/**
    Applies the matrix to the the @x,@y positions
*/
SK_C_API void sk_matrix_map_xy (sk_matrix_t *matrix, float x, float y, sk_point_t* result);

/**
    Applies the matrix to the the @x,@y positions, ignoring the translation component.
*/
SK_C_API void sk_matrix_map_vector (sk_matrix_t *matrix, float x, float y, sk_point_t* result);

/**
    Return the mean radius of a circle after it has been mapped by
    this matrix. NOTE: in perspective this value assumes the circle
    has its center at the origin.
*/
SK_C_API float sk_matrix_map_radius (sk_matrix_t *matrix, float radius);


SK_C_API sk_3dview_t* sk_3dview_new ();
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
SK_C_API sk_matrix44_t* sk_matrix44_new ();
SK_C_API sk_matrix44_t* sk_matrix44_new_identity ();
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
SK_C_API void sk_matrix44_set_sccle (sk_matrix44_t* matrix, float sx, float sy, float sz);
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
