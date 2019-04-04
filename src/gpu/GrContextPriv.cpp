/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrContextPriv.h"

#include "GrClip.h"
#include "GrContextThreadSafeProxy.h"
#include "GrContextThreadSafeProxyPriv.h"
#include "GrDrawingManager.h"
#include "GrGpu.h"
#include "GrMemoryPool.h"
#include "GrRenderTargetContext.h"
#include "GrSkSLFPFactoryCache.h"
#include "GrSurfacePriv.h"
#include "GrTexture.h"
#include "GrTextureContext.h"
#include "SkAutoPixmapStorage.h"
#include "SkImage_Base.h"
#include "SkImage_Gpu.h"
#include "SkGr.h"
#include "text/GrTextBlobCache.h"

#define ASSERT_OWNED_PROXY_PRIV(P) \
    SkASSERT(!(P) || !((P)->peekTexture()) || (P)->peekTexture()->getContext() == fContext)
#define ASSERT_SINGLE_OWNER_PRIV \
    SkDEBUGCODE(GrSingleOwner::AutoEnforce debug_SingleOwner(fContext->singleOwner());)
#define RETURN_IF_ABANDONED_PRIV if (fContext->abandoned()) { return; }
#define RETURN_FALSE_IF_ABANDONED_PRIV if (fContext->abandoned()) { return false; }

sk_sp<const GrCaps> GrContextPriv::refCaps() const {
    return fContext->refCaps();
}

sk_sp<GrSkSLFPFactoryCache> GrContextPriv::fpFactoryCache() {
    return fContext->fpFactoryCache();
}

sk_sp<GrOpMemoryPool> GrContextPriv::refOpMemoryPool() {
    return fContext->refOpMemoryPool();
}

void GrContextPriv::addOnFlushCallbackObject(GrOnFlushCallbackObject* onFlushCBObject) {
    fContext->addOnFlushCallbackObject(onFlushCBObject);
}

sk_sp<GrSurfaceContext> GrContextPriv::makeWrappedSurfaceContext(
                                                    sk_sp<GrSurfaceProxy> proxy,
                                                    sk_sp<SkColorSpace> colorSpace,
                                                    const SkSurfaceProps* props) {
    return fContext->makeWrappedSurfaceContext(std::move(proxy), std::move(colorSpace), props);
}

sk_sp<GrSurfaceContext> GrContextPriv::makeDeferredSurfaceContext(
                                                    const GrBackendFormat& format,
                                                    const GrSurfaceDesc& dstDesc,
                                                    GrSurfaceOrigin origin,
                                                    GrMipMapped mipMapped,
                                                    SkBackingFit fit,
                                                    SkBudgeted isDstBudgeted,
                                                    sk_sp<SkColorSpace> colorSpace,
                                                    const SkSurfaceProps* props) {
    return fContext->makeDeferredSurfaceContext(format, dstDesc, origin, mipMapped, fit,
                                                isDstBudgeted, std::move(colorSpace), props);
}

sk_sp<GrRenderTargetContext> GrContextPriv::makeDeferredRenderTargetContext(
                                        const GrBackendFormat& format,
                                        SkBackingFit fit,
                                        int width, int height,
                                        GrPixelConfig config,
                                        sk_sp<SkColorSpace> colorSpace,
                                        int sampleCnt,
                                        GrMipMapped mipMapped,
                                        GrSurfaceOrigin origin,
                                        const SkSurfaceProps* surfaceProps,
                                        SkBudgeted budgeted) {
    return fContext->makeDeferredRenderTargetContext(format, fit, width, height, config,
                                                     std::move(colorSpace), sampleCnt, mipMapped,
                                                     origin, surfaceProps, budgeted);
}

sk_sp<GrRenderTargetContext> GrContextPriv::makeDeferredRenderTargetContextWithFallback(
                                        const GrBackendFormat& format,
                                        SkBackingFit fit,
                                        int width, int height,
                                        GrPixelConfig config,
                                        sk_sp<SkColorSpace> colorSpace,
                                        int sampleCnt,
                                        GrMipMapped mipMapped,
                                        GrSurfaceOrigin origin,
                                        const SkSurfaceProps* surfaceProps,
                                        SkBudgeted budgeted) {
    return fContext->makeDeferredRenderTargetContextWithFallback(format, fit, width, height, config,
                                                                 std::move(colorSpace), sampleCnt,
                                                                 mipMapped, origin, surfaceProps,
                                                                 budgeted);
}

