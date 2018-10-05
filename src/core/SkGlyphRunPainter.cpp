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
#include "SkGlyphCache.h"
#include "SkMaskFilter.h"
#include "SkPaintPriv.h"
#include "SkPathEffect.h"
#include "SkRasterClip.h"
#include "SkStrikeCache.h"

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

bool SkGlyphRunListPainter::ShouldDrawAsPath(const SkPaint& paint, const SkMatrix& matrix) {
    // hairline glyphs are fast enough so we don't need to cache them
    if (SkPaint::kStroke_Style == paint.getStyle() && 0 == paint.getStrokeWidth()) {
        return true;
    }

    // we don't cache perspective
    if (matrix.hasPerspective()) {
        return true;
    }

    SkMatrix textM;
    SkPaintPriv::MakeTextMatrix(&textM, paint);
    return SkPaint::TooBigToUseCache(matrix, textM, 1024);
}

bool SkGlyphRunListPainter::ensureBitmapBuffers(size_t runSize) {
    if (runSize > fMaxRunSize) {
        fPositions.reset(runSize);
        fMaxRunSize = runSize;
    }

    return true;
}

void SkGlyphRunListPainter::drawUsingPaths(
        const SkGlyphRun& glyphRun, SkPoint origin, SkGlyphCache* cache, PerPath perPath) const {

    auto eachGlyph =
            [perPath{std::move(perPath)}, origin, &cache]
                    (SkGlyphID glyphID, SkPoint position) {
                const SkGlyph& glyph = cache->getGlyphIDMetrics(glyphID);
                if (glyph.fWidth > 0) {
                    const SkPath* path = cache->findPath(glyph);
                    SkPoint loc = position + origin;
                    perPath(path, glyph, loc);
                }
            };

    glyphRun.forEachGlyphAndPosition(eachGlyph);
}

static bool prepare_mask(
        SkGlyphCache* cache, const SkGlyph& glyph, SkPoint position, SkMask* mask) {
    if (glyph.fWidth == 0) { return false; }

    // Prevent glyphs from being drawn outside of or straddling the edge of device space.
    // Comparisons written a little weirdly so that NaN coordinates are treated safely.
    auto gt = [](float a, int b) { return !(a <= (float)b); };
    auto lt = [](float a, int b) { return !(a >= (float)b); };
    if (gt(position.fX, INT_MAX - (INT16_MAX + SkTo<int>(UINT16_MAX))) ||
        lt(position.fX, INT_MIN - (INT16_MIN + 0 /*UINT16_MIN*/)) ||
        gt(position.fY, INT_MAX - (INT16_MAX + SkTo<int>(UINT16_MAX))) ||
        lt(position.fY, INT_MIN - (INT16_MIN + 0 /*UINT16_MIN*/))) {
        return false;
    }

    int left = SkScalarFloorToInt(position.fX);
    int top  = SkScalarFloorToInt(position.fY);

    left += glyph.fLeft;
    top  += glyph.fTop;

    int right   = left + glyph.fWidth;
    int bottom  = top  + glyph.fHeight;

    mask->fBounds.set(left, top, right, bottom);
    SkASSERT(!mask->fBounds.isEmpty());

    uint8_t* bits = (uint8_t*)(cache->findImage(glyph));
    if (nullptr == bits) {
        return false;  // can't rasterize glyph
    }

    mask->fImage    = bits;
    mask->fRowBytes = glyph.rowBytes();
    mask->fFormat   = static_cast<SkMask::Format>(glyph.fMaskFormat);

    return true;
}

void SkGlyphRunListPainter::drawGlyphRunAsSubpixelMask(
        SkGlyphCache* cache, const SkGlyphRun& glyphRun,
        SkPoint origin, const SkMatrix& deviceMatrix,
        PerMask perMask) {
    auto runSize = glyphRun.runSize();
    if (this->ensureBitmapBuffers(runSize)) {
        // Add rounding and origin.
        SkMatrix matrix = deviceMatrix;
        SkPoint rounding = cache->rounding();
        matrix.preTranslate(origin.x(), origin.y());
        matrix.postTranslate(rounding.x(), rounding.y());
        matrix.mapPoints(fPositions, glyphRun.positions().data(), runSize);

        const SkPoint* positionCursor = fPositions;
        for (auto glyphID : glyphRun.shuntGlyphsIDs()) {
            auto position = *positionCursor++;
            if (SkScalarsAreFinite(position.fX, position.fY)) {
                const SkGlyph& glyph = cache->getGlyphMetrics(glyphID, position);

                SkMask mask;
                if (prepare_mask(cache, glyph, position, &mask)) {
                    perMask(mask, glyph, position);
                }
            }
        }
    }
}

