/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkYUVMath_DEFINED
#define SkYUVMath_DEFINED

#include "include/core/SkImageInfo.h"

void SkColorMatrix_RGB2YUV(SkYUVColorSpace, float m[20]);
void SkColorMatrix_YUV2RGB(SkYUVColorSpace, float m[20]);
void SkMatrix44_RGB2YUV(SkYUVColorSpace, SkMatrix44*);
void SkMatrix44_YUV2RGB(SkYUVColorSpace, SkMatrix44*);

#endif
