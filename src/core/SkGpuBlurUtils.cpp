/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkGpuBlurUtils.h"

#include "include/core/SkRect.h"

#if SK_SUPPORT_GPU
#include "include/private/GrRecordingContext.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrFixedClip.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrRenderTargetContextPriv.h"
#include "src/gpu/effects/GrGaussianConvolutionFragmentProcessor.h"
#include "src/gpu/effects/GrMatrixConvolutionEffect.h"

#include "src/gpu/SkGr.h"

#define MAX_BLUR_SIGMA 4.0f

using Direction = GrGaussianConvolutionFragmentProcessor::Direction;

static int sigma_radius(float sigma) {
    SkASSERT(sigma >= 0);
    int radius = static_cast<int>(ceilf(sigma * 3.0f));
    SkASSERT(radius <= GrGaussianConvolutionFragmentProcessor::kMaxKernelRadius);
    return radius;
}

/**
 * Draws 'rtcRect' into 'renderTargetContext' evaluating a 1D Gaussian over 'srcView'. The src rect
 * is 'rtcRect' offset by 'rtcToSrcOffset'. 'mode' and 'bounds' are applied to the src coords.
 */
static void convolve_gaussian_1d(GrRenderTargetContext* renderTargetContext,
                                 GrSurfaceProxyView srcView,
                                 SkIVector rtcToSrcOffset,
                                 const SkIRect& rtcRect,
                                 SkAlphaType srcAlphaType,
                                 Direction direction,
                                 int radius,
                                 float sigma,
                                 SkTileMode mode,
                                 int bounds[2]) {
    GrPaint paint;
    auto wm = SkTileModeToWrapMode(mode);
    int realBounds[2];
    if (bounds) {
        realBounds[0] = bounds[0]; realBounds[1] = bounds[1];
    } else {
        auto proxy = srcView.proxy();
        realBounds[0] = 0;
        realBounds[1] = direction == Direction::kX ? proxy->width() : proxy->height();
    }
    std::unique_ptr<GrFragmentProcessor> conv(GrGaussianConvolutionFragmentProcessor::Make(
            std::move(srcView), srcAlphaType, direction, radius, sigma, wm, realBounds,
            *renderTargetContext->caps()));
    paint.addColorFragmentProcessor(std::move(conv));
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
    auto srcRect = SkRect::Make(rtcRect.makeOffset(rtcToSrcOffset));
    renderTargetContext->fillRectToRect(GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(),
                                        SkRect::Make(rtcRect), srcRect);
}

static std::unique_ptr<GrRenderTargetContext> convolve_gaussian_2d(GrRecordingContext* context,
                                                                   GrSurfaceProxyView srcView,
                                                                   GrColorType srcColorType,
                                                                   const SkIRect& srcBounds,
                                                                   const SkIRect& dstBounds,
                                                                   int radiusX,
                                                                   int radiusY,
                                                                   SkScalar sigmaX,
                                                                   SkScalar sigmaY,
                                                                   SkTileMode mode,
                                                                   sk_sp<SkColorSpace> finalCS,
                                                                   SkBackingFit dstFit) {
    auto renderTargetContext = GrRenderTargetContext::Make(
            context, srcColorType, std::move(finalCS), dstFit, dstBounds.size(), 1,
            GrMipMapped::kNo, srcView.proxy()->isProtected(), srcView.origin());
    if (!renderTargetContext) {
        return nullptr;
    }

    SkISize size = SkISize::Make(2 * radiusX + 1,  2 * radiusY + 1);
    SkIPoint kernelOffset = SkIPoint::Make(radiusX, radiusY);
    GrPaint paint;
    auto wm = SkTileModeToWrapMode(mode);
    auto conv = GrMatrixConvolutionEffect::MakeGaussian(std::move(srcView), srcBounds, size, 1.0,
                                                        0.0, kernelOffset, wm, true, sigmaX, sigmaY,
                                                        *renderTargetContext->caps());
    paint.addColorFragmentProcessor(std::move(conv));
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);

    // 'dstBounds' is actually in 'srcView' proxy space. It represents the blurred area from src
    // space that we want to capture in the new RTC at {0, 0}. Hence, we use its size as the rect to
    // draw and it directly as the local rect.
    renderTargetContext->fillRectToRect(GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(),
                                        SkRect::Make(dstBounds.size()), SkRect::Make(dstBounds));

    return renderTargetContext;
}

