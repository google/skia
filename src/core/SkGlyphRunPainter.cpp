/*
 * Copyright 2018 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGlyphRunPainter.h"

#if SK_SUPPORT_GPU
#include "GrColorSpaceInfo.h"
#include "GrRenderTargetContext.h"
#include "SkGr.h"
#include "text/GrTextBlobCache.h"
#include "text/GrTextContext.h"
#endif

#include "SkColorFilter.h"
#include "SkDevice.h"
#include "SkDistanceFieldGen.h"
#include "SkDraw.h"
#include "SkFontPriv.h"
#include "SkGlyphCache.h"
#include "SkMaskFilter.h"
#include "SkPaintPriv.h"
#include "SkPathEffect.h"
#include "SkRasterClip.h"
#include "SkRemoteGlyphCacheImpl.h"
#include "SkStrikeCache.h"
#include "SkTDArray.h"
#include "SkTraceEvent.h"

// -- SkGlyphCacheCommon ---------------------------------------------------------------------------

SkVector SkGlyphCacheCommon::PixelRounding(bool isSubpixel, SkAxisAlignment axisAlignment) {
    if (!isSubpixel) {
        return {SK_ScalarHalf, SK_ScalarHalf};
    } else {
        static constexpr SkScalar kSubpixelRounding = SkFixedToScalar(SkGlyph::kSubpixelRound);
        switch (axisAlignment) {
            case kX_SkAxisAlignment:
                return {kSubpixelRounding, SK_ScalarHalf};
            case kY_SkAxisAlignment:
                return {SK_ScalarHalf, kSubpixelRounding};
            case kNone_SkAxisAlignment:
                return {kSubpixelRounding, kSubpixelRounding};
        }
    }

    // Some compilers need this.
    return {0, 0};
}

SkIPoint SkGlyphCacheCommon::SubpixelLookup(SkAxisAlignment axisAlignment, SkPoint position) {
    // TODO: SkScalarFraction uses truncf to calculate the fraction. This should be floorf.
    SkFixed lookupX = SkScalarToFixed(SkScalarFraction(position.x())),
            lookupY = SkScalarToFixed(SkScalarFraction(position.y()));

    // Snap to a given axis if alignment is requested.
    if (axisAlignment == kX_SkAxisAlignment) {
        lookupY = 0;
    } else if (axisAlignment == kY_SkAxisAlignment) {
        lookupX = 0;
    }

    return {lookupX, lookupY};
}

bool SkGlyphCacheCommon::GlyphTooBigForAtlas(const SkGlyph& glyph) {
    return glyph.fWidth > kSkSideTooBigForAtlas || glyph.fHeight > kSkSideTooBigForAtlas;
}

// -- SkGlyphRunListPainter ------------------------------------------------------------------------
SkGlyphRunListPainter::SkGlyphRunListPainter(
        const SkSurfaceProps& props, SkColorType colorType, SkScalerContextFlags flags)
        : fDeviceProps{props}
        , fBitmapFallbackProps{SkSurfaceProps{props.flags(), kUnknown_SkPixelGeometry}}
        , fColorType{colorType}
        , fScalerContextFlags{flags} {}

#if SK_SUPPORT_GPU

// TODO: unify with code in GrTextContext.cpp
static SkScalerContextFlags compute_scaler_context_flags(
        const GrColorSpaceInfo& colorSpaceInfo) {
    // If we're doing linear blending, then we can disable the gamma hacks.
    // Otherwise, leave them on. In either case, we still want the contrast boost:
    // TODO: Can we be even smarter about mask gamma based on the dest transfer function?
    if (colorSpaceInfo.isLinearlyBlended()) {
        return SkScalerContextFlags::kBoostContrast;
    } else {
        return SkScalerContextFlags::kFakeGammaAndBoostContrast;
    }
}

SkGlyphRunListPainter::SkGlyphRunListPainter(
        const SkSurfaceProps& props, const GrColorSpaceInfo& csi)
        : SkGlyphRunListPainter(props, kUnknown_SkColorType, compute_scaler_context_flags(csi)) {}

SkGlyphRunListPainter::SkGlyphRunListPainter(const GrRenderTargetContext& rtc)
        : SkGlyphRunListPainter{rtc.surfaceProps(), rtc.colorSpaceInfo()} {}

#endif

bool SkGlyphRunListPainter::ShouldDrawAsPath(
        const SkPaint& paint, const SkFont& font, const SkMatrix& matrix) {
    // hairline glyphs are fast enough so we don't need to cache them
    if (SkPaint::kStroke_Style == paint.getStyle() && 0 == paint.getStrokeWidth()) {
        return true;
    }

    // we don't cache perspective
    if (matrix.hasPerspective()) {
        return true;
    }

    return SkPaint::TooBigToUseCache(matrix, SkFontPriv::MakeTextMatrix(font), 1024);
}

void SkGlyphRunListPainter::ensureBitmapBuffers(size_t runSize) {
    if (runSize > fMaxRunSize) {
        fPositions.reset(runSize);
        fMaxRunSize = runSize;
    }
}

static bool check_glyph_position(SkPoint position) {
    // Prevent glyphs from being drawn outside of or straddling the edge of device space.
    // Comparisons written a little weirdly so that NaN coordinates are treated safely.
    auto gt = [](float a, int b) { return !(a <= (float)b); };
    auto lt = [](float a, int b) { return !(a >= (float)b); };
    return !(gt(position.fX, INT_MAX - (INT16_MAX + SkTo<int>(UINT16_MAX))) ||
             lt(position.fX, INT_MIN - (INT16_MIN + 0 /*UINT16_MIN*/)) ||
             gt(position.fY, INT_MAX - (INT16_MAX + SkTo<int>(UINT16_MAX))) ||
             lt(position.fY, INT_MIN - (INT16_MIN + 0 /*UINT16_MIN*/)));
}