void SkGlyphRunListPainter::drawGlyphRunAsFullpixelMask(
        SkGlyphCache* cache, const SkGlyphRun& glyphRun,
        SkPoint origin, const SkMatrix& deviceMatrix,
        PerMask perMask) {
    auto runSize = glyphRun.runSize();
    if (this->ensureBitmapBuffers(runSize)) {

        // Add rounding and origin.
        SkMatrix matrix = deviceMatrix;
        matrix.preTranslate(origin.x(), origin.y());
        matrix.postTranslate(SK_ScalarHalf, SK_ScalarHalf);
        matrix.mapPoints(fPositions, glyphRun.positions().data(), runSize);

        const SkPoint* positionCursor = fPositions;
        for (auto glyphID : glyphRun.shuntGlyphsIDs()) {
            auto position = *positionCursor++;
            if (SkScalarsAreFinite(position.fX, position.fY)) {
                const SkGlyph& glyph = cache->getGlyphIDMetrics(glyphID);
                SkMask mask;
                if (prepare_mask(cache, glyph, position, &mask)) {
                    perMask(mask, glyph, position);
                }
            }
        }
    }
}

void SkGlyphRunListPainter::drawForBitmapDevice(
        const SkGlyphRunList& glyphRunList, const SkMatrix& deviceMatrix,
        PerMaskCreator perMaskCreator, PerPathCreator perPathCreator) {

    SkPoint origin = glyphRunList.origin();
    for (auto& glyphRun : glyphRunList) {
        SkSTArenaAlloc<3332> alloc;
        // The bitmap blitters can only draw lcd text to a N32 bitmap in srcOver. Otherwise,
        // convert the lcd text into A8 text. The props communicates this to the scaler.
        auto& props = (kN32_SkColorType == fColorType && glyphRun.paint().isSrcOver())
                      ? fDeviceProps
                      : fBitmapFallbackProps;
        auto paint = glyphRun.paint();
        if (ShouldDrawAsPath(glyphRun.paint(), deviceMatrix)) {

            // setup our std pathPaint, in hopes of getting hits in the cache
            SkPaint pathPaint(glyphRun.paint());
            SkScalar matrixScale = pathPaint.setupForAsPaths();

            auto pathCache = SkStrikeCache::FindOrCreateStrikeExclusive(
                    pathPaint, &props, fScalerContextFlags, nullptr);

            auto perPath = perPathCreator(paint, matrixScale, &alloc);
            this->drawUsingPaths(glyphRun, origin, pathCache.get(), perPath);
        } else {
            auto cache = SkStrikeCache::FindOrCreateStrikeExclusive(
                    paint, &props, fScalerContextFlags, &deviceMatrix);
            auto perMask = perMaskCreator(paint, &alloc);
            this->drawUsingMasks(cache.get(), glyphRun, origin, deviceMatrix, perMask);
        }
    }
}

void SkGlyphRunListPainter::drawUsingMasks(
        SkGlyphCache* cache, const SkGlyphRun& glyphRun,
        SkPoint origin, const SkMatrix& deviceMatrix, PerMask perMask) {
    if (cache->isSubpixel()) {
        this->drawGlyphRunAsSubpixelMask(cache, glyphRun, origin, deviceMatrix, perMask);
    } else {
        this->drawGlyphRunAsFullpixelMask(cache, glyphRun, origin, deviceMatrix, perMask);
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
        SkScalar maxGlyphDimension, const SkPaint& runPaint, SkPoint origin,
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
        argbFallback(runPaint, glyphIDs, positions, SK_Scalar1, viewMatrix, kTransformDone);

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

        SkPaint fallbackPaint{runPaint};
        fallbackPaint.setTextSize(fallbackTextSize);
        SkScalar fallbackTextScale = runPaintTextSize / fallbackTextSize;
        auto positions = SkSpan<const SkPoint>{fARGBPositions};
        argbFallback(
                fallbackPaint, glyphIDs, positions, fallbackTextScale, SkMatrix::I(), kDoTransform);
    }
}

#if SK_SUPPORT_GPU
// -- GrTextContext --------------------------------------------------------------------------------
GrColor generate_filtered_color(const SkPaint& paint, const GrColorSpaceInfo& colorSpaceInfo) {
    GrColor4f filteredColor = SkColor4fToUnpremulGrColor4f(paint.getColor4f(), colorSpaceInfo);
    if (paint.getColorFilter() != nullptr) {
        filteredColor = GrColor4f::FromRGBA4f(paint.getColorFilter()->filterColor4f(
                filteredColor.asRGBA4f<kUnpremul_SkAlphaType>(),colorSpaceInfo.colorSpace()));
    }
    return filteredColor.premul().toGrColor();
}

