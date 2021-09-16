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

class SurfaceDrawContext final : public SurfaceFillContext {
public:
    ~SurfaceDrawContext() override;

    static std::unique_ptr<SurfaceDrawContext> Make(GrRecordingContext*,
                                                    GrColorType,
                                                    sk_sp<GrSurfaceProxy>,
                                                    sk_sp<SkColorSpace>,
                                                    GrSurfaceOrigin,
                                                    const SkSurfaceProps&);

    static std::unique_ptr<SurfaceDrawContext> Make(GrRecordingContext*,
                                                    GrColorType,
                                                    sk_sp<SkColorSpace>,
                                                    SkBackingFit,
                                                    SkISize dimensions,
                                                    const SkSurfaceProps&,
                                                    int sampleCnt,
                                                    GrMipmapped,
                                                    GrProtected,
                                                    GrSurfaceOrigin,
                                                    SkBudgeted);

    const SkSurfaceProps& surfaceProps() const { return fSurfaceProps; }

public:
    SurfaceDrawContext(GrRecordingContext*,
                       GrSurfaceProxyView readView,
                       GrSurfaceProxyView writeView,
                       GrColorType,
                       sk_sp<SkColorSpace>,
                       const SkSurfaceProps&);

    const SkSurfaceProps fSurfaceProps;
};

} // namespace skgpu::v2

#endif // SurfaceDrawContext_v2_DEFINED
