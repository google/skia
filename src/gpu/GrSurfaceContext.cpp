/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrSurfaceContext.h"

#include "include/private/GrRecordingContext.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/gpu/GrAuditTrail.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrDataUtils.h"
#include "src/gpu/GrDrawingManager.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrOpList.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrSurfaceContextPriv.h"
#include "src/gpu/GrSurfacePriv.h"
#include "src/gpu/GrTextureContext.h"
#include "src/gpu/SkGr.h"

#define ASSERT_SINGLE_OWNER \
    SkDEBUGCODE(GrSingleOwner::AutoEnforce debug_SingleOwner(this->singleOwner());)
#define RETURN_FALSE_IF_ABANDONED  if (this->fContext->priv().abandoned()) { return false; }

// In MDB mode the reffing of the 'getLastOpList' call's result allows in-progress
// GrOpLists to be picked up and added to by renderTargetContexts lower in the call
// stack. When this occurs with a closed GrOpList, a new one will be allocated
// when the renderTargetContext attempts to use it (via getOpList).
GrSurfaceContext::GrSurfaceContext(GrRecordingContext* context,
                                   SkAlphaType alphaType,
                                   sk_sp<SkColorSpace> colorSpace,
                                   GrPixelConfig config)
        : fContext(context), fColorSpaceInfo(alphaType, std::move(colorSpace), config) {}

GrAuditTrail* GrSurfaceContext::auditTrail() {
    return fContext->priv().auditTrail();
}

GrDrawingManager* GrSurfaceContext::drawingManager() {
    return fContext->priv().drawingManager();
}

const GrDrawingManager* GrSurfaceContext::drawingManager() const {
    return fContext->priv().drawingManager();
}

