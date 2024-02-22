/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkYUVMath_DEFINED
#define SkYUVMath_DEFINED

enum SkYUVColorSpace : int;

void SkColorMatrix_RGB2YUV(SkYUVColorSpace, float m[20]);
void SkColorMatrix_YUV2RGB(SkYUVColorSpace, float m[20]);

// Used to create the pre-compiled tables in SkYUVMath.cpp
void SkColorMatrix_DumpYUVMatrixTables();

#endif
