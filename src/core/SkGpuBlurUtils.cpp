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
#include "GrContext.h"
#include "GrFixedClip.h"
#include "GrRenderTargetContext.h"
#include "GrRenderTargetContextPriv.h"
#include "effects/GrGaussianConvolutionFragmentProcessor.h"
#include "effects/GrMatrixConvolutionEffect.h"

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
    paint.setGammaCorrect(renderTargetContext->colorSpaceInfo().isGammaCorrect());

    std::unique_ptr<GrFragmentProcessor> conv(GrGaussianConvolutionFragmentProcessor::Make(
            std::move(proxy), direction, radius, sigma, mode, bounds));
    paint.addColorFragmentProcessor(std::move(conv));
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
    SkMatrix localMatrix = SkMatrix::MakeTrans(-SkIntToScalar(srcOffset.x()),
                                               -SkIntToScalar(srcOffset.y()));
    renderTargetContext->fillRectWithLocalMatrix(clip, std::move(paint), GrAA::kNo, SkMatrix::I(),
                                                 SkRect::Make(dstRect), localMatrix);
}

static void convolve_gaussian_2d(GrRenderTargetContext* renderTargetContext,
                                 const GrClip& clip,
                                 const SkIRect& dstRect,
                                 const SkIPoint& srcOffset,
                                 sk_sp<GrTextureProxy> proxy,
                                 int radiusX,
                                 int radiusY,
                                 SkScalar sigmaX,
                                 SkScalar sigmaY,
                                 const SkIRect& srcBounds,
                                 GrTextureDomain::Mode mode) {
    SkMatrix localMatrix = SkMatrix::MakeTrans(-SkIntToScalar(srcOffset.x()),
                                               -SkIntToScalar(srcOffset.y()));
    SkISize size = SkISize::Make(2 * radiusX + 1,  2 * radiusY + 1);
    SkIPoint kernelOffset = SkIPoint::Make(radiusX, radiusY);
    GrPaint paint;
    paint.setGammaCorrect(renderTargetContext->colorSpaceInfo().isGammaCorrect());

    auto conv = GrMatrixConvolutionEffect::MakeGaussian(std::move(proxy), srcBounds, size, 1.0, 0.0,
                                                        kernelOffset, mode, true, sigmaX, sigmaY);
    paint.addColorFragmentProcessor(std::move(conv));
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
    renderTargetContext->fillRectWithLocalMatrix(clip, std::move(paint), GrAA::kNo, SkMatrix::I(),
                                                 SkRect::Make(dstRect), localMatrix);
}