static SkMask create_mask(const SkGlyph& glyph, SkPoint position, const void* image) {
    SkMask mask;
    int left = SkScalarFloorToInt(position.fX);
    int top  = SkScalarFloorToInt(position.fY);

    left += glyph.fLeft;
    top  += glyph.fTop;

    int right   = left + glyph.fWidth;
    int bottom  = top  + glyph.fHeight;

    mask.fBounds.set(left, top, right, bottom);
    SkASSERT(!mask.fBounds.isEmpty());

    mask.fImage    = (uint8_t*)image;
    mask.fRowBytes = glyph.rowBytes();
    mask.fFormat   = static_cast<SkMask::Format>(glyph.fMaskFormat);

    return mask;
}

void SkGlyphRunListPainter::drawForBitmapDevice(
        const SkGlyphRunList& glyphRunList, const SkMatrix& deviceMatrix,
        const BitmapDevicePainter* bitmapDevice) {
    const SkPaint& runPaint = glyphRunList.paint();
    // The bitmap blitters can only draw lcd text to a N32 bitmap in srcOver. Otherwise,
    // convert the lcd text into A8 text. The props communicates this to the scaler.
    auto& props = (kN32_SkColorType == fColorType && runPaint.isSrcOver())
                  ? fDeviceProps
                  : fBitmapFallbackProps;

    SkPoint origin = glyphRunList.origin();
    for (auto& glyphRun : glyphRunList) {
        const SkFont& runFont = glyphRun.font();
        auto runSize = glyphRun.runSize();
        this->ensureBitmapBuffers(runSize);

        if (ShouldDrawAsPath(runPaint, runFont, deviceMatrix)) {
            SkMatrix::MakeTrans(origin.x(), origin.y()).mapPoints(
                    fPositions, glyphRun.positions().data(), runSize);
            // setup our std pathPaint, in hopes of getting hits in the cache
            SkPaint pathPaint(runPaint);
            SkFont  pathFont{runFont};
            SkScalar textScale = pathFont.setupForAsPaths(&pathPaint);

            auto pathCache = SkStrikeCache::FindOrCreateStrikeExclusive(
                                pathFont, pathPaint, props,
                                fScalerContextFlags, SkMatrix::I());

            SkTDArray<PathAndPos> pathsAndPositions;
            pathsAndPositions.setReserve(runSize);
            SkPoint* positionCursor = fPositions;
            for (auto glyphID : glyphRun.glyphsIDs()) {
                SkPoint position = *positionCursor++;
                if (check_glyph_position(position)) {
                    const SkGlyph& glyph = pathCache->getGlyphMetrics(glyphID, {0, 0});
                    if (!glyph.isEmpty()) {
                        const SkPath* path = pathCache->findPath(glyph);
                        if (path != nullptr) {
                            pathsAndPositions.push_back(PathAndPos{path, position});
                        }
                    }
                }
            }

            // The paint we draw paths with must have the same anti-aliasing state as the runFont
            // allowing the paths to have the same edging as the glyph masks.
            pathPaint = runPaint;
            pathPaint.setAntiAlias(runFont.hasSomeAntiAliasing());

            bitmapDevice->paintPaths(
                    SkSpan<const PathAndPos>{pathsAndPositions.begin(), pathsAndPositions.size()},
                    textScale, pathPaint);
        } else {
            auto cache = SkStrikeCache::FindOrCreateStrikeExclusive(
                                        runFont, runPaint, props,
                                        fScalerContextFlags, deviceMatrix);

            // Add rounding and origin.
            SkMatrix matrix = deviceMatrix;
            matrix.preTranslate(origin.x(), origin.y());
            SkPoint rounding = cache->rounding();
            matrix.postTranslate(rounding.x(), rounding.y());
            matrix.mapPoints(fPositions, glyphRun.positions().data(), runSize);

            SkTDArray<SkMask> masks;
            masks.setReserve(runSize);
            const SkPoint* positionCursor = fPositions;
            for (auto glyphID : glyphRun.glyphsIDs()) {
                auto position = *positionCursor++;
                if (check_glyph_position(position)) {
                    const SkGlyph& glyph = cache->getGlyphMetrics(glyphID, position);
                    const void* image;
                    if (!glyph.isEmpty() && (image = cache->findImage(glyph))) {
                        masks.push_back(create_mask(glyph, position, image));
                    }
                }
            }
            bitmapDevice->paintMasks(SkSpan<const SkMask>{masks.begin(), masks.size()}, runPaint);
        }
    }
}