sk_sp<GrTextureContext> GrContextPriv::makeBackendTextureContext(const GrBackendTexture& tex,
                                                                 GrSurfaceOrigin origin,
                                                                 sk_sp<SkColorSpace> colorSpace) {
    ASSERT_SINGLE_OWNER_PRIV

    sk_sp<GrSurfaceProxy> proxy = this->proxyProvider()->wrapBackendTexture(
            tex, origin, kBorrow_GrWrapOwnership, GrWrapCacheable::kNo, kRW_GrIOType);
    if (!proxy) {
        return nullptr;
    }

    return this->drawingManager()->makeTextureContext(std::move(proxy), std::move(colorSpace));
}

sk_sp<GrRenderTargetContext> GrContextPriv::makeBackendTextureRenderTargetContext(
                                                                   const GrBackendTexture& tex,
                                                                   GrSurfaceOrigin origin,
                                                                   int sampleCnt,
                                                                   sk_sp<SkColorSpace> colorSpace,
                                                                   const SkSurfaceProps* props,
                                                                   ReleaseProc releaseProc,
                                                                   ReleaseContext releaseCtx) {
    ASSERT_SINGLE_OWNER_PRIV
    SkASSERT(sampleCnt > 0);

    sk_sp<GrTextureProxy> proxy(this->proxyProvider()->wrapRenderableBackendTexture(
            tex, origin, sampleCnt, kBorrow_GrWrapOwnership, GrWrapCacheable::kNo, releaseProc,
            releaseCtx));
    if (!proxy) {
        return nullptr;
    }

    return this->drawingManager()->makeRenderTargetContext(std::move(proxy),
                                                           std::move(colorSpace), props);
}

sk_sp<GrRenderTargetContext> GrContextPriv::makeBackendRenderTargetRenderTargetContext(
                                                const GrBackendRenderTarget& backendRT,
                                                GrSurfaceOrigin origin,
                                                sk_sp<SkColorSpace> colorSpace,
                                                const SkSurfaceProps* surfaceProps,
                                                ReleaseProc releaseProc,
                                                ReleaseContext releaseCtx) {
    ASSERT_SINGLE_OWNER_PRIV

    sk_sp<GrSurfaceProxy> proxy = this->proxyProvider()->wrapBackendRenderTarget(
            backendRT, origin, releaseProc, releaseCtx);
    if (!proxy) {
        return nullptr;
    }

    return this->drawingManager()->makeRenderTargetContext(std::move(proxy),
                                                           std::move(colorSpace),
                                                           surfaceProps);
}

sk_sp<GrRenderTargetContext> GrContextPriv::makeBackendTextureAsRenderTargetRenderTargetContext(
                                                     const GrBackendTexture& tex,
                                                     GrSurfaceOrigin origin,
                                                     int sampleCnt,
                                                     sk_sp<SkColorSpace> colorSpace,
                                                     const SkSurfaceProps* props) {
    ASSERT_SINGLE_OWNER_PRIV
    SkASSERT(sampleCnt > 0);
    sk_sp<GrSurfaceProxy> proxy(
            this->proxyProvider()->wrapBackendTextureAsRenderTarget(tex, origin, sampleCnt));
    if (!proxy) {
        return nullptr;
    }

    return this->drawingManager()->makeRenderTargetContext(std::move(proxy),
                                                           std::move(colorSpace),
                                                           props);
}

sk_sp<GrRenderTargetContext> GrContextPriv::makeVulkanSecondaryCBRenderTargetContext(
        const SkImageInfo& imageInfo, const GrVkDrawableInfo& vkInfo, const SkSurfaceProps* props) {
    ASSERT_SINGLE_OWNER_PRIV
    sk_sp<GrSurfaceProxy> proxy(
            this->proxyProvider()->wrapVulkanSecondaryCBAsRenderTarget(imageInfo, vkInfo));
    if (!proxy) {
        return nullptr;
    }

    return this->drawingManager()->makeRenderTargetContext(std::move(proxy),
                                                           imageInfo.refColorSpace(),
                                                           props);
}

void GrContextPriv::flush(GrSurfaceProxy* proxy) {
    ASSERT_SINGLE_OWNER_PRIV
    RETURN_IF_ABANDONED_PRIV
    ASSERT_OWNED_PROXY_PRIV(proxy);

    fContext->drawingManager()->flush(proxy, SkSurface::BackendSurfaceAccess::kNoAccess,
                                      SkSurface::kNone_FlushFlags, 0, nullptr);
}