static std::unique_ptr<GrRenderTargetContext> convolve_gaussian(GrRecordingContext* context,
                                                                GrSurfaceProxyView srcView,
                                                                GrColorType srcColorType,
                                                                SkAlphaType srcAlphaType,
                                                                SkIRect* contentRect,
                                                                SkIRect dstBounds,
                                                                Direction direction,
                                                                int radius,
                                                                float sigma,
                                                                SkTileMode mode,
                                                                sk_sp<SkColorSpace> finalCS,
                                                                SkBackingFit fit) {
    // Logically we're creating an infinite blur of 'contentRect' of 'srcView' with 'mode' tiling
    // and then capturing the 'dstBounds' portion in a new RTC where the top left of 'dstBounds' is
    // at {0, 0} in the new RTC.
    auto dstRenderTargetContext = GrRenderTargetContext::Make(
            context, srcColorType, std::move(finalCS), fit, dstBounds.size(), 1, GrMipMapped::kNo,
            srcView.proxy()->isProtected(), srcView.origin());
    if (!dstRenderTargetContext) {
        return nullptr;
    }

    // This represents the translation from 'dstRenderTargetContext' coords to 'srcView' coords.
    auto rtcToSrcOffset = dstBounds.topLeft();

    if (SkTileMode::kClamp == mode &&
        contentRect->contains(SkIRect::MakeSize(srcView.proxy()->backingStoreDimensions()))) {
        auto dstRect = SkIRect::MakeSize(dstBounds.size());
        convolve_gaussian_1d(dstRenderTargetContext.get(), std::move(srcView), rtcToSrcOffset,
                             dstRect, srcAlphaType, direction, radius, sigma, SkTileMode::kClamp,
                             nullptr);
        *contentRect = dstRect;
        return dstRenderTargetContext;
    }

    // 'left' and 'right' are the sub rects of 'contentTect' where 'mode' must be enforced.
    // 'mid' is the area where we can ignore the mode because the kernel does not reach to the
    // edge of 'contentRect'. The names are derived from the Direction::kX case.
    // TODO: When mode is kMirror or kRepeat it makes more sense to think of 'contentRect'
    // as a tile and figure out the collection of mid/left/right rects that cover 'dstBounds'.
    // Also if 'mid' is small and 'left' or 'right' is non-empty we should probably issue one
    // draw that implements the mode in the shader rather than break it up in this fashion.
    SkIRect mid, left, right;
    // 'top' and 'bottom' are areas of 'dstBounds' that are entirely above/below
    // 'contentRect'. These are areas that we can simply clear in the dst. If 'contentRect'
    // straddles the top edge of 'dstBounds' then 'top' will be inverted and we will skip
    // the clear. Similar for 'bottom'. The positional/directional labels above refer to the
    // Direction::kX case and one should think of these as 'left' and 'right' for Direction::kY.
    SkIRect top, bottom;
    int bounds[2];
    if (Direction::kX == direction) {
        bounds[0] = contentRect->left();
        bounds[1] = contentRect->right();

        top    = {dstBounds.left(), dstBounds.top()      , dstBounds.right(), contentRect->top()};
        bottom = {dstBounds.left(), contentRect->bottom(), dstBounds.right(), dstBounds.bottom()};

        // Inset for sub-rect of 'contentRect' where the x-dir kernel doesn't reach the edges.
        // TODO: Consider clipping mid/left/right to dstBounds to increase likelihood of doing
        // fewer draws below.
        mid = contentRect->makeInset(radius, 0);

        left  = {dstBounds.left(), mid.top(), mid.left()       , mid.bottom()};
        right = {mid.right(),      mid.top(), dstBounds.right(), mid.bottom()};

        // The new 'contentRect' when we're done will be the area between the clears.
        *contentRect = {dstBounds.left(), top.bottom(), dstBounds.right(), bottom.top()};
    } else {
        // This is the same as the x direction code if you turn your head 90 degrees CCW. Swap x and
        // y and swap top/bottom with left/right.
        bounds[0] = contentRect->top();
        bounds[1] = contentRect->bottom();

        top    = {dstBounds.left(),     dstBounds.top() , contentRect->left(), dstBounds.bottom()};
        bottom = {contentRect->right(), dstBounds.top() , dstBounds.right()  , dstBounds.bottom()};

        mid = contentRect->makeInset(0, radius);

        left  = {mid.left(), dstBounds.top(), mid.right(), mid.top()         };
        right = {mid.left(), mid.bottom()   , mid.right(), dstBounds.bottom()};

        *contentRect = {top.right(), dstBounds.top(), bottom.left(), dstBounds.bottom()};
    }
    // Move all the rects from 'srcView' coord system to 'dstRenderTargetContext' coord system.
    mid   .offset(-rtcToSrcOffset);
    top   .offset(-rtcToSrcOffset);
    bottom.offset(-rtcToSrcOffset);
    left  .offset(-rtcToSrcOffset);
    right .offset(-rtcToSrcOffset);

    contentRect->offset(-rtcToSrcOffset);

    if (!top.isEmpty()) {
        dstRenderTargetContext->clear(&top, SK_PMColor4fTRANSPARENT,
                                      GrRenderTargetContext::CanClearFullscreen::kYes);
    }

    if (!bottom.isEmpty()) {
        dstRenderTargetContext->clear(&bottom, SK_PMColor4fTRANSPARENT,
                                      GrRenderTargetContext::CanClearFullscreen::kYes);
    }

    if (mid.isEmpty()) {
        convolve_gaussian_1d(dstRenderTargetContext.get(), std::move(srcView), rtcToSrcOffset,
                             *contentRect, srcAlphaType, direction, radius, sigma, mode, bounds);
    } else {
        // Draw right and left margins with bounds; middle without.
        convolve_gaussian_1d(dstRenderTargetContext.get(), srcView, rtcToSrcOffset, left,
                             srcAlphaType, direction, radius, sigma, mode, bounds);
        convolve_gaussian_1d(dstRenderTargetContext.get(), srcView, rtcToSrcOffset, right,
                             srcAlphaType, direction, radius, sigma, mode, bounds);
        convolve_gaussian_1d(dstRenderTargetContext.get(), std::move(srcView), rtcToSrcOffset, mid,
                             srcAlphaType, direction, radius, sigma, SkTileMode::kClamp, nullptr);
    }

    return dstRenderTargetContext;
}