// Getting glyphs to the screen in a fallback situation can be complex. Here is the set of
// transformations that have to happen. Normally, they would all be accommodated by the font
// scaler, but the atlas has an upper limit to the glyphs it can handle. So the GPU is used to
// make up the difference from the smaller atlas size to the larger size needed by the final
// transform. Here are the transformations that are applied.
//
// final transform = [view matrix] * [text scale] * [text size]
//
// There are three cases:
// * Go Fast - view matrix is scale and translate, and all the glyphs are small enough
//   Just scale the positions, and have the glyph cache handle the view matrix transformation.
//   The text scale is 1.
// * It's complicated - view matrix is not scale and translate, and the glyphs are small enough
//   The glyph cache does not handle the view matrix, but stores the glyphs at the text size
//   specified by the run paint. The GPU handles the rotation, etc. specified by the view matrix.
//   The text scale is 1.
// * Too big - The glyphs are too big to fit in the atlas
//   Reduce the text size so the glyphs will fit in the atlas, but don't apply any
//   transformations from the view matrix. Calculate a text scale based on that reduction. This
//   scale factor is used to increase the size of the destination rectangles. The destination
//   rectangles are then scaled, rotated, etc. by the GPU using the view matrix.
void SkGlyphRunListPainter::processARGBFallback(
        SkScalar maxGlyphDimension, const SkPaint& runPaint, const SkFont& runFont,
        const SkMatrix& viewMatrix, SkScalar textScale, ARGBFallback argbFallback) {
    SkASSERT(!fARGBGlyphsIDs.empty());

    SkScalar maxScale = viewMatrix.getMaxScale();

    // This is a conservative estimate of the longest dimension among all the glyph widths and
    // heights.
    SkScalar conservativeMaxGlyphDimension = maxGlyphDimension * textScale * maxScale;

    // If the situation that the matrix is simple, and all the glyphs are small enough. Go fast!
    bool useFastPath =
            viewMatrix.isScaleTranslate() && conservativeMaxGlyphDimension <= maxGlyphDimension;

    auto glyphIDs = SkSpan<const SkGlyphID>{fARGBGlyphsIDs};

    // A scaled and translated transform is the common case, and is handled directly in fallback.
    // Even if the transform is scale and translate, fallback must be careful to use glyphs that
    // fit in the atlas. If a glyph will not fit in the atlas, then the general transform case is
    // used to render the glyphs.
    if (useFastPath) {
        // Translate the positions to device space.
        viewMatrix.mapPoints(fARGBPositions.data(), fARGBPositions.size());
        for (SkPoint& point : fARGBPositions) {
            point.fX =  SkScalarFloorToScalar(point.fX);
            point.fY =  SkScalarFloorToScalar(point.fY);
        }

        auto positions = SkSpan<const SkPoint>{fARGBPositions};
        argbFallback(runPaint,
                     runFont,
                     glyphIDs,
                     positions,
                     SK_Scalar1,
                     viewMatrix,
                     kTransformDone);

    } else {
        // If the matrix is complicated or if scaling is used to fit the glyphs in the cache,
        // then this case is used.

        // Subtract 2 to account for the bilerp pad around the glyph
        SkScalar maxAtlasDimension = SkGlyphCacheCommon::kSkSideTooBigForAtlas - 2;

        SkScalar runPaintTextSize = runPaint.getTextSize();

        // Scale the text size down so the long side of all the glyphs will fit in the atlas.
        SkScalar reducedTextSize =
                (maxAtlasDimension / conservativeMaxGlyphDimension) * runPaintTextSize;

        // If there's a glyph in the font that's particularly large, it's possible
        // that fScaledFallbackTextSize may end up minimizing too much. We'd rather skip
        // that glyph than make the others blurry, so we set a minimum size of half the
        // maximum text size to avoid this case.
        SkScalar fallbackTextSize =
                SkScalarFloorToScalar(std::max(reducedTextSize, 0.5f * runPaintTextSize));

        // Don't allow the text size to get too big. This will also improve glyph cache hit rate
        // for larger text sizes.
        fallbackTextSize = std::min(fallbackTextSize, 256.0f);

        SkFont fallbackFont{runFont};
        fallbackFont.setSize(fallbackTextSize);
        SkScalar fallbackTextScale = runPaintTextSize / fallbackTextSize;
        auto positions = SkSpan<const SkPoint>{fARGBPositions};
        argbFallback(runPaint,
                     fallbackFont,
                     glyphIDs,
                     positions,
                     fallbackTextScale,
                     SkMatrix::I(),
                     kDoTransform);
    }
}

// Beware! The following code will end up holding two glyph caches at the same time, but they
// will not be the same cache (which would cause two separate caches to be created).
template <typename PerEmptyT, typename PerPathT>
void SkGlyphRunListPainter::drawGlyphRunAsPathWithARGBFallback(
        SkGlyphCacheInterface* pathCache, const SkGlyphRun& glyphRun,
        SkPoint origin, const SkPaint& runPaint, const SkMatrix& viewMatrix, SkScalar textScale,
        PerEmptyT&& perEmpty, PerPathT&& perPath, ARGBFallback&& argbFallback) {
    fARGBGlyphsIDs.clear();
    fARGBPositions.clear();
    SkScalar maxFallbackDimension{-SK_ScalarInfinity};

    const SkPoint* positionCursor = glyphRun.positions().data();
    for (auto glyphID : glyphRun.glyphsIDs()) {
        SkPoint glyphPos = origin + *positionCursor++;
        const SkGlyph& glyph = pathCache->getGlyphMetrics(glyphID, {0, 0});
        if (glyph.isEmpty()) {
            perEmpty(glyph, glyphPos);
        } else if (glyph.fMaskFormat != SkMask::kARGB32_Format) {
            if (pathCache->hasPath(glyph)) {
                perPath(glyph, glyphPos);
            } else {
                perEmpty(glyph, glyphPos);
            }
        } else {
            SkScalar largestDimension = std::max(glyph.fWidth, glyph.fHeight);
            maxFallbackDimension = std::max(maxFallbackDimension, largestDimension);
            fARGBGlyphsIDs.push_back(glyphID);
            fARGBPositions.push_back(glyphPos);
        }
    }

    if (!fARGBGlyphsIDs.empty()) {
        this->processARGBFallback(
                maxFallbackDimension, runPaint, glyphRun.font(), viewMatrix, textScale,
                std::move(argbFallback));

    }
}

template <typename PerEmptyT, typename PerGlyphT, typename PerPathT>
void SkGlyphRunListPainter::drawGlyphRunAsBMPWithPathFallback(
        SkGlyphCacheInterface* cache, const SkGlyphRun& glyphRun,
        SkPoint origin, const SkMatrix& deviceMatrix,
        PerEmptyT&& perEmpty, PerGlyphT&& perGlyph, PerPathT&& perPath) {

    SkMatrix mapping = deviceMatrix;
    mapping.preTranslate(origin.x(), origin.y());
    SkVector rounding = cache->rounding();
    mapping.postTranslate(rounding.x(), rounding.y());

    auto runSize = glyphRun.runSize();
    this->ensureBitmapBuffers(runSize);
    mapping.mapPoints(fPositions, glyphRun.positions().data(), runSize);
    const SkPoint* mappedPtCursor = fPositions;
    for (auto glyphID : glyphRun.glyphsIDs()) {
        auto mappedPt = *mappedPtCursor++;
        if (SkScalarsAreFinite(mappedPt.x(), mappedPt.y())) {
            const SkGlyph& glyph = cache->getGlyphMetrics(glyphID, mappedPt);
            if (glyph.isEmpty()) {
                perEmpty(glyph, mappedPt);
            } else if (!SkGlyphCacheCommon::GlyphTooBigForAtlas(glyph)) {
                // TODO: this check is probably not needed. Remove when proven.
                if (cache->hasImage(glyph)) {
                    perGlyph(glyph, mappedPt);
                } else {
                    perEmpty(glyph, mappedPt);
                }
            } else {
                if (cache->hasPath(glyph)) {
                    perPath(glyph, mappedPt);
                } else {
                    perEmpty(glyph, mappedPt);
                }
            }
        }
    }
}