void GrContextPriv::flushSurfaceWrites(GrSurfaceProxy* proxy) {
    ASSERT_SINGLE_OWNER_PRIV
    RETURN_IF_ABANDONED_PRIV
    SkASSERT(proxy);
    ASSERT_OWNED_PROXY_PRIV(proxy);
    if (proxy->priv().hasPendingWrite()) {
        this->flush(proxy);
    }
}

void GrContextPriv::flushSurfaceIO(GrSurfaceProxy* proxy) {
    ASSERT_SINGLE_OWNER_PRIV
    RETURN_IF_ABANDONED_PRIV
    SkASSERT(proxy);
    ASSERT_OWNED_PROXY_PRIV(proxy);
    if (proxy->priv().hasPendingIO()) {
        this->flush(proxy);
    }
}

void GrContextPriv::prepareSurfaceForExternalIO(GrSurfaceProxy* proxy) {
    ASSERT_SINGLE_OWNER_PRIV
    RETURN_IF_ABANDONED_PRIV
    SkASSERT(proxy);
    ASSERT_OWNED_PROXY_PRIV(proxy);
    fContext->drawingManager()->prepareSurfaceForExternalIO(proxy,
            SkSurface::BackendSurfaceAccess::kNoAccess, SkSurface::kNone_FlushFlags, 0, nullptr);
}

static bool valid_premul_color_type(GrColorType ct) {
    switch (ct) {
        case GrColorType::kUnknown:          return false;
        case GrColorType::kAlpha_8:          return false;
        case GrColorType::kRGB_565:          return false;
        case GrColorType::kABGR_4444:        return true;
        case GrColorType::kRGBA_8888:        return true;
        case GrColorType::kRGB_888x:         return false;
        case GrColorType::kRG_88:            return false;
        case GrColorType::kBGRA_8888:        return true;
        case GrColorType::kRGBA_1010102:     return true;
        case GrColorType::kGray_8:           return false;
        case GrColorType::kAlpha_F16:        return false;
        case GrColorType::kRGBA_F16:         return true;
        case GrColorType::kRGBA_F16_Clamped: return true;
        case GrColorType::kRG_F32:           return false;
        case GrColorType::kRGBA_F32:         return true;
        case GrColorType::kRGB_ETC1:         return false;
    }
    SK_ABORT("Invalid GrColorType");
    return false;
}

// TODO: This will be removed when GrSurfaceContexts are aware of their color types.
// (skbug.com/6718)
static bool valid_premul_config(GrPixelConfig config) {
    switch (config) {
        case kUnknown_GrPixelConfig:            return false;
        case kAlpha_8_GrPixelConfig:            return false;
        case kGray_8_GrPixelConfig:             return false;
        case kRGB_565_GrPixelConfig:            return false;
        case kRGBA_4444_GrPixelConfig:          return true;
        case kRGBA_8888_GrPixelConfig:          return true;
        case kRGB_888_GrPixelConfig:            return false;
        case kRGB_888X_GrPixelConfig:           return false;
        case kRG_88_GrPixelConfig:              return false;
        case kBGRA_8888_GrPixelConfig:          return true;
        case kSRGBA_8888_GrPixelConfig:         return true;
        case kSBGRA_8888_GrPixelConfig:         return true;
        case kRGBA_1010102_GrPixelConfig:       return true;
        case kRGBA_float_GrPixelConfig:         return true;
        case kRG_float_GrPixelConfig:           return false;
        case kAlpha_half_GrPixelConfig:         return false;
        case kRGBA_half_GrPixelConfig:          return true;
        case kRGBA_half_Clamped_GrPixelConfig:  return true;
        case kRGB_ETC1_GrPixelConfig:           return false;
        case kAlpha_8_as_Alpha_GrPixelConfig:   return false;
        case kAlpha_8_as_Red_GrPixelConfig:     return false;
        case kAlpha_half_as_Red_GrPixelConfig:  return false;
        case kGray_8_as_Lum_GrPixelConfig:      return false;
        case kGray_8_as_Red_GrPixelConfig:      return false;
    }
    SK_ABORT("Invalid GrPixelConfig");
    return false;
}

