/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/SkGpuDevice.h"

#include "include/core/SkYUVAIndex.h"
#include "src/core/SkDraw.h"
#include "src/core/SkMaskFilterBase.h"
#include "src/gpu/GrBitmapTextureMaker.h"
#include "src/gpu/GrBlurUtils.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrColorSpaceXform.h"
#include "src/gpu/GrImageTextureMaker.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrStyle.h"
#include "src/gpu/GrTextureAdjuster.h"
#include "src/gpu/GrTextureMaker.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrBicubicEffect.h"
#include "src/gpu/effects/GrTextureDomain.h"
#include "src/gpu/effects/generated/GrSimpleTextureEffect.h"
#include "src/gpu/geometry/GrShape.h"
#include "src/image/SkImage_Base.h"

namespace {

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
                            int numSamples) {
    // Only gets called if has_aligned_samples returned false.
    // So we can assume that sampling is axis aligned but not texel aligned.
    SkASSERT(!has_aligned_samples(srcRect, transformedRect));
    SkRect innerSrcRect(srcRect), innerTransformedRect, outerTransformedRect(transformedRect);
    if (numSamples > 1) {
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
                                         int numSamples) {
    if (srcRectToDeviceSpace.rectStaysRect()) {
        // sampling is axis-aligned
        SkRect transformedRect;
        srcRectToDeviceSpace.mapRect(&transformedRect, srcRect);

        if (has_aligned_samples(srcRect, transformedRect) ||
            !may_color_bleed(srcRect, transformedRect, srcRectToDeviceSpace, numSamples)) {
            return true;
        }
    }
    return false;
}

enum class ImageDrawMode {
    // Src and dst have been restricted to the image content. May need to clamp, no need to decal.
    kOptimized,
    // Src and dst are their original sizes, requires use of a decal instead of plain clamping.
    // This is used when a dst clip is provided and extends outside of the optimized dst rect.
    kDecal,
    // Src or dst are empty, or do not intersect the image content so don't draw anything.
    kSkip
};

/**
 * Optimize the src rect sampling area within an image (sized 'width' x 'height') such that
 * 'outSrcRect' will be completely contained in the image's bounds. The corresponding rect
 * to draw will be output to 'outDstRect'. The mapping between src and dst will be cached in
 * 'srcToDst'. Outputs are not always updated when kSkip is returned.
 *
 * If 'origSrcRect' is null, implicitly use the image bounds. If 'origDstRect' is null, use the
 * original src rect. 'dstClip' should be null when there is no additional clipping.
 */
static ImageDrawMode optimize_sample_area(const SkISize& image, const SkRect* origSrcRect,
                                          const SkRect* origDstRect, const SkPoint dstClip[4],
                                          SkRect* outSrcRect, SkRect* outDstRect,
                                          SkMatrix* srcToDst) {
    SkRect srcBounds = SkRect::MakeIWH(image.fWidth, image.fHeight);

    SkRect src = origSrcRect ? *origSrcRect : srcBounds;
    SkRect dst = origDstRect ? *origDstRect : src;

    if (src.isEmpty() || dst.isEmpty()) {
        return ImageDrawMode::kSkip;
    }

    if (outDstRect) {
        srcToDst->setRectToRect(src, dst, SkMatrix::kFill_ScaleToFit);
    } else {
        srcToDst->setIdentity();
    }

    if (origSrcRect && !srcBounds.contains(src)) {
        if (!src.intersect(srcBounds)) {
            return ImageDrawMode::kSkip;
        }
        srcToDst->mapRect(&dst, src);

        // Both src and dst have gotten smaller. If dstClip is provided, confirm it is still
        // contained in dst, otherwise cannot optimize the sample area and must use a decal instead
        if (dstClip) {
            for (int i = 0; i < 4; ++i) {
                if (!dst.contains(dstClip[i].fX, dstClip[i].fY)) {
                    // Must resort to using a decal mode restricted to the clipped 'src', and
                    // use the original dst rect (filling in src bounds as needed)
                    *outSrcRect = src;
                    *outDstRect = (origDstRect ? *origDstRect
                                               : (origSrcRect ? *origSrcRect : srcBounds));
                    return ImageDrawMode::kDecal;
                }
            }
        }
    }

    // The original src and dst were fully contained in the image, or there was no dst clip to
    // worry about, or the clip was still contained in the restricted dst rect.
    *outSrcRect = src;
    *outDstRect = dst;
    return ImageDrawMode::kOptimized;
}

/**
 * Checks whether the paint is compatible with using GrRenderTargetContext::drawTexture. It is more
 * efficient than the GrTextureProducer general case.
 */
static bool can_use_draw_texture(const SkPaint& paint) {
    return (!paint.getColorFilter() && !paint.getShader() && !paint.getMaskFilter() &&
            !paint.getImageFilter() && paint.getFilterQuality() < kMedium_SkFilterQuality);
}

// Assumes srcRect and dstRect have already been optimized to fit the proxy
static void draw_texture(GrRenderTargetContext* rtc, const GrClip& clip, const SkMatrix& ctm,
                         const SkPaint& paint, const SkRect& srcRect, const SkRect& dstRect,
                         const SkPoint dstClip[4], GrAA aa, GrQuadAAFlags aaFlags,
                         SkCanvas::SrcRectConstraint constraint, sk_sp<GrTextureProxy> proxy,
                         SkAlphaType alphaType, SkColorSpace* colorSpace) {
    const GrColorInfo& dstInfo(rtc->colorInfo());
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
        color = SkColor4fPrepForDst(paint.getColor4f(), dstInfo).premul();
    } else {
        float paintAlpha = paint.getColor4f().fA;
        color = { paintAlpha, paintAlpha, paintAlpha, paintAlpha };
    }

    if (dstClip) {
        // Get source coords corresponding to dstClip
        SkPoint srcQuad[4];
        GrMapRectPoints(dstRect, srcRect, dstClip, srcQuad, 4);

        rtc->drawTextureQuad(clip, std::move(proxy), filter, paint.getBlendMode(), color,
                             srcQuad, dstClip, aa, aaFlags,
                             constraint == SkCanvas::kStrict_SrcRectConstraint ? &srcRect : nullptr,
                             ctm, std::move(textureXform));
    } else {
        rtc->drawTexture(clip, std::move(proxy), filter, paint.getBlendMode(), color, srcRect,
                         dstRect, aa, aaFlags, constraint, ctm, std::move(textureXform));
    }
}

