/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrBlurUtils.h"

#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/GrBitmapTextureMaker.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrFixedClip.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrSoftwarePathRenderer.h"
#include "src/gpu/GrStyle.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/GrThreadSafeCache.h"
#include "src/gpu/effects/GrTextureEffect.h"
#include "src/gpu/geometry/GrStyledShape.h"

#include "include/core/SkPaint.h"
#include "src/core/SkDraw.h"
#include "src/core/SkMaskFilterBase.h"
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkTLazy.h"
#include "src/gpu/SkGr.h"

static bool clip_bounds_quick_reject(const SkIRect& clipBounds, const SkIRect& rect) {
    return clipBounds.isEmpty() || rect.isEmpty() || !SkIRect::Intersects(clipBounds, rect);
}

static constexpr auto kMaskOrigin = kTopLeft_GrSurfaceOrigin;

// Draw a mask using the supplied paint. Since the coverage/geometry
// is already burnt into the mask this boils down to a rect draw.
// Return true if the mask was successfully drawn.
static bool draw_mask(GrSurfaceDrawContext* surfaceDrawContext,
                      const GrClip* clip,
                      const SkMatrix& viewMatrix,
                      const SkIRect& maskBounds,
                      GrPaint&& paint,
                      GrSurfaceProxyView mask) {
    SkMatrix inverse;
    if (!viewMatrix.invert(&inverse)) {
        return false;
    }

    mask.concatSwizzle(GrSwizzle("aaaa"));

    SkMatrix matrix = SkMatrix::Translate(-SkIntToScalar(maskBounds.fLeft),
                                          -SkIntToScalar(maskBounds.fTop));
    matrix.preConcat(viewMatrix);
    paint.setCoverageFragmentProcessor(
            GrTextureEffect::Make(std::move(mask), kUnknown_SkAlphaType, matrix));

    surfaceDrawContext->fillPixelsWithLocalMatrix(clip, std::move(paint), maskBounds, inverse);
    return true;
}

static void mask_release_proc(void* addr, void* /*context*/) {
    SkMask::FreeImage(addr);
}

// This stores the mapping from an unclipped, integerized, device-space, shape bounds to
// the filtered mask's draw rect.
struct DrawRectData {
    SkIVector fOffset;
    SkISize   fSize;
};

static sk_sp<SkData> create_data(const SkIRect& drawRect, const SkIRect& origDevBounds) {

    DrawRectData drawRectData { {drawRect.fLeft - origDevBounds.fLeft,
                                 drawRect.fTop - origDevBounds.fTop},
                                drawRect.size() };

    return SkData::MakeWithCopy(&drawRectData, sizeof(drawRectData));
}

static SkIRect extract_draw_rect_from_data(SkData* data, const SkIRect& origDevBounds) {
    auto drawRectData = static_cast<const DrawRectData*>(data->data());

    return SkIRect::MakeXYWH(origDevBounds.fLeft + drawRectData->fOffset.fX,
                             origDevBounds.fTop + drawRectData->fOffset.fY,
                             drawRectData->fSize.fWidth,
                             drawRectData->fSize.fHeight);
}

