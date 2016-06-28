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

/** Set the matrix to identity */
void sk_matrix_set_identity(sk_matrix_t*);

/** Set the matrix to translate by (tx, ty). */
void sk_matrix_set_translate(sk_matrix_t*, float tx, float ty);
/**
    Preconcats the matrix with the specified translation.
        M' = M * T(dx, dy)
*/
void sk_matrix_pre_translate(sk_matrix_t*, float tx, float ty);
/**
    Postconcats the matrix with the specified translation.
        M' = T(dx, dy) * M
*/
void sk_matrix_post_translate(sk_matrix_t*, float tx, float ty);

/** Set the matrix to scale by sx and sy. */
void sk_matrix_set_scale(sk_matrix_t*, float sx, float sy);
/**
    Preconcats the matrix with the specified scale.
        M' = M * S(sx, sy)
*/
void sk_matrix_pre_scale(sk_matrix_t*, float sx, float sy);
/**
    Postconcats the matrix with the specified scale.
        M' = S(sx, sy) * M
*/
void sk_matrix_post_scale(sk_matrix_t*, float sx, float sy);

/**
    Returns the matrix type
 */
SK_API int sk_matrix_try_invert (sk_matrix_t *matrix, sk_matrix_t *result);

/**
    Sets a matrix to the concatenation of the other two.
 */
SK_API void sk_matrix_concat (sk_matrix_t *result, sk_matrix_t *first, sk_matrix_t *second);

/**
    Preconcatenates the matrix
 */
SK_API void sk_matrix_pre_concat (sk_matrix_t *result, sk_matrix_t *matrix);
/**
    Sets a matrix to the concatenation of the other two.
 */
SK_API void sk_matrix_post_concat (sk_matrix_t *result, sk_matrix_t *matrix);

/**
    Apply the @matrix to the coordinates in rectangle @source using the matrix definition into @dest
*/
SK_API void sk_matrix_map_rect (sk_matrix_t *matrix, sk_rect_t *dest, sk_rect_t *source);

/**
    Apply the @matrix to the array of points @src containing @count points into @dst
*/
SK_API void sk_matrix_map_points (sk_matrix_t *matrix, sk_point_t *dst, sk_point_t *src, int count);

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
SK_API void sk_matrix_map_vectors (sk_matrix_t *matrix, sk_point_t *dst, sk_point_t *src, int count);

/**
    Applies the matrix to the the @x,@y positions
*/
SK_API sk_point_t sk_matrix_map_xy (sk_matrix_t *matrix, float x, float y);

/**
    Applies the matrix to the the @x,@y positions, ignoring the translation component.
*/
SK_API sk_point_t sk_matrix_map_vector (sk_matrix_t *matrix, float x, float y);

/**
    Return the mean radius of a circle after it has been mapped by
    this matrix. NOTE: in perspective this value assumes the circle
    has its center at the origin.
*/
SK_API float sk_matrix_map_radius (sk_matrix_t *matrix, float radius);

SK_C_PLUS_PLUS_END_GUARD

#endif
