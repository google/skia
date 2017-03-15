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
                                 Gr1DKernelEffect::Direction direction,
                                 int radius,
                                 float sigma,
                                 bool useBounds,
                                 int bounds[2]) {
    GrPaint paint;
    paint.setGammaCorrect(renderTargetContext->isGammaCorrect());

    GrResourceProvider* resourceProvider = renderTargetContext->resourceProvider();

    sk_sp<GrFragmentProcessor> conv(GrGaussianConvolutionFragmentProcessor::Make(
            resourceProvider, std::move(proxy), direction, radius, sigma, useBounds, bounds));
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
                                 const SkIRect* srcBounds) {
    SkMatrix localMatrix = SkMatrix::MakeTrans(-SkIntToScalar(srcOffset.x()),
                                               -SkIntToScalar(srcOffset.y()));
    SkISize size = SkISize::Make(2 * radiusX + 1,  2 * radiusY + 1);
    SkIPoint kernelOffset = SkIPoint::Make(radiusX, radiusY);
    GrPaint paint;
    paint.setGammaCorrect(renderTargetContext->isGammaCorrect());
    SkIRect bounds = srcBounds ? *srcBounds : SkIRect::EmptyIRect();

    GrResourceProvider* resourceProvider = renderTargetContext->resourceProvider();

    sk_sp<GrFragmentProcessor> conv(GrMatrixConvolutionEffect::MakeGaussian(
            resourceProvider, std::move(proxy), bounds, size, 1.0, 0.0, kernelOffset,
            srcBounds ? GrTextureDomain::kDecal_Mode : GrTextureDomain::kIgnore_Mode,
            true, sigmaX, sigmaY));
    paint.addColorFragmentProcessor(std::move(conv));
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
    renderTargetContext->fillRectWithLocalMatrix(clip, std::move(paint), GrAA::kNo, SkMatrix::I(),
                                                 SkRect::Make(dstRect), localMatrix);
}

static void convolve_gaussian(GrRenderTargetContext* renderTargetContext,
                              const GrClip& clip,
                              const SkIRect& srcRect,
                              sk_sp<GrTextureProxy> proxy,
                              Gr1DKernelEffect::Direction direction,
                              int radius,
                              float sigma,
                              const SkIRect* srcBounds,
                              const SkIPoint& srcOffset) {
    int bounds[2] = { 0, 0 };
    SkIRect dstRect = SkIRect::MakeWH(srcRect.width(), srcRect.height());
    if (!srcBounds) {
        convolve_gaussian_1d(renderTargetContext, clip, dstRect, srcOffset,
                             std::move(proxy), direction, radius, sigma, false, bounds);
        return;
    }
    SkIRect midRect = *srcBounds, leftRect, rightRect;
    midRect.offset(srcOffset);
    SkIRect topRect, bottomRect;
    if (direction == Gr1DKernelEffect::kX_Direction) {
        bounds[0] = srcBounds->left();
        bounds[1] = srcBounds->right();
        topRect = SkIRect::MakeLTRB(0, 0, dstRect.right(), midRect.top());
        bottomRect = SkIRect::MakeLTRB(0, midRect.bottom(), dstRect.right(), dstRect.bottom());
        midRect.inset(radius, 0);
        leftRect = SkIRect::MakeLTRB(0, midRect.top(), midRect.left(), midRect.bottom());
        rightRect =
            SkIRect::MakeLTRB(midRect.right(), midRect.top(), dstRect.width(), midRect.bottom());
        dstRect.fTop = midRect.top();
        dstRect.fBottom = midRect.bottom();
    } else {
        bounds[0] = srcBounds->top();
        bounds[1] = srcBounds->bottom();
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
        renderTargetContext->clear(&topRect, 0, false);
    }

    if (!bottomRect.isEmpty()) {
        renderTargetContext->clear(&bottomRect, 0, false);
    }
    if (midRect.isEmpty()) {
        // Blur radius covers srcBounds; use bounds over entire draw
        convolve_gaussian_1d(renderTargetContext, clip, dstRect, srcOffset,
                             std::move(proxy), direction, radius, sigma, true, bounds);
    } else {
        // Draw right and left margins with bounds; middle without.
        convolve_gaussian_1d(renderTargetContext, clip, leftRect, srcOffset,
                             proxy, direction, radius, sigma, true, bounds);
        convolve_gaussian_1d(renderTargetContext, clip, rightRect, srcOffset,
                             proxy, direction, radius, sigma, true, bounds);
        convolve_gaussian_1d(renderTargetContext, clip, midRect, srcOffset,
                             std::move(proxy), direction, radius, sigma, false, bounds);
    }
}