static GrSurfaceProxyView sw_create_filtered_mask(GrRecordingContext* rContext,
                                                  const SkMatrix& viewMatrix,
                                                  const GrStyledShape& shape,
                                                  const SkMaskFilter* filter,
                                                  const SkIRect& unclippedDevShapeBounds,
                                                  const SkIRect& clipBounds,
                                                  SkIRect* drawRect,
                                                  GrUniqueKey* key) {
    SkASSERT(filter);
    SkASSERT(!shape.style().applies());

    auto threadSafeCache = rContext->priv().threadSafeCache();

    GrSurfaceProxyView filteredMaskView;
    sk_sp<SkData> data;

    if (key->isValid()) {
        std::tie(filteredMaskView, data) = threadSafeCache->findWithData(*key);
    }

    if (filteredMaskView) {
        SkASSERT(data);
        SkASSERT(kMaskOrigin == filteredMaskView.origin());

        *drawRect = extract_draw_rect_from_data(data.get(), unclippedDevShapeBounds);
    } else {
        SkStrokeRec::InitStyle fillOrHairline = shape.style().isSimpleHairline()
                                                        ? SkStrokeRec::kHairline_InitStyle
                                                        : SkStrokeRec::kFill_InitStyle;

        // TODO: it seems like we could create an SkDraw here and set its fMatrix field rather
        // than explicitly transforming the path to device space.
        SkPath devPath;

        shape.asPath(&devPath);

        devPath.transform(viewMatrix);

        SkMask srcM, dstM;
        if (!SkDraw::DrawToMask(devPath, &clipBounds, filter, &viewMatrix, &srcM,
                                SkMask::kComputeBoundsAndRenderImage_CreateMode, fillOrHairline)) {
            return {};
        }
        SkAutoMaskFreeImage autoSrc(srcM.fImage);

        SkASSERT(SkMask::kA8_Format == srcM.fFormat);

        if (!as_MFB(filter)->filterMask(&dstM, srcM, viewMatrix, nullptr)) {
            return {};
        }
        // this will free-up dstM when we're done (allocated in filterMask())
        SkAutoMaskFreeImage autoDst(dstM.fImage);

        if (clip_bounds_quick_reject(clipBounds, dstM.fBounds)) {
            return {};
        }

        // we now have a device-aligned 8bit mask in dstM, ready to be drawn using
        // the current clip (and identity matrix) and GrPaint settings
        SkBitmap bm;
        if (!bm.installPixels(SkImageInfo::MakeA8(dstM.fBounds.width(), dstM.fBounds.height()),
                              autoDst.release(), dstM.fRowBytes, mask_release_proc, nullptr)) {
            return {};
        }
        bm.setImmutable();

        GrBitmapTextureMaker maker(rContext, bm, SkBackingFit::kApprox);
        filteredMaskView = maker.view(GrMipmapped::kNo);
        if (!filteredMaskView.proxy()) {
            return {};
        }

        SkASSERT(kMaskOrigin == filteredMaskView.origin());

        *drawRect = dstM.fBounds;

        if (key->isValid()) {
            key->setCustomData(create_data(*drawRect, unclippedDevShapeBounds));
            std::tie(filteredMaskView, data) = threadSafeCache->addWithData(*key, filteredMaskView);
            // If we got a different view back from 'addWithData' it could have a different drawRect
            *drawRect = extract_draw_rect_from_data(data.get(), unclippedDevShapeBounds);
        }
    }

    return filteredMaskView;
}

// Create a mask of 'shape' and return the resulting surfaceDrawContext
static std::unique_ptr<GrSurfaceDrawContext> create_mask_GPU(GrRecordingContext* context,
                                                             const SkIRect& maskRect,
                                                             const SkMatrix& origViewMatrix,
                                                             const GrStyledShape& shape,
                                                             int sampleCnt) {
    // Use GrResourceProvider::MakeApprox to implement our own approximate size matching, but demand
    // a "SkBackingFit::kExact" size match on the actual render target. We do this because the
    // filter will reach outside the src bounds, so we need to pre-clear these values to ensure a
    // "decal" sampling effect (i.e., ensure reads outside the src bounds return alpha=0).
    //
    // FIXME: Reads outside the left and top edges will actually clamp to the edge pixel. And in the
    // event that MakeApprox does not change the size, reads outside the right and/or bottom will do
    // the same. We should offset our filter within the render target and expand the size as needed
    // to guarantee at least 1px of padding on all sides.
    auto approxSize = GrResourceProvider::MakeApprox(maskRect.size());
    auto rtContext = GrSurfaceDrawContext::MakeWithFallback(
            context, GrColorType::kAlpha_8, nullptr, SkBackingFit::kExact, approxSize, sampleCnt,
            GrMipmapped::kNo, GrProtected::kNo, kMaskOrigin);
    if (!rtContext) {
        return nullptr;
    }

    rtContext->clear(SK_PMColor4fTRANSPARENT);

    GrPaint maskPaint;
    maskPaint.setCoverageSetOpXPFactory(SkRegion::kReplace_Op);

    // setup new clip
    GrFixedClip clip(rtContext->dimensions(), SkIRect::MakeWH(maskRect.width(), maskRect.height()));

    // Draw the mask into maskTexture with the path's integerized top-left at the origin using
    // maskPaint.
    SkMatrix viewMatrix = origViewMatrix;
    viewMatrix.postTranslate(-SkIntToScalar(maskRect.fLeft), -SkIntToScalar(maskRect.fTop));
    rtContext->drawShape(&clip, std::move(maskPaint), GrAA::kYes, viewMatrix, GrStyledShape(shape));
    return rtContext;
}

