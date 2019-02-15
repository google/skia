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
#include "GrShape.h"
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
 * Checks whether the paint is compatible with using GrRenderTargetContext::drawTexture. It is more
 * efficient than the GrTextureProducer general case.
 */
static bool can_use_draw_texture(const SkPaint& paint) {
    return (!paint.getColorFilter() && !paint.getShader() && !paint.getMaskFilter() &&
            !paint.getImageFilter() && paint.getFilterQuality() < kMedium_SkFilterQuality);
}

static void draw_texture(GrRenderTargetContext* rtc, const GrClip& clip, const SkMatrix& ctm,
                         const SkPaint& paint, const SkRect* src, const SkRect* dst,
                         const SkPoint dstClip[4], GrQuadAAFlags aaFlags,
                         SkCanvas::SrcRectConstraint constraint, sk_sp<GrTextureProxy> proxy,
                         SkAlphaType alphaType, SkColorSpace* colorSpace) {
    SkASSERT(!(SkToBool(src) && !SkToBool(dst)));

    SkRect srcRect = src ? *src : SkRect::MakeWH(proxy->width(), proxy->height());
    SkRect dstRect = dst ? *dst : srcRect;
    if (src && !SkRect::MakeIWH(proxy->width(), proxy->height()).contains(srcRect)) {
        // Shrink the src rect to be within bounds and proportionately shrink the dst rect.
        SkMatrix srcToDst;
        srcToDst.setRectToRect(srcRect, dstRect, SkMatrix::kFill_ScaleToFit);
        SkAssertResult(srcRect.intersect(SkRect::MakeIWH(proxy->width(), proxy->height())));
        srcToDst.mapRect(&dstRect, srcRect);
        // FIXME how do we modify the dstClip? not sure downscaling it is the equivalently correct operation
    }
    const GrColorSpaceInfo& dstInfo(rtc->colorSpaceInfo());
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
        color = SkColor4fPrepForDst(paint.getColor4f(), dstInfo, *rtc->caps()).premul();
    } else {
        float paintAlpha = paint.getColor4f().fA;
        color = { paintAlpha, paintAlpha, paintAlpha, paintAlpha };
    }

    GrAA aa(paint.isAntiAlias());
    if (dstClip) {
        // Get source coords corresponding to dstClip
        SkPoint srcQuad[4];
        GrMapRectPoints(dstRect, srcRect, dstClip, srcQuad, 4);

        SkRect* domain = nullptr;
        if (constraint == SkCanvas::kStrict_SrcRectConstraint) {
            domain = &srcRect;
        }
        rtc->drawTextureQuad(clip, std::move(proxy), filter, paint.getBlendMode(), color,
                             srcQuad, dstClip, aa, aaFlags, domain, ctm, std::move(textureXform));
    } else {
        rtc->drawTexture(clip, std::move(proxy), filter, paint.getBlendMode(), color, srcRect,
                         dstRect, aa, aaFlags, constraint, ctm, std::move(textureXform));
    }
}

