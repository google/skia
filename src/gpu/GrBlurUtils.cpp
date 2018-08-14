/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBlurUtils.h"

#include "GrCaps.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrFixedClip.h"
#include "GrProxyProvider.h"
#include "GrRenderTargetContext.h"
#include "GrRenderTargetContextPriv.h"
#include "GrShape.h"
#include "GrStyle.h"
#include "GrTextureProxy.h"
#include "effects/GrSimpleTextureEffect.h"

#include "SkDraw.h"
#include "SkGr.h"
#include "SkMaskFilterBase.h"
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
                      GrPaint&& paint,
                      sk_sp<GrTextureProxy> mask) {
    SkMatrix inverse;
    if (!viewMatrix.invert(&inverse)) {
        return false;
    }

    SkMatrix matrix = SkMatrix::MakeTrans(-SkIntToScalar(maskRect.fLeft),
                                          -SkIntToScalar(maskRect.fTop));
    matrix.preConcat(viewMatrix);
    paint.addCoverageFragmentProcessor(GrSimpleTextureEffect::Make(std::move(mask), matrix));

    renderTargetContext->fillRectWithLocalMatrix(clip, std::move(paint), GrAA::kNo, SkMatrix::I(),
                                                 SkRect::Make(maskRect), inverse);
    return true;
}

static void mask_release_proc(void* addr, void* /*context*/) {
    SkMask::FreeImage(addr);
}

static bool sw_draw_with_mask_filter(GrContext* context,
                                     GrRenderTargetContext* renderTargetContext,
                                     const GrClip& clipData,
                                     const SkMatrix& viewMatrix,
                                     const SkPath& devPath,
                                     const SkMaskFilter* filter,
                                     const SkIRect& clipBounds,
                                     GrPaint&& paint,
                                     SkStrokeRec::InitStyle fillOrHairline,
                                     const GrUniqueKey& key) {
    auto proxyProvider = context->contextPriv().proxyProvider();

    sk_sp<GrTextureProxy> maskProxy;

    if (key.isValid()) {
        maskProxy = proxyProvider->findProxyByUniqueKey(key, renderTargetContext->origin());
    };

    SkIRect drawRect;
    if (maskProxy) {
        SkDebugf("--------------------------------found it\n");

        SkMask srcM, dstM;
        if (!SkDraw::DrawToMask(devPath, clipBounds, filter, viewMatrix, &srcM,
                                SkMask::kJustComputeBounds_CreateMode, fillOrHairline)) {
            return false;
        }

        srcM.fBounds;

        if (!as_MFB(filter)->filterMask(&dstM, srcM, viewMatrix, nullptr)) {
            return false;
        }

        SkASSERT(dstM.fBounds.width() == maskProxy->width());
        SkASSERT(dstM.fBounds.height() == maskProxy->height());
        drawRect = dstM.fBounds;
    } else {
        SkMask srcM, dstM;
        if (!SkDraw::DrawToMask(devPath, clipBounds, filter, viewMatrix, &srcM,
                                SkMask::kComputeBoundsAndRenderImage_CreateMode, fillOrHairline)) {
            return false;
        }
        SkAutoMaskFreeImage autoSrc(srcM.fImage);

        if (!as_MFB(filter)->filterMask(&dstM, srcM, viewMatrix, nullptr)) {
            return false;
        }
        // this will free-up dstM when we're done (allocated in filterMask())
        SkAutoMaskFreeImage autoDst(dstM.fImage);

        if (clip_bounds_quick_reject(clipBounds, dstM.fBounds)) {
            return false;
        }

        // we now have a device-aligned 8bit mask in dstM, ready to be drawn using
        // the current clip (and identity matrix) and GrPaint settings
        SkBitmap bm;
        if (!bm.installPixels(SkImageInfo::MakeA8(dstM.fBounds.width(), dstM.fBounds.height()),
                              autoDst.release(), dstM.fRowBytes, mask_release_proc, nullptr)) {
            return false;
        }
        bm.setImmutable();

        sk_sp<SkImage> image = SkImage::MakeFromBitmap(bm);
        if (!image) {
            return false;
        }

        maskProxy = proxyProvider->createTextureProxy(std::move(image),
                                                      kNone_GrSurfaceFlags,
                                                      1, SkBudgeted::kYes,
                                                      SkBackingFit::kApprox);
        if (!maskProxy) {
            return false;
        }

        if (key.isValid()) {
            proxyProvider->assignUniqueKeyToProxy(key, maskProxy.get());
        }

        drawRect = dstM.fBounds;
    }

    return draw_mask(renderTargetContext, clipData, viewMatrix, drawRect,
                     std::move(paint), std::move(maskProxy));
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
        sampleCnt = 1;
    }

    sk_sp<GrRenderTargetContext> rtContext(
        context->contextPriv().makeDeferredRenderTargetContextWithFallback(
            SkBackingFit::kApprox, maskRect.width(), maskRect.height(), kAlpha_8_GrPixelConfig,
            nullptr, sampleCnt));
    if (!rtContext) {
        return nullptr;
    }

    rtContext->priv().absClear(nullptr, 0x0);

    GrPaint maskPaint;
    maskPaint.setCoverageSetOpXPFactory(SkRegion::kReplace_Op);

    // setup new clip
    const SkIRect clipRect = SkIRect::MakeWH(maskRect.width(), maskRect.height());
    GrFixedClip clip(clipRect);

    // Draw the mask into maskTexture with the path's integerized top-left at
    // the origin using maskPaint.
    SkMatrix translate;
    translate.setTranslate(-SkIntToScalar(maskRect.fLeft), -SkIntToScalar(maskRect.fTop));
    rtContext->drawPath(clip, std::move(maskPaint), aa, translate, devPath,
                        GrStyle(fillOrHairline));
    return rtContext->asTextureProxyRef();
}

