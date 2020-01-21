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

static GrTextureDomain::Mode to_texture_domain_mode(SkTileMode tileMode) {
    switch (tileMode) {
        case SkTileMode::kClamp:
            return GrTextureDomain::kClamp_Mode;
        case SkTileMode::kDecal:
            return GrTextureDomain::kDecal_Mode;
        case SkTileMode::kMirror:
            // TODO (michaelludwig) - Support mirror mode, treat as repeat for now
        case SkTileMode::kRepeat:
            return GrTextureDomain::kRepeat_Mode;
        default:
            SK_ABORT("Unsupported tile mode.");
    }
}

/**
 * Blurs the 'proxyRect' area of 'proxy' and stored the result in 'dst' at a rect
 * the same size as 'proxyRect' but translated by 'translateToDst'. 'bounds' is
 * optional and specifies the edges of the content area of 'proxy' at which
 * 'mode' should be applied. If null the proxy edges are used.
 */
static void convolve_gaussian_1d(GrRenderTargetContext* renderTargetContext,
                                 sk_sp<GrTextureProxy> proxy,
                                 SkIVector srcOffset,
                                 const SkIRect& dstRect,
                                 SkAlphaType srcAlphaType,
                                 Direction direction,
                                 int radius,
                                 float sigma,
                                 SkTileMode mode,
                                 int bounds[2]) {
    GrPaint paint;
    auto domainMode = to_texture_domain_mode(mode);
    int realBounds[2];
    if (bounds) {
        realBounds[0] = bounds[0]; realBounds[1] = bounds[1];
    } else {
        realBounds[0] = 0;
        realBounds[1] = direction == Direction::kX ? proxy->width() : proxy->height();
    }
    std::unique_ptr<GrFragmentProcessor> conv(GrGaussianConvolutionFragmentProcessor::Make(
            std::move(proxy), srcAlphaType, direction, radius, sigma, domainMode, realBounds));
    paint.addColorFragmentProcessor(std::move(conv));
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
    auto srcRect = SkRect::Make(dstRect.makeOffset(srcOffset));
    renderTargetContext->fillRectToRect(GrNoClip(), std::move(paint), GrAA::kNo,
                                                 SkMatrix::I(), SkRect::Make(dstRect), srcRect);
}

static std::unique_ptr<GrRenderTargetContext> convolve_gaussian_2d(GrRecordingContext* context,
                                                                   sk_sp<GrTextureProxy> srcProxy,
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
            GrMipMapped::kNo, srcProxy->isProtected(), srcProxy->origin());
    if (!renderTargetContext) {
        return nullptr;
    }

    SkMatrix localMatrix = SkMatrix::MakeTrans(dstBounds.topLeft());
    SkISize size = SkISize::Make(2 * radiusX + 1,  2 * radiusY + 1);
    SkIPoint kernelOffset = SkIPoint::Make(radiusX, radiusY);
    GrPaint paint;
    auto domainMode = to_texture_domain_mode(mode);
    auto conv = GrMatrixConvolutionEffect::MakeGaussian(std::move(srcProxy), srcBounds, size,
                                                        1.0, 0.0, kernelOffset, domainMode, true,
                                                        sigmaX, sigmaY);
    paint.addColorFragmentProcessor(std::move(conv));
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);

    renderTargetContext->fillRectWithLocalMatrix(GrNoClip(), std::move(paint), GrAA::kNo,
                                                 SkMatrix::I(), SkRect::Make(dstBounds.size()), localMatrix);

    return renderTargetContext;
}