static void draw_texture_producer(GrContext* context, GrRenderTargetContext* rtc,
                                  const GrClip& clip, const SkMatrix& ctm,
                                  const SkPaint& paint, GrTextureProducer* producer,
                                  const SkRect* srcRect, const SkRect* dstRect,
                                  const SkPoint dstClip[4], GrQuadAAFlags aaFlags,
                                  SkCanvas::SrcRectConstraint constraint, bool attemptDrawTexture) {
    if (attemptDrawTexture && can_use_draw_texture(paint)) {
        // We've done enough checks above to allow us to pass ClampNearest() and not check for
        // scaling adjustments.
        auto proxy = producer->refTextureProxyForParams(GrSamplerState::ClampNearest(), nullptr);
        if (!proxy) {
            return;
        }
        draw_texture(rtc, clip, ctm, paint, srcRect, dstRect, dstClip, aaFlags, constraint,
                     std::move(proxy), producer->alphaType(), producer->colorSpace());
        return;
    }

    // This is the funnel for all non-tiled bitmap/image draw calls. Log a histogram entry.
    SK_HISTOGRAM_BOOLEAN("DrawTiled", false);

    // Figure out the actual dst and src rect by clipping the src rect to the bounds of the
    // adjuster. If the src rect is clipped then the dst rect must be recomputed. Also determine
    // the matrix that maps the src rect to the dst rect.
    // FIXME adjust dstClip as well, somehow
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

    const SkMaskFilter* mf = paint.getMaskFilter();

    // The shader expects proper local coords, so we can't replace local coords with texture coords
    // if the shader will be used. If we have a mask filter we will change the underlying geometry
    // that is rendered.
    bool canUseTextureCoordsAsLocalCoords = !use_shader(producer->isAlphaOnly(), paint) && !mf;

    // Specifying the texture coords as local coordinates is an attempt to enable more GrDrawOp
    // combining by not baking anything about the srcRect, dstRect, or viewMatrix, into the texture
    // FP. In the future this should be an opaque optimization enabled by the combination of
    // GrDrawOp/GP and FP.
    if (mf && as_MFB(mf)->hasFragmentProcessor()) {
        mf = nullptr;
    }
    bool doBicubic;
    GrSamplerState::Filter fm = GrSkFilterQualityToGrFilterMode(
            paint.getFilterQuality(), viewMatrix, srcToDstMatrix,
            fContext->priv().options().fSharpenMipmappedTextures, &doBicubic);
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
    auto fp = producer->createFragmentProcessor(*textureMatrix, clippedSrcRect, constraintMode,
                                                coordsAllInsideSrcRect, filterMode);
    fp = GrColorSpaceXformEffect::Make(std::move(fp), producer->colorSpace(), producer->alphaType(),
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
        // FIXME if we can use texture coordinates, then it should be okay to calculate the src quad
        // from dstclip and use fillQuadToQuad. To prevent an API explosion, perhaps fillQuad should
        // just take optional local coordinates. Might to do the same for edgeAA rect drawing though
        fRenderTargetContext->fillRectToRect(this->clip(), std::move(grPaint), aa, viewMatrix,
                                             clippedDstRect, clippedSrcRect);
        return;
    }

    if (!mf) {
        // FIXME must use fillQuadWithEdgeAA or fillRectWithEdgeAA instead of this
        fRenderTargetContext->drawRect(this->clip(), std::move(grPaint), aa, viewMatrix,
                                       clippedDstRect);
        return;
    }


    // FIXME how do we handle the dst clips per aa edges? I can pass dstClip in as the shape ctor,
    // but then i'd lose aa.
    // I think the best option will be to add per-edge AA flags to GrShape since it already has
    // the rect option, and a quad option, then update the drawShape() fast paths in GrRTC to have
    // support for rect+edgeAA and quad+edgeAA
    GrShape shape(clippedDstRect, GrStyle::SimpleFill());
    GrBlurUtils::drawShapeWithMaskFilter(this->context(), fRenderTargetContext.get(), this->clip(),
                                         shape, std::move(grPaint), viewMatrix, mf);
}

//////////////////////////////////////////////////////////////////////////////

void SkGpuDevice::drawImageQuad(const SkImage* image, const SkRect* src, const SkRect& dst,
                                const SkPoint dstClip[4], GrQuadAAFlags aaFlags,
                                const SkPaint& paint, SkCanvas::SrcRectConstraint constraint) {
    if (!src || src->contains(image->bounds())) {
        constraint = SkCanvas::kFast_SrcRectConstraint;
    }
    // Depending on the nature of image, it can flow through more or less optimal pipelines

    // YUVA images can be stored in multiple images with different plane resolutions, so this
    // uses an effect to combine them dynamically on the GPU. This is done before requesting a
    // pinned texture proxy because YUV images force-flatten to RGBA in that scenario.
    if (as_IB(image)->isYUVA()) {
        GrYUVAImageTextureMaker maker(fContext.get(), image);
        draw_texture_producer(fContext.get(), fRenderTargetContext.get(), this->clip(), this->ctm(),
                              paint, &maker, src, &dst, dstClip, aaFlags, constraint, false);
        return;
    }

    // Pinned texture proxies can be rendered directly as textures, or with relatively simple
    // adjustments applied to the image content (scaling, mipmaps, color space, etc.)
    uint32_t pinnedUniqueID;
    if (sk_sp<GrTextureProxy> proxy = as_IB(image)->refPinnedTextureProxy(&pinnedUniqueID)) {
        SkAlphaType alphaType = image->alphaType();
        SkColorSpace* colorSpace = as_IB(image)->colorSpace();
        if (can_use_draw_texture(paint)) {
            draw_texture(fRenderTargetContext.get(), this->clip(), this->ctm(), paint, srcRect,
                         dstRect, dstClip, aaFlags, constraint, std::move(proxy),
                         alphaType, colorSpace);
            return;
        }

        GrTextureAdjuster adjuster(fContext.get(), std::move(proxy), alphaType, pinnedUniqueID,
                                   colorSpace);
        draw_texture_producer(fContext.get(), fRenderTargetContext.get(), this->clip(), this->ctm(),
                              paint, &adjuster, src, &dst, dstClip, aaFlags, constraint, false);
        return;
    }

    // Next up, try tiling the image
    // TODO (michaelludwig): Implement this with per-edge AA flags to handle seaming properly
    // instead of going through drawBitmapRect (which will be removed from SkDevice in the future)
    SkBitmap bm;
    SkMatrix srcToDstRect;
    srcToDstRect.setRectToRect((src ? *src : SkRect::MakeIWH(image->width(), image->height())),
                               dst, SkMatrix::kFill_ScaleToFit);
    if (this->shouldTileImage(image, src, constraint, paint.getFilterQuality(), this->ctm(),
                              srcToDstRect)) {
        // only support tiling as bitmap at the moment, so force raster-version
        if (!as_IB(image)->getROPixels(&bm)) {
            return;
        }
        this->drawBitmapRect(bm, src, dst, paint, constraint);
        return;
    }

    // Lazily generated images must get drawn as a texture producer that handles the final
    // texture creation.
    if (image->isLazyGenerated()) {
        GrImageTextureMaker maker(fContext.get(), image, SkImage::kAllow_CachingHint);
        draw_texture_producer(fContext.get(), fRenderTargetContext.get(), this->clip(), this->ctm(),
                              paint, &maker, src, &dst, dstClip, aaFlags, constraint, true);
        return;
    }
    if (as_IB(image)->getROPixels(&bm)) {
        GrBitmapTextureMaker maker(fContext.get(), bm);
        draw_texture_producer(fContext.get(), fRenderTargetContext.get(), this->clip(), this->ctm(),
                              paint, &maker, src, &dst, dstClip, aaFlags, constraint, true);
    }

    // Otherwise don't know how to draw it
}

