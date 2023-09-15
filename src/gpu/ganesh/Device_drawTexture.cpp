/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/Device.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkColorSpace.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/private/base/SkTPin.h"
#include "src/core/SkDraw.h"
#include "src/core/SkMaskFilterBase.h"
#include "src/core/SkSamplingPriv.h"
#include "src/core/SkSpecialImage.h"
#include "src/gpu/TiledTextureUtils.h"
#include "src/gpu/ganesh/GrBlurUtils.h"
#include "src/gpu/ganesh/GrColorSpaceXform.h"
#include "src/gpu/ganesh/GrFPArgs.h"
#include "src/gpu/ganesh/GrFragmentProcessors.h"
#include "src/gpu/ganesh/GrOpsTypes.h"
#include "src/gpu/ganesh/GrStyle.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "src/gpu/ganesh/effects/GrBlendFragmentProcessor.h"
#include "src/gpu/ganesh/effects/GrTextureEffect.h"
#include "src/gpu/ganesh/geometry/GrRect.h"
#include "src/gpu/ganesh/geometry/GrStyledShape.h"
#include "src/gpu/ganesh/image/GrImageUtils.h"
#include "src/gpu/ganesh/image/SkImage_Ganesh.h"
#include "src/gpu/ganesh/image/SkSpecialImage_Ganesh.h"
#include "src/image/SkImage_Base.h"

using namespace skia_private;

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
//  Helper functions for drawing an image with ganesh::SurfaceDrawContext

/**
 * Checks whether the paint is compatible with using SurfaceDrawContext::drawTexture. It is more
 * efficient than the SkImage general case.
 */
bool can_use_draw_texture(const SkPaint& paint, const SkSamplingOptions& sampling) {
    return (!paint.getColorFilter() && !paint.getShader() && !paint.getMaskFilter() &&
            !paint.getImageFilter() && !paint.getBlender() && !sampling.isAniso() &&
            !sampling.useCubic && sampling.mipmap == SkMipmapMode::kNone);
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
void draw_texture(skgpu::ganesh::SurfaceDrawContext* sdc,
                  const GrClip* clip,
                  const SkMatrix& ctm,
                  const SkPaint& paint,
                  GrSamplerState::Filter filter,
                  const SkRect& srcRect,
                  const SkRect& dstRect,
                  const SkPoint dstClip[4],
                  GrQuadAAFlags aaFlags,
                  SkCanvas::SrcRectConstraint constraint,
                  GrSurfaceProxyView view,
                  const GrColorInfo& srcColorInfo) {
    if (GrColorTypeIsAlphaOnly(srcColorInfo.colorType())) {
        view.concatSwizzle(skgpu::Swizzle("aaaa"));
    }
    const GrColorInfo& dstInfo = sdc->colorInfo();
    auto textureXform = GrColorSpaceXform::Make(srcColorInfo, sdc->colorInfo());
    GrSurfaceProxy* proxy = view.proxy();
    // Must specify the strict constraint when the proxy is not functionally exact and the src
    // rect would access pixels outside the proxy's content area without the constraint.
    if (constraint != SkCanvas::kStrict_SrcRectConstraint && !proxy->isFunctionallyExact()) {
        // Conservative estimate of how much a coord could be outset from src rect:
        // 1/2 pixel for AA and 1/2 pixel for linear filtering
        float buffer = 0.5f * (aaFlags != GrQuadAAFlags::kNone) +
                       GrTextureEffect::kLinearInset * (filter == GrSamplerState::Filter::kLinear);
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
                         aaFlags,
                         constraint,
                         ctm,
                         std::move(textureXform));
    }
}

SkFilterMode downgrade_to_filter(const SkSamplingOptions& sampling) {
    SkFilterMode filter = sampling.filter;
    if (sampling.isAniso() || sampling.useCubic || sampling.mipmap != SkMipmapMode::kNone) {
        // if we were "fancier" than just bilerp, only do bilerp
        filter = SkFilterMode::kLinear;
    }
    return filter;
}

} // anonymous namespace


//////////////////////////////////////////////////////////////////////////////