static std::unique_ptr<GrRenderTargetContext> convolve_gaussian(GrRecordingContext* context,
                                                                sk_sp<GrTextureProxy> srcProxy,
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
    // Logically we're creating an infinite blur of 'contentRect' of 'srcProxy' with 'mode' tiling
    // and then capturing the 'dstBounds' portion in a new RTC where the top left of 'dstBounds' is
    // at {0, 0} in the new RTC.
    auto dstRenderTargetContext = GrRenderTargetContext::Make(
            context, srcColorType, std::move(finalCS), fit, dstBounds.size(), 1,
            GrMipMapped::kNo, srcProxy->isProtected(), srcProxy->origin());
    if (!dstRenderTargetContext) {
        return nullptr;
    }

    // This represents the translation from 'dstRenderTargetContext' coords to 'srcProxy' coords.
    auto dstToSrcOffset = dstBounds.topLeft();

    if (SkTileMode::kClamp == mode &&
        contentRect->contains(SkIRect::MakeSize(srcProxy->backingStoreDimensions()))) {
        auto dstRect = SkIRect::MakeSize(dstBounds.size());
        convolve_gaussian_1d(dstRenderTargetContext.get(), std::move(srcProxy), dstToSrcOffset, dstRect,
                             srcAlphaType, direction, radius, sigma, SkTileMode::kClamp, nullptr);
        *contentRect = dstRect;
        return dstRenderTargetContext;
    }

    // 'leftRect' and 'rightRect' are the rects where 'mode' must be enforced. 'midRect' is the area
    // where we can ignore the mode because the kernel does not reach to the edge of 'contentRect'.
    // The names are derived from the Direction::kX case.
    // TODO: When mode is kMirror or kRepeat it makes more sense to think of contentRect
    // as a tile and figure out the collection of mid/left/right rects that cover dstBounds.
    // Also if 'midRect' is small relative to left/right we should probably issue one draw
    // that implements the mode in the shader rather than break it up in this fashion.
    SkIRect midRect, leftRect, rightRect;
    // 'topRect' and 'bottomRect' are areas of 'dstBounds' that are entirely above/below
    // 'contentRect'. These are areas that we can simply clear in the dst. If 'contentRect'
    // straddles the top edge of 'dstBounds' then 'topRect' will be inverted and we will skip
    // the clear. Similar for 'bottomRect'. The positional/directional labels above refer to the
    // Direction::kX case and one should think of these as 'left' and 'right' for Direction::kY.
    SkIRect topRect, bottomRect;
    int bounds[2];
    if (Direction::kX == direction) {
        bounds[0] = contentRect->left();
        bounds[1] = contentRect->right();
        // Inset to get the portion of 'contentRect' where the kernel doesn't reach the edges.
        midRect = contentRect->makeInset(radius, 0);
        // 'topRect' is the rect of 'dstBounds' above the top of 'contentRect'. It will be inverted
        // if 'contentRect's top is above 'dstBounds'. 'bottomRect' is the area below 'contentRect'
        topRect    = {dstBounds.left(), dstBounds.top()      , dstBounds.right(), contentRect->top()};
        bottomRect = {dstBounds.left(), contentRect->bottom(), dstBounds.right(), dstBounds.bottom()};
        // 'leftRect' and 'rightRect' are the areas of 'dstBounds' intersected with 'contentRect'
        // where the kernel reaches the edge of 'contentRect' and therefore 'mode' must be considered.
        leftRect   = {dstBounds.left(), midRect.top()        , midRect.left()   , midRect.bottom()  };
        rightRect  = {midRect.right(),  midRect.top()        , dstBounds.right(), midRect.bottom()  };
        // The new 'contentRect' will be the area between the clears.
        *contentRect = {dstBounds.left(), topRect.bottom(), dstBounds.right(), bottomRect.top()};
    } else {
        // This is the same as the x direction code if you turn your head 90 degrees ccw. Think
        // 'leftRect' for 'topRect', 'rightRect' for 'bottomRect' and vice-versa.
        bounds[0] = contentRect->top();
        bounds[1] = contentRect->bottom();
        // Inset to get the portion of 'contentRect' where the kernel doesn't reach the edges.
        midRect = contentRect->makeInset(0, radius);
        topRect    = {dstBounds.left(),     dstBounds.top() , contentRect->left(), dstBounds.bottom()};
        bottomRect = {contentRect->right(), dstBounds.top() , dstBounds.right()  , dstBounds.bottom()};
        leftRect   = {midRect.left()      , dstBounds.top() , midRect.right()    , midRect.bottom()  };
        rightRect  = {midRect.left()      , midRect.bottom(), midRect.right()    , dstBounds.bottom()};
        *contentRect = {topRect.right(), dstBounds.top(), bottomRect.left(), dstBounds.bottom()};
    }
    // Move all the rects into 'dstRenderTargetContext' coord system.
    midRect   .offset(-dstToSrcOffset);
    topRect   .offset(-dstToSrcOffset);
    bottomRect.offset(-dstToSrcOffset);
    leftRect  .offset(-dstToSrcOffset);
    rightRect .offset(-dstToSrcOffset);

    contentRect->offset(-dstToSrcOffset);

    if (!topRect.isEmpty()) {
        dstRenderTargetContext->clear(&topRect, SK_PMColor4fTRANSPARENT,
                                      GrRenderTargetContext::CanClearFullscreen::kYes);
    }

    if (!bottomRect.isEmpty()) {
        dstRenderTargetContext->clear(&bottomRect, SK_PMColor4fTRANSPARENT,
                                      GrRenderTargetContext::CanClearFullscreen::kYes);
    }

    if (midRect.isEmpty()) {
        convolve_gaussian_1d(dstRenderTargetContext.get(), std::move(srcProxy), dstToSrcOffset, *contentRect,
                             srcAlphaType, direction, radius, sigma, mode, bounds);
    } else {
        // Draw right and left margins with bounds; middle without.
        convolve_gaussian_1d(dstRenderTargetContext.get(), srcProxy, dstToSrcOffset, leftRect,
                             srcAlphaType, direction, radius, sigma, mode, bounds);
        convolve_gaussian_1d(dstRenderTargetContext.get(), srcProxy, dstToSrcOffset, rightRect,
                             srcAlphaType, direction, radius, sigma, mode, bounds);
        convolve_gaussian_1d(dstRenderTargetContext.get(), std::move(srcProxy), dstToSrcOffset, midRect,
                             srcAlphaType, direction, radius, sigma, SkTileMode::kClamp, nullptr);
    }

    return dstRenderTargetContext;
}

