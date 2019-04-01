/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGpuBlurUtils.h"

#include "SkRect.h"

#if SK_SUPPORT_GPU
#include "GrCaps.h"
#include "GrFixedClip.h"
#include "GrRecordingContext.h"
#include "GrRecordingContextPriv.h"
#include "GrRenderTargetContext.h"
#include "GrRenderTargetContextPriv.h"
#include "effects/GrGaussianConvolutionFragmentProcessor.h"
#include "effects/GrMatrixConvolutionEffect.h"

#include "SkGr.h"

#define MAX_BLUR_SIGMA 4.0f

using Direction = GrGaussianConvolutionFragmentProcessor::Direction;

static void scale_irect_roundout(SkIRect* rect, float xScale, float yScale) {
    rect->fLeft   = SkScalarFloorToInt(rect->fLeft  * xScale);
    rect->fTop    = SkScalarFloorToInt(rect->fTop   * yScale);
    rect->fRight  = SkScalarCeilToInt(rect->fRight  * xScale);
    rect->fBottom = SkScalarCeilToInt(rect->fBottom * yScale);
}

static void scale_irect(SkIRect* rect, int xScale, int yScale) {
    rect->fLeft   *= xScale;
    rect->fTop    *= yScale;
    rect->fRight  *= xScale;
    rect->fBottom *= yScale;
}

#ifdef SK_DEBUG
static inline int is_even(int x) { return !(x & 1); }
#endif

static void shrink_irect_by_2(SkIRect* rect, bool xAxis, bool yAxis) {
    if (xAxis) {
        SkASSERT(is_even(rect->fLeft) && is_even(rect->fRight));
        rect->fLeft /= 2;
        rect->fRight /= 2;
    }
    if (yAxis) {
        SkASSERT(is_even(rect->fTop) && is_even(rect->fBottom));
        rect->fTop /= 2;
        rect->fBottom /= 2;
    }
}

static float adjust_sigma(float sigma, int maxTextureSize, int *scaleFactor, int *radius) {
    *scaleFactor = 1;
    while (sigma > MAX_BLUR_SIGMA) {
        *scaleFactor *= 2;
        sigma *= 0.5f;
        if (*scaleFactor > maxTextureSize) {
            *scaleFactor = maxTextureSize;
            sigma = MAX_BLUR_SIGMA;
        }
    }
    *radius = static_cast<int>(ceilf(sigma * 3.0f));
    SkASSERT(*radius <= GrGaussianConvolutionFragmentProcessor::kMaxKernelRadius);
    return sigma;
}

static void convolve_gaussian_1d(GrRenderTargetContext* renderTargetContext,
                                 const GrClip& clip,
                                 const SkIRect& dstRect,
                                 const SkIPoint& srcOffset,
                                 sk_sp<GrTextureProxy> proxy,
                                 Direction direction,
                                 int radius,
                                 float sigma,
                                 GrTextureDomain::Mode mode,
                                 int bounds[2]) {
    GrPaint paint;
    std::unique_ptr<GrFragmentProcessor> conv(GrGaussianConvolutionFragmentProcessor::Make(
            std::move(proxy), direction, radius, sigma, mode, bounds));
    paint.addColorFragmentProcessor(std::move(conv));
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
    SkMatrix localMatrix = SkMatrix::MakeTrans(-SkIntToScalar(srcOffset.x()),
                                               -SkIntToScalar(srcOffset.y()));
    renderTargetContext->fillRectWithLocalMatrix(clip, std::move(paint), GrAA::kNo, SkMatrix::I(),
                                                 SkRect::Make(dstRect), localMatrix);
}

static GrPixelConfig get_blur_config(GrTextureProxy* proxy) {
    GrPixelConfig config = proxy->config();

    SkASSERT(kBGRA_8888_GrPixelConfig == config || kRGBA_8888_GrPixelConfig == config ||
             kRGB_888_GrPixelConfig == config || kRGBA_4444_GrPixelConfig == config ||
             kRGB_565_GrPixelConfig == config || kSRGBA_8888_GrPixelConfig == config ||
             kSBGRA_8888_GrPixelConfig == config || kRGBA_half_GrPixelConfig == config ||
             kAlpha_8_GrPixelConfig == config || kRGBA_1010102_GrPixelConfig == config ||
             kRGBA_half_Clamped_GrPixelConfig == config);

    return config;
}

