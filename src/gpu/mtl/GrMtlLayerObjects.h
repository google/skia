/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlLayerObjects_DEFINED
#define GrMtlLayerObjects_DEFINED

#include "src/gpu/mtl/GrMtlRenderTarget.h"

#import <QuartzCore/CAMetalLayer.h>

class GrMtlLayerRenderTarget: public GrMtlRenderTarget {
public:
    static sk_sp<GrMtlLayerRenderTarget> MakeWrappedRenderTarget(GrMtlGpu*,
                                                            const GrSurfaceDesc&,
                                                            int sampleCnt,
                                                            CAMetalLayer* layer,
                                                            GrMTLHandle* drawable);

    ~GrMtlLayerRenderTarget() override {}

    id<MTLTexture> mtlColorTexture() const override;
    id<MTLTexture> mtlResolveTexture() const override;

    GrBackendRenderTarget getBackendRenderTarget() const override;

    GrBackendFormat backendFormat() const override;

private:
    GrMtlLayerRenderTarget(GrMtlGpu* gpu,
                      const GrSurfaceDesc& desc,
                      int sampleCnt,
                      id<MTLTexture> colorTexture,
                      CAMetalLayer* layer,
                      GrMTLHandle* drawable);
    GrMtlLayerRenderTarget(GrMtlGpu* gpu,
                      const GrSurfaceDesc& desc,
                      CAMetalLayer* layer,
                      GrMTLHandle* drawable);

    CAMetalLayer* fLayer;
    mutable GrMTLHandle* fDrawable;
    int fSampleCount;
};

#endif

