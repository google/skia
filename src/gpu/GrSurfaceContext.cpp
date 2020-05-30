/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrSurfaceContext.h"

#include "include/effects/SkRuntimeEffect.h"
#include "include/private/GrRecordingContext.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/gpu/GrAuditTrail.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrDataUtils.h"
#include "src/gpu/GrDrawingManager.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrImageInfo.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrSurfaceContextPriv.h"
#include "src/gpu/GrSurfacePriv.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrBicubicEffect.h"
#include "src/gpu/effects/GrSkSLFP.h"

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
        // Will we ever want a swizzle that is not the default write swizzle for the format and
        // colorType here? If so we will need to manually pass that in.
        GrSwizzle writeSwizzle;
        if (colorType != GrColorType::kUnknown) {
            writeSwizzle =
                    context->priv().caps()->getWriteSwizzle(proxy->backendFormat(), colorType);
        }
        GrSurfaceProxyView writeView(readView.refProxy(), readView.origin(), writeSwizzle);
        surfaceContext.reset(new GrRenderTargetContext(context, std::move(readView),
                                                       std::move(writeView), colorType,
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
    GrSwizzle swizzle;
    if (colorType != GrColorType::kUnknown && !context->priv().caps()->isFormatCompressed(format)) {
        swizzle = context->priv().caps()->getReadSwizzle(format, colorType);
    }

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
                nullptr, std::move(paint), GrAA::kNo, SkMatrix::I(),
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
    direct->submit();
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
                    nullptr, std::move(paint), GrAA::kNo, SkMatrix::I(),
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

template <int N> static std::unique_ptr<GrSkSLFP> make_avg_effect(GrRecordingContext* context) {
    static sk_sp<SkRuntimeEffect> gEffect;
    static SkString gName;
    if (!gEffect) {
        SkString string;
        for (int i = 0; i < N; ++i) {
            string.appendf("in fragmentProcessor child%d;\n", i);
        }
        string.append("void main(float2 p, inout half4 color) {\n");
        for (int i = 0; i < N; ++i) {
            string.appendf("    color %c= sample(child%d, p);\n", i ? '+' : ' ', i);
        }
        string.appendf("    color /= half(%d);\n"
                       "}\n", N);
        gEffect = std::get<0>(SkRuntimeEffect::Make(std::move(string)));
        SkASSERT(gEffect);
        gName.printf("Avg%d", N);
    }
    return GrSkSLFP::Make(context, gEffect, gName.c_str(), nullptr);
}

template <int NX, int NY>
static std::unique_ptr<GrFragmentProcessor> make_multibilerp_effect(GrRecordingContext* context,
                                                                    GrSurfaceProxyView srcView,
                                                                    SkAlphaType alphaType,
                                                                    const SkIRect& srcRect,
                                                                    const SkISize& dstSize) {
    auto effect = make_avg_effect<NX*NY>(context);

    // scale factors.
    float sx = static_cast<float>(srcRect.width()) /dstSize.width(),
          sy = static_cast<float>(srcRect.height())/dstSize.height();
    // spacing between bilerp samples.
    float dx = sx/NX,
          dy = sy/NY;
    // offset in src from back projection of dst pixel center to left/upper-most bilerp sample.
    float x0 = (dx - sx)/2,
          y0 = (dy - sy)/2;

    const auto& caps = *context->priv().caps();
    for (int j = 0; j < NY; ++j) {
        for (int i = 0; i < NX; ++i) {
            float tx = x0 + i*dx,
                  ty = y0 + j*dy;
            SkMatrix m = SkMatrix::Scale(sx, sy);
            m.postTranslate(tx + srcRect.x(), ty + srcRect.y());
            SkRect domain = SkRect::Make(srcRect).makeInset(sx/2, sy/2).makeOffset(tx, ty);
            effect->addChild(GrTextureEffect::MakeSubset(srcView, alphaType, m,
                                                         GrSamplerState::Filter::kBilerp,
                                                         SkRect::Make(srcRect), domain, caps));
        }
    }
    return std::move(effect);
}

std::unique_ptr<GrRenderTargetContext> GrSurfaceContext::rescale(
        const GrImageInfo& info,
        GrSurfaceOrigin origin,
        SkIRect srcRect,
        SkSurface::RescaleGamma rescaleGamma,
        SkFilterQuality rescaleQuality) {
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

    GrSurfaceProxyView texView = this->readSurfaceView();
    SkAlphaType srcAlphaType = this->colorInfo().alphaType();
    if (!texView.asTextureProxy()) {
        texView = GrSurfaceProxyView::Copy(fContext, std::move(texView), GrMipMapped::kNo, srcRect,
                                           SkBackingFit::kApprox, SkBudgeted::kNo);
        if (!texView) {
            return nullptr;
        }
        SkASSERT(texView.asTextureProxy());
        srcRect = SkIRect::MakeSize(srcRect.size());
    }

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
                fContext, GrColorType::kRGBA_F16, cs, SkBackingFit::kApprox, srcRect.size(), 1,
                GrMipMapped::kNo, GrProtected::kNo, origin);
        if (!linearRTC) {
            return nullptr;
        }
        // 1-to-1 draw can always be kFast.
        linearRTC->drawTexture(nullptr, std::move(texView), srcAlphaType,
                               GrSamplerState::Filter::kNearest, SkBlendMode::kSrc,
                               SK_PMColor4fWHITE, SkRect::Make(srcRect),
                               SkRect::Make(srcRect.size()), GrAA::kNo, GrQuadAAFlags::kNone,
                               SkCanvas::kFast_SrcRectConstraint, SkMatrix::I(), std::move(xform));
        texView = linearRTC->readSurfaceView();
        SkASSERT(texView.asTextureProxy());
        tempA = std::move(linearRTC);
        srcRect = SkIRect::MakeSize(srcRect.size());
    }

    enum StepType {
        kNearest,
        kBilinear,
        kFourTapBilinear,
        kBicubic,
    };
    auto determineNextStep = [rescaleQuality, finalSize = info.dimensions()](SkISize srcSize) {
        if (rescaleQuality == kNone_SkFilterQuality) {
            return std::make_tuple(StepType::kNearest, finalSize);
        }
        SkISize nextSize;
        if (srcSize.width() > finalSize.width()) {
            nextSize.fWidth = std::max((srcSize.width() + 1)/2, finalSize.width());
        } else {
            nextSize.fWidth = std::min(srcSize.width()*2, finalSize.width());
        }
        if (srcSize.height() > finalSize.height()) {
            nextSize.fHeight = std::max((srcSize.height() + 1)/2, finalSize.height());
        } else {
            nextSize.fHeight = std::min(srcSize.height()*2, finalSize.height());
        }
        if (rescaleQuality == kHigh_SkFilterQuality) {
            return std::make_tuple(StepType::kBicubic, nextSize);
        }
        // See if we can do multiple bilinear steps in one.
        if (nextSize.width() > finalSize.width() && nextSize.height() > finalSize.height()) {
            nextSize = {std::max((nextSize.width()  + 1)/2, finalSize.width()),
                        std::max((nextSize.height() + 1)/2, finalSize.height())};
            return std::make_tuple(StepType::kFourTapBilinear, nextSize);
        }
        return std::make_tuple(StepType::kBilinear, nextSize);
    };
    while (srcRect.size() != info.dimensions()) {
        auto [stepType, nextDims] = determineNextStep(srcRect.size());
        auto input = tempA ? tempA.get() : this;
        auto colorType = input->colorInfo().colorType();
        auto cs = input->colorInfo().refColorSpace();
        sk_sp<GrColorSpaceXform> xform;
        auto prevAlphaType = input->colorInfo().alphaType();
        if (nextDims == info.dimensions()) {
            // Might as well fold conversion to final info in the last step.
            cs = info.refColorSpace();
            xform = GrColorSpaceXform::Make(input->colorInfo().colorSpace(),
                                            input->colorInfo().alphaType(), cs.get(),
                                            info.alphaType());
        }
        tempB = GrRenderTargetContext::MakeWithFallback(fContext, colorType, std::move(cs),
                                                        SkBackingFit::kApprox, nextDims, 1,
                                                        GrMipMapped::kNo, GrProtected::kNo, origin);
        if (!tempB) {
            return nullptr;
        }
        std::unique_ptr<GrFragmentProcessor> srcFP;
        switch (stepType) {
            case StepType::kBicubic: {
                SkMatrix matrix;
                matrix.setScaleTranslate((float)srcRect.width() /nextDims.width(),
                                         (float)srcRect.height()/nextDims.height(),
                                         srcRect.x(),
                                         srcRect.y());
                auto dir = GrBicubicEffect::Direction::kXY;
                if (nextDims.width() == srcRect.width()) {
                    dir = GrBicubicEffect::Direction::kY;
                } else if (nextDims.height() == srcRect.height()) {
                    dir = GrBicubicEffect::Direction::kX;
                }
                static constexpr GrSamplerState::WrapMode kWM = GrSamplerState::WrapMode::kClamp;
                srcFP = GrBicubicEffect::MakeSubset(std::move(texView), prevAlphaType, matrix, kWM,
                                                    kWM, SkRect::Make(srcRect), dir, *this->caps());
                break;
            }
            case StepType::kFourTapBilinear:
                srcFP = make_multibilerp_effect<2, 2>(fContext, texView, srcAlphaType, srcRect,
                                                      nextDims);
                break;
            default: {
                auto filter = stepType == StepType::kNearest ? GrSamplerState::Filter::kNearest
                                                             : GrSamplerState::Filter::kBilerp;
                float sx = static_cast<float>(srcRect.width()) /nextDims.width(),
                      sy = static_cast<float>(srcRect.height())/nextDims.height();
                SkMatrix matrix;
                matrix.setScaleTranslate(sx, sy, srcRect.x(), srcRect.y());
                SkRect domain = SkRect::Make(srcRect).makeInset(sx/2, sy/2);
                srcFP = GrTextureEffect::MakeSubset(std::move(texView), srcAlphaType, matrix,
                                                    filter, SkRect::Make(srcRect), domain,
                                                    *this->caps());
                break;
            }
        }
        GrPaint paint;
        paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
        srcFP = GrColorSpaceXformEffect::Make(std::move(srcFP), std::move(xform));
        paint.addColorFragmentProcessor(std::move(srcFP));
        tempB->drawRect(nullptr, std::move(paint), GrAA::kNo, SkMatrix::I(),
                        SkRect::Make(nextDims));
        texView = tempB->readSurfaceView();
        tempA = std::move(tempB);
        srcRect = SkIRect::MakeSize(nextDims);
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
    if (this->colorInfo().colorType() != GrColorType::kUnknown) {
        SkASSERT(fContext->priv().caps()->areColorTypeAndFormatCompatible(
                this->colorInfo().colorType(), fReadView.proxy()->backendFormat()));
    }
    this->onValidate();
}
#endif