namespace skgpu::ganesh {

void Device::drawEdgeAAImage(const SkImage* image,
                             const SkRect& src,
                             const SkRect& dst,
                             const SkPoint dstClip[4],
                             SkCanvas::QuadAAFlags canvasAAFlags,
                             const SkMatrix& localToDevice,
                             const SkSamplingOptions& sampling,
                             const SkPaint& paint,
                             SkCanvas::SrcRectConstraint constraint,
                             const SkMatrix& srcToDst,
                             SkTileMode tm) {
    GrRecordingContext* rContext = fContext.get();
    SurfaceDrawContext* sdc = fSurfaceDrawContext.get();
    const GrClip* clip = this->clip();

    GrQuadAAFlags aaFlags = SkToGrQuadAAFlags(canvasAAFlags);
    auto ib = as_IB(image);
    if (tm == SkTileMode::kClamp && !ib->isYUVA() && can_use_draw_texture(paint, sampling)) {
        // We've done enough checks above to allow us to pass ClampNearest() and not check for
        // scaling adjustments.
        auto [view, ct] = skgpu::ganesh::AsView(rContext, image, skgpu::Mipmapped::kNo);
        if (!view) {
            return;
        }
        GrColorInfo info(image->imageInfo().colorInfo());
        info = info.makeColorType(ct);
        draw_texture(sdc,
                     clip,
                     localToDevice,
                     paint,
                     sampling.filter,
                     src,
                     dst,
                     dstClip,
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
    bool canUseTextureCoordsAsLocalCoords = !use_shader(image->isAlphaOnly(), paint) && !mf;

    // Specifying the texture coords as local coordinates is an attempt to enable more GrDrawOp
    // combining by not baking anything about the srcRect, dstRect, or ctm, into the texture
    // FP. In the future this should be an opaque optimization enabled by the combination of
    // GrDrawOp/GP and FP.
    if (GrFragmentProcessors::IsSupported(mf)) {
        mf = nullptr;
    }

    bool restrictToSubset = SkCanvas::kStrict_SrcRectConstraint == constraint;

    // If we have to outset for AA then we will generate texture coords outside the src rect. The
    // same happens for any mask filter that extends the bounds rendered in the dst.
    // This is conservative as a mask filter does not have to expand the bounds rendered.
    bool coordsAllInsideSrcRect = aaFlags == GrQuadAAFlags::kNone && !mf;

    // Check for optimization to drop the src rect constraint when using linear filtering.
    // TODO: Just rely on image to handle this.
    if (sampling.isAniso() && !sampling.useCubic && sampling.filter == SkFilterMode::kLinear &&
        restrictToSubset && sampling.mipmap == SkMipmapMode::kNone && coordsAllInsideSrcRect &&
        !ib->isYUVA()) {
        SkMatrix combinedMatrix;
        combinedMatrix.setConcat(localToDevice, srcToDst);
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
    std::unique_ptr<GrFragmentProcessor> fp = skgpu::ganesh::AsFragmentProcessor(
            rContext, image, sampling, tileModes, textureMatrix, subset, domain);
    fp = GrColorSpaceXformEffect::Make(
            std::move(fp), image->imageInfo().colorInfo(), sdc->colorInfo());
    if (image->isAlphaOnly()) {
        if (const auto* shader = as_SB(paint.getShader())) {
            auto shaderFP = GrFragmentProcessors::Make(shader,
                                                       GrFPArgs(rContext,
                                                                &sdc->colorInfo(),
                                                                sdc->surfaceProps(),
                                                                GrFPArgs::Scope::kDefault),
                                                       localToDevice);
            if (!shaderFP) {
                return;
            }
            fp = GrBlendFragmentProcessor::Make<SkBlendMode::kDstIn>(std::move(fp),
                                                                     std::move(shaderFP));
        } else {
            // Multiply the input (paint) color by the texture (alpha)
            fp = GrFragmentProcessor::MulInputByChildAlpha(std::move(fp));
        }
    }

    GrPaint grPaint;
    if (!SkPaintToGrPaintReplaceShader(rContext,
                                       sdc->colorInfo(),
                                       paint,
                                       localToDevice,
                                       std::move(fp),
                                       sdc->surfaceProps(),
                                       &grPaint)) {
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
            sdc->fillQuadWithEdgeAA(clip, std::move(grPaint), aaFlags, localToDevice,
                                    dstClip, srcClip);
        } else {
            // Provide explicit texture coords when possible, otherwise rely on texture matrix
            sdc->fillRectWithEdgeAA(clip, std::move(grPaint), aaFlags, localToDevice, dst,
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

        GrBlurUtils::DrawShapeWithMaskFilter(
                rContext, sdc, clip, shape, std::move(grPaint), localToDevice, mf);
    }
}

void Device::drawSpecial(SkSpecialImage* special,
                         const SkMatrix& localToDevice,
                         const SkSamplingOptions& origSampling,
                         const SkPaint& paint) {
    SkASSERT(!paint.getMaskFilter() && !paint.getImageFilter());
    SkASSERT(special->isGaneshBacked());

    SkRect src = SkRect::Make(special->subset());
    SkRect dst = SkRect::MakeWH(special->width(), special->height());
    SkMatrix srcToDst = SkMatrix::RectToRect(src, dst);

    SkSamplingOptions sampling = SkSamplingOptions(downgrade_to_filter(origSampling));
    GrAA aa = fSurfaceDrawContext->chooseAA(paint);
    SkCanvas::QuadAAFlags aaFlags = (aa == GrAA::kYes) ? SkCanvas::kAll_QuadAAFlags
                                                       : SkCanvas::kNone_QuadAAFlags;

    GrSurfaceProxyView view = SkSpecialImages::AsView(this->recordingContext(), special);
    SkImage_Ganesh image(sk_ref_sp(special->getContext()),
                         special->uniqueID(),
                         std::move(view),
                         special->colorInfo());
    // In most cases this ought to hit draw_texture since there won't be a color filter,
    // alpha-only texture+shader, or a high filter quality.
    this->drawEdgeAAImage(&image,
                          src,
                          dst,
                          /* dstClip= */nullptr,
                          aaFlags,
                          localToDevice,
                          sampling,
                          paint,
                          SkCanvas::kStrict_SrcRectConstraint,
                          srcToDst,
                          SkTileMode::kClamp);
}

void Device::drawImageQuadDirect(const SkImage* image,
                                 const SkRect& srcRect,
                                 const SkRect& dstRect,
                                 const SkPoint dstClip[4],
                                 SkCanvas::QuadAAFlags aaFlags,
                                 const SkMatrix* preViewMatrix,
                                 const SkSamplingOptions& origSampling,
                                 const SkPaint& paint,
                                 SkCanvas::SrcRectConstraint constraint) {
    SkRect src;
    SkRect dst;
    SkMatrix srcToDst;
    auto mode = TiledTextureUtils::OptimizeSampleArea(SkISize::Make(image->width(),
                                                                    image->height()),
                                                      srcRect, dstRect, dstClip,
                                                      &src, &dst, &srcToDst);
    if (mode == TiledTextureUtils::ImageDrawMode::kSkip) {
        return;
    }

    if (src.contains(image->bounds())) {
        constraint = SkCanvas::kFast_SrcRectConstraint;
    }
    // Depending on the nature of image, it can flow through more or less optimal pipelines
    SkTileMode tileMode = mode == TiledTextureUtils::ImageDrawMode::kDecal ? SkTileMode::kDecal
                                                                           : SkTileMode::kClamp;

    // Get final CTM matrix
    SkMatrix ctm = this->localToDevice();
    if (preViewMatrix) {
        ctm.preConcat(*preViewMatrix);
    }

    SkSamplingOptions sampling = origSampling;
    if (sampling.mipmap != SkMipmapMode::kNone &&
        TiledTextureUtils::CanDisableMipmap(ctm, srcToDst)) {
        sampling = SkSamplingOptions(sampling.filter);
    }

    this->drawEdgeAAImage(image,
                          src,
                          dst,
                          dstClip,
                          aaFlags,
                          ctm,
                          sampling,
                          paint,
                          constraint,
                          srcToDst,
                          tileMode);
}

void Device::drawEdgeAAImageSet(const SkCanvas::ImageSetEntry set[], int count,
                                const SkPoint dstClips[], const SkMatrix preViewMatrices[],
                                const SkSamplingOptions& sampling, const SkPaint& paint,
                                SkCanvas::SrcRectConstraint constraint) {
    SkASSERT(count > 0);
    if (!can_use_draw_texture(paint, sampling)) {
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
            this->drawImageQuadDirect(
                    set[i].fImage.get(), set[i].fSrcRect, set[i].fDstRect,
                    set[i].fHasClip ? dstClips + dstClipIndex : nullptr,
                    static_cast<SkCanvas::QuadAAFlags>(set[i].fAAFlags),
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

    AutoTArray<GrTextureSetEntry> textures(count);
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

        // The default SkDevice implementation is based on drawImageRect which does not allow
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
            std::tie(view, std::ignore) =
                    skgpu::ganesh::AsView(this->recordingContext(), image, skgpu::Mipmapped::kNo);
            if (image->isAlphaOnly()) {
                skgpu::Swizzle swizzle = skgpu::Swizzle::Concat(view.swizzle(),
                                                                skgpu::Swizzle("aaaa"));
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
            this->drawImageQuadDirect(
                    image, set[i].fSrcRect, set[i].fDstRect, clip,
                    static_cast<SkCanvas::QuadAAFlags>(set[i].fAAFlags),
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

}  // namespace skgpu::ganesh
