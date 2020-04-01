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
#include "src/gpu/GrImageInfo.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrSurfaceContextPriv.h"
#include "src/gpu/GrSurfacePriv.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrBicubicEffect.h"

#define ASSERT_SINGLE_OWNER \
    SkDEBUGCODE(GrSingleOwner::AutoEnforce debug_SingleOwner(this->singleOwner());)
#define RETURN_FALSE_IF_ABANDONED  if (this->fContext->priv().abandoned()) { return false; }

std::unique_ptr<GrSurfaceContext> GrSurfaceContext::Make(GrRecordingContext* context,
                                                         GrSurfaceProxyView readView,
                                                         GrColorType colorType,
                                                         SkAlphaType alphaType,
                                                         sk_sp<SkColorSpace> colorSpace) {
    // It is probably not necessary to check if the context is abandoned here since uses of the
    // GrSurfaceContext which need the context will mostly likely fail later on without an issue.
    // However having this hear adds some reassurance in case there is a path doesn't handle an
    // abandoned context correctly. It also lets us early out of some extra work.
    if (context->priv().abandoned()) {
        return nullptr;
    }
    GrSurfaceProxy* proxy = readView.proxy();
    SkASSERT(proxy && proxy->asTextureProxy());

    std::unique_ptr<GrSurfaceContext> surfaceContext;
    if (proxy->asRenderTargetProxy()) {
        SkASSERT(kPremul_SkAlphaType == alphaType || kOpaque_SkAlphaType == alphaType);
        // Will we ever want a swizzle that is not the default output swizzle for the format and
        // colorType here? If so we will need to manually pass that in.
        GrSwizzle outSwizzle =
                context->priv().caps()->getWriteSwizzle(proxy->backendFormat(), colorType);
        GrSurfaceProxyView outputView(readView.refProxy(), readView.origin(), outSwizzle);
        surfaceContext.reset(new GrRenderTargetContext(context, std::move(readView),
                                                       std::move(outputView), colorType,
                                                       std::move(colorSpace), nullptr));
    } else {
        surfaceContext.reset(new GrSurfaceContext(context, std::move(readView), colorType,
                                                  alphaType, std::move(colorSpace)));
    }
    SkDEBUGCODE(surfaceContext->validate();)
    return surfaceContext;
}

std::unique_ptr<GrSurfaceContext> GrSurfaceContext::Make(GrRecordingContext* context,
                                                         SkISize dimensions,
                                                         const GrBackendFormat& format,
                                                         GrRenderable renderable,
                                                         int renderTargetSampleCnt,
                                                         GrMipMapped mipMapped,
                                                         GrProtected isProtected,
                                                         GrSurfaceOrigin origin,
                                                         GrColorType colorType,
                                                         SkAlphaType alphaType,
                                                         sk_sp<SkColorSpace> colorSpace,
                                                         SkBackingFit fit,
                                                         SkBudgeted budgeted) {
    GrSwizzle swizzle = context->priv().caps()->getReadSwizzle(format, colorType);

    sk_sp<GrTextureProxy> proxy = context->priv().proxyProvider()->createProxy(
            format, dimensions, renderable, renderTargetSampleCnt, mipMapped, fit, budgeted,
            isProtected);
    if (!proxy) {
        return nullptr;
    }

    GrSurfaceProxyView view(std::move(proxy), origin, swizzle);
    return GrSurfaceContext::Make(context, std::move(view), colorType, alphaType,
                                  std::move(colorSpace));
}

