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
#include "SkGr.h"
#include "SkMaskFilter.h"
#include "SkPaint.h"

static bool clip_bounds_quick_reject(const SkIRect& clipBounds, const SkIRect& rect) {
    return clipBounds.isEmpty() || rect.isEmpty() || !SkIRect::Intersects(clipBounds, rect);
}

// Draw a mask using the supplied paint. Since the coverage/geometry
// is already burnt into the mask this boils down to a rect draw.
// Return true if the mask was successfully drawn.
static bool draw_mask(GrDrawContext* drawContext,
                      GrRenderTarget* rt,
                      const GrClip& clip,
                      const SkMatrix& viewMatrix,
                      const SkRect& maskRect,
                      GrPaint* grp,
                      GrTexture* mask) {
    SkMatrix matrix;
    matrix.setTranslate(-maskRect.fLeft, -maskRect.fTop);
    matrix.postIDiv(mask->width(), mask->height());

    grp->addCoverageProcessor(GrSimpleTextureEffect::Create(grp->getProcessorDataManager(),
                                                            mask, matrix,
                                                            kDevice_GrCoordSet))->unref();

    SkMatrix inverse;
    if (!viewMatrix.invert(&inverse)) {
        return false;
    }
    drawContext->drawNonAARectWithLocalMatrix(rt, clip, *grp, SkMatrix::I(), maskRect, inverse);
    return true;
}

static bool draw_with_mask_filter(GrDrawContext* drawContext,
                                  GrTextureProvider* textureProvider,
                                  GrRenderTarget* rt,
                                  const GrClip& clipData,
                                  const SkMatrix& viewMatrix,
                                  const SkPath& devPath,
                                  SkMaskFilter* filter,
                                  const SkIRect& clipBounds,
                                  GrPaint* grp,
                                  SkPaint::Style style) {
    SkMask  srcM, dstM;

    if (!SkDraw::DrawToMask(devPath, &clipBounds, filter, &viewMatrix, &srcM,
                            SkMask::kComputeBoundsAndRenderImage_CreateMode, style)) {
        return false;
    }
    SkAutoMaskFreeImage autoSrc(srcM.fImage);

    if (!filter->filterMask(&dstM, srcM, viewMatrix, NULL)) {
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

    return draw_mask(drawContext, rt, clipData, viewMatrix, maskRect, grp, texture);
}

// Create a mask of 'devPath' and place the result in 'mask'.
static GrTexture* create_mask_GPU(GrContext* context,
                                  const SkRect& maskRect,
                                  const SkPath& devPath,
                                  const GrStrokeInfo& strokeInfo,
                                  bool doAA,
                                  int sampleCnt) {
    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = SkScalarCeilToInt(maskRect.width());
    desc.fHeight = SkScalarCeilToInt(maskRect.height());
    desc.fSampleCnt = doAA ? sampleCnt : 0;
    // We actually only need A8, but it often isn't supported as a
    // render target so default to RGBA_8888
    desc.fConfig = kRGBA_8888_GrPixelConfig;

    if (context->caps()->isConfigRenderable(kAlpha_8_GrPixelConfig, desc.fSampleCnt > 0)) {
        desc.fConfig = kAlpha_8_GrPixelConfig;
    }

    GrTexture* mask = context->textureProvider()->createApproxTexture(desc);
    if (NULL == mask) {
        return NULL;
    }

    SkRect clipRect = SkRect::MakeWH(maskRect.width(), maskRect.height());

    GrDrawContext* drawContext = context->drawContext();
    if (!drawContext) {
        return NULL;
    }

    drawContext->clear(mask->asRenderTarget(), NULL, 0x0, true);

    GrPaint tempPaint;
    tempPaint.setAntiAlias(doAA);
    tempPaint.setCoverageSetOpXPFactory(SkRegion::kReplace_Op);

    // setup new clip
    GrClip clip(clipRect);

    // Draw the mask into maskTexture with the path's top-left at the origin using tempPaint.
    SkMatrix translate;
    translate.setTranslate(-maskRect.fLeft, -maskRect.fTop);
    drawContext->drawPath(mask->asRenderTarget(), clip, tempPaint, translate, devPath, strokeInfo);
    return mask;
}

void GrBlurUtils::drawPathWithMaskFilter(GrContext* context, 
                                         GrDrawContext* drawContext,
                                         GrRenderTarget* renderTarget,
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
        if (NULL == paint.getMaskFilter() && NULL == pathEffect && NULL == paint.getShader() &&
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

    GrPaint grPaint;
    if (!SkPaint2GrPaint(context, renderTarget, paint, viewMatrix, true, &grPaint)) {
        return;
    }

    const SkRect* cullRect = NULL;  // TODO: what is our bounds?
    if (!strokeInfo.isDashed() && pathEffect && pathEffect->filterPath(effectPath.init(), *pathPtr,
                                                                       &strokeInfo, cullRect)) {
        pathPtr = effectPath.get();
        pathIsMutable = true;
    }

    if (paint.getMaskFilter()) {
        if (!strokeInfo.isHairlineStyle()) {
            SkPath* strokedPath = pathIsMutable ? pathPtr : tmpPath.init();
            if (strokeInfo.isDashed()) {
                if (pathEffect->filterPath(strokedPath, *pathPtr, &strokeInfo, cullRect)) {
                    pathPtr = strokedPath;
                    pathIsMutable = true;
                }
                strokeInfo.removeDash();
            }
            if (strokeInfo.applyToPath(strokedPath, *pathPtr)) {
                pathPtr = strokedPath;
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
        if (paint.getMaskFilter()->canFilterMaskGPU(devPathPtr->getBounds(),
                                                    clipBounds,
                                                    viewMatrix,
                                                    &maskRect)) {
            SkIRect finalIRect;
            maskRect.roundOut(&finalIRect);
            if (clip_bounds_quick_reject(clipBounds, finalIRect)) {
                // clipped out
                return;
            }

            if (paint.getMaskFilter()->directFilterMaskGPU(context->textureProvider(),
                                                           drawContext,
                                                           renderTarget,
                                                           &grPaint,
                                                           clip,
                                                           viewMatrix,
                                                           strokeInfo,
                                                           *devPathPtr)) {
                // the mask filter was able to draw itself directly, so there's nothing
                // left to do.
                return;
            }

            SkAutoTUnref<GrTexture> mask(create_mask_GPU(context,
                                                         maskRect,
                                                         *devPathPtr,
                                                         strokeInfo,
                                                         grPaint.isAntiAlias(),
                                                         renderTarget->numColorSamples()));
            if (mask) {
                GrTexture* filtered;

                if (paint.getMaskFilter()->filterMaskGPU(mask, viewMatrix, maskRect,
                                                         &filtered, true)) {
                    // filterMaskGPU gives us ownership of a ref to the result
                    SkAutoTUnref<GrTexture> atu(filtered);
                    if (draw_mask(drawContext,
                                  renderTarget,
                                  clip,
                                  viewMatrix,
                                  maskRect,
                                  &grPaint,
                                  filtered)) {
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
        draw_with_mask_filter(drawContext, context->textureProvider(), renderTarget,
                              clip, viewMatrix, *devPathPtr,
                              paint.getMaskFilter(), clipBounds, &grPaint, style);
        return;
    }

    drawContext->drawPath(renderTarget, clip, grPaint, viewMatrix, *pathPtr, strokeInfo);
}

