/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkGpuBlurUtils.h"

#include "include/core/SkRect.h"

#if SK_SUPPORT_GPU
#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/GrCaps.h"
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
    return static_cast<int>(ceilf(sigma * 3.0f));
}

/**
 * Draws 'rtcRect' into 'renderTargetContext' evaluating a 1D Gaussian over 'srcView'. The src rect
 * is 'rtcRect' offset by 'rtcToSrcOffset'. 'mode' and 'bounds' are applied to the src coords.
 */
static void convolve_gaussian_1d(GrRenderTargetContext* renderTargetContext,
                                 GrSurfaceProxyView srcView,
                                 const SkIRect srcSubset,
                                 SkIVector rtcToSrcOffset,
                                 const SkIRect& rtcRect,
                                 SkAlphaType srcAlphaType,
                                 Direction direction,
                                 int radius,
                                 float sigma,
                                 SkTileMode mode) {
    GrPaint paint;
    auto wm = SkTileModeToWrapMode(mode);
    auto srcRect = rtcRect.makeOffset(rtcToSrcOffset);
    std::unique_ptr<GrFragmentProcessor> conv(GrGaussianConvolutionFragmentProcessor::Make(
            std::move(srcView), srcAlphaType, direction, radius, sigma, wm, srcSubset, &srcRect,
            *renderTargetContext->caps()));
    paint.setColorFragmentProcessor(std::move(conv));
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
    renderTargetContext->fillRectToRect(nullptr, std::move(paint), GrAA::kNo, SkMatrix::I(),
                                        SkRect::Make(rtcRect), SkRect::Make(srcRect));
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
            GrMipmapped::kNo, srcView.proxy()->isProtected(), srcView.origin());
    if (!renderTargetContext) {
        return nullptr;
    }

    SkISize size = SkISize::Make(2 * radiusX + 1,  2 * radiusY + 1);
    SkIPoint kernelOffset = SkIPoint::Make(radiusX, radiusY);
    GrPaint paint;
    auto wm = SkTileModeToWrapMode(mode);
    auto conv = GrMatrixConvolutionEffect::MakeGaussian(context, std::move(srcView), srcBounds,
                                                        size, 1.0, 0.0, kernelOffset, wm, true,
                                                        sigmaX, sigmaY,
                                                        *renderTargetContext->caps());
    paint.setColorFragmentProcessor(std::move(conv));
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);

    // 'dstBounds' is actually in 'srcView' proxy space. It represents the blurred area from src
    // space that we want to capture in the new RTC at {0, 0}. Hence, we use its size as the rect to
    // draw and it directly as the local rect.
    renderTargetContext->fillRectToRect(nullptr, std::move(paint), GrAA::kNo, SkMatrix::I(),
                                        SkRect::Make(dstBounds.size()), SkRect::Make(dstBounds));

    return renderTargetContext;
}

