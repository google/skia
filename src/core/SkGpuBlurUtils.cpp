/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkGpuBlurUtils.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkRect.h"

#if SK_SUPPORT_GPU
#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/effects/GrGaussianConvolutionFragmentProcessor.h"
#include "src/gpu/effects/GrMatrixConvolutionEffect.h"

#include "src/gpu/SkGr.h"


using Direction = GrGaussianConvolutionFragmentProcessor::Direction;

static void fill_in_2D_gaussian_kernel(float* kernel, int width, int height,
                                       SkScalar sigmaX, SkScalar sigmaY) {
    const float twoSigmaSqrdX = 2.0f * SkScalarToFloat(SkScalarSquare(sigmaX));
    const float twoSigmaSqrdY = 2.0f * SkScalarToFloat(SkScalarSquare(sigmaY));

    // SkGpuBlurUtils::GaussianBlur() should have detected the cases where a 2D blur
    // degenerates to a 1D on X or Y, or to the identity.
    SkASSERT(!SkGpuBlurUtils::IsEffectivelyZeroSigma(sigmaX) &&
             !SkGpuBlurUtils::IsEffectivelyZeroSigma(sigmaY));
    SkASSERT(!SkScalarNearlyZero(twoSigmaSqrdX) && !SkScalarNearlyZero(twoSigmaSqrdY));

    const float sigmaXDenom = 1.0f / twoSigmaSqrdX;
    const float sigmaYDenom = 1.0f / twoSigmaSqrdY;
    const int xRadius = width / 2;
    const int yRadius = height / 2;

    float sum = 0.0f;
    for (int x = 0; x < width; x++) {
        float xTerm = static_cast<float>(x - xRadius);
        xTerm = xTerm * xTerm * sigmaXDenom;
        for (int y = 0; y < height; y++) {
            float yTerm = static_cast<float>(y - yRadius);
            float xyTerm = sk_float_exp(-(xTerm + yTerm * yTerm * sigmaYDenom));
            // Note that the constant term (1/(sqrt(2*pi*sigma^2)) of the Gaussian
            // is dropped here, since we renormalize the kernel below.
            kernel[y * width + x] = xyTerm;
            sum += xyTerm;
        }
    }
    // Normalize the kernel
    float scale = 1.0f / sum;
    for (int i = 0; i < width * height; ++i) {
        kernel[i] *= scale;
    }
}

/**
 * Draws 'rtcRect' into 'surfaceDrawContext' evaluating a 1D Gaussian over 'srcView'. The src rect
 * is 'rtcRect' offset by 'rtcToSrcOffset'. 'mode' and 'bounds' are applied to the src coords.
 */
static void convolve_gaussian_1d(GrSurfaceDrawContext* surfaceDrawContext,
                                 GrSurfaceProxyView srcView,
                                 const SkIRect srcSubset,
                                 SkIVector rtcToSrcOffset,
                                 const SkIRect& rtcRect,
                                 SkAlphaType srcAlphaType,
                                 Direction direction,
                                 int radius,
                                 float sigma,
                                 SkTileMode mode) {
    SkASSERT(radius && !SkGpuBlurUtils::IsEffectivelyZeroSigma(sigma));
    GrPaint paint;
    auto wm = SkTileModeToWrapMode(mode);
    auto srcRect = rtcRect.makeOffset(rtcToSrcOffset);

    // NOTE: This could just be GrMatrixConvolutionEffect with one of the dimensions set to 1
    // and the appropriate kernel already computed, but there's value in keeping the shader simpler.
    // TODO(michaelludwig): Is this true? If not, is the shader key simplicity worth it two have
    // two convolution effects?
    std::unique_ptr<GrFragmentProcessor> conv(GrGaussianConvolutionFragmentProcessor::Make(
            std::move(srcView), srcAlphaType, direction, radius, sigma, wm, srcSubset, &srcRect,
            *surfaceDrawContext->caps()));
    paint.setColorFragmentProcessor(std::move(conv));
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
    surfaceDrawContext->fillRectToRect(nullptr, std::move(paint), GrAA::kNo, SkMatrix::I(),
                                       SkRect::Make(rtcRect), SkRect::Make(srcRect));
}

