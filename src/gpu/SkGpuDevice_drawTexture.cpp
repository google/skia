/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGpuDevice.h"
#include "GrBlurUtils.h"
#include "GrCaps.h"
#include "GrColorSpaceXform.h"
#include "GrRenderTargetContext.h"
#include "GrShape.h"
#include "GrStyle.h"
#include "GrTextureAdjuster.h"
#include "GrTextureMaker.h"
#include "SkDraw.h"
#include "SkGr.h"
#include "SkMaskFilterBase.h"
#include "effects/GrBicubicEffect.h"
#include "effects/GrSimpleTextureEffect.h"
#include "effects/GrTextureDomain.h"

static inline bool use_shader(bool textureIsAlphaOnly, const SkPaint& paint) {
    return textureIsAlphaOnly && paint.getShader();
}

//////////////////////////////////////////////////////////////////////////////
//  Helper functions for dropping src rect constraint in bilerp mode.

static const SkScalar kColorBleedTolerance = 0.001f;

static bool has_aligned_samples(const SkRect& srcRect, const SkRect& transformedRect) {
    // detect pixel disalignment
    if (SkScalarAbs(SkScalarRoundToScalar(transformedRect.left()) - transformedRect.left()) < kColorBleedTolerance &&
        SkScalarAbs(SkScalarRoundToScalar(transformedRect.top())  - transformedRect.top())  < kColorBleedTolerance &&
        SkScalarAbs(transformedRect.width()  - srcRect.width())  < kColorBleedTolerance &&
        SkScalarAbs(transformedRect.height() - srcRect.height()) < kColorBleedTolerance) {
        return true;
    }
    return false;
}

static bool may_color_bleed(const SkRect& srcRect,
                            const SkRect& transformedRect,
                            const SkMatrix& m,
                            GrFSAAType fsaaType) {
    // Only gets called if has_aligned_samples returned false.
    // So we can assume that sampling is axis aligned but not texel aligned.
    SkASSERT(!has_aligned_samples(srcRect, transformedRect));
    SkRect innerSrcRect(srcRect), innerTransformedRect, outerTransformedRect(transformedRect);
    if (GrFSAAType::kUnifiedMSAA == fsaaType) {
        innerSrcRect.inset(SK_Scalar1, SK_Scalar1);
    } else {
        innerSrcRect.inset(SK_ScalarHalf, SK_ScalarHalf);
    }
    m.mapRect(&innerTransformedRect, innerSrcRect);

    // The gap between outerTransformedRect and innerTransformedRect
    // represents the projection of the source border area, which is
    // problematic for color bleeding.  We must check whether any
    // destination pixels sample the border area.
    outerTransformedRect.inset(kColorBleedTolerance, kColorBleedTolerance);
    innerTransformedRect.outset(kColorBleedTolerance, kColorBleedTolerance);
    SkIRect outer, inner;
    outerTransformedRect.round(&outer);
    innerTransformedRect.round(&inner);
    // If the inner and outer rects round to the same result, it means the
    // border does not overlap any pixel centers. Yay!
    return inner != outer;
}

static bool can_ignore_bilerp_constraint(const GrTextureProducer& producer,
                                         const SkRect& srcRect,
                                         const SkMatrix& srcRectToDeviceSpace,
                                         GrFSAAType fsaaType) {
    if (srcRectToDeviceSpace.rectStaysRect()) {
        // sampling is axis-aligned
        SkRect transformedRect;
        srcRectToDeviceSpace.mapRect(&transformedRect, srcRect);

        if (has_aligned_samples(srcRect, transformedRect) ||
            !may_color_bleed(srcRect, transformedRect, srcRectToDeviceSpace, fsaaType)) {
            return true;
        }
    }
    return false;
}

/**
 * Checks whether the paint is compatible with using GrRenderTargetContext::drawTexture. It is more
 * efficient than the GrTextureProducer general case.
 */
static bool can_use_draw_texture(const SkPaint& paint) {
    return (!paint.getColorFilter() && !paint.getShader() && !paint.getMaskFilter() &&
            !paint.getImageFilter() && paint.getFilterQuality() < kMedium_SkFilterQuality);
}