template <typename PerEmptyT, typename PerSDFT, typename PerPathT>
void SkGlyphRunListPainter::drawGlyphRunAsSDFWithARGBFallback(
        SkGlyphCacheInterface* cache, const SkGlyphRun& glyphRun,
        SkPoint origin, const SkPaint& runPaint, const SkMatrix& viewMatrix, SkScalar textScale,
        PerEmptyT&& perEmpty, PerSDFT&& perSDF, PerPathT&& perPath, ARGBFallback&& argbFallback) {
    fARGBGlyphsIDs.clear();
    fARGBPositions.clear();
    SkScalar maxFallbackDimension{-SK_ScalarInfinity};

    const SkPoint* positionCursor = glyphRun.positions().data();
    for (auto glyphID : glyphRun.glyphsIDs()) {
        const SkGlyph& glyph = cache->getGlyphMetrics(glyphID, {0, 0});
        SkPoint glyphPos = origin + *positionCursor++;
        if (glyph.isEmpty()) {
            perEmpty(glyph, glyphPos);
        } else if (glyph.fMaskFormat == SkMask::kSDF_Format) {
            if (!SkGlyphCacheCommon::GlyphTooBigForAtlas(glyph)) {
                // TODO: this check is probably not needed. Remove when proven.
                if (cache->hasImage(glyph)) {
                    perSDF(glyph, glyphPos);
                } else {
                    perEmpty(glyph, glyphPos);
                }
            } else {
                if (cache->hasPath(glyph)) {
                    perPath(glyph, glyphPos);
                } else {
                    perEmpty(glyph, glyphPos);
                }
            }
        } else {
            SkASSERT(glyph.fMaskFormat == SkMask::kARGB32_Format);
            SkScalar largestDimension = std::max(glyph.fWidth, glyph.fHeight);
            maxFallbackDimension = std::max(maxFallbackDimension, largestDimension);
            fARGBGlyphsIDs.push_back(glyphID);
            fARGBPositions.push_back(glyphPos);
        }
    }

    if (!fARGBGlyphsIDs.empty()) {
        this->processARGBFallback(
                maxFallbackDimension, runPaint, glyphRun.font(), viewMatrix, textScale,
                std::move(argbFallback));
    }
}

#if SK_SUPPORT_GPU
// -- GrTextContext --------------------------------------------------------------------------------
SkPMColor4f generate_filtered_color(const SkPaint& paint, const GrColorSpaceInfo& colorSpaceInfo) {
    SkColor4f filteredColor = paint.getColor4f();
    if (auto* xform = colorSpaceInfo.colorSpaceXformFromSRGB()) {
        filteredColor = xform->apply(filteredColor);
    }
    if (paint.getColorFilter() != nullptr) {
        filteredColor = paint.getColorFilter()->filterColor4f(filteredColor,
                                                              colorSpaceInfo.colorSpace());
    }
    return filteredColor.premul();
}

void GrTextContext::drawGlyphRunList(
        GrContext* context, GrTextTarget* target, const GrClip& clip,
        const SkMatrix& viewMatrix, const SkSurfaceProps& props,
        const SkGlyphRunList& glyphRunList) {
    SkPoint origin = glyphRunList.origin();

    // Get the first paint to use as the key paint.
    const SkPaint& listPaint = glyphRunList.paint();

    SkPMColor4f filteredColor = generate_filtered_color(listPaint, target->colorSpaceInfo());
    GrColor color = generate_filtered_color(listPaint, target->colorSpaceInfo()).toBytes_RGBA();

    // If we have been abandoned, then don't draw
    if (context->abandoned()) {
        return;
    }

    SkMaskFilterBase::BlurRec blurRec;
    // It might be worth caching these things, but its not clear at this time
    // TODO for animated mask filters, this will fill up our cache.  We need a safeguard here
    const SkMaskFilter* mf = listPaint.getMaskFilter();
    bool canCache = glyphRunList.canCache() && !(listPaint.getPathEffect() ||
                                                 (mf && !as_MFB(mf)->asABlur(&blurRec)));
    SkScalerContextFlags scalerContextFlags = ComputeScalerContextFlags(target->colorSpaceInfo());

    auto glyphCache = context->contextPriv().getGlyphCache();
    GrTextBlobCache* textBlobCache = context->contextPriv().getTextBlobCache();

    sk_sp<GrTextBlob> cacheBlob;
    GrTextBlob::Key key;
    if (canCache) {
        bool hasLCD = glyphRunList.anyRunsLCD();

        // We canonicalize all non-lcd draws to use kUnknown_SkPixelGeometry
        SkPixelGeometry pixelGeometry = hasLCD ? props.pixelGeometry() :
                                        kUnknown_SkPixelGeometry;

        // TODO we want to figure out a way to be able to use the canonical color on LCD text,
        // see the note on ComputeCanonicalColor above.  We pick a dummy value for LCD text to
        // ensure we always match the same key
        GrColor canonicalColor = hasLCD ? SK_ColorTRANSPARENT :
                                 ComputeCanonicalColor(listPaint, hasLCD);

        key.fPixelGeometry = pixelGeometry;
        key.fUniqueID = glyphRunList.uniqueID();
        key.fStyle = listPaint.getStyle();
        key.fHasBlur = SkToBool(mf);
        key.fCanonicalColor = canonicalColor;
        key.fScalerContextFlags = scalerContextFlags;
        cacheBlob = textBlobCache->find(key);
    }

    if (cacheBlob) {
        if (cacheBlob->mustRegenerate(listPaint, glyphRunList.anyRunsSubpixelPositioned(),
                                      blurRec, viewMatrix, origin.x(),origin.y())) {
            // We have to remake the blob because changes may invalidate our masks.
            // TODO we could probably get away reuse most of the time if the pointer is unique,
            // but we'd have to clear the subrun information
            textBlobCache->remove(cacheBlob.get());
            cacheBlob = textBlobCache->makeCachedBlob(glyphRunList, key, blurRec, listPaint, color);
            cacheBlob->generateFromGlyphRunList(
                    glyphCache, *context->contextPriv().caps()->shaderCaps(), fOptions,
                    listPaint, scalerContextFlags, viewMatrix, props,
                    glyphRunList, target->glyphPainter());
        } else {
            textBlobCache->makeMRU(cacheBlob.get());

            if (CACHE_SANITY_CHECK) {
                sk_sp<GrTextBlob> sanityBlob(textBlobCache->makeBlob(glyphRunList, color));
                sanityBlob->setupKey(key, blurRec, listPaint);
                cacheBlob->generateFromGlyphRunList(
                        glyphCache, *context->contextPriv().caps()->shaderCaps(), fOptions,
                        listPaint, scalerContextFlags, viewMatrix, props, glyphRunList,
                        target->glyphPainter());
                GrTextBlob::AssertEqual(*sanityBlob, *cacheBlob);
            }
        }
    } else {
        if (canCache) {
            cacheBlob = textBlobCache->makeCachedBlob(glyphRunList, key, blurRec, listPaint, color);
        } else {
            cacheBlob = textBlobCache->makeBlob(glyphRunList, color);
        }
        cacheBlob->generateFromGlyphRunList(
                glyphCache, *context->contextPriv().caps()->shaderCaps(), fOptions, listPaint,
                scalerContextFlags, viewMatrix, props, glyphRunList,
                target->glyphPainter());
    }

    cacheBlob->flush(target, props, fDistanceAdjustTable.get(), listPaint, filteredColor,
                     clip, viewMatrix, origin.x(), origin.y());
}