// Create a mask of 'shape' and place the result in 'mask'.
static sk_sp<GrTextureProxy> create_mask_GPU2(GrContext* context,
                                             const SkIRect& maskRect,
                                             const SkMatrix& origViewMatrix,
                                             const GrShape& shape,
                                             GrAA aa,
                                             int sampleCnt) {
    if (GrAA::kNo == aa) {
        // Don't need MSAA if mask isn't AA
        sampleCnt = 1;
    }

    sk_sp<GrRenderTargetContext> rtContext(
        context->contextPriv().makeDeferredRenderTargetContextWithFallback(
            SkBackingFit::kApprox, maskRect.width(), maskRect.height(), kAlpha_8_GrPixelConfig,
            nullptr, sampleCnt));
    if (!rtContext) {
        return nullptr;
    }

    rtContext->priv().absClear(nullptr, 0x0);

    GrPaint maskPaint;
    maskPaint.setCoverageSetOpXPFactory(SkRegion::kReplace_Op);

    // setup new clip
    const SkIRect clipRect = SkIRect::MakeWH(maskRect.width(), maskRect.height());
    GrFixedClip clip(clipRect);

    // Draw the mask into maskTexture with the path's integerized top-left at
    // the origin using maskPaint.
    SkMatrix viewMatrix = origViewMatrix;
    viewMatrix.preTranslate(-SkIntToScalar(maskRect.fLeft), -SkIntToScalar(maskRect.fTop));
    rtContext->drawShape(clip, std::move(maskPaint), aa, viewMatrix, shape);
    return rtContext->asTextureProxyRef();
}

#include "GrSoftwarePathRenderer.h"

