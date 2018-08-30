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
#include "SkFindAndPlaceGlyph.h"
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
        SkAxisAlignment axisAlignment = cache->getScalerContext()->computeAxisAlignmentForHText();
        SkPoint rounding = SkFindAndPlaceGlyph::SubpixelPositionRounding(axisAlignment);
        matrix.preTranslate(origin.x(), origin.y());
        matrix.postTranslate(rounding.x(), rounding.y());
        matrix.mapPoints(fPositions, glyphRun.positions().data(), runSize);

        const SkPoint* positionCursor = fPositions;
        for (auto glyphID : glyphRun.shuntGlyphsIDs()) {
            auto position = *positionCursor++;
            if (SkScalarsAreFinite(position.fX, position.fY)) {
                SkFixed lookupX = SkScalarToFixed(SkScalarFraction(position.fX)),
                        lookupY = SkScalarToFixed(SkScalarFraction(position.fY));

                // Snap to a given axis if alignment is requested.
                if (axisAlignment == kX_SkAxisAlignment ) {
                    lookupY = 0;
                } else if (axisAlignment == kY_SkAxisAlignment) {
                    lookupX = 0;
                }

                const SkGlyph& glyph = cache->getGlyphIDMetrics(glyphID, lookupX, lookupY);
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

            // Temporarily jam in kFill, so we only ever ask for the raw outline from the cache.
            pathPaint.setStyle(SkPaint::kFill_Style);
            pathPaint.setPathEffect(nullptr);

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


#if SK_SUPPORT_GPU

template <typename PerSDFT, typename PerPathT, typename PerFallbackT>
void SkGlyphRunListPainter::drawGlyphRunAsSDFWithFallback(
        SkGlyphCache* cache, const SkGlyphRun& glyphRun,
        SkPoint origin, SkScalar textRatio,
        PerSDFT perSDF, PerPathT perPath, PerFallbackT perFallback) {

    const SkPoint* positionCursor = glyphRun.positions().data();
    for (auto glyphID : glyphRun.shuntGlyphsIDs()) {
        const SkGlyph& glyph = cache->getGlyphIDMetrics(glyphID);
        SkPoint glyphPos = origin + *positionCursor++;
        if (glyph.fWidth > 0) {
            if (glyph.fMaskFormat == SkMask::kSDF_Format) {

                if (SkGlyphCacheCommon::GlyphTooBigForAtlas(glyph)) {
                    SkRect glyphRect =
                            rect_to_draw(glyph, glyphPos, textRatio, true);
                    if (!glyphRect.isEmpty()) {
                        const SkPath* glyphPath = cache->findPath(glyph);
                        if (glyphPath != nullptr) {
                            perPath(glyphPath, glyph, glyphPos);
                        }
                    }
                } else {
                    perSDF(glyph, glyphPos);
                }

            } else {
                perFallback(glyph, glyphPos);
            }
        }
    }
}

// -- GrTextContext --------------------------------------------------------------------------------
GrColor generate_filtered_color(const SkPaint& paint, const GrColorSpaceInfo& colorSpaceInfo) {
    GrColor4f filteredColor = SkColorToUnpremulGrColor4f(paint.getColor(), colorSpaceInfo);
    if (paint.getColorFilter() != nullptr) {
        filteredColor = GrColor4f::FromSkColor4f(
                paint.getColorFilter()->filterColor4f(filteredColor.toSkColor4f(),
                                                      colorSpaceInfo.colorSpace()));
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
                                GrGlyphCache* grGlyphCache,
                                sk_sp<GrTextStrike>* strike,
                                const SkGlyph& skGlyph, GrGlyph::MaskStyle maskStyle,
                                SkScalar sx, SkScalar sy,
                                GrColor color, SkGlyphCache* skGlyphCache,
                                SkScalar textRatio, bool needsTransform) {
    if (!*strike) {
        *strike = grGlyphCache->getStrike(skGlyphCache);
    }

    GrGlyph::PackedID id = GrGlyph::Pack(skGlyph.getGlyphID(),
                                         skGlyph.getSubXFixed(),
                                         skGlyph.getSubYFixed(),
                                         maskStyle);
    GrGlyph* glyph = (*strike)->getGlyph(skGlyph, id, skGlyphCache);
    if (!glyph) {
        return;
    }

    SkASSERT(skGlyph.fWidth == glyph->width());
    SkASSERT(skGlyph.fHeight == glyph->height());

    bool isDFT = maskStyle == GrGlyph::kDistance_MaskStyle;

    SkRect glyphRect = rect_to_draw(skGlyph, {sx, sy}, textRatio, isDFT);

    if (!glyphRect.isEmpty()) {
        blob->appendGlyph(runIndex, glyphRect, color, *strike, glyph, !needsTransform);
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

            FallbackGlyphRunHelper fallbackTextHelper(
                    viewMatrix, runPaint, glyphCache->getGlyphSizeLimit(), textRatio);

            sk_sp<GrTextStrike> currStrike;

            {

                auto cache = cacheBlob->setupCache(
                        runIndex, props, flags, distanceFieldPaint, nullptr);

                auto perSDF =
                        [cacheBlob, runIndex, glyphCache, &currStrike,
                                filteredColor, cache{cache.get()}, textRatio]
                                (const SkGlyph& glyph, SkPoint position) {
                            SkScalar sx = position.fX,
                                    sy = position.fY;
                            AppendGlyph(cacheBlob, runIndex, glyphCache, &currStrike,
                                        glyph, GrGlyph::kDistance_MaskStyle, sx, sy,
                                        filteredColor,
                                        cache, textRatio, true);
                        };

                auto perPath =
                        [cacheBlob, runIndex, textRatio]
                                (const SkPath* path, const SkGlyph& glyph, SkPoint position) {
                            SkScalar sx = position.fX,
                                    sy = position.fY;
                            cacheBlob->appendPathGlyph(
                                    runIndex, *path, sx, sy, textRatio, false);
                        };

                auto perFallback =
                        [&fallbackTextHelper]
                                (const SkGlyph& glyph, SkPoint position) {
                            fallbackTextHelper.appendGlyph(glyph, glyph.getGlyphID(), position);
                        };

                glyphPainter->drawGlyphRunAsSDFWithFallback(
                        cache.get(), glyphRun, origin, textRatio, perSDF, perPath, perFallback);
            }

            fallbackTextHelper.drawGlyphs(
                    cacheBlob, runIndex, glyphCache, props,
                    runPaint, filteredColor, scalerContextFlags);

        } else if (SkDraw::ShouldDrawTextAsPaths(runPaint, viewMatrix)) {
            // Ensure the blob is set for bitmaptext
            cacheBlob->setHasBitmap();

            // setup our std runPaint, in hopes of getting hits in the cache
            SkPaint pathPaint(runPaint);
            SkScalar matrixScale = pathPaint.setupForAsPaths();

            FallbackGlyphRunHelper fallbackTextHelper(
                    viewMatrix, runPaint, glyphCache->getGlyphSizeLimit(), matrixScale);

            // Temporarily jam in kFill, so we only ever ask for the raw outline from the cache.
            pathPaint.setStyle(SkPaint::kFill_Style);
            pathPaint.setPathEffect(nullptr);

            auto cache = SkStrikeCache::FindOrCreateStrikeExclusive(
                    pathPaint, &props, SkScalerContextFlags::kFakeGammaAndBoostContrast, nullptr);

            auto drawOnePath =
                    [&fallbackTextHelper, matrixScale, runIndex, cacheBlob]
                            (const SkPath* path, const SkGlyph& glyph, SkPoint position) {
                        if (glyph.fMaskFormat == SkMask::kARGB32_Format) {
                            fallbackTextHelper.appendGlyph(glyph, glyph.getGlyphID(), position);
                        } else {
                            if (path != nullptr) {
                                cacheBlob->appendPathGlyph(
                                        runIndex, *path, position.fX, position.fY, matrixScale, false);
                            }
                        }
                    };

            glyphPainter->drawUsingPaths(glyphRun, origin, cache.get(), drawOnePath);

            fallbackTextHelper.drawGlyphs(
                    cacheBlob, runIndex, glyphCache, props,
                    runPaint, filteredColor, scalerContextFlags);

        } else {
            // Ensure the blob is set for bitmaptext
            cacheBlob->setHasBitmap();
            sk_sp<GrTextStrike> currStrike;
            auto cache = cacheBlob->setupCache(
                    runIndex, props, scalerContextFlags, runPaint, &viewMatrix);

            auto perGlyph =
                    [cacheBlob, runIndex, glyphCache, &currStrike, filteredColor, cache{cache.get()}]
                            (const SkGlyph& glyph, SkPoint mappedPt) {
                        if (!glyph.isEmpty()) {
                            const void* glyphImage = cache->findImage(glyph);
                            if (glyphImage != nullptr) {
                                SkScalar sx = SkScalarFloorToScalar(mappedPt.fX),
                                        sy = SkScalarFloorToScalar(mappedPt.fY);
                                AppendGlyph(cacheBlob, runIndex, glyphCache, &currStrike,
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
