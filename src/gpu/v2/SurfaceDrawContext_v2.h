/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SurfaceDrawContext_v2_DEFINED
#define SurfaceDrawContext_v2_DEFINED

#include "src/gpu/v2/SurfaceFillContext_v2.h"

namespace skgpu::v2 {

class SurfaceDrawContext : public SurfaceFillContext {
public:
    SurfaceDrawContext(GrRecordingContext*,
                       GrSurfaceProxyView readView,
                       GrSurfaceProxyView writeView,
                       GrColorType,
                       sk_sp<SkColorSpace>,
                       const SkSurfaceProps&,
                       bool flushTimeOpsTask = false);

private:
    using INHERITED = SurfaceFillContext;
};

} // namespace skgpu::v2

#endif // SurfaceDrawContext_v2_DEFINED