static void draw_shape_with_mask_filter2(GrContext* context,
                                       GrRenderTargetContext* renderTargetContext,
                                       const GrClip& clip,
                                       GrPaint&& paint,
                                       GrAA aa,
                                       const SkMatrix& viewMatrix,
                                       const SkMaskFilterBase* maskFilter,
                                       const GrShape& origShape) {
    SkASSERT(maskFilter);

#if 1
    const GrShape* shapeyShape = &origShape;
    GrShape tempShape1;

    if (origShape.style().modifiesGeometry()) {
        SkScalar styleScale =  GrStyle::MatrixToScaleFactor(viewMatrix);
        if (0 == styleScale) {
            return;
        }

        tempShape1 = origShape.applyStyle(GrStyle::Apply::kPathEffectAndStrokeRec, styleScale);
        if (tempShape1.isEmpty()) {
            return;
        }

        shapeyShape = &tempShape1;
    }

    SkASSERT(!shapeyShape->style().modifiesGeometry());
    // We really need to know if the shape will be inverse filled or not
    // If the path is hairline, ignore inverse fill.
    bool inverseFilled = shapeyShape->inverseFilled() &&
                         !GrPathRenderer::IsStrokeHairlineOrEquivalent(shapeyShape->style(),
                                                                       viewMatrix, nullptr);

    SkIRect unclippedDevShapeBounds, clippedDevShapeBounds, devClipBounds1;
    // To prevent overloading the cache with entries during animations we limit the cache of masks
    // to cases where the matrix preserves axis alignment.
    bool useCache = !inverseFilled && viewMatrix.preservesAxisAlignment() &&
                    shapeyShape->hasUnstyledKey() && // && GrAAType::kCoverage == args.fAAType;
                    as_MFB(maskFilter)->asABlur1(nullptr);

    if (!GrSoftwarePathRenderer::GetShapeAndClipBounds(renderTargetContext,
                                                       clip, *shapeyShape,
                                                       viewMatrix,
                                                       &unclippedDevShapeBounds,
                                                       &clippedDevShapeBounds,
                                                       &devClipBounds1)) {
#if 0
        if (inverseFilled) {
            DrawAroundInvPath(args.fRenderTargetContext, std::move(args.fPaint),
                                *args.fUserStencilSettings, *args.fClip, *args.fViewMatrix,
                                devClipBounds, unclippedDevShapeBounds);
        }
#endif
        return;
    }

    const SkIRect* boundsForMask = &clippedDevShapeBounds;
    const SkIRect* boundsForClip = &devClipBounds1;
    if (useCache) {
        // Use the cache only if >50% of the path is visible.
        int unclippedWidth = unclippedDevShapeBounds.width();
        int unclippedHeight = unclippedDevShapeBounds.height();
        int64_t unclippedArea = sk_64_mul(unclippedWidth, unclippedHeight);
        int64_t clippedArea = sk_64_mul(clippedDevShapeBounds.width(),
                                        clippedDevShapeBounds.height());
        int maxTextureSize = renderTargetContext->caps()->maxTextureSize();
        if (unclippedArea > 2 * clippedArea || unclippedWidth > maxTextureSize ||
            unclippedHeight > maxTextureSize) {
            useCache = false;
        } else {
            boundsForMask = &unclippedDevShapeBounds;
            boundsForClip = &unclippedDevShapeBounds;
        }
    }

    SkRect maskRect;
    if (maskFilter->canFilterMaskGPU(*shapeyShape, //SkRRect::MakeRect(path->getBounds()),
                                     SkRect::Make(*boundsForMask),
                                     *boundsForClip,
                                     viewMatrix,
                                     &maskRect)) {
        // This mask will ultimately be drawn as a non-AA rect (see draw_mask).
        // Non-AA rects have a bad habit of snapping arbitrarily. Integerize here
        // so the mask draws in a reproducible manner.
        SkIRect finalIRect;
        maskRect.roundOut(&finalIRect);
        if (clip_bounds_quick_reject(*boundsForClip, finalIRect)) {
            // clipped out
            return;
        }

        if (maskFilter->directFilterMaskGPU(context,
                                            renderTargetContext,
                                            std::move(paint),
                                            clip,
                                            viewMatrix,
                                            *shapeyShape)) {
            // the mask filter was able to draw itself directly, so there's nothing
            // left to do.
            return;
        }

        GrProxyProvider* proxyProvider = context->contextPriv().proxyProvider();

        GrUniqueKey maskKey;
        if (useCache) {
            // We require the upper left 2x2 of the matrix to match exactly for a cache hit.
            SkScalar sx = viewMatrix.get(SkMatrix::kMScaleX);
            SkScalar sy = viewMatrix.get(SkMatrix::kMScaleY);
            SkScalar kx = viewMatrix.get(SkMatrix::kMSkewX);
            SkScalar ky = viewMatrix.get(SkMatrix::kMSkewY);
            static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
            GrUniqueKey::Builder builder(&maskKey, kDomain, 5 + 2 + shapeyShape->unstyledKeySize(),
                                         "Mask Filtered Masks");

            SkScalar tx = viewMatrix.get(SkMatrix::kMTransX);
            SkScalar ty = viewMatrix.get(SkMatrix::kMTransY);
            // Allow 8 bits each in x and y of subpixel positioning.
            SkFixed fracX = SkScalarToFixed(SkScalarFraction(tx)) & 0x0000FF00;
            SkFixed fracY = SkScalarToFixed(SkScalarFraction(ty)) & 0x0000FF00;

            builder[0] = SkFloat2Bits(sx);
            builder[1] = SkFloat2Bits(sy);
            builder[2] = SkFloat2Bits(kx);
            builder[3] = SkFloat2Bits(ky);
            // Distinguish between hairline and filled paths. For hairlines, we also need to include
            // the cap. (SW grows hairlines by 0.5 pixel with round and square caps). Note that
            // stroke-and-fill of hairlines is turned into pure fill by SkStrokeRec, so this covers
            // all cases we might see.
            uint32_t styleBits = shapeyShape->style().isSimpleHairline() ?
                                 ((shapeyShape->style().strokeRec().getCap() << 1) | 1) : 0;
            builder[4] = fracX | (fracY >> 8) | (styleBits << 16);

            SkMaskFilterBase::BlurRec rec;
            SkAssertResult(as_MFB(maskFilter)->asABlur1(&rec));

            builder[5] = rec.fStyle;
            builder[6] = rec.fSigma;
            shapeyShape->writeUnstyledKey(&builder[7]);
        }

#if 1
        sk_sp<GrTextureProxy> filteredMask;

        if (maskKey.isValid()) {
            SkASSERT(useCache);

            filteredMask = proxyProvider->findOrCreateProxyByUniqueKey(maskKey,
                                                                       renderTargetContext->origin());
            if (filteredMask) {
                SkDebugf("found it\n");
            } else {
                SkDebugf("didn't find it\n");
            }
        }

        if (!filteredMask) {
            sk_sp<GrTextureProxy> maskProxy = create_mask_GPU2(context,
                                                              finalIRect,
                                                              viewMatrix,
                                                              *shapeyShape,
                                                              aa,
                                                              renderTargetContext->numColorSamples());

            filteredMask = maskFilter->filterMaskGPU(context,
                                                     std::move(maskProxy),
                                                     viewMatrix,
                                                     finalIRect);
            if (filteredMask && maskKey.isValid()) {
                SkDebugf("created it and assigning key\n");
                proxyProvider->assignUniqueKeyToProxy(maskKey, filteredMask.get());
            }
        } else {
            SkDebugf("found it\n");
        }

        if (filteredMask) {
            if (draw_mask(renderTargetContext, clip, viewMatrix,
                          finalIRect, std::move(paint), std::move(filteredMask))) {
                // This path is completely drawn
                return;
            }
        }
#endif
    }

//    sw_draw_with_mask_filter(context, renderTargetContext, clip, viewMatrix, *path, maskFilter,
//                             clipBounds, std::move(paint), fillOrHairline, maskKey);
#endif
}