// Returns a high quality scaled-down version of src. This is used to create an intermediate,
// shrunken version of the source image in the event that the requested blur sigma exceeds
// MAX_BLUR_SIGMA.
static sk_sp<GrTextureProxy> decimate(GrRecordingContext* context,
                                      sk_sp<GrTextureProxy> srcProxy,
                                      GrColorType srcColorType,
                                      SkAlphaType srcAlphaType,
                                      SkIPoint srcOffset,
                                      SkIRect* contentRect,
                                      int scaleFactorX,
                                      int scaleFactorY,
                                      SkTileMode mode,
                                      sk_sp<SkColorSpace> finalCS) {
    SkASSERT(SkIsPow2(scaleFactorX) && SkIsPow2(scaleFactorY));
    SkASSERT(scaleFactorX > 1 || scaleFactorY > 1);

    SkIRect srcRect = contentRect->makeOffset(srcOffset);

    scale_irect_roundout(&srcRect, 1.0f / scaleFactorX, 1.0f / scaleFactorY);
    scale_irect(&srcRect, scaleFactorX, scaleFactorY);

    SkIRect dstRect(srcRect);

    std::unique_ptr<GrRenderTargetContext> dstRenderTargetContext;

    for (int i = 1; i < scaleFactorX || i < scaleFactorY; i *= 2) {
        shrink_irect_by_2(&dstRect, i < scaleFactorX, i < scaleFactorY);

        dstRenderTargetContext = GrRenderTargetContext::Make(
                context, srcColorType, finalCS, SkBackingFit::kApprox,
                {dstRect.fRight, dstRect.fBottom}, 1, GrMipMapped::kNo, srcProxy->isProtected(),
                srcProxy->origin());
        if (!dstRenderTargetContext) {
            return nullptr;
        }

        GrPaint paint;
        auto fp = GrTextureEffect::Make(std::move(srcProxy), srcAlphaType, SkMatrix::I(),
                                        GrSamplerState::Filter::kBilerp);
        if (i == 1) {
            // GrDomainEffect does not support kRepeat_Mode with GrSamplerState::Filter.
            GrTextureDomain::Mode domainMode;
            if (mode == SkTileMode::kClamp) {
                domainMode = GrTextureDomain::kClamp_Mode;
            } else {
                // GrDomainEffect does not support k[Mirror]Repeat with GrSamplerState::Filter.
                // So we use decal.
                domainMode = GrTextureDomain::kDecal_Mode;
            }
            SkRect domain = SkRect::Make(*contentRect);
            domain.inset((i < scaleFactorX) ? SK_ScalarHalf + SK_ScalarNearlyZero : 0.0f,
                         (i < scaleFactorY) ? SK_ScalarHalf + SK_ScalarNearlyZero : 0.0f);
            // Ensure that the insetting doesn't invert the domain rectangle.
            if (domain.fRight < domain.fLeft) {
                domain.fLeft = domain.fRight = SkScalarAve(domain.fLeft, domain.fRight);
            }
            if (domain.fBottom < domain.fTop) {
                domain.fTop = domain.fBottom = SkScalarAve(domain.fTop, domain.fBottom);
            }
            fp = GrDomainEffect::Make(std::move(fp), domain, domainMode, true);
            srcRect.offset(-srcOffset);
        }
        paint.addColorFragmentProcessor(std::move(fp));
        paint.setPorterDuffXPFactory(SkBlendMode::kSrc);

        dstRenderTargetContext->fillRectToRect(GrFixedClip::Disabled(), std::move(paint), GrAA::kNo,
                                               SkMatrix::I(), SkRect::Make(dstRect),
                                               SkRect::Make(srcRect));

        srcProxy = dstRenderTargetContext->asTextureProxyRef();
        if (!srcProxy) {
            return nullptr;
        }
        srcRect = dstRect;
    }

    *contentRect = dstRect;

    SkASSERT(dstRenderTargetContext);

    return dstRenderTargetContext->asTextureProxyRef();
}

