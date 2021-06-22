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
 * If `inMatrix` is invertible, true is returned. If not, false is returned and `outMatrix`
 * is left in an unspecified state.
 */
bool SkInvert2x2Matrix(const SkScalar inMatrix[4],  SkScalar outMatrix[4]);
bool SkInvert3x3Matrix(const SkScalar inMatrix[9],  SkScalar outMatrix[9]);
bool SkInvert4x4Matrix(const SkScalar inMatrix[16], SkScalar outMatrix[16]);

#endif
