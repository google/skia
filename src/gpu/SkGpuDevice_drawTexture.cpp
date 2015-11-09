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

/** Determines how to combine the texture FP with the paint's color and SkShader, if any. */
static const GrFragmentProcessor* mix_texture_fp_with_paint_color_and_shader(
                                                            const GrFragmentProcessor* textureFP,
                                                            bool textureIsAlphaOnly,
                                                            GrContext* context,
                                                            const SkMatrix& viewMatrix,
                                                            const SkPaint& paint) {
    // According to the SkCanvas API, we only consider the shader if the bitmap or image being
    // rendered is alpha-only.
    if (textureIsAlphaOnly) {
        if (const SkShader* shader = paint.getShader()) {
            SkAutoTUnref<const GrFragmentProcessor> shaderFP(
                shader->asFragmentProcessor(context,
                                            viewMatrix,
                                            nullptr,
                                            paint.getFilterQuality()));
            if (!shaderFP) {
                return nullptr;
            }
            const GrFragmentProcessor* fpSeries[] = { shaderFP, textureFP };
            return GrFragmentProcessor::RunInSeries(fpSeries, 2);
        } else {
            return GrFragmentProcessor::MulOutputByInputUnpremulColor(textureFP);
        }
    } else {
        return GrFragmentProcessor::MulOutputByInputAlpha(textureFP);
    }
}

void SkGpuDevice::drawTextureAdjuster(GrTextureAdjuster* adjuster,
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
    SkIRect contentIBounds;
    adjuster->getContentArea(&contentIBounds);
    const SkRect contentBounds = SkRect::Make(contentIBounds);
    SkMatrix srcToDstMatrix;
    if (srcRect) {
        if (!dstRect) {
            dstRect = &contentBounds;
        }
        if (!contentBounds.contains(*srcRect)) {
            clippedSrcRect = *srcRect;
            if (!clippedSrcRect.intersect(contentBounds)) {
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
        clippedSrcRect = contentBounds;
        if (dstRect) {
            clippedDstRect = *dstRect;
            if (!srcToDstMatrix.setRectToRect(contentBounds, *dstRect,
                                              SkMatrix::kFill_ScaleToFit)) {
                return;
            }
        } else {
            clippedDstRect = contentBounds;
            srcToDstMatrix.reset();
        }
    }

    this->drawTextureAdjusterImpl(adjuster, alphaOnly, clippedSrcRect, clippedDstRect, constraint,
                                  viewMatrix, srcToDstMatrix, clip, paint);
}

void SkGpuDevice::drawTextureAdjusterImpl(GrTextureAdjuster* adjuster,
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
    const SkMatrix* textureFPMatrix;
    SkMatrix tempMatrix;
    const SkMaskFilter* mf = paint.getMaskFilter();
    GrTexture* texture = adjuster->originalTexture();
    // The shader expects proper local coords, so we can't replace local coords with texture coords
    // if the shader will be used. If we have a mask filter we will change the underlying geometry
    // that is rendered.
    bool canUseTextureCoordsAsLocalCoords = !use_shader(alphaTexture, paint) && !mf;
    if (canUseTextureCoordsAsLocalCoords) {
        textureFPMatrix = &SkMatrix::I();
    } else {
        if (!srcToDstMatrix.invert(&tempMatrix)) {
            return;
        }
        tempMatrix.postIDiv(texture->width(), texture->height());
        textureFPMatrix = &tempMatrix;
    }

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

    SkAutoTUnref<const GrFragmentProcessor> fp(adjuster->createFragmentProcessor(
        *textureFPMatrix, clippedSrcRect, constraintMode, coordsAllInsideSrcRect, filterMode));
    if (!fp) {
        return;
    }
    fp.reset(mix_texture_fp_with_paint_color_and_shader(fp, alphaTexture, this->context(),
                                                        viewMatrix, paint));
    GrPaint grPaint;
    if (!SkPaintToGrPaintReplaceShader(fContext, paint, fp, &grPaint)) {
        return;
    }

    if (canUseTextureCoordsAsLocalCoords) {
        SkRect localRect;
        localRect.fLeft = clippedSrcRect.fLeft / texture->width();
        localRect.fBottom = clippedSrcRect.fBottom / texture->height();
        localRect.fRight = clippedSrcRect.fRight / texture->width();
        localRect.fTop = clippedSrcRect.fTop / texture->height();
        fDrawContext->fillRectToRect(clip, grPaint, viewMatrix, clippedDstRect, localRect);
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
    GrBlurUtils::drawPathWithMaskFilter(this->context(), fDrawContext, fRenderTarget, fClip,
                                        rectPath, &grPaint, viewMatrix, mf, paint.getPathEffect(),
                                        GrStrokeInfo::FillInfo());
}
