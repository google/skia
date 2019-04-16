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

// -- SkGlyphRunListPainter ------------------------------------------------------------------------
SkGlyphRunListPainter::SkGlyphRunListPainter(const SkSurfaceProps& props,
                                             SkColorType colorType,
                                             SkScalerContextFlags flags,
                                             SkStrikeCacheInterface* strikeCache)
        : fDeviceProps{props}
        ,  fBitmapFallbackProps{SkSurfaceProps{props.flags(), kUnknown_SkPixelGeometry}}
        ,  fColorType{colorType}, fScalerContextFlags{flags}
        ,  fStrikeCache{strikeCache} {}

// TODO: unify with code in GrTextContext.cpp
static SkScalerContextFlags compute_scaler_context_flags(const SkColorSpace* cs) {
    // If we're doing linear blending, then we can disable the gamma hacks.
    // Otherwise, leave them on. In either case, we still want the contrast boost:
    // TODO: Can we be even smarter about mask gamma based on the dest transfer function?
    if (cs && cs->gammaIsLinear()) {
        return SkScalerContextFlags::kBoostContrast;
    } else {
        return SkScalerContextFlags::kFakeGammaAndBoostContrast;
    }
}

SkGlyphRunListPainter::SkGlyphRunListPainter(const SkSurfaceProps& props,
                                             SkColorType colorType,
                                             SkColorSpace* cs,
                                             SkStrikeCacheInterface* strikeCache)
        : SkGlyphRunListPainter(props, colorType, compute_scaler_context_flags(cs), strikeCache) {}