static std::unique_ptr<GrSurfaceDrawContext> convolve_gaussian_2d(GrRecordingContext* context,
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
    SkASSERT(radiusX && radiusY);
    SkASSERT(!SkGpuBlurUtils::IsEffectivelyZeroSigma(sigmaX) &&
             !SkGpuBlurUtils::IsEffectivelyZeroSigma(sigmaY));
    auto surfaceDrawContext = GrSurfaceDrawContext::Make(
            context, srcColorType, std::move(finalCS), dstFit, dstBounds.size(), 1,
            GrMipmapped::kNo, srcView.proxy()->isProtected(), srcView.origin());
    if (!surfaceDrawContext) {
        return nullptr;
    }

    SkISize size = SkISize::Make(SkGpuBlurUtils::KernelWidth(radiusX),
                                 SkGpuBlurUtils::KernelWidth(radiusY));
    SkIPoint kernelOffset = SkIPoint::Make(radiusX, radiusY);
    GrPaint paint;
    auto wm = SkTileModeToWrapMode(mode);

    // GaussianBlur() should have downsampled the request until we can handle the 2D blur with
    // just a uniform array.
    SkASSERT(size.area() <= GrMatrixConvolutionEffect::kMaxUniformSize);
    float kernel[GrMatrixConvolutionEffect::kMaxUniformSize];
    fill_in_2D_gaussian_kernel(kernel, size.width(), size.height(), sigmaX, sigmaY);
    auto conv = GrMatrixConvolutionEffect::Make(context, std::move(srcView), srcBounds,
                                                size, kernel, 1.0f, 0.0f, kernelOffset, wm, true,
                                                *surfaceDrawContext->caps());

    paint.setColorFragmentProcessor(std::move(conv));
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);

    // 'dstBounds' is actually in 'srcView' proxy space. It represents the blurred area from src
    // space that we want to capture in the new RTC at {0, 0}. Hence, we use its size as the rect to
    // draw and it directly as the local rect.
    surfaceDrawContext->fillRectToRect(nullptr, std::move(paint), GrAA::kNo, SkMatrix::I(),
                                       SkRect::Make(dstBounds.size()), SkRect::Make(dstBounds));

    return surfaceDrawContext;
}

