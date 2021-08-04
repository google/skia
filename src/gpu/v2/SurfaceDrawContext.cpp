/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/v2/SurfaceDrawContext_v2.h"

namespace skgpu::v2 {

SurfaceDrawContext::SurfaceDrawContext(GrRecordingContext* rContext,
                                       GrSurfaceProxyView readView,
                                       GrSurfaceProxyView writeView,
                                       GrColorType colorType,
                                       sk_sp<SkColorSpace> colorSpace,
                                       const SkSurfaceProps& surfaceProps,
                                       bool flushTimeOpsTask)
    : SurfaceFillContext(rContext,
                         std::move(readView),
                         std::move(writeView),
                         {colorType, kPremul_SkAlphaType, std::move(colorSpace)},
                         flushTimeOpsTask) {
}

} // namespace skgpu::v2
