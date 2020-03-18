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
#include "src/gpu/effects/GrTextureEffect.h"
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

//////////////////////////////////////////////////////////////////////////////
//  Helper functions for tiling a large SkBitmap

static const int kBmpSmallTileSize = 1 << 10;

static inline int get_tile_count(const SkIRect& srcRect, int tileSize)  {
    int tilesX = (srcRect.fRight / tileSize) - (srcRect.fLeft / tileSize) + 1;
    int tilesY = (srcRect.fBottom / tileSize) - (srcRect.fTop / tileSize) + 1;
    return tilesX * tilesY;
}

static int determine_tile_size(const SkIRect& src, int maxTileSize) {
    if (maxTileSize <= kBmpSmallTileSize) {
        return maxTileSize;
    }

    size_t maxTileTotalTileSize = get_tile_count(src, maxTileSize);
    size_t smallTotalTileSize = get_tile_count(src, kBmpSmallTileSize);

    maxTileTotalTileSize *= maxTileSize * maxTileSize;
    smallTotalTileSize *= kBmpSmallTileSize * kBmpSmallTileSize;

    if (maxTileTotalTileSize > 2 * smallTotalTileSize) {
        return kBmpSmallTileSize;
    } else {
        return maxTileSize;
    }
}

// Given a bitmap, an optional src rect, and a context with a clip and matrix determine what
// pixels from the bitmap are necessary.
static void determine_clipped_src_rect(int width, int height,
                                       const GrClip& clip,
                                       const SkMatrix& viewMatrix,
                                       const SkMatrix& srcToDstRect,
                                       const SkISize& imageDimensions,
                                       const SkRect* srcRectPtr,
                                       SkIRect* clippedSrcIRect) {
    clip.getConservativeBounds(width, height, clippedSrcIRect, nullptr);
    SkMatrix inv = SkMatrix::Concat(viewMatrix, srcToDstRect);
    if (!inv.invert(&inv)) {
        clippedSrcIRect->setEmpty();
        return;
    }
    SkRect clippedSrcRect = SkRect::Make(*clippedSrcIRect);
    inv.mapRect(&clippedSrcRect);
    if (srcRectPtr) {
        if (!clippedSrcRect.intersect(*srcRectPtr)) {
            clippedSrcIRect->setEmpty();
            return;
        }
    }
    clippedSrcRect.roundOut(clippedSrcIRect);
    SkIRect bmpBounds = SkIRect::MakeSize(imageDimensions);
    if (!clippedSrcIRect->intersect(bmpBounds)) {
        clippedSrcIRect->setEmpty();
    }
}

// tileSize and clippedSubset are valid if true is returned
static bool should_tile_image_id(GrContext* context,
                                 SkISize rtSize,
                                 const GrClip& clip,
                                 uint32_t imageID,
                                 const SkISize& imageSize,
                                 const SkMatrix& ctm,
                                 const SkMatrix& srcToDst,
                                 const SkRect* src,
                                 int maxTileSize,
                                 int* tileSize,
                                 SkIRect* clippedSubset) {
    // if it's larger than the max tile size, then we have no choice but tiling.
    if (imageSize.width() > maxTileSize || imageSize.height() > maxTileSize) {
        determine_clipped_src_rect(rtSize.width(), rtSize.height(), clip, ctm, srcToDst,
                                   imageSize, src, clippedSubset);
        *tileSize = determine_tile_size(*clippedSubset, maxTileSize);
        return true;
    }

    // If the image would only produce 4 tiles of the smaller size, don't bother tiling it.
    const size_t area = imageSize.width() * imageSize.height();
    if (area < 4 * kBmpSmallTileSize * kBmpSmallTileSize) {
        return false;
    }

    // At this point we know we could do the draw by uploading the entire bitmap
    // as a texture. However, if the texture would be large compared to the
    // cache size and we don't require most of it for this draw then tile to
    // reduce the amount of upload and cache spill.

    // assumption here is that sw bitmap size is a good proxy for its size as
    // a texture
    size_t bmpSize = area * sizeof(SkPMColor);  // assume 32bit pixels
    size_t cacheSize = context->getResourceCacheLimit();
    if (bmpSize < cacheSize / 2) {
        return false;
    }

    // Figure out how much of the src we will need based on the src rect and clipping. Reject if
    // tiling memory savings would be < 50%.
    determine_clipped_src_rect(rtSize.width(), rtSize.height(), clip, ctm, srcToDst, imageSize, src,
                               clippedSubset);
    *tileSize = kBmpSmallTileSize; // already know whole bitmap fits in one max sized tile.
    size_t usedTileBytes = get_tile_count(*clippedSubset, kBmpSmallTileSize) *
                           kBmpSmallTileSize * kBmpSmallTileSize *
                           sizeof(SkPMColor);  // assume 32bit pixels;

    return usedTileBytes * 2 < bmpSize;
}