void GrTextBlob::SubRun::appendGlyph(GrTextBlob* blob, GrGlyph* glyph, SkRect dstRect) {

    this->joinGlyphBounds(dstRect);

    bool hasW = this->hasWCoord();
    // glyphs drawn in perspective must always have a w coord.
    SkASSERT(hasW || !blob->fInitialViewMatrix.hasPerspective());
    auto maskFormat = this->maskFormat();
    size_t vertexStride = GetVertexStride(maskFormat, hasW);

    intptr_t vertex = reinterpret_cast<intptr_t>(blob->fVertices + fVertexEndIndex);

    // We always write the third position component used by SDFs. If it is unused it gets
    // overwritten. Similarly, we always write the color and the blob will later overwrite it
    // with texture coords if it is unused.
    size_t colorOffset = hasW ? sizeof(SkPoint3) : sizeof(SkPoint);
    // V0
    *reinterpret_cast<SkPoint3*>(vertex) = {dstRect.fLeft, dstRect.fTop, 1.f};
    *reinterpret_cast<GrColor*>(vertex + colorOffset) = fColor;
    vertex += vertexStride;

    // V1
    *reinterpret_cast<SkPoint3*>(vertex) = {dstRect.fLeft, dstRect.fBottom, 1.f};
    *reinterpret_cast<GrColor*>(vertex + colorOffset) = fColor;
    vertex += vertexStride;

    // V2
    *reinterpret_cast<SkPoint3*>(vertex) = {dstRect.fRight, dstRect.fTop, 1.f};
    *reinterpret_cast<GrColor*>(vertex + colorOffset) = fColor;
    vertex += vertexStride;

    // V3
    *reinterpret_cast<SkPoint3*>(vertex) = {dstRect.fRight, dstRect.fBottom, 1.f};
    *reinterpret_cast<GrColor*>(vertex + colorOffset) = fColor;

    fVertexEndIndex += vertexStride * kVerticesPerGlyph;
    blob->fGlyphs[fGlyphEndIndex++] = glyph;
}

static SkRect rect_to_draw(
        const SkGlyph& glyph, SkPoint origin, SkScalar textScale, bool isDFT) {

    SkScalar dx = SkIntToScalar(glyph.fLeft);
    SkScalar dy = SkIntToScalar(glyph.fTop);
    SkScalar width = SkIntToScalar(glyph.fWidth);
    SkScalar height = SkIntToScalar(glyph.fHeight);

    if (isDFT) {
        dx += SK_DistanceFieldInset;
        dy += SK_DistanceFieldInset;
        width -= 2 * SK_DistanceFieldInset;
        height -= 2 * SK_DistanceFieldInset;
    }

    dx *= textScale;
    dy *= textScale;
    width *= textScale;
    height *= textScale;

    return SkRect::MakeXYWH(origin.x() + dx, origin.y() + dy, width, height);
}

void GrTextBlob::Run::appendGlyph(GrTextBlob* blob,
                                  const sk_sp<GrTextStrike>& strike,
                                  const SkGlyph& skGlyph, GrGlyph::MaskStyle maskStyle,
                                  SkPoint origin, SkScalar textRatio, bool needsTransform) {

    GrGlyph::PackedID id = GrGlyph::Pack(skGlyph.getGlyphID(),
                                         skGlyph.getSubXFixed(),
                                         skGlyph.getSubYFixed(),
                                         maskStyle);
    GrGlyph* glyph = strike->getGlyph(skGlyph, id);
    if (!glyph) {
        return;
    }

    SkASSERT(skGlyph.fWidth == glyph->width());
    SkASSERT(skGlyph.fHeight == glyph->height());

    bool isDFT = maskStyle == GrGlyph::kDistance_MaskStyle;

    SkRect glyphRect = rect_to_draw(skGlyph, origin, textRatio, isDFT);
    if (!glyphRect.isEmpty()) {
        GrMaskFormat format = glyph->fMaskFormat;

        SubRun* subRun = &fSubRunInfo.back();
        if (fInitialized && subRun->maskFormat() != format) {
            subRun = pushBackSubRun(fDescriptor, fColor);
            subRun->setStrike(strike);
        } else if (!fInitialized) {
            subRun->setStrike(strike);
        }

        fInitialized = true;
        subRun->setMaskFormat(format);
        subRun->setNeedsTransform(needsTransform);

        subRun->appendGlyph(blob, glyph, glyphRect);
    }
}


