/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlRenderTarget.h"

#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/mtl/GrMtlFramebuffer.h"
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
            auto rp = gpu->getContext()->priv().resourceProvider();
            sk_sp<GrAttachment> msaaAttachment = rp->makeMSAAAttachment(
                    dimensions, GrBackendFormat::MakeMtl(format), sampleCnt, GrProtected::kNo);
            if (!msaaAttachment) {
                return nullptr;
            }
            sk_sp<GrMtlAttachment> colorAttachment =
                    sk_sp<GrMtlAttachment>(static_cast<GrMtlAttachment*>(msaaAttachment.release()));
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

static int renderpass_features_to_index(bool hasResolve, bool hasStencil) {
    int index = 0;
    if (hasResolve) {
        index += 1;
    }
    if (hasStencil) {
        index += 2;
    }
    return index;
}

const GrMtlFramebuffer* GrMtlRenderTarget::getFramebuffer(bool withResolve,
                                                          bool withStencil) {
    int cacheIndex =
            renderpass_features_to_index(withResolve, withStencil);
    SkASSERT(cacheIndex < GrMtlRenderTarget::kNumCachedFramebuffers);

    if (fCachedFramebuffers[cacheIndex]) {
        return fCachedFramebuffers[cacheIndex].get();
    }

    GrMtlAttachment* resolve = withResolve ? this->resolveAttachment() : nullptr;
    GrMtlAttachment* colorAttachment = this->colorAttachment();

    // Stencil attachment view is stored in the base RT stencil attachment
    GrMtlAttachment* stencil =
            withStencil ? static_cast<GrMtlAttachment*>(this->getStencilAttachment())
                        : nullptr;
    fCachedFramebuffers[cacheIndex] =
            GrMtlFramebuffer::Make(colorAttachment, resolve, stencil);
    return fCachedFramebuffers[cacheIndex].get();
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
