/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <cstddef>
#include <cstring>
#include <type_traits>

#include "GrAHardwareBufferImageGenerator.h"
#include "GrBackendSurface.h"
#include "GrBackendTextureImageGenerator.h"
#include "GrBitmapTextureMaker.h"
#include "GrCaps.h"
#include "GrColorSpaceXform.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrGpu.h"
#include "GrImageTextureMaker.h"
#include "GrProxyProvider.h"
#include "GrRenderTargetContext.h"
#include "GrResourceProvider.h"
#include "GrSemaphore.h"
#include "GrSurfacePriv.h"
#include "GrTexture.h"
#include "GrTextureAdjuster.h"
#include "GrTexturePriv.h"
#include "GrTextureProxy.h"
#include "GrTextureProxyPriv.h"
#include "SkAutoPixmapStorage.h"
#include "SkBitmapCache.h"
#include "SkCanvas.h"
#include "SkGr.h"
#include "SkImageInfoPriv.h"
#include "SkImage_GpuYUV.h"
#include "SkMipMap.h"
#include "SkPixelRef.h"
#include "SkReadPixelsRec.h"
#include "SkTraceEvent.h"
#include "effects/GrYUVtoRGBEffect.h"
#include "gl/GrGLTexture.h"

SkImage_GpuYUV::SkImage_GpuYUV(sk_sp<GrContext> context, uint32_t uniqueID, SkAlphaType at,
                         sk_sp<GrTextureProxy> proxy, sk_sp<SkColorSpace> colorSpace,
                         SkBudgeted budgeted)
        : INHERITED(proxy->worstCaseWidth(), proxy->worstCaseHeight(), uniqueID)
        , fContext(std::move(context))
        , fProxy(std::move(proxy))
        , fAlphaType(at)
        , fBudgeted(budgeted)
        , fColorSpace(std::move(colorSpace)) {}

SkImage_GpuYUV::~SkImage_GpuYUV() {}

SkImageInfo SkImage_GpuYUV::onImageInfo() const {
    return SkImageInfo::Make(fProxy->width(), fProxy->height(), this->onColorType(), fAlphaType,
                             fColorSpace);
}

SkColorType SkImage_GpuYUV::onColorType() const {
    SkColorType ct;
    if (!GrPixelConfigToColorType(fProxy->config(), &ct)) {
        ct = kUnknown_SkColorType;
    }
    return ct;
}

bool SkImage_GpuYUV::getROPixels(SkBitmap* dst, SkColorSpace*, CachingHint chint) const {
    if (!fContext->contextPriv().resourceProvider()) {
        // DDL TODO: buffer up the readback so it occurs when the DDL is drawn?
        return false;
    }

    // The SkColorSpace parameter "dstColorSpace" is really just a hint about how/where the bitmap
    // will be used. The client doesn't expect that we convert to that color space, it's intended
    // for codec-backed images, to drive our decoding heuristic. In theory we *could* read directly
    // into that color space (to save the client some effort in whatever they're about to do), but
    // that would make our use of the bitmap cache incorrect (or much less efficient, assuming we
    // rolled the dstColorSpace into the key).
    const auto desc = SkBitmapCacheDesc::Make(this);
    if (SkBitmapCache::Find(desc, dst)) {
        SkASSERT(dst->getGenerationID() == this->uniqueID());
        SkASSERT(dst->isImmutable());
        SkASSERT(dst->getPixels());
        return true;
    }

    SkBitmapCache::RecPtr rec = nullptr;
    SkPixmap pmap;
    if (kAllow_CachingHint == chint) {
        rec = SkBitmapCache::Alloc(desc, this->onImageInfo(), &pmap);
        if (!rec) {
            return false;
        }
    } else {
        if (!dst->tryAllocPixels(this->onImageInfo()) || !dst->peekPixels(&pmap)) {
            return false;
        }
    }

    sk_sp<GrSurfaceContext> sContext = fContext->contextPriv().makeWrappedSurfaceContext(
                                                                                    fProxy,
                                                                                    fColorSpace);
    if (!sContext) {
        return false;
    }

    if (!sContext->readPixels(pmap.info(), pmap.writable_addr(), pmap.rowBytes(), 0, 0)) {
        return false;
    }

    if (rec) {
        SkBitmapCache::Add(std::move(rec), dst);
        this->notifyAddedToRasterCache();
    }
    return true;
}