static sk_sp<GrRenderTargetContext> convolve_gaussian_2d(GrRecordingContext* context,
                                                         sk_sp<GrTextureProxy> proxy,
                                                         const SkIRect& srcBounds,
                                                         const SkIPoint& srcOffset,
                                                         int radiusX,
                                                         int radiusY,
                                                         SkScalar sigmaX,
                                                         SkScalar sigmaY,
                                                         GrTextureDomain::Mode mode,
                                                         const SkImageInfo& dstII,
                                                         SkBackingFit dstFit) {

    GrPixelConfig config = get_blur_config(proxy.get());

    GrBackendFormat format = proxy->backendFormat().makeTexture2D();
    if (!format.isValid()) {
        return nullptr;
    }

    sk_sp<GrRenderTargetContext> renderTargetContext;
    renderTargetContext = context->priv().makeDeferredRenderTargetContext(
                                                         format,
                                                         dstFit, dstII.width(), dstII.height(),
                                                         config, dstII.refColorSpace(),
                                                         1, GrMipMapped::kNo,
                                                         proxy->origin());
    if (!renderTargetContext) {
        return nullptr;
    }

    SkMatrix localMatrix = SkMatrix::MakeTrans(-SkIntToScalar(srcOffset.x()),
                                               -SkIntToScalar(srcOffset.y()));
    SkISize size = SkISize::Make(2 * radiusX + 1,  2 * radiusY + 1);
    SkIPoint kernelOffset = SkIPoint::Make(radiusX, radiusY);
    GrPaint paint;
    auto conv = GrMatrixConvolutionEffect::MakeGaussian(std::move(proxy), srcBounds, size, 1.0, 0.0,
                                                        kernelOffset, mode, true, sigmaX, sigmaY);
    paint.addColorFragmentProcessor(std::move(conv));
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
    GrFixedClip clip(dstII.bounds());

    renderTargetContext->fillRectWithLocalMatrix(clip, std::move(paint), GrAA::kNo, SkMatrix::I(),
                                                 SkRect::Make(dstII.bounds()), localMatrix);

    return renderTargetContext;
}