static void draw_texture(const SkPaint& paint, const SkMatrix& ctm, const SkRect* src,
                         const SkRect* dst, GrAA aa, SkCanvas::SrcRectConstraint constraint,
                         sk_sp<GrTextureProxy> proxy, SkAlphaType alphaType,
                         SkColorSpace* colorSpace, const GrClip& clip, GrRenderTargetContext* rtc) {
    SkASSERT(!(SkToBool(src) && !SkToBool(dst)));
    SkRect srcRect = src ? *src : SkRect::MakeWH(proxy->width(), proxy->height());
    SkRect dstRect = dst ? *dst : srcRect;
    if (src && !SkRect::MakeIWH(proxy->width(), proxy->height()).contains(srcRect)) {
        // Shrink the src rect to be within bounds and proportionately shrink the dst rect.
        SkMatrix srcToDst;
        srcToDst.setRectToRect(srcRect, dstRect, SkMatrix::kFill_ScaleToFit);
        SkAssertResult(srcRect.intersect(SkRect::MakeIWH(proxy->width(), proxy->height())));
        srcToDst.mapRect(&dstRect, srcRect);
    }
    const GrColorSpaceInfo& dstInfo(rtc->colorSpaceInfo());
    auto textureXform =
        GrColorSpaceXform::Make(colorSpace          , alphaType,
                                dstInfo.colorSpace(), kPremul_SkAlphaType);
    GrSamplerState::Filter filter;
    switch (paint.getFilterQuality()) {
        case kNone_SkFilterQuality:
            filter = GrSamplerState::Filter::kNearest;
            break;
        case kLow_SkFilterQuality:
            filter = GrSamplerState::Filter::kBilerp;
            break;
        case kMedium_SkFilterQuality:
        case kHigh_SkFilterQuality:
            SK_ABORT("Quality level not allowed.");
    }

    // Must specify the strict constraint when the proxy is not functionally exact and the src
    // rect would access pixels outside the proxy's content area without the constraint.
    if (constraint != SkCanvas::kStrict_SrcRectConstraint &&
        !GrProxyProvider::IsFunctionallyExact(proxy.get())) {
        // Conservative estimate of how much a coord could be outset from src rect:
        // 1/2 pixel for AA and 1/2 pixel for bilerp
        float buffer = 0.5f * (aa == GrAA::kYes) +
                       0.5f * (filter == GrSamplerState::Filter::kBilerp);
        SkRect safeBounds = SkRect::MakeWH(proxy->width(), proxy->height());
        safeBounds.inset(buffer, buffer);
        if (!safeBounds.contains(srcRect)) {
            constraint = SkCanvas::kStrict_SrcRectConstraint;
        }
    }
    SkPMColor4f color;
    if (GrPixelConfigIsAlphaOnly(proxy->config())) {
        color = SkColor4fPrepForDst(paint.getColor4f(), dstInfo, *rtc->caps()).premul();
    } else {
        float paintAlpha = paint.getColor4f().fA;
        color = { paintAlpha, paintAlpha, paintAlpha, paintAlpha };
    }
    GrQuadAAFlags aaFlags = aa == GrAA::kYes ? GrQuadAAFlags::kAll : GrQuadAAFlags::kNone;
    rtc->drawTexture(clip, std::move(proxy), filter, paint.getBlendMode(), color, srcRect, dstRect,
                     aa, aaFlags, constraint, ctm, std::move(textureXform));
}

//////////////////////////////////////////////////////////////////////////////