static std::unique_ptr<GrRenderTargetContext> convolve_gaussian(GrRecordingContext* context,
                                                                GrSurfaceProxyView srcView,
                                                                GrColorType srcColorType,
                                                                SkAlphaType srcAlphaType,
                                                                SkIRect srcBounds,
                                                                SkIRect dstBounds,
                                                                Direction direction,
                                                                int radius,
                                                                float sigma,
                                                                SkTileMode mode,
                                                                sk_sp<SkColorSpace> finalCS,
                                                                SkBackingFit fit) {
    // Logically we're creating an infinite blur of 'srcBounds' of 'srcView' with 'mode' tiling
    // and then capturing the 'dstBounds' portion in a new RTC where the top left of 'dstBounds' is
    // at {0, 0} in the new RTC.
    auto dstRenderTargetContext = GrRenderTargetContext::Make(
            context, srcColorType, std::move(finalCS), fit, dstBounds.size(), 1, GrMipmapped::kNo,
            srcView.proxy()->isProtected(), srcView.origin());
    if (!dstRenderTargetContext) {
        return nullptr;
    }
    // This represents the translation from 'dstRenderTargetContext' coords to 'srcView' coords.
    auto rtcToSrcOffset = dstBounds.topLeft();

    auto srcBackingBounds = SkIRect::MakeSize(srcView.proxy()->backingStoreDimensions());
    // We've implemented splitting the dst bounds up into areas that do and do not need to
    // use shader based tiling but only for some modes...
    bool canSplit = mode == SkTileMode::kDecal || mode == SkTileMode::kClamp;
    // ...but it's not worth doing the splitting if we'll get HW tiling instead of shader tiling.
    bool canHWTile =
            srcBounds.contains(srcBackingBounds) &&
            !(mode == SkTileMode::kDecal && !context->priv().caps()->clampToBorderSupport());
    if (!canSplit || canHWTile) {
        auto dstRect = SkIRect::MakeSize(dstBounds.size());
        convolve_gaussian_1d(dstRenderTargetContext.get(), std::move(srcView), srcBounds,
                             rtcToSrcOffset, dstRect, srcAlphaType, direction, radius, sigma, mode);
        return dstRenderTargetContext;
    }

    // 'left' and 'right' are the sub rects of 'srcBounds' where 'mode' must be enforced.
    // 'mid' is the area where we can ignore the mode because the kernel does not reach to the
    // edge of 'srcBounds'.
    SkIRect mid, left, right;
    // 'top' and 'bottom' are areas of 'dstBounds' that are entirely above/below 'srcBounds'.
    // These are areas that we can simply clear in the dst in kDecal mode. If 'srcBounds'
    // straddles the top edge of 'dstBounds' then 'top' will be inverted and we will skip
    // processing for the rect. Similar for 'bottom'. The positional/directional labels above refer
    // to the Direction::kX case and one should think of these as 'left' and 'right' for
    // Direction::kY.
    SkIRect top, bottom;
    if (Direction::kX == direction) {
        top    = {dstBounds.left(), dstBounds.top()   , dstBounds.right(), srcBounds.top()   };
        bottom = {dstBounds.left(), srcBounds.bottom(), dstBounds.right(), dstBounds.bottom()};

        // Inset for sub-rect of 'srcBounds' where the x-dir kernel doesn't reach the edges, clipped
        // vertically to dstBounds.
        int midA = std::max(srcBounds.top()   , dstBounds.top()   );
        int midB = std::min(srcBounds.bottom(), dstBounds.bottom());
        mid = {srcBounds.left() + radius, midA, srcBounds.right() - radius, midB};
        if (mid.isEmpty()) {
            // There is no middle where the bounds can be ignored. Make the left span the whole
            // width of dst and we will not draw mid or right.
            left = {dstBounds.left(), mid.top(), dstBounds.right(), mid.bottom()};
        } else {
            left  = {dstBounds.left(), mid.top(), mid.left()       , mid.bottom()};
            right = {mid.right(),      mid.top(), dstBounds.right(), mid.bottom()};
        }
    } else {
        // This is the same as the x direction code if you turn your head 90 degrees CCW. Swap x and
        // y and swap top/bottom with left/right.
        top    = {dstBounds.left(),  dstBounds.top(), srcBounds.left() , dstBounds.bottom()};
        bottom = {srcBounds.right(), dstBounds.top(), dstBounds.right(), dstBounds.bottom()};

        int midA = std::max(srcBounds.left() , dstBounds.left() );
        int midB = std::min(srcBounds.right(), dstBounds.right());
        mid = {midA, srcBounds.top() + radius, midB, srcBounds.bottom() - radius};

        if (mid.isEmpty()) {
            left = {mid.left(), dstBounds.top(), mid.right(), dstBounds.bottom()};
        } else {
            left  = {mid.left(), dstBounds.top(), mid.right(), mid.top()         };
            right = {mid.left(), mid.bottom()   , mid.right(), dstBounds.bottom()};
        }
    }

    auto convolve = [&](SkIRect rect) {
        // Transform rect into the render target's coord system.
        rect.offset(-rtcToSrcOffset);
        convolve_gaussian_1d(dstRenderTargetContext.get(), srcView, srcBounds, rtcToSrcOffset, rect,
                             srcAlphaType, direction, radius, sigma, mode);
    };
    auto clear = [&](SkIRect rect) {
        // Transform rect into the render target's coord system.
        rect.offset(-rtcToSrcOffset);
        dstRenderTargetContext->priv().clearAtLeast(rect, SK_PMColor4fTRANSPARENT);
    };

    // Doing mid separately will cause two draws to occur (left and right batch together). At
    // small sizes of mid it is worse to issue more draws than to just execute the slightly
    // more complicated shader that implements the tile mode across mid. This threshold is
    // very arbitrary right now. It is believed that a 21x44 mid on a Moto G4 is a significant
    // regression compared to doing one draw but it has not been locally evaluated or tuned.
    // The optimal cutoff is likely to vary by GPU.
    if (!mid.isEmpty() && mid.width()*mid.height() < 256*256) {
        left.join(mid);
        left.join(right);
        mid = SkIRect::MakeEmpty();
        right = SkIRect::MakeEmpty();
        // It's unknown whether for kDecal it'd be better to expand the draw rather than a draw and
        // up to two clears.
        if (mode == SkTileMode::kClamp) {
            left.join(top);
            left.join(bottom);
            top = SkIRect::MakeEmpty();
            bottom = SkIRect::MakeEmpty();
        }
    }

    if (!top.isEmpty()) {
        if (mode == SkTileMode::kDecal) {
            clear(top);
        } else {
            convolve(top);
        }
    }

    if (!bottom.isEmpty()) {
        if (mode == SkTileMode::kDecal) {
            clear(bottom);
        } else {
            convolve(bottom);
        }
    }

    if (mid.isEmpty()) {
        convolve(left);
    } else {
        convolve(left);
        convolve(right);
        convolve(mid);
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
            context, srcColorType, std::move(colorSpace), fit, dstSize, 1, GrMipmapped::kNo,
            srcView.proxy()->isProtected(), srcView.origin());
    if (!dstRenderTargetContext) {
        return nullptr;
    }

    GrPaint paint;
    auto fp = GrTextureEffect::MakeSubset(std::move(srcView), srcAlphaType, SkMatrix::I(),
                                          GrSamplerState::Filter::kLinear, srcBounds, srcBounds,
                                          *context->priv().caps());
    paint.setColorFragmentProcessor(std::move(fp));
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);

    dstRenderTargetContext->fillRectToRect(nullptr, std::move(paint), GrAA::kNo, SkMatrix::I(),
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
    SkASSERT(sigmaX || sigmaY);
    std::unique_ptr<GrRenderTargetContext> dstRenderTargetContext;
    if (sigmaX > 0.0f) {
        SkBackingFit xFit = sigmaY > 0 ? SkBackingFit::kApprox : fit;
        // Expand the dstBounds vertically to produce necessary content for the y-pass. Then we will
        // clip these in a tile-mode dependent way to ensure the tile-mode gets implemented
        // correctly. However, if we're not going to do a y-pass then we must use the original
        // dstBounds without clipping to produce the correct output size.
        SkIRect xPassDstBounds = dstBounds;
        if (sigmaY) {
            xPassDstBounds.outset(0, radiusY);
            if (mode == SkTileMode::kRepeat || mode == SkTileMode::kMirror) {
                int srcH = srcBounds.height();
                int srcTop = srcBounds.top();
                if (mode == SkTileMode::kMirror) {
                    srcTop -= srcH;
                    srcH *= 2;
                }

                float floatH = srcH;
                // First row above the dst rect where we should restart the tile mode.
                int n = sk_float_floor2int_no_saturate((xPassDstBounds.top() - srcTop)/floatH);
                int topClip = srcTop + n*srcH;

                // First row above below the dst rect where we should restart the tile mode.
                n = sk_float_ceil2int_no_saturate(
                        (xPassDstBounds.bottom() - srcBounds.bottom())/floatH);
                int bottomClip = srcBounds.bottom() + n*srcH;

                xPassDstBounds.fTop    = std::max(xPassDstBounds.top(),    topClip);
                xPassDstBounds.fBottom = std::min(xPassDstBounds.bottom(), bottomClip);
            } else {
                if (xPassDstBounds.fBottom <= srcBounds.top()) {
                    if (mode == SkTileMode::kDecal) {
                        return nullptr;
                    }
                    xPassDstBounds.fTop = srcBounds.top();
                    xPassDstBounds.fBottom = xPassDstBounds.fTop + 1;
                } else if (xPassDstBounds.fTop >= srcBounds.bottom()) {
                    if (mode == SkTileMode::kDecal) {
                        return nullptr;
                    }
                    xPassDstBounds.fBottom = srcBounds.bottom();
                    xPassDstBounds.fTop = xPassDstBounds.fBottom - 1;
                } else {
                    xPassDstBounds.fTop    = std::max(xPassDstBounds.fTop,    srcBounds.top());
                    xPassDstBounds.fBottom = std::min(xPassDstBounds.fBottom, srcBounds.bottom());
                }
                int leftSrcEdge  = srcBounds.fLeft  - radiusX ;
                int rightSrcEdge = srcBounds.fRight + radiusX;
                if (mode == SkTileMode::kClamp) {
                    // In clamp the column just outside the src bounds has the same value as the
                    // column just inside, unlike decal.
                    leftSrcEdge  += 1;
                    rightSrcEdge -= 1;
                }
                if (xPassDstBounds.fRight <= leftSrcEdge) {
                    if (mode == SkTileMode::kDecal) {
                        return nullptr;
                    }
                    xPassDstBounds.fLeft = xPassDstBounds.fRight - 1;
                } else {
                    xPassDstBounds.fLeft = std::max(xPassDstBounds.fLeft, leftSrcEdge);
                }
                if (xPassDstBounds.fLeft >= rightSrcEdge) {
                    if (mode == SkTileMode::kDecal) {
                        return nullptr;
                    }
                    xPassDstBounds.fRight = xPassDstBounds.fLeft + 1;
                } else {
                    xPassDstBounds.fRight = std::min(xPassDstBounds.fRight, rightSrcEdge);
                }
            }
        }
        dstRenderTargetContext = convolve_gaussian(
                context, std::move(srcView), srcColorType, srcAlphaType, srcBounds, xPassDstBounds,
                Direction::kX, radiusX, sigmaX, mode, colorSpace, xFit);
        if (!dstRenderTargetContext) {
            return nullptr;
        }
        srcView = dstRenderTargetContext->readSurfaceView();
        SkIVector newDstBoundsOffset = dstBounds.topLeft() - xPassDstBounds.topLeft();
        dstBounds = SkIRect::MakeSize(dstBounds.size()).makeOffset(newDstBoundsOffset);
        srcBounds = SkIRect::MakeSize(xPassDstBounds.size());
    }

    if (sigmaY == 0.0f) {
        return dstRenderTargetContext;
    }

    return convolve_gaussian(context, std::move(srcView), srcColorType, srcAlphaType, srcBounds,
                             dstBounds, Direction::kY, radiusY, sigmaY, mode, colorSpace, fit);
}