// In MDB mode the reffing of the 'getLastOpsTask' call's result allows in-progress
// GrOpsTasks to be picked up and added to by renderTargetContexts lower in the call
// stack. When this occurs with a closed GrOpsTask, a new one will be allocated
// when the renderTargetContext attempts to use it (via getOpsTask).
GrSurfaceContext::GrSurfaceContext(GrRecordingContext* context,
                                   GrSurfaceProxyView readView,
                                   GrColorType colorType,
                                   SkAlphaType alphaType,
                                   sk_sp<SkColorSpace> colorSpace)
        : fContext(context)
        , fReadView(std::move(readView))
        , fColorInfo(colorType, alphaType, std::move(colorSpace)) {
    SkASSERT(!context->priv().abandoned());
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
GrSingleOwner* GrSurfaceContext::singleOwner() {
    return fContext->priv().singleOwner();
}
#endif

bool GrSurfaceContext::readPixels(const GrImageInfo& origDstInfo, void* dst, size_t rowBytes,
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

    if (srcProxy->framebufferOnly()) {
        return false;
    }

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

    SkColorSpaceXformSteps::Flags flags = SkColorSpaceXformSteps{this->colorInfo(), dstInfo}.flags;
    bool unpremul            = flags.unpremul,
         needColorConversion = flags.linearize || flags.gamut_transform || flags.encode,
         premul              = flags.premul;

    const GrCaps* caps = direct->priv().caps();
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
                            direct->priv().validPMUPMConversionExists();

    auto readFlag = caps->surfaceSupportsReadPixels(srcSurface);
    if (readFlag == GrCaps::SurfaceReadPixelsSupport::kUnsupported) {
        return false;
    }

    if (readFlag == GrCaps::SurfaceReadPixelsSupport::kCopyToTexture2D || canvas2DFastPath) {
        GrColorType colorType = (canvas2DFastPath || srcIsCompressed)
                                    ? GrColorType::kRGBA_8888 : this->colorInfo().colorType();
        sk_sp<SkColorSpace> cs = canvas2DFastPath ? nullptr : this->colorInfo().refColorSpace();

        auto tempCtx = GrRenderTargetContext::Make(
                direct, colorType, std::move(cs), SkBackingFit::kApprox, dstInfo.dimensions(),
                1, GrMipMapped::kNo, GrProtected::kNo, kTopLeft_GrSurfaceOrigin);
        if (!tempCtx) {
            return false;
        }

        std::unique_ptr<GrFragmentProcessor> fp;
        if (canvas2DFastPath) {
            fp = direct->priv().createPMToUPMEffect(
                    GrTextureEffect::Make(this->readSurfaceView(), this->colorInfo().alphaType()));
            if (dstInfo.colorType() == GrColorType::kBGRA_8888) {
                fp = GrFragmentProcessor::SwizzleOutput(std::move(fp), GrSwizzle::BGRA());
                dstInfo = dstInfo.makeColorType(GrColorType::kRGBA_8888);
            }
            // The render target context is incorrectly tagged as kPremul even though we're writing
            // unpremul data thanks to the PMToUPM effect. Fake out the dst alpha type so we don't
            // double unpremul.
            dstInfo = dstInfo.makeAlphaType(kPremul_SkAlphaType);
        } else {
            fp = GrTextureEffect::Make(this->readSurfaceView(), this->colorInfo().alphaType());
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
        tmpPixels.reset(new char[size]());

        readDst = tmpPixels.get();
        readRB = tmpRB;
        pt.fY = flip ? srcSurface->height() - pt.fY - dstInfo.height() : pt.fY;
    }

    direct->priv().flushSurface(srcProxy);

    if (!direct->priv().getGpu()->readPixels(srcSurface, pt.fX, pt.fY, dstInfo.width(),
                                             dstInfo.height(), this->colorInfo().colorType(),
                                             supportedRead.fColorType, readDst, readRB)) {
        return false;
    }

    if (convert) {
        return GrConvertPixels(dstInfo, dst, rowBytes, tmpInfo, readDst, readRB, flip);
    }
    return true;
}

bool GrSurfaceContext::writePixels(const GrImageInfo& origSrcInfo, const void* src, size_t rowBytes,
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

    if (dstProxy->framebufferOnly()) {
        return false;
    }

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

    SkColorSpaceXformSteps::Flags flags = SkColorSpaceXformSteps{srcInfo, this->colorInfo()}.flags;
    bool unpremul            = flags.unpremul,
         needColorConversion = flags.linearize || flags.gamut_transform || flags.encode,
         premul              = flags.premul;

    const GrCaps* caps = direct->priv().caps();

    auto rgbaDefaultFormat = caps->getDefaultBackendFormat(GrColorType::kRGBA_8888,
                                                           GrRenderable::kNo);

    GrColorType dstColorType = this->colorInfo().colorType();
    // For canvas2D putImageData performance we have a special code path for unpremul RGBA_8888 srcs
    // that are premultiplied on the GPU. This is kept as narrow as possible for now.
    bool canvas2DFastPath = !caps->avoidWritePixelsFastPath() && premul && !needColorConversion &&
                            (srcInfo.colorType() == GrColorType::kRGBA_8888 ||
                             srcInfo.colorType() == GrColorType::kBGRA_8888) &&
                            SkToBool(this->asRenderTargetContext()) &&
                            (dstColorType == GrColorType::kRGBA_8888 ||
                             dstColorType == GrColorType::kBGRA_8888) &&
                            rgbaDefaultFormat.isValid() &&
                            direct->priv().validPMUPMConversionExists();

    if (!caps->surfaceSupportsWritePixels(dstSurface) || canvas2DFastPath) {
        GrColorType colorType;

        GrBackendFormat format;
        SkAlphaType alphaType;
        GrSwizzle tempReadSwizzle;
        if (canvas2DFastPath) {
            colorType = GrColorType::kRGBA_8888;
            format = rgbaDefaultFormat;
            alphaType = kUnpremul_SkAlphaType;
        } else {
            colorType = this->colorInfo().colorType();
            format = dstProxy->backendFormat().makeTexture2D();
            if (!format.isValid()) {
                return false;
            }
            alphaType = this->colorInfo().alphaType();
            tempReadSwizzle = this->readSwizzle();
        }

        // It is more efficient for us to write pixels into a top left origin so we prefer that.
        // However, if the final proxy isn't a render target then we must use a copy to move the
        // data into it which requires the origins to match. If the final proxy is a render target
        // we can use a draw instead which doesn't have this origin restriction. Thus for render
        // targets we will use top left and otherwise we will make the origins match.
        GrSurfaceOrigin tempOrigin =
                this->asRenderTargetContext() ? kTopLeft_GrSurfaceOrigin : this->origin();
        auto tempProxy = direct->priv().proxyProvider()->createProxy(
                format, srcInfo.dimensions(), GrRenderable::kNo, 1, GrMipMapped::kNo,
                SkBackingFit::kApprox, SkBudgeted::kYes, GrProtected::kNo);
        if (!tempProxy) {
            return false;
        }
        GrSurfaceProxyView tempView(tempProxy, tempOrigin, tempReadSwizzle);
        GrSurfaceContext tempCtx(direct, tempView, colorType, alphaType,
                                 this->colorInfo().refColorSpace());

        // In the fast path we always write the srcData to the temp context as though it were RGBA.
        // When the data is really BGRA the write will cause the R and B channels to be swapped in
        // the intermediate surface which gets corrected by a swizzle effect when drawing to the
        // dst.
        if (canvas2DFastPath) {
            srcInfo = srcInfo.makeColorType(GrColorType::kRGBA_8888);
        }
        if (!tempCtx.writePixels(srcInfo, src, rowBytes, {0, 0}, direct)) {
            return false;
        }

        if (this->asRenderTargetContext()) {
            std::unique_ptr<GrFragmentProcessor> fp;
            if (canvas2DFastPath) {
                fp = direct->priv().createUPMToPMEffect(
                        GrTextureEffect::Make(std::move(tempView), alphaType));
                // Important: check the original src color type here!
                if (origSrcInfo.colorType() == GrColorType::kBGRA_8888) {
                    fp = GrFragmentProcessor::SwizzleOutput(std::move(fp), GrSwizzle::BGRA());
                }
            } else {
                fp = GrTextureEffect::Make(std::move(tempView), alphaType);
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
            if (!this->copy(tempProxy.get(), tempOrigin, srcRect, dstPoint)) {
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
    direct->priv().flushSurface(caps->preferVRAMUseOverFlushes() ? dstProxy : nullptr);

    return direct->priv().getGpu()->writePixels(dstSurface, pt.fX, pt.fY, srcInfo.width(),
                                                srcInfo.height(), this->colorInfo().colorType(),
                                                srcColorType, src, rowBytes);
}

bool GrSurfaceContext::copy(GrSurfaceProxy* src, GrSurfaceOrigin origin, const SkIRect& srcRect,
                            const SkIPoint& dstPoint) {
    ASSERT_SINGLE_OWNER
    RETURN_FALSE_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(this->auditTrail(), "GrSurfaceContextPriv::copy");

    const GrCaps* caps = fContext->priv().caps();

    SkASSERT(src->backendFormat().textureType() != GrTextureType::kExternal);
    SkASSERT(src->backendFormat() == this->asSurfaceProxy()->backendFormat());
    SkASSERT(origin == this->origin());

    if (this->asSurfaceProxy()->framebufferOnly()) {
        return false;
    }

    if (!caps->canCopySurface(this->asSurfaceProxy(), src, srcRect, dstPoint)) {
        return false;
    }

    // The swizzle doesn't matter for copies and it is not used.
    return this->drawingManager()->newCopyRenderTask(
            GrSurfaceProxyView(sk_ref_sp(src), origin, GrSwizzle()), srcRect,
            this->readSurfaceView(), dstPoint);
}

std::unique_ptr<GrRenderTargetContext> GrSurfaceContext::rescale(
        const SkImageInfo& info,
        const SkIRect& srcRect,
        SkSurface::RescaleGamma rescaleGamma,
        SkFilterQuality rescaleQuality) {
    auto direct = fContext->priv().asDirectContext();
    if (!direct) {
        return nullptr;
    }
    auto rtProxy = this->asRenderTargetProxy();
    if (rtProxy && rtProxy->wrapsVkSecondaryCB()) {
        return nullptr;
    }

    if (this->asSurfaceProxy()->framebufferOnly()) {
        return nullptr;
    }

    // We rescale by drawing and don't currently support drawing to a kUnpremul destination.
    if (info.alphaType() == kUnpremul_SkAlphaType) {
        return nullptr;
    }

    int srcW = srcRect.width();
    int srcH = srcRect.height();
    int srcX = srcRect.fLeft;
    int srcY = srcRect.fTop;
    GrSurfaceProxyView texView = this->readSurfaceView();
    SkCanvas::SrcRectConstraint constraint = SkCanvas::kStrict_SrcRectConstraint;
    GrColorType srcColorType = this->colorInfo().colorType();
    SkAlphaType srcAlphaType = this->colorInfo().alphaType();
    if (!texView.asTextureProxy()) {
        texView = GrSurfaceProxy::Copy(fContext, this->asSurfaceProxy(), this->origin(),
                                       srcColorType, GrMipMapped::kNo, srcRect,
                                       SkBackingFit::kApprox, SkBudgeted::kNo);
        if (!texView.proxy()) {
            return nullptr;
        }
        SkASSERT(texView.asTextureProxy());
        srcX = 0;
        srcY = 0;
        constraint = SkCanvas::kFast_SrcRectConstraint;
    }

    float sx = (float)info.width() / srcW;
    float sy = (float)info.height() / srcH;

    // How many bilerp/bicubic steps to do in X and Y. + means upscaling, - means downscaling.
    int stepsX;
    int stepsY;
    if (rescaleQuality > kNone_SkFilterQuality) {
        stepsX = static_cast<int>((sx > 1.f) ? ceil(log2f(sx)) : floor(log2f(sx)));
        stepsY = static_cast<int>((sy > 1.f) ? ceil(log2f(sy)) : floor(log2f(sy)));
    } else {
        stepsX = sx != 1.f;
        stepsY = sy != 1.f;
    }
    SkASSERT(stepsX || stepsY);

    // Within a rescaling pass A is the input (if not null) and B is the output. At the end of the
    // pass B is moved to A. If 'this' is the input on the first pass then tempA is null.
    std::unique_ptr<GrRenderTargetContext> tempA;
    std::unique_ptr<GrRenderTargetContext> tempB;

    // Assume we should ignore the rescale linear request if the surface has no color space since
    // it's unclear how we'd linearize from an unknown color space.
    if (rescaleGamma == SkSurface::kLinear && this->colorInfo().colorSpace() &&
        !this->colorInfo().colorSpace()->gammaIsLinear()) {
        auto cs = this->colorInfo().colorSpace()->makeLinearGamma();
        auto xform = GrColorSpaceXform::Make(this->colorInfo().colorSpace(), srcAlphaType, cs.get(),
                                             kPremul_SkAlphaType);
        // We'll fall back to kRGBA_8888 if half float not supported.
        auto linearRTC = GrRenderTargetContext::MakeWithFallback(
                fContext, GrColorType::kRGBA_F16, cs, SkBackingFit::kExact, {srcW, srcH}, 1,
                GrMipMapped::kNo, GrProtected::kNo, kTopLeft_GrSurfaceOrigin);
        if (!linearRTC) {
            return nullptr;
        }
        linearRTC->drawTexture(GrNoClip(), std::move(texView), srcAlphaType,
                               GrSamplerState::Filter::kNearest, SkBlendMode::kSrc,
                               SK_PMColor4fWHITE, SkRect::Make(srcRect), SkRect::MakeWH(srcW, srcH),
                               GrAA::kNo, GrQuadAAFlags::kNone, constraint, SkMatrix::I(),
                               std::move(xform));
        texView = linearRTC->readSurfaceView();
        SkASSERT(texView.asTextureProxy());
        tempA = std::move(linearRTC);
        srcX = 0;
        srcY = 0;
        constraint = SkCanvas::kFast_SrcRectConstraint;
    }
    while (stepsX || stepsY) {
        int nextW = info.width();
        int nextH = info.height();
        if (stepsX < 0) {
            nextW = info.width() << (-stepsX - 1);
            stepsX++;
        } else if (stepsX != 0) {
            if (stepsX > 1) {
                nextW = srcW * 2;
            }
            --stepsX;
        }
        if (stepsY < 0) {
            nextH = info.height() << (-stepsY - 1);
            stepsY++;
        } else if (stepsY != 0) {
            if (stepsY > 1) {
                nextH = srcH * 2;
            }
            --stepsY;
        }
        auto input = tempA ? tempA.get() : this;
        GrColorType colorType = input->colorInfo().colorType();
        auto cs = input->colorInfo().refColorSpace();
        sk_sp<GrColorSpaceXform> xform;
        auto prevAlphaType = input->colorInfo().alphaType();
        if (!stepsX && !stepsY) {
            // Might as well fold conversion to final info in the last step.
            cs = info.refColorSpace();
            colorType = SkColorTypeToGrColorType(info.colorType());
            xform = GrColorSpaceXform::Make(input->colorInfo().colorSpace(),
                                            input->colorInfo().alphaType(), cs.get(),
                                            info.alphaType());
        }
        tempB = GrRenderTargetContext::MakeWithFallback(
                fContext, colorType, std::move(cs), SkBackingFit::kExact, {nextW, nextH}, 1,
                GrMipMapped::kNo, GrProtected::kNo, kTopLeft_GrSurfaceOrigin);
        if (!tempB) {
            return nullptr;
        }
        auto dstRect = SkRect::MakeWH(nextW, nextH);
        if (rescaleQuality == kHigh_SkFilterQuality) {
            SkMatrix matrix;
            matrix.setScaleTranslate((float)srcW / nextW, (float)srcH / nextH, srcX, srcY);
            std::unique_ptr<GrFragmentProcessor> fp;
            auto dir = GrBicubicEffect::Direction::kXY;
            if (nextW == srcW) {
                dir = GrBicubicEffect::Direction::kY;
            } else if (nextH == srcH) {
                dir = GrBicubicEffect::Direction::kX;
            }
            static constexpr GrSamplerState::WrapMode kWM = GrSamplerState::WrapMode::kClamp;
            auto subset = SkRect::MakeXYWH(srcX, srcY, srcW, srcH);
            fp = GrBicubicEffect::MakeSubset(std::move(texView), prevAlphaType, matrix, kWM, kWM,
                                             subset, dir, *this->caps());
            if (xform) {
                fp = GrColorSpaceXformEffect::Make(std::move(fp), std::move(xform));
            }
            GrPaint paint;
            paint.addColorFragmentProcessor(std::move(fp));
            paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
            tempB->fillRectToRect(GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(), dstRect,
                                    dstRect);
        } else {
            auto filter = rescaleQuality == kNone_SkFilterQuality ? GrSamplerState::Filter::kNearest
                                                                  : GrSamplerState::Filter::kBilerp;
            auto srcSubset = SkRect::MakeXYWH(srcX, srcY, srcW, srcH);
            tempB->drawTexture(GrNoClip(), std::move(texView), srcAlphaType, filter,
                               SkBlendMode::kSrc, SK_PMColor4fWHITE, srcSubset, dstRect, GrAA::kNo,
                               GrQuadAAFlags::kNone, constraint, SkMatrix::I(), std::move(xform));
        }
        texView = tempB->readSurfaceView();
        tempA = std::move(tempB);
        srcX = srcY = 0;
        srcW = nextW;
        srcH = nextH;
        constraint = SkCanvas::kFast_SrcRectConstraint;
    }
    SkASSERT(tempA);
    return tempA;
}

GrSurfaceContext::PixelTransferResult GrSurfaceContext::transferPixels(GrColorType dstCT,
                                                                       const SkIRect& rect) {
    SkASSERT(rect.fLeft >= 0 && rect.fRight <= this->width());
    SkASSERT(rect.fTop >= 0 && rect.fBottom <= this->height());
    auto direct = fContext->priv().asDirectContext();
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
    SkASSERT(fContext->priv().caps()->areColorTypeAndFormatCompatible(
            this->colorInfo().colorType(), fReadView.proxy()->backendFormat()));

    this->onValidate();
}
#endif

