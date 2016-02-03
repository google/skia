/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBlurUtils.h"
#include "GrDrawContext.h"
#include "GrCaps.h"
#include "GrContext.h"
#include "effects/GrSimpleTextureEffect.h"
#include "GrStrokeInfo.h"
#include "GrTexture.h"
#include "GrTextureProvider.h"
#include "SkDraw.h"
#include "SkGrPriv.h"
#include "SkMaskFilter.h"
#include "SkPaint.h"

static bool clip_bounds_quick_reject(const SkIRect& clipBounds, const SkIRect& rect) {
    return clipBounds.isEmpty() || rect.isEmpty() || !SkIRect::Intersects(clipBounds, rect);
}

// Draw a mask using the supplied paint. Since the coverage/geometry
// is already burnt into the mask this boils down to a rect draw.
// Return true if the mask was successfully drawn.
static bool draw_mask(GrDrawContext* drawContext,
                      const GrClip& clip,
                      const SkMatrix& viewMatrix,
                      const SkRect& maskRect,
                      GrPaint* grp,
                      GrTexture* mask) {
    SkMatrix matrix;
    matrix.setTranslate(-maskRect.fLeft, -maskRect.fTop);
    matrix.postIDiv(mask->width(), mask->height());

    grp->addCoverageFragmentProcessor(GrSimpleTextureEffect::Create(mask, matrix,
                                                                    kDevice_GrCoordSet))->unref();

    SkMatrix inverse;
    if (!viewMatrix.invert(&inverse)) {
        return false;
    }
    drawContext->fillRectWithLocalMatrix(clip, *grp, SkMatrix::I(), maskRect, inverse);
    return true;
}

static bool sw_draw_with_mask_filter(GrDrawContext* drawContext,
                                     GrTextureProvider* textureProvider,
                                     const GrClip& clipData,
                                     const SkMatrix& viewMatrix,
                                     const SkPath& devPath,
                                     const SkMaskFilter* filter,
                                     const SkIRect& clipBounds,
                                     GrPaint* grp,
                                     SkPaint::Style style) {
    SkMask  srcM, dstM;

    if (!SkDraw::DrawToMask(devPath, &clipBounds, filter, &viewMatrix, &srcM,
                            SkMask::kComputeBoundsAndRenderImage_CreateMode, style)) {
        return false;
    }
    SkAutoMaskFreeImage autoSrc(srcM.fImage);

    if (!filter->filterMask(&dstM, srcM, viewMatrix, nullptr)) {
        return false;
    }
    // this will free-up dstM when we're done (allocated in filterMask())
    SkAutoMaskFreeImage autoDst(dstM.fImage);

    if (clip_bounds_quick_reject(clipBounds, dstM.fBounds)) {
        return false;
    }

    // we now have a device-aligned 8bit mask in dstM, ready to be drawn using
    // the current clip (and identity matrix) and GrPaint settings
    GrSurfaceDesc desc;
    desc.fWidth = dstM.fBounds.width();
    desc.fHeight = dstM.fBounds.height();
    desc.fConfig = kAlpha_8_GrPixelConfig;

    SkAutoTUnref<GrTexture> texture(textureProvider->createApproxTexture(desc));
    if (!texture) {
        return false;
    }
    texture->writePixels(0, 0, desc.fWidth, desc.fHeight, desc.fConfig,
                               dstM.fImage, dstM.fRowBytes);

    SkRect maskRect = SkRect::Make(dstM.fBounds);

    return draw_mask(drawContext, clipData, viewMatrix, maskRect, grp, texture);
}