static void convolve_gaussian(GrRenderTargetContext* renderTargetContext,
                              const GrClip& clip,
                              const SkIRect& srcRect,
                              sk_sp<GrTextureProxy> proxy,
                              Direction direction,
                              int radius,
                              float sigma,
                              const SkIRect& srcBounds,
                              const SkIPoint& srcOffset,
                              GrTextureDomain::Mode mode) {
    int bounds[2] = { 0, 0 };
    SkIRect dstRect = SkIRect::MakeWH(srcRect.width(), srcRect.height());
    if (GrTextureDomain::kIgnore_Mode == mode) {
        convolve_gaussian_1d(renderTargetContext, clip, dstRect, srcOffset,
                             std::move(proxy), direction, radius, sigma,
                             GrTextureDomain::kIgnore_Mode, bounds);
        return;
    }
    SkIRect midRect = srcBounds, leftRect, rightRect;
    midRect.offset(srcOffset);
    SkIRect topRect, bottomRect;
    if (Direction::kX == direction) {
        bounds[0] = srcBounds.left();
        bounds[1] = srcBounds.right();
        topRect = SkIRect::MakeLTRB(0, 0, dstRect.right(), midRect.top());
        bottomRect = SkIRect::MakeLTRB(0, midRect.bottom(), dstRect.right(), dstRect.bottom());
        midRect.inset(radius, 0);
        leftRect = SkIRect::MakeLTRB(0, midRect.top(), midRect.left(), midRect.bottom());
        rightRect =
            SkIRect::MakeLTRB(midRect.right(), midRect.top(), dstRect.width(), midRect.bottom());
        dstRect.fTop = midRect.top();
        dstRect.fBottom = midRect.bottom();
    } else {
        bounds[0] = srcBounds.top();
        bounds[1] = srcBounds.bottom();
        topRect = SkIRect::MakeLTRB(0, 0, midRect.left(), dstRect.bottom());
        bottomRect = SkIRect::MakeLTRB(midRect.right(), 0, dstRect.right(), dstRect.bottom());
        midRect.inset(0, radius);
        leftRect = SkIRect::MakeLTRB(midRect.left(), 0, midRect.right(), midRect.top());
        rightRect =
            SkIRect::MakeLTRB(midRect.left(), midRect.bottom(), midRect.right(), dstRect.height());
        dstRect.fLeft = midRect.left();
        dstRect.fRight = midRect.right();
    }
    if (!topRect.isEmpty()) {
        renderTargetContext->clear(&topRect, 0, GrRenderTargetContext::CanClearFullscreen::kNo);
    }

    if (!bottomRect.isEmpty()) {
        renderTargetContext->clear(&bottomRect, 0, GrRenderTargetContext::CanClearFullscreen::kNo);
    }

    if (midRect.isEmpty()) {
        // Blur radius covers srcBounds; use bounds over entire draw
        convolve_gaussian_1d(renderTargetContext, clip, dstRect, srcOffset,
                             std::move(proxy), direction, radius, sigma, mode, bounds);
    } else {
        // Draw right and left margins with bounds; middle without.
        convolve_gaussian_1d(renderTargetContext, clip, leftRect, srcOffset,
                             proxy, direction, radius, sigma, mode, bounds);
        convolve_gaussian_1d(renderTargetContext, clip, rightRect, srcOffset,
                             proxy, direction, radius, sigma, mode, bounds);
        convolve_gaussian_1d(renderTargetContext, clip, midRect, srcOffset,
                             std::move(proxy), direction, radius, sigma,
                             GrTextureDomain::kIgnore_Mode, bounds);
    }
}