// Expand the contents of 'srcRenderTargetContext' to fit in 'dstII'. At this point, we are
// expanding an intermediate image, so there's no need to account for a proxy offset from the
// original input.
static std::unique_ptr<GrRenderTargetContext> reexpand(GrRecordingContext* context,
                                                       std::unique_ptr<GrRenderTargetContext> src,
                                                       const SkRect& srcBounds,
                                                       SkISize dstSize,
                                                       sk_sp<SkColorSpace> colorSpace,
                                                       SkBackingFit fit) {
    GrSurfaceProxyView srcView = src->readSurfaceView();
    if (!srcView.asTextureProxy()) {
        return nullptr;
    }

    GrColorType srcColorType = src->colorInfo().colorType();
    SkAlphaType srcAlphaType = src->colorInfo().alphaType();

    src.reset(); // no longer needed

    auto dstRenderTargetContext = GrRenderTargetContext::Make(
            context, srcColorType, std::move(colorSpace), fit, dstSize, 1, GrMipMapped::kNo,
            srcView.proxy()->isProtected(), srcView.origin());
    if (!dstRenderTargetContext) {
        return nullptr;
    }

    GrPaint paint;
    auto fp = GrTextureEffect::Make(std::move(srcView), srcAlphaType, SkMatrix::I(),
                                    GrSamplerState::Filter::kBilerp);
    paint.addColorFragmentProcessor(std::move(fp));
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);

    dstRenderTargetContext->fillRectToRect(GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(),
                                           SkRect::Make(dstSize), srcBounds);

    return dstRenderTargetContext;
}

static std::unique_ptr<GrRenderTargetContext> two_pass_gaussian(GrRecordingContext* context,
                                                                GrSurfaceProxyView srcView,
                                                                GrColorType srcColorType,
                                                                SkAlphaType srcAlphaType,
                                                                sk_sp<SkColorSpace> colorSpace,
                                                                SkIRect srcBounds,
                                                                SkIRect dstBounds,
                                                                float sigmaX,
                                                                float sigmaY,
                                                                int radiusX,
                                                                int radiusY,
                                                                SkTileMode mode,
                                                                SkBackingFit fit) {
    std::unique_ptr<GrRenderTargetContext> dstRenderTargetContext;
    if (sigmaX > 0.0f) {
        SkBackingFit xFit = sigmaY > 0 ? SkBackingFit::kApprox : fit;
        dstRenderTargetContext = convolve_gaussian(
                context, std::move(srcView), srcColorType, srcAlphaType, &srcBounds, dstBounds,
                Direction::kX, radiusX, sigmaX, mode, colorSpace, xFit);
        if (!dstRenderTargetContext) {
            return nullptr;
        }
        srcView = dstRenderTargetContext->readSurfaceView();
        dstBounds = SkIRect::MakeSize(dstBounds.size());
    }

    if (sigmaY == 0.0f) {
        return dstRenderTargetContext;
    }

    return convolve_gaussian(context, std::move(srcView), srcColorType, srcAlphaType, &srcBounds,
                             dstBounds, Direction::kY, radiusY, sigmaY, mode, colorSpace, fit);
}