static bool valid_pixel_conversion(GrColorType cpuColorType, GrPixelConfig gpuConfig,
                                   bool premulConversion) {
    // We only allow premul <-> unpremul conversions for some formats
    if (premulConversion &&
        (!valid_premul_color_type(cpuColorType) || !valid_premul_config(gpuConfig))) {
        return false;
    }
    return true;
}

bool GrContextPriv::readSurfacePixels(GrSurfaceContext* src, int left, int top, int width,
                                      int height, GrColorType dstColorType,
                                      SkColorSpace* dstColorSpace, void* buffer, size_t rowBytes,
                                      uint32_t pixelOpsFlags) {
    ASSERT_SINGLE_OWNER_PRIV
    RETURN_FALSE_IF_ABANDONED_PRIV
    SkASSERT(src);
    SkASSERT(buffer);
    ASSERT_OWNED_PROXY_PRIV(src->asSurfaceProxy());
    GR_CREATE_TRACE_MARKER_CONTEXT("GrContextPriv", "readSurfacePixels", fContext);

    // MDB TODO: delay this instantiation until later in the method
    if (!src->asSurfaceProxy()->instantiate(this->resourceProvider(), true)) {
        return false;
    }

    GrSurfaceProxy* srcProxy = src->asSurfaceProxy();
    GrSurface* srcSurface = srcProxy->peekSurface();

    if (!GrSurfacePriv::AdjustReadPixelParams(srcSurface->width(), srcSurface->height(),
                                              GrColorTypeBytesPerPixel(dstColorType), &left, &top,
                                              &width, &height, &buffer, &rowBytes)) {
        return false;
    }

    // TODO: Make GrSurfaceContext know its alpha type and pass dst buffer's alpha type.
    bool unpremul = SkToBool(kUnpremul_PixelOpsFlag & pixelOpsFlags);

    if (!valid_pixel_conversion(dstColorType, srcProxy->config(), unpremul)) {
        return false;
    }

    bool needColorConversion =
            SkColorSpaceXformSteps::Required(src->colorSpaceInfo().colorSpace(), dstColorSpace);

    // This is the getImageData equivalent to the canvas2D putImageData fast path. We probably don't
    // care so much about getImageData performance. However, in order to ensure putImageData/
    // getImageData in "legacy" mode are round-trippable we use the GPU to do the complementary
    // unpremul step to writeSurfacePixels's premul step (which is determined empirically in
    // fContext->vaildaPMUPMConversionExists()).
    bool canvas2DFastPath =
            unpremul &&
            !needColorConversion &&
            (GrColorType::kRGBA_8888 == dstColorType || GrColorType::kBGRA_8888 == dstColorType) &&
            SkToBool(srcProxy->asTextureProxy()) &&
            (srcProxy->config() == kRGBA_8888_GrPixelConfig ||
             srcProxy->config() == kBGRA_8888_GrPixelConfig) &&
            fContext->priv().caps()->isConfigRenderable(kRGBA_8888_GrPixelConfig) &&
            fContext->validPMUPMConversionExists();

    if (!fContext->priv().caps()->surfaceSupportsReadPixels(srcSurface) ||
        canvas2DFastPath) {
        GrSurfaceDesc desc;
        desc.fWidth = width;
        desc.fHeight = height;
        desc.fSampleCnt = 1;

        GrBackendFormat format;
        if (canvas2DFastPath) {
            desc.fFlags = kRenderTarget_GrSurfaceFlag;
            desc.fConfig = kRGBA_8888_GrPixelConfig;
            format = this->caps()->getBackendFormatFromColorType(kRGBA_8888_SkColorType);
        } else {
            desc.fFlags = kNone_GrSurfaceFlags;
            desc.fConfig = srcProxy->config();
            format = srcProxy->backendFormat().makeTexture2D();
            if (!format.isValid()) {
                return false;
            }
        }

        auto tempProxy = this->proxyProvider()->createProxy(
                format, desc, kTopLeft_GrSurfaceOrigin, SkBackingFit::kApprox, SkBudgeted::kYes);
        if (!tempProxy) {
            return false;
        }
        sk_sp<GrSurfaceContext> tempCtx;
        if (canvas2DFastPath) {
            tempCtx = this->drawingManager()->makeRenderTargetContext(std::move(tempProxy), nullptr,
                                                                      nullptr);
            SkASSERT(tempCtx->asRenderTargetContext());
            tempCtx->asRenderTargetContext()->discard();
        } else {
            tempCtx = this->drawingManager()->makeTextureContext(
                    std::move(tempProxy), src->colorSpaceInfo().refColorSpace());
        }
        if (!tempCtx) {
            return false;
        }
        if (canvas2DFastPath) {
            GrPaint paint;
            paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
            auto fp = fContext->createPMToUPMEffect(
                    GrSimpleTextureEffect::Make(sk_ref_sp(srcProxy->asTextureProxy()),
                                                SkMatrix::I()));
            if (dstColorType == GrColorType::kBGRA_8888) {
                fp = GrFragmentProcessor::SwizzleOutput(std::move(fp), GrSwizzle::BGRA());
                dstColorType = GrColorType::kRGBA_8888;
            }
            if (!fp) {
                return false;
            }
            paint.addColorFragmentProcessor(std::move(fp));
            tempCtx->asRenderTargetContext()->fillRectToRect(
                    GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(),
                    SkRect::MakeWH(width, height), SkRect::MakeXYWH(left, top, width, height));
        } else if (!tempCtx->copy(srcProxy, SkIRect::MakeXYWH(left, top, width, height), {0, 0})) {
            return false;
        }
        uint32_t flags = canvas2DFastPath ? 0 : pixelOpsFlags;
        return this->readSurfacePixels(tempCtx.get(), 0, 0, width, height, dstColorType,
                                       dstColorSpace, buffer, rowBytes, flags);
    }

    bool convert = unpremul || needColorConversion;

    bool flip = srcProxy->origin() == kBottomLeft_GrSurfaceOrigin;
    if (flip) {
        top = srcSurface->height() - top - height;
    }

    GrColorType allowedColorType = fContext->priv().caps()->supportedReadPixelsColorType(
            srcProxy->config(), dstColorType);
    convert = convert || (dstColorType != allowedColorType);

    SkAutoPixmapStorage tempPixmap;
    SkPixmap finalPixmap;
    if (convert) {
        SkColorType srcSkColorType = GrColorTypeToSkColorType(allowedColorType);
        SkColorType dstSkColorType = GrColorTypeToSkColorType(dstColorType);
        bool srcAlwaysOpaque = SkColorTypeIsAlwaysOpaque(srcSkColorType);
        bool dstAlwaysOpaque = SkColorTypeIsAlwaysOpaque(dstSkColorType);
        if (kUnknown_SkColorType == srcSkColorType || kUnknown_SkColorType == dstSkColorType) {
            return false;
        }
        auto tempAT = srcAlwaysOpaque ? kOpaque_SkAlphaType : kPremul_SkAlphaType;
        auto tempII = SkImageInfo::Make(width, height, srcSkColorType, tempAT,
                                        src->colorSpaceInfo().refColorSpace());
        SkASSERT(!unpremul || !dstAlwaysOpaque);
        auto finalAT = (srcAlwaysOpaque || dstAlwaysOpaque)
                               ? kOpaque_SkAlphaType
                               : unpremul ? kUnpremul_SkAlphaType : kPremul_SkAlphaType;
        auto finalII =
                SkImageInfo::Make(width, height, dstSkColorType, finalAT, sk_ref_sp(dstColorSpace));
        if (!SkImageInfoValidConversion(finalII, tempII)) {
            return false;
        }
        if (!tempPixmap.tryAlloc(tempII)) {
            return false;
        }
        finalPixmap.reset(finalII, buffer, rowBytes);
        buffer = tempPixmap.writable_addr();
        rowBytes = tempPixmap.rowBytes();
        // Chrome msan bots require this.
        sk_bzero(buffer, tempPixmap.computeByteSize());
    }

    if (srcSurface->surfacePriv().hasPendingWrite()) {
        this->flush(nullptr);  // MDB TODO: tighten this
    }

    if (!fContext->fGpu->readPixels(srcSurface, left, top, width, height, allowedColorType, buffer,
                                    rowBytes)) {
        return false;
    }

    if (flip) {
        size_t trimRowBytes = GrColorTypeBytesPerPixel(allowedColorType) * width;
        std::unique_ptr<char[]> row(new char[trimRowBytes]);
        char* upper = reinterpret_cast<char*>(buffer);
        char* lower = reinterpret_cast<char*>(buffer) + (height - 1) * rowBytes;
        for (int y = 0; y < height / 2; ++y, upper += rowBytes, lower -= rowBytes) {
            memcpy(row.get(), upper, trimRowBytes);
            memcpy(upper, lower, trimRowBytes);
            memcpy(lower, row.get(), trimRowBytes);
        }
    }
    if (convert) {
        if (!tempPixmap.readPixels(finalPixmap)) {
            return false;
        }
    }
    return true;
}

