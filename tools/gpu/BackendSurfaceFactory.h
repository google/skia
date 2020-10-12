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

namespace sk_gpu_test {

sk_sp<SkSurface> MakeBackendTextureSurface(GrDirectContext*,
                                           const SkImageInfo&,
                                           GrSurfaceOrigin,
                                           int sampleCnt,
                                           GrMipmapped = GrMipmapped::kNo,
                                           GrProtected = GrProtected::kNo,
                                           const SkSurfaceProps* = nullptr);

sk_sp<SkSurface> MakeBackendTextureSurface(GrDirectContext*,
                                           SkISize,
                                           GrSurfaceOrigin,
                                           int sampleCnt,
                                           SkColorType,
                                           sk_sp<SkColorSpace> = nullptr,
                                           GrMipmapped = GrMipmapped::kNo,
                                           GrProtected = GrProtected::kNo,
                                           const SkSurfaceProps* = nullptr);

/** Creates an SkSurface backed by a non-textureable render target. */
sk_sp<SkSurface> MakeBackendRenderTargetSurface(GrDirectContext*,
                                                const SkImageInfo&,
                                                GrSurfaceOrigin,
                                                int sampleCnt,
                                                GrProtected = GrProtected::kNo,
                                                const SkSurfaceProps* = nullptr);

sk_sp<SkSurface> MakeBackendRenderTargetSurface(GrDirectContext*,
                                                SkISize,
                                                GrSurfaceOrigin,
                                                int sampleCnt,
                                                SkColorType,
                                                sk_sp<SkColorSpace> = nullptr,
                                                GrProtected = GrProtected::kNo,
                                                const SkSurfaceProps* = nullptr);
}  // namespace sk_gpu_test

#endif
