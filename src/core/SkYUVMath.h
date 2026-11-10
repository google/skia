/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkYUVMath_DEFINED
#define SkYUVMath_DEFINED

#include "include/core/SkSpan.h"

enum SkYUVColorSpace : int;

void SkColorMatrix_RGB2YUV(SkYUVColorSpace, SkSpan<float, 20> m);
void SkColorMatrix_YUV2RGB(SkYUVColorSpace, SkSpan<float, 20> m);

// Used to create the pre-compiled tables in SkYUVMath.cpp
void SkColorMatrix_DumpYUVMatrixTables();

#endif