sk_sp<GrTextureProxy> SkImage_GpuYUV::asTextureProxyRef(GrContext* context,
                                                     const GrSamplerState& params,
                                                     SkColorSpace* dstColorSpace,
                                                     sk_sp<SkColorSpace>* texColorSpace,
                                                     SkScalar scaleAdjust[2]) const {
    if (context->uniqueID() != fContext->uniqueID()) {
        SkASSERT(0);
        return nullptr;
    }

    GrTextureAdjuster adjuster(fContext.get(), fProxy, this->alphaType(), this->uniqueID(),
                               this->fColorSpace.get());
    return adjuster.refTextureProxyForParams(params, dstColorSpace, texColorSpace, scaleAdjust);
}

static void apply_premul(const SkImageInfo& info, void* pixels, size_t rowBytes) {
    switch (info.colorType()) {
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
            break;
        default:
            return; // nothing to do
    }

    // SkColor is not necesarily RGBA or BGRA, but it is one of them on little-endian,
    // and in either case, the alpha-byte is always in the same place, so we can safely call
    // SkPreMultiplyColor()
    //
    SkColor* row = (SkColor*)pixels;
    for (int y = 0; y < info.height(); ++y) {
        for (int x = 0; x < info.width(); ++x) {
            row[x] = SkPreMultiplyColor(row[x]);
        }
        row = (SkColor*)((char*)(row) + rowBytes);
    }
}

GrBackendTexture SkImage_GpuYUV::onGetBackendTexture(bool flushPendingGrContextIO,
                                                  GrSurfaceOrigin* origin) const {
    SkASSERT(fProxy);

    if (!fContext->contextPriv().resourceProvider() && !fProxy->isInstantiated()) {
        // This image was created with a DDL context and cannot be instantiated.
        return GrBackendTexture();
    }

    if (!fProxy->instantiate(fContext->contextPriv().resourceProvider())) {
        return GrBackendTexture(); // invalid
    }

    GrTexture* texture = fProxy->peekTexture();

    if (texture) {
        if (flushPendingGrContextIO) {
            fContext->contextPriv().prepareSurfaceForExternalIO(fProxy.get());
        }
        if (origin) {
            *origin = fProxy->origin();
        }
        return texture->getBackendTexture();
    }
    return GrBackendTexture(); // invalid
}

GrTexture* SkImage_GpuYUV::onGetTexture() const {
    GrTextureProxy* proxy = this->peekProxy();
    if (!proxy) {
        return nullptr;
    }

    if (!fContext->contextPriv().resourceProvider() && !fProxy->isInstantiated()) {
        // This image was created with a DDL context and cannot be instantiated.
        return nullptr;
    }

    if (!proxy->instantiate(fContext->contextPriv().resourceProvider())) {
        return nullptr;
    }

    return proxy->peekTexture();
}

bool SkImage_GpuYUV::onReadPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRB,
                               int srcX, int srcY, CachingHint) const {
    if (!fContext->contextPriv().resourceProvider()) {
        // DDL TODO: buffer up the readback so it occurs when the DDL is drawn?
        return false;
    }

    if (!SkImageInfoValidConversion(dstInfo, this->onImageInfo())) {
        return false;
    }

    SkReadPixelsRec rec(dstInfo, dstPixels, dstRB, srcX, srcY);
    if (!rec.trim(this->width(), this->height())) {
        return false;
    }

    // TODO: this seems to duplicate code in GrTextureContext::onReadPixels and
    // GrRenderTargetContext::onReadPixels
    uint32_t flags = 0;
    if (kUnpremul_SkAlphaType == rec.fInfo.alphaType() && kPremul_SkAlphaType == fAlphaType) {
        // let the GPU perform this transformation for us
        flags = GrContextPriv::kUnpremul_PixelOpsFlag;
    }

    sk_sp<GrSurfaceContext> sContext = fContext->contextPriv().makeWrappedSurfaceContext(
            fProxy, fColorSpace);
    if (!sContext) {
        return false;
    }

    if (!sContext->readPixels(rec.fInfo, rec.fPixels, rec.fRowBytes, rec.fX, rec.fY, flags)) {
        return false;
    }

    // do we have to manually fix-up the alpha channel?
    //      src         dst
    //      unpremul    premul      fix manually
    //      premul      unpremul    done by kUnpremul_PixelOpsFlag
    // all other combos need to change.
    //
    // Should this be handled by Ganesh? todo:?
    //
    if (kPremul_SkAlphaType == rec.fInfo.alphaType() && kUnpremul_SkAlphaType == fAlphaType) {
        apply_premul(rec.fInfo, rec.fPixels, rec.fRowBytes);
    }
    return true;
}

