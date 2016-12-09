/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBlurUtils.h"
#include "GrRenderTargetContext.h"
#include "GrCaps.h"
#include "GrContext.h"
#include "GrFixedClip.h"
#include "GrRenderTargetContextPriv.h"
#include "effects/GrSimpleTextureEffect.h"
#include "GrStyle.h"
#include "GrTexture.h"
#include "GrTextureProxy.h"
#include "GrTextureProvider.h"
#include "SkDraw.h"
#include "SkGrPriv.h"
#include "SkMaskFilter.h"
#include "SkPaint.h"
#include "SkTLazy.h"

static bool clip_bounds_quick_reject(const SkIRect& clipBounds, const SkIRect& rect) {
    return clipBounds.isEmpty() || rect.isEmpty() || !SkIRect::Intersects(clipBounds, rect);
}

// Draw a mask using the supplied paint. Since the coverage/geometry
// is already burnt into the mask this boils down to a rect draw.
// Return true if the mask was successfully drawn.
static bool draw_mask(GrRenderTargetContext* renderTargetContext,
                      const GrClip& clip,
                      const SkMatrix& viewMatrix,
                      const SkIRect& maskRect,
                      GrPaint* grp,
                      GrTexture* mask) {
    SkMatrix matrix;
    matrix.setTranslate(-SkIntToScalar(maskRect.fLeft), -SkIntToScalar(maskRect.fTop));
    matrix.postIDiv(mask->width(), mask->height());
    matrix.preConcat(viewMatrix);
    grp->addCoverageFragmentProcessor(GrSimpleTextureEffect::Make(mask, nullptr, matrix));

    SkMatrix inverse;
    if (!viewMatrix.invert(&inverse)) {
        return false;
    }
    renderTargetContext->fillRectWithLocalMatrix(clip, *grp, GrAA::kNo, SkMatrix::I(),
                                                 SkRect::Make(maskRect), inverse);
    return true;
}

