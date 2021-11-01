/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/v1/Device_v1.h"

#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/private/SkTPin.h"
#include "src/core/SkDraw.h"
#include "src/core/SkImagePriv.h"
#include "src/core/SkMaskFilterBase.h"
#include "src/core/SkSpecialImage.h"
#include "src/gpu/GrBlurUtils.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrColorSpaceXform.h"
#include "src/gpu/GrOpsTypes.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrStyle.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrBicubicEffect.h"
#include "src/gpu/effects/GrBlendFragmentProcessor.h"
#include "src/gpu/effects/GrTextureEffect.h"
#include "src/gpu/geometry/GrRect.h"
#include "src/gpu/geometry/GrStyledShape.h"
#include "src/gpu/v1/SurfaceDrawContext_v1.h"
#include "src/image/SkImage_Base.h"
#include "src/image/SkImage_Gpu.h"

namespace {

inline bool use_shader(bool textureIsAlphaOnly, const SkPaint& paint) {
    return textureIsAlphaOnly && paint.getShader();
}

//////////////////////////////////////////////////////////////////////////////
//  Helper functions for dropping src rect subset with GrSamplerState::Filter::kLinear.

static const SkScalar kColorBleedTolerance = 0.001f;

bool has_aligned_samples(const SkRect& srcRect, const SkRect& transformedRect) {
    // detect pixel disalignment
    if (SkScalarAbs(SkScalarRoundToScalar(transformedRect.left()) - transformedRect.left()) < kColorBleedTolerance &&
        SkScalarAbs(SkScalarRoundToScalar(transformedRect.top())  - transformedRect.top())  < kColorBleedTolerance &&
        SkScalarAbs(transformedRect.width()  - srcRect.width())  < kColorBleedTolerance &&
        SkScalarAbs(transformedRect.height() - srcRect.height()) < kColorBleedTolerance) {
        return true;
    }
    return false;
}

bool may_color_bleed(const SkRect& srcRect,
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

bool can_ignore_linear_filtering_subset(const SkRect& srcSubset,
                                        const SkMatrix& srcRectToDeviceSpace,
                                        int numSamples) {
    if (srcRectToDeviceSpace.rectStaysRect()) {
        // sampling is axis-aligned
        SkRect transformedRect;
        srcRectToDeviceSpace.mapRect(&transformedRect, srcSubset);

        if (has_aligned_samples(srcSubset, transformedRect) ||
            !may_color_bleed(srcSubset, transformedRect, srcRectToDeviceSpace, numSamples)) {
            return true;
        }
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////////
//  Helper functions for tiling a large SkBitmap

static const int kBmpSmallTileSize = 1 << 10;

inline int get_tile_count(const SkIRect& srcRect, int tileSize)  {
    int tilesX = (srcRect.fRight / tileSize) - (srcRect.fLeft / tileSize) + 1;
    int tilesY = (srcRect.fBottom / tileSize) - (srcRect.fTop / tileSize) + 1;
    return tilesX * tilesY;
}

int determine_tile_size(const SkIRect& src, int maxTileSize) {
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
SkIRect determine_clipped_src_rect(int width, int height,
                                   const GrClip* clip,
                                   const SkMatrix& viewMatrix,
                                   const SkMatrix& srcToDstRect,
                                   const SkISize& imageDimensions,
                                   const SkRect* srcRectPtr) {
    SkIRect clippedSrcIRect = clip ? clip->getConservativeBounds()
                                   : SkIRect::MakeWH(width, height);
    SkMatrix inv = SkMatrix::Concat(viewMatrix, srcToDstRect);
    if (!inv.invert(&inv)) {
        return SkIRect::MakeEmpty();
    }
    SkRect clippedSrcRect = SkRect::Make(clippedSrcIRect);
    inv.mapRect(&clippedSrcRect);
    if (srcRectPtr) {
        if (!clippedSrcRect.intersect(*srcRectPtr)) {
            return SkIRect::MakeEmpty();
        }
    }
    clippedSrcRect.roundOut(&clippedSrcIRect);
    SkIRect bmpBounds = SkIRect::MakeSize(imageDimensions);
    if (!clippedSrcIRect.intersect(bmpBounds)) {
        return SkIRect::MakeEmpty();
    }

    return clippedSrcIRect;
}

// tileSize and clippedSubset are valid if true is returned
bool should_tile_image_id(GrRecordingContext* context,
                          SkISize rtSize,
                          const GrClip* clip,
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
        *clippedSubset = determine_clipped_src_rect(rtSize.width(), rtSize.height(), clip, ctm,
                                                    srcToDst, imageSize, src);
        *tileSize = determine_tile_size(*clippedSubset, maxTileSize);
        return true;
    }

    // If the image would only produce 4 tiles of the smaller size, don't bother tiling it.
    const size_t area = imageSize.width() * imageSize.height();
    if (area < 4 * kBmpSmallTileSize * kBmpSmallTileSize) {
        return false;
    }

    // At this point we know we could do the draw by uploading the entire bitmap as a texture.
    // However, if the texture would be large compared to the cache size and we don't require most
    // of it for this draw then tile to reduce the amount of upload and cache spill.
    // NOTE: if the context is not a direct context, it doesn't have access to the resource cache,
    // and theoretically, the resource cache's limits could be being changed on another thread, so
    // even having access to just the limit wouldn't be a reliable test during recording here.
    // Instead, we will just upload the entire image to be on the safe side and not tile.
    auto direct = context->asDirectContext();
    if (!direct) {
        return false;
    }

    // assumption here is that sw bitmap size is a good proxy for its size as
    // a texture
    size_t bmpSize = area * sizeof(SkPMColor);  // assume 32bit pixels
    size_t cacheSize = direct->getResourceCacheLimit();
    if (bmpSize < cacheSize / 2) {
        return false;
    }

    // Figure out how much of the src we will need based on the src rect and clipping. Reject if
    // tiling memory savings would be < 50%.
    *clippedSubset = determine_clipped_src_rect(rtSize.width(), rtSize.height(), clip, ctm,
                                                srcToDst, imageSize, src);
    *tileSize = kBmpSmallTileSize; // already know whole bitmap fits in one max sized tile.
    size_t usedTileBytes = get_tile_count(*clippedSubset, kBmpSmallTileSize) *
                           kBmpSmallTileSize * kBmpSmallTileSize *
                           sizeof(SkPMColor);  // assume 32bit pixels;

    return usedTileBytes * 2 < bmpSize;
}

// This method outsets 'iRect' by 'outset' all around and then clamps its extents to
// 'clamp'. 'offset' is adjusted to remain positioned over the top-left corner
// of 'iRect' for all possible outsets/clamps.
inline void clamped_outset_with_offset(SkIRect* iRect, int outset, SkPoint* offset,
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
//  Helper functions for drawing an image with v1::SurfaceDrawContext

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
ImageDrawMode optimize_sample_area(const SkISize& image, const SkRect* origSrcRect,
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
        *srcToDst = SkMatrix::RectToRect(src, dst);
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
 * Checks whether the paint is compatible with using SurfaceDrawContext::drawTexture. It is more
 * efficient than the SkImage general case.
 */
bool can_use_draw_texture(const SkPaint& paint, bool useCubicResampler, SkMipmapMode mm) {
    return (!paint.getColorFilter() && !paint.getShader() && !paint.getMaskFilter() &&
            !paint.getImageFilter() && !paint.getBlender() && !useCubicResampler &&
            mm == SkMipmapMode::kNone);
}

SkPMColor4f texture_color(SkColor4f paintColor, float entryAlpha, GrColorType srcColorType,
                          const GrColorInfo& dstColorInfo) {
    paintColor.fA *= entryAlpha;
    if (GrColorTypeIsAlphaOnly(srcColorType)) {
        return SkColor4fPrepForDst(paintColor, dstColorInfo).premul();
    } else {
        float paintAlpha = SkTPin(paintColor.fA, 0.f, 1.f);
        return { paintAlpha, paintAlpha, paintAlpha, paintAlpha };
    }
}

// Assumes srcRect and dstRect have already been optimized to fit the proxy
void draw_texture(skgpu::v1::SurfaceDrawContext* sdc,
                  const GrClip* clip,
                  const SkMatrix& ctm,
                  const SkPaint& paint,
                  GrSamplerState::Filter filter,
                  const SkRect& srcRect,
                  const SkRect& dstRect,
                  const SkPoint dstClip[4],
                  GrAA aa,
                  GrQuadAAFlags aaFlags,
                  SkCanvas::SrcRectConstraint constraint,
                  GrSurfaceProxyView view,
                  const GrColorInfo& srcColorInfo) {
    if (GrColorTypeIsAlphaOnly(srcColorInfo.colorType())) {
        view.concatSwizzle(GrSwizzle("aaaa"));
    }
    const GrColorInfo& dstInfo = sdc->colorInfo();
    auto textureXform = GrColorSpaceXform::Make(srcColorInfo, sdc->colorInfo());
    GrSurfaceProxy* proxy = view.proxy();
    // Must specify the strict constraint when the proxy is not functionally exact and the src
    // rect would access pixels outside the proxy's content area without the constraint.
    if (constraint != SkCanvas::kStrict_SrcRectConstraint && !proxy->isFunctionallyExact()) {
        // Conservative estimate of how much a coord could be outset from src rect:
        // 1/2 pixel for AA and 1/2 pixel for linear filtering
        float buffer = 0.5f * (aa == GrAA::kYes) +
                       0.5f * (filter == GrSamplerState::Filter::kLinear);
        SkRect safeBounds = proxy->getBoundsRect();
        safeBounds.inset(buffer, buffer);
        if (!safeBounds.contains(srcRect)) {
            constraint = SkCanvas::kStrict_SrcRectConstraint;
        }
    }

    SkPMColor4f color = texture_color(paint.getColor4f(), 1.f, srcColorInfo.colorType(), dstInfo);
    if (dstClip) {
        // Get source coords corresponding to dstClip
        SkPoint srcQuad[4];
        GrMapRectPoints(dstRect, srcRect, dstClip, srcQuad, 4);

        sdc->drawTextureQuad(clip,
                             std::move(view),
                             srcColorInfo.colorType(),
                             srcColorInfo.alphaType(),
                             filter,
                             GrSamplerState::MipmapMode::kNone,
                             paint.getBlendMode_or(SkBlendMode::kSrcOver),
                             color,
                             srcQuad,
                             dstClip,
                             aa,
                             aaFlags,
                             constraint == SkCanvas::kStrict_SrcRectConstraint ? &srcRect : nullptr,
                             ctm,
                             std::move(textureXform));
    } else {
        sdc->drawTexture(clip,
                         std::move(view),
                         srcColorInfo.alphaType(),
                         filter,
                         GrSamplerState::MipmapMode::kNone,
                         paint.getBlendMode_or(SkBlendMode::kSrcOver),
                         color,
                         srcRect,
                         dstRect,
                         aa,
                         aaFlags,
                         constraint,
                         ctm,
                         std::move(textureXform));
    }
}

// Assumes srcRect and dstRect have already been optimized to fit the proxy.
void draw_image(GrRecordingContext* rContext,
                skgpu::v1::SurfaceDrawContext* sdc,
                const GrClip* clip,
                const SkMatrixProvider& matrixProvider,
                const SkPaint& paint,
                const SkImage_Base& image,
                const SkRect& src,
                const SkRect& dst,
                const SkPoint dstClip[4],
                const SkMatrix& srcToDst,
                GrAA aa,
                GrQuadAAFlags aaFlags,
                SkCanvas::SrcRectConstraint constraint,
                SkSamplingOptions sampling,
                SkTileMode tm = SkTileMode::kClamp) {
    const SkMatrix& ctm(matrixProvider.localToDevice());
    if (tm == SkTileMode::kClamp &&
        !image.isYUVA()          &&
        can_use_draw_texture(paint, sampling.useCubic, sampling.mipmap)) {
        // We've done enough checks above to allow us to pass ClampNearest() and not check for
        // scaling adjustments.
        auto [view, ct] = image.asView(rContext, GrMipmapped::kNo);
        if (!view) {
            return;
        }
        GrColorInfo info(image.imageInfo().colorInfo());
        info = info.makeColorType(ct);
        draw_texture(sdc,
                     clip,
                     ctm,
                     paint,
                     sampling.filter,
                     src,
                     dst,
                     dstClip,
                     aa,
                     aaFlags,
                     constraint,
                     std::move(view),
                     info);
        return;
    }

    const SkMaskFilter* mf = paint.getMaskFilter();

    // The shader expects proper local coords, so we can't replace local coords with texture coords
    // if the shader will be used. If we have a mask filter we will change the underlying geometry
    // that is rendered.
    bool canUseTextureCoordsAsLocalCoords = !use_shader(image.isAlphaOnly(), paint) && !mf;

    // Specifying the texture coords as local coordinates is an attempt to enable more GrDrawOp
    // combining by not baking anything about the srcRect, dstRect, or ctm, into the texture
    // FP. In the future this should be an opaque optimization enabled by the combination of
    // GrDrawOp/GP and FP.
    if (mf && as_MFB(mf)->hasFragmentProcessor()) {
        mf = nullptr;
    }

    bool restrictToSubset = SkCanvas::kStrict_SrcRectConstraint == constraint;

    // If we have to outset for AA then we will generate texture coords outside the src rect. The
    // same happens for any mask filter that extends the bounds rendered in the dst.
    // This is conservative as a mask filter does not have to expand the bounds rendered.
    bool coordsAllInsideSrcRect = aaFlags == GrQuadAAFlags::kNone && !mf;

    // Check for optimization to drop the src rect constraint when using linear filtering.
    // TODO: Just rely on image to handle this.
    if (!sampling.useCubic                       &&
        sampling.filter == SkFilterMode::kLinear &&
        restrictToSubset                         &&
        sampling.mipmap == SkMipmapMode::kNone   &&
        coordsAllInsideSrcRect                   &&
        !image.isYUVA()) {
        SkMatrix combinedMatrix;
        combinedMatrix.setConcat(ctm, srcToDst);
        if (can_ignore_linear_filtering_subset(src, combinedMatrix, sdc->numSamples())) {
            restrictToSubset = false;
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
    const SkRect* subset = restrictToSubset       ? &src : nullptr;
    const SkRect* domain = coordsAllInsideSrcRect ? &src : nullptr;
    SkTileMode tileModes[] = {tm, tm};
    std::unique_ptr<GrFragmentProcessor> fp = image.asFragmentProcessor(rContext,
                                                                        sampling,
                                                                        tileModes,
                                                                        textureMatrix,
                                                                        subset,
                                                                        domain);
    fp = GrColorSpaceXformEffect::Make(std::move(fp),
                                       image.imageInfo().colorInfo(),
                                       sdc->colorInfo());
    if (image.isAlphaOnly()) {
        if (const auto* shader = as_SB(paint.getShader())) {
            auto shaderFP = shader->asFragmentProcessor(
                    GrFPArgs(rContext, matrixProvider, &sdc->colorInfo()));
            if (!shaderFP) {
                return;
            }
            fp = GrBlendFragmentProcessor::Make(
                    std::move(fp), std::move(shaderFP), SkBlendMode::kDstIn);
        } else {
            // This takes the input (paint) color, premultiplies it, then multiplies by the texture
            fp = GrFragmentProcessor::MakeInputPremulAndMulByOutput(std::move(fp));
        }

    } else {
        if (paint.getColor4f().isOpaque()) {
            // If the input alpha is known to be 1, we don't need to take the kSrcIn path. This is
            // just an optimization. However, we can't just return 'fp' here. We need to actually
            // inhibit the coverage-as-alpha optimization, or we'll fail to incorporate AA
            // correctly. The OverrideInput FP happens to do that, so wrap our fp in one of those.
            // The texture FP doesn't actually use the input color at all, so the overridden input
            // is irrelevant.
            fp = GrFragmentProcessor::OverrideInput(std::move(fp), SK_PMColor4fWHITE, false);
        } else {
            // If the paint color isn't opaque, then scale the shader's output by input (paint)
            // alpha.
            // TODO(skia:11942): Move this alpha modulation to paint-conversion. This won't need to
            // do anything.
            fp = GrFragmentProcessor::MulChildByInputAlpha(std::move(fp));
        }
    }

    GrPaint grPaint;
    if (!SkPaintToGrPaintReplaceShader(
                rContext, sdc->colorInfo(), paint, matrixProvider, std::move(fp), &grPaint)) {
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
            sdc->fillQuadWithEdgeAA(clip, std::move(grPaint), aa, aaFlags, ctm, dstClip, srcClip);
        } else {
            // Provide explicit texture coords when possible, otherwise rely on texture matrix
            sdc->fillRectWithEdgeAA(clip, std::move(grPaint), aa, aaFlags, ctm, dst,
                                    canUseTextureCoordsAsLocalCoords ? &src : nullptr);
        }
    } else {
        // Must draw the mask filter as a GrStyledShape. For now, this loses the per-edge AA
        // information since it always draws with AA, but that should not be noticeable since the
        // mask filter is probably a blur.
        GrStyledShape shape;
        if (dstClip) {
            // Represent it as an SkPath formed from the dstClip
            SkPath path;
            path.addPoly(dstClip, 4, true);
            shape = GrStyledShape(path);
        } else {
            shape = GrStyledShape(dst);
        }

        GrBlurUtils::drawShapeWithMaskFilter(
                rContext, sdc, clip, shape, std::move(grPaint), ctm, mf);
    }
}

void draw_tiled_bitmap(GrRecordingContext* rContext,
                       skgpu::v1::SurfaceDrawContext* sdc,
                       const GrClip* clip,
                       const SkBitmap& bitmap,
                       int tileSize,
                       const SkMatrixProvider& matrixProvider,
                       const SkMatrix& srcToDst,
                       const SkRect& srcRect,
                       const SkIRect& clippedSrcIRect,
                       const SkPaint& paint,
                       GrAA aa,
                       SkCanvas::SrcRectConstraint constraint,
                       SkSamplingOptions sampling,
                       SkTileMode tileMode) {
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
            if (sampling.filter != SkFilterMode::kNearest || sampling.useCubic) {
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
                int outset = sampling.useCubic ? GrBicubicEffect::kFilterTexelPad : 1;
                clamped_outset_with_offset(&iTileR, outset, &offset, iClampRect);
            }

            // We must subset as a bitmap and then turn into an SkImage if we want caching to work.
            // Image subsets always make a copy of the pixels and lose the association with the
            // original's SkPixelRef.
            if (SkBitmap subsetBmp; bitmap.extractSubset(&subsetBmp, iTileR)) {
                auto image = SkMakeImageFromRasterBitmap(subsetBmp, kNever_SkCopyPixelsMode);
                // We should have already handled bitmaps larger than the max texture size.
                SkASSERT(image->width()  <= rContext->priv().caps()->maxTextureSize() &&
                         image->height() <= rContext->priv().caps()->maxTextureSize());

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
                draw_image(rContext,
                           sdc,
                           clip,
                           matrixProvider,
                           paint,
                           *as_IB(image.get()),
                           tileR,
                           rectToDraw,
                           nullptr,
                           offsetSrcToDst,
                           aa,
                           aaFlags,
                           constraint,
                           sampling,
                           tileMode);
            }
        }
    }
}

SkFilterMode downgrade_to_filter(const SkSamplingOptions& sampling) {
    SkFilterMode filter = sampling.filter;
    if (sampling.useCubic || sampling.mipmap != SkMipmapMode::kNone) {
        // if we were "fancier" than just bilerp, only do bilerp
        filter = SkFilterMode::kLinear;
    }
    return filter;
}

bool can_disable_mipmap(const SkMatrix& viewM,
                        const SkMatrix& localM,
                        bool sharpenMipmappedTextures) {
    SkMatrix matrix;
    matrix.setConcat(viewM, localM);
    // With sharp mips, we bias lookups by -0.5. That means our final LOD is >= 0 until
    // the computed LOD is >= 0.5. At what scale factor does a texture get an LOD of
    // 0.5?
    //
    // Want:  0       = log2(1/s) - 0.5
    //        0.5     = log2(1/s)
    //        2^0.5   = 1/s
    //        1/2^0.5 = s
    //        2^0.5/2 = s
    SkScalar mipScale = sharpenMipmappedTextures ? SK_ScalarRoot2Over2 : SK_Scalar1;
    return matrix.getMinScale() >= mipScale;
}

} // anonymous namespace

//////////////////////////////////////////////////////////////////////////////

namespace skgpu::v1 {

void Device::drawSpecial(SkSpecialImage* special,
                         const SkMatrix& localToDevice,
                         const SkSamplingOptions& origSampling,
                         const SkPaint& paint) {
    SkASSERT(!paint.getMaskFilter() && !paint.getImageFilter());
    SkASSERT(special->isTextureBacked());

    SkRect src = SkRect::Make(special->subset());
    SkRect dst = SkRect::MakeWH(special->width(), special->height());
    SkMatrix srcToDst = SkMatrix::RectToRect(src, dst);

    SkSamplingOptions sampling = SkSamplingOptions(downgrade_to_filter(origSampling));
    GrAA aa = fSurfaceDrawContext->chooseAA(paint);
    GrQuadAAFlags aaFlags = (aa == GrAA::kYes) ? GrQuadAAFlags::kAll : GrQuadAAFlags::kNone;

    SkColorInfo colorInfo(special->colorType(),
                          special->alphaType(),
                          sk_ref_sp(special->getColorSpace()));

    GrSurfaceProxyView view = special->view(this->recordingContext());
    SkImage_Gpu image(sk_ref_sp(special->getContext()),
                      special->uniqueID(),
                      std::move(view),
                      std::move(colorInfo));
    // In most cases this ought to hit draw_texture since there won't be a color filter,
    // alpha-only texture+shader, or a high filter quality.
    SkOverrideDeviceMatrixProvider matrixProvider(this->asMatrixProvider(), localToDevice);
    draw_image(fContext.get(),
               fSurfaceDrawContext.get(),
               this->clip(),
               matrixProvider,
               paint,
               image,
               src,
               dst,
               nullptr,
               srcToDst,
               aa,
               aaFlags,
               SkCanvas::kStrict_SrcRectConstraint,
               sampling);
}

void Device::drawImageQuad(const SkImage* image,
                           const SkRect* srcRect,
                           const SkRect* dstRect,
                           const SkPoint dstClip[4],
                           GrAA aa,
                           GrQuadAAFlags aaFlags,
                           const SkMatrix* preViewMatrix,
                           const SkSamplingOptions& origSampling,
                           const SkPaint& paint,
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
    SkTileMode tileMode = mode == ImageDrawMode::kDecal ? SkTileMode::kDecal : SkTileMode::kClamp;

    // Get final CTM matrix
    SkPreConcatMatrixProvider matrixProvider(this->asMatrixProvider(),
                                             preViewMatrix ? *preViewMatrix : SkMatrix::I());
    const SkMatrix& ctm(matrixProvider.localToDevice());

    SkSamplingOptions sampling = origSampling;
    bool sharpenMM = fContext->priv().options().fSharpenMipmappedTextures;
    if (sampling.mipmap != SkMipmapMode::kNone && can_disable_mipmap(ctm, srcToDst, sharpenMM)) {
        sampling = SkSamplingOptions(sampling.filter);
    }
    auto clip = this->clip();

    if (!image->isTextureBacked() && !as_IB(image)->isPinnedOnContext(fContext.get())) {
        int tileFilterPad;
        if (sampling.useCubic) {
            tileFilterPad = GrBicubicEffect::kFilterTexelPad;
        } else if (sampling.filter == SkFilterMode::kNearest) {
            tileFilterPad = 0;
        } else {
            tileFilterPad = 1;
        }
        int maxTileSize = fContext->priv().caps()->maxTextureSize() - 2*tileFilterPad;
        int tileSize;
        SkIRect clippedSubset;
        if (should_tile_image_id(fContext.get(),
                                 fSurfaceDrawContext->dimensions(),
                                 clip,
                                 image->unique(),
                                 image->dimensions(),
                                 ctm,
                                 srcToDst,
                                 &src,
                                 maxTileSize,
                                 &tileSize,
                                 &clippedSubset)) {
            // Extract pixels on the CPU, since we have to split into separate textures before
            // sending to the GPU if tiling.
            if (SkBitmap bm; as_IB(image)->getROPixels(nullptr, &bm)) {
                // This is the funnel for all paths that draw tiled bitmaps/images.
                draw_tiled_bitmap(fContext.get(),
                                  fSurfaceDrawContext.get(),
                                  clip,
                                  bm,
                                  tileSize,
                                  matrixProvider,
                                  srcToDst,
                                  src,
                                  clippedSubset,
                                  paint,
                                  aa,
                                  constraint,
                                  sampling,
                                  tileMode);
                return;
            }
        }
    }

    draw_image(fContext.get(),
               fSurfaceDrawContext.get(),
               clip,
               matrixProvider,
               paint,
               *as_IB(image),
               src,
               dst,
               dstClip,
               srcToDst,
               aa,
               aaFlags,
               constraint,
               sampling);
    return;
}

void Device::drawEdgeAAImageSet(const SkCanvas::ImageSetEntry set[], int count,
                                const SkPoint dstClips[], const SkMatrix preViewMatrices[],
                                const SkSamplingOptions& sampling, const SkPaint& paint,
                                SkCanvas::SrcRectConstraint constraint) {
    SkASSERT(count > 0);
    if (!can_use_draw_texture(paint, sampling.useCubic, sampling.mipmap)) {
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
                    sampling, *entryPaint, constraint);
            dstClipIndex += 4 * set[i].fHasClip;
        }
        return;
    }

    GrSamplerState::Filter filter = sampling.filter == SkFilterMode::kNearest
                                            ? GrSamplerState::Filter::kNearest
                                            : GrSamplerState::Filter::kLinear;
    SkBlendMode mode = paint.getBlendMode_or(SkBlendMode::kSrcOver);

    SkAutoTArray<GrTextureSetEntry> textures(count);
    // We accumulate compatible proxies until we find an an incompatible one or reach the end and
    // issue the accumulated 'n' draws starting at 'base'. 'p' represents the number of proxy
    // switches that occur within the 'n' entries.
    int base = 0, n = 0, p = 0;
    auto draw = [&](int nextBase) {
        if (n > 0) {
            auto textureXform = GrColorSpaceXform::Make(set[base].fImage->imageInfo().colorInfo(),
                                                        fSurfaceDrawContext->colorInfo());
            fSurfaceDrawContext->drawTextureSet(this->clip(),
                                                textures.get() + base,
                                                n,
                                                p,
                                                filter,
                                                GrSamplerState::MipmapMode::kNone,
                                                mode,
                                                GrAA::kYes,
                                                constraint,
                                                this->localToDevice(),
                                                std::move(textureXform));
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
            std::tie(view, std::ignore) = image->asView(this->recordingContext(), GrMipmapped::kNo);
            if (image->isAlphaOnly()) {
                GrSwizzle swizzle = GrSwizzle::Concat(view.swizzle(), GrSwizzle("aaaa"));
                view = {view.detachProxy(), view.origin(), swizzle};
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
                    sampling, *entryPaint, constraint);
            continue;
        }

        textures[i].fProxyView = std::move(view);
        textures[i].fSrcAlphaType = image->alphaType();
        textures[i].fSrcRect = set[i].fSrcRect;
        textures[i].fDstRect = set[i].fDstRect;
        textures[i].fDstClipQuad = clip;
        textures[i].fPreViewMatrix =
                set[i].fMatrixIndex < 0 ? nullptr : preViewMatrices + set[i].fMatrixIndex;
        textures[i].fColor = texture_color(paint.getColor4f(), set[i].fAlpha,
                                           SkColorTypeToGrColorType(image->colorType()),
                                           fSurfaceDrawContext->colorInfo());
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

} // namespace skgpu::v1
