/*
 * Copyright 2018 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGlyphRunPainter.h"

#if SK_SUPPORT_GPU
#include "GrCaps.h"
#include "GrColorSpaceInfo.h"
#include "GrContextPriv.h"
#include "GrRecordingContext.h"
#include "GrRecordingContextPriv.h"
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
#include "SkMaskFilter.h"
#include "SkPaintPriv.h"
#include "SkPathEffect.h"
#include "SkRasterClip.h"
#include "SkRemoteGlyphCacheImpl.h"
#include "SkStrikeInterface.h"
#include "SkStrike.h"
#include "SkStrikeCache.h"
#include "SkTDArray.h"
#include "SkTraceEvent.h"

// -- SkGlyphCacheCommon ---------------------------------------------------------------------------

SkVector SkStrikeCommon::PixelRounding(bool isSubpixel, SkAxisAlignment axisAlignment) {
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

SkIPoint SkStrikeCommon::SubpixelLookup(SkAxisAlignment axisAlignment, SkPoint position) {
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

bool SkStrikeCommon::GlyphTooBigForAtlas(const SkGlyph& glyph) {
    return glyph.fWidth > kSkSideTooBigForAtlas || glyph.fHeight > kSkSideTooBigForAtlas;
}

// -- SkGlyphRunListPainter ------------------------------------------------------------------------
SkGlyphRunListPainter::SkGlyphRunListPainter(const SkSurfaceProps& props,
                                             SkColorType colorType,
                                             SkScalerContextFlags flags,
                                             SkStrikeCacheInterface* strikeCache)
        : fDeviceProps{props}
        ,  fBitmapFallbackProps{SkSurfaceProps{props.flags(), kUnknown_SkPixelGeometry}}
        ,  fColorType{colorType}, fScalerContextFlags{flags}
        ,  fStrikeCache{strikeCache} {}

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
        : SkGlyphRunListPainter(props,
                                kUnknown_SkColorType,
                                compute_scaler_context_flags(csi),
                                SkStrikeCache::GlobalStrikeCache()) {}

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

    return SkFontPriv::TooBigToUseCache(matrix, SkFontPriv::MakeTextMatrix(font), 1024);
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
    ScopedBuffers _ = this->ensureBuffers(glyphRunList);

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
template<typename ProcessDeviceT, typename ProcessSourceT>
void SkGlyphRunListPainter::processARGBFallback(SkScalar maxGlyphDimension,
                                                const SkPaint& runPaint,
                                                const SkFont& runFont,
                                                const SkMatrix& viewMatrix,
                                                SkScalar textScale,
                                                ProcessDeviceT&& processDevice,
                                                ProcessSourceT&& processSource) {
    SkASSERT(!fARGBGlyphsIDs.empty());

    SkScalar maxScale = viewMatrix.getMaxScale();

    // This is a conservative estimate of the longest dimension among all the glyph widths and
    // heights.
    SkScalar conservativeMaxGlyphDimension = maxGlyphDimension * textScale * maxScale;

    // If the situation that the matrix is simple, and all the glyphs are small enough. Go fast!
    // N.B. If the matrix has scale, that will be reflected in the strike through the viewMatrix
    // in the useFastPath case.
    bool useFastPath =
            viewMatrix.isScaleTranslate() && conservativeMaxGlyphDimension <= maxGlyphDimension;

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

        SkAutoDescriptor ad;
        SkScalerContextEffects effects;
        SkScalerContext::CreateDescriptorAndEffectsUsingPaint(
                runFont, runPaint, fDeviceProps, fScalerContextFlags, viewMatrix, &ad, &effects);

        SkScopedStrike strike =
                fStrikeCache->findOrCreateScopedStrike(
                        *ad.getDesc(), effects, *runFont.getTypefaceOrDefault());

        SkPoint* posCursor = fARGBPositions.data();
        int glyphCount = 0;
        for (SkGlyphID glyphID : fARGBGlyphsIDs) {
            SkPoint pos = *posCursor++;
            const SkGlyph& glyph = strike->getGlyphMetrics(glyphID, pos);
            fGlyphPos[glyphCount++] = {&glyph, pos};
        }

        processDevice(SkSpan<const GlyphAndPos>{fGlyphPos, SkTo<size_t>(glyphCount)}, strike.get());

    } else {
        // If the matrix is complicated or if scaling is used to fit the glyphs in the cache,
        // then this case is used.

        // Subtract 2 to account for the bilerp pad around the glyph
        SkScalar maxAtlasDimension = SkStrikeCommon::kSkSideTooBigForAtlas - 2;

        SkScalar runFontTextSize = runFont.getSize();

        // Scale the text size down so the long side of all the glyphs will fit in the atlas.
        SkScalar reducedTextSize =
                (maxAtlasDimension / conservativeMaxGlyphDimension) * runFontTextSize;

        // If there's a glyph in the font that's particularly large, it's possible
        // that fScaledFallbackTextSize may end up minimizing too much. We'd rather skip
        // that glyph than make the others blurry, so we set a minimum size of half the
        // maximum text size to avoid this case.
        SkScalar fallbackTextSize =
                SkScalarFloorToScalar(std::max(reducedTextSize, 0.5f * runFontTextSize));

        // Don't allow the text size to get too big. This will also improve glyph cache hit rate
        // for larger text sizes.
        fallbackTextSize = std::min(fallbackTextSize, 256.0f);

        SkFont fallbackFont{runFont};
        fallbackFont.setSize(fallbackTextSize);

        // The scale factor to go from strike size to the source size for glyphs.
        SkScalar fallbackTextScale = runFontTextSize / fallbackTextSize;

        SkAutoDescriptor ad;
        SkScalerContextEffects effects;
        SkScalerContext::CreateDescriptorAndEffectsUsingPaint(fallbackFont,
                                                              runPaint,
                                                              fDeviceProps,
                                                              fScalerContextFlags,
                                                              viewMatrix,
                                                              &ad,
                                                              &effects);

        SkScopedStrike strike =
                fStrikeCache->findOrCreateScopedStrike(
                        *ad.getDesc(), effects, *fallbackFont.getTypefaceOrDefault());

        SkPoint* posCursor = fARGBPositions.data();
        int glyphCount = 0;
        for (SkGlyphID glyphID : fARGBGlyphsIDs) {
            SkPoint pos = *posCursor++;
            const SkGlyph& glyph = strike->getGlyphMetrics(glyphID, pos);
            fGlyphPos[glyphCount++] = {&glyph, pos};
        }

        processSource(SkSpan<const GlyphAndPos>{fGlyphPos, SkTo<size_t>(glyphCount)},
                      strike.get(),
                      fallbackTextScale,
                      viewMatrix.hasPerspective());
    }
}

// Beware! The following code will end up holding two glyph caches at the same time, but they
// will not be the same cache (which would cause two separate caches to be created).
template<typename ProcessPathsT, typename ProcessDeviceT, typename ProcessSourceT>
void SkGlyphRunListPainter::drawGlyphRunAsPathWithARGBFallback(
        const SkPaint& runPaint, const SkFont& runFont,
        const SkGlyphRun& glyphRun, SkPoint origin, const SkMatrix& viewMatrix,
        ProcessPathsT&& processPaths, ProcessDeviceT&& processDevice,
        ProcessSourceT&& processSource) {
    fARGBGlyphsIDs.clear();
    fARGBPositions.clear();
    ScopedBuffers _ = this->ensureBuffers(glyphRun);
    SkScalar maxFallbackDimension{-SK_ScalarInfinity};

    // setup our std runPaint, in hopes of getting hits in the cache
    SkPaint pathPaint{runPaint};
    SkFont pathFont{runFont};

    // The factor to get from the size stored in the strike to the size needed for the source.
    SkScalar strikeToSourceRatio = pathFont.setupForAsPaths(&pathPaint);

    SkAutoDescriptor ad;
    SkScalerContextEffects effects;
    SkScalerContext::CreateDescriptorAndEffectsUsingPaint(
            pathFont, pathPaint, fDeviceProps, fScalerContextFlags, SkMatrix::I(), &ad, &effects);

    {
        SkScopedStrike strike =
                fStrikeCache->findOrCreateScopedStrike(
                        *ad.getDesc(), effects,*pathFont.getTypefaceOrDefault());

        int glyphCount = 0;
        const SkPoint* positionCursor = glyphRun.positions().data();
        for (auto glyphID : glyphRun.glyphsIDs()) {
            SkPoint glyphPos = origin + *positionCursor++;

            // Use outline from {0, 0} because all transforms including subpixel translation happen
            // during drawing.
            const SkGlyph& glyph = strike->getGlyphMetrics(glyphID, {0, 0});
            if (!glyph.isEmpty()) {
                if (glyph.fMaskFormat != SkMask::kARGB32_Format) {
                    if (strike->decideCouldDrawFromPath(glyph)) {
                        fGlyphPos[glyphCount++] = {&glyph, glyphPos};
                    }
                } else {
                    SkScalar largestDimension = std::max(glyph.fWidth, glyph.fHeight);
                    maxFallbackDimension = std::max(maxFallbackDimension, largestDimension);
                    fARGBGlyphsIDs.push_back(glyphID);
                    fARGBPositions.push_back(glyphPos);
                }
            }
        }

        if (glyphCount > 0) {
            processPaths(SkSpan<const GlyphAndPos>{fGlyphPos, SkTo<size_t>(glyphCount)},
                         strike.get(),
                         strikeToSourceRatio);
        }
    }

    {
        // fGlyphPos will be reused here.
        if (!fARGBGlyphsIDs.empty()) {
            this->processARGBFallback(
                    maxFallbackDimension, runPaint, glyphRun.font(), viewMatrix,
                    strikeToSourceRatio,
                    std::forward<ProcessDeviceT>(processDevice),
                    std::forward<ProcessSourceT>(processSource));
        }
    }
}

template <typename MasksT, typename PathsT>
void SkGlyphRunListPainter::drawGlyphRunAsBMPWithPathFallback(
        const SkPaint& paint, const SkFont& font,
        const SkGlyphRun& glyphRun, SkPoint origin, const SkMatrix& deviceMatrix,
        MasksT&& processMasks, PathsT&& processPaths) {

    SkAutoDescriptor ad;
    SkScalerContextEffects effects;

    SkScalerContext::CreateDescriptorAndEffectsUsingPaint(
            font, paint, fDeviceProps, fScalerContextFlags, deviceMatrix, &ad, &effects);

    SkTypeface* typeface = font.getTypefaceOrDefault();
    SkScopedStrike strike =
            fStrikeCache->findOrCreateScopedStrike(*ad.getDesc(), effects, *typeface);

    ScopedBuffers _ = this->ensureBuffers(glyphRun);

    SkMatrix mapping = deviceMatrix;
    mapping.preTranslate(origin.x(), origin.y());
    SkVector rounding = strike->rounding();
    mapping.postTranslate(rounding.x(), rounding.y());
    mapping.mapPoints(fPositions,  glyphRun.positions().data(), glyphRun.runSize());

    int glyphCount = 0;
    const SkPoint* posCursor = fPositions;
    for (auto glyphID : glyphRun.glyphsIDs()) {
        SkPoint mappedPt = *posCursor++;

        if (SkScalarsAreFinite(mappedPt.x(), mappedPt.y())) {
            const SkGlyph& glyph = strike->getGlyphMetrics(glyphID, mappedPt);
            if (!glyph.isEmpty()) {
                if (SkStrikeCommon::GlyphTooBigForAtlas(glyph)) {
                    if (strike->decideCouldDrawFromPath(glyph)) {
                        fPaths.push_back({&glyph, mappedPt});
                    }
                } else {
                    // If the glyph is not empty, then it will have a pointer to mask data.
                    fGlyphPos[glyphCount++] = {&glyph, mappedPt};
                }
            }
        }
    }

    if (glyphCount > 0) {
        mapping.mapPoints(fPositions, glyphCount);
        processMasks(SkSpan<const GlyphAndPos>{fGlyphPos, SkTo<size_t>(glyphCount)}, strike.get());
    }
    if (!fPaths.empty()) {
        processPaths(SkSpan<const GlyphAndPos>{fPaths});
    }
}

#if SK_SUPPORT_GPU
template <typename ProcessMasksT, typename ProcessPathsT,
          typename ProcessDeviceT, typename ProcessSourceT>
void SkGlyphRunListPainter::drawGlyphRunAsSDFWithARGBFallback(
        const SkPaint& runPaint, const SkFont& runFont,
        const SkGlyphRun& glyphRun, SkPoint origin, const SkMatrix& viewMatrix,
        const GrTextContext::Options& options,
        ProcessMasksT&& processMasks, ProcessPathsT&& processPaths,
        ProcessDeviceT&& processDevice, ProcessSourceT&& processSource) {
    fARGBGlyphsIDs.clear();
    fARGBPositions.clear();
    ScopedBuffers _ = this->ensureBuffers(glyphRun);
    SkScalar maxFallbackDimension{-SK_ScalarInfinity};

    // Setup distance field runPaint and text ratio
    SkPaint dfPaint = GrTextContext::InitDistanceFieldPaint(runPaint);
    SkScalar textScale;
    SkFont dfFont = GrTextContext::InitDistanceFieldFont(
            runFont, viewMatrix, options, &textScale);
    // Fake-gamma and subpixel antialiasing are applied in the shader, so we ignore the
    // passed-in scaler context flags. (It's only used when we fall-back to bitmap text).
    SkScalerContextFlags flags = SkScalerContextFlags::kNone;

    SkScalar minScale, maxScale;
    std::tie(minScale, maxScale) = GrTextContext::InitDistanceFieldMinMaxScale(
            runFont.getSize(), viewMatrix, options);

    SkAutoDescriptor ad;
    SkScalerContextEffects effects;
    SkScalerContext::CreateDescriptorAndEffectsUsingPaint(
            dfFont, dfPaint, fDeviceProps, flags, SkMatrix::I(), &ad, &effects);
    SkScopedStrike strike =
            fStrikeCache->findOrCreateScopedStrike(
                    *ad.getDesc(), effects, *dfFont.getTypefaceOrDefault());

    std::vector<GlyphAndPos> paths;

    int glyphCount = 0;
    const SkPoint* positionCursor = glyphRun.positions().data();
    for (auto glyphID : glyphRun.glyphsIDs()) {
        const SkGlyph& glyph = strike->getGlyphMetrics(glyphID, {0, 0});
        SkPoint glyphPos = origin + *positionCursor++;
        if (!glyph.isEmpty()) {
            if (glyph.fMaskFormat == SkMask::kSDF_Format) {
                if (!SkStrikeCommon::GlyphTooBigForAtlas(glyph)) {
                    // If the glyph is not empty, then it will have a pointer to SDF data.
                    fGlyphPos[glyphCount++] = {&glyph, glyphPos};
                } else {
                    if (strike->decideCouldDrawFromPath(glyph)) {
                        paths.push_back({&glyph, glyphPos});
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
    }

    if (glyphCount > 0) {
        processMasks(SkSpan<const GlyphAndPos>{fGlyphPos, SkTo<size_t>(glyphCount)},
                strike.get(), textScale, minScale, maxScale);
    }

    if (!paths.empty()) {
        processPaths(SkSpan<const GlyphAndPos>{paths}, strike.get(), textScale);
    }

    {
        // fGlyphPos will be reused here.
        if (!fARGBGlyphsIDs.empty()) {
            this->processARGBFallback(
                    maxFallbackDimension, runPaint, glyphRun.font(), viewMatrix,
                    textScale,
                    std::forward<ProcessDeviceT>(processDevice),
                    std::forward<ProcessSourceT>(processSource));
        }
    }
}
#endif

SkGlyphRunListPainter::ScopedBuffers
SkGlyphRunListPainter::ensureBuffers(const SkGlyphRunList& glyphRunList) {
    size_t size = 0;
    for (const SkGlyphRun& run : glyphRunList) {
        size = std::max(run.runSize(), size);
    }
    return ScopedBuffers(this, size);
}

SkGlyphRunListPainter::ScopedBuffers
SkGlyphRunListPainter::ensureBuffers(const SkGlyphRun& glyphRun) {
    return ScopedBuffers(this, glyphRun.runSize());
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
        GrRecordingContext* context, GrTextTarget* target, const GrClip& clip,
        const SkMatrix& viewMatrix, const SkSurfaceProps& props,
        const SkGlyphRunList& glyphRunList) {
    SkPoint origin = glyphRunList.origin();

    // Get the first paint to use as the key paint.
    const SkPaint& listPaint = glyphRunList.paint();

    SkPMColor4f filteredColor = generate_filtered_color(listPaint, target->colorSpaceInfo());
    GrColor color = generate_filtered_color(listPaint, target->colorSpaceInfo()).toBytes_RGBA();

    // If we have been abandoned, then don't draw
    if (context->priv().abandoned()) {
        return;
    }

    SkMaskFilterBase::BlurRec blurRec;
    // It might be worth caching these things, but its not clear at this time
    // TODO for animated mask filters, this will fill up our cache.  We need a safeguard here
    const SkMaskFilter* mf = listPaint.getMaskFilter();
    bool canCache = glyphRunList.canCache() && !(listPaint.getPathEffect() ||
                                                 (mf && !as_MFB(mf)->asABlur(&blurRec)));
    SkScalerContextFlags scalerContextFlags = ComputeScalerContextFlags(target->colorSpaceInfo());

    auto glyphCache = context->priv().getGlyphCache();
    GrTextBlobCache* textBlobCache = context->priv().getTextBlobCache();

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
                    glyphCache, *context->priv().caps()->shaderCaps(), fOptions,
                    listPaint, scalerContextFlags, viewMatrix, props,
                    glyphRunList, target->glyphPainter());
        } else {
            textBlobCache->makeMRU(cacheBlob.get());

            if (CACHE_SANITY_CHECK) {
                sk_sp<GrTextBlob> sanityBlob(textBlobCache->makeBlob(glyphRunList, color));
                sanityBlob->setupKey(key, blurRec, listPaint);
                cacheBlob->generateFromGlyphRunList(
                        glyphCache, *context->priv().caps()->shaderCaps(), fOptions,
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
                glyphCache, *context->priv().caps()->shaderCaps(), fOptions, listPaint,
                scalerContextFlags, viewMatrix, props, glyphRunList,
                target->glyphPainter());
    }

    cacheBlob->flush(target, props, fDistanceAdjustTable.get(), listPaint, filteredColor,
                     clip, viewMatrix, origin.x(), origin.y());
}

void GrTextBlob::SubRun::appendGlyph(GrGlyph* glyph, SkRect dstRect) {

    this->joinGlyphBounds(dstRect);

    GrTextBlob* blob = fRun->fBlob;

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

void GrTextBlob::Run::switchSubRunIfNeededAndAppendGlyph(GrGlyph* glyph,
                                                         const sk_sp<GrTextStrike>& strike,
                                                         const SkRect& destRect,
                                                         bool needsTransform) {
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
    subRun->appendGlyph(glyph, destRect);
}

void GrTextBlob::Run::appendDeviceSpaceGlyph(const sk_sp<GrTextStrike>& strike,
                                             const SkGlyph& skGlyph, SkPoint origin) {
    if (GrGlyph* glyph = strike->getGlyph(skGlyph)) {

        SkRect glyphRect = glyph->destRect(origin);

        if (!glyphRect.isEmpty()) {
            this->switchSubRunIfNeededAndAppendGlyph(glyph, strike, glyphRect, false);
        }
    }
}

void GrTextBlob::Run::appendSourceSpaceGlyph(const sk_sp<GrTextStrike>& strike,
                                             const SkGlyph& skGlyph,
                                             SkPoint origin,
                                             SkScalar textScale) {
    if (GrGlyph* glyph = strike->getGlyph(skGlyph)) {

        SkRect glyphRect = glyph->destRect(origin, textScale);

        if (!glyphRect.isEmpty()) {
            this->switchSubRunIfNeededAndAppendGlyph(glyph, strike, glyphRect, true);
        }
    }
}

void GrTextBlob::generateFromGlyphRunList(GrStrikeCache* glyphCache,
                                          const GrShaderCaps& shaderCaps,
                                          const GrTextContext::Options& options,
                                          const SkPaint& paint,
                                          SkScalerContextFlags scalerContextFlags,
                                          const SkMatrix& viewMatrix,
                                          const SkSurfaceProps& props,
                                          const SkGlyphRunList& glyphRunList,
                                          SkGlyphRunListPainter* glyphPainter) {
    SkPoint origin = glyphRunList.origin();
    const SkPaint& runPaint = glyphRunList.paint();
    this->initReusableBlob(SkPaintPriv::ComputeLuminanceColor(runPaint), viewMatrix,
                           origin.x(), origin.y());

    for (const auto& glyphRun : glyphRunList) {
        const SkFont& runFont = glyphRun.font();

        Run* run = this->pushBackRun();

        run->setRunFontAntiAlias(runFont.hasSomeAntiAliasing());

        if (GrTextContext::CanDrawAsDistanceFields(runPaint, runFont, viewMatrix, props,
                                    shaderCaps.supportsDistanceFieldText(), options)) {
            bool hasWCoord = viewMatrix.hasPerspective()
                             || options.fDistanceFieldVerticesAlwaysHaveW;

            this->setHasDistanceField();
            run->setSubRunHasDistanceFields(
                    runFont.getEdging() == SkFont::Edging::kSubpixelAntiAlias,
                    runFont.hasSomeAntiAliasing(),
                    hasWCoord);

            auto processMasks =
                    [run, glyphCache, blob{this}]
                            (SkSpan<const SkGlyphRunListPainter::GlyphAndPos> masks,
                             SkStrikeInterface* strike, SkScalar textScale, SkScalar minScale,
                             SkScalar maxScale) {
                        blob->setMinAndMaxScale(minScale, maxScale);
                        run->setupFont(strike->strikeSpec());
                        sk_sp<GrTextStrike> currStrike =
                                glyphCache->getStrike(strike->getDescriptor());
                        for (const auto& mask : masks) {
                            run->appendSourceSpaceGlyph(
                                    currStrike, *mask.glyph, mask.position, textScale);
                        }
                    };

            auto processPaths =
                [run](SkSpan<const SkGlyphRunListPainter::GlyphAndPos> paths,
                      SkStrikeInterface* strike, SkScalar textScale) {
                    run->setupFont(strike->strikeSpec());
                    for (const auto& path : paths) {
                        if (const SkPath* glyphPath = path.glyph->path()) {
                            run->appendPathGlyph(*glyphPath, path.position, textScale,
                                                 false);
                        }
                    }
                };

            auto argbFallbackDevice = [blob{this}, run, glyphCache]
                    (SkSpan<const SkGlyphRunListPainter::GlyphAndPos> masks,
                     SkStrikeInterface* strike) {
                blob->setHasBitmap();
                sk_sp<GrTextStrike> grStrike = glyphCache->getStrike(strike->getDescriptor());
                auto subRun = run->initARGBFallback();
                run->setupFont(strike->strikeSpec());
                subRun->setStrike(grStrike);
                for (const auto& mask : masks) {
                    run->appendDeviceSpaceGlyph(grStrike, *mask.glyph, mask.position);
                }
            };

            auto argbFallbackSource = [blob{this}, run, glyphCache]
                    (SkSpan<const SkGlyphRunListPainter::GlyphAndPos> masks,
                     SkStrikeInterface* strike,
                     SkScalar strikeToSourceRatio,
                     bool hasW) {
                blob->setHasBitmap();
                sk_sp<GrTextStrike> grStrike = glyphCache->getStrike(strike->getDescriptor());
                auto subRun = run->initARGBFallback();
                run->setupFont(strike->strikeSpec());
                subRun->setStrike(grStrike);
                subRun->setHasWCoord(hasW);
                for (const auto& mask : masks) {
                    run->appendSourceSpaceGlyph
                            (grStrike, *mask.glyph, mask.position, strikeToSourceRatio);
                }
            };

            glyphPainter->drawGlyphRunAsSDFWithARGBFallback(
                runPaint, glyphRun.font(),
                glyphRun, origin, viewMatrix,
                options,
                std::move(processMasks), std::move(processPaths),
                std::move(argbFallbackDevice), std::move(argbFallbackSource));

        } else if (SkGlyphRunListPainter::ShouldDrawAsPath(runPaint, runFont, viewMatrix)) {
            // The glyphs are big, so use paths to draw them.

            this->setHasBitmap();

            auto processPaths =
                [run](SkSpan<const SkGlyphRunListPainter::GlyphAndPos> paths,
                      SkStrikeInterface* strike, SkScalar textScale) {
                    run->setupFont(strike->strikeSpec());
                    for (const auto& path : paths) {
                        if (const SkPath* glyphPath = path.glyph->path()) {
                            run->appendPathGlyph(*glyphPath, path.position, textScale,
                                                 false);
                        }
                    }
                };

            auto argbFallbackDevice = [blob{this}, run, glyphCache]
                    (SkSpan<const SkGlyphRunListPainter::GlyphAndPos> masks,
                     SkStrikeInterface* strike) {
                blob->setHasBitmap();
                sk_sp<GrTextStrike> grStrike = glyphCache->getStrike(strike->getDescriptor());
                auto subRun = run->initARGBFallback();
                run->setupFont(strike->strikeSpec());
                subRun->setStrike(grStrike);
                for (const auto& mask : masks) {
                    run->appendDeviceSpaceGlyph(grStrike, *mask.glyph, mask.position);
                }
            };

            auto argbFallbackSource = [blob{this}, run, glyphCache]
                    (SkSpan<const SkGlyphRunListPainter::GlyphAndPos> masks,
                     SkStrikeInterface* strike,
                     SkScalar strikeToSourceRatio,
                     bool hasW) {
                blob->setHasBitmap();
                sk_sp<GrTextStrike> grStrike = glyphCache->getStrike(strike->getDescriptor());
                auto subRun = run->initARGBFallback();
                run->setupFont(strike->strikeSpec());
                subRun->setStrike(grStrike);
                subRun->setHasWCoord(hasW);
                for (const auto& mask : masks) {
                    run->appendSourceSpaceGlyph
                            (grStrike, *mask.glyph, mask.position, strikeToSourceRatio);
                }
            };

            glyphPainter->drawGlyphRunAsPathWithARGBFallback(
                    runPaint, runFont,
                    glyphRun, origin, viewMatrix,
                    std::move(processPaths),
                    std::move(argbFallbackDevice), std::move(argbFallbackSource));
        } else {
            // Ensure the blob is set for bitmaptext
            this->setHasBitmap();

            auto processMasks =
                [run, glyphCache]
                (SkSpan<const SkGlyphRunListPainter::GlyphAndPos> masks,
                        SkStrikeInterface* strike) {
                    run->setupFont(strike->strikeSpec());
                    sk_sp<GrTextStrike> currStrike = glyphCache->getStrike(strike->getDescriptor());
                    for (const auto& mask : masks) {
                        SkPoint pt{SkScalarFloorToScalar(mask.position.fX),
                                   SkScalarFloorToScalar(mask.position.fY)};
                        run->appendDeviceSpaceGlyph(currStrike, *mask.glyph, pt);
                    }
                };

            auto processPaths =
                [run]
                (SkSpan<const SkGlyphRunListPainter::GlyphAndPos> paths) {
                    for (const auto& path : paths) {
                        SkPoint pt{SkScalarFloorToScalar(path.position.fX),
                                   SkScalarFloorToScalar(path.position.fY)};
                        // TODO: path should always be set. Remove when proven.
                        if (const SkPath* glyphPath = path.glyph->path()) {
                            run->appendPathGlyph(*glyphPath, pt, SK_Scalar1, true);
                        }
                    }
                };

            glyphPainter->drawGlyphRunAsBMPWithPathFallback(
                    runPaint, runFont,
                    glyphRun, origin, viewMatrix,
                    std::move(processMasks), std::move(processPaths));
        }
    }
}

#if GR_TEST_UTILS

#include "GrRenderTargetContext.h"
#include "GrRecordingContextPriv.h"

std::unique_ptr<GrDrawOp> GrTextContext::createOp_TestingOnly(GrRecordingContext* context,
                                                              GrTextContext* textContext,
                                                              GrRenderTargetContext* rtc,
                                                              const SkPaint& skPaint,
                                                              const SkFont& font,
                                                              const SkMatrix& viewMatrix,
                                                              const char* text,
                                                              int x,
                                                              int y) {
    auto direct = context->priv().asDirectContext();
    if (!direct) {
        return nullptr;
    }

    auto glyphCache = direct->priv().getGlyphCache();

    static SkSurfaceProps surfaceProps(SkSurfaceProps::kLegacyFontHost_InitType);

    size_t textLen = (int)strlen(text);

    SkPMColor4f filteredColor = generate_filtered_color(skPaint, rtc->colorSpaceInfo());
    GrColor color = filteredColor.toBytes_RGBA();

    auto origin = SkPoint::Make(x, y);
    SkGlyphRunBuilder builder;
    builder.drawTextUTF8(skPaint, font, text, textLen, origin);

    auto glyphRunList = builder.useGlyphRunList();
    sk_sp<GrTextBlob> blob;
    if (!glyphRunList.empty()) {
        blob = direct->priv().getTextBlobCache()->makeBlob(glyphRunList, color);
        // Use the text and textLen below, because we don't want to mess with the paint.
        SkScalerContextFlags scalerContextFlags =
                ComputeScalerContextFlags(rtc->colorSpaceInfo());
        blob->generateFromGlyphRunList(
                glyphCache, *context->priv().caps()->shaderCaps(), textContext->fOptions,
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

    auto processMasks = [] (
            SkSpan<const SkGlyphRunListPainter::GlyphAndPos> masks, SkStrikeInterface*) { };

    auto processPaths = [] (SkSpan<const SkGlyphRunListPainter::GlyphAndPos> paths) { };

    fPainter.drawGlyphRunAsBMPWithPathFallback(
            runPaint, glyphRun.font(),
            glyphRun, origin, runMatrix,
            std::move(processMasks), std::move(processPaths));
}

SkScalar SkTextBlobCacheDiffCanvas::SetupForPath(SkPaint* paint, SkFont* font) {
    return font->setupForAsPaths(paint);
}

void SkTextBlobCacheDiffCanvas::TrackLayerDevice::processGlyphRunForPaths(
        const SkGlyphRun& glyphRun, const SkMatrix& runMatrix,
        SkPoint origin, const SkPaint& runPaint) {
    TRACE_EVENT0("skia", "SkTextBlobCacheDiffCanvas::processGlyphRunForPaths");

    // This processor is empty because all changes to the cache are tracked through
    // getGlyphMetrics and decideCouldDrawFromPath.
    auto processPaths = [](
            SkSpan<const SkGlyphRunListPainter::GlyphAndPos>, SkStrikeInterface*, SkScalar) { };

    auto argbFallbackDevice = [](
            SkSpan<const SkGlyphRunListPainter::GlyphAndPos> masks, SkStrikeInterface* strike) { };

    auto argbFallbackSource = [](
            SkSpan<const SkGlyphRunListPainter::GlyphAndPos> masks,
            SkStrikeInterface* strike,
            SkScalar strikeToSourceRatio,
            bool hasW) { };

    fPainter.drawGlyphRunAsPathWithARGBFallback(
            runPaint, glyphRun.font(),
            glyphRun, origin, runMatrix,
            std::move(processPaths), std::move(argbFallbackDevice), std::move(argbFallbackSource));
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

    auto processMasks =
            [](SkSpan<const SkGlyphRunListPainter::GlyphAndPos> masks,
               SkStrikeInterface* strike, SkScalar textScale, SkScalar min, SkScalar max) {};

    auto processPaths =
            [](SkSpan<const SkGlyphRunListPainter::GlyphAndPos> paths,
               SkStrikeInterface* strike, SkScalar textScale) {};

    auto argbFallbackDevice = [](
            SkSpan<const SkGlyphRunListPainter::GlyphAndPos> masks, SkStrikeInterface* strike) { };

    auto argbFallbackSource = [](
            SkSpan<const SkGlyphRunListPainter::GlyphAndPos> masks,
            SkStrikeInterface* strike,
            SkScalar strikeToSourceRatio,
            bool hasW) { };

    fPainter.drawGlyphRunAsSDFWithARGBFallback(
            runPaint, glyphRun.font(),
            glyphRun, origin, runMatrix, options,
            std::move(processMasks), std::move(processPaths),
            std::move(argbFallbackDevice), std::move(argbFallbackSource));

    return true;
}
#endif

SkGlyphRunListPainter::ScopedBuffers::ScopedBuffers(SkGlyphRunListPainter* painter, int size)
        : fPainter{painter} {
    SkASSERT(size >= 0);
    if (fPainter->fMaxRunSize < size) {
        fPainter->fMaxRunSize = size;

        fPainter->fPositions.reset(size);
        fPainter->fGlyphPos.reset(size);
    }
}

SkGlyphRunListPainter::ScopedBuffers::~ScopedBuffers() {
    fPainter->fPaths.clear();
    fPainter->fARGBGlyphsIDs.clear();
    fPainter->fARGBPositions.clear();

    if (fPainter->fMaxRunSize > 200) {
        fPainter->fMaxRunSize = 0;
        fPainter->fPositions.reset();
        fPainter->fGlyphPos.reset();
        fPainter->fPaths.shrink_to_fit();
        fPainter->fARGBGlyphsIDs.shrink_to_fit();
        fPainter->fARGBPositions.shrink_to_fit();
    }
}