void GrTextContext::drawGlyphRunList(
        GrContext* context, GrTextTarget* target, const GrClip& clip,
        const SkMatrix& viewMatrix, const SkSurfaceProps& props,
        const SkGlyphRunList& glyphRunList) {
    SkPoint origin = glyphRunList.origin();

    // Get the first paint to use as the key paint.
    const SkPaint& listPaint = glyphRunList.paint();
    GrColor filteredColor = generate_filtered_color(listPaint, target->colorSpaceInfo());

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
        if (cacheBlob->mustRegenerate(listPaint, blurRec, viewMatrix, origin.x(), origin.y())) {
            // We have to remake the blob because changes may invalidate our masks.
            // TODO we could probably get away reuse most of the time if the pointer is unique,
            // but we'd have to clear the subrun information
            textBlobCache->remove(cacheBlob.get());
            cacheBlob = textBlobCache->makeCachedBlob(glyphRunList, key, blurRec, listPaint);
            this->regenerateGlyphRunList(cacheBlob.get(), glyphCache,
                                         *context->contextPriv().caps()->shaderCaps(), listPaint,
                                         filteredColor, scalerContextFlags, viewMatrix, props,
                                         glyphRunList, target->glyphPainter());
        } else {
            textBlobCache->makeMRU(cacheBlob.get());

            if (CACHE_SANITY_CHECK) {
                int glyphCount = glyphRunList.totalGlyphCount();
                int runCount = glyphRunList.runCount();
                sk_sp<GrTextBlob> sanityBlob(textBlobCache->makeBlob(glyphCount, runCount));
                sanityBlob->setupKey(key, blurRec, listPaint);
                this->regenerateGlyphRunList(
                        sanityBlob.get(), glyphCache, *context->contextPriv().caps()->shaderCaps(),
                        listPaint, filteredColor, scalerContextFlags, viewMatrix, props, glyphRunList,
                        target->glyphPainter());
                GrTextBlob::AssertEqual(*sanityBlob, *cacheBlob);
            }
        }
    } else {
        if (canCache) {
            cacheBlob = textBlobCache->makeCachedBlob(glyphRunList, key, blurRec, listPaint);
        } else {
            cacheBlob = textBlobCache->makeBlob(glyphRunList);
        }
        this->regenerateGlyphRunList(cacheBlob.get(), glyphCache,
                                     *context->contextPriv().caps()->shaderCaps(), listPaint,
                                     filteredColor, scalerContextFlags, viewMatrix, props,
                                     glyphRunList, target->glyphPainter());
    }

    cacheBlob->flush(target, props, fDistanceAdjustTable.get(), listPaint, filteredColor,
                     clip, viewMatrix, origin.x(), origin.y());
}

void GrTextContext::AppendGlyph(GrTextBlob* blob, int runIndex,
                                const sk_sp<GrTextStrike>& strike,
                                const SkGlyph& skGlyph, GrGlyph::MaskStyle maskStyle,
                                SkScalar sx, SkScalar sy,
                                GrColor color, SkGlyphCache* skGlyphCache,
                                SkScalar textRatio, bool needsTransform) {
    GrGlyph::PackedID id = GrGlyph::Pack(skGlyph.getGlyphID(),
                                         skGlyph.getSubXFixed(),
                                         skGlyph.getSubYFixed(),
                                         maskStyle);
    GrGlyph* glyph = strike->getGlyph(skGlyph, id, skGlyphCache);
    if (!glyph) {
        return;
    }

    SkASSERT(skGlyph.fWidth == glyph->width());
    SkASSERT(skGlyph.fHeight == glyph->height());

    bool isDFT = maskStyle == GrGlyph::kDistance_MaskStyle;

    SkRect glyphRect = rect_to_draw(skGlyph, {sx, sy}, textRatio, isDFT);

    if (!glyphRect.isEmpty()) {
        blob->appendGlyph(runIndex, glyphRect, color, strike, glyph, !needsTransform);
    }
}