void GrTextBlob::generateFromGlyphRunList(GrGlyphCache* glyphCache,
                                          const GrShaderCaps& shaderCaps,
                                          const GrTextContext::Options& options,
                                          const SkPaint& paint,
                                          SkScalerContextFlags scalerContextFlags,
                                          const SkMatrix& viewMatrix,
                                          const SkSurfaceProps& props,
                                          const SkGlyphRunList& glyphRunList,
                                          SkGlyphRunListPainter* glyphPainter) {
    struct ARGBFallbackHelper {
        void operator()(const SkPaint& fallbackPaint, const SkFont& fallbackFont,
                        SkSpan<const SkGlyphID> glyphIDs,
                        SkSpan<const SkPoint> positions, SkScalar textScale,
                        const SkMatrix& glyphCacheMatrix,
                        SkGlyphRunListPainter::NeedsTransform needsTransform) const {
            fBlob->setHasBitmap();
            fRun->setSubRunHasW(glyphCacheMatrix.hasPerspective());
            auto subRun = fRun->initARGBFallback();
            SkExclusiveStrikePtr fallbackCache =
                    fRun->setupCache(fallbackPaint,
                                     fallbackFont,
                                     fProps,
                                     fScalerContextFlags,
                                     glyphCacheMatrix);
            sk_sp<GrTextStrike> strike = fGlyphCache->getStrike(fallbackCache.get());
            SkASSERT(strike != nullptr);
            subRun->setStrike(strike);
            const SkPoint* glyphPos = positions.data();
            for (auto glyphID : glyphIDs) {
                const SkGlyph& glyph = fallbackCache->getGlyphIDMetrics(glyphID);
                fRun->appendGlyph(fBlob, strike, glyph,
                                 GrGlyph::kCoverage_MaskStyle,
                                 *glyphPos, textScale, needsTransform);
                glyphPos++;
            }
        }

        GrTextBlob* const fBlob;
        GrTextBlob::Run* fRun;
        const SkSurfaceProps& fProps;
        const SkScalerContextFlags fScalerContextFlags;
        GrGlyphCache* const fGlyphCache;
    };

    SkPoint origin = glyphRunList.origin();
    const SkPaint& runPaint = glyphRunList.paint();
    this->initReusableBlob(runPaint.computeLuminanceColor(), viewMatrix, origin.x(), origin.y());

    for (const auto& glyphRun : glyphRunList) {
        const SkFont& runFont = glyphRun.font();

        Run* run = this->pushBackRun();

        run->setRunFontAntiAlias(runFont.hasSomeAntiAliasing());

        if (GrTextContext::CanDrawAsDistanceFields(runPaint, runFont, viewMatrix, props,
                                    shaderCaps.supportsDistanceFieldText(), options)) {
            bool hasWCoord = viewMatrix.hasPerspective()
                             || options.fDistanceFieldVerticesAlwaysHaveW;

            // Setup distance field runPaint and text ratio
            SkScalar textScale;
            SkPaint distanceFieldPaint{runPaint};
            runFont.LEGACY_applyToPaint(&distanceFieldPaint);
            SkScalerContextFlags flags;
            GrTextContext::InitDistanceFieldPaint(this, &distanceFieldPaint, viewMatrix,
                                                  options, &textScale, &flags);
            this->setHasDistanceField();
            run->setSubRunHasDistanceFields(
                    runFont.getEdging() == SkFont::Edging::kSubpixelAntiAlias,
                    runFont.hasSomeAntiAliasing(),
                    hasWCoord);

            {
                SkFont font = SkFont::LEGACY_ExtractFromPaint(distanceFieldPaint);
                auto cache = run->setupCache(distanceFieldPaint, font, props, flags, SkMatrix::I());

                sk_sp<GrTextStrike> currStrike = glyphCache->getStrike(cache.get());

                auto perEmpty = [](const SkGlyph&, SkPoint) {};

                auto perSDF =
                    [this, run, &currStrike, textScale]
                    (const SkGlyph& glyph, SkPoint position) {
                        run->appendGlyph(this, currStrike,
                                    glyph, GrGlyph::kDistance_MaskStyle, position,
                                    textScale, true);
                    };

                auto perPath =
                    [run, textScale]
                    (const SkGlyph& glyph, SkPoint position) {
                        // TODO: path should always be set. Remove when proven.
                        if (const SkPath* glyphPath = glyph.path()) {
                            run->appendPathGlyph(*glyphPath, position, textScale, false);
                        }
                    };

                ARGBFallbackHelper argbFallback{this, run, props, scalerContextFlags,
                                                glyphCache};

                glyphPainter->drawGlyphRunAsSDFWithARGBFallback(
                    cache.get(), glyphRun, origin, runPaint, viewMatrix, textScale,
                    std::move(perEmpty), std::move(perSDF), std::move(perPath),
                    std::move(argbFallback));
            }

        } else if (SkGlyphRunListPainter::ShouldDrawAsPath(runPaint, runFont, viewMatrix)) {
            // The glyphs are big, so use paths to draw them.

            // Ensure the blob is set for bitmaptext
            this->setHasBitmap();

            // setup our std runPaint, in hopes of getting hits in the cache
            SkPaint pathPaint{runPaint};
            SkFont pathFont{runFont};
            SkScalar textScale = pathFont.setupForAsPaths(&pathPaint);

            auto pathCache = SkStrikeCache::FindOrCreateStrikeExclusive(
                                pathFont, pathPaint, props,
                                scalerContextFlags, SkMatrix::I());

            auto perEmpty = [](const SkGlyph&, SkPoint) {};

            // Given a glyph that is not ARGB, draw it.
            auto perPath = [textScale, run]
                           (const SkGlyph& glyph, SkPoint position) {
                // TODO: path should always be set. Remove when proven.
                if (const SkPath* glyphPath = glyph.path()) {
                    run->appendPathGlyph(*glyphPath, position, textScale, false);
                }
            };

            ARGBFallbackHelper argbFallback{this, run, props, scalerContextFlags, glyphCache};

            glyphPainter->drawGlyphRunAsPathWithARGBFallback(
                pathCache.get(), glyphRun, origin, runPaint, viewMatrix, textScale,
                std::move(perEmpty), std::move(perPath), std::move(argbFallback));
        } else {
            // Ensure the blob is set for bitmaptext
            this->setHasBitmap();

            auto cache = run->setupCache(runPaint, runFont, props, scalerContextFlags, viewMatrix);

            sk_sp<GrTextStrike> currStrike = glyphCache->getStrike(cache.get());

            auto perEmpty = [](const SkGlyph&, SkPoint) {};

            auto perGlyph =
                [this, run, &currStrike]
                (const SkGlyph& glyph, SkPoint mappedPt) {
                    SkPoint pt{SkScalarFloorToScalar(mappedPt.fX),
                               SkScalarFloorToScalar(mappedPt.fY)};
                    run->appendGlyph(this, currStrike,
                                     glyph, GrGlyph::kCoverage_MaskStyle, pt,
                                     SK_Scalar1, false);
                };

            auto perPath =
                [run]
                (const SkGlyph& glyph, SkPoint position) {
                    SkPoint pt{SkScalarFloorToScalar(position.fX),
                               SkScalarFloorToScalar(position.fY)};
                    // TODO: path should always be set. Remove when proven.
                    if (const SkPath* glyphPath = glyph.path()) {
                        run->appendPathGlyph(*glyphPath, pt, SK_Scalar1, true);
                    }
                };

            glyphPainter->drawGlyphRunAsBMPWithPathFallback(
                    cache.get(), glyphRun, origin, viewMatrix,
                    std::move(perEmpty), std::move(perGlyph), std::move(perPath));
        }
    }
}