static bool get_unclipped_shape_dev_bounds(const GrStyledShape& shape, const SkMatrix& matrix,
                                           SkIRect* devBounds) {
    SkRect shapeBounds = shape.styledBounds();
    if (shapeBounds.isEmpty()) {
        return false;
    }
    SkRect shapeDevBounds;
    matrix.mapRect(&shapeDevBounds, shapeBounds);
    // Even though these are "unclipped" bounds we still clip to the int32_t range.
    // This is the largest int32_t that is representable exactly as a float. The next 63 larger ints
    // would round down to this value when cast to a float, but who really cares.
    // INT32_MIN is exactly representable.
    static constexpr int32_t kMaxInt = 2147483520;
    if (!shapeDevBounds.intersect(SkRect::MakeLTRB(INT32_MIN, INT32_MIN, kMaxInt, kMaxInt))) {
        return false;
    }
    // Make sure that the resulting SkIRect can have representable width and height
    if (SkScalarRoundToInt(shapeDevBounds.width()) > kMaxInt ||
        SkScalarRoundToInt(shapeDevBounds.height()) > kMaxInt) {
        return false;
    }
    shapeDevBounds.roundOut(devBounds);
    return true;
}

// Gets the shape bounds, the clip bounds, and the intersection (if any). Returns false if there
// is no intersection.
static bool get_shape_and_clip_bounds(GrSurfaceDrawContext* surfaceDrawContext,
                                      const GrClip* clip,
                                      const GrStyledShape& shape,
                                      const SkMatrix& matrix,
                                      SkIRect* unclippedDevShapeBounds,
                                      SkIRect* devClipBounds) {
    // compute bounds as intersection of rt size, clip, and path
    *devClipBounds = clip ? clip->getConservativeBounds()
                          : SkIRect::MakeWH(surfaceDrawContext->width(),
                                            surfaceDrawContext->height());

    if (!get_unclipped_shape_dev_bounds(shape, matrix, unclippedDevShapeBounds)) {
        *unclippedDevShapeBounds = SkIRect::MakeEmpty();
        return false;
    }

    return true;
}

