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
                                   GrColorType colorType,
                                   SkAlphaType alphaType,
                                   sk_sp<SkColorSpace> colorSpace)
        : fContext(context), fColorSpaceInfo(colorType, alphaType, std::move(colorSpace)) {}

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

bool GrSurfaceContext::readPixels(const GrPixelInfo& origDstInfo, void* dst, size_t rowBytes,
                                  SkIPoint pt, GrContext* direct) {
    ASSERT_SINGLE_OWNER
    RETURN_FALSE_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(this->auditTrail(), "GrSurfaceContext::readPixels");

    if (!direct && !(direct = fContext->priv().asDirectContext())) {
        return false;
    }

    if (!dst) {
        return false;
    }

    size_t tightRowBytes = origDstInfo.minRowBytes();
    if (!rowBytes) {
        rowBytes = tightRowBytes;
    } else if (rowBytes < tightRowBytes) {
        return false;
    }

    if (!origDstInfo.isValid()) {
        return false;
    }

    GrSurfaceProxy* srcProxy = this->asSurfaceProxy();

    // MDB TODO: delay this instantiation until later in the method
    if (!srcProxy->instantiate(direct->priv().resourceProvider())) {
        return false;
    }

    GrSurface* srcSurface = srcProxy->peekSurface();

    auto dstInfo = origDstInfo;
    if (!dstInfo.clip(this->width(), this->height(), &pt, &dst, rowBytes)) {
        return false;
    }
    // Our tight row bytes may have been changed by clipping.
    tightRowBytes = dstInfo.minRowBytes();

    bool premul   = this->colorSpaceInfo().alphaType() == kUnpremul_SkAlphaType &&
                    dstInfo.alphaType() == kPremul_SkAlphaType;
    bool unpremul = this->colorSpaceInfo().alphaType() == kPremul_SkAlphaType &&
                    dstInfo.alphaType() == kUnpremul_SkAlphaType;

    bool needColorConversion = SkColorSpaceXformSteps::Required(this->colorSpaceInfo().colorSpace(),
                                                                dstInfo.colorSpace());

    const GrCaps* caps = direct->priv().caps();
    // This is the getImageData equivalent to the canvas2D putImageData fast path. We probably don't
    // care so much about getImageData performance. However, in order to ensure putImageData/
    // getImageData in "legacy" mode are round-trippable we use the GPU to do the complementary
    // unpremul step to writeSurfacePixels's premul step (which is determined empirically in
    // fContext->vaildaPMUPMConversionExists()).
    bool canvas2DFastPath = unpremul && !needColorConversion &&
                            (GrColorType::kRGBA_8888 == dstInfo.colorType() ||
                             GrColorType::kBGRA_8888 == dstInfo.colorType()) &&
                            SkToBool(srcProxy->asTextureProxy()) &&
                            (srcProxy->config() == kRGBA_8888_GrPixelConfig ||
                             srcProxy->config() == kBGRA_8888_GrPixelConfig) &&
                            caps->isConfigRenderable(kRGBA_8888_GrPixelConfig) &&
                            direct->priv().validPMUPMConversionExists();

    auto readFlag = caps->surfaceSupportsReadPixels(srcSurface);
    if (readFlag == GrCaps::SurfaceReadPixelsSupport::kUnsupported) {
        return false;
    }

    if (readFlag == GrCaps::SurfaceReadPixelsSupport::kCopyToTexture2D || canvas2DFastPath) {
        GrColorType colorType = canvas2DFastPath ? GrColorType::kRGBA_8888
                                                 : this->colorSpaceInfo().colorType();
        sk_sp<SkColorSpace> cs = canvas2DFastPath ? nullptr
                                                  : this->colorSpaceInfo().refColorSpace();

        sk_sp<GrRenderTargetContext> tempCtx = direct->priv().makeDeferredRenderTargetContext(
                SkBackingFit::kApprox, dstInfo.width(), dstInfo.height(), colorType, std::move(cs),
                1, GrMipMapped::kNo, kTopLeft_GrSurfaceOrigin, nullptr, SkBudgeted::kYes);
        if (!tempCtx) {
            return false;
        }

        std::unique_ptr<GrFragmentProcessor> fp;
        if (canvas2DFastPath) {
            fp = direct->priv().createPMToUPMEffect(
                    GrSimpleTextureEffect::Make(sk_ref_sp(srcProxy->asTextureProxy()),
                                                SkMatrix::I()));
            if (dstInfo.colorType() == GrColorType::kBGRA_8888) {
                fp = GrFragmentProcessor::SwizzleOutput(std::move(fp), GrSwizzle::BGRA());
                dstInfo = dstInfo.makeColorType(GrColorType::kRGBA_8888);
            }
            // The render target context is incorrectly tagged as kPremul even though we're writing
            // unpremul data thanks to the PMToUPM effect. Fake out the dst alpha type so we don't
            // double unpremul.
            dstInfo = dstInfo.makeAlphaType(kPremul_SkAlphaType);
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
                SkRect::MakeWH(dstInfo.width(), dstInfo.height()),
                SkRect::MakeXYWH(pt.fX, pt.fY, dstInfo.width(), dstInfo.height()));

        return tempCtx->readPixels(dstInfo, dst, rowBytes, {0, 0}, direct);
    }

    bool flip = srcProxy->origin() == kBottomLeft_GrSurfaceOrigin;

    auto supportedRead = caps->supportedReadPixelsColorType(
            this->colorSpaceInfo().colorType(), srcProxy->backendFormat(), dstInfo.colorType());

    bool makeTight = !caps->readPixelsRowBytesSupport() && tightRowBytes != rowBytes;

    bool convert = unpremul || premul || needColorConversion || flip || makeTight ||
                   (dstInfo.colorType() != supportedRead.fColorType) ||
                   supportedRead.fSwizzle != GrSwizzle::RGBA();

    std::unique_ptr<char[]> tmpPixels;
    GrPixelInfo tmpInfo;
    void* readDst = dst;
    size_t readRB = rowBytes;
    if (convert) {
        tmpInfo = {supportedRead.fColorType, this->colorSpaceInfo().alphaType(),
                   this->colorSpaceInfo().refColorSpace(), dstInfo.width(), dstInfo.height()};
        size_t tmpRB = tmpInfo.minRowBytes();
        size_t size = tmpRB * tmpInfo.height();
        // Chrome MSAN bots require the data to be initialized (hence the ()).
        tmpPixels.reset(new char[size]());

        readDst = tmpPixels.get();
        readRB = tmpRB;
        pt.fY = flip ? srcSurface->height() - pt.fY - dstInfo.height() : pt.fY;
    }

    direct->priv().flushSurface(srcProxy);

    if (!direct->priv().getGpu()->readPixels(srcSurface, pt.fX, pt.fY, dstInfo.width(),
                                             dstInfo.height(), supportedRead.fColorType, readDst,
                                             readRB)) {
        return false;
    }

    if (convert) {
        return GrConvertPixels(dstInfo, dst, rowBytes, tmpInfo, readDst, readRB, flip,
                               supportedRead.fSwizzle);
    }
    return true;
}