#ifdef SK_DEBUG
GrSingleOwner* GrSurfaceContext::singleOwner() {
    return fContext->priv().singleOwner();
}
#endif
static bool valid_premul_color_type(GrColorType ct) {
    switch (ct) {
        case GrColorType::kUnknown:          return false;
        case GrColorType::kAlpha_8:          return false;
        case GrColorType::kBGR_565:          return false;
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
        case GrColorType::kR_16:             return false;
        case GrColorType::kRG_1616:          return false;
        // Experimental (for Y416 and mutant P016/P010)
        case GrColorType::kRGBA_16161616:    return false;
        case GrColorType::kRG_F16:           return false;
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
        case kR_16_GrPixelConfig:               return false;
        case kRG_1616_GrPixelConfig:            return false;
        // Experimental (for Y416 and mutant P016/P010)
        case kRGBA_16161616_GrPixelConfig:      return false;
        case kRG_half_GrPixelConfig:            return false;
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

bool GrSurfaceContext::readPixelsImpl(GrContext* direct, int left, int top, int width,
                                      int height, GrColorType dstColorType,
                                      SkColorSpace* dstColorSpace, void* buffer, size_t rowBytes,
                                      uint32_t pixelOpsFlags) {
    SkASSERT(buffer);

    GrSurfaceProxy* srcProxy = this->asSurfaceProxy();

    // MDB TODO: delay this instantiation until later in the method
    if (!srcProxy->instantiate(direct->priv().resourceProvider())) {
        return false;
    }

    GrSurface* srcSurface = srcProxy->peekSurface();

    if (!GrSurfacePriv::AdjustReadPixelParams(srcSurface->width(), srcSurface->height(),
                                              GrColorTypeBytesPerPixel(dstColorType), &left, &top,
                                              &width, &height, &buffer, &rowBytes)) {
        return false;
    }

    // TODO: Pass dst buffer's alpha type.
    bool unpremul = SkToBool(kUnpremul_PixelOpsFlag & pixelOpsFlags);
    SkASSERT(!unpremul || this->colorSpaceInfo().alphaType() != kUnpremul_SkAlphaType);

    if (!valid_pixel_conversion(dstColorType, srcProxy->config(), unpremul)) {
        return false;
    }

    bool needColorConversion =
            SkColorSpaceXformSteps::Required(this->colorSpaceInfo().colorSpace(), dstColorSpace);

    const GrCaps* caps = direct->priv().caps();
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
            caps->isConfigRenderable(kRGBA_8888_GrPixelConfig) &&
            direct->priv().validPMUPMConversionExists();

    auto readFlag = caps->surfaceSupportsReadPixels(srcSurface);
    if (readFlag == GrCaps::kProtected_ReadFlag) {
        return false;
    }

    if (readFlag == GrCaps::kRequiresCopy_ReadFlag || canvas2DFastPath) {
        GrBackendFormat format;
        GrPixelConfig config;
        if (canvas2DFastPath) {
            config = kRGBA_8888_GrPixelConfig;
            format = caps->getBackendFormatFromColorType(kRGBA_8888_SkColorType);
        } else {
            config = srcProxy->config();
            format = srcProxy->backendFormat().makeTexture2D();
            if (!format.isValid()) {
                return false;
            }
        }
        sk_sp<SkColorSpace> cs = canvas2DFastPath ? nullptr : this->colorSpaceInfo().refColorSpace();

        sk_sp<GrRenderTargetContext> tempCtx = direct->priv().makeDeferredRenderTargetContext(
                format, SkBackingFit::kApprox, width, height, config, std::move(cs), 1,
                GrMipMapped::kNo, kTopLeft_GrSurfaceOrigin, nullptr, SkBudgeted::kYes);
        if (!tempCtx) {
            return false;
        }

        std::unique_ptr<GrFragmentProcessor> fp;
        if (canvas2DFastPath) {
            fp = direct->priv().createPMToUPMEffect(
                    GrSimpleTextureEffect::Make(sk_ref_sp(srcProxy->asTextureProxy()),
                                                SkMatrix::I()));
            if (dstColorType == GrColorType::kBGRA_8888) {
                fp = GrFragmentProcessor::SwizzleOutput(std::move(fp), GrSwizzle::BGRA());
                dstColorType = GrColorType::kRGBA_8888;
            }
        } else {
            fp = GrSimpleTextureEffect::Make(sk_ref_sp(srcProxy->asTextureProxy()), SkMatrix::I());
        }
        if (!fp) {
            return false;
        }
        GrPaint paint;
        paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
        paint.addColorFragmentProcessor(std::move(fp));

        tempCtx->asRenderTargetContext()->fillRectToRect(
                GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(),
                SkRect::MakeWH(width, height), SkRect::MakeXYWH(left, top, width, height));

        uint32_t flags = canvas2DFastPath ? 0 : pixelOpsFlags;
        return tempCtx->readPixelsImpl(direct, 0, 0, width, height, dstColorType, dstColorSpace,
                                       buffer, rowBytes, flags);
    }

    bool flip = srcProxy->origin() == kBottomLeft_GrSurfaceOrigin;
    auto supportedRead = caps->supportedReadPixelsColorType(
            srcProxy->config(), srcProxy->backendFormat(), dstColorType);

    bool convert = unpremul || needColorConversion || flip ||
                   (dstColorType != supportedRead.fColorType) ||
                   supportedRead.fSwizzle != GrSwizzle::RGBA();

    std::unique_ptr<char[]> tmpPixels;
    GrPixelInfo tmpInfo;
    GrPixelInfo dstInfo;
    void* readDst = buffer;
    if (convert) {
        tmpInfo.fColorInfo.fColorType = supportedRead.fColorType;
        tmpInfo.fColorInfo.fAlphaType = kPremul_SkAlphaType;
        tmpInfo.fColorInfo.fColorSpace = this->colorSpaceInfo().colorSpace();
        tmpInfo.fOrigin = srcProxy->origin();
        tmpInfo.fRowBytes = GrColorTypeBytesPerPixel(supportedRead.fColorType) * width;

        dstInfo.fColorInfo.fColorType = dstColorType;
        dstInfo.fColorInfo.fAlphaType = unpremul ? kUnpremul_SkAlphaType : kPremul_SkAlphaType;
        dstInfo.fColorInfo.fColorSpace = dstColorSpace;
        dstInfo.fOrigin = kTopLeft_GrSurfaceOrigin;
        dstInfo.fRowBytes = rowBytes;

        dstInfo.fWidth = tmpInfo.fWidth = width;
        dstInfo.fHeight = tmpInfo.fHeight = height;

        size_t size = tmpInfo.fRowBytes * height;
        tmpPixels.reset(new char[size]);
        // Chrome MSAN bots require this.
        sk_bzero(tmpPixels.get(), size);
        readDst = tmpPixels.get();
        rowBytes = tmpInfo.fRowBytes;
        top = flip ? srcSurface->height() - top - height : top;
    }

    direct->priv().flushSurface(srcProxy);

    if (!direct->priv().getGpu()->readPixels(srcSurface, left, top, width, height,
                                             supportedRead.fColorType, readDst, rowBytes)) {
        return false;
    }

    if (convert) {
        return GrConvertPixels(dstInfo, buffer, tmpInfo, tmpPixels.get(), supportedRead.fSwizzle);
    }
    return true;
}

bool GrSurfaceContext::readPixels(const SkImageInfo& dstInfo, void* dstBuffer,
                                  size_t dstRowBytes, int x, int y, uint32_t flags) {
    ASSERT_SINGLE_OWNER
    RETURN_FALSE_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(this->auditTrail(), "GrSurfaceContext::readPixels");

    // TODO: this seems to duplicate code in SkImage_Gpu::onReadPixels
    if (kUnpremul_SkAlphaType == dstInfo.alphaType() &&
        !GrPixelConfigIsOpaque(this->asSurfaceProxy()->config())) {
        flags |= kUnpremul_PixelOpsFlag;
    }
    auto colorType = SkColorTypeToGrColorType(dstInfo.colorType());
    if (GrColorType::kUnknown == colorType) {
        return false;
    }

    auto direct = fContext->priv().asDirectContext();
    if (!direct) {
        return false;
    }

    return this->readPixelsImpl(direct, x, y, dstInfo.width(), dstInfo.height(), colorType,
                                dstInfo.colorSpace(), dstBuffer, dstRowBytes, flags);
}

bool GrSurfaceContext::writePixelsImpl(GrContext* direct, int left, int top, int width, int height,
                                       GrColorType srcColorType, SkColorSpace* srcColorSpace,
                                       const void* srcBuffer, size_t srcRowBytes,
                                       uint32_t pixelOpsFlags) {
    if (GrColorType::kUnknown == srcColorType) {
        return false;
    }

    GrSurfaceProxy* dstProxy = this->asSurfaceProxy();
    if (!dstProxy->instantiate(direct->priv().resourceProvider())) {
        return false;
    }

    GrSurface* dstSurface = dstProxy->peekSurface();

    if (!GrSurfacePriv::AdjustWritePixelParams(dstSurface->width(), dstSurface->height(),
                                               GrColorTypeBytesPerPixel(srcColorType), &left, &top,
                                               &width, &height, &srcBuffer, &srcRowBytes)) {
        return false;
    }

    // TODO: Pass src buffer's alpha type.
    bool premul = SkToBool(kUnpremul_PixelOpsFlag & pixelOpsFlags);
    SkASSERT(!premul || this->colorSpaceInfo().alphaType() != kUnpremul_SkAlphaType);

    bool needColorConversion =
            SkColorSpaceXformSteps::Required(srcColorSpace, this->colorSpaceInfo().colorSpace());

    const GrCaps* caps = direct->priv().caps();
    // For canvas2D putImageData performance we have a special code path for unpremul RGBA_8888 srcs
    // that are premultiplied on the GPU. This is kept as narrow as possible for now.
    bool canvas2DFastPath =
            !caps->avoidWritePixelsFastPath() &&
            premul &&
            !needColorConversion &&
            (srcColorType == GrColorType::kRGBA_8888 || srcColorType == GrColorType::kBGRA_8888) &&
            SkToBool(this->asRenderTargetContext()) &&
            (dstProxy->config() == kRGBA_8888_GrPixelConfig ||
             dstProxy->config() == kBGRA_8888_GrPixelConfig) &&
            direct->priv().caps()->isConfigTexturable(kRGBA_8888_GrPixelConfig) &&
            direct->priv().validPMUPMConversionExists();

    if (!caps->surfaceSupportsWritePixels(dstSurface) || canvas2DFastPath) {
        GrSurfaceDesc desc;
        desc.fWidth = width;
        desc.fHeight = height;
        desc.fSampleCnt = 1;

        GrBackendFormat format;
        SkAlphaType alphaType;
        if (canvas2DFastPath) {
            desc.fConfig = kRGBA_8888_GrPixelConfig;
            format = caps->getBackendFormatFromColorType(kRGBA_8888_SkColorType);
            alphaType = kUnpremul_SkAlphaType;
        } else {
            desc.fConfig =  dstProxy->config();
            format = dstProxy->backendFormat().makeTexture2D();
            if (!format.isValid()) {
                return false;
            }
            alphaType = this->colorSpaceInfo().alphaType();
        }

        // It is more efficient for us to write pixels into a top left origin so we prefer that.
        // However, if the final proxy isn't a render target then we must use a copy to move the
        // data into it which requires the origins to match. If the final proxy is a render target
        // we can use a draw instead which doesn't have this origin restriction. Thus for render
        // targets we will use top left and otherwise we will make the origins match.
        GrSurfaceOrigin tempOrigin =
                this->asRenderTargetContext() ? kTopLeft_GrSurfaceOrigin : dstProxy->origin();
        auto tempProxy = direct->priv().proxyProvider()->createProxy(
                format, desc, tempOrigin, SkBackingFit::kApprox, SkBudgeted::kYes);

        if (!tempProxy) {
            return false;
        }
        auto tempCtx = direct->priv().drawingManager()->makeTextureContext(
                tempProxy, alphaType, this->colorSpaceInfo().refColorSpace());
        if (!tempCtx) {
            return false;
        }
        uint32_t flags = canvas2DFastPath ? 0 : pixelOpsFlags;

        // In the fast path we always write the srcData to the temp context as though it were RGBA.
        // When the data is really BGRA the write will cause the R and B channels to be swapped in
        // the intermediate surface which gets corrected by a swizzle effect when drawing to the
        // dst.
        auto tmpColorType = canvas2DFastPath ? GrColorType::kRGBA_8888 : srcColorType;
        if (!tempCtx->writePixelsImpl(direct, 0, 0, width, height, tmpColorType, srcColorSpace,
                                      srcBuffer, srcRowBytes, flags)) {
            return false;
        }

        if (this->asRenderTargetContext()) {
            std::unique_ptr<GrFragmentProcessor> fp;
            if (canvas2DFastPath) {
                fp = direct->priv().createUPMToPMEffect(
                        GrSimpleTextureEffect::Make(std::move(tempProxy), SkMatrix::I()));
                if (srcColorType == GrColorType::kBGRA_8888) {
                    fp = GrFragmentProcessor::SwizzleOutput(std::move(fp), GrSwizzle::BGRA());
                }
            } else {
                fp = GrSimpleTextureEffect::Make(std::move(tempProxy), SkMatrix::I());
            }
            if (!fp) {
                return false;
            }
            GrPaint paint;
            paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
            paint.addColorFragmentProcessor(std::move(fp));
            this->asRenderTargetContext()->fillRectToRect(
                    GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(),
                    SkRect::MakeXYWH(left, top, width, height), SkRect::MakeWH(width, height));
        } else {
            SkIRect srcRect = SkIRect::MakeWH(width, height);
            SkIPoint dstPoint = SkIPoint::Make(left, top);
            if (!this->copy(tempProxy.get(), srcRect, dstPoint)) {
                return false;
            }
        }
        return true;
    }

    if (!valid_pixel_conversion(srcColorType, dstProxy->config(), premul)) {
        return false;
    }

    GrColorType allowedColorType = caps->supportedWritePixelsColorType(dstProxy->config(),
                                                                       srcColorType);
    bool convert = premul || needColorConversion || (srcColorType != allowedColorType) ||
                   dstProxy->origin() == kBottomLeft_GrSurfaceOrigin;

    std::unique_ptr<char[]> tmpPixels;
    if (convert) {
        GrPixelInfo srcInfo;
        srcInfo.fColorInfo.fAlphaType = (premul ? kUnpremul_SkAlphaType : kPremul_SkAlphaType);
        srcInfo.fColorInfo.fColorType = srcColorType;
        srcInfo.fColorInfo.fColorSpace = srcColorSpace;
        srcInfo.fRowBytes = srcRowBytes;
        srcInfo.fOrigin = kTopLeft_GrSurfaceOrigin;

        GrPixelInfo tmpInfo;
        tmpInfo.fColorInfo.fAlphaType = kPremul_SkAlphaType;
        tmpInfo.fColorInfo.fColorType = allowedColorType;
        tmpInfo.fColorInfo.fColorSpace = this->colorSpaceInfo().colorSpace();
        tmpInfo.fRowBytes = GrColorTypeBytesPerPixel(allowedColorType) * width;
        tmpInfo.fOrigin = dstProxy->origin();

        srcInfo.fWidth  = tmpInfo.fWidth  = width;
        srcInfo.fHeight = tmpInfo.fHeight = height;

        tmpPixels.reset(new char[tmpInfo.fRowBytes * height]);

        GrConvertPixels(tmpInfo, tmpPixels.get(), srcInfo, srcBuffer);

        srcColorType = tmpInfo.fColorInfo.fColorType;
        srcBuffer = tmpPixels.get();
        srcRowBytes = tmpInfo.fRowBytes;
        if (dstProxy->origin() == kBottomLeft_GrSurfaceOrigin) {
            top = dstSurface->height() - top - height;
        }
    }

    // On platforms that prefer flushes over VRAM use (i.e., ANGLE) we're better off forcing a
    // complete flush here. On platforms that prefer VRAM use over flushes we're better off
    // giving the drawing manager the chance of skipping the flush (i.e., by passing in the
    // destination proxy)
    // TODO: should this policy decision just be moved into the drawing manager?
    direct->priv().flushSurface(caps->preferVRAMUseOverFlushes() ? dstProxy : nullptr);

    return direct->priv().getGpu()->writePixels(dstSurface, left, top, width, height, srcColorType,
                                                srcBuffer, srcRowBytes);
}

bool GrSurfaceContext::writePixels(const SkImageInfo& srcInfo, const void* srcBuffer,
                                   size_t srcRowBytes, int x, int y, uint32_t flags) {
    ASSERT_SINGLE_OWNER
    RETURN_FALSE_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(this->auditTrail(), "GrSurfaceContext::writePixels");

    if (kUnpremul_SkAlphaType == srcInfo.alphaType()) {
        flags |= kUnpremul_PixelOpsFlag;
    }
    auto colorType = SkColorTypeToGrColorType(srcInfo.colorType());
    if (GrColorType::kUnknown == colorType) {
        return false;
    }

    auto direct = fContext->priv().asDirectContext();
    if (!direct) {
        return false;
    }

    if (this->asSurfaceProxy()->readOnly()) {
        return false;
    }

    return this->writePixelsImpl(direct, x, y, srcInfo.width(), srcInfo.height(), colorType,
                                 srcInfo.colorSpace(), srcBuffer, srcRowBytes, flags);
}

bool GrSurfaceContext::copy(GrSurfaceProxy* src, const SkIRect& srcRect, const SkIPoint& dstPoint) {
    ASSERT_SINGLE_OWNER
    RETURN_FALSE_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(this->auditTrail(), "GrSurfaceContextPriv::copy");

    SkASSERT(src->backendFormat().textureType() != GrTextureType::kExternal);
    SkASSERT(src->origin() == this->asSurfaceProxy()->origin());
    SkASSERT(src->config() == this->asSurfaceProxy()->config());

    GrSurfaceProxy* dst = this->asSurfaceProxy();

    if (!fContext->priv().caps()->canCopySurface(dst, src, srcRect, dstPoint)) {
        return false;
    }

    return this->getOpList()->copySurface(fContext, dst, src, srcRect, dstPoint);
}