// The key and clip-bounds are computed together because the caching decision can impact the
// clip-bound - since we only cache un-clipped masks the clip can be removed entirely.
// A 'false' return value indicates that the shape is known to be clipped away.
static bool compute_key_and_clip_bounds(GrUniqueKey* maskKey,
                                        SkIRect* boundsForClip,
                                        const GrCaps* caps,
                                        const SkMatrix& viewMatrix,
                                        bool inverseFilled,
                                        const SkMaskFilterBase* maskFilter,
                                        const GrStyledShape& shape,
                                        const SkIRect& unclippedDevShapeBounds,
                                        const SkIRect& devClipBounds) {
    *boundsForClip = devClipBounds;

#ifndef SK_DISABLE_MASKFILTERED_MASK_CACHING
    // To prevent overloading the cache with entries during animations we limit the cache of masks
    // to cases where the matrix preserves axis alignment.
    bool useCache = !inverseFilled && viewMatrix.preservesAxisAlignment() &&
                    shape.hasUnstyledKey() && as_MFB(maskFilter)->asABlur(nullptr);

    if (useCache) {
        SkIRect clippedMaskRect, unClippedMaskRect;
        maskFilter->canFilterMaskGPU(shape, unclippedDevShapeBounds, devClipBounds,
                                     viewMatrix, &clippedMaskRect);
        maskFilter->canFilterMaskGPU(shape, unclippedDevShapeBounds, unclippedDevShapeBounds,
                                     viewMatrix, &unClippedMaskRect);
        if (clippedMaskRect.isEmpty()) {
            return false;
        }

        // Use the cache only if >50% of the filtered mask is visible.
        int unclippedWidth = unClippedMaskRect.width();
        int unclippedHeight = unClippedMaskRect.height();
        int64_t unclippedArea = sk_64_mul(unclippedWidth, unclippedHeight);
        int64_t clippedArea = sk_64_mul(clippedMaskRect.width(), clippedMaskRect.height());
        int maxTextureSize = caps->maxTextureSize();
        if (unclippedArea > 2 * clippedArea || unclippedWidth > maxTextureSize ||
            unclippedHeight > maxTextureSize) {
            useCache = false;
        } else {
            // Make the clip not affect the mask
            *boundsForClip = unclippedDevShapeBounds;
        }
    }

    if (useCache) {
        static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
        GrUniqueKey::Builder builder(maskKey, kDomain, 5 + 2 + shape.unstyledKeySize(),
                                     "Mask Filtered Masks");

        // We require the upper left 2x2 of the matrix to match exactly for a cache hit.
        SkScalar sx = viewMatrix.get(SkMatrix::kMScaleX);
        SkScalar sy = viewMatrix.get(SkMatrix::kMScaleY);
        SkScalar kx = viewMatrix.get(SkMatrix::kMSkewX);
        SkScalar ky = viewMatrix.get(SkMatrix::kMSkewY);
        SkScalar tx = viewMatrix.get(SkMatrix::kMTransX);
        SkScalar ty = viewMatrix.get(SkMatrix::kMTransY);
        // Allow 8 bits each in x and y of subpixel positioning. But, note that we're allowing
        // reuse for integer translations.
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
        uint32_t styleBits = shape.style().isSimpleHairline()
                                    ? ((shape.style().strokeRec().getCap() << 1) | 1)
                                    : 0;
        builder[4] = fracX | (fracY >> 8) | (styleBits << 16);

        SkMaskFilterBase::BlurRec rec;
        SkAssertResult(as_MFB(maskFilter)->asABlur(&rec));

        builder[5] = rec.fStyle;  // TODO: we could put this with the other style bits
        builder[6] = SkFloat2Bits(rec.fSigma);
        shape.writeUnstyledKey(&builder[7]);
    }
#endif

    return true;
}

