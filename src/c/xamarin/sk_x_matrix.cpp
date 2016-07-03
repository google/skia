/*
 * Copyright 2016 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkStream.h"

#include "xamarin/sk_x_matrix.h"

#include "../sk_types_priv.h"
#include "sk_x_types_priv.h"

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
}

void sk_matrix_map_points (sk_matrix_t *matrix, sk_point_t *dst, sk_point_t *src, int count)
{
	SkMatrix skmatrix;
	from_c (matrix, &skmatrix);
	skmatrix.mapPoints (AsPoint (dst), AsPoint (src), count);
}

void sk_matrix_map_vectors (sk_matrix_t *matrix, sk_point_t *dst, sk_point_t *src, int count)
{
	SkMatrix skmatrix;
	from_c (matrix, &skmatrix);
	skmatrix.mapVectors (AsPoint (dst), AsPoint (src), count);
}

sk_point_t sk_matrix_map_xy (sk_matrix_t *matrix, float x, float y)
{
	SkMatrix skmatrix;
	SkPoint result;
	from_c (matrix, &skmatrix);
	skmatrix.mapXY (x, y, &result);
	return *ToPoint(&result);
}

sk_point_t sk_matrix_map_vector (sk_matrix_t *matrix, float x, float y)
{
	SkMatrix skmatrix;
	SkPoint result;
	from_c (matrix, &skmatrix);
	skmatrix.mapVector (x, y, &result);
	return *ToPoint(&result);
}

float sk_matrix_map_radius (sk_matrix_t *matrix, float radius)
{
	SkMatrix skmatrix;
	from_c (matrix, &skmatrix);
	return skmatrix.mapRadius (radius);
}

