/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlTextureRenderTarget_DEFINED
#define GrMtlTextureRenderTarget_DEFINED

#include "src/gpu/mtl/GrMtlRenderTarget.h"
#include "src/gpu/mtl/GrMtlTexture.h"

class GrMtlTextureRenderTarget: public GrMtlTexture, public GrMtlRenderTarget {
public:
    static sk_sp<GrMtlTextureRenderTarget> CreateNewTextureRenderTarget(GrMtlGpu*,
                                                                        SkBudgeted,
                                                                        const GrSurfaceDesc&,
                                                                        MTLTextureDescriptor*,
                                                                        GrMipMapsStatus);

    static sk_sp<GrMtlTextureRenderTarget> MakeWrappedTextureRenderTarget(GrMtlGpu*,
                                                                          const GrSurfaceDesc&,
                                                                          id<MTLTexture>,
                                                                          GrWrapCacheable);
    GrBackendFormat backendFormat() const override {
        return GrMtlTexture::backendFormat();
    }

protected:
    void onAbandon() override {
        GrMtlRenderTarget::onAbandon();
        GrMtlTexture::onAbandon();
    }

    void onRelease() override {
        GrMtlRenderTarget::onRelease();
        GrMtlTexture::onRelease();
    }

private:
    GrMtlTextureRenderTarget(GrMtlGpu* gpu,
                             SkBudgeted budgeted,
                             const GrSurfaceDesc& desc,
                             id<MTLTexture> renderTexture,
                             id<MTLTexture> resolveTexture,
                             GrMipMapsStatus);

    GrMtlTextureRenderTarget(GrMtlGpu* gpu,
                             SkBudgeted budgeted,
                             const GrSurfaceDesc& desc,
                             id<MTLTexture> renderTexture,
                             GrMipMapsStatus);

    GrMtlTextureRenderTarget(GrMtlGpu* gpu,
                             const GrSurfaceDesc& desc,
                             id<MTLTexture> renderTexture,
                             id<MTLTexture> resolveTexture,
                             GrMipMapsStatus);

    GrMtlTextureRenderTarget(GrMtlGpu* gpu,
                             const GrSurfaceDesc& desc,
                             id<MTLTexture> renderTexture,
                             GrMipMapsStatus,
                             GrWrapCacheable cacheable);

    size_t onGpuMemorySize() const override {
        // TODO: When used as render targets certain formats may actually have a larger size than
        // the base format size. Check to make sure we are reporting the correct value here.
        // The plus 1 is to account for the resolve texture or if not using msaa the RT itself
        int numColorSamples = this->numColorSamples();
        if (numColorSamples > 1) {
            ++numColorSamples;
        }
        return GrSurface::ComputeSize(this->config(), this->width(), this->height(),
                                      numColorSamples, GrMipMapped::kNo, false);
    }
};

#endif