static GrSurfaceProxyView hw_create_filtered_mask(GrDirectContext* dContext,
                                                  GrSurfaceDrawContext* surfaceDrawContext,
                                                  const SkMatrix& viewMatrix,
                                                  const GrStyledShape& shape,
                                                  const SkMaskFilterBase* filter,
                                                  const SkIRect& unclippedDevShapeBounds,
                                                  const SkIRect& clipBounds,
                                                  SkIRect* maskRect,
                                                  GrUniqueKey* key) {
    if (!filter->canFilterMaskGPU(shape,
                                  unclippedDevShapeBounds,
                                  clipBounds,
                                  viewMatrix,
                                  maskRect)) {
        return {};
    }

    if (clip_bounds_quick_reject(clipBounds, *maskRect)) {
        // clipped out
        return {};
    }

    auto threadSafeCache = dContext->priv().threadSafeCache();

    GrSurfaceProxyView lazyView;
    sk_sp<GrThreadSafeCache::Trampoline> trampoline;

    if (key->isValid()) {
        // In this case, we want GPU-filtered masks to have priority over SW-generated ones so
        // we pre-emptively add a lazy-view to the cache and fill it in later.
        std::tie(lazyView, trampoline) = GrThreadSafeCache::CreateLazyView(
                dContext, GrColorType::kAlpha_8, maskRect->size(),
                kMaskOrigin, SkBackingFit::kApprox);
        if (!lazyView) {
            return {}; // fall back to a SW-created mask - 'create_mask_GPU' probably won't succeed
        }

        key->setCustomData(create_data(*maskRect, unclippedDevShapeBounds));
        auto [cachedView, data] = threadSafeCache->findOrAddWithData(*key, lazyView);
        if (cachedView != lazyView) {
            // In this case, the gpu-thread lost out to a recording thread - use its result.
            SkASSERT(data);
            SkASSERT(cachedView.asTextureProxy());
            SkASSERT(cachedView.origin() == kMaskOrigin);

            *maskRect = extract_draw_rect_from_data(data.get(), unclippedDevShapeBounds);
            return cachedView;
        }
    }

    std::unique_ptr<GrSurfaceDrawContext> maskRTC(create_mask_GPU(
                                                            dContext,
                                                            *maskRect,
                                                            viewMatrix,
                                                            shape,
                                                            surfaceDrawContext->numSamples()));
    if (!maskRTC) {
        if (key->isValid()) {
            // It is very unlikely that 'create_mask_GPU' will fail after 'CreateLazyView'
            // succeeded but, if it does, remove the lazy-view from the cache and fallback to
            // a SW-created mask. Note that any recording threads that glommed onto the
            // lazy-view will have to, later, drop those draws.
            threadSafeCache->remove(*key);
        }
        return {};
    }

    auto filteredMaskView = filter->filterMaskGPU(dContext,
                                                  maskRTC->readSurfaceView(),
                                                  maskRTC->colorInfo().colorType(),
                                                  maskRTC->colorInfo().alphaType(),
                                                  viewMatrix,
                                                  *maskRect);
    if (!filteredMaskView) {
        if (key->isValid()) {
            // Remove the lazy-view from the cache and fallback to a SW-created mask. Note that
            // any recording threads that glommed onto the lazy-view will have to, later, drop
            // those draws.
            threadSafeCache->remove(*key);
        }
        return {};
    }

    if (key->isValid()) {
        SkASSERT(filteredMaskView.dimensions() == lazyView.dimensions());
        SkASSERT(filteredMaskView.swizzle() == lazyView.swizzle());
        SkASSERT(filteredMaskView.origin() == lazyView.origin());

        trampoline->fProxy = filteredMaskView.asTextureProxyRef();
        return lazyView;
    }

    return filteredMaskView;
}