// Expand the contents of 'srcRenderTargetContext' to fit in 'dstII'. At this point, we are
// expanding an intermediate image, so there's no need to account for a proxy offset from the
// original input.
static std::unique_ptr<GrRenderTargetContext> reexpand(GrRecordingContext* context,
                                                       std::unique_ptr<GrRenderTargetContext> src,
                                                       const SkIRect& srcBounds,
                                                       int scaleFactorX,
                                                       int scaleFactorY,
                                                       SkISize dstSize,
                                                       sk_sp<SkColorSpace> colorSpace,
                                                       SkBackingFit fit) {
    const SkIRect srcRect = SkIRect::MakeWH(src->width(), src->height());

    sk_sp<GrTextureProxy> srcProxy = src->asTextureProxyRef();
    if (!srcProxy) {
        return nullptr;
    }

    GrColorType srcColorType = src->colorInfo().colorType();
    SkAlphaType srcAlphaType = src->colorInfo().alphaType();

    src.reset(); // no longer needed

    auto dstRenderTargetContext = GrRenderTargetContext::Make(
            context, srcColorType, std::move(colorSpace), fit, dstSize, 1, GrMipMapped::kNo,
            srcProxy->isProtected(), srcProxy->origin());
    if (!dstRenderTargetContext) {
        return nullptr;
    }

    GrPaint paint;
    SkRect domain = GrTextureDomain::MakeTexelDomain(srcBounds, GrTextureDomain::kClamp_Mode,
                                                     GrTextureDomain::kClamp_Mode);
    auto fp = GrTextureEffect::Make(std::move(srcProxy), srcAlphaType, SkMatrix::I(),
                                    GrSamplerState::Filter::kBilerp);
    fp = GrDomainEffect::Make(std::move(fp), domain, GrTextureDomain::kClamp_Mode, true);
    paint.addColorFragmentProcessor(std::move(fp));
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
    GrFixedClip clip(SkIRect::MakeSize(dstSize));

    // TODO: using dstII as dstRect results in some image diffs - why?
    SkIRect dstRect(srcRect);
    scale_irect(&dstRect, scaleFactorX, scaleFactorY);

    dstRenderTargetContext->fillRectToRect(clip, std::move(paint), GrAA::kNo, SkMatrix::I(),
                                           SkRect::Make(dstRect), SkRect::Make(srcRect));

    return dstRenderTargetContext;
}

