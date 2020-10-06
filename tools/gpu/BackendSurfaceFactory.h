/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef TestSurface_DEFINED
#define TestSurface_DEFINED

#include "include/core/SkImageInfo.h"
#include "include/core/SkSize.h"
#include "include/gpu/GrTypes.h"
#include "include/private/SkColorData.h"

class GrDirectContext;
class SkSurface;
class SkSurfaceProps;

/** Creates an SkSurface backed by a non-textureable render target. */
sk_sp<SkSurface> MakeBackendRenderTargetSurface(GrDirectContext*,
                                                SkISize,
                                                int sampleCnt,
                                                GrSurfaceOrigin,
                                                SkColorType,
                                                sk_sp<SkColorSpace> = nullptr,
                                                const SkSurfaceProps* = nullptr);

#endif