// Create a mask of 'devPath' and place the result in 'mask'.
static GrTexture* create_mask_GPU(GrContext* context,
                                  SkRect* maskRect,
                                  const SkPath& devPath,
                                  const GrStrokeInfo& strokeInfo,
                                  bool doAA,
                                  int sampleCnt) {
    // This mask will ultimately be drawn as a non-AA rect (see draw_mask). 
    // Non-AA rects have a bad habit of snapping arbitrarily. Integerize here 
    // so the mask draws in a reproducible manner.
    *maskRect = SkRect::Make(maskRect->roundOut());

    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = SkScalarCeilToInt(maskRect->width());
    desc.fHeight = SkScalarCeilToInt(maskRect->height());
    desc.fSampleCnt = doAA ? sampleCnt : 0;
    // We actually only need A8, but it often isn't supported as a
    // render target so default to RGBA_8888
    desc.fConfig = kRGBA_8888_GrPixelConfig;

    if (context->caps()->isConfigRenderable(kAlpha_8_GrPixelConfig, desc.fSampleCnt > 0)) {
        desc.fConfig = kAlpha_8_GrPixelConfig;
    }

    GrTexture* mask = context->textureProvider()->createApproxTexture(desc);
    if (nullptr == mask) {
        return nullptr;
    }

    SkRect clipRect = SkRect::MakeWH(maskRect->width(), maskRect->height());

    SkAutoTUnref<GrDrawContext> drawContext(context->drawContext(mask->asRenderTarget()));
    if (!drawContext) {
        return nullptr;
    }

    drawContext->clear(nullptr, 0x0, true);

    GrPaint tempPaint;
    tempPaint.setAntiAlias(doAA);
    tempPaint.setCoverageSetOpXPFactory(SkRegion::kReplace_Op);

    // setup new clip
    GrClip clip(clipRect);

    // Draw the mask into maskTexture with the path's integerized top-left at
    // the origin using tempPaint.
    SkMatrix translate;
    translate.setTranslate(-maskRect->fLeft, -maskRect->fTop);
    drawContext->drawPath(clip, tempPaint, translate, devPath, strokeInfo);
    return mask;
}

static void draw_path_with_mask_filter(GrContext* context,
                                       GrDrawContext* drawContext,
                                       const GrClip& clip,
                                       GrPaint* paint,
                                       const SkMatrix& viewMatrix,
                                       const SkMaskFilter* maskFilter,
                                       const SkPathEffect* pathEffect,
                                       const GrStrokeInfo& origStrokeInfo,
                                       SkPath* pathPtr,
                                       bool pathIsMutable) {
    SkASSERT(maskFilter);

    SkIRect clipBounds;
    clip.getConservativeBounds(drawContext->width(), drawContext->height(), &clipBounds);
    SkTLazy<SkPath> tmpPath;
    GrStrokeInfo strokeInfo(origStrokeInfo);

    static const SkRect* cullRect = nullptr;  // TODO: what is our bounds?

    SkASSERT(strokeInfo.isDashed() || !pathEffect);

    if (!strokeInfo.isHairlineStyle()) {
        SkPath* strokedPath = pathIsMutable ? pathPtr : tmpPath.init();
        if (strokeInfo.isDashed()) {
            if (pathEffect->filterPath(strokedPath, *pathPtr, &strokeInfo, cullRect)) {
                pathPtr = strokedPath;
                pathPtr->setIsVolatile(true);
                pathIsMutable = true;
            }
            strokeInfo.removeDash();
        }
        if (strokeInfo.applyToPath(strokedPath, *pathPtr)) {
            // Apply the stroke to the path if there is one
            pathPtr = strokedPath;
            pathPtr->setIsVolatile(true);
            pathIsMutable = true;
            strokeInfo.setFillStyle();
        }
    }

    // avoid possibly allocating a new path in transform if we can
    SkPath* devPathPtr = pathIsMutable ? pathPtr : tmpPath.init();
    if (!pathIsMutable) {
        devPathPtr->setIsVolatile(true);
    }

    // transform the path into device space
    pathPtr->transform(viewMatrix, devPathPtr);

    SkRect maskRect;
    if (maskFilter->canFilterMaskGPU(SkRRect::MakeRect(devPathPtr->getBounds()),
                                     clipBounds,
                                     viewMatrix,
                                     &maskRect)) {
        SkIRect finalIRect;
        maskRect.roundOut(&finalIRect);
        if (clip_bounds_quick_reject(clipBounds, finalIRect)) {
            // clipped out
            return;
        }

        if (maskFilter->directFilterMaskGPU(context->textureProvider(),
                                            drawContext,
                                            paint,
                                            clip,
                                            viewMatrix,
                                            strokeInfo,
                                            *devPathPtr)) {
            // the mask filter was able to draw itself directly, so there's nothing
            // left to do.
            return;
        }

        SkAutoTUnref<GrTexture> mask(create_mask_GPU(context,
                                                     &maskRect,
                                                     *devPathPtr,
                                                     strokeInfo,
                                                     paint->isAntiAlias(),
                                                     drawContext->numColorSamples()));
        if (mask) {
            GrTexture* filtered;

            if (maskFilter->filterMaskGPU(mask, viewMatrix, maskRect, &filtered, true)) {
                // filterMaskGPU gives us ownership of a ref to the result
                SkAutoTUnref<GrTexture> atu(filtered);
                if (draw_mask(drawContext, clip, viewMatrix, maskRect, paint, filtered)) {
                    // This path is completely drawn
                    return;
                }
            }
        }
    }

    // draw the mask on the CPU - this is a fallthrough path in case the
    // GPU path fails
    SkPaint::Style style = strokeInfo.isHairlineStyle() ? SkPaint::kStroke_Style :
                                                          SkPaint::kFill_Style;
    sw_draw_with_mask_filter(drawContext, context->textureProvider(),
                             clip, viewMatrix, *devPathPtr,
                             maskFilter, clipBounds, paint, style);
}