static std::unique_ptr<GrRenderTargetContext> two_pass_gaussian(GrRecordingContext* context,
                                                                sk_sp<GrTextureProxy> srcProxy,
                                                                GrColorType srcColorType,
                                                                SkAlphaType srcAlphaType,
                                                                sk_sp<SkColorSpace> colorSpace,
                                                                SkIRect* srcBounds,
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
                context, std::move(srcProxy), srcColorType, srcAlphaType, srcBounds, dstBounds,
                Direction::kX, radiusX, sigmaX, mode, colorSpace, xFit);
        if (!dstRenderTargetContext) {
            return nullptr;
        }
        srcProxy = dstRenderTargetContext->asTextureProxyRef();
        dstBounds = SkIRect::MakeSize(dstBounds.size());
    }

    if (sigmaY == 0.0f) {
        return dstRenderTargetContext;
    }

    return convolve_gaussian(context, std::move(srcProxy), srcColorType, srcAlphaType, srcBounds, dstBounds,
                             Direction::kY, radiusY, sigmaY, mode, colorSpace,
                             fit);
}

namespace SkGpuBlurUtils {

std::unique_ptr<GrRenderTargetContext> GaussianBlur(GrRecordingContext* context,
                                                    sk_sp<GrTextureProxy> srcProxy,
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

    int scaleFactorX, radiusX;
    int scaleFactorY, radiusY;
    int maxTextureSize = context->priv().caps()->maxTextureSize();
    sigmaX = adjust_sigma(sigmaX, maxTextureSize, &scaleFactorX, &radiusX);
    sigmaY = adjust_sigma(sigmaY, maxTextureSize, &scaleFactorY, &radiusY);
    SkASSERT(sigmaX || sigmaY);

    auto localSrcBounds = srcBounds;

    if (scaleFactorX == 1 && scaleFactorY == 1) {
        // For really small blurs (certainly no wider than 5x5 on desktop GPUs) it is faster to just
        // launch a single non separable kernel vs two launches.
        if (sigmaX > 0 && sigmaY > 0 && (2 * radiusX + 1) * (2 * radiusY + 1) <= MAX_KERNEL_SIZE) {
            // Apply the proxy offset to src bounds and offset directly
            return convolve_gaussian_2d(context, std::move(srcProxy), srcColorType, srcBounds,
                                        dstBounds, radiusX, radiusY, sigmaX, sigmaY, mode,
                                        colorSpace, fit);
        }
        return two_pass_gaussian(context, std::move(srcProxy), srcColorType, srcAlphaType,
                                 std::move(colorSpace), &localSrcBounds, dstBounds, sigmaX,
                                 sigmaY, radiusX, radiusY, mode, fit);
    }

    auto srcOffset = -dstBounds.topLeft();
    srcProxy = decimate(context, std::move(srcProxy), srcColorType, srcAlphaType, srcOffset,
                        &localSrcBounds, scaleFactorX, scaleFactorY, mode, colorSpace);
    if (!srcProxy) {
        return nullptr;
    }
    auto scaledDstBounds = SkIRect::MakeWH(sk_float_ceil(dstBounds.width()  / (float)scaleFactorX),
                                           sk_float_ceil(dstBounds.height() / (float)scaleFactorY));
    auto rtc = two_pass_gaussian(context, std::move(srcProxy), srcColorType, srcAlphaType,
                                 colorSpace, &localSrcBounds, scaledDstBounds, sigmaX, sigmaY,
                                 radiusX, radiusY, mode, SkBackingFit::kApprox);
    if (!rtc) {
        return nullptr;
    }
    return reexpand(context, std::move(rtc), localSrcBounds, scaleFactorX, scaleFactorY,
                    dstBounds.size(), std::move(colorSpace), fit);
}

}

#endif