#if GR_TEST_UTILS

#include "GrRenderTargetContext.h"

std::unique_ptr<GrDrawOp> GrTextContext::createOp_TestingOnly(GrContext* context,
                                                              GrTextContext* textContext,
                                                              GrRenderTargetContext* rtc,
                                                              const SkPaint& skPaint,
                                                              const SkMatrix& viewMatrix,
                                                              const char* text,
                                                              int x,
                                                              int y) {
    auto glyphCache = context->contextPriv().getGlyphCache();

    static SkSurfaceProps surfaceProps(SkSurfaceProps::kLegacyFontHost_InitType);

    size_t textLen = (int)strlen(text);

    SkPMColor4f filteredColor = generate_filtered_color(skPaint, rtc->colorSpaceInfo());
    GrColor color = filteredColor.toBytes_RGBA();

    auto origin = SkPoint::Make(x, y);
    SkGlyphRunBuilder builder;
    builder.drawText(skPaint, text, textLen, origin);

    auto glyphRunList = builder.useGlyphRunList();
    sk_sp<GrTextBlob> blob;
    if (!glyphRunList.empty()) {
        blob = context->contextPriv().getTextBlobCache()->makeBlob(glyphRunList, color);
        // Use the text and textLen below, because we don't want to mess with the paint.
        SkScalerContextFlags scalerContextFlags =
                ComputeScalerContextFlags(rtc->colorSpaceInfo());
        blob->generateFromGlyphRunList(
                glyphCache, *context->contextPriv().caps()->shaderCaps(), textContext->fOptions,
                skPaint, scalerContextFlags, viewMatrix, surfaceProps,
                glyphRunList, rtc->textTarget()->glyphPainter());
    }

    return blob->test_makeOp(textLen, 0, 0, viewMatrix, x, y, skPaint, filteredColor, surfaceProps,
                             textContext->dfAdjustTable(), rtc->textTarget());
}

#endif  // GR_TEST_UTILS
#endif  // SK_SUPPORT_GPU

// -- SkTextBlobCacheDiffCanvas::TrackLayerDevice --------------------------------------------------

void SkTextBlobCacheDiffCanvas::TrackLayerDevice::processGlyphRun(
        const SkPoint& origin, const SkGlyphRun& glyphRun, const SkPaint& runPaint) {
    TRACE_EVENT0("skia", "SkTextBlobCacheDiffCanvas::processGlyphRun");

    const SkMatrix& runMatrix = this->ctm();

    // If the matrix has perspective, we fall back to using distance field text or paths.
#if SK_SUPPORT_GPU
    if (this->maybeProcessGlyphRunForDFT(glyphRun, runMatrix, origin, runPaint)) {
        return;
    } else
#endif
    if (SkGlyphRunListPainter::ShouldDrawAsPath(runPaint, glyphRun.font(), runMatrix)) {
        this->processGlyphRunForPaths(glyphRun, runMatrix, origin, runPaint);
    } else {
        this->processGlyphRunForMask(glyphRun, runMatrix, origin, runPaint);
    }
}

void SkTextBlobCacheDiffCanvas::TrackLayerDevice::processGlyphRunForMask(
        const SkGlyphRun& glyphRun, const SkMatrix& runMatrix,
        SkPoint origin, const SkPaint& runPaint) {
    TRACE_EVENT0("skia", "SkTextBlobCacheDiffCanvas::processGlyphRunForMask");

    SkScalerContextEffects effects;
    auto* glyphCacheState = fStrikeServer->getOrCreateCache(
            runPaint, glyphRun.font(), this->surfaceProps(), runMatrix,
            SkScalerContextFlags::kFakeGammaAndBoostContrast, &effects);
    SkASSERT(glyphCacheState);

    auto perEmpty = [glyphCacheState] (const SkGlyph& glyph, SkPoint mappedPt) {
        glyphCacheState->addGlyph(glyph.getPackedID(), false);
    };

    auto perGlyph = [glyphCacheState] (const SkGlyph& glyph, SkPoint mappedPt) {
        glyphCacheState->addGlyph(glyph.getPackedID(), false);
    };

    // Glyphs which are too large for the atlas still request images when computing the bounds
    // for the glyph, which is why its necessary to send both. See related code in
    // get_packed_glyph_bounds in GrGlyphCache.cpp and crbug.com/510931.
    auto perPath = [glyphCacheState] (const SkGlyph& glyph, SkPoint mappedPt) {
        glyphCacheState->addGlyph(glyph.getPackedID(), true);
        glyphCacheState->addGlyph(glyph.getPackedID(), false);
    };

    fPainter.drawGlyphRunAsBMPWithPathFallback(
            glyphCacheState, glyphRun, origin, runMatrix,
            std::move(perEmpty), std::move(perGlyph), std::move(perPath));
}