bool GrContextPriv::writeSurfacePixels(GrSurfaceContext* dst, int left, int top, int width,
                                       int height, GrColorType srcColorType,
                                       SkColorSpace* srcColorSpace, const void* buffer,
                                       size_t rowBytes, uint32_t pixelOpsFlags) {
    ASSERT_SINGLE_OWNER_PRIV
    RETURN_FALSE_IF_ABANDONED_PRIV
    SkASSERT(dst);
    SkASSERT(buffer);
    ASSERT_OWNED_PROXY_PRIV(dst->asSurfaceProxy());
    GR_CREATE_TRACE_MARKER_CONTEXT("GrContextPriv", "writeSurfacePixels", fContext);

    if (GrColorType::kUnknown == srcColorType) {
        return false;
    }

    if (!dst->asSurfaceProxy()->instantiate(this->resourceProvider(), true)) {
        return false;
    }

    GrSurfaceProxy* dstProxy = dst->asSurfaceProxy();
    GrSurface* dstSurface = dstProxy->peekSurface();

    if (!GrSurfacePriv::AdjustWritePixelParams(dstSurface->width(), dstSurface->height(),
                                               GrColorTypeBytesPerPixel(srcColorType), &left, &top,
                                               &width, &height, &buffer, &rowBytes)) {
        return false;
    }

    // TODO: Make GrSurfaceContext know its alpha type and pass src buffer's alpha type.
    bool premul = SkToBool(kUnpremul_PixelOpsFlag & pixelOpsFlags);

    bool needColorConversion =
            SkColorSpaceXformSteps::Required(srcColorSpace, dst->colorSpaceInfo().colorSpace());

    // For canvas2D putImageData performance we have a special code path for unpremul RGBA_8888 srcs
    // that are premultiplied on the GPU. This is kept as narrow as possible for now.
    bool canvas2DFastPath =
            !fContext->priv().caps()->avoidWritePixelsFastPath() &&
            premul &&
            !needColorConversion &&
            (srcColorType == GrColorType::kRGBA_8888 || srcColorType == GrColorType::kBGRA_8888) &&
            SkToBool(dst->asRenderTargetContext()) &&
            (dstProxy->config() == kRGBA_8888_GrPixelConfig ||
             dstProxy->config() == kBGRA_8888_GrPixelConfig) &&
            fContext->priv().caps()->isConfigTexturable(kRGBA_8888_GrPixelConfig) &&
            fContext->validPMUPMConversionExists();

    const GrCaps* caps = this->caps();
    if (!caps->surfaceSupportsWritePixels(dstSurface) || canvas2DFastPath) {
        GrSurfaceDesc desc;
        desc.fWidth = width;
        desc.fHeight = height;
        desc.fSampleCnt = 1;

        GrBackendFormat format;
        if (canvas2DFastPath) {
            desc.fConfig = kRGBA_8888_GrPixelConfig;
            format =
              fContext->priv().caps()->getBackendFormatFromColorType(kRGBA_8888_SkColorType);
        } else {
            desc.fConfig =  dstProxy->config();
            format = dstProxy->backendFormat().makeTexture2D();
            if (!format.isValid()) {
                return false;
            }
        }

        auto tempProxy = this->proxyProvider()->createProxy(
                format, desc, kTopLeft_GrSurfaceOrigin, SkBackingFit::kApprox, SkBudgeted::kYes);
        if (!tempProxy) {
            return false;
        }
        auto tempCtx = this->drawingManager()->makeTextureContext(
                tempProxy, dst->colorSpaceInfo().refColorSpace());
        if (!tempCtx) {
            return false;
        }
        uint32_t flags = canvas2DFastPath ? 0 : pixelOpsFlags;
        // In the fast path we always write the srcData to the temp context as though it were RGBA.
        // When the data is really BGRA the write will cause the R and B channels to be swapped in
        // the intermediate surface which gets corrected by a swizzle effect when drawing to the
        // dst.
        auto tmpColorType = canvas2DFastPath ? GrColorType::kRGBA_8888 : srcColorType;
        if (!this->writeSurfacePixels(tempCtx.get(), 0, 0, width, height, tmpColorType,
                                      srcColorSpace, buffer, rowBytes, flags)) {
            return false;
        }
        if (canvas2DFastPath) {
            GrPaint paint;
            paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
            auto fp = fContext->createUPMToPMEffect(
                    GrSimpleTextureEffect::Make(std::move(tempProxy), SkMatrix::I()));
            if (srcColorType == GrColorType::kBGRA_8888) {
                fp = GrFragmentProcessor::SwizzleOutput(std::move(fp), GrSwizzle::BGRA());
            }
            if (!fp) {
                return false;
            }
            paint.addColorFragmentProcessor(std::move(fp));
            dst->asRenderTargetContext()->fillRectToRect(
                    GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(),
                    SkRect::MakeXYWH(left, top, width, height), SkRect::MakeWH(width, height));
            return true;
        } else {
            return dst->copy(tempProxy.get(), SkIRect::MakeWH(width, height), {left, top});
        }
    }

    bool convert = premul || needColorConversion;

    if (!valid_pixel_conversion(srcColorType, dstProxy->config(), premul)) {
        return false;
    }

    GrColorType allowedColorType = fContext->priv().caps()->supportedWritePixelsColorType(
            dstProxy->config(), srcColorType);
    convert = convert || (srcColorType != allowedColorType);

    std::unique_ptr<char[]> tempBuffer;
    if (convert) {
        auto srcSkColorType = GrColorTypeToSkColorType(srcColorType);
        auto dstSkColorType = GrColorTypeToSkColorType(allowedColorType);
        if (kUnknown_SkColorType == srcSkColorType || kUnknown_SkColorType == dstSkColorType) {
            return false;
        }
        auto srcAlphaType = SkColorTypeIsAlwaysOpaque(srcSkColorType)
                ? kOpaque_SkAlphaType
                : (premul ? kUnpremul_SkAlphaType : kPremul_SkAlphaType);
        SkPixmap src(SkImageInfo::Make(width, height, srcSkColorType, srcAlphaType,
                                       sk_ref_sp(srcColorSpace)),
                     buffer, rowBytes);
        auto tempSrcII = SkImageInfo::Make(width, height, dstSkColorType, kPremul_SkAlphaType,
                                           dst->colorSpaceInfo().refColorSpace());
        auto size = tempSrcII.computeMinByteSize();
        if (!size) {
            return false;
        }
        tempBuffer.reset(new char[size]);
        SkPixmap tempSrc(tempSrcII, tempBuffer.get(), tempSrcII.minRowBytes());
        if (!src.readPixels(tempSrc)) {
            return false;
        }
        srcColorType = allowedColorType;
        buffer = tempSrc.addr();
        rowBytes = tempSrc.rowBytes();
        if (dstProxy->origin() == kBottomLeft_GrSurfaceOrigin) {
            std::unique_ptr<char[]> row(new char[rowBytes]);
            for (int y = 0; y < height / 2; ++y) {
                memcpy(row.get(), tempSrc.addr(0, y), rowBytes);
                memcpy(tempSrc.writable_addr(0, y), tempSrc.addr(0, height - 1 - y), rowBytes);
                memcpy(tempSrc.writable_addr(0, height - 1 - y), row.get(), rowBytes);
            }
            top = dstSurface->height() - top - height;
        }
    } else if (dstProxy->origin() == kBottomLeft_GrSurfaceOrigin) {
        size_t trimRowBytes = GrColorTypeBytesPerPixel(srcColorType) * width;
        tempBuffer.reset(new char[trimRowBytes * height]);
        char* dst = reinterpret_cast<char*>(tempBuffer.get()) + trimRowBytes * (height - 1);
        const char* src = reinterpret_cast<const char*>(buffer);
        for (int i = 0; i < height; ++i, src += rowBytes, dst -= trimRowBytes) {
            memcpy(dst, src, trimRowBytes);
        }
        buffer = tempBuffer.get();
        rowBytes = trimRowBytes;
        top = dstSurface->height() - top - height;
    }

    if (dstSurface->surfacePriv().hasPendingIO()) {
        this->flush(nullptr);  // MDB TODO: tighten this
    }

    return this->getGpu()->writePixels(dstSurface, left, top, width, height, srcColorType, buffer,
                                       rowBytes);
}