void SkGpuDevice::tmp_drawImageSetV2(const SkCanvas::ImageSetEntry set[], int dstClipCounts[],
                                     int count, const SkPoint dstClips[], const SkPaint& paint,
                                     SrcRectConstraint constraint) {
    SkASSERT(count > 0);

    // FIXME analyze the paint to determine if using drawTextureSet is possible. It is allowed if
    // draw_texture is normally allowed (so this impl should be moved into SkGpuDevice_drawTexture).
    // Assuming that, we iterate as normal and configure the set entry to use the dst clip.
    // and we should also handle textures that aren't just pinned texture proxies, by passing them
    // to the drawTextureProducer

    GrSamplerState sampler;
    sampler.setFilterMode(kNone_SkFilterQuality == filterQuality ? GrSamplerState::Filter::kNearest
                                                                 : GrSamplerState::Filter::kBilerp);
    SkAutoTArray<GrRenderTargetContext::TextureSetEntry> textures(count);
    // We accumulate compatible proxies until we find an an incompatible one or reach the end and
    // issue the accumulated 'n' draws starting at 'base'.
    int base = 0, n = 0;
    auto draw = [&] {
        if (n > 0) {
            auto textureXform = GrColorSpaceXform::Make(
                    set[base].fImage->colorSpace(), set[base].fImage->alphaType(),
                    fRenderTargetContext->colorSpaceInfo().colorSpace(), kPremul_SkAlphaType);
            fRenderTargetContext->drawTextureSet(this->clip(), textures.get() + base, n,
                                                 sampler.filter(), mode, GrAA::kYes, this->ctm(),
                                                 std::move(textureXform));
        }
    };
    for (int i = 0; i < count; ++i) {
        // The default SkBaseDevice implementation is based on drawImageRect which does not allow
        // non-sorted src rects. TODO: Decide this is OK or make sure we handle it.
        if (!set[i].fSrcRect.isSorted()) {
            draw();
            base = i + 1;
            n = 0;
            continue;
        }
        uint32_t uniqueID;
        textures[i].fProxy = as_IB(set[i].fImage.get())->refPinnedTextureProxy(&uniqueID);
        if (!textures[i].fProxy) {
            textures[i].fProxy =
                    as_IB(set[i].fImage.get())
                            ->asTextureProxyRef(fContext.get(), GrSamplerState::ClampBilerp(),
                                                nullptr);
            // If we failed to make a proxy then flush the accumulated set and reset for the next
            // image.
            if (!textures[i].fProxy) {
                draw();
                base = i + 1;
                n = 0;
                continue;
            }
        }
        textures[i].fSrcRect = set[i].fSrcRect;
        textures[i].fDstRect = set[i].fDstRect;
        textures[i].fDstClip = nullptr; // TODO(michaelludwig) Not exposed in SkGpuDevice API yet
        textures[i].fAlpha = set[i].fAlpha;
        textures[i].fAAFlags = SkToGrQuadAAFlags(set[i].fAAFlags);
        // Not enabled yet at this API level
        textures[i].fDstClip = nullptr;
        textures[i].fDstClipCount = 0;
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