static sk_sp<GrRenderTargetContext> convolve_gaussian(GrRecordingContext* context,
                                                      sk_sp<GrTextureProxy> proxy,
                                                      const SkIRect& srcRect,
                                                      const SkIPoint& srcOffset,
                                                      Direction direction,
                                                      int radius,
                                                      float sigma,
                                                      SkIRect* contentRect,
                                                      GrTextureDomain::Mode mode,
                                                      const SkImageInfo& dstII,
                                                      SkBackingFit fit) {
    SkASSERT(srcRect.width() <= dstII.width() && srcRect.height() <= dstII.height());

    GrPixelConfig config = get_blur_config(proxy.get());

    GrBackendFormat format = proxy->backendFormat().makeTexture2D();
    if (!format.isValid()) {
        return nullptr;
    }

    sk_sp<GrRenderTargetContext> dstRenderTargetContext;
    dstRenderTargetContext = context->priv().makeDeferredRenderTargetContext(
                                                                format,
                                                                fit, srcRect.width(),
                                                                srcRect.height(),
                                                                config,
                                                                dstII.refColorSpace(),
                                                                1, GrMipMapped::kNo,
                                                                proxy->origin());
    if (!dstRenderTargetContext) {
        return nullptr;
    }

    GrFixedClip clip(dstII.bounds());

    int bounds[2] = { 0, 0 };
    SkIRect dstRect = SkIRect::MakeWH(srcRect.width(), srcRect.height());
    if (GrTextureDomain::kIgnore_Mode == mode) {
        *contentRect = dstRect;
        convolve_gaussian_1d(dstRenderTargetContext.get(), clip, dstRect, srcOffset,
                             std::move(proxy), direction, radius, sigma,
                             GrTextureDomain::kIgnore_Mode, bounds);
        return dstRenderTargetContext;
    }

    SkIRect midRect = *contentRect, leftRect, rightRect;
    midRect.offset(srcOffset);
    SkIRect topRect, bottomRect;
    if (Direction::kX == direction) {
        bounds[0] = contentRect->left();
        bounds[1] = contentRect->right();
        topRect = SkIRect::MakeLTRB(0, 0, dstRect.right(), midRect.top());
        bottomRect = SkIRect::MakeLTRB(0, midRect.bottom(), dstRect.right(), dstRect.bottom());
        midRect.inset(radius, 0);
        leftRect = SkIRect::MakeLTRB(0, midRect.top(), midRect.left(), midRect.bottom());
        rightRect =
            SkIRect::MakeLTRB(midRect.right(), midRect.top(), dstRect.width(), midRect.bottom());
        dstRect.fTop = midRect.top();
        dstRect.fBottom = midRect.bottom();

        contentRect->fLeft = dstRect.fLeft;
        contentRect->fTop = midRect.fTop;
        contentRect->fRight = dstRect.fRight;
        contentRect->fBottom = midRect.fBottom;
    } else {
        bounds[0] = contentRect->top();
        bounds[1] = contentRect->bottom();
        topRect = SkIRect::MakeLTRB(0, 0, midRect.left(), dstRect.bottom());
        bottomRect = SkIRect::MakeLTRB(midRect.right(), 0, dstRect.right(), dstRect.bottom());
        midRect.inset(0, radius);
        leftRect = SkIRect::MakeLTRB(midRect.left(), 0, midRect.right(), midRect.top());
        rightRect =
            SkIRect::MakeLTRB(midRect.left(), midRect.bottom(), midRect.right(), dstRect.height());
        dstRect.fLeft = midRect.left();
        dstRect.fRight = midRect.right();

        contentRect->fLeft = midRect.fLeft;
        contentRect->fTop = dstRect.fTop;
        contentRect->fRight = midRect.fRight;
        contentRect->fBottom = dstRect.fBottom;
    }
    if (!topRect.isEmpty()) {
        dstRenderTargetContext->clear(&topRect, SK_PMColor4fTRANSPARENT,
                                      GrRenderTargetContext::CanClearFullscreen::kNo);
    }

    if (!bottomRect.isEmpty()) {
        dstRenderTargetContext->clear(&bottomRect, SK_PMColor4fTRANSPARENT,
                                      GrRenderTargetContext::CanClearFullscreen::kNo);
    }

    if (midRect.isEmpty()) {
        // Blur radius covers srcBounds; use bounds over entire draw
        convolve_gaussian_1d(dstRenderTargetContext.get(), clip, dstRect, srcOffset,
                             std::move(proxy), direction, radius, sigma, mode, bounds);
    } else {
        // Draw right and left margins with bounds; middle without.
        convolve_gaussian_1d(dstRenderTargetContext.get(), clip, leftRect, srcOffset,
                             proxy, direction, radius, sigma, mode, bounds);
        convolve_gaussian_1d(dstRenderTargetContext.get(), clip, rightRect, srcOffset,
                             proxy, direction, radius, sigma, mode, bounds);
        convolve_gaussian_1d(dstRenderTargetContext.get(), clip, midRect, srcOffset,
                             std::move(proxy), direction, radius, sigma,
                             GrTextureDomain::kIgnore_Mode, bounds);
    }

    return dstRenderTargetContext;
}