static std::unique_ptr<GrSurfaceDrawContext> convolve_gaussian(GrRecordingContext* context,
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
    using namespace SkGpuBlurUtils;
    SkASSERT(radius > 0 && !SkGpuBlurUtils::IsEffectivelyZeroSigma(sigma));
    // Logically we're creating an infinite blur of 'srcBounds' of 'srcView' with 'mode' tiling
    // and then capturing the 'dstBounds' portion in a new RTC where the top left of 'dstBounds' is
    // at {0, 0} in the new RTC.
    auto dstRenderTargetContext = GrSurfaceDrawContext::Make(
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
        dstRenderTargetContext->clearAtLeast(rect, SK_PMColor4fTRANSPARENT);
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
static std::unique_ptr<GrSurfaceDrawContext> reexpand(GrRecordingContext* context,
                                                      std::unique_ptr<GrSurfaceContext> src,
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

    auto dstRenderTargetContext = GrSurfaceDrawContext::Make(
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

static std::unique_ptr<GrSurfaceDrawContext> two_pass_gaussian(GrRecordingContext* context,
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
    SkASSERT(radiusX || radiusY);
    std::unique_ptr<GrSurfaceDrawContext> dstRenderTargetContext;
    if (radiusX > 0) {
        SkBackingFit xFit = radiusY > 0 ? SkBackingFit::kApprox : fit;
        // Expand the dstBounds vertically to produce necessary content for the y-pass. Then we will
        // clip these in a tile-mode dependent way to ensure the tile-mode gets implemented
        // correctly. However, if we're not going to do a y-pass then we must use the original
        // dstBounds without clipping to produce the correct output size.
        SkIRect xPassDstBounds = dstBounds;
        if (radiusY) {
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

    if (!radiusY) {
        return dstRenderTargetContext;
    }

    return convolve_gaussian(context, std::move(srcView), srcColorType, srcAlphaType, srcBounds,
                             dstBounds, Direction::kY, radiusY, sigmaY, mode, colorSpace, fit);
}

namespace SkGpuBlurUtils {

std::unique_ptr<GrSurfaceDrawContext> GaussianBlur(GrRecordingContext* context,
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
    SkASSERT(context);
    TRACE_EVENT2("skia.gpu", "GaussianBlur", "sigmaX", sigmaX, "sigmaY", sigmaY);

    if (!srcView.asTextureProxy()) {
        return nullptr;
    }

    int maxRenderTargetSize = context->priv().caps()->maxRenderTargetSize();
    if (dstBounds.width() > maxRenderTargetSize || dstBounds.height() > maxRenderTargetSize) {
        return nullptr;
    }

    int radiusX = SigmaRadius(sigmaX);
    int radiusY = SigmaRadius(sigmaY);
    // Attempt to reduce the srcBounds in order to detect that we can set the sigmas to zero or
    // to reduce the amount of work to rescale the source if sigmas are large. TODO: Could consider
    // how to minimize the required source bounds for repeat/mirror modes.
    if (mode == SkTileMode::kClamp || mode == SkTileMode::kDecal) {
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
            radiusX = 0;
        }
        if (srcBounds.height() == 1) {
            sigmaY = 0.f;
            radiusY = 0;
        }
    }

    // If we determined that there is no blurring necessary in either direction then just do a
    // a draw that applies the tile mode.
    if (!radiusX && !radiusY) {
        auto result = GrSurfaceDrawContext::Make(context, srcColorType, std::move(colorSpace), fit,
                                                  dstBounds.size(), 1, GrMipmapped::kNo,
                                                  srcView.proxy()->isProtected(), srcView.origin());
        GrSamplerState sampler(SkTileModeToWrapMode(mode), GrSamplerState::Filter::kNearest);
        auto fp = GrTextureEffect::MakeSubset(std::move(srcView),
                                              srcAlphaType,
                                              SkMatrix::I(),
                                              sampler,
                                              SkRect::Make(srcBounds),
                                              SkRect::Make(dstBounds),
                                              *context->priv().caps());
        result->fillRectToRectWithFP(dstBounds, SkIRect::MakeSize(dstBounds.size()), std::move(fp));
        return result;
    }

    if (sigmaX <= kMaxSigma && sigmaY <= kMaxSigma) {
        SkASSERT(radiusX <= GrGaussianConvolutionFragmentProcessor::kMaxKernelRadius);
        SkASSERT(radiusY <= GrGaussianConvolutionFragmentProcessor::kMaxKernelRadius);
        // For really small blurs (certainly no wider than 5x5 on desktop GPUs) it is faster to just
        // launch a single non separable kernel vs two launches.
        const int kernelSize = (2 * radiusX + 1) * (2 * radiusY + 1);
        if (radiusX > 0 && radiusY > 0 &&
            kernelSize <= GrMatrixConvolutionEffect::kMaxUniformSize) {
            // Apply the proxy offset to src bounds and offset directly
            return convolve_gaussian_2d(context, std::move(srcView), srcColorType, srcBounds,
                                        dstBounds, radiusX, radiusY, sigmaX, sigmaY, mode,
                                        std::move(colorSpace), fit);
        }
        // This will automatically degenerate into a single pass of X or Y if only one of the
        // radii are non-zero.
        return two_pass_gaussian(context, std::move(srcView), srcColorType, srcAlphaType,
                                 std::move(colorSpace), srcBounds, dstBounds, sigmaX, sigmaY,
                                 radiusX, radiusY, mode, fit);
    }

    float scaleX = sigmaX > kMaxSigma ? kMaxSigma/sigmaX : 1.f;
    float scaleY = sigmaY > kMaxSigma ? kMaxSigma/sigmaY : 1.f;

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

    GrColorInfo colorInfo(srcColorType, srcAlphaType, colorSpace);
    auto srcCtx = GrSurfaceContext::Make(context, srcView, colorInfo);
    SkASSERT(srcCtx);
    // When we are in clamp mode any artifacts in the edge pixels due to downscaling may be
    // exacerbated because of the tile mode. The particularly egregious case is when the original
    // image has transparent black around the edges and the downscaling pulls in some non-zero
    // values from the interior. Ultimately it'd be better for performance if the calling code could
    // give us extra context around the blur to account for this. We don't currently have a good way
    // to communicate this up stack. So we leave a 1 pixel border around the rescaled src bounds.
    // We populate the top 1 pixel tall row of this border by rescaling the top row of the original
    // source bounds into it. Because this is only rescaling in x (i.e. rescaling a 1 pixel high
    // row into a shorter but still 1 pixel high row) we won't read any interior values. And similar
    // for the other three borders. We'll adjust the source/dest bounds rescaled blur so that this
    // border of extra pixels is used as the edge pixels for clamp mode but the dest bounds
    // corresponds only to the pixels inside the border (the normally rescaled pixels inside this
    // border).
    int pad = mode == SkTileMode::kClamp ? 1 : 0;
    auto rescaledSDC = GrSurfaceDrawContext::Make(
            srcCtx->recordingContext(),
            colorInfo.colorType(),
            colorInfo.refColorSpace(),
            SkBackingFit::kApprox,
            {rescaledSize.width() + 2*pad, rescaledSize.height() + 2*pad},
            1,
            GrMipmapped::kNo,
            srcCtx->asSurfaceProxy()->isProtected(),
            srcCtx->origin());

    if (!rescaledSDC) {
        return nullptr;
    }
    if (!srcCtx->rescaleInto(rescaledSDC.get(),
                             SkIRect::MakeSize(rescaledSize).makeOffset(pad, pad),
                             srcBounds,
                             SkSurface::RescaleGamma::kSrc,
                             SkSurface::RescaleMode::kRepeatedLinear)) {
        return nullptr;
    }
    if (pad) {
        // Rather than run a potentially multi-pass rescaler on single rows/columns we just do a
        // single bilerp draw. If we find this quality unacceptable we should think more about how
        // to rescale these with better quality but without 4 separate multi-pass downscales.
        auto cheapDownscale = [&](SkIRect dstRect, SkIRect srcRect) {
                rescaledSDC->drawTexture(nullptr,
                                         srcCtx->readSurfaceView(),
                                         srcAlphaType,
                                         GrSamplerState::Filter::kLinear,
                                         GrSamplerState::MipmapMode::kNone,
                                         SkBlendMode::kSrc,
                                         SK_PMColor4fWHITE,
                                         SkRect::Make(srcRect),
                                         SkRect::Make(dstRect),
                                         GrAA::kNo,
                                         GrQuadAAFlags::kNone,
                                         SkCanvas::SrcRectConstraint::kFast_SrcRectConstraint,
                                         SkMatrix::I(),
                                         nullptr);
        };
        auto [dw, dh] = rescaledSize;
        // The are the src rows and columns from the source that we will scale into the dst padding.
        float sLCol = srcBounds.left();
        float sTRow = srcBounds.top();
        float sRCol = srcBounds.right() - 1;
        float sBRow = srcBounds.bottom() - 1;

        int sx = srcBounds.left();
        int sy = srcBounds.top();
        int sw = srcBounds.width();
        int sh = srcBounds.height();

        // Downscale the edges from the original source. These draws should batch together (and with
        // the above interior rescaling when it is a single pass).
        cheapDownscale(SkIRect::MakeXYWH(     0,      1,  1, dh),
                       SkIRect::MakeXYWH( sLCol,     sy,  1, sh));
        cheapDownscale(SkIRect::MakeXYWH(     1,      0, dw,  1),
                       SkIRect::MakeXYWH(    sx,  sTRow, sw,  1));
        cheapDownscale(SkIRect::MakeXYWH(dw + 1,      1,  1, dh),
                       SkIRect::MakeXYWH( sRCol,     sy,  1, sh));
        cheapDownscale(SkIRect::MakeXYWH(     1, dh + 1, dw,  1),
                       SkIRect::MakeXYWH(    sx,  sBRow, sw,  1));

        // Copy the corners from the original source. These would batch with the edges except that
        // at time of writing we recognize these can use kNearest and downgrade the filter. So they
        // batch with each other but not the edge draws.
        cheapDownscale(SkIRect::MakeXYWH(    0,     0,  1, 1),
                       SkIRect::MakeXYWH(sLCol, sTRow,  1, 1));
        cheapDownscale(SkIRect::MakeXYWH(dw + 1,     0, 1, 1),
                       SkIRect::MakeXYWH(sRCol, sTRow,  1, 1));
        cheapDownscale(SkIRect::MakeXYWH(dw + 1,dh + 1, 1, 1),
                       SkIRect::MakeXYWH(sRCol, sBRow,  1, 1));
        cheapDownscale(SkIRect::MakeXYWH(    0, dh + 1, 1, 1),
                       SkIRect::MakeXYWH(sLCol, sBRow,  1, 1));
    }
    srcView = rescaledSDC->readSurfaceView();
    // Drop the contexts so we don't hold the proxies longer than necessary.
    rescaledSDC.reset();
    srcCtx.reset();

    // Compute the dst bounds in the scaled down space. First move the origin to be at the top
    // left since we trimmed off everything above and to the left of the original src bounds during
    // the rescale.
    SkRect scaledDstBounds = SkRect::Make(dstBounds.makeOffset(-srcBounds.topLeft()));
    scaledDstBounds.fLeft   *= scaleX;
    scaledDstBounds.fTop    *= scaleY;
    scaledDstBounds.fRight  *= scaleX;
    scaledDstBounds.fBottom *= scaleY;
    // Account for padding in our rescaled src, if any.
    scaledDstBounds.offset(pad, pad);
    // Turn the scaled down dst bounds into an integer pixel rect.
    auto scaledDstBoundsI = scaledDstBounds.roundOut();

    SkIRect scaledSrcBounds = SkIRect::MakeSize(srcView.dimensions());
    auto sdc = GaussianBlur(context,
                            std::move(srcView),
                            srcColorType,
                            srcAlphaType,
                            colorSpace,
                            scaledDstBoundsI,
                            scaledSrcBounds,
                            sigmaX,
                            sigmaY,
                            mode,
                            fit);
    if (!sdc) {
        return nullptr;
    }
    // We rounded out the integer scaled dst bounds. Select the fractional dst bounds from the
    // integer dimension blurred result when we scale back up.
    scaledDstBounds.offset(-scaledDstBoundsI.left(), -scaledDstBoundsI.top());
    return reexpand(context, std::move(sdc), scaledDstBounds, dstBounds.size(),
                    std::move(colorSpace), fit);
}

bool ComputeBlurredRRectParams(const SkRRect& srcRRect, const SkRRect& devRRect,
                               SkScalar sigma, SkScalar xformedSigma,
                               SkRRect* rrectToDraw,
                               SkISize* widthHeight,
                               SkScalar rectXs[kBlurRRectMaxDivisions],
                               SkScalar rectYs[kBlurRRectMaxDivisions],
                               SkScalar texXs[kBlurRRectMaxDivisions],
                               SkScalar texYs[kBlurRRectMaxDivisions]) {
    unsigned int devBlurRadius = 3*SkScalarCeilToInt(xformedSigma-1/6.0f);
    SkScalar srcBlurRadius = 3.0f * sigma;

    const SkRect& devOrig = devRRect.getBounds();
    const SkVector& devRadiiUL = devRRect.radii(SkRRect::kUpperLeft_Corner);
    const SkVector& devRadiiUR = devRRect.radii(SkRRect::kUpperRight_Corner);
    const SkVector& devRadiiLR = devRRect.radii(SkRRect::kLowerRight_Corner);
    const SkVector& devRadiiLL = devRRect.radii(SkRRect::kLowerLeft_Corner);

    const int devLeft  = SkScalarCeilToInt(std::max<SkScalar>(devRadiiUL.fX, devRadiiLL.fX));
    const int devTop   = SkScalarCeilToInt(std::max<SkScalar>(devRadiiUL.fY, devRadiiUR.fY));
    const int devRight = SkScalarCeilToInt(std::max<SkScalar>(devRadiiUR.fX, devRadiiLR.fX));
    const int devBot   = SkScalarCeilToInt(std::max<SkScalar>(devRadiiLL.fY, devRadiiLR.fY));

    // This is a conservative check for nine-patchability
    if (devOrig.fLeft + devLeft + devBlurRadius >= devOrig.fRight  - devRight - devBlurRadius ||
        devOrig.fTop  + devTop  + devBlurRadius >= devOrig.fBottom - devBot   - devBlurRadius) {
        return false;
    }

    const SkVector& srcRadiiUL = srcRRect.radii(SkRRect::kUpperLeft_Corner);
    const SkVector& srcRadiiUR = srcRRect.radii(SkRRect::kUpperRight_Corner);
    const SkVector& srcRadiiLR = srcRRect.radii(SkRRect::kLowerRight_Corner);
    const SkVector& srcRadiiLL = srcRRect.radii(SkRRect::kLowerLeft_Corner);

    const SkScalar srcLeft  = std::max<SkScalar>(srcRadiiUL.fX, srcRadiiLL.fX);
    const SkScalar srcTop   = std::max<SkScalar>(srcRadiiUL.fY, srcRadiiUR.fY);
    const SkScalar srcRight = std::max<SkScalar>(srcRadiiUR.fX, srcRadiiLR.fX);
    const SkScalar srcBot   = std::max<SkScalar>(srcRadiiLL.fY, srcRadiiLR.fY);

    int newRRWidth = 2*devBlurRadius + devLeft + devRight + 1;
    int newRRHeight = 2*devBlurRadius + devTop + devBot + 1;
    widthHeight->fWidth = newRRWidth + 2 * devBlurRadius;
    widthHeight->fHeight = newRRHeight + 2 * devBlurRadius;

    const SkRect srcProxyRect = srcRRect.getBounds().makeOutset(srcBlurRadius, srcBlurRadius);

    rectXs[0] = srcProxyRect.fLeft;
    rectXs[1] = srcProxyRect.fLeft + 2*srcBlurRadius + srcLeft;
    rectXs[2] = srcProxyRect.fRight - 2*srcBlurRadius - srcRight;
    rectXs[3] = srcProxyRect.fRight;

    rectYs[0] = srcProxyRect.fTop;
    rectYs[1] = srcProxyRect.fTop + 2*srcBlurRadius + srcTop;
    rectYs[2] = srcProxyRect.fBottom - 2*srcBlurRadius - srcBot;
    rectYs[3] = srcProxyRect.fBottom;

    texXs[0] = 0.0f;
    texXs[1] = 2.0f*devBlurRadius + devLeft;
    texXs[2] = 2.0f*devBlurRadius + devLeft + 1;
    texXs[3] = SkIntToScalar(widthHeight->fWidth);

    texYs[0] = 0.0f;
    texYs[1] = 2.0f*devBlurRadius + devTop;
    texYs[2] = 2.0f*devBlurRadius + devTop + 1;
    texYs[3] = SkIntToScalar(widthHeight->fHeight);

    const SkRect newRect = SkRect::MakeXYWH(SkIntToScalar(devBlurRadius),
                                            SkIntToScalar(devBlurRadius),
                                            SkIntToScalar(newRRWidth),
                                            SkIntToScalar(newRRHeight));
    SkVector newRadii[4];
    newRadii[0] = { SkScalarCeilToScalar(devRadiiUL.fX), SkScalarCeilToScalar(devRadiiUL.fY) };
    newRadii[1] = { SkScalarCeilToScalar(devRadiiUR.fX), SkScalarCeilToScalar(devRadiiUR.fY) };
    newRadii[2] = { SkScalarCeilToScalar(devRadiiLR.fX), SkScalarCeilToScalar(devRadiiLR.fY) };
    newRadii[3] = { SkScalarCeilToScalar(devRadiiLL.fX), SkScalarCeilToScalar(devRadiiLL.fY) };

    rrectToDraw->setRectRadii(newRect, newRadii);
    return true;
}

// TODO: it seems like there should be some synergy with SkBlurMask::ComputeBlurProfile
// TODO: maybe cache this on the cpu side?
int CreateIntegralTable(float sixSigma, SkBitmap* table) {
    // The texture we're producing represents the integral of a normal distribution over a
    // six-sigma range centered at zero. We want enough resolution so that the linear
    // interpolation done in texture lookup doesn't introduce noticeable artifacts. We
    // conservatively choose to have 2 texels for each dst pixel.
    int minWidth = 2 * sk_float_ceil2int(sixSigma);
    // Bin by powers of 2 with a minimum so we get good profile reuse.
    int width = std::max(SkNextPow2(minWidth), 32);

    if (!table) {
        return width;
    }

    if (!table->tryAllocPixels(SkImageInfo::MakeA8(width, 1))) {
        return 0;
    }
    *table->getAddr8(0, 0) = 255;
    const float invWidth = 1.f / width;
    for (int i = 1; i < width - 1; ++i) {
        float x = (i + 0.5f) * invWidth;
        x = (-6 * x + 3) * SK_ScalarRoot2Over2;
        float integral = 0.5f * (std::erf(x) + 1.f);
        *table->getAddr8(i, 0) = SkToU8(sk_float_round2int(255.f * integral));
    }

    *table->getAddr8(width - 1, 0) = 0;
    table->setImmutable();
    return table->width();
}


void Compute1DGaussianKernel(float* kernel, float sigma, int radius) {
    SkASSERT(radius == SigmaRadius(sigma));
    if (SkGpuBlurUtils::IsEffectivelyZeroSigma(sigma)) {
        // Calling SigmaRadius() produces 1, just computing ceil(sigma)*3 produces 3
        SkASSERT(KernelWidth(radius) == 1);
        std::fill_n(kernel, 1, 0.f);
        kernel[0] = 1.f;
        return;
    }

    // If this fails, kEffectivelyZeroSigma isn't big enough to prevent precision issues
    SkASSERT(!SkScalarNearlyZero(2.f * sigma * sigma));

    const float sigmaDenom = 1.0f / (2.f * sigma * sigma);
    int size = KernelWidth(radius);
    float sum = 0.0f;
    for (int i = 0; i < size; ++i) {
        float term = static_cast<float>(i - radius);
        // Note that the constant term (1/(sqrt(2*pi*sigma^2)) of the Gaussian
        // is dropped here, since we renormalize the kernel below.
        kernel[i] = sk_float_exp(-term * term * sigmaDenom);
        sum += kernel[i];
    }
    // Normalize the kernel
    float scale = 1.0f / sum;
    for (int i = 0; i < size; ++i) {
        kernel[i] *= scale;
    }
}

}  // namespace SkGpuBlurUtils

#endif
