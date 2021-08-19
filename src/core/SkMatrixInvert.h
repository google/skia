/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMatrixInvert_DEFINED
#define SkMatrixInvert_DEFINED

#include "include/core/SkScalar.h"

/**
 * Computes the inverse of `inMatrix`, passed in column-major order.
 * `inMatrix` and `outMatrix` are allowed to point to the same array of scalars in memory.
 * `outMatrix` is allowed to be null.
 * The return value is the determinant of the input matrix. If zero is returned, the matrix was
 * non-invertible, and `outMatrix` has been left in an indeterminate state.
 */
SkScalar SkInvert2x2Matrix(const SkScalar inMatrix[4], SkScalar outMatrix[4]);
SkScalar SkInvert3x3Matrix(const SkScalar inMatrix[9], SkScalar outMatrix[9]);
SkScalar SkInvert4x4Matrix(const SkScalar inMatrix[16], SkScalar outMatrix[16]);

#endif