// Assumes srcRect and dstRect have already been optimized to fit the proxy.
static void draw_texture_producer(GrContext* context, GrRenderTargetContext* rtc,
                                  const GrClip& clip, const SkMatrix& ctm,
                                  const SkPaint& paint, GrTextureProducer* producer,
                                  const SkRect& src, const SkRect& dst, const SkPoint dstClip[4],
                                  const SkMatrix& srcToDst, GrAA aa, GrQuadAAFlags aaFlags,
                                  SkCanvas::SrcRectConstraint constraint, bool attemptDrawTexture) {
    if (attemptDrawTexture && can_use_draw_texture(paint)) {
        // We've done enough checks above to allow us to pass ClampNearest() and not check for
        // scaling adjustments.
        auto proxy = producer->refTextureProxyForParams(GrSamplerState::ClampNearest(), nullptr);
        if (!proxy) {
            return;
        }

        draw_texture(rtc, clip, ctm, paint, src, dst, dstClip, aa, aaFlags, constraint,
                     std::move(proxy), producer->alphaType(),  producer->colorSpace());
        return;
    }

    const SkMaskFilter* mf = paint.getMaskFilter();

    // The shader expects proper local coords, so we can't replace local coords with texture coords
    // if the shader will be used. If we have a mask filter we will change the underlying geometry
    // that is rendered.
    bool canUseTextureCoordsAsLocalCoords = !use_shader(producer->isAlphaOnly(), paint) && !mf;

    // Specifying the texture coords as local coordinates is an attempt to enable more GrDrawOp
    // combining by not baking anything about the srcRect, dstRect, or ctm, into the texture
    // FP. In the future this should be an opaque optimization enabled by the combination of
    // GrDrawOp/GP and FP.
    if (mf && as_MFB(mf)->hasFragmentProcessor()) {
        mf = nullptr;
    }
    bool doBicubic;
    GrSamplerState::Filter fm = GrSkFilterQualityToGrFilterMode(
            producer->width(), producer->height(), paint.getFilterQuality(), ctm, srcToDst,
            context->priv().options().fSharpenMipmappedTextures, &doBicubic);
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
    bool coordsAllInsideSrcRect = aaFlags == GrQuadAAFlags::kNone && !mf;

    // Check for optimization to drop the src rect constraint when on bilerp.
    if (filterMode && GrSamplerState::Filter::kBilerp == *filterMode &&
        GrTextureAdjuster::kYes_FilterConstraint == constraintMode && coordsAllInsideSrcRect &&
        !producer->hasMixedResolutions()) {
        SkMatrix combinedMatrix;
        combinedMatrix.setConcat(ctm, srcToDst);
        if (can_ignore_bilerp_constraint(*producer, src, combinedMatrix, rtc->numSamples())) {
            constraintMode = GrTextureAdjuster::kNo_FilterConstraint;
        }
    }

    SkMatrix textureMatrix;
    if (canUseTextureCoordsAsLocalCoords) {
        textureMatrix = SkMatrix::I();
    } else {
        if (!srcToDst.invert(&textureMatrix)) {
            return;
        }
    }
    auto fp = producer->createFragmentProcessor(textureMatrix, src, constraintMode,
                                                coordsAllInsideSrcRect, filterMode);
    fp = GrColorSpaceXformEffect::Make(std::move(fp), producer->colorSpace(), producer->alphaType(),
                                       rtc->colorInfo().colorSpace());
    if (!fp) {
        return;
    }

    GrPaint grPaint;
    if (!SkPaintToGrPaintWithTexture(context, rtc->colorInfo(), paint, ctm, std::move(fp),
                                     producer->isAlphaOnly(), &grPaint)) {
        return;
    }

    if (!mf) {
        // Can draw the image directly (any mask filter on the paint was converted to an FP already)
        if (dstClip) {
            SkPoint srcClipPoints[4];
            SkPoint* srcClip = nullptr;
            if (canUseTextureCoordsAsLocalCoords) {
                // Calculate texture coordinates that match the dst clip
                GrMapRectPoints(dst, src, dstClip, srcClipPoints, 4);
                srcClip = srcClipPoints;
            }
            rtc->fillQuadWithEdgeAA(clip, std::move(grPaint), aa, aaFlags, ctm, dstClip, srcClip);
        } else {
            // Provide explicit texture coords when possible, otherwise rely on texture matrix
            rtc->fillRectWithEdgeAA(clip, std::move(grPaint), aa, aaFlags, ctm, dst,
                                    canUseTextureCoordsAsLocalCoords ? &src : nullptr);
        }
    } else {
        // Must draw the mask filter as a GrShape. For now, this loses the per-edge AA information
        // since it always draws with AA, but that is should not be noticeable since the mask filter
        // is probably a blur.
        GrShape shape;
        if (dstClip) {
            // Represent it as an SkPath formed from the dstClip
            SkPath path;
            path.addPoly(dstClip, 4, true);
            shape = GrShape(path);
        } else {
            shape = GrShape(dst);
        }

        GrBlurUtils::drawShapeWithMaskFilter(
                context, rtc, clip, shape, std::move(grPaint), ctm, mf);
    }
}

} // anonymous namespace