namespace SkGpuBlurUtils {

std::unique_ptr<GrRenderTargetContext> LegacyGaussianBlur(GrRecordingContext* context,
                                                          GrSurfaceProxyView srcView,
                                                          GrColorType srcColorType,
                                                          SkAlphaType srcAlphaType,
                                                          sk_sp<SkColorSpace> colorSpace,
                                                          const SkIRect& dstBounds,
                                                          const SkIRect& srcBounds,
                                                          float sigmaX,
                                                          float sigmaY,
                                                          SkTileMode mode,
                                                          SkBackingFit fit);

std::unique_ptr<GrRenderTargetContext> GaussianBlur(GrRecordingContext* context,
                                                    GrSurfaceProxyView srcView,
                                                    GrColorType srcColorType,
                                                    SkAlphaType srcAlphaType,
                                                    sk_sp<SkColorSpace> colorSpace,
                                                    SkIRect dstBounds,
                                                    SkIRect srcBounds,
                                                    float sigmaX,
                                                    float sigmaY,
                                                    SkTileMode mode,
                                                    SkBackingFit fit) {
#ifdef SK_USE_LEGACY_GPU_BLUR
    return LegacyGaussianBlur(context, srcView, srcColorType, srcAlphaType, std::move(colorSpace),
                              dstBounds, srcBounds, sigmaX, sigmaY, mode, fit);
#endif
    SkASSERT(context);
    TRACE_EVENT2("skia.gpu", "GaussianBlur", "sigmaX", sigmaX, "sigmaY", sigmaY);

    if (!srcView.asTextureProxy()) {
        return nullptr;
    }

    int maxRenderTargetSize = context->priv().caps()->maxRenderTargetSize();
    if (dstBounds.width() > maxRenderTargetSize || dstBounds.height() > maxRenderTargetSize) {
        return nullptr;
    }

    // Attempt to reduce the srcBounds in order to detect that we can set the sigmas to zero or
    // to reduce the amount of work to rescale the source if sigmas are large. TODO: Could consider
    // how to minimize the required source bounds for repeat/mirror modes.
    if (mode == SkTileMode::kClamp || mode == SkTileMode::kDecal) {
        int radiusX = sigma_radius(sigmaX);
        int radiusY = sigma_radius(sigmaY);
        SkIRect reach = dstBounds.makeOutset(radiusX, radiusY);
        SkIRect intersection;
        if (!intersection.intersect(reach, srcBounds)) {
            if (mode == SkTileMode::kDecal) {
                return nullptr;
            } else {
                if (reach.fLeft >= srcBounds.fRight) {
                    srcBounds.fLeft = srcBounds.fRight - 1;
                } else if (reach.fRight <= srcBounds.fLeft) {
                    srcBounds.fRight = srcBounds.fLeft + 1;
                }
                if (reach.fTop >= srcBounds.fBottom) {
                    srcBounds.fTop = srcBounds.fBottom - 1;
                } else if (reach.fBottom <= srcBounds.fTop) {
                    srcBounds.fBottom = srcBounds.fTop + 1;
                }
            }
        } else {
            srcBounds = intersection;
        }
    }

    if (mode != SkTileMode::kDecal) {
        // All non-decal tile modes are equivalent for one pixel width/height src and amount to a
        // single color value repeated at each column/row. Applying the normalized kernel to that
        // column/row yields that same color. So no blurring is necessary.
        if (srcBounds.width() == 1) {
            sigmaX = 0.f;
        }
        if (srcBounds.height() == 1) {
            sigmaY = 0.f;
        }
    }

    // If we determined that there is no blurring necessary in either direction then just do a
    // a draw that applies the tile mode.
    if (!sigmaX && !sigmaY) {
        auto result = GrRenderTargetContext::Make(context, srcColorType, std::move(colorSpace), fit,
                                                  dstBounds.size());
        GrSamplerState sampler(SkTileModeToWrapMode(mode), GrSamplerState::Filter::kNearest);
        auto fp = GrTextureEffect::MakeSubset(std::move(srcView), srcAlphaType, SkMatrix::I(),
                                              sampler, SkRect::Make(srcBounds),
                                              SkRect::Make(dstBounds), *context->priv().caps());
        GrPaint paint;
        paint.setColorFragmentProcessor(std::move(fp));
        result->drawRect(nullptr, std::move(paint), GrAA::kNo, SkMatrix::I(),
                         SkRect::Make(dstBounds.size()));
        return result;
    }

    if (sigmaX <= MAX_BLUR_SIGMA && sigmaY <= MAX_BLUR_SIGMA) {
        int radiusX = sigma_radius(sigmaX);
        int radiusY = sigma_radius(sigmaY);
        SkASSERT(radiusX <= GrGaussianConvolutionFragmentProcessor::kMaxKernelRadius);
        SkASSERT(radiusY <= GrGaussianConvolutionFragmentProcessor::kMaxKernelRadius);
        // For really small blurs (certainly no wider than 5x5 on desktop GPUs) it is faster to just
        // launch a single non separable kernel vs two launches.
        const int kernelSize = (2 * radiusX + 1) * (2 * radiusY + 1);
        if (sigmaX > 0 && sigmaY > 0 && kernelSize <= GrMatrixConvolutionEffect::kMaxUniformSize) {
            // Apply the proxy offset to src bounds and offset directly
            return convolve_gaussian_2d(context, std::move(srcView), srcColorType, srcBounds,
                                        dstBounds, radiusX, radiusY, sigmaX, sigmaY, mode,
                                        std::move(colorSpace), fit);
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
    if (rescaledSize.isEmpty()) {
        // TODO: Handle this degenerate case.
        return nullptr;
    }
    // Compute the sigmas using the actual scale factors used once we integerized the rescaledSize.
    scaleX = static_cast<float>(rescaledSize.width()) /srcBounds.width();
    scaleY = static_cast<float>(rescaledSize.height())/srcBounds.height();
    sigmaX *= scaleX;
    sigmaY *= scaleY;

    auto srcCtx = GrSurfaceContext::Make(context, srcView, srcColorType, srcAlphaType, colorSpace);
    SkASSERT(srcCtx);
    GrImageInfo rescaledII(srcColorType, srcAlphaType, colorSpace, rescaledSize);
    srcCtx = srcCtx->rescale(rescaledII, srcCtx->origin(), srcBounds, SkSurface::RescaleGamma::kSrc,
                             kLow_SkFilterQuality);
    if (!srcCtx) {
        return nullptr;
    }
    srcView = srcCtx->readSurfaceView();
    // Drop the context so we don't hold the proxy longer than necessary.
    srcCtx.reset();

    // Compute the dst bounds in the scaled down space. First move the origin to be at the top
    // left since we trimmed off everything above and to the left of the original src bounds during
    // the rescale.
    SkRect scaledDstBounds = SkRect::Make(dstBounds.makeOffset(-srcBounds.topLeft()));
    scaledDstBounds.fLeft   *= scaleX;
    scaledDstBounds.fTop    *= scaleY;
    scaledDstBounds.fRight  *= scaleX;
    scaledDstBounds.fBottom *= scaleY;
    // Turn the scaled down dst bounds into an integer pixel rect.
    auto scaledDstBoundsI = scaledDstBounds.roundOut();

    auto rtc = GaussianBlur(context, std::move(srcView), srcColorType, srcAlphaType, colorSpace,
                            scaledDstBoundsI, SkIRect::MakeSize(rescaledSize), sigmaX, sigmaY, mode,
                            fit);
    if (!rtc) {
        return nullptr;
    }
    // We rounded out the integer scaled dst bounds. Select the fractional dst bounds from the
    // integer dimension blurred result when we scale back up.
    scaledDstBounds.offset(-scaledDstBoundsI.left(), -scaledDstBoundsI.top());
    return reexpand(context, std::move(rtc), scaledDstBounds, dstBounds.size(),
                    std::move(colorSpace), fit);
}
}  // namespace SkGpuBlurUtils

#endif