static sk_sp<GrTextureProxy> decimate(GrRecordingContext* context,
                                      sk_sp<GrTextureProxy> src,
                                      SkIPoint* srcOffset,
                                      SkIRect* contentRect,
                                      int scaleFactorX, int scaleFactorY,
                                      bool willBeXFiltering, bool willBeYFiltering,
                                      int radiusX, int radiusY,
                                      GrTextureDomain::Mode mode,
                                      const SkImageInfo& dstII) {
    SkASSERT(SkIsPow2(scaleFactorX) && SkIsPow2(scaleFactorY));
    SkASSERT(scaleFactorX > 1 || scaleFactorY > 1);

    GrPixelConfig config = get_blur_config(src.get());

    SkIRect srcRect;
    if (GrTextureDomain::kIgnore_Mode == mode) {
        srcRect = dstII.bounds();
    } else {
        srcRect = *contentRect;
        srcRect.offset(*srcOffset);
    }

    scale_irect_roundout(&srcRect, 1.0f / scaleFactorX, 1.0f / scaleFactorY);
    scale_irect(&srcRect, scaleFactorX, scaleFactorY);

    SkIRect dstRect(srcRect);

    sk_sp<GrRenderTargetContext> dstRenderTargetContext;

    GrBackendFormat format = src->backendFormat().makeTexture2D();
    if (!format.isValid()) {
        return nullptr;
    }

    for (int i = 1; i < scaleFactorX || i < scaleFactorY; i *= 2) {
        shrink_irect_by_2(&dstRect, i < scaleFactorX, i < scaleFactorY);

        // We know this will not be the final draw so we are free to make it an approx match.
        dstRenderTargetContext = context->priv().makeDeferredRenderTargetContext(
                                                    format,
                                                    SkBackingFit::kApprox,
                                                    dstRect.fRight,
                                                    dstRect.fBottom,
                                                    config, dstII.refColorSpace(),
                                                    1, GrMipMapped::kNo,
                                                    src->origin());
        if (!dstRenderTargetContext) {
            return nullptr;
        }

        GrPaint paint;
        if (GrTextureDomain::kIgnore_Mode != mode && i == 1) {
            // GrTextureDomainEffect does not support kRepeat_Mode with GrSamplerState::Filter.
            GrTextureDomain::Mode modeForScaling = GrTextureDomain::kRepeat_Mode == mode
                                                                ? GrTextureDomain::kDecal_Mode
                                                                : mode;

            SkRect domain = SkRect::Make(*contentRect);
            domain.inset((i < scaleFactorX) ? SK_ScalarHalf + SK_ScalarNearlyZero : 0.0f,
                         (i < scaleFactorY) ? SK_ScalarHalf + SK_ScalarNearlyZero : 0.0f);
            auto fp = GrTextureDomainEffect::Make(std::move(src),
                                                  SkMatrix::I(),
                                                  domain,
                                                  modeForScaling,
                                                  GrSamplerState::Filter::kBilerp);
            paint.addColorFragmentProcessor(std::move(fp));
            srcRect.offset(-(*srcOffset));
            // TODO: consume the srcOffset in both first draws and always set it to zero
            // back in GaussianBlur
            srcOffset->set(0, 0);
        } else {
            paint.addColorTextureProcessor(std::move(src), SkMatrix::I(),
                                           GrSamplerState::ClampBilerp());
        }
        paint.setPorterDuffXPFactory(SkBlendMode::kSrc);

        GrFixedClip clip(dstRect);
        dstRenderTargetContext->fillRectToRect(clip, std::move(paint), GrAA::kNo,
                                               SkMatrix::I(), SkRect::Make(dstRect),
                                               SkRect::Make(srcRect));

        src = dstRenderTargetContext->asTextureProxyRef();
        if (!src) {
            return nullptr;
        }
        srcRect = dstRect;
    }

    *contentRect = dstRect;

    SkASSERT(dstRenderTargetContext);

    if (willBeXFiltering) {
        if (scaleFactorX > 1) {
            // Clear out a radius to the right of the contentRect to prevent the
            // X convolution from reading garbage.
            SkIRect clearRect = SkIRect::MakeXYWH(contentRect->fRight, contentRect->fTop,
                                                  radiusX, contentRect->height());
            dstRenderTargetContext->priv().absClear(&clearRect, SK_PMColor4fTRANSPARENT);
        }
    } else {
        if (scaleFactorY > 1) {
            // Clear out a radius below the contentRect to prevent the Y
            // convolution from reading garbage.
            SkIRect clearRect = SkIRect::MakeXYWH(contentRect->fLeft, contentRect->fBottom,
                                                  contentRect->width(), radiusY);
            dstRenderTargetContext->priv().absClear(&clearRect, SK_PMColor4fTRANSPARENT);
        }
    }

    return dstRenderTargetContext->asTextureProxyRef();
}