sk_sp<SkImage> SkImage_GpuYUV::onMakeSubset(const SkIRect& subset) const {
    GrSurfaceDesc desc;
    desc.fWidth = subset.width();
    desc.fHeight = subset.height();
    desc.fConfig = fProxy->config();

    sk_sp<GrSurfaceContext> sContext(fContext->contextPriv().makeDeferredSurfaceContext(
            desc, fProxy->origin(), GrMipMapped::kNo, SkBackingFit::kExact, fBudgeted));
    if (!sContext) {
        return nullptr;
    }

    if (!sContext->copy(fProxy.get(), subset, SkIPoint::Make(0, 0))) {
        return nullptr;
    }

    // MDB: this call is okay bc we know 'sContext' was kExact
    return sk_make_sp<SkImage_GpuYUV>(fContext, kNeedNewImageUniqueID,
                                   fAlphaType, sContext->asTextureProxyRef(),
                                   fColorSpace, fBudgeted);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static sk_sp<SkImage> new_wrapped_texture_common(GrContext* ctx,
                                                 const GrBackendTexture& backendTex,
                                                 GrSurfaceOrigin origin,
                                                 SkAlphaType at, sk_sp<SkColorSpace> colorSpace,
                                                 GrWrapOwnership ownership,
                                                 SkImage::TextureReleaseProc releaseProc,
                                                 SkImage::ReleaseContext releaseCtx) {
    if (!backendTex.isValid() || backendTex.width() <= 0 || backendTex.height() <= 0) {
        return nullptr;
    }

    GrProxyProvider* proxyProvider = ctx->contextPriv().proxyProvider();
    sk_sp<GrTextureProxy> proxy = proxyProvider->wrapBackendTexture(backendTex, origin, ownership,
                                                                    releaseProc, releaseCtx);
    if (!proxy) {
        return nullptr;
    }
    return sk_make_sp<SkImage_GpuYUV>(sk_ref_sp(ctx), kNeedNewImageUniqueID, at, std::move(proxy),
                                   std::move(colorSpace), SkBudgeted::kNo);
}

bool validate_backend_texture(GrContext* ctx, const GrBackendTexture& tex, GrPixelConfig* config,
                              SkColorType ct, SkAlphaType at, sk_sp<SkColorSpace> cs) {
    if (!tex.isValid()) {
        return false;
    }
    // TODO: Create a SkImageColorInfo struct for color, alpha, and color space so we don't need to
    // create a fake image info here.
    SkImageInfo info = SkImageInfo::Make(1, 1, ct, at, cs);
    if (!SkImageInfoIsValid(info)) {
        return false;
    }

    return ctx->contextPriv().caps()->validateBackendTexture(tex, ct, config);
}

sk_sp<SkImage> SkImage::MakeFromTexture(GrContext* ctx,
                                        const GrBackendTexture& tex, GrSurfaceOrigin origin,
                                        SkColorType ct, SkAlphaType at, sk_sp<SkColorSpace> cs,
                                        TextureReleaseProc releaseP, ReleaseContext releaseC) {
    if (!ctx) {
        return nullptr;
    }
    GrBackendTexture texCopy = tex;
    if (!validate_backend_texture(ctx, texCopy, &texCopy.fConfig, ct, at, cs)) {
        return nullptr;
    }
    return new_wrapped_texture_common(ctx, texCopy, origin, at, std::move(cs),
                                      kBorrow_GrWrapOwnership, releaseP, releaseC);
}

sk_sp<SkImage> SkImage::MakeFromAdoptedTexture(GrContext* ctx,
                                               const GrBackendTexture& tex, GrSurfaceOrigin origin,
                                               SkColorType ct, SkAlphaType at,
                                               sk_sp<SkColorSpace> cs) {
    if (!ctx || !ctx->contextPriv().resourceProvider()) {
        // We have a DDL context and we don't support adopted textures for them.
        return nullptr;
    }
    GrBackendTexture texCopy = tex;
    if (!validate_backend_texture(ctx, texCopy, &texCopy.fConfig, ct, at, cs)) {
        return nullptr;
    }
    return new_wrapped_texture_common(ctx, texCopy, origin, at, std::move(cs),
                                      kAdopt_GrWrapOwnership, nullptr, nullptr);
}



sk_sp<SkImage> SkImage_GpuYUV::MakeFromYUVATextures(GrContext* ctx,
                                                            SkYUVColorSpace colorSpace,
                                                            const GrBackendTexture yuvaTextures[],
                                                            SkYUVAIndex yuvaIndices[4],
                                                            SkISize size,
                                                            GrSurfaceOrigin origin,
                                                            sk_sp<SkColorSpace> imageColorSpace) {
    const int width = size.width();
    const int height = size.height();

    //*** TODO
    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * This helper holds the normal hard ref for the Release proc as well as a hard ref on the DoneProc.
 * Thus when a GrTexture is being released, it will unref both the ReleaseProc and DoneProc.
 */
class PromiseReleaseProcHelper : public GrReleaseProcHelper {
public:
    PromiseReleaseProcHelper(SkImage_GpuYUV::TextureReleaseProc releaseProc,
                             SkImage_GpuYUV::TextureContext context,
                             sk_sp<GrReleaseProcHelper> doneHelper)
            : INHERITED(releaseProc, context)
            , fDoneProcHelper(std::move(doneHelper)) {}

    void weak_dispose() const override {
        // Call the inherited weak_dispose first so that we call the ReleaseProc before the DoneProc
        // if we hold the last ref to the DoneProc.
        INHERITED::weak_dispose();
        fDoneProcHelper.reset();
    }

private:
    mutable sk_sp<GrReleaseProcHelper> fDoneProcHelper;

    typedef GrReleaseProcHelper INHERITED;
};

/**
 * This helper class manages the ref counting for the the ReleaseProc and DoneProc for promise
 * images. It holds a weak ref on the ReleaseProc (hard refs are owned by GrTextures). The weak ref
 * allows us to reuse an outstanding ReleaseProc (because we dropped our GrTexture but the GrTexture
 * isn't done on the GPU) without needing to call FulfillProc again. It also holds a hard ref on the
 * DoneProc. The idea is that after every flush we may call the ReleaseProc so that the client can
 * free up their GPU memory if they want to. The life time of the DoneProc matches that of any
 * outstanding ReleaseProc as well as the PromiseImageHelper. Thus we won't call the DoneProc until
 * all ReleaseProcs are finished and we are finished with the PromiseImageHelper (i.e. won't call
 * FulfillProc again).
 */
class PromiseImageHelper {
public:
    PromiseImageHelper(SkImage_GpuYUV::TextureFulfillProc fulFillProc,
                       SkImage_GpuYUV::TextureReleaseProc releaseProc,
                       SkImage_GpuYUV::PromiseDoneProc doneProc,
                       SkImage_GpuYUV::TextureContext context)
            : fFulfillProc(fulFillProc)
            , fReleaseProc(releaseProc)
            , fContext(context)
            , fDoneHelper(new GrReleaseProcHelper(doneProc, context)) {}

    void reset() {
        this->resetReleaseHelper();
        fDoneHelper.reset();
    }

    sk_sp<GrTexture> getTexture(GrResourceProvider* resourceProvider, GrPixelConfig config) {
        // Releases the promise helper if there are no outstanding hard refs. This means that we
        // don't have any ReleaseProcs waiting to be called so we will need to do a fulfill.
        if (fReleaseHelper && fReleaseHelper->weak_expired()) {
            this->resetReleaseHelper();
        }

        sk_sp<GrTexture> tex;
        if (!fReleaseHelper) {
            fFulfillProc(fContext, &fBackendTex);
            fBackendTex.fConfig = config;
            if (!fBackendTex.isValid()) {
                // Even though the GrBackendTexture is not valid, we must call the release
                // proc to keep our contract of always calling Fulfill and Release in pairs.
                fReleaseProc(fContext);
                return sk_sp<GrTexture>();
            }

            tex = resourceProvider->wrapBackendTexture(fBackendTex, kBorrow_GrWrapOwnership);
            if (!tex) {
                // Even though the GrBackendTexture is not valid, we must call the release
                // proc to keep our contract of always calling Fulfill and Release in pairs.
                fReleaseProc(fContext);
                return sk_sp<GrTexture>();
            }
            fReleaseHelper = new PromiseReleaseProcHelper(fReleaseProc, fContext, fDoneHelper);
            // Take a weak ref
            fReleaseHelper->weak_ref();
        } else {
            SkASSERT(fBackendTex.isValid());
            tex = resourceProvider->wrapBackendTexture(fBackendTex, kBorrow_GrWrapOwnership);
            if (!tex) {
                // We weren't able to make a texture here, but since we are in this branch
                // of the calls (promiseHelper.fReleaseHelper is valid) there is already a
                // texture out there which will call the release proc so we don't need to
                // call it here.
                return sk_sp<GrTexture>();
            }

            SkAssertResult(fReleaseHelper->try_ref());
        }
        SkASSERT(tex);
        // Pass the hard ref off to the texture
        tex->setRelease(sk_sp<GrReleaseProcHelper>(fReleaseHelper));
        return tex;
    }

private:
    // Weak unrefs fReleaseHelper and sets it to null
    void resetReleaseHelper() {
        if (fReleaseHelper) {
            fReleaseHelper->weak_unref();
            fReleaseHelper = nullptr;
        }
    }

    SkImage_GpuYUV::TextureFulfillProc fFulfillProc;
    SkImage_GpuYUV::TextureReleaseProc fReleaseProc;
    SkImage_GpuYUV::TextureContext     fContext;

    // We cache the GrBackendTexture so that if we deleted the GrTexture but the the release proc
    // has yet not been called (this can happen on Vulkan), then we can create a new texture without
    // needing to call the fulfill proc again.
    GrBackendTexture fBackendTex;
    // The fReleaseHelper is used to track a weak ref on the release proc. This helps us make sure
    // we are always pairing fulfill and release proc calls correctly.
    PromiseReleaseProcHelper* fReleaseHelper = nullptr;
    // We don't want to call the fDoneHelper until we are done with the PromiseImageHelper and all
    // ReleaseHelpers are finished. Thus we hold a hard ref here and we will pass a hard ref to each
    // fReleaseHelper we make.
    sk_sp<GrReleaseProcHelper> fDoneHelper;
};

static GrTextureType TextureTypeFromBackendFormat(const GrBackendFormat& backendFormat) {
    if (const GrGLenum* target = backendFormat.getGLTarget()) {
        return GrGLTexture::TextureTypeFromTarget(*target);
    }
    return GrTextureType::k2D;
}

sk_sp<SkImage> SkImage_GpuYUV::MakePromiseTexture(GrContext* context,
                                               const GrBackendFormat& backendFormat,
                                               int width,
                                               int height,
                                               GrMipMapped mipMapped,
                                               GrSurfaceOrigin origin,
                                               SkColorType colorType,
                                               SkAlphaType alphaType,
                                               sk_sp<SkColorSpace> colorSpace,
                                               TextureFulfillProc textureFulfillProc,
                                               TextureReleaseProc textureReleaseProc,
                                               PromiseDoneProc promiseDoneProc,
                                               TextureContext textureContext) {
    if (!context) {
        return nullptr;
    }

    if (width <= 0 || height <= 0) {
        return nullptr;
    }

    if (!textureFulfillProc || !textureReleaseProc || !promiseDoneProc) {
        return nullptr;
    }

    SkImageInfo info = SkImageInfo::Make(width, height, colorType, alphaType, colorSpace);
    if (!SkImageInfoIsValid(info)) {
        return nullptr;
    }
    GrPixelConfig config = kUnknown_GrPixelConfig;
    if (!context->contextPriv().caps()->getConfigFromBackendFormat(backendFormat, colorType,
                                                                   &config)) {
        return nullptr;
    }

    GrTextureType textureType = TextureTypeFromBackendFormat(backendFormat);

    if (mipMapped == GrMipMapped::kYes && GrTextureTypeHasRestrictedSampling(textureType)) {
        // It is invalid to have a GL_TEXTURE_EXTERNAL or GL_TEXTURE_RECTANGLE and have mips as
        // well.
        return nullptr;
    }

    GrProxyProvider* proxyProvider = context->contextPriv().proxyProvider();

    GrSurfaceDesc desc;
    desc.fWidth = width;
    desc.fHeight = height;
    desc.fConfig = config;

    PromiseImageHelper promiseHelper(textureFulfillProc, textureReleaseProc, promiseDoneProc,
                                     textureContext);

    sk_sp<GrTextureProxy> proxy = proxyProvider->createLazyProxy(
            [promiseHelper, config](GrResourceProvider* resourceProvider) mutable {
                if (!resourceProvider) {
                    promiseHelper.reset();
                    return sk_sp<GrTexture>();
                }

                return promiseHelper.getTexture(resourceProvider, config);
            },
            desc, origin, mipMapped, textureType, GrInternalSurfaceFlags::kNone,
            SkBackingFit::kExact, SkBudgeted::kNo,
            GrSurfaceProxy::LazyInstantiationType::kUninstantiate);

    if (!proxy) {
        return nullptr;
    }

    return sk_make_sp<SkImage_GpuYUV>(sk_ref_sp(context), kNeedNewImageUniqueID, alphaType,
                                   std::move(proxy), std::move(colorSpace), SkBudgeted::kNo);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkImage> SkImage_GpuYUV::onMakeColorSpace(sk_sp<SkColorSpace> target, SkColorType) const {
    sk_sp<SkColorSpace> srcSpace = fColorSpace;
    if (!fColorSpace) {
        if (target->isSRGB()) {
            return sk_ref_sp(const_cast<SkImage*>((SkImage*)this));
        }

        srcSpace = SkColorSpace::MakeSRGB();
    }

    auto xform = GrColorSpaceXformEffect::Make(srcSpace.get(), this->alphaType(), target.get());
    if (!xform) {
        return sk_ref_sp(const_cast<SkImage_GpuYUV*>(this));
    }

    sk_sp<GrRenderTargetContext> renderTargetContext(
            fContext->contextPriv().makeDeferredRenderTargetContext(
                                            SkBackingFit::kExact, this->width(), this->height(),
                                            kRGBA_8888_GrPixelConfig, nullptr));
    if (!renderTargetContext) {
        return nullptr;
    }

    GrPaint paint;
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
    paint.addColorTextureProcessor(fProxy, SkMatrix::I());
    paint.addColorFragmentProcessor(std::move(xform));

    const SkRect rect = SkRect::MakeIWH(this->width(), this->height());

    renderTargetContext->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(), rect);

    if (!renderTargetContext->asTextureProxy()) {
        return nullptr;
    }

    SkAlphaType newAlphaType = (kUnpremul_SkAlphaType == fAlphaType) ? kPremul_SkAlphaType
                                                                     : fAlphaType;
    // MDB: this call is okay bc we know 'renderTargetContext' was exact
    return sk_make_sp<SkImage_GpuYUV>(fContext, kNeedNewImageUniqueID,
                                   newAlphaType, renderTargetContext->asTextureProxyRef(),
                                   std::move(target), fBudgeted);

}

bool SkImage_GpuYUV::onIsValid(GrContext* context) const {
    // The base class has already checked that context isn't abandoned (if it's not nullptr)
    if (fContext->abandoned()) {
        return false;
    }

    if (context && context != fContext.get()) {
        return false;
    }

    return true;
}