void GrContextPriv::moveOpListsToDDL(SkDeferredDisplayList* ddl) {
    fContext->drawingManager()->moveOpListsToDDL(ddl);
}

void GrContextPriv::copyOpListsFromDDL(const SkDeferredDisplayList* ddl,
                                       GrRenderTargetProxy* newDest) {
    fContext->drawingManager()->copyOpListsFromDDL(ddl, newDest);
}

//////////////////////////////////////////////////////////////////////////////
#ifdef SK_ENABLE_DUMP_GPU
#include "SkJSONWriter.h"
SkString GrContextPriv::dump() const {
    SkDynamicMemoryWStream stream;
    SkJSONWriter writer(&stream, SkJSONWriter::Mode::kPretty);
    writer.beginObject();

    static const char* kBackendStr[] = {
        "Metal",
        "OpenGL",
        "Vulkan",
        "Mock",
    };
    GR_STATIC_ASSERT(0 == (unsigned)GrBackendApi::kMetal);
    GR_STATIC_ASSERT(1 == (unsigned)GrBackendApi::kOpenGL);
    GR_STATIC_ASSERT(2 == (unsigned)GrBackendApi::kVulkan);
    GR_STATIC_ASSERT(3 == (unsigned)GrBackendApi::kMock);
    writer.appendString("backend", kBackendStr[(unsigned)fContext->backend()]);

    writer.appendName("caps");
    fContext->caps()->dumpJSON(&writer);

    writer.appendName("gpu");
    fContext->fGpu->dumpJSON(&writer);

    // Flush JSON to the memory stream
    writer.endObject();
    writer.flush();

    // Null terminate the JSON data in the memory stream
    stream.write8(0);

    // Allocate a string big enough to hold all the data, then copy out of the stream
    SkString result(stream.bytesWritten());
    stream.copyToAndReset(result.writable_str());
    return result;
}
#endif