// Expand the contents of 'srcRenderTargetContext' to fit in 'dstII'.
static sk_sp<GrRenderTargetContext> reexpand(GrRecordingContext* context,
                                             sk_sp<GrRenderTargetContext> srcRenderTargetContext,
                                             const SkIRect& localSrcBounds,
                                             int scaleFactorX, int scaleFactorY,
                                             GrTextureDomain::Mode mode,
                                             const SkImageInfo& dstII,
                                             SkBackingFit fit) {
    const SkIRect srcRect = SkIRect::MakeWH(srcRenderTargetContext->width(),
                                            srcRenderTargetContext->height());

    // Clear one pixel to the right and below, to accommodate bilinear upsampling.
    // TODO: it seems like we should actually be clamping here rather than darkening
    // the bottom right edges.
    SkIRect clearRect = SkIRect::MakeXYWH(srcRect.fLeft, srcRect.fBottom, srcRect.width() + 1, 1);
    srcRenderTargetContext->priv().absClear(&clearRect, SK_PMColor4fTRANSPARENT);
    clearRect = SkIRect::MakeXYWH(srcRect.fRight, srcRect.fTop, 1, srcRect.height());
    srcRenderTargetContext->priv().absClear(&clearRect, SK_PMColor4fTRANSPARENT);

    sk_sp<GrTextureProxy> srcProxy = srcRenderTargetContext->asTextureProxyRef();
    if (!srcProxy) {
        return nullptr;
    }

    srcRenderTargetContext = nullptr; // no longer needed

    GrPixelConfig config = get_blur_config(srcProxy.get());

    GrBackendFormat format = srcProxy->backendFormat().makeTexture2D();
    if (!format.isValid()) {
        return nullptr;
    }

    sk_sp<GrRenderTargetContext> dstRenderTargetContext =
        context->priv().makeDeferredRenderTargetContext(format,
                                                               fit, dstII.width(), dstII.height(),
                                                               config, dstII.refColorSpace(),
                                                               1, GrMipMapped::kNo,
                                                               srcProxy->origin());
    if (!dstRenderTargetContext) {
        return nullptr;
    }

    GrPaint paint;
    if (GrTextureDomain::kIgnore_Mode != mode) {
        // GrTextureDomainEffect does not support kRepeat_Mode with GrSamplerState::Filter.
        GrTextureDomain::Mode modeForScaling = GrTextureDomain::kRepeat_Mode == mode
                                                            ? GrTextureDomain::kDecal_Mode
                                                            : mode;

        SkRect domain = SkRect::Make(localSrcBounds);
        auto fp = GrTextureDomainEffect::Make(std::move(srcProxy),
                                                SkMatrix::I(),
                                                domain,
                                                modeForScaling,
                                                GrSamplerState::Filter::kBilerp);
        paint.addColorFragmentProcessor(std::move(fp));
    } else {
        // FIXME:  this should be mitchell, not bilinear.
        paint.addColorTextureProcessor(std::move(srcProxy), SkMatrix::I(),
                                       GrSamplerState::ClampBilerp());
    }
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
    GrFixedClip clip(dstII.bounds());

    // TODO: using dstII as dstRect results in some image diffs - why?
    SkIRect dstRect(srcRect);
    scale_irect(&dstRect, scaleFactorX, scaleFactorY);

    dstRenderTargetContext->fillRectToRect(clip, std::move(paint), GrAA::kNo, SkMatrix::I(),
                                           SkRect::Make(dstRect), SkRect::Make(srcRect));

    return dstRenderTargetContext;
}