struct ARGBHelper {
    void operator()(const SkPaint& fallbackPaint,
                    const SkFont& fallbackFont,
                    SkSpan<const SkGlyphID> glyphIDs,
                    SkSpan<const SkPoint> positions,
                    SkScalar textScale,
                    const SkMatrix& glyphCacheMatrix,
                    SkGlyphRunListPainter::NeedsTransform needsTransform) {
        TRACE_EVENT0("skia", "argbFallback");

        SkScalerContextEffects effects;
        auto* fallbackCache =
                fStrikeServer->getOrCreateCache(
                        fallbackPaint, fallbackFont, fSurfaceProps, fFallbackMatrix,
                        SkScalerContextFlags::kFakeGammaAndBoostContrast, &effects);

        for (auto glyphID : glyphIDs) {
            fallbackCache->addGlyph(SkPackedGlyphID(glyphID, 0, 0), false);
        }
    }

    const SkMatrix& fFallbackMatrix;
    const SkSurfaceProps& fSurfaceProps;
    SkStrikeServer* const fStrikeServer;
};

SkScalar SkTextBlobCacheDiffCanvas::SetupForPath(SkPaint* paint, SkFont* font) {
    return font->setupForAsPaths(paint);
}

void SkTextBlobCacheDiffCanvas::TrackLayerDevice::processGlyphRunForPaths(
        const SkGlyphRun& glyphRun, const SkMatrix& runMatrix,
        SkPoint origin, const SkPaint& runPaint) {
    TRACE_EVENT0("skia", "SkTextBlobCacheDiffCanvas::processGlyphRunForPaths");

    SkPaint pathPaint{runPaint};
    SkFont pathFont{glyphRun.font()};
    SkScalar textScale = SetupForPath(&pathPaint, &pathFont);

    SkScalerContextEffects effects;
    auto* glyphCacheState = fStrikeServer->getOrCreateCache(
            pathPaint, pathFont, this->surfaceProps(), SkMatrix::I(),
            SkScalerContextFlags::kFakeGammaAndBoostContrast, &effects);

    auto perEmpty = [glyphCacheState] (const SkGlyph& glyph, SkPoint mappedPt) {
        glyphCacheState->addGlyph(glyph.getPackedID(), false);
    };

    auto perPath = [glyphCacheState](const SkGlyph& glyph, SkPoint position) {
        const bool asPath = true;
        glyphCacheState->addGlyph(glyph.getGlyphID(), asPath);
    };

    ARGBHelper argbFallback{runMatrix, surfaceProps(), fStrikeServer};

    fPainter.drawGlyphRunAsPathWithARGBFallback(
            glyphCacheState, glyphRun, origin, runPaint, runMatrix, textScale,
            std::move(perEmpty), std::move(perPath), std::move(argbFallback));
}

#if SK_SUPPORT_GPU
bool SkTextBlobCacheDiffCanvas::TrackLayerDevice::maybeProcessGlyphRunForDFT(
        const SkGlyphRun& glyphRun, const SkMatrix& runMatrix,
        SkPoint origin, const SkPaint& runPaint) {
    TRACE_EVENT0("skia", "SkTextBlobCacheDiffCanvas::maybeProcessGlyphRunForDFT");

    const SkFont& runFont = glyphRun.font();

    GrTextContext::Options options;
    options.fMinDistanceFieldFontSize = fSettings.fMinDistanceFieldFontSize;
    options.fMaxDistanceFieldFontSize = fSettings.fMaxDistanceFieldFontSize;
    GrTextContext::SanitizeOptions(&options);
    if (!GrTextContext::CanDrawAsDistanceFields(runPaint, runFont,
                                                runMatrix, this->surfaceProps(),
                                                fSettings.fContextSupportsDistanceFieldText,
                                                options)) {
        return false;
    }

    SkScalar textRatio;
    SkPaint dfPaint(runPaint);
    runFont.LEGACY_applyToPaint(&dfPaint);
    SkScalerContextFlags flags;
    GrTextContext::InitDistanceFieldPaint(nullptr, &dfPaint, runMatrix, options, &textRatio,
                                          &flags);
    SkScalerContextEffects effects;
    SkFont dfFont = SkFont::LEGACY_ExtractFromPaint(dfPaint);
    auto* sdfCache = fStrikeServer->getOrCreateCache(dfPaint, dfFont, this->surfaceProps(),
                                                     SkMatrix::I(), flags, &effects);

    ARGBHelper argbFallback{runMatrix, surfaceProps(), fStrikeServer};

    auto perEmpty = [sdfCache] (const SkGlyph& glyph, SkPoint mappedPt) {
        sdfCache->addGlyph(glyph.getPackedID(), false);
    };

    auto perSDF = [sdfCache] (const SkGlyph& glyph, SkPoint position) {
        const bool asPath = false;
        sdfCache->addGlyph(glyph.getGlyphID(), asPath);
    };

    auto perPath = [sdfCache] (const SkGlyph& glyph, SkPoint position) {
        const bool asPath = true;
        sdfCache->addGlyph(glyph.getGlyphID(), asPath);
    };

    fPainter.drawGlyphRunAsSDFWithARGBFallback(
            sdfCache, glyphRun, origin, runPaint, runMatrix, textRatio,
            std::move(perEmpty), std::move(perSDF), std::move(perPath),
            std::move(argbFallback));

    return true;
}
#endif
