/*
 * Copyright 2016 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkStream.h"

#include "sk_matrix.h"

#include "sk_types_priv.h"

void sk_matrix_set_identity (sk_matrix_t* matrix)
{
    SkMatrix skmatrix;
    from_c (matrix, &skmatrix);
    skmatrix.setIdentity ();
    from_sk (&skmatrix, matrix);
}

void sk_matrix_set_translate (sk_matrix_t* matrix, float tx, float ty)
{
    SkMatrix skmatrix;
    from_c (matrix, &skmatrix);
    skmatrix.setTranslate (tx, ty);
    from_sk (&skmatrix, matrix);
}

void sk_matrix_pre_translate (sk_matrix_t* matrix, float tx, float ty)
{
    SkMatrix skmatrix;
    from_c (matrix, &skmatrix);
    skmatrix.preTranslate (tx, ty);
    from_sk (&skmatrix, matrix);
}

void sk_matrix_post_translate (sk_matrix_t* matrix, float tx, float ty)
{
    SkMatrix skmatrix;
    from_c (matrix, &skmatrix);
    skmatrix.postTranslate (tx, ty);
    from_sk (&skmatrix, matrix);
}

void sk_matrix_set_scale (sk_matrix_t* matrix, float sx, float sy)
{
    SkMatrix skmatrix;
    from_c (matrix, &skmatrix);
    skmatrix.setScale (sx, sy);
    from_sk (&skmatrix, matrix);
}

void sk_matrix_pre_scale (sk_matrix_t* matrix, float sx, float sy)
{
    SkMatrix skmatrix;
    from_c (matrix, &skmatrix);
    skmatrix.preScale (sx, sy);
    from_sk (&skmatrix, matrix);
}

void sk_matrix_post_scale (sk_matrix_t* matrix, float sx, float sy)
{
    SkMatrix skmatrix;
    from_c (matrix, &skmatrix);
    skmatrix.postScale (sx, sy);
    from_sk (&skmatrix, matrix);
}

int sk_matrix_try_invert (sk_matrix_t *matrix, sk_matrix_t *result)
{
    SkMatrix copy, inverse;
    from_c (matrix, &copy);
    if (copy.invert (&inverse)){
        from_sk (&inverse, result);
        return 1;
    }
    return 0;
}

void sk_matrix_concat (sk_matrix_t *matrix, sk_matrix_t *first, sk_matrix_t *second)
{
    SkMatrix target, skfirst, sksecond;
    from_c (matrix, &target);
    from_c (first, &skfirst);
    from_c (second, &sksecond);
    target.setConcat (skfirst, sksecond);
    from_sk (&target, matrix);
}

void sk_matrix_pre_concat (sk_matrix_t *target, sk_matrix_t *matrix)
{
    SkMatrix sktarget, skmatrix;
    from_c (target, &sktarget);
    from_c (matrix, &skmatrix);
    sktarget.preConcat (skmatrix);
    from_sk (&sktarget, target);
}

void sk_matrix_post_concat (sk_matrix_t *target, sk_matrix_t *matrix)
{
    SkMatrix sktarget, skmatrix;
    from_c (target, &sktarget);
    from_c (matrix, &skmatrix);
    sktarget.postConcat (skmatrix);
    from_sk (&sktarget, target);
}

void sk_matrix_map_rect (sk_matrix_t *matrix, sk_rect_t *dest, sk_rect_t *source)
{
    SkMatrix skmatrix;
    from_c (matrix, &skmatrix);
    skmatrix.mapRect (AsRect (dest), *AsRect(source));
    from_sk (&skmatrix, matrix);
}

void sk_matrix_map_points (sk_matrix_t *matrix, sk_point_t *dst, sk_point_t *src, int count)
{
    SkMatrix skmatrix;
    from_c (matrix, &skmatrix);
    skmatrix.mapPoints (AsPoint (dst), AsPoint (src), count);
    from_sk (&skmatrix, matrix);
}

void sk_matrix_map_vectors (sk_matrix_t *matrix, sk_point_t *dst, sk_point_t *src, int count)
{
    SkMatrix skmatrix;
    from_c (matrix, &skmatrix);
    skmatrix.mapVectors (AsPoint (dst), AsPoint (src), count);
    from_sk (&skmatrix, matrix);
}

void sk_matrix_map_xy (sk_matrix_t *matrix, float x, float y, sk_point_t* cresult)
{
    SkMatrix skmatrix;
    from_c (matrix, &skmatrix);
    SkPoint result;
    skmatrix.mapXY (x, y, &result);
    *cresult = *ToPoint (&result);
}

void sk_matrix_map_vector (sk_matrix_t *matrix, float x, float y, sk_point_t* cresult)
{
    SkMatrix skmatrix;
    from_c (matrix, &skmatrix);
    SkPoint result;
    skmatrix.mapVector (x, y, &result);
    *cresult = *ToPoint (&result);
}

float sk_matrix_map_radius (sk_matrix_t *matrix, float radius)
{
    SkMatrix skmatrix;
    from_c (matrix, &skmatrix);
    return skmatrix.mapRadius (radius);
}

sk_3dview_t* sk_3dview_new ()
{
    return To3DView (new Sk3DView ());
}

void sk_3dview_destroy (sk_3dview_t* cview)
{
    delete As3DView (cview);
}

void sk_3dview_save (sk_3dview_t* cview)
{
    As3DView (cview)->save ();
}

void sk_3dview_restore (sk_3dview_t* cview)
{
    As3DView (cview)->restore ();
}

void sk_3dview_translate (sk_3dview_t* cview, float x, float y, float z)
{
    As3DView (cview)->translate (x, y, z);
}

void sk_3dview_rotate_x_degrees (sk_3dview_t* cview, float degrees)
{
    As3DView (cview)->rotateX (degrees);
}

void sk_3dview_rotate_y_degrees (sk_3dview_t* cview, float degrees)
{
    As3DView (cview)->rotateY (degrees);
}

void sk_3dview_rotate_z_degrees (sk_3dview_t* cview, float degrees)
{
    As3DView (cview)->rotateZ (degrees);
}

void sk_3dview_rotate_x_radians (sk_3dview_t* cview, float radians)
{
    As3DView (cview)->rotateX (SkRadiansToDegrees (radians));
}

void sk_3dview_rotate_y_radians (sk_3dview_t* cview, float radians)
{
    As3DView (cview)->rotateY (SkRadiansToDegrees (radians));
}

void sk_3dview_rotate_z_radians (sk_3dview_t* cview, float radians)
{
    As3DView (cview)->rotateZ (SkRadiansToDegrees (radians));
}

void sk_3dview_get_matrix (sk_3dview_t* cview, sk_matrix_t* cmatrix)
{
    SkMatrix matrix;
    As3DView (cview)->getMatrix (&matrix);
    from_sk (&matrix, cmatrix);
}

void sk_3dview_apply_to_canvas (sk_3dview_t* cview, sk_canvas_t* ccanvas)
{
    As3DView (cview)->applyToCanvas (AsCanvas (ccanvas));
}

float sk_3dview_dot_with_normal (sk_3dview_t* cview, float dx, float dy, float dz)
{
    return As3DView (cview)->dotWithNormal (dx, dy, dz);
}

