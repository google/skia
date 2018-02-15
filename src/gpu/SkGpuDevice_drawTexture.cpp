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
 * Checks whether the paint, matrix, and constraint are compatible with using
 * GrRenderTargetContext::drawTextureAffine. It is more effecient than the GrTextureProducer
 * general case.
 */
static bool can_use_draw_texture_affine(const SkPaint& paint, GrAA aa, const SkMatrix& ctm,
                                        SkCanvas::SrcRectConstraint constraint) {
// This is disabled in Chrome until crbug.com/802408 and crbug.com/801783 can be sorted out.
#ifdef SK_DISABLE_TEXTURE_OP_AA
    if (GrAA::kYes == aa) {
        return false;
    }
#endif
    return (!paint.getColorFilter() && !paint.getShader() && !paint.getMaskFilter() &&
            !paint.getImageFilter() && paint.getFilterQuality() < kMedium_SkFilterQuality &&
            paint.getBlendMode() == SkBlendMode::kSrcOver && !ctm.hasPerspective() &&
            SkCanvas::kFast_SrcRectConstraint == constraint);
}

static void draw_texture_affine(const SkPaint& paint, const SkMatrix& ctm, const SkRect* src,
                                const SkRect* dst, GrAA aa, sk_sp<GrTextureProxy> proxy,
                                SkColorSpace* colorSpace, const GrClip& clip,
                                GrRenderTargetContext* rtc) {
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
    auto csxf = GrColorSpaceXform::Make(colorSpace, proxy->config(),
                                        rtc->colorSpaceInfo().colorSpace());
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
    GrColor color = GrPixelConfigIsAlphaOnly(proxy->config())
                            ? SkColorToPremulGrColor(paint.getColor())
                            : SkColorAlphaToGrColor(paint.getColor());
    rtc->drawTextureAffine(clip, std::move(proxy), filter, color, srcRect, dstRect, aa, ctm,
                           std::move(csxf));
}

//////////////////////////////////////////////////////////////////////////////

void SkGpuDevice::drawPinnedTextureProxy(sk_sp<GrTextureProxy> proxy, uint32_t pinnedUniqueID,
                                         SkColorSpace* colorSpace, SkAlphaType alphaType,
                                         const SkRect* srcRect, const SkRect* dstRect,
                                         SkCanvas::SrcRectConstraint constraint,
                                         const SkMatrix& viewMatrix, const SkPaint& paint) {
    GrAA aa = GrAA(paint.isAntiAlias());
    if (can_use_draw_texture_affine(paint, aa, this->ctm(), constraint)) {
        draw_texture_affine(paint, viewMatrix, srcRect, dstRect, aa, std::move(proxy), colorSpace,
                            this->clip(), fRenderTargetContext.get());
        return;
    }
    GrTextureAdjuster adjuster(this->context(), std::move(proxy), alphaType, pinnedUniqueID,
                               colorSpace);
    this->drawTextureProducer(&adjuster, srcRect, dstRect, constraint, viewMatrix, paint);
}

void SkGpuDevice::drawTextureMaker(GrTextureMaker* maker, int imageW, int imageH,
                                   const SkRect* srcRect, const SkRect* dstRect,
                                   SkCanvas::SrcRectConstraint constraint,
                                   const SkMatrix& viewMatrix, const SkPaint& paint) {
    GrAA aa = GrAA(paint.isAntiAlias());
    if (can_use_draw_texture_affine(paint, aa, viewMatrix, constraint)) {
        sk_sp<SkColorSpace> cs;
        // We've done enough checks above to allow us to pass ClampNearest() and not check for
        // scaling adjustments.
        auto proxy = maker->refTextureProxyForParams(
                GrSamplerState::ClampNearest(), fRenderTargetContext->colorSpaceInfo().colorSpace(),
                &cs, nullptr);
        if (!proxy) {
            return;
        }
        draw_texture_affine(paint, viewMatrix, srcRect, dstRect, aa, std::move(proxy), cs.get(),
                            this->clip(), fRenderTargetContext.get());
        return;
    }
    this->drawTextureProducer(maker, srcRect, dstRect, constraint, viewMatrix, paint);
}

void SkGpuDevice::drawTextureProducer(GrTextureProducer* producer,
                                      const SkRect* srcRect,
                                      const SkRect* dstRect,
                                      SkCanvas::SrcRectConstraint constraint,
                                      const SkMatrix& viewMatrix,
                                      const SkPaint& paint) {
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
    LogDrawScaleFactor(SkMatrix::Concat(viewMatrix, srcToDstMatrix), paint.getFilterQuality());

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
    // Specifying the texture coords as local coordinates is an attempt to enable more GrDrawOp
    // combining by not baking anything about the srcRect, dstRect, or viewMatrix, into the texture
    // FP. In the future this should be an opaque optimization enabled by the combination of
    // GrDrawOp/GP and FP.
    const SkMaskFilter* mf = paint.getMaskFilter();
    if (mf && as_MFB(mf)->hasFragmentProcessor()) {
        mf = nullptr;
    }
    // The shader expects proper local coords, so we can't replace local coords with texture coords
    // if the shader will be used. If we have a mask filter we will change the underlying geometry
    // that is rendered.
    bool canUseTextureCoordsAsLocalCoords = !use_shader(producer->isAlphaOnly(), paint) && !mf;

    bool doBicubic;
    GrSamplerState::Filter fm = GrSkFilterQualityToGrFilterMode(
            paint.getFilterQuality(), viewMatrix, srcToDstMatrix,
            fContext->contextPriv().sharpenMipmappedTextures(), &doBicubic);
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
    auto fp = producer->createFragmentProcessor(
            *textureMatrix, clippedSrcRect, constraintMode, coordsAllInsideSrcRect, filterMode,
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

    // First see if we can do the draw + mask filter direct to the dst.
    if (viewMatrix.isScaleTranslate()) {
        SkRect devClippedDstRect;
        viewMatrix.mapRectScaleTranslate(&devClippedDstRect, clippedDstRect);

        SkStrokeRec rec(SkStrokeRec::kFill_InitStyle);
        if (as_MFB(mf)->directFilterRRectMaskGPU(fContext.get(),
                                                 fRenderTargetContext.get(),
                                                 std::move(grPaint),
                                                 this->clip(),
                                                 viewMatrix,
                                                 rec,
                                                 SkRRect::MakeRect(clippedDstRect),
                                                 SkRRect::MakeRect(devClippedDstRect))) {
            return;
        }
    }

    SkPath rectPath;
    rectPath.addRect(clippedDstRect);
    rectPath.setIsVolatile(true);
    GrBlurUtils::drawPathWithMaskFilter(this->context(), fRenderTargetContext.get(), this->clip(),
                                        rectPath, std::move(grPaint), aa, viewMatrix, mf,
                                        GrStyle::SimpleFill(), true);
}