//////////////////////////////////////////////////////////////////////////////

void SkGpuDevice::drawImageQuad(const SkImage* image, const SkRect* srcRect, const SkRect* dstRect,
                                const SkPoint dstClip[4], GrAA aa, GrQuadAAFlags aaFlags,
                                const SkMatrix* preViewMatrix, const SkPaint& paint,
                                SkCanvas::SrcRectConstraint constraint) {
    SkRect src;
    SkRect dst;
    SkMatrix srcToDst;
    ImageDrawMode mode = optimize_sample_area(SkISize::Make(image->width(), image->height()),
                                              srcRect, dstRect, dstClip, &src, &dst, &srcToDst);
    if (mode == ImageDrawMode::kSkip) {
        return;
    }

    if (src.contains(image->bounds())) {
        constraint = SkCanvas::kFast_SrcRectConstraint;
    }
    // Depending on the nature of image, it can flow through more or less optimal pipelines
    bool useDecal = mode == ImageDrawMode::kDecal;
    bool attemptDrawTexture = !useDecal; // rtc->drawTexture() only clamps

    // Get final CTM matrix
    SkMatrix ctm = this->ctm();
    if (preViewMatrix) {
        ctm.preConcat(*preViewMatrix);
    }

    // YUVA images can be stored in multiple images with different plane resolutions, so this
    // uses an effect to combine them dynamically on the GPU. This is done before requesting a
    // pinned texture proxy because YUV images force-flatten to RGBA in that scenario.
    if (as_IB(image)->isYUVA()) {
        SK_HISTOGRAM_BOOLEAN("DrawTiled", false);
        LogDrawScaleFactor(ctm, srcToDst, paint.getFilterQuality());

        GrYUVAImageTextureMaker maker(fContext.get(), image, useDecal);
        draw_texture_producer(fContext.get(), fRenderTargetContext.get(), this->clip(), ctm,
                              paint, &maker, src, dst, dstClip, srcToDst, aa, aaFlags, constraint,
                              /* attempt draw texture */ false);
        return;
    }

    // Pinned texture proxies can be rendered directly as textures, or with relatively simple
    // adjustments applied to the image content (scaling, mipmaps, color space, etc.)
    uint32_t pinnedUniqueID;
    if (sk_sp<GrTextureProxy> proxy = as_IB(image)->refPinnedTextureProxy(this->context(),
                                                                          &pinnedUniqueID)) {
        SK_HISTOGRAM_BOOLEAN("DrawTiled", false);
        LogDrawScaleFactor(ctm, srcToDst, paint.getFilterQuality());

        SkAlphaType alphaType = image->alphaType();
        SkColorSpace* colorSpace = as_IB(image)->colorSpace();

        if (attemptDrawTexture && can_use_draw_texture(paint)) {
            draw_texture(fRenderTargetContext.get(), this->clip(), ctm, paint, src,  dst,
                         dstClip, aa, aaFlags, constraint, std::move(proxy), alphaType, colorSpace);
            return;
        }
        auto colorType = SkColorTypeToGrColorType(image->colorType());
        GrTextureAdjuster adjuster(fContext.get(), std::move(proxy), colorType, alphaType,
                                   pinnedUniqueID, colorSpace, useDecal);
        draw_texture_producer(fContext.get(), fRenderTargetContext.get(), this->clip(), ctm,
                              paint, &adjuster, src, dst, dstClip, srcToDst, aa, aaFlags,
                              constraint, /* attempt draw_texture */ false);
        return;
    }

    // Next up, try tiling the image
    // TODO (michaelludwig): Implement this with per-edge AA flags to handle seaming properly
    // instead of going through drawBitmapRect (which will be removed from SkDevice in the future)
    SkBitmap bm;
    if (this->shouldTileImage(image, &src, constraint, paint.getFilterQuality(), ctm, srcToDst)) {
        // only support tiling as bitmap at the moment, so force raster-version
        if (!as_IB(image)->getROPixels(&bm)) {
            return;
        }
        this->drawBitmapRect(bm, &src, dst, paint, constraint);
        return;
    }

    // This is the funnel for all non-tiled bitmap/image draw calls. Log a histogram entry.
    SK_HISTOGRAM_BOOLEAN("DrawTiled", false);
    LogDrawScaleFactor(ctm, srcToDst, paint.getFilterQuality());

    // Lazily generated images must get drawn as a texture producer that handles the final
    // texture creation.
    if (image->isLazyGenerated()) {
        GrImageTextureMaker maker(fContext.get(), image, SkImage::kAllow_CachingHint, useDecal);
        draw_texture_producer(fContext.get(), fRenderTargetContext.get(), this->clip(), ctm,
                              paint, &maker, src, dst, dstClip, srcToDst, aa, aaFlags, constraint,
                              attemptDrawTexture);
        return;
    }
    if (as_IB(image)->getROPixels(&bm)) {
        GrBitmapTextureMaker maker(fContext.get(), bm, useDecal);
        draw_texture_producer(fContext.get(), fRenderTargetContext.get(), this->clip(), ctm,
                              paint, &maker, src, dst, dstClip, srcToDst, aa, aaFlags, constraint,
                              attemptDrawTexture);
    }

    // Otherwise don't know how to draw it
}

