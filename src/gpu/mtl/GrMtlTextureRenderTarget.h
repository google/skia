/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlTextureRenderTarget_DEFINED
#define GrMtlTextureRenderTarget_DEFINED

#include "GrMtlRenderTarget.h"
#include "GrMtlTexture.h"

class GrMtlTextureRenderTarget: public GrMtlTexture, public GrMtlRenderTarget {
public:
    static sk_sp<GrMtlTextureRenderTarget> CreateNewTextureRenderTarget(GrMtlGpu*,
                                                                        const GrSurfaceDesc&,
                                                                        SkBudgeted,
                                                                        int mipLevels);

    static sk_sp<GrMtlTextureRenderTarget> MakeWrappedTextureRenderTarget(GrMtlGpu*,
                                                                          const GrSurfaceDesc&,
                                                                          GrWrapOwnership,
                                                                          int mipLevels);

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
                             const GrSurfaceDesc& desc,
                             SkBudgeted budgeted,
                             id<MTLTexture> renderTexture,
                             id<MTLTexture> resolveTexture);

    GrMtlTextureRenderTarget(GrMtlGpu* gpu,
                             const GrSurfaceDesc& desc,
                             SkBudgeted budgeted,
                             id<MTLTexture> renderTexture);

    GrMtlTextureRenderTarget(GrMtlGpu* gpu,
                             const GrSurfaceDesc& desc,
                             id<MTLTexture> renderTexture,
                             id<MTLTexture> resolveTexture);

    GrMtlTextureRenderTarget(GrMtlGpu* gpu,
                             const GrSurfaceDesc& desc,
                             id<MTLTexture> renderTexture);

    static sk_sp<GrMtlTextureRenderTarget> Make(GrMtlGpu*,
                                                const GrSurfaceDesc&,
                                                SkBudgeted budgeted,
                                                id<MTLTexture> resolveTexture);

    size_t onGpuMemorySize() const override {
        // TODO: When used as render targets certain formats may actually have a larger size than
        // the base format size. Check to make sure we are reporting the correct value here.
        // The plus 1 is to account for the resolve texture or if not using msaa the RT itself
        int numColorSamples = this->numColorSamples() + 1;
        return GrSurface::ComputeSize(this->config(), this->width(), this->height(),
                                      numColorSamples, false);
    }
};
#endif