#if GR_TEST_UTILS
void GrContextPriv::resetGpuStats() const {
#if GR_GPU_STATS
    fContext->fGpu->stats()->reset();
#endif
}

void GrContextPriv::dumpCacheStats(SkString* out) const {
#if GR_CACHE_STATS
    fContext->fResourceCache->dumpStats(out);
#endif
}

void GrContextPriv::dumpCacheStatsKeyValuePairs(SkTArray<SkString>* keys,
                                                SkTArray<double>* values) const {
#if GR_CACHE_STATS
    fContext->fResourceCache->dumpStatsKeyValuePairs(keys, values);
#endif
}

void GrContextPriv::printCacheStats() const {
    SkString out;
    this->dumpCacheStats(&out);
    SkDebugf("%s", out.c_str());
}

void GrContextPriv::dumpGpuStats(SkString* out) const {
#if GR_GPU_STATS
    return fContext->fGpu->stats()->dump(out);
#endif
}

void GrContextPriv::dumpGpuStatsKeyValuePairs(SkTArray<SkString>* keys,
                                              SkTArray<double>* values) const {
#if GR_GPU_STATS
    return fContext->fGpu->stats()->dumpKeyValuePairs(keys, values);
#endif
}

void GrContextPriv::printGpuStats() const {
    SkString out;
    this->dumpGpuStats(&out);
    SkDebugf("%s", out.c_str());
}