void SkGpuDevice::drawEdgeAAImageSet(const SkCanvas::ImageSetEntry set[], int count,
                                     const SkPoint dstClips[], const SkMatrix preViewMatrices[],
                                     const SkPaint& paint, SkCanvas::SrcRectConstraint constraint) {
    SkASSERT(count > 0);
    if (!can_use_draw_texture(paint)) {
        // Send every entry through drawImageQuad() to handle the more complicated paint
        int dstClipIndex = 0;
        for (int i = 0; i < count; ++i) {
            // Only no clip or quad clip are supported
            SkASSERT(!set[i].fHasClip || dstClips);
            SkASSERT(set[i].fMatrixIndex < 0 || preViewMatrices);

            SkTCopyOnFirstWrite<SkPaint> entryPaint(paint);
            if (set[i].fAlpha != 1.f) {
                auto paintAlpha = paint.getAlphaf();
                entryPaint.writable()->setAlphaf(paintAlpha * set[i].fAlpha);
            }
            // Always send GrAA::kYes to preserve seaming across tiling in MSAA
            this->drawImageQuad(
                    set[i].fImage.get(), &set[i].fSrcRect, &set[i].fDstRect,
                    set[i].fHasClip ? dstClips + dstClipIndex : nullptr, GrAA::kYes,
                    SkToGrQuadAAFlags(set[i].fAAFlags),
                    set[i].fMatrixIndex < 0 ? nullptr : preViewMatrices + set[i].fMatrixIndex,
                    *entryPaint, constraint);
            dstClipIndex += 4 * set[i].fHasClip;
        }
        return;
    }

    GrSamplerState::Filter filter = kNone_SkFilterQuality == paint.getFilterQuality() ?
            GrSamplerState::Filter::kNearest : GrSamplerState::Filter::kBilerp;
    SkBlendMode mode = paint.getBlendMode();

    SkAutoTArray<GrRenderTargetContext::TextureSetEntry> textures(count);
    // We accumulate compatible proxies until we find an an incompatible one or reach the end and
    // issue the accumulated 'n' draws starting at 'base'.
    int base = 0, n = 0;
    auto draw = [&] {
        if (n > 0) {
            auto textureXform = GrColorSpaceXform::Make(
                    set[base].fImage->colorSpace(), set[base].fImage->alphaType(),
                    fRenderTargetContext->colorInfo().colorSpace(), kPremul_SkAlphaType);
            fRenderTargetContext->drawTextureSet(this->clip(), textures.get() + base, n,
                                                 filter, mode, GrAA::kYes, constraint, this->ctm(),
                                                 std::move(textureXform));
        }
    };
    int dstClipIndex = 0;
    for (int i = 0; i < count; ++i) {
        SkASSERT(!set[i].fHasClip || dstClips);
        SkASSERT(set[i].fMatrixIndex < 0 || preViewMatrices);

        // Manage the dst clip pointer tracking before any continues are used so we don't lose
        // our place in the dstClips array.
        const SkPoint* clip = set[i].fHasClip ? dstClips + dstClipIndex : nullptr;
        dstClipIndex += 4 * set[i].fHasClip;

        // The default SkBaseDevice implementation is based on drawImageRect which does not allow
        // non-sorted src rects. TODO: Decide this is OK or make sure we handle it.
        if (!set[i].fSrcRect.isSorted()) {
            draw();
            base = i + 1;
            n = 0;
            continue;
        }

        sk_sp<GrTextureProxy> proxy;
        const SkImage_Base* image = as_IB(set[i].fImage.get());
        // Extract proxy from image, but skip YUV images so they get processed through
        // drawImageQuad and the proper effect to dynamically sample their planes.
        if (!image->isYUVA()) {
            uint32_t uniqueID;
            proxy = image->refPinnedTextureProxy(this->context(), &uniqueID);
            if (!proxy) {
                proxy = image->asTextureProxyRef(this->context(), GrSamplerState::ClampBilerp(),
                                                 nullptr);
            }
        }

        if (!proxy) {
            // This image can't go through the texture op, send through general image pipeline
            // after flushing current batch.
            draw();
            base = i + 1;
            n = 0;
            SkTCopyOnFirstWrite<SkPaint> entryPaint(paint);
            if (set[i].fAlpha != 1.f) {
                auto paintAlpha = paint.getAlphaf();
                entryPaint.writable()->setAlphaf(paintAlpha * set[i].fAlpha);
            }
            this->drawImageQuad(
                    image, &set[i].fSrcRect, &set[i].fDstRect, clip, GrAA::kYes,
                    SkToGrQuadAAFlags(set[i].fAAFlags),
                    set[i].fMatrixIndex < 0 ? nullptr : preViewMatrices + set[i].fMatrixIndex,
                    *entryPaint, constraint);
            continue;
        }

        textures[i].fProxy = std::move(proxy);
        textures[i].fSrcRect = set[i].fSrcRect;
        textures[i].fDstRect = set[i].fDstRect;
        textures[i].fDstClipQuad = clip;
        textures[i].fPreViewMatrix =
                set[i].fMatrixIndex < 0 ? nullptr : preViewMatrices + set[i].fMatrixIndex;
        textures[i].fAlpha = set[i].fAlpha * paint.getAlphaf();
        textures[i].fAAFlags = SkToGrQuadAAFlags(set[i].fAAFlags);

        if (n > 0 &&
            (!GrTextureProxy::ProxiesAreCompatibleAsDynamicState(textures[i].fProxy.get(),
                                                                 textures[base].fProxy.get()) ||
             set[i].fImage->alphaType() != set[base].fImage->alphaType() ||
             !SkColorSpace::Equals(set[i].fImage->colorSpace(), set[base].fImage->colorSpace()))) {
            draw();
            base = i;
            n = 1;
        } else {
            ++n;
        }
    }
    draw();
}

