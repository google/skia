/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkYUVMath_DEFINED
#define SkYUVMath_DEFINED

#include "include/core/SkImageInfo.h"

class SkMatrix44;

// we just drop the alpha rol/col from the colormatrix
// output is |        tr |
//           |  3x3   tg |
//           |        tb |
//           | 0 0 0  1  |
void SkColorMatrix_to_Matrix44(const float[20], SkMatrix44*);

// input: ignore the bottom row
// output: inject identity row/column for alpha
void SkMatrix44_to_ColorMatrix(const SkMatrix44&, float[20]);

void SkColorMatrix_RGB2YUV(SkYUVColorSpace, float m[20]);
void SkColorMatrix_YUV2RGB(SkYUVColorSpace, float m[20]);
void SkMatrix44_RGB2YUV(SkYUVColorSpace, SkMatrix44*);
void SkMatrix44_YUV2RGB(SkYUVColorSpace, SkMatrix44*);

#endif