static void draw_shape_with_mask_filter(GrRecordingContext* rContext,
                                        GrSurfaceDrawContext* surfaceDrawContext,
                                        const GrClip* clip,
                                        GrPaint&& paint,
                                        const SkMatrix& viewMatrix,
                                        const SkMaskFilterBase* maskFilter,
                                        const GrStyledShape& origShape) {
    SkASSERT(maskFilter);

    const GrStyledShape* shape = &origShape;
    SkTLazy<GrStyledShape> tmpShape;

    if (origShape.style().applies()) {
        SkScalar styleScale =  GrStyle::MatrixToScaleFactor(viewMatrix);
        if (styleScale == 0) {
            return;
        }

        tmpShape.init(origShape.applyStyle(GrStyle::Apply::kPathEffectAndStrokeRec, styleScale));
        if (tmpShape->isEmpty()) {
            return;
        }

        shape = tmpShape.get();
    }

    if (maskFilter->directFilterMaskGPU(rContext, surfaceDrawContext, std::move(paint), clip,
                                        viewMatrix, *shape)) {
        // the mask filter was able to draw itself directly, so there's nothing
        // left to do.
        return;
    }
    assert_alive(paint);

    // If the path is hairline, ignore inverse fill.
    bool inverseFilled = shape->inverseFilled() &&
                         !GrPathRenderer::IsStrokeHairlineOrEquivalent(shape->style(),
                                                                       viewMatrix, nullptr);

    SkIRect unclippedDevShapeBounds, devClipBounds;
    if (!get_shape_and_clip_bounds(surfaceDrawContext, clip, *shape, viewMatrix,
                                   &unclippedDevShapeBounds, &devClipBounds)) {
        // TODO: just cons up an opaque mask here
        if (!inverseFilled) {
            return;
        }
    }

    GrUniqueKey maskKey;
    SkIRect boundsForClip;
    if (!compute_key_and_clip_bounds(&maskKey, &boundsForClip,
                                     surfaceDrawContext->caps(),
                                     viewMatrix, inverseFilled,
                                     maskFilter, *shape,
                                     unclippedDevShapeBounds,
                                     devClipBounds)) {
        return; // 'shape' was entirely clipped out
    }

    GrSurfaceProxyView filteredMaskView;
    SkIRect maskRect;

    if (auto dContext = rContext->asDirectContext()) {
        filteredMaskView = hw_create_filtered_mask(dContext, surfaceDrawContext,
                                                   viewMatrix, *shape, maskFilter,
                                                   unclippedDevShapeBounds, boundsForClip,
                                                   &maskRect, &maskKey);
        if (filteredMaskView) {
            if (draw_mask(surfaceDrawContext, clip, viewMatrix, maskRect, std::move(paint),
                          std::move(filteredMaskView))) {
                // This path is completely drawn
                return;
            }
            assert_alive(paint);
        }
    }

    // Either HW mask rendering failed or we're in a DDL recording thread
    filteredMaskView = sw_create_filtered_mask(rContext,
                                               viewMatrix, *shape, maskFilter,
                                               unclippedDevShapeBounds, boundsForClip,
                                               &maskRect, &maskKey);
    if (filteredMaskView) {
        if (draw_mask(surfaceDrawContext, clip, viewMatrix, maskRect, std::move(paint),
                      std::move(filteredMaskView))) {
            return;
        }
        assert_alive(paint);
    }
}

void GrBlurUtils::drawShapeWithMaskFilter(GrRecordingContext* context,
                                          GrSurfaceDrawContext* surfaceDrawContext,
                                          const GrClip* clip,
                                          const GrStyledShape& shape,
                                          GrPaint&& paint,
                                          const SkMatrix& viewMatrix,
                                          const SkMaskFilter* mf) {
    draw_shape_with_mask_filter(context, surfaceDrawContext, clip, std::move(paint),
                                viewMatrix, as_MFB(mf), shape);
}

void GrBlurUtils::drawShapeWithMaskFilter(GrRecordingContext* context,
                                          GrSurfaceDrawContext* surfaceDrawContext,
                                          const GrClip* clip,
                                          const SkPaint& paint,
                                          const SkMatrixProvider& matrixProvider,
                                          const GrStyledShape& shape) {
    if (context->abandoned()) {
        return;
    }

    GrPaint grPaint;
    if (!SkPaintToGrPaint(context, surfaceDrawContext->colorInfo(), paint, matrixProvider,
                          &grPaint)) {
        return;
    }

    const SkMatrix& viewMatrix(matrixProvider.localToDevice());
    SkMaskFilterBase* mf = as_MFB(paint.getMaskFilter());
    if (mf && !mf->hasFragmentProcessor()) {
        // The MaskFilter wasn't already handled in SkPaintToGrPaint
        draw_shape_with_mask_filter(context, surfaceDrawContext, clip, std::move(grPaint),
                                    viewMatrix, mf, shape);
    } else {
        surfaceDrawContext->drawShape(clip, std::move(grPaint), context->priv().chooseAA(paint),
                                      viewMatrix, GrStyledShape(shape));
    }
}