// TODO (michaelludwig) - to be removed when drawBitmapRect doesn't need it anymore
void SkGpuDevice::drawTextureProducer(GrTextureProducer* producer,
                                      const SkRect* srcRect,
                                      const SkRect* dstRect,
                                      SkCanvas::SrcRectConstraint constraint,
                                      const SkMatrix& viewMatrix,
                                      const SkPaint& paint,
                                      bool attemptDrawTexture) {
    // The texture refactor split the old logic of drawTextureProducer into the beginning of
    // drawImageQuad() and into the static draw_texture_producer. Replicate necessary logic that
    // drawImageQuad() handles.
    SkRect src;
    SkRect dst;
    SkMatrix srcToDst;
    ImageDrawMode mode = optimize_sample_area(SkISize::Make(producer->width(), producer->height()),
                                              srcRect, dstRect, nullptr, &src, &dst, &srcToDst);
    if (mode == ImageDrawMode::kSkip) {
        return;
    }
    // There's no dstClip to worry about and the producer is already made so we wouldn't be able
    // to tell it to use decals if we had to
    SkASSERT(mode != ImageDrawMode::kDecal);

    draw_texture_producer(fContext.get(), fRenderTargetContext.get(), this->clip(), viewMatrix,
                          paint, producer, src, dst, /* clip */ nullptr, srcToDst,
                          GrAA(paint.isAntiAlias()),
                          paint.isAntiAlias() ? GrQuadAAFlags::kAll : GrQuadAAFlags::kNone,
                          constraint, attemptDrawTexture);
}