void GrBlurUtils::drawPathWithMaskFilter(GrContext* context,
                                         GrDrawContext* drawContext,
                                         const GrClip& clip,
                                         const SkPath& origPath,
                                         GrPaint* paint,
                                         const SkMatrix& viewMatrix,
                                         const SkMaskFilter* mf,
                                         const SkPathEffect* pathEffect,
                                         const GrStrokeInfo& origStrokeInfo,
                                         bool pathIsMutable) {
    SkPath* pathPtr = const_cast<SkPath*>(&origPath);

    SkTLazy<SkPath> tmpPath;
    GrStrokeInfo strokeInfo(origStrokeInfo);

    if (!strokeInfo.isDashed() && pathEffect && pathEffect->filterPath(tmpPath.init(), *pathPtr,
                                                                       &strokeInfo, nullptr)) {
        pathPtr = tmpPath.get();
        pathPtr->setIsVolatile(true);
        pathIsMutable = true;
        pathEffect = nullptr;
    }

    draw_path_with_mask_filter(context, drawContext, clip, paint, viewMatrix, mf, pathEffect,
                               strokeInfo, pathPtr, pathIsMutable);
}

void GrBlurUtils::drawPathWithMaskFilter(GrContext* context, 
                                         GrDrawContext* drawContext,
                                         const GrClip& clip,
                                         const SkPath& origSrcPath,
                                         const SkPaint& paint,
                                         const SkMatrix& origViewMatrix,
                                         const SkMatrix* prePathMatrix,
                                         const SkIRect& clipBounds,
                                         bool pathIsMutable) {
    SkASSERT(!pathIsMutable || origSrcPath.isVolatile());

    GrStrokeInfo strokeInfo(paint);

    // If we have a prematrix, apply it to the path, optimizing for the case
    // where the original path can in fact be modified in place (even though
    // its parameter type is const).
    SkPath* pathPtr = const_cast<SkPath*>(&origSrcPath);
    SkTLazy<SkPath> tmpPath;
    SkTLazy<SkPath> effectPath;
    SkPathEffect* pathEffect = paint.getPathEffect();

    SkMatrix viewMatrix = origViewMatrix;

    if (prePathMatrix) {
        // stroking, path effects, and blurs are supposed to be applied *after* the prePathMatrix.
        // The pre-path-matrix also should not affect shading.
        if (!paint.getMaskFilter() && !pathEffect && !paint.getShader() &&
            (strokeInfo.isFillStyle() || strokeInfo.isHairlineStyle())) {
            viewMatrix.preConcat(*prePathMatrix);
        } else {
            SkPath* result = pathPtr;

            if (!pathIsMutable) {
                result = tmpPath.init();
                result->setIsVolatile(true);
                pathIsMutable = true;
            }
            // should I push prePathMatrix on our MV stack temporarily, instead
            // of applying it here? See SkDraw.cpp
            pathPtr->transform(*prePathMatrix, result);
            pathPtr = result;
        }
    }
    // at this point we're done with prePathMatrix
    SkDEBUGCODE(prePathMatrix = (const SkMatrix*)0x50FF8001;)

    SkTLazy<SkPath> tmpPath2;

    if (!strokeInfo.isDashed() && pathEffect &&
        pathEffect->filterPath(tmpPath2.init(), *pathPtr, &strokeInfo, nullptr)) {
        pathPtr = tmpPath2.get();
        pathPtr->setIsVolatile(true);
        pathIsMutable = true;
        pathEffect = nullptr;
    }

    GrPaint grPaint;
    if (!SkPaintToGrPaint(context, paint, viewMatrix, &grPaint)) {
        return;
    }

    if (paint.getMaskFilter()) {
        draw_path_with_mask_filter(context, drawContext, clip, &grPaint, viewMatrix,
                                   paint.getMaskFilter(), pathEffect, strokeInfo,
                                   pathPtr, pathIsMutable);
    } else {
        drawContext->drawPath(clip, grPaint, viewMatrix, *pathPtr, strokeInfo);
    }
}

