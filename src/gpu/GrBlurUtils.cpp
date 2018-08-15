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
#include "GrSoftwarePathRenderer.h"
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
                                     const GrShape& shape,
                                     const SkMaskFilter* filter,
                                     const SkIRect& clipBounds,
                                     GrPaint&& paint,
                                     const GrUniqueKey& key) {
    SkASSERT(filter);

    auto proxyProvider = context->contextPriv().proxyProvider();

    SkASSERT(!shape.style().applies());

    sk_sp<GrTextureProxy> maskProxy;

    if (key.isValid()) {
        // TODO: this cache look up is duplicated in draw_shape_with_mask_filter for gpu
        maskProxy = proxyProvider->findProxyByUniqueKey(key, renderTargetContext->origin());
    }

    SkStrokeRec::InitStyle fillOrHairline = shape.style().isSimpleHairline()
                                                    ? SkStrokeRec::kHairline_InitStyle
                                                    : SkStrokeRec::kFill_InitStyle;

    SkIRect drawRect;
    if (maskProxy) {
        SkRect devBounds = shape.bounds();
        viewMatrix.mapRect(&devBounds);

        // Here we need to recompute the destination bounds in order to draw the mask correctly
        SkMask srcM, dstM;
        if (!SkDraw::ComputeMaskBounds(devBounds, clipBounds, filter, viewMatrix, &srcM.fBounds)) {
            return false;
        }

        if (!as_MFB(filter)->filterMask(&dstM, srcM, viewMatrix, nullptr)) {
            return false;
        }

        SkASSERT(dstM.fBounds.width() == maskProxy->width());
        SkASSERT(dstM.fBounds.height() == maskProxy->height());
        drawRect = dstM.fBounds;
    } else {
        // TODO: it seems like we could create an SkDraw here and set its fMatrix field rather
        // than explicitly transforming the path to device space.
        SkPath devPath;

        shape.asPath(&devPath);

        devPath.transform(viewMatrix);

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

static void draw_shape_with_mask_filter(GrContext* context,
                                        GrRenderTargetContext* renderTargetContext,
                                        const GrClip& clip,
                                        GrPaint&& paint,
                                        GrAA aa,
                                        const SkMatrix& viewMatrix,
                                         const SkMaskFilterBase* maskFilter,
                                        const GrShape& origShape) {
    SkASSERT(maskFilter);

    const GrShape* shapeyShape = &origShape;
    GrShape tempShape1;

    if (origShape.style().applies()) {
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

    SkASSERT(!shapeyShape->style().applies());
    // We really need to know if the shape will be inverse filled or not
    // If the path is hairline, ignore inverse fill.
    bool inverseFilled = shapeyShape->inverseFilled() &&
                         !GrPathRenderer::IsStrokeHairlineOrEquivalent(shapeyShape->style(),
                                                                       viewMatrix, nullptr);

    SkIRect unclippedDevShapeBounds, clippedDevShapeBounds, devClipBounds1;
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

    // To prevent overloading the cache with entries during animations we limit the cache of masks
    // to cases where the matrix preserves axis alignment.
    bool useCache = !inverseFilled && viewMatrix.preservesAxisAlignment() &&
                    shapeyShape->hasUnstyledKey() && // && GrAAType::kCoverage == args.fAAType;
                    as_MFB(maskFilter)->asABlur(nullptr);

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

    GrUniqueKey maskKey;
    if (useCache) {
        static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
        GrUniqueKey::Builder builder(&maskKey, kDomain, 5 + 2 + shapeyShape->unstyledKeySize(),
                                     "Mask Filtered Masks");

        // We require the upper left 2x2 of the matrix to match exactly for a cache hit.
        SkScalar sx = viewMatrix.get(SkMatrix::kMScaleX);
        SkScalar sy = viewMatrix.get(SkMatrix::kMScaleY);
        SkScalar kx = viewMatrix.get(SkMatrix::kMSkewX);
        SkScalar ky = viewMatrix.get(SkMatrix::kMSkewY);
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
        SkAssertResult(as_MFB(maskFilter)->asABlur(&rec));

        builder[5] = rec.fStyle;  // TODO: we could put this with the other style bits
        builder[6] = rec.fSigma;
        shapeyShape->writeUnstyledKey(&builder[7]);
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

        GrProxyProvider* proxyProvider = context->contextPriv().proxyProvider();

        sk_sp<GrTextureProxy> filteredMask;

        if (maskKey.isValid()) {
            // TODO: this cache look up is duplicated in sw_draw_with_mask_filter for raster
            filteredMask = proxyProvider->findOrCreateProxyByUniqueKey(maskKey,
                                                                       renderTargetContext->origin());
        }

        if (!filteredMask) {
            sk_sp<GrTextureProxy> maskProxy = create_mask_GPU2(context,
                                                               finalIRect,
                                                               viewMatrix,
                                                               *shapeyShape,
                                                               aa,
                                                               renderTargetContext->numColorSamples());

            if (maskProxy) {
                filteredMask = maskFilter->filterMaskGPU(context,
                                                         std::move(maskProxy),
                                                         viewMatrix,
                                                         finalIRect);
                if (filteredMask && maskKey.isValid()) {
                    proxyProvider->assignUniqueKeyToProxy(maskKey, filteredMask.get());
                }
            }
        }

        if (filteredMask) {
            if (draw_mask(renderTargetContext, clip, viewMatrix,
                          finalIRect, std::move(paint), std::move(filteredMask))) {
                // This path is completely drawn
                return;
            }
        }
    }

    sw_draw_with_mask_filter(context, renderTargetContext, clip, viewMatrix, *shapeyShape, maskFilter,
                             *boundsForMask, std::move(paint), maskKey);
}

void GrBlurUtils::drawShapeWithMaskFilter(GrContext* context,
                                          GrRenderTargetContext* renderTargetContext,
                                          const GrClip& clip,
                                          const GrShape& shape,
                                          GrPaint&& paint,
                                          GrAA aa,
                                          const SkMatrix& viewMatrix,
                                          const SkMaskFilter* mf) {
    draw_shape_with_mask_filter(context, renderTargetContext, clip, std::move(paint), aa,
                                viewMatrix, as_MFB(mf), shape);
}

void GrBlurUtils::drawShapeWithMaskFilter(GrContext* context,
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
        draw_shape_with_mask_filter(context, renderTargetContext, clip, std::move(grPaint), aa,
                                    viewMatrix, mf, shape);
    } else {
        renderTargetContext->drawShape(clip, std::move(grPaint), aa, viewMatrix, shape);
    }
}
