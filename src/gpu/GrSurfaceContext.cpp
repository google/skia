/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrSurfaceContext.h"

#include <memory>

#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkYUVMath.h"
#include "src/gpu/GrAuditTrail.h"
#include "src/gpu/GrColorSpaceXform.h"
#include "src/gpu/GrDataUtils.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrDrawingManager.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrImageInfo.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrBicubicEffect.h"
#include "src/gpu/effects/generated/GrColorMatrixFragmentProcessor.h"

#define ASSERT_SINGLE_OWNER        GR_ASSERT_SINGLE_OWNER(this->singleOwner())
#define RETURN_FALSE_IF_ABANDONED  if (this->fContext->abandoned()) { return false; }

std::unique_ptr<GrSurfaceContext> GrSurfaceContext::Make(GrRecordingContext* context,
                                                         GrSurfaceProxyView readView,
                                                         const GrColorInfo& info) {
    // It is probably not necessary to check if the context is abandoned here since uses of the
    // GrSurfaceContext which need the context will mostly likely fail later on without an issue.
    // However having this hear adds some reassurance in case there is a path doesn't handle an
    // abandoned context correctly. It also lets us early out of some extra work.
    if (context->abandoned()) {
        return nullptr;
    }
    GrSurfaceProxy* proxy = readView.proxy();
    SkASSERT(proxy && proxy->asTextureProxy());

    std::unique_ptr<GrSurfaceContext> surfaceContext;
    if (proxy->asRenderTargetProxy()) {
        // Will we ever want a swizzle that is not the default write swizzle for the format and
        // colorType here? If so we will need to manually pass that in.
        GrSwizzle writeSwizzle;
        if (info.colorType() != GrColorType::kUnknown) {
            writeSwizzle = context->priv().caps()->getWriteSwizzle(proxy->backendFormat(),
                                                                   info.colorType());
        }
        GrSurfaceProxyView writeView(readView.refProxy(), readView.origin(), writeSwizzle);
        if (info.alphaType() == kPremul_SkAlphaType || info.alphaType() == kOpaque_SkAlphaType) {
            surfaceContext = std::make_unique<GrSurfaceDrawContext>(context,
                                                                    std::move(readView),
                                                                    std::move(writeView),
                                                                    info.colorType(),
                                                                    info.refColorSpace(),
                                                                    /*surface props*/ nullptr);
        } else {
            surfaceContext = std::make_unique<GrSurfaceFillContext>(context,
                                                                    std::move(readView),
                                                                    std::move(writeView),
                                                                    info);
        }
    } else {
        surfaceContext = std::make_unique<GrSurfaceContext>(context, std::move(readView), info);
    }
    SkDEBUGCODE(surfaceContext->validate();)
    return surfaceContext;
}

std::unique_ptr<GrSurfaceContext> GrSurfaceContext::Make(GrRecordingContext* context,
                                                         const GrImageInfo& info,
                                                         const GrBackendFormat& format,
                                                         SkBackingFit fit,
                                                         GrSurfaceOrigin origin,
                                                         GrRenderable renderable,
                                                         int sampleCount,
                                                         GrMipmapped mipmapped,
                                                         GrProtected isProtected,
                                                         SkBudgeted budgeted) {
    SkASSERT(context);
    SkASSERT(renderable == GrRenderable::kYes || sampleCount == 1);
    if (context->abandoned()) {
        return nullptr;
    }
    sk_sp<GrTextureProxy> proxy = context->priv().proxyProvider()->createProxy(format,
                                                                               info.dimensions(),
                                                                               renderable,
                                                                               sampleCount,
                                                                               mipmapped,
                                                                               fit,
                                                                               budgeted,
                                                                               isProtected);
    if (!proxy) {
        return nullptr;
    }

    GrSwizzle swizzle;
    if (info.colorType() != GrColorType::kUnknown &&
        !context->priv().caps()->isFormatCompressed(format)) {
        swizzle = context->priv().caps()->getReadSwizzle(format, info.colorType());
    }

    GrSurfaceProxyView view(std::move(proxy), origin, swizzle);
    return GrSurfaceContext::Make(context, std::move(view), info.colorInfo());
}

std::unique_ptr<GrSurfaceContext> GrSurfaceContext::Make(GrRecordingContext* context,
                                                         const GrImageInfo& info,
                                                         SkBackingFit fit,
                                                         GrSurfaceOrigin origin,
                                                         GrRenderable renderable,
                                                         int sampleCount,
                                                         GrMipmapped mipmapped,
                                                         GrProtected isProtected,
                                                         SkBudgeted budgeted) {
    GrBackendFormat format = context->priv().caps()->getDefaultBackendFormat(info.colorType(),
                                                                             renderable);
    return Make(context,
                info,
                format,
                fit,
                origin,
                renderable,
                sampleCount,
                mipmapped,
                isProtected,
                budgeted);
}

// In MDB mode the reffing of the 'getLastOpsTask' call's result allows in-progress
// GrOpsTasks to be picked up and added to by renderTargetContexts lower in the call
// stack. When this occurs with a closed GrOpsTask, a new one will be allocated
// when the surfaceDrawContext attempts to use it (via getOpsTask).
GrSurfaceContext::GrSurfaceContext(GrRecordingContext* context,
                                   GrSurfaceProxyView readView,
                                   const GrColorInfo& info)
        : fContext(context), fReadView(std::move(readView)), fColorInfo(info) {
    SkASSERT(!context->abandoned());
}

const GrCaps* GrSurfaceContext::caps() const { return fContext->priv().caps(); }

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
GrSingleOwner* GrSurfaceContext::singleOwner() const { return fContext->priv().singleOwner(); }
#endif