bool GrSurfaceContext::writePixels(const GrPixelInfo& origSrcInfo, const void* src, size_t rowBytes,
                                   SkIPoint pt, GrContext* direct) {
    ASSERT_SINGLE_OWNER
    RETURN_FALSE_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(this->auditTrail(), "GrSurfaceContext::writePixels");

    if (!direct && !(direct = fContext->priv().asDirectContext())) {
        return false;
    }

    if (this->asSurfaceProxy()->readOnly()) {
        return false;
    }

    if (!src) {
        return false;
    }

    size_t tightRowBytes = origSrcInfo.minRowBytes();
    if (!rowBytes) {
        rowBytes = tightRowBytes;
    } else if (rowBytes < tightRowBytes) {
        return false;
    }

    if (!origSrcInfo.isValid()) {
        return false;
    }

    GrSurfaceProxy* dstProxy = this->asSurfaceProxy();
    if (!dstProxy->instantiate(direct->priv().resourceProvider())) {
        return false;
    }

    GrSurface* dstSurface = dstProxy->peekSurface();

    auto srcInfo = origSrcInfo;
    if (!srcInfo.clip(this->width(), this->height(), &pt, &src, rowBytes)) {
        return false;
    }
    // Our tight row bytes may have been changed by clipping.
    tightRowBytes = srcInfo.minRowBytes();

    bool premul = this->colorSpaceInfo().alphaType() == kPremul_SkAlphaType &&
            srcInfo.alphaType() == kUnpremul_SkAlphaType;
    bool unpremul = this->colorSpaceInfo().alphaType() == kUnpremul_SkAlphaType &&
            srcInfo.alphaType() == kPremul_SkAlphaType;

    bool needColorConversion = SkColorSpaceXformSteps::Required(
            srcInfo.colorSpace(), this->colorSpaceInfo().colorSpace());

    const GrCaps* caps = direct->priv().caps();
    // For canvas2D putImageData performance we have a special code path for unpremul RGBA_8888 srcs
    // that are premultiplied on the GPU. This is kept as narrow as possible for now.
    bool canvas2DFastPath = !caps->avoidWritePixelsFastPath() && premul && !needColorConversion &&
                            (srcInfo.colorType() == GrColorType::kRGBA_8888 ||
                             srcInfo.colorType() == GrColorType::kBGRA_8888) &&
                            SkToBool(this->asRenderTargetContext()) &&
                            (dstProxy->config() == kRGBA_8888_GrPixelConfig ||
                             dstProxy->config() == kBGRA_8888_GrPixelConfig) &&
                            direct->priv().caps()->isConfigTexturable(kRGBA_8888_GrPixelConfig) &&
                            direct->priv().validPMUPMConversionExists();

    if (!caps->surfaceSupportsWritePixels(dstSurface) || canvas2DFastPath) {
        GrSurfaceDesc desc;
        desc.fWidth = srcInfo.width();
        desc.fHeight = srcInfo.height();
        desc.fSampleCnt = 1;
        GrColorType colorType;

        GrBackendFormat format;
        SkAlphaType alphaType;
        if (canvas2DFastPath) {
            desc.fConfig = kRGBA_8888_GrPixelConfig;
            colorType = GrColorType::kRGBA_8888;
            format = caps->getBackendFormatFromColorType(colorType);
            alphaType = kUnpremul_SkAlphaType;
        } else {
            desc.fConfig =  dstProxy->config();
            colorType = this->colorSpaceInfo().colorType();
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
                format, desc, GrRenderable::kNo, tempOrigin, SkBackingFit::kApprox,
                SkBudgeted::kYes);

        if (!tempProxy) {
            return false;
        }
        auto tempCtx = direct->priv().drawingManager()->makeTextureContext(
                tempProxy, colorType, alphaType, this->colorSpaceInfo().refColorSpace());
        if (!tempCtx) {
            return false;
        }

        // In the fast path we always write the srcData to the temp context as though it were RGBA.
        // When the data is really BGRA the write will cause the R and B channels to be swapped in
        // the intermediate surface which gets corrected by a swizzle effect when drawing to the
        // dst.
        if (canvas2DFastPath) {
            srcInfo = srcInfo.makeColorType(GrColorType::kRGBA_8888);
        }
        if (!tempCtx->writePixels(srcInfo, src, rowBytes, {0, 0}, direct)) {
            return false;
        }

        if (this->asRenderTargetContext()) {
            std::unique_ptr<GrFragmentProcessor> fp;
            if (canvas2DFastPath) {
                fp = direct->priv().createUPMToPMEffect(
                        GrSimpleTextureEffect::Make(std::move(tempProxy), SkMatrix::I()));
                // Important: check the original src color type here!
                if (origSrcInfo.colorType() == GrColorType::kBGRA_8888) {
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
                    SkRect::MakeXYWH(pt.fX, pt.fY, srcInfo.width(), srcInfo.height()),
                    SkRect::MakeWH(srcInfo.width(), srcInfo.height()));
        } else {
            SkIRect srcRect = SkIRect::MakeWH(srcInfo.width(), srcInfo.height());
            SkIPoint dstPoint = SkIPoint::Make(pt.fX, pt.fY);
            if (!this->copy(tempProxy.get(), srcRect, dstPoint)) {
                return false;
            }
        }
        return true;
    }

    GrColorType allowedColorType =
            caps->supportedWritePixelsColorType(dstProxy->config(), srcInfo.colorType());
    bool flip = dstProxy->origin() == kBottomLeft_GrSurfaceOrigin;
    bool makeTight = !caps->writePixelsRowBytesSupport() && rowBytes != tightRowBytes;
    bool convert = premul || unpremul || needColorConversion || makeTight ||
                   (srcInfo.colorType() != allowedColorType) || flip;

    std::unique_ptr<char[]> tmpPixels;
    GrColorType srcColorType = srcInfo.colorType();
    if (convert) {
        GrPixelInfo tmpInfo(allowedColorType, this->colorSpaceInfo().alphaType(),
                            this->colorSpaceInfo().refColorSpace(), srcInfo.width(),
                            srcInfo.height());
        auto tmpRB = tmpInfo.minRowBytes();
        tmpPixels.reset(new char[tmpRB * tmpInfo.height()]);

        GrConvertPixels(tmpInfo, tmpPixels.get(), tmpRB, srcInfo, src, rowBytes, flip);

        srcColorType = tmpInfo.colorType();
        rowBytes = tmpRB;
        src = tmpPixels.get();
        pt.fY = flip ? dstSurface->height() - pt.fY - tmpInfo.height() : pt.fY;
    }

    // On platforms that prefer flushes over VRAM use (i.e., ANGLE) we're better off forcing a
    // complete flush here. On platforms that prefer VRAM use over flushes we're better off
    // giving the drawing manager the chance of skipping the flush (i.e., by passing in the
    // destination proxy)
    // TODO: should this policy decision just be moved into the drawing manager?
    direct->priv().flushSurface(caps->preferVRAMUseOverFlushes() ? dstProxy : nullptr);

    return direct->priv().getGpu()->writePixels(dstSurface, pt.fX, pt.fY, srcInfo.width(),
                                                srcInfo.height(), srcColorType, src, rowBytes);
}

bool GrSurfaceContext::copy(GrSurfaceProxy* src, const SkIRect& srcRect, const SkIPoint& dstPoint) {
    ASSERT_SINGLE_OWNER
    RETURN_FALSE_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(this->auditTrail(), "GrSurfaceContextPriv::copy");

    const GrCaps* caps = fContext->priv().caps();

    SkASSERT(src->backendFormat().textureType() != GrTextureType::kExternal);
    SkASSERT(src->origin() == this->asSurfaceProxy()->origin());
    SkASSERT(caps->makeConfigSpecific(src->config(), src->backendFormat()) ==
             caps->makeConfigSpecific(this->asSurfaceProxy()->config(),
                                      this->asSurfaceProxy()->backendFormat()));

    GrSurfaceProxy* dst = this->asSurfaceProxy();

    if (!caps->canCopySurface(dst, src, srcRect, dstPoint)) {
        return false;
    }

    return this->getOpList()->copySurface(fContext, dst, src, srcRect, dstPoint);
}