void SkGpuDevice::drawPinnedTextureProxy(sk_sp<GrTextureProxy> proxy, uint32_t pinnedUniqueID,
                                         SkColorSpace* colorSpace, SkAlphaType alphaType,
                                         const SkRect* srcRect, const SkRect* dstRect,
                                         SkCanvas::SrcRectConstraint constraint,
                                         const SkMatrix& viewMatrix, const SkPaint& paint) {
    GrAA aa = GrAA(paint.isAntiAlias());
    if (can_use_draw_texture(paint)) {
        draw_texture(paint, viewMatrix, srcRect, dstRect, aa, constraint, std::move(proxy),
                     alphaType, colorSpace, this->clip(), fRenderTargetContext.get());
        return;
    }
    GrTextureAdjuster adjuster(this->context(), std::move(proxy), alphaType, pinnedUniqueID,
                               colorSpace);
    this->drawTextureProducer(&adjuster, srcRect, dstRect, constraint, viewMatrix, paint, false);
}

void SkGpuDevice::drawTextureProducer(GrTextureProducer* producer,
                                      const SkRect* srcRect,
                                      const SkRect* dstRect,
                                      SkCanvas::SrcRectConstraint constraint,
                                      const SkMatrix& viewMatrix,
                                      const SkPaint& paint,
                                      bool attemptDrawTexture) {
    if (attemptDrawTexture && can_use_draw_texture(paint)) {
        GrAA aa = GrAA(paint.isAntiAlias());
        // We've done enough checks above to allow us to pass ClampNearest() and not check for
        // scaling adjustments.
        auto proxy = producer->refTextureProxyForParams(GrSamplerState::ClampNearest(), nullptr);
        if (!proxy) {
            return;
        }
        draw_texture(paint, viewMatrix, srcRect, dstRect, aa, constraint, std::move(proxy),
                     producer->alphaType(), producer->colorSpace(), this->clip(),
                     fRenderTargetContext.get());
        return;
    }

    // This is the funnel for all non-tiled bitmap/image draw calls. Log a histogram entry.
    SK_HISTOGRAM_BOOLEAN("DrawTiled", false);

    // Figure out the actual dst and src rect by clipping the src rect to the bounds of the
    // adjuster. If the src rect is clipped then the dst rect must be recomputed. Also determine
    // the matrix that maps the src rect to the dst rect.
    SkRect clippedSrcRect;
    SkRect clippedDstRect;
    const SkRect srcBounds = SkRect::MakeIWH(producer->width(), producer->height());
    SkMatrix srcToDstMatrix;
    if (srcRect) {
        if (!dstRect) {
            dstRect = &srcBounds;
        }
        if (!srcBounds.contains(*srcRect)) {
            clippedSrcRect = *srcRect;
            if (!clippedSrcRect.intersect(srcBounds)) {
                return;
            }
            if (!srcToDstMatrix.setRectToRect(*srcRect, *dstRect, SkMatrix::kFill_ScaleToFit)) {
                return;
            }
            srcToDstMatrix.mapRect(&clippedDstRect, clippedSrcRect);
        } else {
            clippedSrcRect = *srcRect;
            clippedDstRect = *dstRect;
            if (!srcToDstMatrix.setRectToRect(*srcRect, *dstRect, SkMatrix::kFill_ScaleToFit)) {
                return;
            }
        }
    } else {
        clippedSrcRect = srcBounds;
        if (dstRect) {
            clippedDstRect = *dstRect;
            if (!srcToDstMatrix.setRectToRect(srcBounds, *dstRect, SkMatrix::kFill_ScaleToFit)) {
                return;
            }
        } else {
            clippedDstRect = srcBounds;
            srcToDstMatrix.reset();
        }
    }

    // Now that we have both the view and srcToDst matrices, log our scale factor.
    LogDrawScaleFactor(viewMatrix, srcToDstMatrix, paint.getFilterQuality());

    this->drawTextureProducerImpl(producer, clippedSrcRect, clippedDstRect, constraint, viewMatrix,
                                  srcToDstMatrix, paint);
}