bool GrSurfaceContext::readPixels(GrDirectContext* dContext, const GrImageInfo& origDstInfo,
                                  void* dst, size_t rowBytes, SkIPoint pt) {
    ASSERT_SINGLE_OWNER
    RETURN_FALSE_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(this->auditTrail(), "GrSurfaceContext::readPixels");
    if (!fContext->priv().matches(dContext)) {
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

    if (srcProxy->framebufferOnly()) {
        return false;
    }

    // MDB TODO: delay this instantiation until later in the method
    if (!srcProxy->instantiate(dContext->priv().resourceProvider())) {
        return false;
    }

    GrSurface* srcSurface = srcProxy->peekSurface();

    auto dstInfo = origDstInfo;
    if (!dstInfo.clip(this->width(), this->height(), &pt, &dst, rowBytes)) {
        return false;
    }
    // Our tight row bytes may have been changed by clipping.
    tightRowBytes = dstInfo.minRowBytes();

    SkColorSpaceXformSteps::Flags flags = SkColorSpaceXformSteps{this->colorInfo(), dstInfo}.flags;
    bool unpremul            = flags.unpremul,
         needColorConversion = flags.linearize || flags.gamut_transform || flags.encode,
         premul              = flags.premul;

    const GrCaps* caps = dContext->priv().caps();
    bool srcIsCompressed = caps->isFormatCompressed(srcSurface->backendFormat());
    // This is the getImageData equivalent to the canvas2D putImageData fast path. We probably don't
    // care so much about getImageData performance. However, in order to ensure putImageData/
    // getImageData in "legacy" mode are round-trippable we use the GPU to do the complementary
    // unpremul step to writeSurfacePixels's premul step (which is determined empirically in
    // fContext->vaildaPMUPMConversionExists()).
    GrBackendFormat defaultRGBAFormat = caps->getDefaultBackendFormat(GrColorType::kRGBA_8888,
                                                                      GrRenderable::kYes);
    GrColorType srcColorType = this->colorInfo().colorType();
    bool canvas2DFastPath = unpremul && !needColorConversion &&
                            (GrColorType::kRGBA_8888 == dstInfo.colorType() ||
                             GrColorType::kBGRA_8888 == dstInfo.colorType()) &&
                            SkToBool(srcProxy->asTextureProxy()) &&
                            (srcColorType == GrColorType::kRGBA_8888 ||
                             srcColorType == GrColorType::kBGRA_8888) &&
                            defaultRGBAFormat.isValid() &&
                            dContext->priv().validPMUPMConversionExists();

    auto readFlag = caps->surfaceSupportsReadPixels(srcSurface);
    if (readFlag == GrCaps::SurfaceReadPixelsSupport::kUnsupported) {
        return false;
    }

    if (readFlag == GrCaps::SurfaceReadPixelsSupport::kCopyToTexture2D || canvas2DFastPath) {
        std::unique_ptr<GrSurfaceContext> tempCtx;
        if (this->asTextureProxy()) {
            GrColorType colorType = (canvas2DFastPath || srcIsCompressed)
                                            ? GrColorType::kRGBA_8888
                                            : this->colorInfo().colorType();
            SkAlphaType alphaType = canvas2DFastPath ? dstInfo.alphaType()
                                                     : this->colorInfo().alphaType();
            GrImageInfo tempInfo(colorType,
                                 alphaType,
                                 this->colorInfo().refColorSpace(),
                                 dstInfo.dimensions());
            auto sfc = GrSurfaceFillContext::Make(dContext, tempInfo, SkBackingFit::kApprox);
            if (!sfc) {
                return false;
            }

            std::unique_ptr<GrFragmentProcessor> fp;
            if (canvas2DFastPath) {
                fp = dContext->priv().createPMToUPMEffect(GrTextureEffect::Make(
                        this->readSurfaceView(), this->colorInfo().alphaType()));
                if (dstInfo.colorType() == GrColorType::kBGRA_8888) {
                    fp = GrFragmentProcessor::SwizzleOutput(std::move(fp), GrSwizzle::BGRA());
                    dstInfo = dstInfo.makeColorType(GrColorType::kRGBA_8888);
                }
            } else {
                fp = GrTextureEffect::Make(this->readSurfaceView(), this->colorInfo().alphaType());
            }
            if (!fp) {
                return false;
            }
            sfc->fillRectToRectWithFP(SkIRect::MakePtSize(pt, dstInfo.dimensions()),
                                      SkIRect::MakeSize(dstInfo.dimensions()),
                                      std::move(fp));
            pt = {0, 0};
            tempCtx = std::move(sfc);
        } else {
            auto restrictions = this->caps()->getDstCopyRestrictions(this->asRenderTargetProxy(),
                                                                     this->colorInfo().colorType());
            sk_sp<GrSurfaceProxy> copy;
            static constexpr auto kFit = SkBackingFit::kExact;
            static constexpr auto kBudgeted = SkBudgeted::kYes;
            static constexpr auto kMipMapped = GrMipMapped::kNo;
            if (restrictions.fMustCopyWholeSrc) {
                copy = GrSurfaceProxy::Copy(fContext, srcProxy, this->origin(), kMipMapped, kFit,
                                            kBudgeted);
            } else {
                auto srcRect = SkIRect::MakeXYWH(pt.fX, pt.fY, dstInfo.width(), dstInfo.height());
                copy = GrSurfaceProxy::Copy(fContext, srcProxy, this->origin(), kMipMapped, srcRect,
                                            kFit, kBudgeted, restrictions.fRectsMustMatch);
                pt = {0, 0};
            }
            if (!copy) {
                return false;
            }
            GrSurfaceProxyView view{std::move(copy), this->origin(), this->readSwizzle()};
            tempCtx = GrSurfaceContext::Make(dContext, std::move(view), this->colorInfo());
            SkASSERT(tempCtx);
        }
        return tempCtx->readPixels(dContext, dstInfo, dst, rowBytes, pt);
    }

    bool flip = this->origin() == kBottomLeft_GrSurfaceOrigin;

    auto supportedRead = caps->supportedReadPixelsColorType(
            this->colorInfo().colorType(), srcProxy->backendFormat(), dstInfo.colorType());

    bool makeTight = !caps->readPixelsRowBytesSupport() && tightRowBytes != rowBytes;

    bool convert = unpremul || premul || needColorConversion || flip || makeTight ||
                   (dstInfo.colorType() != supportedRead.fColorType);

    std::unique_ptr<char[]> tmpPixels;
    GrImageInfo tmpInfo;
    void* readDst = dst;
    size_t readRB = rowBytes;
    if (convert) {
        tmpInfo = {supportedRead.fColorType, this->colorInfo().alphaType(),
                   this->colorInfo().refColorSpace(), dstInfo.width(), dstInfo.height()};
        size_t tmpRB = tmpInfo.minRowBytes();
        size_t size = tmpRB * tmpInfo.height();
        // Chrome MSAN bots require the data to be initialized (hence the ()).
        tmpPixels = std::make_unique<char[]>(size);

        readDst = tmpPixels.get();
        readRB = tmpRB;
        pt.fY = flip ? srcSurface->height() - pt.fY - dstInfo.height() : pt.fY;
    }

    dContext->priv().flushSurface(srcProxy);
    dContext->submit();
    if (!dContext->priv().getGpu()->readPixels(srcSurface, pt.fX, pt.fY, dstInfo.width(),
                                               dstInfo.height(), this->colorInfo().colorType(),
                                               supportedRead.fColorType, readDst, readRB)) {
        return false;
    }

    if (convert) {
        return GrConvertPixels(dstInfo, dst, rowBytes, tmpInfo, readDst, readRB, flip);
    }
    return true;
}

bool GrSurfaceContext::writePixels(GrDirectContext* dContext, const GrImageInfo& origSrcInfo,
                                   const void* src, size_t rowBytes, SkIPoint pt) {
    ASSERT_SINGLE_OWNER
    RETURN_FALSE_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(this->auditTrail(), "GrSurfaceContext::writePixels");

    if (!dContext) {
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

    if (dstProxy->framebufferOnly()) {
        return false;
    }

    if (!dstProxy->instantiate(dContext->priv().resourceProvider())) {
        return false;
    }

    GrSurface* dstSurface = dstProxy->peekSurface();

    auto srcInfo = origSrcInfo;
    if (!srcInfo.clip(this->width(), this->height(), &pt, &src, rowBytes)) {
        return false;
    }
    // Our tight row bytes may have been changed by clipping.
    tightRowBytes = srcInfo.minRowBytes();

    SkColorSpaceXformSteps::Flags flags = SkColorSpaceXformSteps{srcInfo, this->colorInfo()}.flags;
    bool unpremul            = flags.unpremul,
         needColorConversion = flags.linearize || flags.gamut_transform || flags.encode,
         premul              = flags.premul;

    const GrCaps* caps = dContext->priv().caps();

    auto rgbaDefaultFormat = caps->getDefaultBackendFormat(GrColorType::kRGBA_8888,
                                                           GrRenderable::kNo);

    GrColorType dstColorType = this->colorInfo().colorType();
    // For canvas2D putImageData performance we have a special code path for unpremul RGBA_8888 srcs
    // that are premultiplied on the GPU. This is kept as narrow as possible for now.
    bool canvas2DFastPath = !caps->avoidWritePixelsFastPath() && premul && !needColorConversion &&
                            (srcInfo.colorType() == GrColorType::kRGBA_8888 ||
                             srcInfo.colorType() == GrColorType::kBGRA_8888) &&
                            this->asFillContext() &&
                            (dstColorType == GrColorType::kRGBA_8888 ||
                             dstColorType == GrColorType::kBGRA_8888) &&
                            rgbaDefaultFormat.isValid() &&
                            dContext->priv().validPMUPMConversionExists();

    if (!caps->surfaceSupportsWritePixels(dstSurface) || canvas2DFastPath) {
        GrColorInfo tempColorInfo;
        GrBackendFormat format;
        GrSwizzle tempReadSwizzle;
        if (canvas2DFastPath) {
            tempColorInfo = {GrColorType::kRGBA_8888,
                             kUnpremul_SkAlphaType,
                             this->colorInfo().refColorSpace()};
            format = rgbaDefaultFormat;
        } else {
            tempColorInfo = this->colorInfo();
            format = dstProxy->backendFormat().makeTexture2D();
            if (!format.isValid()) {
                return false;
            }
            tempReadSwizzle = this->readSwizzle();
        }

        // It is more efficient for us to write pixels into a top left origin so we prefer that.
        // However, if the final proxy isn't a render target then we must use a copy to move the
        // data into it which requires the origins to match. If the final proxy is a render target
        // we can use a draw instead which doesn't have this origin restriction. Thus for render
        // targets we will use top left and otherwise we will make the origins match.
        GrSurfaceOrigin tempOrigin =
                this->asFillContext() ? kTopLeft_GrSurfaceOrigin : this->origin();
        auto tempProxy = dContext->priv().proxyProvider()->createProxy(
                format, srcInfo.dimensions(), GrRenderable::kNo, 1, GrMipmapped::kNo,
                SkBackingFit::kApprox, SkBudgeted::kYes, GrProtected::kNo);
        if (!tempProxy) {
            return false;
        }
        GrSurfaceProxyView tempView(tempProxy, tempOrigin, tempReadSwizzle);
        GrSurfaceContext tempCtx(dContext, tempView, tempColorInfo);

        // In the fast path we always write the srcData to the temp context as though it were RGBA.
        // When the data is really BGRA the write will cause the R and B channels to be swapped in
        // the intermediate surface which gets corrected by a swizzle effect when drawing to the
        // dst.
        if (canvas2DFastPath) {
            srcInfo = srcInfo.makeColorType(GrColorType::kRGBA_8888);
        }
        if (!tempCtx.writePixels(dContext, srcInfo, src, rowBytes, {0, 0})) {
            return false;
        }

        if (this->asFillContext()) {
            std::unique_ptr<GrFragmentProcessor> fp;
            if (canvas2DFastPath) {
                fp = dContext->priv().createUPMToPMEffect(
                        GrTextureEffect::Make(std::move(tempView), tempColorInfo.alphaType()));
                // Important: check the original src color type here!
                if (origSrcInfo.colorType() == GrColorType::kBGRA_8888) {
                    fp = GrFragmentProcessor::SwizzleOutput(std::move(fp), GrSwizzle::BGRA());
                }
            } else {
                fp = GrTextureEffect::Make(std::move(tempView), tempColorInfo.alphaType());
            }
            if (!fp) {
                return false;
            }
            this->asFillContext()->fillRectToRectWithFP(
                    SkIRect::MakeSize(srcInfo.dimensions()),
                    SkIRect::MakePtSize(pt, srcInfo.dimensions()),
                    std::move(fp));
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
            caps->supportedWritePixelsColorType(this->colorInfo().colorType(),
                                                dstProxy->backendFormat(),
                                                srcInfo.colorType()).fColorType;
    bool flip = this->origin() == kBottomLeft_GrSurfaceOrigin;
    bool makeTight = !caps->writePixelsRowBytesSupport() && rowBytes != tightRowBytes;
    bool convert = premul || unpremul || needColorConversion || makeTight ||
                   (srcInfo.colorType() != allowedColorType) || flip;

    std::unique_ptr<char[]> tmpPixels;
    GrColorType srcColorType = srcInfo.colorType();
    if (convert) {
        GrImageInfo tmpInfo(allowedColorType, this->colorInfo().alphaType(),
                            this->colorInfo().refColorSpace(), srcInfo.width(), srcInfo.height());
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
    dContext->priv().flushSurface(caps->preferVRAMUseOverFlushes() ? dstProxy : nullptr);

    return dContext->priv().getGpu()->writePixels(dstSurface, pt.fX, pt.fY, srcInfo.width(),
                                                  srcInfo.height(), this->colorInfo().colorType(),
                                                  srcColorType, src, rowBytes);
}

void GrSurfaceContext::asyncRescaleAndReadPixels(GrDirectContext* dContext,
                                                 const SkImageInfo& info,
                                                 const SkIRect& srcRect,
                                                 RescaleGamma rescaleGamma,
                                                 SkFilterQuality rescaleQuality,
                                                 ReadPixelsCallback callback,
                                                 ReadPixelsContext callbackContext) {
    // We implement this by rendering and we don't currently support rendering kUnpremul.
    if (info.alphaType() == kUnpremul_SkAlphaType) {
        callback(callbackContext, nullptr);
        return;
    }
    if (!dContext) {
        callback(callbackContext, nullptr);
        return;
    }
    auto rt = this->asRenderTargetProxy();
    if (rt && rt->wrapsVkSecondaryCB()) {
        callback(callbackContext, nullptr);
        return;
    }
    if (rt && rt->framebufferOnly()) {
        callback(callbackContext, nullptr);
        return;
    }
    auto dstCT = SkColorTypeToGrColorType(info.colorType());
    if (dstCT == GrColorType::kUnknown) {
        callback(callbackContext, nullptr);
        return;
    }
    bool needsRescale = srcRect.width() != info.width() || srcRect.height() != info.height();
    auto colorTypeOfFinalContext = this->colorInfo().colorType();
    auto backendFormatOfFinalContext = this->asSurfaceProxy()->backendFormat();
    if (needsRescale) {
        colorTypeOfFinalContext = dstCT;
        backendFormatOfFinalContext =
                this->caps()->getDefaultBackendFormat(dstCT, GrRenderable::kYes);
    }
    auto readInfo = this->caps()->supportedReadPixelsColorType(colorTypeOfFinalContext,
                                                               backendFormatOfFinalContext, dstCT);
    // Fail if we can't read from the source surface's color type.
    if (readInfo.fColorType == GrColorType::kUnknown) {
        callback(callbackContext, nullptr);
        return;
    }
    // Fail if read color type does not have all of dstCT's color channels and those missing color
    // channels are in the src.
    uint32_t dstChannels = GrColorTypeChannelFlags(dstCT);
    uint32_t legalReadChannels = GrColorTypeChannelFlags(readInfo.fColorType);
    uint32_t srcChannels = GrColorTypeChannelFlags(this->colorInfo().colorType());
    if ((~legalReadChannels & dstChannels) & srcChannels) {
        callback(callbackContext, nullptr);
        return;
    }

    std::unique_ptr<GrSurfaceDrawContext> tempRTC;
    int x = srcRect.fLeft;
    int y = srcRect.fTop;
    if (needsRescale) {
        tempRTC = this->rescale(info, kTopLeft_GrSurfaceOrigin, srcRect, rescaleGamma,
                                rescaleQuality);
        if (!tempRTC) {
            callback(callbackContext, nullptr);
            return;
        }
        SkASSERT(SkColorSpace::Equals(tempRTC->colorInfo().colorSpace(), info.colorSpace()));
        SkASSERT(tempRTC->origin() == kTopLeft_GrSurfaceOrigin);
        x = y = 0;
    } else {
        sk_sp<GrColorSpaceXform> xform = GrColorSpaceXform::Make(this->colorInfo().colorSpace(),
                                                                 this->colorInfo().alphaType(),
                                                                 info.colorSpace(),
                                                                 info.alphaType());
        // Insert a draw to a temporary surface if we need to do a y-flip or color space conversion.
        if (this->origin() == kBottomLeft_GrSurfaceOrigin || xform) {
            GrSurfaceProxyView texProxyView = this->readSurfaceView();
            SkRect srcRectToDraw = SkRect::Make(srcRect);
            // If the src is not texturable first try to make a copy to a texture.
            if (!texProxyView.asTextureProxy()) {
                texProxyView =
                        GrSurfaceProxyView::Copy(fContext, texProxyView, GrMipmapped::kNo, srcRect,
                                                 SkBackingFit::kApprox, SkBudgeted::kNo);
                if (!texProxyView) {
                    callback(callbackContext, nullptr);
                    return;
                }
                SkASSERT(texProxyView.asTextureProxy());
                srcRectToDraw = SkRect::MakeWH(srcRect.width(), srcRect.height());
            }
            tempRTC = GrSurfaceDrawContext::Make(dContext, this->colorInfo().colorType(),
                                                 info.refColorSpace(), SkBackingFit::kApprox,
                                                 srcRect.size(), 1, GrMipmapped::kNo,
                                                 GrProtected::kNo, kTopLeft_GrSurfaceOrigin);
            if (!tempRTC) {
                callback(callbackContext, nullptr);
                return;
            }
            tempRTC->drawTexture(nullptr,
                                 std::move(texProxyView),
                                 this->colorInfo().alphaType(),
                                 GrSamplerState::Filter::kNearest,
                                 GrSamplerState::MipmapMode::kNone,
                                 SkBlendMode::kSrc,
                                 SK_PMColor4fWHITE,
                                 srcRectToDraw,
                                 SkRect::MakeWH(srcRect.width(), srcRect.height()),
                                 GrAA::kNo,
                                 GrQuadAAFlags::kNone,
                                 SkCanvas::kFast_SrcRectConstraint,
                                 SkMatrix::I(),
                                 std::move(xform));
            x = y = 0;
        }
    }
    auto rtc = tempRTC ? tempRTC.get() : this;
    return rtc->asyncReadPixels(dContext, SkIRect::MakeXYWH(x, y, info.width(), info.height()),
                                info.colorType(), callback, callbackContext);
}

class GrSurfaceContext::AsyncReadResult : public SkImage::AsyncReadResult {
public:
    AsyncReadResult(uint32_t inboxID) : fInboxID(inboxID) {}
    ~AsyncReadResult() override {
        for (int i = 0; i < fPlanes.count(); ++i) {
            if (!fPlanes[i].fMappedBuffer) {
                delete[] static_cast<const char*>(fPlanes[i].fData);
            } else {
                GrClientMappedBufferManager::BufferFinishedMessageBus::Post(
                        {std::move(fPlanes[i].fMappedBuffer), fInboxID});
            }
        }
    }

    int count() const override { return fPlanes.count(); }
    const void* data(int i) const override { return fPlanes[i].fData; }
    size_t rowBytes(int i) const override { return fPlanes[i].fRowBytes; }

    bool addTransferResult(const PixelTransferResult& result,
                           SkISize dimensions,
                           size_t rowBytes,
                           GrClientMappedBufferManager* manager) {
        SkASSERT(!result.fTransferBuffer->isMapped());
        const void* mappedData = result.fTransferBuffer->map();
        if (!mappedData) {
            return false;
        }
        if (result.fPixelConverter) {
            std::unique_ptr<char[]> convertedData(new char[rowBytes * dimensions.height()]);
            result.fPixelConverter(convertedData.get(), mappedData);
            this->addCpuPlane(std::move(convertedData), rowBytes);
            result.fTransferBuffer->unmap();
        } else {
            manager->insert(result.fTransferBuffer);
            this->addMappedPlane(mappedData, rowBytes, std::move(result.fTransferBuffer));
        }
        return true;
    }

    void addCpuPlane(std::unique_ptr<const char[]> data, size_t rowBytes) {
        SkASSERT(data);
        SkASSERT(rowBytes > 0);
        fPlanes.emplace_back(data.release(), rowBytes, nullptr);
    }

private:
    void addMappedPlane(const void* data, size_t rowBytes, sk_sp<GrGpuBuffer> mappedBuffer) {
        SkASSERT(data);
        SkASSERT(rowBytes > 0);
        SkASSERT(mappedBuffer);
        SkASSERT(mappedBuffer->isMapped());
        fPlanes.emplace_back(data, rowBytes, std::move(mappedBuffer));
    }

    struct Plane {
        Plane(const void* data, size_t rowBytes, sk_sp<GrGpuBuffer> buffer)
                : fData(data), fRowBytes(rowBytes), fMappedBuffer(std::move(buffer)) {}
        const void* fData;
        size_t fRowBytes;
        // If this is null then fData is heap alloc and must be delete[]ed as const char[].
        sk_sp<GrGpuBuffer> fMappedBuffer;
    };
    SkSTArray<3, Plane> fPlanes;
    uint32_t fInboxID;
};

void GrSurfaceContext::asyncReadPixels(GrDirectContext* dContext,
                                       const SkIRect& rect,
                                       SkColorType colorType,
                                       ReadPixelsCallback callback,
                                       ReadPixelsContext callbackContext) {
    SkASSERT(rect.fLeft >= 0 && rect.fRight <= this->width());
    SkASSERT(rect.fTop >= 0 && rect.fBottom <= this->height());

    if (!dContext || this->asSurfaceProxy()->isProtected() == GrProtected::kYes) {
        callback(callbackContext, nullptr);
        return;
    }

    auto mappedBufferManager = dContext->priv().clientMappedBufferManager();

    auto transferResult = this->transferPixels(SkColorTypeToGrColorType(colorType), rect);

    if (!transferResult.fTransferBuffer) {
        auto ii = SkImageInfo::Make(rect.size(), colorType, this->colorInfo().alphaType(),
                                    this->colorInfo().refColorSpace());
        auto result = std::make_unique<AsyncReadResult>(0);
        std::unique_ptr<char[]> data(new char[ii.computeMinByteSize()]);
        SkPixmap pm(ii, data.get(), ii.minRowBytes());
        result->addCpuPlane(std::move(data), pm.rowBytes());

        SkIPoint pt{rect.fLeft, rect.fTop};
        if (!this->readPixels(dContext, ii, pm.writable_addr(), pm.rowBytes(), pt)) {
            callback(callbackContext, nullptr);
            return;
        }
        callback(callbackContext, std::move(result));
        return;
    }

    struct FinishContext {
        ReadPixelsCallback* fClientCallback;
        ReadPixelsContext fClientContext;
        SkISize fSize;
        SkColorType fColorType;
        GrClientMappedBufferManager* fMappedBufferManager;
        PixelTransferResult fTransferResult;
    };
    // Assumption is that the caller would like to flush. We could take a parameter or require an
    // explicit flush from the caller. We'd have to have a way to defer attaching the finish
    // callback to GrGpu until after the next flush that flushes our op list, though.
    auto* finishContext = new FinishContext{callback,
                                            callbackContext,
                                            rect.size(),
                                            colorType,
                                            mappedBufferManager,
                                            std::move(transferResult)};
    auto finishCallback = [](GrGpuFinishedContext c) {
        const auto* context = reinterpret_cast<const FinishContext*>(c);
        auto result = std::make_unique<AsyncReadResult>(context->fMappedBufferManager->inboxID());
        size_t rowBytes = context->fSize.width() * SkColorTypeBytesPerPixel(context->fColorType);
        if (!result->addTransferResult(context->fTransferResult, context->fSize, rowBytes,
                                       context->fMappedBufferManager)) {
            result.reset();
        }
        (*context->fClientCallback)(context->fClientContext, std::move(result));
        delete context;
    };
    GrFlushInfo flushInfo;
    flushInfo.fFinishedContext = finishContext;
    flushInfo.fFinishedProc = finishCallback;

    dContext->priv().flushSurface(this->asSurfaceProxy(),
                                  SkSurface::BackendSurfaceAccess::kNoAccess,
                                  flushInfo);
}

void GrSurfaceContext::asyncRescaleAndReadPixelsYUV420(GrDirectContext* dContext,
                                                       SkYUVColorSpace yuvColorSpace,
                                                       sk_sp<SkColorSpace> dstColorSpace,
                                                       const SkIRect& srcRect,
                                                       SkISize dstSize,
                                                       RescaleGamma rescaleGamma,
                                                       SkFilterQuality rescaleQuality,
                                                       ReadPixelsCallback callback,
                                                       ReadPixelsContext callbackContext) {
    SkASSERT(srcRect.fLeft >= 0 && srcRect.fRight <= this->width());
    SkASSERT(srcRect.fTop >= 0 && srcRect.fBottom <= this->height());
    SkASSERT(!dstSize.isZero());
    SkASSERT((dstSize.width() % 2 == 0) && (dstSize.height() % 2 == 0));

    if (!dContext) {
        callback(callbackContext, nullptr);
        return;
    }
    auto rt = this->asRenderTargetProxy();
    if (rt && rt->wrapsVkSecondaryCB()) {
        callback(callbackContext, nullptr);
        return;
    }
    if (rt && rt->framebufferOnly()) {
        callback(callbackContext, nullptr);
        return;
    }
    if (this->asSurfaceProxy()->isProtected() == GrProtected::kYes) {
        callback(callbackContext, nullptr);
        return;
    }
    int x = srcRect.fLeft;
    int y = srcRect.fTop;
    bool needsRescale = srcRect.size() != dstSize;
    GrSurfaceProxyView srcView;
    if (needsRescale) {
        // We assume the caller wants kPremul. There is no way to indicate a preference.
        auto info = SkImageInfo::Make(dstSize, kRGBA_8888_SkColorType, kPremul_SkAlphaType,
                                      dstColorSpace);
        // TODO: Incorporate the YUV conversion into last pass of rescaling.
        auto tempRTC = this->rescale(info, kTopLeft_GrSurfaceOrigin, srcRect, rescaleGamma,
                                     rescaleQuality);
        if (!tempRTC) {
            callback(callbackContext, nullptr);
            return;
        }
        SkASSERT(SkColorSpace::Equals(tempRTC->colorInfo().colorSpace(), info.colorSpace()));
        SkASSERT(tempRTC->origin() == kTopLeft_GrSurfaceOrigin);
        x = y = 0;
        srcView = tempRTC->readSurfaceView();
    } else {
        srcView = this->readSurfaceView();
        if (!srcView.asTextureProxy()) {
            srcView = GrSurfaceProxyView::Copy(fContext, std::move(srcView), GrMipmapped::kNo,
                                               srcRect, SkBackingFit::kApprox, SkBudgeted::kYes);
            if (!srcView) {
                // If we can't get a texture copy of the contents then give up.
                callback(callbackContext, nullptr);
                return;
            }
            SkASSERT(srcView.asTextureProxy());
            x = y = 0;
        }
        // We assume the caller wants kPremul. There is no way to indicate a preference.
        sk_sp<GrColorSpaceXform> xform = GrColorSpaceXform::Make(
                this->colorInfo().colorSpace(), this->colorInfo().alphaType(), dstColorSpace.get(),
                kPremul_SkAlphaType);
        if (xform) {
            SkRect srcRectToDraw = SkRect::MakeXYWH(x, y, srcRect.width(), srcRect.height());
            auto tempRTC = GrSurfaceDrawContext::Make(
                    dContext, this->colorInfo().colorType(), dstColorSpace, SkBackingFit::kApprox,
                    dstSize, 1, GrMipmapped::kNo, GrProtected::kNo, kTopLeft_GrSurfaceOrigin);
            if (!tempRTC) {
                callback(callbackContext, nullptr);
                return;
            }
            tempRTC->drawTexture(nullptr,
                                 std::move(srcView),
                                 this->colorInfo().alphaType(),
                                 GrSamplerState::Filter::kNearest,
                                 GrSamplerState::MipmapMode::kNone,
                                 SkBlendMode::kSrc,
                                 SK_PMColor4fWHITE,
                                 srcRectToDraw,
                                 SkRect::Make(srcRect.size()),
                                 GrAA::kNo,
                                 GrQuadAAFlags::kNone,
                                 SkCanvas::kFast_SrcRectConstraint,
                                 SkMatrix::I(),
                                 std::move(xform));
            srcView = tempRTC->readSurfaceView();
            SkASSERT(srcView.asTextureProxy());
            x = y = 0;
        }
    }

    auto yRTC = GrSurfaceDrawContext::MakeWithFallback(
            dContext, GrColorType::kAlpha_8, dstColorSpace, SkBackingFit::kApprox, dstSize, 1,
            GrMipmapped::kNo, GrProtected::kNo, kTopLeft_GrSurfaceOrigin);
    int halfW = dstSize.width() /2;
    int halfH = dstSize.height()/2;
    auto uRTC = GrSurfaceDrawContext::MakeWithFallback(
            dContext, GrColorType::kAlpha_8, dstColorSpace, SkBackingFit::kApprox, {halfW, halfH},
            1, GrMipmapped::kNo, GrProtected::kNo, kTopLeft_GrSurfaceOrigin);
    auto vRTC = GrSurfaceDrawContext::MakeWithFallback(
            dContext, GrColorType::kAlpha_8, dstColorSpace, SkBackingFit::kApprox, {halfW, halfH},
            1, GrMipmapped::kNo, GrProtected::kNo, kTopLeft_GrSurfaceOrigin);
    if (!yRTC || !uRTC || !vRTC) {
        callback(callbackContext, nullptr);
        return;
    }

    float baseM[20];
    SkColorMatrix_RGB2YUV(yuvColorSpace, baseM);

    // TODO: Use one transfer buffer for all three planes to reduce map/unmap cost?

    auto texMatrix = SkMatrix::Translate(x, y);

    SkRect dstRectY = SkRect::Make(dstSize);
    SkRect dstRectUV = SkRect::MakeWH(halfW, halfH);

    bool doSynchronousRead = !this->caps()->transferFromSurfaceToBufferSupport();
    PixelTransferResult yTransfer, uTransfer, vTransfer;

    // This matrix generates (r,g,b,a) = (0, 0, 0, y)
    float yM[20];
    std::fill_n(yM, 15, 0.f);
    std::copy_n(baseM + 0, 5, yM + 15);
    GrPaint yPaint;
    auto yTexFP = GrTextureEffect::Make(srcView, this->colorInfo().alphaType(), texMatrix);
    auto yColFP = GrColorMatrixFragmentProcessor::Make(std::move(yTexFP), yM,
                                                       /*unpremulInput=*/false,
                                                       /*clampRGBOutput=*/true,
                                                       /*premulOutput=*/false);
    yPaint.setColorFragmentProcessor(std::move(yColFP));
    yPaint.setPorterDuffXPFactory(SkBlendMode::kSrc);
    yRTC->fillRectToRect(nullptr, std::move(yPaint), GrAA::kNo, SkMatrix::I(), dstRectY, dstRectY);
    if (!doSynchronousRead) {
        yTransfer = yRTC->transferPixels(GrColorType::kAlpha_8,
                                         SkIRect::MakeWH(yRTC->width(), yRTC->height()));
        if (!yTransfer.fTransferBuffer) {
            callback(callbackContext, nullptr);
            return;
        }
    }

    texMatrix.preScale(2.f, 2.f);
    // This matrix generates (r,g,b,a) = (0, 0, 0, u)
    float uM[20];
    std::fill_n(uM, 15, 0.f);
    std::copy_n(baseM + 5, 5, uM + 15);
    GrPaint uPaint;
    auto uTexFP = GrTextureEffect::Make(srcView, this->colorInfo().alphaType(), texMatrix,
                                        GrSamplerState::Filter::kLinear);
    auto uColFP = GrColorMatrixFragmentProcessor::Make(std::move(uTexFP), uM,
                                                       /*unpremulInput=*/false,
                                                       /*clampRGBOutput=*/true,
                                                       /*premulOutput=*/false);
    uPaint.setColorFragmentProcessor(std::move(uColFP));
    uPaint.setPorterDuffXPFactory(SkBlendMode::kSrc);
    uRTC->fillRectToRect(nullptr, std::move(uPaint), GrAA::kNo, SkMatrix::I(), dstRectUV,
                         dstRectUV);
    if (!doSynchronousRead) {
        uTransfer = uRTC->transferPixels(GrColorType::kAlpha_8,
                                         SkIRect::MakeWH(uRTC->width(), uRTC->height()));
        if (!uTransfer.fTransferBuffer) {
            callback(callbackContext, nullptr);
            return;
        }
    }

    // This matrix generates (r,g,b,a) = (0, 0, 0, v)
    float vM[20];
    std::fill_n(vM, 15, 0.f);
    std::copy_n(baseM + 10, 5, vM + 15);
    GrPaint vPaint;
    auto vTexFP = GrTextureEffect::Make(std::move(srcView), this->colorInfo().alphaType(),
                                        texMatrix, GrSamplerState::Filter::kLinear);
    auto vColFP = GrColorMatrixFragmentProcessor::Make(std::move(vTexFP), vM,
                                                       /*unpremulInput=*/false,
                                                       /*clampRGBOutput=*/true,
                                                       /*premulOutput=*/false);
    vPaint.setColorFragmentProcessor(std::move(vColFP));
    vPaint.setPorterDuffXPFactory(SkBlendMode::kSrc);
    vRTC->fillRectToRect(nullptr, std::move(vPaint), GrAA::kNo, SkMatrix::I(), dstRectUV,
                         dstRectUV);
    if (!doSynchronousRead) {
        vTransfer = vRTC->transferPixels(GrColorType::kAlpha_8,
                                         SkIRect::MakeWH(vRTC->width(), vRTC->height()));
        if (!vTransfer.fTransferBuffer) {
            callback(callbackContext, nullptr);
            return;
        }
    }

    if (doSynchronousRead) {
        GrImageInfo yInfo(GrColorType::kAlpha_8, kPremul_SkAlphaType, nullptr, dstSize);
        GrImageInfo uvInfo = yInfo.makeWH(halfW, halfH);
        size_t yRB  = yInfo.minRowBytes();
        size_t uvRB = uvInfo.minRowBytes();
        std::unique_ptr<char[]> y(new char[yRB * yInfo.height()]);
        std::unique_ptr<char[]> u(new char[uvRB*uvInfo.height()]);
        std::unique_ptr<char[]> v(new char[uvRB*uvInfo.height()]);
        if (!yRTC->readPixels(dContext, yInfo,  y.get(), yRB,  {0, 0}) ||
            !uRTC->readPixels(dContext, uvInfo, u.get(), uvRB, {0, 0}) ||
            !vRTC->readPixels(dContext, uvInfo, v.get(), uvRB, {0, 0})) {
            callback(callbackContext, nullptr);
            return;
        }
        auto result = std::make_unique<AsyncReadResult>(dContext->priv().contextID());
        result->addCpuPlane(std::move(y), yRB );
        result->addCpuPlane(std::move(u), uvRB);
        result->addCpuPlane(std::move(v), uvRB);
        callback(callbackContext, std::move(result));
        return;
    }

    struct FinishContext {
        ReadPixelsCallback* fClientCallback;
        ReadPixelsContext fClientContext;
        GrClientMappedBufferManager* fMappedBufferManager;
        SkISize fSize;
        PixelTransferResult fYTransfer;
        PixelTransferResult fUTransfer;
        PixelTransferResult fVTransfer;
    };
    // Assumption is that the caller would like to flush. We could take a parameter or require an
    // explicit flush from the caller. We'd have to have a way to defer attaching the finish
    // callback to GrGpu until after the next flush that flushes our op list, though.
    auto* finishContext = new FinishContext{callback,
                                            callbackContext,
                                            dContext->priv().clientMappedBufferManager(),
                                            dstSize,
                                            std::move(yTransfer),
                                            std::move(uTransfer),
                                            std::move(vTransfer)};
    auto finishCallback = [](GrGpuFinishedContext c) {
        const auto* context = reinterpret_cast<const FinishContext*>(c);
        auto result = std::make_unique<AsyncReadResult>(context->fMappedBufferManager->inboxID());
        auto manager = context->fMappedBufferManager;
        size_t rowBytes = SkToSizeT(context->fSize.width());
        if (!result->addTransferResult(context->fYTransfer, context->fSize, rowBytes, manager)) {
            (*context->fClientCallback)(context->fClientContext, nullptr);
            delete context;
            return;
        }
        rowBytes /= 2;
        SkISize uvSize = {context->fSize.width() / 2, context->fSize.height() / 2};
        if (!result->addTransferResult(context->fUTransfer, uvSize, rowBytes, manager)) {
            (*context->fClientCallback)(context->fClientContext, nullptr);
            delete context;
            return;
        }
        if (!result->addTransferResult(context->fVTransfer, uvSize, rowBytes, manager)) {
            (*context->fClientCallback)(context->fClientContext, nullptr);
            delete context;
            return;
        }
        (*context->fClientCallback)(context->fClientContext, std::move(result));
        delete context;
    };
    GrFlushInfo flushInfo;
    flushInfo.fFinishedContext = finishContext;
    flushInfo.fFinishedProc = finishCallback;
    dContext->priv().flushSurface(this->asSurfaceProxy(),
                                  SkSurface::BackendSurfaceAccess::kNoAccess,
                                  flushInfo);
}

bool GrSurfaceContext::copy(GrSurfaceProxy* src, const SkIRect& srcRect, const SkIPoint& dstPoint) {
    ASSERT_SINGLE_OWNER
    RETURN_FALSE_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(this->auditTrail(), "GrSurfaceContextPriv::copy");

    const GrCaps* caps = fContext->priv().caps();

    SkASSERT(src->backendFormat().textureType() != GrTextureType::kExternal);
    SkASSERT(src->backendFormat() == this->asSurfaceProxy()->backendFormat());

    if (this->asSurfaceProxy()->framebufferOnly()) {
        return false;
    }

    if (!caps->canCopySurface(this->asSurfaceProxy(), src, srcRect, dstPoint)) {
        return false;
    }

    // The swizzle doesn't matter for copies and it is not used.
    return this->drawingManager()->newCopyRenderTask(
            GrSurfaceProxyView(sk_ref_sp(src), this->origin(), GrSwizzle("rgba")), srcRect,
            this->readSurfaceView(), dstPoint);
}

std::unique_ptr<GrSurfaceDrawContext> GrSurfaceContext::rescale(const GrImageInfo& info,
                                                                GrSurfaceOrigin origin,
                                                                SkIRect srcRect,
                                                                RescaleGamma rescaleGamma,
                                                                SkFilterQuality rescaleQuality) {
    // We rescale by drawing and currently only support drawing to premul.
    if (info.alphaType() != kPremul_SkAlphaType) {
        return nullptr;
    }
    auto sdc = GrSurfaceDrawContext::MakeWithFallback(fContext,
                                                      info.colorType(),
                                                      info.refColorSpace(),
                                                      SkBackingFit::kExact,
                                                      info.dimensions(),
                                                      1,
                                                      GrMipmapped::kNo,
                                                      this->asSurfaceProxy()->isProtected(),
                                                      origin);
    if (!sdc || !this->rescaleInto(sdc.get(),
                                   SkIRect::MakeSize(sdc->dimensions()),
                                   srcRect,
                                   rescaleGamma,
                                   rescaleQuality)) {
        return nullptr;
    }
    return sdc;
}

bool GrSurfaceContext::rescaleInto(GrSurfaceDrawContext* dst,
                                   SkIRect dstRect,
                                   SkIRect srcRect,
                                   RescaleGamma rescaleGamma,
                                   SkFilterQuality rescaleQuality) {
    SkASSERT(dst);
    if (!SkIRect::MakeSize(dst->dimensions()).contains((dstRect))) {
        return false;
    }

    auto rtProxy = this->asRenderTargetProxy();
    if (rtProxy && rtProxy->wrapsVkSecondaryCB()) {
        return false;
    }

    if (this->asSurfaceProxy()->framebufferOnly()) {
        return false;
    }

    GrSurfaceProxyView texView = this->readSurfaceView();
    SkAlphaType srcAlphaType = this->colorInfo().alphaType();
    if (!texView.asTextureProxy()) {
        texView = GrSurfaceProxyView::Copy(fContext, std::move(texView), GrMipmapped::kNo, srcRect,
                                           SkBackingFit::kApprox, SkBudgeted::kNo);
        if (!texView) {
            return false;
        }
        SkASSERT(texView.asTextureProxy());
        srcRect = SkIRect::MakeSize(srcRect.size());
    }

    SkISize finalSize = dstRect.size();

    // Within a rescaling pass A is the input (if not null) and B is the output. At the end of the
    // pass B is moved to A. If 'this' is the input on the first pass then tempA is null.
    std::unique_ptr<GrSurfaceDrawContext> tempA;
    std::unique_ptr<GrSurfaceDrawContext> tempB;

    // Assume we should ignore the rescale linear request if the surface has no color space since
    // it's unclear how we'd linearize from an unknown color space.
    if (rescaleGamma == RescaleGamma::kLinear && this->colorInfo().colorSpace() &&
        !this->colorInfo().colorSpace()->gammaIsLinear()) {
        auto cs = this->colorInfo().colorSpace()->makeLinearGamma();
        auto xform = GrColorSpaceXform::Make(this->colorInfo().colorSpace(), srcAlphaType, cs.get(),
                                             kPremul_SkAlphaType);
        // We'll fall back to kRGBA_8888 if half float not supported.
        auto linearRTC = GrSurfaceDrawContext::MakeWithFallback(
                fContext, GrColorType::kRGBA_F16, cs, SkBackingFit::kApprox, srcRect.size(), 1,
                GrMipmapped::kNo, GrProtected::kNo, dst->origin());
        if (!linearRTC) {
            return false;
        }
        // 1-to-1 draw can always be kFast.
        linearRTC->drawTexture(nullptr,
                               std::move(texView),
                               srcAlphaType,
                               GrSamplerState::Filter::kNearest,
                               GrSamplerState::MipmapMode::kNone,
                               SkBlendMode::kSrc,
                               SK_PMColor4fWHITE,
                               SkRect::Make(srcRect),
                               SkRect::Make(srcRect.size()),
                               GrAA::kNo,
                               GrQuadAAFlags::kNone,
                               SkCanvas::kFast_SrcRectConstraint,
                               SkMatrix::I(),
                               std::move(xform));
        texView = linearRTC->readSurfaceView();
        SkASSERT(texView.asTextureProxy());
        tempA = std::move(linearRTC);
        srcRect = SkIRect::MakeSize(srcRect.size());
    }

    while (srcRect.size() != finalSize) {
        SkISize nextDims = finalSize;
        if (rescaleQuality != kNone_SkFilterQuality) {
            if (srcRect.width() > finalSize.width()) {
                nextDims.fWidth = std::max((srcRect.width() + 1)/2, finalSize.width());
            } else if (srcRect.width() < finalSize.width()) {
                nextDims.fWidth = std::min(srcRect.width()*2, finalSize.width());
            }
            if (srcRect.height() > finalSize.height()) {
                nextDims.fHeight = std::max((srcRect.height() + 1)/2, finalSize.height());
            } else if (srcRect.height() < finalSize.height()) {
                nextDims.fHeight = std::min(srcRect.height()*2, finalSize.height());
            }
        }
        auto input = tempA ? tempA.get() : this;
        sk_sp<GrColorSpaceXform> xform;
        GrSurfaceDrawContext* stepDst;
        SkIRect stepDstRect;
        if (nextDims == finalSize) {
            // Might as well fold conversion to final info in the last step.
            xform = GrColorSpaceXform::Make(input->colorInfo().colorSpace(),
                                            input->colorInfo().alphaType(),
                                            dst->colorInfo().colorSpace(),
                                            dst->colorInfo().alphaType());
            stepDst = dst;
            stepDstRect = dstRect;
        } else {
            tempB = GrSurfaceDrawContext::MakeWithFallback(fContext,
                                                           input->colorInfo().colorType(),
                                                           input->colorInfo().refColorSpace(),
                                                           SkBackingFit::kApprox,
                                                           nextDims,
                                                           1,
                                                           GrMipmapped::kNo,
                                                           GrProtected::kNo,
                                                           dst->origin());
            if (!tempB) {
                return false;
            }
            stepDst = tempB.get();
            stepDstRect = SkIRect::MakeSize(tempB->dimensions());
        }
        if (rescaleQuality == kHigh_SkFilterQuality) {
            SkMatrix matrix;
            matrix.setScaleTranslate((float)srcRect.width()/nextDims.width(),
                                     (float)srcRect.height()/nextDims.height(),
                                     srcRect.x(),
                                     srcRect.y());
            std::unique_ptr<GrFragmentProcessor> fp;
            auto dir = GrBicubicEffect::Direction::kXY;
            if (nextDims.width() == srcRect.width()) {
                dir = GrBicubicEffect::Direction::kY;
            } else if (nextDims.height() == srcRect.height()) {
                dir = GrBicubicEffect::Direction::kX;
            }
            static constexpr auto kWM     = GrSamplerState::WrapMode::kClamp;
            static constexpr auto kKernel = GrBicubicEffect::gCatmullRom;
            fp = GrBicubicEffect::MakeSubset(std::move(texView),
                                             input->colorInfo().alphaType(),
                                             matrix,
                                             kWM,
                                             kWM,
                                             SkRect::Make(srcRect),
                                             kKernel,
                                             dir,
                                             *this->caps());
            if (xform) {
                fp = GrColorSpaceXformEffect::Make(std::move(fp), std::move(xform));
            }
            GrPaint paint;
            paint.setColorFragmentProcessor(std::move(fp));
            paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
            stepDst->fillRectToRect(nullptr,
                                    std::move(paint),
                                    GrAA::kNo,
                                    SkMatrix::I(),
                                    SkRect::Make(stepDstRect),
                                    SkRect::Make(stepDstRect));
        } else {
            auto filter = rescaleQuality == kNone_SkFilterQuality ? GrSamplerState::Filter::kNearest
                                                                  : GrSamplerState::Filter::kLinear;
            // Minimizing draw with integer coord src and dev rects can always be kFast.
            auto constraint = SkCanvas::SrcRectConstraint::kStrict_SrcRectConstraint;
            if (nextDims.width() <= srcRect.width() && nextDims.height() <= srcRect.height()) {
                constraint = SkCanvas::SrcRectConstraint::kFast_SrcRectConstraint;
            }
            stepDst->drawTexture(nullptr,
                                 std::move(texView),
                                 srcAlphaType,
                                 filter,
                                 GrSamplerState::MipmapMode::kNone,
                                 SkBlendMode::kSrc,
                                 SK_PMColor4fWHITE,
                                 SkRect::Make(srcRect),
                                 SkRect::Make(stepDstRect),
                                 GrAA::kNo,
                                 GrQuadAAFlags::kNone,
                                 constraint,
                                 SkMatrix::I(),
                                 std::move(xform));
        }
        texView = stepDst->readSurfaceView();
        tempA = std::move(tempB);
        srcRect = SkIRect::MakeSize(nextDims);
    }
    return true;
}

GrSurfaceContext::PixelTransferResult GrSurfaceContext::transferPixels(GrColorType dstCT,
                                                                       const SkIRect& rect) {
    SkASSERT(rect.fLeft >= 0 && rect.fRight <= this->width());
    SkASSERT(rect.fTop >= 0 && rect.fBottom <= this->height());
    auto direct = fContext->asDirectContext();
    if (!direct) {
        return {};
    }
    auto rtProxy = this->asRenderTargetProxy();
    if (rtProxy && rtProxy->wrapsVkSecondaryCB()) {
        return {};
    }

    auto proxy = this->asSurfaceProxy();
    auto supportedRead = this->caps()->supportedReadPixelsColorType(this->colorInfo().colorType(),
                                                                    proxy->backendFormat(), dstCT);
    // Fail if read color type does not have all of dstCT's color channels and those missing color
    // channels are in the src.
    uint32_t dstChannels = GrColorTypeChannelFlags(dstCT);
    uint32_t legalReadChannels = GrColorTypeChannelFlags(supportedRead.fColorType);
    uint32_t srcChannels = GrColorTypeChannelFlags(this->colorInfo().colorType());
    if ((~legalReadChannels & dstChannels) & srcChannels) {
        return {};
    }

    if (!this->caps()->transferFromSurfaceToBufferSupport() ||
        !supportedRead.fOffsetAlignmentForTransferBuffer) {
        return {};
    }

    size_t rowBytes = GrColorTypeBytesPerPixel(supportedRead.fColorType) * rect.width();
    size_t size = rowBytes * rect.height();
    auto buffer = direct->priv().resourceProvider()->createBuffer(
            size, GrGpuBufferType::kXferGpuToCpu, GrAccessPattern::kStream_GrAccessPattern);
    if (!buffer) {
        return {};
    }
    auto srcRect = rect;
    bool flip = this->origin() == kBottomLeft_GrSurfaceOrigin;
    if (flip) {
        srcRect = SkIRect::MakeLTRB(rect.fLeft, this->height() - rect.fBottom, rect.fRight,
                                    this->height() - rect.fTop);
    }
    this->drawingManager()->newTransferFromRenderTask(this->asSurfaceProxyRef(), srcRect,
                                                      this->colorInfo().colorType(),
                                                      supportedRead.fColorType, buffer, 0);
    PixelTransferResult result;
    result.fTransferBuffer = std::move(buffer);
    auto at = this->colorInfo().alphaType();
    if (supportedRead.fColorType != dstCT || flip) {
        result.fPixelConverter = [w = rect.width(), h = rect.height(), dstCT, supportedRead, at](
                void* dst, const void* src) {
            GrImageInfo srcInfo(supportedRead.fColorType, at, nullptr, w, h);
            GrImageInfo dstInfo(dstCT,                    at, nullptr, w, h);
              GrConvertPixels(dstInfo, dst, dstInfo.minRowBytes(),
                              srcInfo, src, srcInfo.minRowBytes(),
                              /* flipY = */ false);
        };
    }
    return result;
}

#ifdef SK_DEBUG
void GrSurfaceContext::validate() const {
    SkASSERT(fReadView.proxy());
    fReadView.proxy()->validate(fContext);
    if (this->colorInfo().colorType() != GrColorType::kUnknown) {
        SkASSERT(fContext->priv().caps()->areColorTypeAndFormatCompatible(
                this->colorInfo().colorType(), fReadView.proxy()->backendFormat()));
    }
    this->onValidate();
}
#endif