namespace SkGpuBlurUtils {

sk_sp<GrRenderTargetContext> GaussianBlur(GrContext* context,
                                          sk_sp<GrTextureProxy> srcProxy,
                                          sk_sp<SkColorSpace> colorSpace,
                                          const SkIRect& dstBounds,
                                          const SkIRect& srcBounds,
                                          float sigmaX,
                                          float sigmaY,
                                          GrTextureDomain::Mode mode,
                                          SkBackingFit fit) {
    SkASSERT(context);
    SkIRect clearRect;
    int scaleFactorX, radiusX;
    int scaleFactorY, radiusY;
    int maxTextureSize = context->caps()->maxTextureSize();
    sigmaX = adjust_sigma(sigmaX, maxTextureSize, &scaleFactorX, &radiusX);
    sigmaY = adjust_sigma(sigmaY, maxTextureSize, &scaleFactorY, &radiusY);
    SkASSERT(sigmaX || sigmaY);

    SkIPoint srcOffset = SkIPoint::Make(-dstBounds.x(), -dstBounds.y());
    SkIRect localDstBounds = SkIRect::MakeWH(dstBounds.width(), dstBounds.height());
    SkIRect localSrcBounds;
    SkIRect srcRect;
    if (GrTextureDomain::kIgnore_Mode == mode) {
        srcRect = localDstBounds;
    } else {
        srcRect = localSrcBounds = srcBounds;
        srcRect.offset(srcOffset);
    }

    scale_irect_roundout(&srcRect, 1.0f / scaleFactorX, 1.0f / scaleFactorY);
    scale_irect(&srcRect, scaleFactorX, scaleFactorY);

    // setup new clip
    GrFixedClip clip(localDstBounds);

    GrPixelConfig config = srcProxy->config();

    if (GrPixelConfigIsSRGB(config) && !colorSpace) {
        // If the context doesn't have sRGB write control, and we make an sRGB RTC, we won't be
        // able to suppress the linear -> sRGB conversion out of the shader. Not all GL drivers
        // have that feature, and Vulkan is missing it entirely. To keep things simple, switch to
        // a non-sRGB destination, to ensure correct blurring behavior.
        config = kRGBA_8888_GrPixelConfig;
    }

    SkASSERT(kBGRA_8888_GrPixelConfig == config || kRGBA_8888_GrPixelConfig == config ||
             kRGBA_4444_GrPixelConfig == config || kRGB_565_GrPixelConfig == config ||
             kSRGBA_8888_GrPixelConfig == config || kSBGRA_8888_GrPixelConfig == config ||
             kRGBA_half_GrPixelConfig == config || kAlpha_8_GrPixelConfig == config);

    const int width = dstBounds.width();
    const int height = dstBounds.height();

    sk_sp<GrRenderTargetContext> dstRenderTargetContext;

    // For really small blurs (certainly no wider than 5x5 on desktop gpus) it is faster to just
    // launch a single non separable kernel vs two launches
    if (sigmaX > 0.0f && sigmaY > 0.0f &&
            (2 * radiusX + 1) * (2 * radiusY + 1) <= MAX_KERNEL_SIZE) {
        // We shouldn't be scaling because this is a small size blur
        SkASSERT((1 == scaleFactorX) && (1 == scaleFactorY));

        dstRenderTargetContext = context->makeDeferredRenderTargetContext(fit, width, height,
                                                                          config, colorSpace);
        if (!dstRenderTargetContext) {
            return nullptr;
        }

        convolve_gaussian_2d(dstRenderTargetContext.get(),
                             clip, localDstBounds, srcOffset, std::move(srcProxy),
                             radiusX, radiusY, sigmaX, sigmaY, srcBounds, mode);

        return dstRenderTargetContext;
    }

    SkASSERT(SkIsPow2(scaleFactorX) && SkIsPow2(scaleFactorY));

    // GrTextureDomainEffect does not support kRepeat_Mode with GrSamplerState::Filter.
    GrTextureDomain::Mode modeForScaling =
            GrTextureDomain::kRepeat_Mode == mode ? GrTextureDomain::kDecal_Mode : mode;
    for (int i = 1; i < scaleFactorX || i < scaleFactorY; i *= 2) {
        SkIRect dstRect(srcRect);
        shrink_irect_by_2(&dstRect, i < scaleFactorX, i < scaleFactorY);

        dstRenderTargetContext = context->makeDeferredRenderTargetContext(
                                                                fit,
                                                                SkTMin(dstRect.fRight, width),
                                                                SkTMin(dstRect.fBottom, height),
                                                                config, colorSpace);
        if (!dstRenderTargetContext) {
            return nullptr;
        }

        GrPaint paint;
        paint.setGammaCorrect(dstRenderTargetContext->colorSpaceInfo().isGammaCorrect());

        if (GrTextureDomain::kIgnore_Mode != mode && i == 1) {
            SkRect domain = SkRect::Make(localSrcBounds);
            domain.inset((i < scaleFactorX) ? SK_ScalarHalf : 0.0f,
                         (i < scaleFactorY) ? SK_ScalarHalf : 0.0f);
            auto fp = GrTextureDomainEffect::Make(std::move(srcProxy),
                                                  SkMatrix::I(),
                                                  domain,
                                                  modeForScaling,
                                                  GrSamplerState::Filter::kBilerp);
            paint.addColorFragmentProcessor(std::move(fp));
            srcRect.offset(-srcOffset);
            srcOffset.set(0, 0);
        } else {
            paint.addColorTextureProcessor(std::move(srcProxy), SkMatrix::I(),
                                           GrSamplerState::ClampBilerp());
        }
        paint.setPorterDuffXPFactory(SkBlendMode::kSrc);

        dstRenderTargetContext->fillRectToRect(clip, std::move(paint), GrAA::kNo, SkMatrix::I(),
                                               SkRect::Make(dstRect), SkRect::Make(srcRect));

        srcProxy = dstRenderTargetContext->asTextureProxyRef();
        if (!srcProxy) {
            return nullptr;
        }
        srcRect = dstRect;
        localSrcBounds = srcRect;
    }

    srcRect = localDstBounds;
    scale_irect_roundout(&srcRect, 1.0f / scaleFactorX, 1.0f / scaleFactorY);
    if (sigmaX > 0.0f) {
        if (scaleFactorX > 1) {
            SkASSERT(dstRenderTargetContext);

            // Clear out a radius to the right of the srcRect to prevent the
            // X convolution from reading garbage.
            clearRect = SkIRect::MakeXYWH(srcRect.fRight, srcRect.fTop,
                                          radiusX, srcRect.height());
            dstRenderTargetContext->priv().absClear(&clearRect, 0x0);
        }

        SkASSERT(srcRect.width() <= width && srcRect.height() <= height);
        dstRenderTargetContext = context->makeDeferredRenderTargetContext(fit, srcRect.width(),
                                                                          srcRect.height(),
                                                                          config, colorSpace);
        if (!dstRenderTargetContext) {
            return nullptr;
        }

        convolve_gaussian(dstRenderTargetContext.get(), clip, srcRect, std::move(srcProxy),
                          Direction::kX, radiusX, sigmaX, localSrcBounds, srcOffset, mode);

        srcProxy = dstRenderTargetContext->asTextureProxyRef();
        if (!srcProxy) {
            return nullptr;
        }
        srcRect.offsetTo(0, 0);
        localSrcBounds = srcRect;
        if (GrTextureDomain::kClamp_Mode == mode) {
            // We need to adjust bounds because we only fill part of the srcRect in x-pass.
            localSrcBounds.inset(0, radiusY);
        }
        srcOffset.set(0, 0);
    }

    if (sigmaY > 0.0f) {
        if (scaleFactorY > 1 || sigmaX > 0.0f) {
            SkASSERT(dstRenderTargetContext);

            // Clear out a radius below the srcRect to prevent the Y
            // convolution from reading garbage.
            clearRect = SkIRect::MakeXYWH(srcRect.fLeft, srcRect.fBottom,
                                          srcRect.width(), radiusY);
            dstRenderTargetContext->priv().absClear(&clearRect, 0x0);
        }

        SkASSERT(srcRect.width() <= width && srcRect.height() <= height);
        dstRenderTargetContext = context->makeDeferredRenderTargetContext(fit, srcRect.width(),
                                                                          srcRect.height(),
                                                                          config, colorSpace);
        if (!dstRenderTargetContext) {
            return nullptr;
        }

        convolve_gaussian(dstRenderTargetContext.get(), clip, srcRect, std::move(srcProxy),
                          Direction::kY, radiusY, sigmaY, localSrcBounds, srcOffset, mode);

        srcProxy = dstRenderTargetContext->asTextureProxyRef();
        if (!srcProxy) {
            return nullptr;
        }
        srcRect.offsetTo(0, 0);
    }

    SkASSERT(dstRenderTargetContext);
    SkASSERT(srcProxy.get() == dstRenderTargetContext->asTextureProxy());

    if (scaleFactorX > 1 || scaleFactorY > 1) {
        // Clear one pixel to the right and below, to accommodate bilinear upsampling.
        // TODO: it seems like we should actually be clamping here rather than darkening
        // the bottom right edges.
        clearRect = SkIRect::MakeXYWH(srcRect.fLeft, srcRect.fBottom, srcRect.width() + 1, 1);
        dstRenderTargetContext->priv().absClear(&clearRect, 0x0);
        clearRect = SkIRect::MakeXYWH(srcRect.fRight, srcRect.fTop, 1, srcRect.height());
        dstRenderTargetContext->priv().absClear(&clearRect, 0x0);

        SkIRect dstRect(srcRect);
        scale_irect(&dstRect, scaleFactorX, scaleFactorY);

        dstRenderTargetContext = context->makeDeferredRenderTargetContext(
                                                                fit, SkTMin(dstRect.width(), width),
                                                                SkTMin(dstRect.height(), height),
                                                                config, colorSpace);
        if (!dstRenderTargetContext) {
            return nullptr;
        }

        GrPaint paint;
        paint.setGammaCorrect(dstRenderTargetContext->colorSpaceInfo().isGammaCorrect());

        if (GrTextureDomain::kIgnore_Mode != mode) {
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

        dstRenderTargetContext->fillRectToRect(clip, std::move(paint), GrAA::kNo, SkMatrix::I(),
                                               SkRect::Make(dstRect), SkRect::Make(srcRect));

        srcProxy = dstRenderTargetContext->asTextureProxyRef();
        if (!srcProxy) {
            return nullptr;
        }
        srcRect = dstRect;
    }

    return dstRenderTargetContext;
}

}

#endif