void GrTextContext::regenerateGlyphRunList(GrTextBlob* cacheBlob,
                                           GrGlyphCache* glyphCache,
                                           const GrShaderCaps& shaderCaps,
                                           const SkPaint& paint,
                                           GrColor filteredColor,
                                           SkScalerContextFlags scalerContextFlags,
                                           const SkMatrix& viewMatrix,
                                           const SkSurfaceProps& props,
                                           const SkGlyphRunList& glyphRunList,
                                           SkGlyphRunListPainter* glyphPainter) {
    struct ARGBFallbackHelper {
        void operator ()(const SkPaint& fallbackPaint, SkSpan<const SkGlyphID> glyphIDs,
                    SkSpan<const SkPoint> positions, SkScalar textScale,
                    const SkMatrix& glyphCacheMatrix,
                    SkGlyphRunListPainter::NeedsTransform needsTransform) const {
            fBlob->initOverride(fRunIndex);
            fBlob->setHasBitmap();
            fBlob->setSubRunHasW(fRunIndex, glyphCacheMatrix.hasPerspective());
            SkExclusiveStrikePtr fallbackCache =
                    fBlob->setupCache(fRunIndex, fProps, fScalerContextFlags,
                                     fallbackPaint, &glyphCacheMatrix);
            sk_sp<GrTextStrike> strike = fGlyphCache->getStrike(fallbackCache.get());
            const SkPoint* glyphPos = positions.data();
            for (auto glyphID : glyphIDs) {
                const SkGlyph& glyph = fallbackCache->getGlyphIDMetrics(glyphID);
                GrTextContext::AppendGlyph(fBlob, fRunIndex, strike, glyph,
                                           GrGlyph::kCoverage_MaskStyle,
                                           glyphPos->fX, glyphPos->fY, fFilteredColor,
                                           fallbackCache.get(), textScale, needsTransform);
                glyphPos++;
            }
        }

        GrTextBlob* const fBlob;
        const int fRunIndex;
        const SkSurfaceProps& fProps;
        const SkScalerContextFlags fScalerContextFlags;
        GrGlyphCache* const fGlyphCache;
        const GrColor fFilteredColor;
    };

    SkPoint origin = glyphRunList.origin();
    cacheBlob->initReusableBlob(
            glyphRunList.paint().computeLuminanceColor(), viewMatrix, origin.x(), origin.y());

    int runIndex = 0;
    for (const auto& glyphRun : glyphRunList) {
        const SkPaint& runPaint = glyphRun.paint();
        cacheBlob->push_back_run(runIndex);

        cacheBlob->setRunPaintFlags(runIndex, runPaint.getFlags());

        if (CanDrawAsDistanceFields(runPaint, viewMatrix, props,
                                    shaderCaps.supportsDistanceFieldText(), fOptions)) {
            bool hasWCoord = viewMatrix.hasPerspective()
                             || fOptions.fDistanceFieldVerticesAlwaysHaveW;

            // Setup distance field runPaint and text ratio
            SkScalar textRatio;
            SkPaint distanceFieldPaint{runPaint};
            SkScalerContextFlags flags;
            InitDistanceFieldPaint(cacheBlob, &distanceFieldPaint, viewMatrix,
                                   fOptions, &textRatio, &flags);
            cacheBlob->setHasDistanceField();
            cacheBlob->setSubRunHasDistanceFields(runIndex, runPaint.isLCDRenderText(),
                                                  runPaint.isAntiAlias(), hasWCoord);

            {
                auto cache = cacheBlob->setupCache(
                        runIndex, props, flags, distanceFieldPaint, &SkMatrix::I());

                sk_sp<GrTextStrike> currStrike = glyphCache->getStrike(cache.get());

                auto perSDF =
                    [cacheBlob, runIndex, &currStrike, filteredColor, cache{cache.get()}, textRatio]
                    (const SkGlyph& glyph, SkPoint position) {
                        if (!glyph.isEmpty()) {
                            SkScalar sx = position.fX,
                                     sy = position.fY;
                            AppendGlyph(cacheBlob, runIndex, currStrike,
                                        glyph, GrGlyph::kDistance_MaskStyle, sx, sy,
                                        filteredColor,
                                        cache, textRatio, true);
                        }
                    };

                auto perPath =
                    [cacheBlob, runIndex, textRatio, cache{cache.get()}]
                    (const SkGlyph& glyph, SkPoint position) {
                        if (!glyph.isEmpty()) {
                            if (const SkPath* glyphPath = cache->findPath(glyph)) {
                                SkScalar sx = position.fX,
                                         sy = position.fY;
                                cacheBlob->appendPathGlyph(
                                        runIndex, *glyphPath, sx, sy, textRatio, false);
                            }
                        }
                    };

                ARGBFallbackHelper argbFallback{cacheBlob, runIndex, props, scalerContextFlags,
                                                glyphCache, filteredColor};

                glyphPainter->drawGlyphRunAsSDFWithARGBFallback(
                        cache.get(), glyphRun, origin, viewMatrix, textRatio,
                        perSDF, perPath, argbFallback);
            }

        } else if (SkDraw::ShouldDrawTextAsPaths(runPaint, viewMatrix)) {
            // The glyphs are big, so use paths to draw them.

            // Ensure the blob is set for bitmaptext
            cacheBlob->setHasBitmap();

            // setup our std runPaint, in hopes of getting hits in the cache
            SkPaint pathPaint(runPaint);

            SkScalar textScale = pathPaint.setupForAsPaths();
            auto pathCache = SkStrikeCache::FindOrCreateStrikeExclusive(
                    pathPaint, &props, scalerContextFlags, &SkMatrix::I());

            // Given a glyph that is not ARGB, draw it.
            auto perPath = [textScale, runIndex, cacheBlob, &pathCache]
                           (const SkGlyph& glyph, SkPoint position) {
                if (!glyph.isEmpty()) {
                    const SkPath* path = pathCache->findPath(glyph);
                    if (path != nullptr) {
                        cacheBlob->appendPathGlyph(
                                runIndex, *path, position.fX, position.fY, textScale, false);
                    }
                }
            };

            ARGBFallbackHelper argbFallback{cacheBlob, runIndex, props, scalerContextFlags,
                                            glyphCache, filteredColor};

            glyphPainter->drawGlyphRunAsPathWithARGBFallback(
                pathCache.get(), glyphRun, origin, viewMatrix, textScale, perPath, argbFallback);
        } else {
            // Ensure the blob is set for bitmaptext
            cacheBlob->setHasBitmap();

            auto cache = cacheBlob->setupCache(
                    runIndex, props, scalerContextFlags, runPaint, &viewMatrix);

            sk_sp<GrTextStrike> currStrike = glyphCache->getStrike(cache.get());

            auto perGlyph =
                [cacheBlob, runIndex, &currStrike, filteredColor, cache{cache.get()}]
                (const SkGlyph& glyph, SkPoint mappedPt) {
                    if (!glyph.isEmpty()) {
                        const void* glyphImage = cache->findImage(glyph);
                        if (glyphImage != nullptr) {
                            SkScalar sx = SkScalarFloorToScalar(mappedPt.fX),
                                     sy = SkScalarFloorToScalar(mappedPt.fY);
                            AppendGlyph(cacheBlob, runIndex, currStrike,
                                        glyph, GrGlyph::kCoverage_MaskStyle, sx, sy,
                                        filteredColor, cache, SK_Scalar1, false);
                        }
                    }
                };

            auto perPath =
                [cacheBlob, runIndex, cache{cache.get()}]
                (const SkGlyph& glyph, SkPoint position) {
                    const SkPath* glyphPath = cache->findPath(glyph);
                    if (glyphPath != nullptr) {
                        SkScalar sx = SkScalarFloorToScalar(position.fX),
                                 sy = SkScalarFloorToScalar(position.fY);
                        cacheBlob->appendPathGlyph(
                                runIndex, *glyphPath, sx, sy, SK_Scalar1, true);
                    }
                };

            glyphPainter->drawGlyphRunAsBMPWithPathFallback(
                    cache.get(), glyphRun, origin, viewMatrix, perGlyph, perPath);
        }
        runIndex += 1;
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

    GrColor filteredColor = generate_filtered_color(skPaint, rtc->colorSpaceInfo());

    auto origin = SkPoint::Make(x, y);
    SkGlyphRunBuilder builder;
    builder.drawText(skPaint, text, textLen, origin);


    auto glyphRunList = builder.useGlyphRunList();
    sk_sp<GrTextBlob> blob;
    if (!glyphRunList.empty()) {
        blob = context->contextPriv().getTextBlobCache()->makeBlob(glyphRunList);
        // Use the text and textLen below, because we don't want to mess with the paint.
        SkScalerContextFlags scalerContextFlags =
                ComputeScalerContextFlags(rtc->colorSpaceInfo());
        textContext->regenerateGlyphRunList(
                blob.get(), glyphCache, *context->contextPriv().caps()->shaderCaps(), skPaint,
                filteredColor, scalerContextFlags, viewMatrix, surfaceProps,
                glyphRunList, rtc->textTarget()->glyphPainter());
    }

    return blob->test_makeOp(textLen, 0, 0, viewMatrix, x, y, skPaint, filteredColor, surfaceProps,
                             textContext->dfAdjustTable(), rtc->textTarget());
}

#endif  // GR_TEST_UTILS
#endif  // SK_SUPPORT_GPU