#if SK_SUPPORT_GPU
SkGlyphRunListPainter::SkGlyphRunListPainter(const SkSurfaceProps& props,
                                             const GrColorSpaceInfo& csi)
        : SkGlyphRunListPainter(props,
                                kUnknown_SkColorType,
                                compute_scaler_context_flags(csi.colorSpace()),
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

            // The sub-pixel position will always happen when transforming to the screen.
            pathFont.setSubpixel(false);

            SkAutoDescriptor ad;
            SkScalerContextEffects effects;
            SkScalerContext::CreateDescriptorAndEffectsUsingPaint(pathFont,
                                                                  pathPaint,
                                                                  props,
                                                                  fScalerContextFlags,
                                                                  SkMatrix::I(),
                                                                  &ad,
                                                                  &effects);

            SkScopedStrike strike =
                    fStrikeCache->findOrCreateScopedStrike(
                            *ad.getDesc(), effects,*pathFont.getTypefaceOrDefault());

            auto glyphPosSpan = strike->prepareForDrawing(
                    glyphRun.glyphsIDs().data(), fPositions, glyphRun.runSize(), 0, fGlyphPos);

            SkTDArray<SkPathPos> pathsAndPositions;
            pathsAndPositions.setReserve(glyphPosSpan.size());
            for (const SkGlyphPos& glyphPos : glyphPosSpan) {
                const SkGlyph& glyph = *glyphPos.glyph;
                SkPoint position = glyphPos.position;
                if (check_glyph_position(position)
                    && !glyph.isEmpty()
                    && glyph.fMaskFormat != SkMask::kARGB32_Format
                    && glyph.hasPath())
                {
                    // Only draw a path if it exists, and this is not a color glyph.
                    pathsAndPositions.push_back(SkPathPos{glyph.path(), position});
                } else {
                    // TODO: this is here to have chrome layout tests pass. Remove this when
                    //  fallback for CPU works.
                    strike->generatePath(glyph);
                    if (check_glyph_position(position) && !glyph.isEmpty() && glyph.hasPath()) {
                        pathsAndPositions.push_back(SkPathPos{glyph.path(), position});
                    }
                }
            }

            // The paint we draw paths with must have the same anti-aliasing state as the runFont
            // allowing the paths to have the same edging as the glyph masks.
            pathPaint = runPaint;
            pathPaint.setAntiAlias(runFont.hasSomeAntiAliasing());

            bitmapDevice->paintPaths(
                    SkSpan<const SkPathPos>{pathsAndPositions.begin(), pathsAndPositions.size()},
                    textScale, pathPaint);
        } else {
            SkAutoDescriptor ad;
            SkScalerContextEffects effects;

            SkScalerContext::CreateDescriptorAndEffectsUsingPaint(
                    runFont, runPaint, props, fScalerContextFlags, deviceMatrix, &ad,
                    &effects);

            SkTypeface* typeface = runFont.getTypefaceOrDefault();
            SkScopedStrike strike =
                    fStrikeCache->findOrCreateScopedStrike(*ad.getDesc(), effects, *typeface);

            // Add rounding and origin.
            SkMatrix matrix = deviceMatrix;
            matrix.preTranslate(origin.x(), origin.y());
            SkPoint rounding = strike->rounding();
            matrix.postTranslate(rounding.x(), rounding.y());
            matrix.mapPoints(fPositions, glyphRun.positions().data(), runSize);

            SkSpan<const SkGlyphPos> glyphPosSpan = strike->prepareForDrawing(
                    glyphRun.glyphsIDs().data(), fPositions, glyphRun.runSize(),
                    std::numeric_limits<int>::max(), fGlyphPos);

            SkTDArray<SkMask> masks;
            masks.setReserve(glyphPosSpan.size());

            for (const SkGlyphPos& glyphPos : glyphPosSpan) {
                const SkGlyph& glyph = *glyphPos.glyph;
                SkPoint position = glyphPos.position;
                // The glyph could have dimensions (!isEmpty()), but still may have no bits if
                // the width is too wide. So check that there really is an image.
                if (check_glyph_position(position) && !glyph.isEmpty() && glyph.hasImage()) {
                    masks.push_back(glyph.mask(position));
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
void SkGlyphRunListPainter::processARGBFallback(SkScalar maxSourceGlyphDimension,
                                                const SkPaint& runPaint,
                                                const SkFont& runFont,
                                                const SkMatrix& viewMatrix,
                                                SkGlyphRunPainterInterface* process) {
    SkASSERT(!fARGBGlyphsIDs.empty());

    SkScalar maxScale = viewMatrix.getMaxScale();

    // This is a linear estimate of the longest dimension among all the glyph widths and heights.
    SkScalar conservativeMaxGlyphDimension = maxSourceGlyphDimension * maxScale;

    // If the situation that the matrix is simple, and all the glyphs are small enough. Go fast!
    // N.B. If the matrix has scale, that will be reflected in the strike through the viewMatrix
    // in the useFastPath case.
    bool useDeviceCache =
            viewMatrix.isScaleTranslate()
            && conservativeMaxGlyphDimension <= SkStrikeCommon::kSkSideTooBigForAtlas;

    // A scaled and translated transform is the common case, and is handled directly in fallback.
    // Even if the transform is scale and translate, fallback must be careful to use glyphs that
    // fit in the atlas. If a glyph will not fit in the atlas, then the general transform case is
    // used to render the glyphs.
    if (useDeviceCache) {
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

        SkSpan<const SkGlyphPos> glyphPosSpan = strike->prepareForDrawing(
                fARGBGlyphsIDs.data(),
                fARGBPositions.data(),
                fARGBGlyphsIDs.size(),
                SkStrikeCommon::kSkSideTooBigForAtlas,
                fGlyphPos);

        if (process) {
            process->processDeviceFallback(glyphPosSpan, strike.get());
        }

    } else {
        // If the matrix is complicated or if scaling is used to fit the glyphs in the cache,
        // then this case is used.

        // Subtract 2 to account for the bilerp pad around the glyph
        SkScalar maxAtlasDimension = SkStrikeCommon::kSkSideTooBigForAtlas - 2;

        SkScalar runFontTextSize = runFont.getSize();

        // Scale the text size down so the long side of all the glyphs will fit in the atlas.
        SkScalar fallbackTextSize = SkScalarFloorToScalar(
                (maxAtlasDimension / maxSourceGlyphDimension) * runFontTextSize);

        SkFont fallbackFont{runFont};
        fallbackFont.setSize(fallbackTextSize);

        // No sub-pixel needed. The transform to the screen will take care of sub-pixel positioning.
        fallbackFont.setSubpixel(false);

        // The scale factor to go from strike size to the source size for glyphs.
        SkScalar fallbackTextScale = runFontTextSize / fallbackTextSize;

        SkAutoDescriptor ad;
        SkScalerContextEffects effects;
        SkScalerContext::CreateDescriptorAndEffectsUsingPaint(fallbackFont,
                                                              runPaint,
                                                              fDeviceProps,
                                                              fScalerContextFlags,
                                                              SkMatrix::I(),
                                                              &ad,
                                                              &effects);

        SkScopedStrike strike =
                fStrikeCache->findOrCreateScopedStrike(
                        *ad.getDesc(), effects, *fallbackFont.getTypefaceOrDefault());

        auto glyphPosSpan = strike->prepareForDrawing(fARGBGlyphsIDs.data(),
                                                      fARGBPositions.data(),
                                                      fARGBGlyphsIDs.size(),
                                                      SkStrikeCommon::kSkSideTooBigForAtlas,
                                                      fGlyphPos);

        if (process) {
            process->processSourceFallback(
                    glyphPosSpan,
                    strike.get(),
                    fallbackTextScale,
                    viewMatrix.hasPerspective());
        }
    }
}

#if SK_SUPPORT_GPU
void SkGlyphRunListPainter::processGlyphRunList(const SkGlyphRunList& glyphRunList,
                                                const SkMatrix& viewMatrix,
                                                const SkSurfaceProps& props,
                                                bool contextSupportsDistanceFieldText,
                                                const GrTextContext::Options& options,
                                                SkGlyphRunPainterInterface* process) {

    SkPoint origin = glyphRunList.origin();
    const SkPaint& runPaint = glyphRunList.paint();

    for (const auto& glyphRun : glyphRunList) {
        SkScalar maxFallbackDimension{-SK_ScalarInfinity};
        ScopedBuffers _ = this->ensureBuffers(glyphRun);

        auto addFallback = [this, &maxFallbackDimension]
                (const SkGlyph& glyph, SkPoint sourcePosition) {
            maxFallbackDimension = std::max(maxFallbackDimension,
                                            SkIntToScalar(glyph.maxDimension()));
            fARGBGlyphsIDs.push_back(glyph.getGlyphID());
            fARGBPositions.push_back(sourcePosition);
        };

        const SkFont& runFont = glyphRun.font();

        bool useSDFT = GrTextContext::CanDrawAsDistanceFields(
                runPaint, runFont, viewMatrix, props, contextSupportsDistanceFieldText, options);
        if (process) {
            process->startRun(glyphRun, useSDFT);
        }

        if (useSDFT) {
            // Translate all glyphs to the origin.
            SkMatrix translate = SkMatrix::MakeTrans(origin.x(), origin.y());
            translate.mapPoints(fPositions, glyphRun.positions().data(), glyphRun.runSize());

            // Setup distance field runPaint and text ratio
            SkPaint dfPaint = GrTextContext::InitDistanceFieldPaint(runPaint);
            SkScalar cacheToSourceScale;
            SkFont dfFont = GrTextContext::InitDistanceFieldFont(
                    runFont, viewMatrix, options, &cacheToSourceScale);

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

            SkSpan<const SkGlyphPos> glyphPosSpan = strike->prepareForDrawing(
                    glyphRun.glyphsIDs().data(), fPositions, glyphRun.runSize(),
                    SkStrikeCommon::kSkSideTooBigForAtlas, fGlyphPos);

            size_t glyphsWithMaskCount = 0;
            for (const SkGlyphPos& glyphPos : glyphPosSpan) {
                const SkGlyph& glyph = *glyphPos.glyph;
                SkPoint position = glyphPos.position;
                if (glyph.isEmpty()) {
                    // do nothing
                } else if (glyph.fMaskFormat == SkMask::kSDF_Format
                           && glyph.maxDimension() <= SkStrikeCommon::kSkSideTooBigForAtlas) {
                    // SDF mask will work.
                    fGlyphPos[glyphsWithMaskCount++] = glyphPos;
                } else if (glyph.fMaskFormat != SkMask::kARGB32_Format && glyph.hasPath()) {
                    // If not color but too big, use a path.
                    fPaths.push_back(glyphPos);
                } else {
                    // If no path, or it is color, then fallback.
                    addFallback(glyph, position);
                }
            }

            if (process) {
                bool hasWCoord =
                        viewMatrix.hasPerspective() || options.fDistanceFieldVerticesAlwaysHaveW;

                // processSourceSDFT must be called even if there are no glyphs to make sure runs
                // are set correctly.
                process->processSourceSDFT(
                        SkSpan<const SkGlyphPos>{fGlyphPos, glyphsWithMaskCount},
                        strike.get(),
                        runFont,
                        cacheToSourceScale,
                        minScale,
                        maxScale,
                        hasWCoord);

                if (!fPaths.empty()) {
                    process->processSourcePaths(
                            SkSpan<const SkGlyphPos>{fPaths}, strike.get(), cacheToSourceScale);
                }
            }

            // fGlyphPos will be reused here.
            if (!fARGBGlyphsIDs.empty()) {
                this->processARGBFallback(maxFallbackDimension * cacheToSourceScale,
                                          runPaint, runFont, viewMatrix, process);
            }
        } else if (SkGlyphRunListPainter::ShouldDrawAsPath(runPaint, runFont, viewMatrix)) {

            // Translate all glyphs to the origin.
            SkMatrix translate = SkMatrix::MakeTrans(origin.x(), origin.y());
            translate.mapPoints(fPositions, glyphRun.positions().data(), glyphRun.runSize());

            // setup our std runPaint, in hopes of getting hits in the cache
            SkPaint pathPaint{runPaint};
            SkFont pathFont{runFont};

            // The factor to get from the size stored in the strike to the size needed for
            // the source.
            SkScalar strikeToSourceRatio = pathFont.setupForAsPaths(&pathPaint);

            // The sub-pixel position will always happen when transforming to the screen.
            pathFont.setSubpixel(false);

            SkAutoDescriptor ad;
            SkScalerContextEffects effects;
            SkScalerContext::CreateDescriptorAndEffectsUsingPaint(pathFont,
                                                                  pathPaint,
                                                                  fDeviceProps,
                                                                  fScalerContextFlags,
                                                                  SkMatrix::I(),
                                                                  &ad,
                                                                  &effects);

            SkScopedStrike strike =
                    fStrikeCache->findOrCreateScopedStrike(
                            *ad.getDesc(), effects,*pathFont.getTypefaceOrDefault());

            SkSpan<const SkGlyphPos> glyphPosSpan = strike->prepareForDrawing(
                    glyphRun.glyphsIDs().data(), fPositions, glyphRun.runSize(), 0, fGlyphPos);

            // As opposed to SDF and mask, path handling puts paths in fGlyphPos instead of fPaths.
            size_t glyphsWithPathCount = 0;
            for (const SkGlyphPos& glyphPos : glyphPosSpan) {
                const SkGlyph& glyph = *glyphPos.glyph;
                SkPoint position = glyphPos.position;
                if (glyph.isEmpty()) {
                    // do nothing
                } else if (glyph.fMaskFormat != SkMask::kARGB32_Format && glyph.hasPath()) {
                    // Place paths in fGlyphPos
                    fGlyphPos[glyphsWithPathCount++] = glyphPos;
                } else {
                    addFallback(glyph, position);
                }
            }

            if (process) {
                // processSourcePaths must be called even if there are no glyphs to make sure runs
                // are set correctly.
                process->processSourcePaths(
                        SkSpan<const SkGlyphPos>{fGlyphPos, glyphsWithPathCount},
                        strike.get(),
                        strikeToSourceRatio);
            }

            // fGlyphPos will be reused here.
            if (!fARGBGlyphsIDs.empty()) {
                this->processARGBFallback(maxFallbackDimension * strikeToSourceRatio,
                                          runPaint, runFont, viewMatrix, process);
            }
        } else {

            SkAutoDescriptor ad;
            SkScalerContextEffects effects;

            SkScalerContext::CreateDescriptorAndEffectsUsingPaint(
                    runFont, runPaint, fDeviceProps, fScalerContextFlags, viewMatrix, &ad,
                    &effects);

            SkTypeface* typeface = runFont.getTypefaceOrDefault();
            SkScopedStrike strike =
                    fStrikeCache->findOrCreateScopedStrike(*ad.getDesc(), effects, *typeface);

            SkMatrix mapping = viewMatrix;
            mapping.preTranslate(origin.x(), origin.y());
            SkVector rounding = strike->rounding();
            mapping.postTranslate(rounding.x(), rounding.y());
            mapping.mapPoints(fPositions, glyphRun.positions().data(), glyphRun.runSize());

            // Lookup all the glyphs from the cache.
            SkSpan<const SkGlyphPos> glyphPosSpan = strike->prepareForDrawing(
                    glyphRun.glyphsIDs().data(),
                    fPositions,
                    glyphRun.runSize(),
                    SkStrikeCommon::kSkSideTooBigForAtlas,
                    fGlyphPos);

            // Sort glyphs into the three bins: mask (fGlyphPos), path (fPaths), and fallback.
            size_t glyphsWithMaskCount = 0;
            for (const SkGlyphPos& glyphPos : glyphPosSpan) {
                const SkGlyph& glyph = *glyphPos.glyph;
                const SkPoint position = glyphPos.position;

                // Able to position glyph?
                if (!SkScalarsAreFinite(position.x(), position.y())) {
                    continue;
                }

                if (glyph.maxDimension() <= SkStrikeCommon::kSkSideTooBigForAtlas) {
                    fGlyphPos[glyphsWithMaskCount++] = glyphPos;
                } else if (glyph.fMaskFormat != SkMask::kARGB32_Format && glyph.hasPath()) {
                    fPaths.push_back(glyphPos);
                } else {
                    addFallback(glyph, origin + glyphRun.positions()[glyphPos.index]);
                }
            }

            if (process) {
                // processDeviceMasks must be called even if there are no glyphs to make sure runs
                // are set correctly.
                process->processDeviceMasks(
                        SkSpan<const SkGlyphPos>{fGlyphPos, glyphsWithMaskCount}, strike.get());
                if (!fPaths.empty()) {
                    process->processDevicePaths(SkSpan<const SkGlyphPos>{fPaths});
                }
            }

            // fGlyphPos will be reused here.
            if (!fARGBGlyphsIDs.empty()) {
                this->processARGBFallback(maxFallbackDimension / viewMatrix.getMaxScale(),
                                          runPaint, runFont, viewMatrix, process);
            }
        }  // Mask case
    }  // For all glyph runs
}
#endif  // SK_SUPPORT_GPU

auto SkGlyphRunListPainter::ensureBuffers(const SkGlyphRunList& glyphRunList) -> ScopedBuffers {
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

    auto grStrikeCache = context->priv().getGrStrikeCache();
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
            cacheBlob = textBlobCache->makeCachedBlob(
                    glyphRunList, key, blurRec, listPaint, color, grStrikeCache);
            cacheBlob->generateFromGlyphRunList(
                    *context->priv().caps()->shaderCaps(), fOptions,
                    listPaint, scalerContextFlags, viewMatrix, props,
                    glyphRunList, target->glyphPainter());
        } else {
            textBlobCache->makeMRU(cacheBlob.get());

            if (CACHE_SANITY_CHECK) {
                sk_sp<GrTextBlob> sanityBlob(textBlobCache->makeBlob(
                        glyphRunList, color, grStrikeCache));
                sanityBlob->setupKey(key, blurRec, listPaint);
                cacheBlob->generateFromGlyphRunList(
                        *context->priv().caps()->shaderCaps(), fOptions,
                        listPaint, scalerContextFlags, viewMatrix, props, glyphRunList,
                        target->glyphPainter());
                GrTextBlob::AssertEqual(*sanityBlob, *cacheBlob);
            }
        }
    } else {
        if (canCache) {
            cacheBlob = textBlobCache->makeCachedBlob(
                    glyphRunList, key, blurRec, listPaint, color, grStrikeCache);
        } else {
            cacheBlob = textBlobCache->makeBlob(glyphRunList, color, grStrikeCache);
        }
        cacheBlob->generateFromGlyphRunList(
                *context->priv().caps()->shaderCaps(), fOptions, listPaint,
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

void GrTextBlob::generateFromGlyphRunList(const GrShaderCaps& shaderCaps,
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

    glyphPainter->processGlyphRunList(glyphRunList,
                                      viewMatrix,
                                      props,
                                      shaderCaps.supportsDistanceFieldText(),
                                      options,
                                      this);
}

GrTextBlob::Run* GrTextBlob::currentRun() {
    return &fRuns[fRunCount - 1];
}

void GrTextBlob::startRun(const SkGlyphRun& glyphRun, bool useSDFT) {
    if (useSDFT) {
        this->setHasDistanceField();
    }
    Run* run = this->pushBackRun();
    run->setRunFontAntiAlias(glyphRun.font().hasSomeAntiAliasing());
}

void GrTextBlob::processDeviceMasks(SkSpan<const SkGlyphPos> masks,
                                    SkStrikeInterface* strike) {
    Run* run = this->currentRun();
    this->setHasBitmap();
    run->setupFont(strike->strikeSpec());
    sk_sp<GrTextStrike> currStrike = fStrikeCache->getStrike(strike->getDescriptor());
    for (const auto& mask : masks) {
        SkPoint pt{SkScalarFloorToScalar(mask.position.fX),
                   SkScalarFloorToScalar(mask.position.fY)};
        run->appendDeviceSpaceGlyph(currStrike, *mask.glyph, pt);
    }
}

void GrTextBlob::processSourcePaths(SkSpan<const SkGlyphPos> paths,
                                    SkStrikeInterface* strike, SkScalar cacheToSourceScale) {
    Run* run = this->currentRun();
    this->setHasBitmap();
    run->setupFont(strike->strikeSpec());
    for (const auto& path : paths) {
        if (const SkPath* glyphPath = path.glyph->path()) {
            run->appendPathGlyph(*glyphPath, path.position, cacheToSourceScale,
                                 false);
        }
    }
}

void GrTextBlob::processDevicePaths(SkSpan<const SkGlyphPos> paths) {
    Run* run = this->currentRun();
    this->setHasBitmap();
    for (const auto& path : paths) {
        SkPoint pt{SkScalarFloorToScalar(path.position.fX),
                   SkScalarFloorToScalar(path.position.fY)};
        // TODO: path should always be set. Remove when proven.
        if (const SkPath* glyphPath = path.glyph->path()) {
            run->appendPathGlyph(*glyphPath, pt, SK_Scalar1, true);
        }
    }
}

void GrTextBlob::processSourceSDFT(SkSpan<const SkGlyphPos> masks,
                                   SkStrikeInterface* strike,
                                   const SkFont& runFont,
                                   SkScalar cacheToSourceScale,
                                   SkScalar minScale,
                                   SkScalar maxScale,
                                   bool hasWCoord) {

    Run* run = this->currentRun();
    run->setSubRunHasDistanceFields(
            runFont.getEdging() == SkFont::Edging::kSubpixelAntiAlias,
            runFont.hasSomeAntiAliasing(),
            hasWCoord);
    this->setMinAndMaxScale(minScale, maxScale);
    run->setupFont(strike->strikeSpec());
    sk_sp<GrTextStrike> currStrike = fStrikeCache->getStrike(strike->getDescriptor());
    for (const auto& mask : masks) {
        run->appendSourceSpaceGlyph(
                currStrike, *mask.glyph, mask.position, cacheToSourceScale);
    }
}

void GrTextBlob::processSourceFallback(SkSpan<const SkGlyphPos> masks,
                                       SkStrikeInterface* strike, SkScalar cacheToSourceScale,
                                       bool hasW) {
    Run* run = this->currentRun();

    auto subRun = run->initARGBFallback();
    sk_sp<GrTextStrike> grStrike =
            fStrikeCache->getStrike(strike->getDescriptor());
    subRun->setStrike(grStrike);
    subRun->setHasWCoord(hasW);

    this->setHasBitmap();
    run->setupFont(strike->strikeSpec());
    for (const auto& mask : masks) {
        run->appendSourceSpaceGlyph
                (grStrike, *mask.glyph, mask.position, cacheToSourceScale);
    }
}

void GrTextBlob::processDeviceFallback(SkSpan<const SkGlyphPos> masks,
                                       SkStrikeInterface* strike) {
    Run* run = this->currentRun();
    this->setHasBitmap();
    sk_sp<GrTextStrike> grStrike = fStrikeCache->getStrike(strike->getDescriptor());
    auto subRun = run->initARGBFallback();
    run->setupFont(strike->strikeSpec());
    subRun->setStrike(grStrike);
    for (const auto& mask : masks) {
        run->appendDeviceSpaceGlyph(grStrike, *mask.glyph, mask.position);
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

    auto strikeCache = direct->priv().getGrStrikeCache();

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
        blob = direct->priv().getTextBlobCache()->makeBlob(glyphRunList, color, strikeCache);
        // Use the text and textLen below, because we don't want to mess with the paint.
        SkScalerContextFlags scalerContextFlags =
                ComputeScalerContextFlags(rtc->colorSpaceInfo());
        blob->generateFromGlyphRunList(
                *context->priv().caps()->shaderCaps(), textContext->fOptions,
                skPaint, scalerContextFlags, viewMatrix, surfaceProps,
                glyphRunList, rtc->textTarget()->glyphPainter());
    }

    return blob->test_makeOp(textLen, 0, 0, viewMatrix, x, y, skPaint, filteredColor, surfaceProps,
                             textContext->dfAdjustTable(), rtc->textTarget());
}

#endif  // GR_TEST_UTILS
#endif  // SK_SUPPORT_GPU

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
