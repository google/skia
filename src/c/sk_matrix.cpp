/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMatrix.h"
#include "SkCamera.h"
#include "SkMatrix44.h"

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


void sk_matrix44_destroy (sk_matrix44_t* matrix)
{
    delete AsMatrix44 (matrix);
}

sk_matrix44_t* sk_matrix44_new ()
{
    return ToMatrix44 (new SkMatrix44(SkMatrix44::Uninitialized_Constructor::kUninitialized_Constructor));
}

sk_matrix44_t* sk_matrix44_new_identity ()
{
    return ToMatrix44 (new SkMatrix44(SkMatrix44::Identity_Constructor::kIdentity_Constructor));
}

sk_matrix44_t* sk_matrix44_new_copy (const sk_matrix44_t* src)
{
    return ToMatrix44 (new SkMatrix44(AsMatrix44 (*src)));
}

sk_matrix44_t* sk_matrix44_new_concat (const sk_matrix44_t* a, const sk_matrix44_t* b)
{
    return ToMatrix44 (new SkMatrix44(AsMatrix44 (*a), AsMatrix44 (*b)));
}

sk_matrix44_t* sk_matrix44_new_matrix (const sk_matrix_t* src)
{
    SkMatrix skmatrix;
    from_c (src, &skmatrix);
    return ToMatrix44 (new SkMatrix44(skmatrix));
}

bool sk_matrix44_equals (sk_matrix44_t* matrix, const sk_matrix44_t* other)
{
    return AsMatrix44(matrix) == AsMatrix44(other);
}

void sk_matrix44_to_matrix (sk_matrix44_t* matrix, sk_matrix_t* dst)
{
    SkMatrix skmatrix = AsMatrix44(*matrix);
    from_sk (&skmatrix, dst);
}

sk_matrix44_type_mask_t sk_matrix44_get_type (sk_matrix44_t* matrix)
{
    return (sk_matrix44_type_mask_t) AsMatrix44(matrix)->getType ();
}

void sk_matrix44_set_identity (sk_matrix44_t* matrix)
{
    AsMatrix44(matrix)->setIdentity ();
}

float sk_matrix44_get (sk_matrix44_t* matrix, int row, int col)
{
    return AsMatrix44(matrix)->get (row, col);
}

void sk_matrix44_set (sk_matrix44_t* matrix, int row, int col, float value)
{
    AsMatrix44(matrix)->set (row, col, value);
}

void sk_matrix44_as_col_major (sk_matrix44_t* matrix, float* dst)
{
    AsMatrix44(matrix)->asColMajorf (dst);
}

void sk_matrix44_as_row_major (sk_matrix44_t* matrix, float* dst)
{
    AsMatrix44(matrix)->asRowMajorf (dst);
}

void sk_matrix44_set_col_major (sk_matrix44_t* matrix, float* dst)
{
    AsMatrix44(matrix)->setColMajorf (dst);
}

void sk_matrix44_set_row_major (sk_matrix44_t* matrix, float* dst)
{
    AsMatrix44(matrix)->setRowMajorf (dst);
}

void sk_matrix44_set_translate (sk_matrix44_t* matrix, float dx, float dy, float dz)
{
    AsMatrix44(matrix)->setTranslate (dx, dy, dz);
}

void sk_matrix44_pre_translate (sk_matrix44_t* matrix, float dx, float dy, float dz)
{
    AsMatrix44(matrix)->preTranslate (dx, dy, dz);
}

void sk_matrix44_post_translate (sk_matrix44_t* matrix, float dx, float dy, float dz)
{
    AsMatrix44(matrix)->postTranslate (dx, dy, dz);
}

void sk_matrix44_set_scale (sk_matrix44_t* matrix, float sx, float sy, float sz)
{
    AsMatrix44(matrix)->setScale (sx, sy, sz);
}

void sk_matrix44_pre_scale (sk_matrix44_t* matrix, float sx, float sy, float sz)
{
    AsMatrix44(matrix)->preScale (sx, sy, sz);
}

void sk_matrix44_post_scale (sk_matrix44_t* matrix, float sx, float sy, float sz)
{
    AsMatrix44(matrix)->postScale (sx, sy, sz);
}

void sk_matrix44_set_rotate_about_degrees (sk_matrix44_t* matrix, float x, float y, float z, float degrees)
{
    AsMatrix44(matrix)->setRotateDegreesAbout (x, y, z, degrees);
}

void sk_matrix44_set_rotate_about_radians (sk_matrix44_t* matrix, float x, float y, float z, float radians)
{
    AsMatrix44(matrix)->setRotateAbout (x, y, z, radians);
}

void sk_matrix44_set_rotate_about_radians_unit (sk_matrix44_t* matrix, float x, float y, float z, float radians)
{
    AsMatrix44(matrix)->setRotateAboutUnit (x, y, z, radians);
}

void sk_matrix44_set_concat (sk_matrix44_t* matrix, const sk_matrix44_t* a, const sk_matrix44_t* b)
{
    AsMatrix44 (matrix)->setConcat (AsMatrix44 (*a), AsMatrix44 (*b));
}

void sk_matrix44_pre_concat (sk_matrix44_t* matrix, const sk_matrix44_t* m)
{
    AsMatrix44 (matrix)->preConcat (AsMatrix44 (*m));
}

void sk_matrix44_post_concat (sk_matrix44_t* matrix, const sk_matrix44_t* m)
{
    AsMatrix44 (matrix)->postConcat (AsMatrix44 (*m));
}

bool sk_matrix44_invert (sk_matrix44_t* matrix, sk_matrix44_t* inverse)
{
    return AsMatrix44 (matrix)->invert (AsMatrix44 (inverse));
}

void sk_matrix44_transpose (sk_matrix44_t* matrix)
{
    AsMatrix44 (matrix)->transpose ();
}

void sk_matrix44_map_scalars (sk_matrix44_t* matrix, const float* src, float* dst)
{
    AsMatrix44(matrix)->mapScalars (src, dst);
}

void sk_matrix44_map2 (sk_matrix44_t* matrix, const float* src2, int count, float* dst4)
{
    AsMatrix44(matrix)->map2 (src2, count, dst4);
}

bool sk_matrix44_preserves_2d_axis_alignment (sk_matrix44_t* matrix, float epsilon)
{
    return AsMatrix44(matrix)->preserves2dAxisAlignment (epsilon);
}

double sk_matrix44_determinant (sk_matrix44_t* matrix)
{
    return AsMatrix44(matrix)->determinant ();
}

