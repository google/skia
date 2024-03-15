/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlAttachment_DEFINED
#define GrMtlAttachment_DEFINED

#include "include/gpu/ganesh/mtl/GrMtlTypes.h"
#include "src/gpu/ganesh/GrAttachment.h"

#import <Metal/Metal.h>

class GrBackendFormat;
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
                                              skgpu::Budgeted budgeted);

    static sk_sp<GrMtlAttachment> MakeWrapped(GrMtlGpu* gpu,
                                              SkISize dimensions,
                                              id<MTLTexture>,
                                              UsageFlags attachmentUsages,
                                              GrWrapCacheable,
                                              std::string_view label);

    ~GrMtlAttachment() override;

    GrBackendFormat backendFormat() const override;

    MTLPixelFormat mtlFormat() const { return fTexture.pixelFormat; }

    id<MTLTexture> mtlTexture() const { return fTexture; }

    unsigned int sampleCount() const { return SkToU32(fTexture.sampleCount); }

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
                                       skgpu::Budgeted);

    GrMtlAttachment(GrMtlGpu* gpu,
                    SkISize dimensions,
                    UsageFlags supportedUsages,
                    id<MTLTexture> texture,
                    skgpu::Budgeted,
                    std::string_view label);

    GrMtlAttachment(GrMtlGpu* gpu,
                    SkISize dimensions,
                    UsageFlags supportedUsages,
                    id<MTLTexture> texture,
                    GrWrapCacheable,
                    std::string_view label);

    GrMtlGpu* getMtlGpu() const;

    void onSetLabel() override;

    id<MTLTexture> fTexture;
};

#endif