namespace SkGpuBlurUtils {

std::unique_ptr<GrRenderTargetContext> GaussianBlur(GrRecordingContext* context,
                                                    GrSurfaceProxyView srcView,
                                                    GrColorType srcColorType,
                                                    SkAlphaType srcAlphaType,
                                                    sk_sp<SkColorSpace> colorSpace,
                                                    const SkIRect& dstBounds,
                                                    const SkIRect& srcBounds,
                                                    float sigmaX,
                                                    float sigmaY,
                                                    SkTileMode mode,
                                                    SkBackingFit fit) {
    SkASSERT(context);
    TRACE_EVENT2("skia.gpu", "GaussianBlur", "sigmaX", sigmaX, "sigmaY", sigmaY);

    if (!srcView.asTextureProxy()) {
        return nullptr;
    }

    if (sigmaX <= MAX_BLUR_SIGMA && sigmaY <= MAX_BLUR_SIGMA) {
        int radiusX = sigma_radius(sigmaX);
        int radiusY = sigma_radius(sigmaY);
        // For really small blurs (certainly no wider than 5x5 on desktop GPUs) it is faster to just
        // launch a single non separable kernel vs two launches.
        if (sigmaX > 0 && sigmaY > 0 && (2 * radiusX + 1) * (2 * radiusY + 1) <= MAX_KERNEL_SIZE) {
            // Apply the proxy offset to src bounds and offset directly
            return convolve_gaussian_2d(context, std::move(srcView), srcColorType, srcBounds,
                                        dstBounds, radiusX, radiusY, sigmaX, sigmaY, mode,
                                        colorSpace, fit);
        }
        return two_pass_gaussian(context, std::move(srcView), srcColorType, srcAlphaType,
                                 std::move(colorSpace), srcBounds, dstBounds, sigmaX, sigmaY,
                                 radiusX, radiusY, mode, fit);
    }

    float scaleX = sigmaX > MAX_BLUR_SIGMA ? MAX_BLUR_SIGMA/sigmaX : 1.f;
    float scaleY = sigmaY > MAX_BLUR_SIGMA ? MAX_BLUR_SIGMA/sigmaY : 1.f;
    // We round down here so that when we recalculate sigmas we know they will be below
    // MAX_BLUR_SIGMA.
    SkISize rescaledSize = {sk_float_floor2int(srcBounds.width() *scaleX),
                            sk_float_floor2int(srcBounds.height()*scaleY)};
    scaleX = (float)rescaledSize.width() /srcBounds.width();
    scaleY = (float)rescaledSize.height()/srcBounds.height();
    sigmaX *= scaleX;
    sigmaY *= scaleY;

    auto srcCtx = GrSurfaceContext::Make(context, srcView, srcColorType, srcAlphaType, colorSpace);
    GrImageInfo rescaledII(srcColorType, srcAlphaType, colorSpace, rescaledSize);
    srcCtx = srcCtx->rescale(rescaledII, srcBounds, SkSurface::RescaleGamma::kSrc,
                             kLow_SkFilterQuality);
    srcView = srcCtx->readSurfaceView();
    // Drop the context so we don't hold the proxy longer than necessary.
    srcCtx.reset();

    // Compute the dst bounds in the scaled down space. First move the origin to be at the top
    // left of the source, since when we scaled down the source we put the top left of the srcBounds
    // content at the origin of the smaller texture.
    SkRect scaledDstBounds = SkRect::Make(dstBounds.makeOffset(-srcBounds.topLeft()));
    scaledDstBounds.fLeft   *= scaleX;
    scaledDstBounds.fTop    *= scaleY;
    scaledDstBounds.fRight  *= scaleX;
    scaledDstBounds.fBottom *= scaleY;
    auto scaledDstBoundsI = scaledDstBounds.roundOut();

    auto rtc = GaussianBlur(context, std::move(srcView), srcColorType, srcAlphaType,
                            std::move(colorSpace), scaledDstBoundsI,
                            SkIRect::MakeSize(rescaledSize), sigmaX, sigmaY, mode, fit);
    if (!rtc) {
        return nullptr;
    }
    // We rounded out the integer scaled dst bounds. Select the fractional dst bounds from the
    // integer dimension blurred result.
    scaledDstBounds.offset(-scaledDstBoundsI.left(), -scaledDstBoundsI.top());
    return reexpand(context, std::move(rtc), scaledDstBounds, dstBounds.size(),
                    std::move(colorSpace), fit);
}

}

#endif