static bool sw_draw_with_mask_filter(GrRenderTargetContext* renderTargetContext,
                                     GrTextureProvider* textureProvider,
                                     const GrClip& clipData,
                                     const SkMatrix& viewMatrix,
                                     const SkPath& devPath,
                                     const SkMaskFilter* filter,
                                     const SkIRect& clipBounds,
                                     GrPaint* grp,
                                     SkStrokeRec::InitStyle fillOrHairline) {
    SkMask  srcM, dstM;
    if (!SkDraw::DrawToMask(devPath, &clipBounds, filter, &viewMatrix, &srcM,
                            SkMask::kComputeBoundsAndRenderImage_CreateMode, fillOrHairline)) {
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

    sk_sp<GrTexture> texture(textureProvider->createApproxTexture(desc));
    if (!texture) {
        return false;
    }
    texture->writePixels(0, 0, desc.fWidth, desc.fHeight, desc.fConfig,
                               dstM.fImage, dstM.fRowBytes);

    return draw_mask(renderTargetContext, clipData, viewMatrix, dstM.fBounds, grp, texture.get());
}

// Create a mask of 'devPath' and place the result in 'mask'.
static sk_sp<GrTextureProxy> create_mask_GPU(GrContext* context,
                                             const SkIRect& maskRect,
                                             const SkPath& devPath,
                                             SkStrokeRec::InitStyle fillOrHairline,
                                             GrAA aa,
                                             int sampleCnt) {
    if (GrAA::kNo == aa) {
        // Don't need MSAA if mask isn't AA
        sampleCnt = 0;
    }

    sk_sp<GrRenderTargetContext> rtContext(context->makeDeferredRenderTargetContextWithFallback(
        SkBackingFit::kApprox, maskRect.width(), maskRect.height(), kAlpha_8_GrPixelConfig, nullptr,
        sampleCnt));
    if (!rtContext) {
        return nullptr;
    }

    rtContext->priv().absClear(nullptr, 0x0);

    GrPaint tempPaint;
    tempPaint.setCoverageSetOpXPFactory(SkRegion::kReplace_Op);

    // setup new clip
    const SkIRect clipRect = SkIRect::MakeWH(maskRect.width(), maskRect.height());
    GrFixedClip clip(clipRect);

    // Draw the mask into maskTexture with the path's integerized top-left at
    // the origin using tempPaint.
    SkMatrix translate;
    translate.setTranslate(-SkIntToScalar(maskRect.fLeft), -SkIntToScalar(maskRect.fTop));
    rtContext->drawPath(clip, tempPaint, aa, translate, devPath, GrStyle(fillOrHairline));
    return sk_ref_sp(rtContext->asDeferredTexture());
}

static void draw_path_with_mask_filter(GrContext* context,
                                       GrRenderTargetContext* renderTargetContext,
                                       const GrClip& clip,
                                       GrPaint* paint,
                                       GrAA aa,
                                       const SkMatrix& viewMatrix,
                                       const SkMaskFilter* maskFilter,
                                       const GrStyle& style,
                                       const SkPath* path,
                                       bool pathIsMutable) {
    SkASSERT(maskFilter);

    SkIRect clipBounds;
    clip.getConservativeBounds(renderTargetContext->width(),
                               renderTargetContext->height(),
                               &clipBounds);
    SkTLazy<SkPath> tmpPath;
    SkStrokeRec::InitStyle fillOrHairline;

    // We just fully apply the style here.
    if (style.applies()) {
        SkScalar scale = GrStyle::MatrixToScaleFactor(viewMatrix);
        if (0 == scale || !style.applyToPath(tmpPath.init(), &fillOrHairline, *path, scale)) {
            return;
        }
        pathIsMutable = true;
        path = tmpPath.get();
    } else if (style.isSimpleHairline()) {
        fillOrHairline = SkStrokeRec::kHairline_InitStyle;
    } else {
        SkASSERT(style.isSimpleFill());
        fillOrHairline = SkStrokeRec::kFill_InitStyle;
    }

    // transform the path into device space
    if (!viewMatrix.isIdentity()) {
        SkPath* result;
        if (pathIsMutable) {
            result = const_cast<SkPath*>(path);
        } else {
            if (!tmpPath.isValid()) {
                tmpPath.init();
            }
            result = tmpPath.get();
        }
        path->transform(viewMatrix, result);
        path = result;
        result->setIsVolatile(true);
        pathIsMutable = true;
    }

    SkRect maskRect;
    if (maskFilter->canFilterMaskGPU(SkRRect::MakeRect(path->getBounds()),
                                     clipBounds,
                                     viewMatrix,
                                     &maskRect)) {
        // This mask will ultimately be drawn as a non-AA rect (see draw_mask).
        // Non-AA rects have a bad habit of snapping arbitrarily. Integerize here
        // so the mask draws in a reproducible manner.
        SkIRect finalIRect;
        maskRect.roundOut(&finalIRect);
        if (clip_bounds_quick_reject(clipBounds, finalIRect)) {
            // clipped out
            return;
        }

        if (maskFilter->directFilterMaskGPU(context->textureProvider(),
                                            renderTargetContext,
                                            paint,
                                            clip,
                                            viewMatrix,
                                            SkStrokeRec(fillOrHairline),
                                            *path)) {
            // the mask filter was able to draw itself directly, so there's nothing
            // left to do.
            return;
        }

        sk_sp<GrTextureProxy> maskProxy(create_mask_GPU(context,
                                                        finalIRect,
                                                        *path,
                                                        fillOrHairline,
                                                        aa,
                                                        renderTargetContext->numColorSamples()));
        if (maskProxy) {
            GrTexture* filtered;

            GrTexture* mask = maskProxy->instantiate(context->textureProvider());
            if (!mask) {
                return;
            }

            if (maskFilter->filterMaskGPU(mask, viewMatrix, finalIRect, &filtered)) {
                // filterMaskGPU gives us ownership of a ref to the result
                sk_sp<GrTexture> atu(filtered);
                if (draw_mask(renderTargetContext, clip, viewMatrix, finalIRect, paint, filtered)) {
                    // This path is completely drawn
                    return;
                }
            }
        }
    }

    sw_draw_with_mask_filter(renderTargetContext, context->textureProvider(),
                             clip, viewMatrix, *path,
                             maskFilter, clipBounds, paint, fillOrHairline);
}

void GrBlurUtils::drawPathWithMaskFilter(GrContext* context,
                                         GrRenderTargetContext* renderTargetContext,
                                         const GrClip& clip,
                                         const SkPath& path,
                                         GrPaint* paint,
                                         GrAA aa,
                                         const SkMatrix& viewMatrix,
                                         const SkMaskFilter* mf,
                                         const GrStyle& style,
                                         bool pathIsMutable) {
    draw_path_with_mask_filter(context, renderTargetContext, clip, paint, aa, viewMatrix, mf,
                               style, &path, pathIsMutable);
}

void GrBlurUtils::drawPathWithMaskFilter(GrContext* context,
                                         GrRenderTargetContext* renderTargetContext,
                                         const GrClip& clip,
                                         const SkPath& origPath,
                                         const SkPaint& paint,
                                         const SkMatrix& origViewMatrix,
                                         const SkMatrix* prePathMatrix,
                                         const SkIRect& clipBounds,
                                         bool pathIsMutable) {
    SkASSERT(!pathIsMutable || origPath.isVolatile());

    GrStyle style(paint);
    // If we have a prematrix, apply it to the path, optimizing for the case
    // where the original path can in fact be modified in place (even though
    // its parameter type is const).

    const SkPath* path = &origPath;
    SkTLazy<SkPath> tmpPath;

    SkMatrix viewMatrix = origViewMatrix;

    if (prePathMatrix) {
        // Styling, blurs, and shading are supposed to be applied *after* the prePathMatrix.
        if (!paint.getMaskFilter() && !paint.getShader() && !style.applies()) {
            viewMatrix.preConcat(*prePathMatrix);
        } else {
            SkPath* result = pathIsMutable ? const_cast<SkPath*>(path) : tmpPath.init();
            pathIsMutable = true;
            path->transform(*prePathMatrix, result);
            path = result;
            result->setIsVolatile(true);
        }
    }
    // at this point we're done with prePathMatrix
    SkDEBUGCODE(prePathMatrix = (const SkMatrix*)0x50FF8001;)

    GrPaint grPaint;
    if (!SkPaintToGrPaint(context, renderTargetContext, paint, viewMatrix, &grPaint)) {
        return;
    }
    GrAA aa = GrBoolToAA(paint.isAntiAlias());
    SkMaskFilter* mf = paint.getMaskFilter();
    if (mf && !mf->asFragmentProcessor(nullptr, nullptr, viewMatrix)) {
        // The MaskFilter wasn't already handled in SkPaintToGrPaint
        draw_path_with_mask_filter(context, renderTargetContext, clip, &grPaint, aa, viewMatrix,
                                   mf, style,
                                   path, pathIsMutable);
    } else {
        renderTargetContext->drawPath(clip, grPaint, aa, viewMatrix, *path, style);
    }
}
