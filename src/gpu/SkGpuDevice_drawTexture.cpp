/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGpuDevice.h"

#include "GrBlurUtils.h"
#include "GrCaps.h"
#include "GrDrawContext.h"
#include "GrStrokeInfo.h"
#include "GrTextureParamsAdjuster.h"
#include "SkDraw.h"
#include "SkGrPriv.h"
#include "SkMaskFilter.h"
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
                            bool isMSAA) {
    // Only gets called if has_aligned_samples returned false.
    // So we can assume that sampling is axis aligned but not texel aligned.
    SkASSERT(!has_aligned_samples(srcRect, transformedRect));
    SkRect innerSrcRect(srcRect), innerTransformedRect, outerTransformedRect(transformedRect);
    if (isMSAA) {
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
                                         bool isMSAA) {
    if (srcRectToDeviceSpace.rectStaysRect()) {
        // sampling is axis-aligned
        SkRect transformedRect;
        srcRectToDeviceSpace.mapRect(&transformedRect, srcRect);

        if (has_aligned_samples(srcRect, transformedRect) ||
            !may_color_bleed(srcRect, transformedRect, srcRectToDeviceSpace, isMSAA)) {
            return true;
        }
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////////

void SkGpuDevice::drawTextureProducer(GrTextureProducer* producer,
                                      bool alphaOnly,
                                      const SkRect* srcRect,
                                      const SkRect* dstRect,
                                      SkCanvas::SrcRectConstraint constraint,
                                      const SkMatrix& viewMatrix,
                                      const GrClip& clip,
                                      const SkPaint& paint) {
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

    this->drawTextureProducerImpl(producer, alphaOnly, clippedSrcRect, clippedDstRect, constraint,
                                  viewMatrix, srcToDstMatrix, clip, paint);
}

void SkGpuDevice::drawTextureProducerImpl(GrTextureProducer* producer,
                                          bool alphaTexture,
                                          const SkRect& clippedSrcRect,
                                          const SkRect& clippedDstRect,
                                          SkCanvas::SrcRectConstraint constraint,
                                          const SkMatrix& viewMatrix,
                                          const SkMatrix& srcToDstMatrix,
                                          const GrClip& clip,
                                          const SkPaint& paint) {
    // Specifying the texture coords as local coordinates is an attempt to enable more batching
    // by not baking anything about the srcRect, dstRect, or viewMatrix, into the texture FP. In
    // the future this should be an opaque optimization enabled by the combination of batch/GP and
    // FP.
    const SkMaskFilter* mf = paint.getMaskFilter();
    // The shader expects proper local coords, so we can't replace local coords with texture coords
    // if the shader will be used. If we have a mask filter we will change the underlying geometry
    // that is rendered.
    bool canUseTextureCoordsAsLocalCoords = !use_shader(alphaTexture, paint) && !mf;

    bool doBicubic;
    GrTextureParams::FilterMode fm =
        GrSkFilterQualityToGrFilterMode(paint.getFilterQuality(), viewMatrix, srcToDstMatrix,
                                        &doBicubic);
    const GrTextureParams::FilterMode* filterMode = doBicubic ? nullptr : &fm;

    GrTextureAdjuster::FilterConstraint constraintMode;
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
    if (filterMode && GrTextureParams::kBilerp_FilterMode == *filterMode &&
        GrTextureAdjuster::kYes_FilterConstraint == constraintMode && coordsAllInsideSrcRect) {
        SkMatrix combinedMatrix;
        combinedMatrix.setConcat(viewMatrix, srcToDstMatrix);
        if (can_ignore_bilerp_constraint(*producer, clippedSrcRect, combinedMatrix,
                                         fRenderTarget->isUnifiedMultisampled())) {
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
    SkAutoTUnref<const GrFragmentProcessor> fp(producer->createFragmentProcessor(
        *textureMatrix, clippedSrcRect, constraintMode, coordsAllInsideSrcRect, filterMode));
    if (!fp) {
        return;
    }

    GrPaint grPaint;
    if (!SkPaintToGrPaintWithTexture(fContext, paint, viewMatrix, fp, alphaTexture, &grPaint)) {
        return;
    }

    if (canUseTextureCoordsAsLocalCoords) {
        fDrawContext->fillRectToRect(clip, grPaint, viewMatrix, clippedDstRect, clippedSrcRect);
        return;
    }

    if (!mf) {
        fDrawContext->drawRect(clip, grPaint, viewMatrix, clippedDstRect);
        return;
    }

    // First see if we can do the draw + mask filter direct to the dst.
    SkStrokeRec rec(SkStrokeRec::kFill_InitStyle);
    SkRRect rrect;
    rrect.setRect(clippedDstRect);
    if (mf->directFilterRRectMaskGPU(fContext->textureProvider(),
                                      fDrawContext,
                                      &grPaint,
                                      clip,
                                      viewMatrix,
                                      rec,
                                      rrect)) {
        return;
    }
    SkPath rectPath;
    rectPath.addRect(clippedDstRect);
    rectPath.setIsVolatile(true);
    GrBlurUtils::drawPathWithMaskFilter(this->context(), fDrawContext, fClip,
                                        rectPath, &grPaint, viewMatrix, mf, paint.getPathEffect(),
                                        GrStrokeInfo::FillInfo(), true);
}
