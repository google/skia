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
    static sk_sp<GrMtlTextureRenderTarget> MakeNewTextureRenderTarget(GrMtlGpu*,
                                                                      SkBudgeted,
                                                                      SkISize,
                                                                      int sampleCnt,
                                                                      MTLPixelFormat,
                                                                      uint32_t mipLevels,
                                                                      GrMipmapStatus);

    static sk_sp<GrMtlTextureRenderTarget> MakeWrappedTextureRenderTarget(GrMtlGpu*,
                                                                          SkISize,
                                                                          int sampleCnt,
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
                             SkISize,
                             sk_sp<GrMtlAttachment> texture,
                             sk_sp<GrMtlAttachment> colorAttachment,
                             sk_sp<GrMtlAttachment> resolveAttachment,
                             GrMipmapStatus);

    GrMtlTextureRenderTarget(GrMtlGpu* gpu,
                             SkISize,
                             sk_sp<GrMtlAttachment> texture,
                             sk_sp<GrMtlAttachment> colorAttachment,
                             sk_sp<GrMtlAttachment> resolveAttachment,
                             GrMipmapStatus,
                             GrWrapCacheable cacheable);

    size_t onGpuMemorySize() const override;
};

#endif