namespace SkGpuBlurUtils {

sk_sp<GrRenderTargetContext> GaussianBlur(GrRecordingContext* context,
                                          sk_sp<GrTextureProxy> srcProxy,
                                          sk_sp<SkColorSpace> colorSpace,
                                          const SkIRect& dstBounds,
                                          const SkIRect& srcBounds,
                                          float sigmaX,
                                          float sigmaY,
                                          GrTextureDomain::Mode mode,
                                          SkAlphaType at,
                                          SkBackingFit fit) {
    SkASSERT(context);

    const GrPixelConfig config = get_blur_config(srcProxy.get());
    SkColorType ct;
    if (!GrPixelConfigToColorType(config, &ct)) {
        return nullptr;
    }

    const SkImageInfo finalDestII = SkImageInfo::Make(dstBounds.width(), dstBounds.height(),
                                                      ct, at, std::move(colorSpace));

    int scaleFactorX, radiusX;
    int scaleFactorY, radiusY;
    int maxTextureSize = context->priv().caps()->maxTextureSize();
    sigmaX = adjust_sigma(sigmaX, maxTextureSize, &scaleFactorX, &radiusX);
    sigmaY = adjust_sigma(sigmaY, maxTextureSize, &scaleFactorY, &radiusY);
    SkASSERT(sigmaX || sigmaY);

    SkIPoint srcOffset = SkIPoint::Make(-dstBounds.x(), -dstBounds.y());

    // For really small blurs (certainly no wider than 5x5 on desktop gpus) it is faster to just
    // launch a single non separable kernel vs two launches
    if (sigmaX > 0.0f && sigmaY > 0.0f &&
            (2 * radiusX + 1) * (2 * radiusY + 1) <= MAX_KERNEL_SIZE) {
        // We shouldn't be scaling because this is a small size blur
        SkASSERT((1 == scaleFactorX) && (1 == scaleFactorY));

        return convolve_gaussian_2d(context, std::move(srcProxy), srcBounds, srcOffset,
                                    radiusX, radiusY, sigmaX, sigmaY,
                                    mode, finalDestII, fit);
    }

    // Only the last rendered renderTargetContext needs to match the supplied 'fit'
    SkBackingFit xFit = fit, yFit = fit;
    if (scaleFactorX > 1 || scaleFactorY > 1) {
        xFit = yFit = SkBackingFit::kApprox;  // reexpand will be last
    } else if (sigmaY > 0.0f) {
        xFit = SkBackingFit::kApprox;         // the y-pass will be last
    }

    SkIRect localSrcBounds = srcBounds;

    if (scaleFactorX > 1 || scaleFactorY > 1) {
        srcProxy = decimate(context, std::move(srcProxy), &srcOffset, &localSrcBounds,
                            scaleFactorX, scaleFactorY, sigmaX > 0.0f, sigmaY > 0.0f,
                            radiusX, radiusY, mode, finalDestII);
        if (!srcProxy) {
            return nullptr;
        }
    }

    sk_sp<GrRenderTargetContext> dstRenderTargetContext;

    SkIRect srcRect = finalDestII.bounds();
    scale_irect_roundout(&srcRect, 1.0f / scaleFactorX, 1.0f / scaleFactorY);
    if (sigmaX > 0.0f) {
        dstRenderTargetContext = convolve_gaussian(context, std::move(srcProxy), srcRect, srcOffset,
                                                   Direction::kX, radiusX, sigmaX, &localSrcBounds,
                                                   mode, finalDestII, xFit);
        if (!dstRenderTargetContext) {
            return nullptr;
        }

        if (sigmaY > 0.0f) {
            // Clear out a radius below the srcRect to prevent the Y
            // convolution from reading garbage.
            SkIRect clearRect = SkIRect::MakeXYWH(srcRect.fLeft, srcRect.fBottom,
                                                  srcRect.width(), radiusY);
            dstRenderTargetContext->priv().absClear(&clearRect, SK_PMColor4fTRANSPARENT);
        }

        srcProxy = dstRenderTargetContext->asTextureProxyRef();
        if (!srcProxy) {
            return nullptr;
        }

        srcRect.offsetTo(0, 0);
        srcOffset.set(0, 0);
    }

    if (sigmaY > 0.0f) {
        dstRenderTargetContext = convolve_gaussian(context, std::move(srcProxy), srcRect, srcOffset,
                                                   Direction::kY, radiusY, sigmaY, &localSrcBounds,
                                                   mode, finalDestII, yFit);
        if (!dstRenderTargetContext) {
            return nullptr;
        }

        srcProxy = dstRenderTargetContext->asTextureProxyRef();
        if (!srcProxy) {
            return nullptr;
        }

        srcRect.offsetTo(0, 0);
        srcOffset.set(0, 0);
    }

    SkASSERT(dstRenderTargetContext);
    SkASSERT(srcProxy.get() == dstRenderTargetContext->asTextureProxy());

    if (scaleFactorX > 1 || scaleFactorY > 1) {
        dstRenderTargetContext = reexpand(context, std::move(dstRenderTargetContext),
                                          localSrcBounds, scaleFactorX, scaleFactorY,
                                          mode, finalDestII, fit);
    }

    SkASSERT(!dstRenderTargetContext || dstRenderTargetContext->origin() == srcProxy->origin());
    return dstRenderTargetContext;
}

}

#endif