void SkGpuDevice::drawTextureProducerImpl(GrTextureProducer* producer,
                                          const SkRect& clippedSrcRect,
                                          const SkRect& clippedDstRect,
                                          SkCanvas::SrcRectConstraint constraint,
                                          const SkMatrix& viewMatrix,
                                          const SkMatrix& srcToDstMatrix,
                                          const SkPaint& paint) {
    const SkMaskFilter* mf = paint.getMaskFilter();

    // The shader expects proper local coords, so we can't replace local coords with texture coords
    // if the shader will be used. If we have a mask filter we will change the underlying geometry
    // that is rendered.
    bool canUseTextureCoordsAsLocalCoords = !use_shader(producer->isAlphaOnly(), paint) && !mf;

    // Specifying the texture coords as local coordinates is an attempt to enable more GrDrawOp
    // combining by not baking anything about the srcRect, dstRect, or viewMatrix, into the texture
    // FP. In the future this should be an opaque optimization enabled by the combination of
    // GrDrawOp/GP and FP.
    if (mf && as_MFB(mf)->hasFragmentProcessor()) {
        mf = nullptr;
    }

    bool doBicubic;
    GrSamplerState::Filter fm = GrSkFilterQualityToGrFilterMode(
            paint.getFilterQuality(), viewMatrix, srcToDstMatrix,
            fContext->priv().options().fSharpenMipmappedTextures, &doBicubic);
    const GrSamplerState::Filter* filterMode = doBicubic ? nullptr : &fm;

    GrTextureProducer::FilterConstraint constraintMode;
    if (SkCanvas::kFast_SrcRectConstraint == constraint) {
        constraintMode = GrTextureAdjuster::kNo_FilterConstraint;
    } else {
        constraintMode = GrTextureAdjuster::kYes_FilterConstraint;
    }

    // If we have to outset for AA then we will generate texture coords outside the src rect. The
    // same happens for any mask filter that extends the bounds rendered in the dst.
    // This is conservative as a mask filter does not have to expand the bounds rendered.
    bool coordsAllInsideSrcRect = !paint.isAntiAlias() && !mf;

    // Check for optimization to drop the src rect constraint when on bilerp.
    if (filterMode && GrSamplerState::Filter::kBilerp == *filterMode &&
        GrTextureAdjuster::kYes_FilterConstraint == constraintMode && coordsAllInsideSrcRect) {
        SkMatrix combinedMatrix;
        combinedMatrix.setConcat(viewMatrix, srcToDstMatrix);
        if (can_ignore_bilerp_constraint(*producer, clippedSrcRect, combinedMatrix,
                                         fRenderTargetContext->fsaaType())) {
            constraintMode = GrTextureAdjuster::kNo_FilterConstraint;
        }
    }

    const SkMatrix* textureMatrix;
    SkMatrix tempMatrix;
    if (canUseTextureCoordsAsLocalCoords) {
        textureMatrix = &SkMatrix::I();
    } else {
        if (!srcToDstMatrix.invert(&tempMatrix)) {
            return;
        }
        textureMatrix = &tempMatrix;
    }
    auto fp = producer->createFragmentProcessor(*textureMatrix, clippedSrcRect, constraintMode,
                                                coordsAllInsideSrcRect, filterMode);
    fp = GrColorSpaceXformEffect::Make(std::move(fp), producer->colorSpace(), producer->alphaType(),
                                       fRenderTargetContext->colorSpaceInfo().colorSpace());
    if (!fp) {
        return;
    }

    GrPaint grPaint;
    if (!SkPaintToGrPaintWithTexture(fContext.get(), fRenderTargetContext->colorSpaceInfo(), paint,
                                     viewMatrix, std::move(fp), producer->isAlphaOnly(),
                                     &grPaint)) {
        return;
    }
    GrAA aa = GrAA(paint.isAntiAlias());
    if (canUseTextureCoordsAsLocalCoords) {
        fRenderTargetContext->fillRectToRect(this->clip(), std::move(grPaint), aa, viewMatrix,
                                             clippedDstRect, clippedSrcRect);
        return;
    }

    if (!mf) {
        fRenderTargetContext->drawRect(this->clip(), std::move(grPaint), aa, viewMatrix,
                                       clippedDstRect);
        return;
    }

    GrShape shape(clippedDstRect, GrStyle::SimpleFill());

    GrBlurUtils::drawShapeWithMaskFilter(this->context(), fRenderTargetContext.get(), this->clip(),
                                         shape, std::move(grPaint), viewMatrix, mf);
}