static void draw_path_with_mask_filter(GrContext* context,
                                       GrRenderTargetContext* renderTargetContext,
                                       const GrClip& clip,
                                       GrPaint&& paint,
                                       GrAA aa,
                                       const SkMatrix& viewMatrix,
                                       const SkMaskFilterBase* maskFilter,
                                       const GrStyle& style,
                                       const SkPath* path,
                                       bool pathIsMutable,
                                       const GrUniqueKey& key) {
    SkASSERT(maskFilter);

    SkIRect clipConservativeBounds;
    clip.getConservativeBounds(renderTargetContext->width(),
                               renderTargetContext->height(),
                               &clipConservativeBounds);
    SkTLazy<SkPath> tmpPath;
    SkStrokeRec::InitStyle fillOrHairline;

    // We just fully apply the style here.
    if (style.modifiesGeometry()) {
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
    if (maskFilter->canFilterMaskGPU(GrShape(), //SkRRect::MakeRect(path->getBounds()),
                                     path->getBounds(),
                                     clipConservativeBounds,
                                     viewMatrix,
                                     &maskRect)) {
        // This mask will ultimately be drawn as a non-AA rect (see draw_mask).
        // Non-AA rects have a bad habit of snapping arbitrarily. Integerize here
        // so the mask draws in a reproducible manner.
        SkIRect finalIRect;
        maskRect.roundOut(&finalIRect);
        if (clip_bounds_quick_reject(clipConservativeBounds, finalIRect)) {
            // clipped out
            return;
        }

        if (maskFilter->directFilterMaskGPU(context,
                                            renderTargetContext,
                                            std::move(paint),
                                            clip,
                                            viewMatrix,
                                            GrShape())) {
//                                            SkStrokeRec(fillOrHairline),
//                                            *path)) {
            // the mask filter was able to draw itself directly, so there's nothing
            // left to do.
            return;
        }

        GrProxyProvider* proxyProvider = context->contextPriv().proxyProvider();

#if 1
        sk_sp<GrTextureProxy> filteredMask;

        if (key.isValid()) {
            filteredMask = proxyProvider->findOrCreateProxyByUniqueKey(key,
                                                                       renderTargetContext->origin());
            if (filteredMask) {
                SkDebugf("found it\n");
            } else {
                SkDebugf("didn't find it\n");
            }
        }

        if (!filteredMask) {
            sk_sp<GrTextureProxy> maskProxy = create_mask_GPU(context,
                                                              finalIRect,
                                                              *path,
                                                              fillOrHairline,
                                                              aa,
                                                              renderTargetContext->numColorSamples());

            filteredMask = maskFilter->filterMaskGPU(context,
                                                     std::move(maskProxy),
                                                     viewMatrix,
                                                     finalIRect);
            if (filteredMask && key.isValid()) {
                SkDebugf("created it and assigning key\n");
                proxyProvider->assignUniqueKeyToProxy(key, filteredMask.get());
            }
        } else {
            SkDebugf("found it\n");
        }

        if (filteredMask) {
            if (draw_mask(renderTargetContext, clip, viewMatrix,
                          finalIRect, std::move(paint), std::move(filteredMask))) {
                // This path is completely drawn
                return;
            }
        }
#endif
    }

    sw_draw_with_mask_filter(context, renderTargetContext, clip, viewMatrix, *path, maskFilter,
                             clipConservativeBounds, std::move(paint), fillOrHairline, key);
}

void GrBlurUtils::drawPathWithMaskFilter2(GrContext* context,
                                         GrRenderTargetContext* renderTargetContext,
                                         const GrClip& clip,
                                         const SkPath& path,
                                         GrPaint&& paint,
                                         GrAA aa,
                                         const SkMatrix& viewMatrix,
                                         const SkMaskFilter* mf,
                                         const GrStyle& style,
                                         bool pathIsMutable,
                                         const GrUniqueKey& key) {
    draw_path_with_mask_filter(context, renderTargetContext, clip, std::move(paint), aa, viewMatrix,
                               as_MFB(mf), style, &path, pathIsMutable, key);
}

void GrBlurUtils::drawShapeWithMaskFilter3(GrContext* context,
                                           GrRenderTargetContext* renderTargetContext,
                                           const GrClip& clip,
                                           const SkPaint& paint,
                                           const SkMatrix& viewMatrix,
                                           const GrShape& shape) {
    if (context->abandoned()) {
        return;
    }

    GrPaint grPaint;
    if (!SkPaintToGrPaint(context, renderTargetContext->colorSpaceInfo(), paint, viewMatrix,
                          &grPaint)) {
        return;
    }

    GrAA aa = GrAA(paint.isAntiAlias());
    SkMaskFilterBase* mf = as_MFB(paint.getMaskFilter());
    if (mf && !mf->hasFragmentProcessor()) {
        // The MaskFilter wasn't already handled in SkPaintToGrPaint
        draw_shape_with_mask_filter2(context, renderTargetContext, clip, std::move(grPaint), aa,
                                   viewMatrix, mf, shape);
    } else {
        renderTargetContext->drawShape(clip, std::move(grPaint), aa, viewMatrix, shape);
    }
}

void GrBlurUtils::drawPathWithMaskFilter1(GrContext* context,
                                         GrRenderTargetContext* renderTargetContext,
                                         const GrClip& clip,
                                         const SkPath& origPath,
                                         const SkPaint& paint,
                                         const SkMatrix& origViewMatrix,
                                         const SkMatrix* prePathMatrix,
                                         const SkIRect& clipBounds,
                                         bool pathIsMutable,
                                         const GrUniqueKey& key) {
    if (context->abandoned()) {
        return;
    }

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
        if (!paint.getMaskFilter() && !paint.getShader() && !style.modifiesGeometry()) {
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
    if (!SkPaintToGrPaint(context, renderTargetContext->colorSpaceInfo(), paint, viewMatrix,
                          &grPaint)) {
        return;
    }
    GrAA aa = GrAA(paint.isAntiAlias());
    SkMaskFilterBase* mf = as_MFB(paint.getMaskFilter());
    if (mf && !mf->hasFragmentProcessor()) {
        // The MaskFilter wasn't already handled in SkPaintToGrPaint
        draw_path_with_mask_filter(context, renderTargetContext, clip, std::move(grPaint), aa,
                                   viewMatrix, mf, style, path, pathIsMutable, key);
    } else {
        renderTargetContext->drawPath(clip, std::move(grPaint), aa, viewMatrix, *path, style);
    }
}