namespace SkGpuBlurUtils {

sk_sp<GrRenderTargetContext> GaussianBlur(GrContext* context,
                                          sk_sp<GrTextureProxy> srcProxy,
                                          sk_sp<SkColorSpace> colorSpace,
                                          const SkIRect& dstBounds,
                                          const SkIRect* srcBounds,
                                          float sigmaX,
                                          float sigmaY,
                                          SkBackingFit fit) {
    SkASSERT(context);

    {
        // Chrome is crashing with proxies when they need to be instantiated.
        // Force an instantiation here (where, in olden days, we used to require a GrTexture)
        // to see if the input is already un-instantiable.
        GrTexture* temp = srcProxy->instantiate(context->resourceProvider());
        if (!temp) {
            return nullptr;
        }
    }

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
    if (srcBounds) {
        srcRect = localSrcBounds = *srcBounds;
        srcRect.offset(srcOffset);
        srcBounds = &localSrcBounds;
    } else {
        srcRect = localDstBounds;
    }

    scale_irect_roundout(&srcRect, 1.0f / scaleFactorX, 1.0f / scaleFactorY);
    scale_irect(&srcRect, scaleFactorX, scaleFactorY);

    // setup new clip
    GrFixedClip clip(localDstBounds);

    const GrPixelConfig config = srcProxy->config();

    SkASSERT(kBGRA_8888_GrPixelConfig  == config || kRGBA_8888_GrPixelConfig  == config ||
             kSRGBA_8888_GrPixelConfig == config || kSBGRA_8888_GrPixelConfig == config ||
             kRGBA_half_GrPixelConfig  == config || kAlpha_8_GrPixelConfig    == config);

    const int width = dstBounds.width();
    const int height = dstBounds.height();

    sk_sp<GrRenderTargetContext> dstRenderTargetContext(context->makeDeferredRenderTargetContext(
        fit, width, height, config, colorSpace, 0, kBottomLeft_GrSurfaceOrigin));
    if (!dstRenderTargetContext) {
        return nullptr;
    }

    // For really small blurs (certainly no wider than 5x5 on desktop gpus) it is faster to just
    // launch a single non separable kernel vs two launches
    if (sigmaX > 0.0f && sigmaY > 0.0f &&
            (2 * radiusX + 1) * (2 * radiusY + 1) <= MAX_KERNEL_SIZE) {
        // We shouldn't be scaling because this is a small size blur
        SkASSERT((1 == scaleFactorX) && (1 == scaleFactorY));

        convolve_gaussian_2d(dstRenderTargetContext.get(), clip, localDstBounds, srcOffset,
                             std::move(srcProxy), radiusX, radiusY, sigmaX, sigmaY, srcBounds);

        return dstRenderTargetContext;
    } 

    sk_sp<GrRenderTargetContext> tmpRenderTargetContext(context->makeDeferredRenderTargetContext(
        fit, width, height, config, colorSpace, 0, kBottomLeft_GrSurfaceOrigin));
    if (!tmpRenderTargetContext) {
        return nullptr;
    }

    sk_sp<GrRenderTargetContext> srcRenderTargetContext;

    SkASSERT(SkIsPow2(scaleFactorX) && SkIsPow2(scaleFactorY));

    for (int i = 1; i < scaleFactorX || i < scaleFactorY; i *= 2) {
        GrPaint paint;
        paint.setGammaCorrect(dstRenderTargetContext->isGammaCorrect());
        SkIRect dstRect(srcRect);
        if (srcBounds && i == 1) {
            SkRect domain = SkRect::Make(*srcBounds);
            domain.inset((i < scaleFactorX) ? SK_ScalarHalf : 0.0f,
                         (i < scaleFactorY) ? SK_ScalarHalf : 0.0f);
            sk_sp<GrFragmentProcessor> fp(GrTextureDomainEffect::Make(
                                                        context->resourceProvider(),
                                                        std::move(srcProxy),
                                                        nullptr,
                                                        SkMatrix::I(),
                                                        domain,
                                                        GrTextureDomain::kDecal_Mode,
                                                        GrSamplerParams::kBilerp_FilterMode));
            paint.addColorFragmentProcessor(std::move(fp));
            srcRect.offset(-srcOffset);
            srcOffset.set(0, 0);
        } else {
            GrSamplerParams params(SkShader::kClamp_TileMode, GrSamplerParams::kBilerp_FilterMode);
            paint.addColorTextureProcessor(context->resourceProvider(), std::move(srcProxy),
                                           nullptr, SkMatrix::I(), params);
        }
        paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
        shrink_irect_by_2(&dstRect, i < scaleFactorX, i < scaleFactorY);

        dstRenderTargetContext->fillRectToRect(clip, std::move(paint), GrAA::kNo, SkMatrix::I(),
                                               SkRect::Make(dstRect), SkRect::Make(srcRect));

        srcRenderTargetContext = dstRenderTargetContext;
        srcRect = dstRect;
        srcProxy = srcRenderTargetContext->asTextureProxyRef();
        if (!srcProxy) {
            return nullptr;
        }
        dstRenderTargetContext.swap(tmpRenderTargetContext);
        localSrcBounds = srcRect;
    }

    srcRect = localDstBounds;
    scale_irect_roundout(&srcRect, 1.0f / scaleFactorX, 1.0f / scaleFactorY);
    if (sigmaX > 0.0f) {
        if (scaleFactorX > 1) {
            SkASSERT(srcRenderTargetContext);

            // Clear out a radius to the right of the srcRect to prevent the
            // X convolution from reading garbage.
            clearRect = SkIRect::MakeXYWH(srcRect.fRight, srcRect.fTop,
                                          radiusX, srcRect.height());
            srcRenderTargetContext->priv().absClear(&clearRect, 0x0);
        }

        convolve_gaussian(dstRenderTargetContext.get(), clip, srcRect,
                          std::move(srcProxy), Gr1DKernelEffect::kX_Direction, radiusX, sigmaX,
                          srcBounds, srcOffset);
        srcRenderTargetContext = dstRenderTargetContext;
        srcProxy = srcRenderTargetContext->asTextureProxyRef();
        if (!srcProxy) {
            return nullptr;
        }
        srcRect.offsetTo(0, 0);
        dstRenderTargetContext.swap(tmpRenderTargetContext);
        localSrcBounds = srcRect;
        srcOffset.set(0, 0);
    }

    if (sigmaY > 0.0f) {
        if (scaleFactorY > 1 || sigmaX > 0.0f) {
            SkASSERT(srcRenderTargetContext);

            // Clear out a radius below the srcRect to prevent the Y
            // convolution from reading garbage.
            clearRect = SkIRect::MakeXYWH(srcRect.fLeft, srcRect.fBottom,
                                          srcRect.width(), radiusY);
            srcRenderTargetContext->priv().absClear(&clearRect, 0x0);
        }

        convolve_gaussian(dstRenderTargetContext.get(), clip, srcRect,
                          std::move(srcProxy), Gr1DKernelEffect::kY_Direction, radiusY, sigmaY,
                          srcBounds, srcOffset);

        srcRenderTargetContext = dstRenderTargetContext;
        srcRect.offsetTo(0, 0);
        dstRenderTargetContext.swap(tmpRenderTargetContext);
    }

    SkASSERT(srcRenderTargetContext);
    srcProxy.reset(nullptr);  // we don't use this from here on out

    if (scaleFactorX > 1 || scaleFactorY > 1) {
        // Clear one pixel to the right and below, to accommodate bilinear upsampling.
        // TODO: it seems like we should actually be clamping here rather than darkening
        // the bottom right edges.
        clearRect = SkIRect::MakeXYWH(srcRect.fLeft, srcRect.fBottom, srcRect.width() + 1, 1);
        srcRenderTargetContext->priv().absClear(&clearRect, 0x0);
        clearRect = SkIRect::MakeXYWH(srcRect.fRight, srcRect.fTop, 1, srcRect.height());
        srcRenderTargetContext->priv().absClear(&clearRect, 0x0);

        GrPaint paint;
        paint.setGammaCorrect(dstRenderTargetContext->isGammaCorrect());
        // FIXME:  this should be mitchell, not bilinear.
        GrSamplerParams params(SkShader::kClamp_TileMode, GrSamplerParams::kBilerp_FilterMode);
        sk_sp<GrTextureProxy> proxy(srcRenderTargetContext->asTextureProxyRef());
        if (!proxy) {
            return nullptr;
        }

        paint.addColorTextureProcessor(context->resourceProvider(), std::move(proxy),
                                       nullptr, SkMatrix::I(), params);
        paint.setPorterDuffXPFactory(SkBlendMode::kSrc);

        SkIRect dstRect(srcRect);
        scale_irect(&dstRect, scaleFactorX, scaleFactorY);

        dstRenderTargetContext->fillRectToRect(clip, std::move(paint), GrAA::kNo, SkMatrix::I(),
                                               SkRect::Make(dstRect), SkRect::Make(srcRect));

        srcRenderTargetContext = dstRenderTargetContext;
        srcRect = dstRect;
        dstRenderTargetContext.swap(tmpRenderTargetContext);
    }

    return srcRenderTargetContext;
}

}

#endif

