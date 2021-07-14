/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlRenderTarget.h"

#include "src/gpu/mtl/GrMtlGpu.h"
#include "src/gpu/mtl/GrMtlUtil.h"

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

GR_NORETAIN_BEGIN

// Called for wrapped non-texture render targets.
GrMtlRenderTarget::GrMtlRenderTarget(GrMtlGpu* gpu,
                                     SkISize dimensions,
                                     sk_sp<GrMtlAttachment> colorAttachment,
                                     sk_sp<GrMtlAttachment> resolveAttachment,
                                     Wrapped)
        : GrSurface(gpu, dimensions, GrProtected::kNo)
        , GrRenderTarget(gpu, dimensions, colorAttachment->numSamples(), GrProtected::kNo)
        , fColorAttachment(std::move(colorAttachment))
        , fResolveAttachment(std::move(resolveAttachment)) {
    this->registerWithCacheWrapped(GrWrapCacheable::kNo);
}

// Called by subclass constructors.
GrMtlRenderTarget::GrMtlRenderTarget(GrMtlGpu* gpu,
                                     SkISize dimensions,
                                     sk_sp<GrMtlAttachment> colorAttachment,
                                     sk_sp<GrMtlAttachment> resolveAttachment)
        : GrSurface(gpu, dimensions, GrProtected::kNo)
        , GrRenderTarget(gpu, dimensions, colorAttachment->numSamples(), GrProtected::kNo)
        , fColorAttachment(std::move(colorAttachment))
        , fResolveAttachment(std::move(resolveAttachment)) {
}

sk_sp<GrMtlRenderTarget> GrMtlRenderTarget::MakeWrappedRenderTarget(GrMtlGpu* gpu,
                                                                    SkISize dimensions,
                                                                    int sampleCnt,
                                                                    id<MTLTexture> texture) {
    SkASSERT(nil != texture);
    SkASSERT(1 == texture.mipmapLevelCount);
    if (@available(macOS 10.11, iOS 9.0, *)) {
        SkASSERT(MTLTextureUsageRenderTarget & texture.usage);
    }

    sk_sp<GrMtlAttachment> textureAttachment =
            GrMtlAttachment::MakeWrapped(gpu, dimensions, texture,
                                         GrAttachment::UsageFlags::kColorAttachment,
                                         GrWrapCacheable::kNo);

    GrMtlRenderTarget* mtlRT;
    if (sampleCnt > 1) {
        if ([texture sampleCount] == 1) {
            MTLPixelFormat format = texture.pixelFormat;
            if (!gpu->mtlCaps().isFormatRenderable(format, sampleCnt)) {
                return nullptr;
            }
            sk_sp<GrMtlAttachment> colorAttachment = GrMtlAttachment::MakeMSAA(gpu,
                                                                               dimensions,
                                                                               sampleCnt,
                                                                               format);
            if (!colorAttachment) {
                return nullptr;
            }
            mtlRT = new GrMtlRenderTarget(
                    gpu, dimensions, std::move(colorAttachment), std::move(textureAttachment),
                    kWrapped);
            mtlRT->setRequiresManualMSAAResolve();
        } else {
            SkASSERT(sampleCnt == static_cast<int>([texture sampleCount]));
            mtlRT = new GrMtlRenderTarget(gpu, dimensions, std::move(textureAttachment), nil,
                                          kWrapped);
        }
    } else {
        mtlRT = new GrMtlRenderTarget(gpu, dimensions, std::move(textureAttachment), nil,
                                      kWrapped);
    }

    return sk_sp<GrMtlRenderTarget>(mtlRT);
}

GrMtlRenderTarget::~GrMtlRenderTarget() {
    SkASSERT(nil == fColorAttachment);
    SkASSERT(nil == fResolveAttachment);
}

GrBackendRenderTarget GrMtlRenderTarget::getBackendRenderTarget() const {
    GrMtlTextureInfo info;
    info.fTexture.reset(GrRetainPtrFromId(fColorAttachment->mtlTexture()));
    return GrBackendRenderTarget(this->width(), this->height(), info);
}

GrBackendFormat GrMtlRenderTarget::backendFormat() const {
    return GrBackendFormat::MakeMtl(fColorAttachment->mtlFormat());
}

GrMtlGpu* GrMtlRenderTarget::getMtlGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrMtlGpu*>(this->getGpu());
}

void GrMtlRenderTarget::onAbandon() {
    fColorAttachment = nil;
    fResolveAttachment = nil;
    INHERITED::onAbandon();
}

void GrMtlRenderTarget::onRelease() {
    fColorAttachment = nil;
    fResolveAttachment = nil;
    INHERITED::onRelease();
}

bool GrMtlRenderTarget::completeStencilAttachment(GrAttachment* stencil, bool useMSAASurface) {
    SkASSERT(useMSAASurface == (this->numSamples() > 1));
    return true;
}

GR_NORETAIN_END