// This method outsets 'iRect' by 'outset' all around and then clamps its extents to
// 'clamp'. 'offset' is adjusted to remain positioned over the top-left corner
// of 'iRect' for all possible outsets/clamps.
static inline void clamped_outset_with_offset(SkIRect* iRect, int outset, SkPoint* offset,
                                              const SkIRect& clamp) {
    iRect->outset(outset, outset);

    int leftClampDelta = clamp.fLeft - iRect->fLeft;
    if (leftClampDelta > 0) {
        offset->fX -= outset - leftClampDelta;
        iRect->fLeft = clamp.fLeft;
    } else {
        offset->fX -= outset;
    }

    int topClampDelta = clamp.fTop - iRect->fTop;
    if (topClampDelta > 0) {
        offset->fY -= outset - topClampDelta;
        iRect->fTop = clamp.fTop;
    } else {
        offset->fY -= outset;
    }

    if (iRect->fRight > clamp.fRight) {
        iRect->fRight = clamp.fRight;
    }
    if (iRect->fBottom > clamp.fBottom) {
        iRect->fBottom = clamp.fBottom;
    }
}

//////////////////////////////////////////////////////////////////////////////
//  Helper functions for drawing an image with GrRenderTargetContext

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
                         SkCanvas::SrcRectConstraint constraint, GrSurfaceProxyView view,
                         const GrColorInfo& srcColorInfo) {
    const GrColorInfo& dstInfo(rtc->colorInfo());
    auto textureXform =
        GrColorSpaceXform::Make(srcColorInfo.colorSpace(), srcColorInfo.alphaType(),
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
    GrSurfaceProxy* proxy = view.proxy();
    // Must specify the strict constraint when the proxy is not functionally exact and the src
    // rect would access pixels outside the proxy's content area without the constraint.
    if (constraint != SkCanvas::kStrict_SrcRectConstraint && !proxy->isFunctionallyExact()) {
        // Conservative estimate of how much a coord could be outset from src rect:
        // 1/2 pixel for AA and 1/2 pixel for bilerp
        float buffer = 0.5f * (aa == GrAA::kYes) +
                       0.5f * (filter == GrSamplerState::Filter::kBilerp);
        SkRect safeBounds = proxy->getBoundsRect();
        safeBounds.inset(buffer, buffer);
        if (!safeBounds.contains(srcRect)) {
            constraint = SkCanvas::kStrict_SrcRectConstraint;
        }
    }
    SkPMColor4f color;
    if (GrColorTypeIsAlphaOnly(srcColorInfo.colorType())) {
        color = SkColor4fPrepForDst(paint.getColor4f(), dstInfo).premul();
    } else {
        float paintAlpha = paint.getColor4f().fA;
        color = { paintAlpha, paintAlpha, paintAlpha, paintAlpha };
    }

    if (dstClip) {
        // Get source coords corresponding to dstClip
        SkPoint srcQuad[4];
        GrMapRectPoints(dstRect, srcRect, dstClip, srcQuad, 4);

        rtc->drawTextureQuad(clip, std::move(view), srcColorInfo.colorType(),
                             srcColorInfo.alphaType(), filter, paint.getBlendMode(), color, srcQuad,
                             dstClip, aa, aaFlags,
                             constraint == SkCanvas::kStrict_SrcRectConstraint ? &srcRect : nullptr,
                             ctm, std::move(textureXform));
    } else {
        rtc->drawTexture(clip, std::move(view), srcColorInfo.alphaType(), filter,
                         paint.getBlendMode(), color, srcRect, dstRect, aa, aaFlags, constraint,
                         ctm, std::move(textureXform));
    }
}

// Assumes srcRect and dstRect have already been optimized to fit the proxy.
static void draw_texture_producer(GrContext* context,
                                  GrRenderTargetContext* rtc,
                                  const GrClip& clip,
                                  const SkMatrix& ctm,
                                  const SkPaint& paint,
                                  GrTextureProducer* producer,
                                  const SkRect& src,
                                  const SkRect& dst,
                                  const SkPoint dstClip[4],
                                  const SkMatrix& srcToDst,
                                  GrAA aa,
                                  GrQuadAAFlags aaFlags,
                                  SkCanvas::SrcRectConstraint constraint,
                                  GrSamplerState::WrapMode wm,
                                  GrSamplerState::Filter fm,
                                  bool doBicubic) {
    if (wm == GrSamplerState::WrapMode::kClamp && !producer->isPlanar() &&
        can_use_draw_texture(paint)) {
        // We've done enough checks above to allow us to pass ClampNearest() and not check for
        // scaling adjustments.
        auto view = producer->view(GrMipMapped::kNo);
        if (!view) {
            return;
        }

        draw_texture(
                rtc, clip, ctm, paint, src, dst, dstClip, aa, aaFlags, constraint, std::move(view),
                {producer->colorType(), producer->alphaType(), sk_ref_sp(producer->colorSpace())});
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
        !producer->isPlanar()) {
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
                                                coordsAllInsideSrcRect, wm, wm, filterMode);
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

void draw_tiled_bitmap(GrContext* context,
                       GrRenderTargetContext* rtc,
                       const GrClip& clip,
                       const SkBitmap& bitmap,
                       int tileSize,
                       const SkMatrix& ctm,
                       const SkMatrix& srcToDst,
                       const SkRect& srcRect,
                       const SkIRect& clippedSrcIRect,
                       const SkPaint& paint,
                       GrAA aa,
                       SkCanvas::SrcRectConstraint constraint,
                       GrSamplerState::WrapMode wm,
                       GrSamplerState::Filter fm,
                       bool doBicubic) {
    SkRect clippedSrcRect = SkRect::Make(clippedSrcIRect);

    int nx = bitmap.width() / tileSize;
    int ny = bitmap.height() / tileSize;

    for (int x = 0; x <= nx; x++) {
        for (int y = 0; y <= ny; y++) {
            SkRect tileR;
            tileR.setLTRB(SkIntToScalar(x * tileSize),       SkIntToScalar(y * tileSize),
                          SkIntToScalar((x + 1) * tileSize), SkIntToScalar((y + 1) * tileSize));

            if (!SkRect::Intersects(tileR, clippedSrcRect)) {
                continue;
            }

            if (!tileR.intersect(srcRect)) {
                continue;
            }

            SkIRect iTileR;
            tileR.roundOut(&iTileR);
            SkVector offset = SkPoint::Make(SkIntToScalar(iTileR.fLeft),
                                            SkIntToScalar(iTileR.fTop));
            SkRect rectToDraw = tileR;
            srcToDst.mapRect(&rectToDraw);
            if (fm != GrSamplerState::Filter::kNearest || doBicubic) {
                SkIRect iClampRect;

                if (SkCanvas::kFast_SrcRectConstraint == constraint) {
                    // In bleed mode we want to always expand the tile on all edges
                    // but stay within the bitmap bounds
                    iClampRect = SkIRect::MakeWH(bitmap.width(), bitmap.height());
                } else {
                    // In texture-domain/clamp mode we only want to expand the
                    // tile on edges interior to "srcRect" (i.e., we want to
                    // not bleed across the original clamped edges)
                    srcRect.roundOut(&iClampRect);
                }
                int outset = doBicubic ? GrBicubicEffect::kFilterTexelPad : 1;
                clamped_outset_with_offset(&iTileR, outset, &offset, iClampRect);
            }

            SkBitmap tmpB;
            if (bitmap.extractSubset(&tmpB, iTileR)) {
                // We should have already handled bitmaps larger than the max texture size.
                SkASSERT(tmpB.width() <= context->priv().caps()->maxTextureSize() &&
                         tmpB.height() <= context->priv().caps()->maxTextureSize());
                // We should be respecting the max tile size by the time we get here.
                SkASSERT(tmpB.width() <= context->priv().caps()->maxTileSize() &&
                         tmpB.height() <= context->priv().caps()->maxTileSize());

                GrBitmapTextureMaker tileProducer(context, tmpB, GrImageTexGenPolicy::kDraw);

                GrQuadAAFlags aaFlags = GrQuadAAFlags::kNone;
                if (aa == GrAA::kYes) {
                    // If the entire bitmap was anti-aliased, turn on AA for the outside tile edges.
                    if (tileR.fLeft <= srcRect.fLeft) {
                        aaFlags |= GrQuadAAFlags::kLeft;
                    }
                    if (tileR.fRight >= srcRect.fRight) {
                        aaFlags |= GrQuadAAFlags::kRight;
                    }
                    if (tileR.fTop <= srcRect.fTop) {
                        aaFlags |= GrQuadAAFlags::kTop;
                    }
                    if (tileR.fBottom >= srcRect.fBottom) {
                        aaFlags |= GrQuadAAFlags::kBottom;
                    }
                }

                // now offset it to make it "local" to our tmp bitmap
                tileR.offset(-offset.fX, -offset.fY);
                SkMatrix offsetSrcToDst = srcToDst;
                offsetSrcToDst.preTranslate(offset.fX, offset.fY);

                draw_texture_producer(context, rtc, clip, ctm, paint, &tileProducer, tileR,
                                      rectToDraw, nullptr, offsetSrcToDst, aa, aaFlags, constraint,
                                      wm, fm, doBicubic);
            }
        }
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
    GrSamplerState::WrapMode wrapMode = mode == ImageDrawMode::kDecal
                                                ? GrSamplerState::WrapMode::kClampToBorder
                                                : GrSamplerState::WrapMode::kClamp;

    // Get final CTM matrix
    SkMatrix ctm = this->localToDevice();
    if (preViewMatrix) {
        ctm.preConcat(*preViewMatrix);
    }

    bool doBicubic;
    GrSamplerState::Filter fm = GrSkFilterQualityToGrFilterMode(
            image->width(), image->height(), paint.getFilterQuality(), ctm, srcToDst,
            fContext->priv().options().fSharpenMipmappedTextures, &doBicubic);

    auto clip = this->clip();

    // YUVA images can be stored in multiple images with different plane resolutions, so this
    // uses an effect to combine them dynamically on the GPU. This is done before requesting a
    // pinned texture proxy because YUV images force-flatten to RGBA in that scenario.
    if (as_IB(image)->isYUVA()) {
        SK_HISTOGRAM_BOOLEAN("DrawTiled", false);
        LogDrawScaleFactor(ctm, srcToDst, paint.getFilterQuality());

        GrYUVAImageTextureMaker maker(fContext.get(), image);
        draw_texture_producer(fContext.get(), fRenderTargetContext.get(), clip, ctm, paint,
                              &maker, src, dst, dstClip, srcToDst, aa, aaFlags, constraint,
                              wrapMode, fm, doBicubic);
        return;
    }

    // Pinned texture proxies can be rendered directly as textures, or with relatively simple
    // adjustments applied to the image content (scaling, mipmaps, color space, etc.)
    uint32_t pinnedUniqueID;
    if (GrSurfaceProxyView view = as_IB(image)->refPinnedView(this->context(), &pinnedUniqueID)) {
        SK_HISTOGRAM_BOOLEAN("DrawTiled", false);
        LogDrawScaleFactor(ctm, srcToDst, paint.getFilterQuality());

        GrColorInfo colorInfo;
        if (fContext->priv().caps()->isFormatSRGB(view.proxy()->backendFormat())) {
            SkASSERT(image->imageInfo().colorType() == kRGBA_8888_SkColorType);
            colorInfo = GrColorInfo(GrColorType::kRGBA_8888_SRGB, image->imageInfo().alphaType(),
                                    image->imageInfo().refColorSpace());
        } else {
            colorInfo = GrColorInfo(image->imageInfo().colorInfo());
        }

        GrTextureAdjuster adjuster(fContext.get(), std::move(view), colorInfo, pinnedUniqueID);
        draw_texture_producer(fContext.get(), fRenderTargetContext.get(), clip, ctm, paint,
                              &adjuster, src, dst, dstClip, srcToDst, aa, aaFlags, constraint,
                              wrapMode, fm, doBicubic);
        return;
    }

    // Next up, determine if the image must be tiled
    {
        // If image is explicitly already texture backed then we shouldn't get here.
        SkASSERT(!image->isTextureBacked());

        int tileFilterPad;
        if (doBicubic) {
            tileFilterPad = GrBicubicEffect::kFilterTexelPad;
        } else if (GrSamplerState::Filter::kNearest == fm) {
            tileFilterPad = 0;
        } else {
            tileFilterPad = 1;
        }
        int maxTileSize = fContext->priv().caps()->maxTileSize() - 2 * tileFilterPad;
        int tileSize;
        SkIRect clippedSubset;
        if (should_tile_image_id(fContext.get(), SkISize::Make(fRenderTargetContext->width(),
                                                               fRenderTargetContext->height()),
                                 clip, image->unique(), image->dimensions(), ctm, srcToDst, &src,
                                 maxTileSize, &tileSize, &clippedSubset)) {
            // Extract pixels on the CPU, since we have to split into separate textures before
            // sending to the GPU.
            SkBitmap bm;
            if (as_IB(image)->getROPixels(&bm)) {
                // This is the funnel for all paths that draw tiled bitmaps/images. Log histogram
                SK_HISTOGRAM_BOOLEAN("DrawTiled", true);
                LogDrawScaleFactor(ctm, srcToDst, paint.getFilterQuality());
                draw_tiled_bitmap(fContext.get(), fRenderTargetContext.get(), clip, bm, tileSize,
                                  ctm, srcToDst, src, clippedSubset, paint, aa, constraint,
                                  wrapMode, fm, doBicubic);
                return;
            }
        }
    }

    // This is the funnel for all non-tiled bitmap/image draw calls. Log a histogram entry.
    SK_HISTOGRAM_BOOLEAN("DrawTiled", false);
    LogDrawScaleFactor(ctm, srcToDst, paint.getFilterQuality());

    // Lazily generated images must get drawn as a texture producer that handles the final
    // texture creation.
    if (image->isLazyGenerated()) {
        GrImageTextureMaker maker(fContext.get(), image, GrImageTexGenPolicy::kDraw);
        draw_texture_producer(fContext.get(), fRenderTargetContext.get(), clip, ctm, paint,
                              &maker, src, dst, dstClip, srcToDst, aa, aaFlags, constraint,
                              wrapMode, fm, doBicubic);
        return;
    }

    SkBitmap bm;
    if (as_IB(image)->getROPixels(&bm)) {
        GrBitmapTextureMaker maker(fContext.get(), bm, GrImageTexGenPolicy::kDraw);
        draw_texture_producer(fContext.get(), fRenderTargetContext.get(), clip, ctm, paint,
                              &maker, src, dst, dstClip, srcToDst, aa, aaFlags, constraint,
                              wrapMode, fm, doBicubic);
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
    // issue the accumulated 'n' draws starting at 'base'. 'p' represents the number of proxy
    // switches that occur within the 'n' entries.
    int base = 0, n = 0, p = 0;
    auto draw = [&](int nextBase) {
        if (n > 0) {
            auto textureXform = GrColorSpaceXform::Make(
                    set[base].fImage->colorSpace(), set[base].fImage->alphaType(),
                    fRenderTargetContext->colorInfo().colorSpace(), kPremul_SkAlphaType);
            fRenderTargetContext->drawTextureSet(this->clip(), textures.get() + base, n, p,
                                                 filter, mode, GrAA::kYes, constraint,
                                                 this->localToDevice(), std::move(textureXform));
        }
        base = nextBase;
        n = 0;
        p = 0;
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
            draw(i + 1);
            continue;
        }

        GrSurfaceProxyView view;
        const SkImage_Base* image = as_IB(set[i].fImage.get());
        // Extract view from image, but skip YUV images so they get processed through
        // drawImageQuad and the proper effect to dynamically sample their planes.
        if (!image->isYUVA()) {
            uint32_t uniqueID;
            view = image->refPinnedView(this->context(), &uniqueID);
            if (!view) {
                view = image->refView(this->context(), GrMipMapped::kNo);
            }
        }

        if (!view) {
            // This image can't go through the texture op, send through general image pipeline
            // after flushing current batch.
            draw(i + 1);
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

        textures[i].fProxyView = std::move(view);
        textures[i].fSrcAlphaType = image->alphaType();
        textures[i].fSrcRect = set[i].fSrcRect;
        textures[i].fDstRect = set[i].fDstRect;
        textures[i].fDstClipQuad = clip;
        textures[i].fPreViewMatrix =
                set[i].fMatrixIndex < 0 ? nullptr : preViewMatrices + set[i].fMatrixIndex;
        textures[i].fAlpha = set[i].fAlpha * paint.getAlphaf();
        textures[i].fAAFlags = SkToGrQuadAAFlags(set[i].fAAFlags);

        if (n > 0 &&
            (!GrTextureProxy::ProxiesAreCompatibleAsDynamicState(
                    textures[i].fProxyView.proxy(),
                    textures[base].fProxyView.proxy()) ||
             textures[i].fProxyView.swizzle() != textures[base].fProxyView.swizzle() ||
             set[i].fImage->alphaType() != set[base].fImage->alphaType() ||
             !SkColorSpace::Equals(set[i].fImage->colorSpace(), set[base].fImage->colorSpace()))) {
            draw(i);
        }
        // Whether or not we submitted a draw in the above if(), this ith entry is in the current
        // set being accumulated so increment n, and increment p if proxies are different.
        ++n;
        if (n == 1 || textures[i - 1].fProxyView.proxy() != textures[i].fProxyView.proxy()) {
            // First proxy or a different proxy (that is compatible, otherwise we'd have drawn up
            // to i - 1).
            ++p;
        }
    }
    draw(count);
}
