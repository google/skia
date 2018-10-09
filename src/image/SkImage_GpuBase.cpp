/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBackendSurface.h"
#include "GrClip.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrRenderTargetContext.h"
#include "GrTexture.h"
#include "GrTextureAdjuster.h"
#include "SkBitmapCache.h"
#include "SkImage_Gpu.h"
#include "SkImage_GpuBase.h"
#include "SkReadPixelsRec.h"

SkImage_GpuBase::SkImage_GpuBase(sk_sp<GrContext> context, int width, int height, uint32_t uniqueID,
                                 SkAlphaType at, SkBudgeted budgeted, sk_sp<SkColorSpace> cs)
        : INHERITED(width, height, uniqueID)
        , fContext(std::move(context))
        , fAlphaType(at)
        , fBudgeted(budgeted)
        , fColorSpace(std::move(cs)) {}

SkImage_GpuBase::~SkImage_GpuBase() {}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool SkImage_GpuBase::ValidateBackendTexture(GrContext* ctx, const GrBackendTexture& tex,
                                             GrPixelConfig* config, SkColorType ct, SkAlphaType at,
                                             sk_sp<SkColorSpace> cs) {
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

//////////////////////////////////////////////////////////////////////////////////////////////////

bool SkImage_GpuBase::getROPixels(SkBitmap* dst, SkColorSpace*, CachingHint chint) const {
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
        this->asTextureProxyRef(),
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

sk_sp<SkImage> SkImage_GpuBase::onMakeSubset(const SkIRect& subset) const {
    sk_sp<GrSurfaceProxy> proxy = this->asTextureProxyRef();

    GrSurfaceDesc desc;
    desc.fWidth = subset.width();
    desc.fHeight = subset.height();
    desc.fConfig = proxy->config();

    sk_sp<GrSurfaceContext> sContext(fContext->contextPriv().makeDeferredSurfaceContext(
        desc, proxy->origin(), GrMipMapped::kNo, SkBackingFit::kExact, fBudgeted));
    if (!sContext) {
        return nullptr;
    }

    if (!sContext->copy(proxy.get(), subset, SkIPoint::Make(0, 0))) {
        return nullptr;
    }

    // MDB: this call is okay bc we know 'sContext' was kExact
    return sk_make_sp<SkImage_Gpu>(fContext, kNeedNewImageUniqueID,
                                   fAlphaType, sContext->asTextureProxyRef(),
                                   fColorSpace, fBudgeted);
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
        row = (SkColor*)((char*)(row)+rowBytes);
    }
}

bool SkImage_GpuBase::onReadPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRB,
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
        this->asTextureProxyRef(), fColorSpace);
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

sk_sp<GrTextureProxy> SkImage_GpuBase::asTextureProxyRef(GrContext* context,
                                                         const GrSamplerState& params,
                                                         SkColorSpace* dstColorSpace,
                                                         sk_sp<SkColorSpace>* texColorSpace,
                                                         SkScalar scaleAdjust[2]) const {
    if (context->uniqueID() != fContext->uniqueID()) {
        SkASSERT(0);
        return nullptr;
    }

    GrTextureAdjuster adjuster(fContext.get(), this->asTextureProxyRef(), fAlphaType,
                               this->uniqueID(), fColorSpace.get());
    return adjuster.refTextureProxyForParams(params, dstColorSpace, texColorSpace, scaleAdjust);
}

GrBackendTexture SkImage_GpuBase::onGetBackendTexture(bool flushPendingGrContextIO,
                                                      GrSurfaceOrigin* origin) const {
    sk_sp<GrTextureProxy> proxy = this->asTextureProxyRef();
    SkASSERT(proxy);

    if (!fContext->contextPriv().resourceProvider() && !proxy->isInstantiated()) {
        // This image was created with a DDL context and cannot be instantiated.
        return GrBackendTexture();
}

    if (!proxy->instantiate(fContext->contextPriv().resourceProvider())) {
        return GrBackendTexture(); // invalid
    }

    GrTexture* texture = proxy->peekTexture();

    if (texture) {
        if (flushPendingGrContextIO) {
            fContext->contextPriv().prepareSurfaceForExternalIO(proxy.get());
        }
        if (origin) {
            *origin = proxy->origin();
        }
        return texture->getBackendTexture();
    }
    return GrBackendTexture(); // invalid
}

GrTexture* SkImage_GpuBase::onGetTexture() const {
    GrTextureProxy* proxy = this->peekProxy();
    if (!proxy) {
        return nullptr;
    }

    sk_sp<GrTextureProxy> proxyRef = this->asTextureProxyRef();
    if (!fContext->contextPriv().resourceProvider() && !proxyRef->isInstantiated()) {
        // This image was created with a DDL context and cannot be instantiated.
        return nullptr;
    }

    if (!proxy->instantiate(fContext->contextPriv().resourceProvider())) {
        return nullptr;
    }

    return proxy->peekTexture();
}

sk_sp<SkImage> SkImage_GpuBase::onMakeColorSpace(sk_sp<SkColorSpace> target) const {
    SkAlphaType newAlphaType = fAlphaType;
#if defined(SK_LEGACY_MAKE_COLOR_SPACE_IMPL)
    if (kUnpremul_SkAlphaType == fAlphaType) {
        newAlphaType = kPremul_SkAlphaType;
    }
#endif
    auto xform = GrColorSpaceXformEffect::Make(fColorSpace.get(), this->alphaType(),
                                               target.get(), newAlphaType);
    if (!xform) {
        return sk_ref_sp(const_cast<SkImage_GpuBase*>(this));
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
    paint.addColorTextureProcessor(this->asTextureProxyRef(), SkMatrix::I());
    paint.addColorFragmentProcessor(std::move(xform));

    const SkRect rect = SkRect::MakeIWH(this->width(), this->height());

    renderTargetContext->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(), rect);

    if (!renderTargetContext->asTextureProxy()) {
        return nullptr;
    }

    // MDB: this call is okay bc we know 'renderTargetContext' was exact
    return sk_make_sp<SkImage_Gpu>(fContext, kNeedNewImageUniqueID,
                                   newAlphaType, renderTargetContext->asTextureProxyRef(),
                                   std::move(target), fBudgeted);
}

bool SkImage_GpuBase::onIsValid(GrContext* context) const {
    // The base class has already checked that context isn't abandoned (if it's not nullptr)
    if (fContext->abandoned()) {
        return false;
    }

    if (context && context != fContext.get()) {
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
sk_sp<GrTexture> SkPromiseImageHelper::getTexture(GrResourceProvider* resourceProvider,
                                                  GrPixelConfig config) {
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
        fReleaseHelper = new SkPromiseReleaseProcHelper(fReleaseProc, fContext, fDoneHelper);
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
