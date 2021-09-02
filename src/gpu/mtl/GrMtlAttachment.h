/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlAttachment_DEFINED
#define GrMtlAttachment_DEFINED

#include "include/gpu/mtl/GrMtlTypes.h"
#include "src/gpu/GrAttachment.h"

#import <Metal/Metal.h>

class GrMtlGpu;

class GrMtlAttachment : public GrAttachment {
public:
    static sk_sp<GrMtlAttachment> MakeStencil(GrMtlGpu* gpu,
                                              SkISize dimensions,
                                              int sampleCnt,
                                              MTLPixelFormat format);

    static sk_sp<GrMtlAttachment> MakeMSAA(GrMtlGpu* gpu,
                                           SkISize dimensions,
                                           int sampleCnt,
                                           MTLPixelFormat format);

    static sk_sp<GrMtlAttachment> MakeTexture(GrMtlGpu* gpu,
                                              SkISize dimensions,
                                              MTLPixelFormat format,
                                              uint32_t mipLevels,
                                              GrRenderable renderable,
                                              int numSamples,
                                              SkBudgeted budgeted);

    static sk_sp<GrMtlAttachment> MakeWrapped(GrMtlGpu* gpu,
                                              SkISize dimensions,
                                              id<MTLTexture>,
                                              UsageFlags attachmentUsages,
                                              GrWrapCacheable);

    ~GrMtlAttachment() override;

    GrBackendFormat backendFormat() const override {
        return GrBackendFormat::MakeMtl(fTexture.pixelFormat);
    }

    MTLPixelFormat mtlFormat() const { return fTexture.pixelFormat; }

    id<MTLTexture> mtlTexture() const { return fTexture; }

    unsigned int sampleCount() const { return fTexture.sampleCount; }

    bool framebufferOnly() const { return fTexture.framebufferOnly; }

protected:
    void onRelease() override;
    void onAbandon() override;

private:
    static sk_sp<GrMtlAttachment> Make(GrMtlGpu* gpu,
                                       SkISize dimensions,
                                       UsageFlags attachmentUsages,
                                       int sampleCnt,
                                       MTLPixelFormat format,
                                       uint32_t mipLevels,
                                       int mtlTextureUsage,
                                       int mtlStorageMode,
                                       SkBudgeted);

    GrMtlAttachment(GrMtlGpu* gpu,
                    SkISize dimensions,
                    UsageFlags supportedUsages,
                    id<MTLTexture> texture,
                    SkBudgeted);

    GrMtlAttachment(GrMtlGpu* gpu,
                    SkISize dimensions,
                    UsageFlags supportedUsages,
                    id<MTLTexture> texture,
                    GrWrapCacheable);

    GrMtlGpu* getMtlGpu() const;

    id<MTLTexture> fTexture;
};

#endif