void GrContextPriv::testingOnly_setTextBlobCacheLimit(size_t bytes) {
    fContext->priv().getTextBlobCache()->setBudget(bytes);
}

sk_sp<SkImage> GrContextPriv::testingOnly_getFontAtlasImage(GrMaskFormat format, unsigned int index) {
    auto atlasManager = this->getAtlasManager();
    if (!atlasManager) {
        return nullptr;
    }

    unsigned int numActiveProxies;
    const sk_sp<GrTextureProxy>* proxies = atlasManager->getProxies(format, &numActiveProxies);
    if (index >= numActiveProxies || !proxies || !proxies[index]) {
        return nullptr;
    }

    SkASSERT(proxies[index]->priv().isExact());
    sk_sp<SkImage> image(new SkImage_Gpu(sk_ref_sp(fContext), kNeedNewImageUniqueID,
                                         kPremul_SkAlphaType, proxies[index], nullptr));
    return image;
}

void GrContextPriv::testingOnly_purgeAllUnlockedResources() {
    fContext->fResourceCache->purgeAllUnlocked();
}

void GrContextPriv::testingOnly_flushAndRemoveOnFlushCallbackObject(GrOnFlushCallbackObject* cb) {
    fContext->flush();
    fContext->drawingManager()->testingOnly_removeOnFlushCallbackObject(cb);
}
#endif

