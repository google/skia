/*
 * Copyright 2018 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGlyphRun.h"

#include <algorithm>
#include <new>
#include <tuple>

#if SK_SUPPORT_GPU
#include "GrColorSpaceInfo.h"
#include "GrRenderTargetContext.h"
#endif

#include "SkDevice.h"
#include "SkDistanceFieldGen.h"
#include "SkDraw.h"
#include "SkFindAndPlaceGlyph.h"
#include "SkGlyphCache.h"
#include "SkMSAN.h"
#include "SkMakeUnique.h"
#include "SkMatrix.h"
#include "SkPaint.h"
#include "SkPaintPriv.h"
#include "SkPathEffect.h"
#include "SkRasterClip.h"
#include "SkStrikeCache.h"
#include "SkTextBlob.h"
#include "SkTextBlobPriv.h"
#include "SkTo.h"
#include "SkUtils.h"

namespace {
static SkTypeface::Encoding convert_encoding(SkPaint::TextEncoding encoding) {
    switch (encoding) {
        case  SkPaint::kUTF8_TextEncoding: return SkTypeface::kUTF8_Encoding;
        case SkPaint::kUTF16_TextEncoding: return SkTypeface::kUTF16_Encoding;
        case SkPaint::kUTF32_TextEncoding: return SkTypeface::kUTF32_Encoding;
        default: return SkTypeface::kUTF32_Encoding;
    }
}
}  // namespace

// -- SkGlyphRun -----------------------------------------------------------------------------------
SkGlyphRun::SkGlyphRun(SkPaint&& runPaint,
                       SkSpan<const uint16_t> denseIndices,
                       SkSpan<const SkPoint> positions,
                       SkSpan<const SkGlyphID> glyphIDs,
                       SkSpan<const SkGlyphID> uniqueGlyphIDs,
                       SkSpan<const char> text,
                       SkSpan<const uint32_t> clusters)
        : fUniqueGlyphIDIndices{denseIndices}
        , fPositions{positions}
        , fGlyphIDs{glyphIDs}
        , fUniqueGlyphIDs{uniqueGlyphIDs}
        , fText{text}
        , fClusters{clusters}
        , fRunPaint{std::move(runPaint)} {}


void SkGlyphRun::eachGlyphToGlyphRun(SkGlyphRun::PerGlyph perGlyph) {
    SkPaint paint{fRunPaint};
    SkPoint point;
    SkGlyphID glyphID;
    SkGlyphRun run{
        std::move(paint),
        SkSpan<const uint16_t>{},  // No dense indices for now.
        SkSpan<const SkPoint>{&point, 1},
        SkSpan<const SkGlyphID>{&glyphID, 1},
        SkSpan<const SkGlyphID>{},
        SkSpan<const char>{},
        SkSpan<const uint32_t>{}
    };

    auto runSize = fGlyphIDs.size();
    auto runPaint = run.mutablePaint();
    for (size_t i = 0; i < runSize; i++) {
        glyphID = fGlyphIDs[i];
        point = fPositions[i];
        perGlyph(&run, runPaint);
    }
}

void SkGlyphRun::temporaryShuntToDrawPosText(SkBaseDevice* device, SkPoint origin) {

    auto pos = (const SkScalar*) this->positions().data();

    if (!fGlyphIDs.empty()) {
        device->drawPosText(
                fGlyphIDs.data(), fGlyphIDs.size() * sizeof(SkGlyphID),
                pos, 2, origin, fRunPaint);
    }
}

void SkGlyphRun::temporaryShuntToCallback(TemporaryShuntCallback callback) {
    auto bytes = (const char *)fGlyphIDs.data();
    auto pos = (const SkScalar*) this->positions().data();
    callback(fGlyphIDs.size(), bytes, pos);
}

void SkGlyphRun::filloutGlyphsAndPositions(SkGlyphID* glyphIDs, SkPoint* positions) {
    memcpy(glyphIDs, fGlyphIDs.data(), fGlyphIDs.size_bytes());
    memcpy(positions, fPositions.data(), fPositions.size_bytes());
}

// -- SkGlyphRunListDrawer -------------------------------------------------------------------------
SkGlyphRunListDrawer::SkGlyphRunListDrawer(
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

SkGlyphRunListDrawer::SkGlyphRunListDrawer(
        const SkSurfaceProps& props, const GrColorSpaceInfo& csi)
        : SkGlyphRunListDrawer(props, kUnknown_SkColorType, compute_scaler_context_flags(csi)) {}

SkGlyphRunListDrawer::SkGlyphRunListDrawer(const GrRenderTargetContext& rtc)
        : SkGlyphRunListDrawer{rtc.surfaceProps(), rtc.colorSpaceInfo()} {}
#endif

// TODO: all this logic should move to the glyph cache.
static const SkGlyph& lookup_glyph_by_subpixel(
        SkAxisAlignment axisAlignment, SkPoint position, SkGlyphID glyphID, SkGlyphCache* cache) {
    SkFixed lookupX = SkScalarToFixed(SkScalarFraction(position.x())),
            lookupY = SkScalarToFixed(SkScalarFraction(position.y()));

    // Snap to a given axis if alignment is requested.
    if (axisAlignment == kX_SkAxisAlignment) {
        lookupY = 0;
    } else if (axisAlignment == kY_SkAxisAlignment) {
        lookupX = 0;
    }

    return cache->getGlyphIDMetrics(glyphID, lookupX, lookupY);
}


// forEachMappedDrawableGlyph handles positioning for mask type glyph handling for both sub-pixel
// and full pixel positioning.
template <typename EachGlyph>
void SkGlyphRunListDrawer::forEachMappedDrawableGlyph(
        const SkGlyphRun& glyphRun, SkPoint origin, const SkMatrix& deviceMatrix,
        SkGlyphCache* cache, EachGlyph eachGlyph) {
    bool isSubpixel = cache->isSubpixel();

    SkAxisAlignment axisAlignment = kNone_SkAxisAlignment;
    SkMatrix mapping = deviceMatrix;
    mapping.preTranslate(origin.x(), origin.y());
    // TODO: all this logic should move to the glyph cache.
    if (isSubpixel) {
        axisAlignment = cache->getScalerContext()->computeAxisAlignmentForHText();
        SkPoint rounding = SkFindAndPlaceGlyph::SubpixelPositionRounding(axisAlignment);
        mapping.postTranslate(rounding.x(), rounding.y());
    } else {
        mapping.postTranslate(SK_ScalarHalf, SK_ScalarHalf);
    }

    auto runSize = glyphRun.runSize();
    if (this->ensureBitmapBuffers(runSize)) {
        mapping.mapPoints(fPositions, glyphRun.positions().data(), runSize);
        const SkPoint* mappedPtCursor = fPositions;
        const SkPoint* ptCursor = glyphRun.positions().data();
        for (auto glyphID : glyphRun.shuntGlyphsIDs()) {
            auto mappedPt = *mappedPtCursor++;
            auto pt = origin + *ptCursor++;
            if (SkScalarsAreFinite(mappedPt.x(), mappedPt.y())) {
                // TODO: all this logic should move to the glyph cache.
                const SkGlyph& glyph =
                    isSubpixel ? lookup_glyph_by_subpixel(axisAlignment, mappedPt, glyphID, cache)
                               : cache->getGlyphIDMetrics(glyphID);
                if (!glyph.isEmpty()) {
                    // Prevent glyphs from being drawn outside of or straddling the edge
                    // of device space. Comparisons written a little weirdly so that NaN
                    // coordinates are treated safely.
                    auto le = [](float a, int b) { return a <= (float)b; };
                    auto ge = [](float a, int b) { return a >= (float)b; };
                    if (le(mappedPt.fX, INT_MAX - (INT16_MAX + SkTo<int>(UINT16_MAX))) &&
                        ge(mappedPt.fX, INT_MIN - (INT16_MIN + 0 /*UINT16_MIN*/)) &&
                        le(mappedPt.fY, INT_MAX - (INT16_MAX + SkTo<int>(UINT16_MAX))) &&
                        ge(mappedPt.fY, INT_MIN - (INT16_MIN + 0 /*UINT16_MIN*/)))
                    {
                        eachGlyph(glyph, pt, mappedPt);
                    }
                }
            }
        }
    }
}


bool SkGlyphRunListDrawer::ShouldDrawAsPath(const SkPaint& paint, const SkMatrix& matrix) {
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

bool SkGlyphRunListDrawer::ensureBitmapBuffers(size_t runSize) {
    if (runSize > fMaxRunSize) {
        fPositions.reset(runSize);
        fMaxRunSize = runSize;
    }

    return true;
}

void SkGlyphRunListDrawer::drawUsingPaths(
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

static bool glyph_too_big_for_atlas(const SkGlyph& glyph) {
    return glyph.fWidth >= 256 || glyph.fHeight >= 256;
}

void SkGlyphRunListDrawer::drawGlyphRunAsSubpixelMask(
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

void SkGlyphRunListDrawer::drawGlyphRunAsGlyphWithPathFallback(
        SkGlyphCache* cache, const SkGlyphRun& glyphRun,
        SkPoint origin, const SkMatrix& deviceMatrix,
        PerGlyph perGlyph, PerPath perPath) {
    auto eachGlyph =
            [cache, perGlyph{std::move(perGlyph)}, perPath{std::move(perPath)}]
            (const SkGlyph& glyph, SkPoint pt, SkPoint mappedPt) {
        if (glyph_too_big_for_atlas(glyph)) {
            const SkPath* glyphPath = cache->findPath(glyph);
            if (glyphPath != nullptr) {
                perPath(glyphPath, glyph, mappedPt);
            }
        } else {
            const void* glyphImage = cache->findImage(glyph);
            if (glyphImage != nullptr) {
                perGlyph(glyph, mappedPt);
            }
        }
    };

    this->forEachMappedDrawableGlyph(glyphRun, origin, deviceMatrix, cache, eachGlyph);
}

void SkGlyphRunListDrawer::drawGlyphRunAsFullpixelMask(
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


void SkGlyphRunListDrawer::drawForBitmapDevice(
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

void SkGlyphRunListDrawer::drawUsingMasks(
        SkGlyphCache* cache, const SkGlyphRun& glyphRun,
        SkPoint origin, const SkMatrix& deviceMatrix, PerMask perMask) {
    if (cache->isSubpixel()) {
        this->drawGlyphRunAsSubpixelMask(cache, glyphRun, origin, deviceMatrix, perMask);
    } else {
        this->drawGlyphRunAsFullpixelMask(cache, glyphRun, origin, deviceMatrix, perMask);
    }
}

// -- SkGlyphRunList -------------------------------------------------------------------------------
SkGlyphRunList::SkGlyphRunList() = default;
SkGlyphRunList::SkGlyphRunList(
        const SkPaint& paint,
        const SkTextBlob* blob,
        SkPoint origin,
        SkSpan<SkGlyphRun> glyphRunList)
        : fOriginalPaint{&paint}
        , fOriginalTextBlob{blob}
        , fOrigin{origin}
        , fGlyphRuns{glyphRunList} { }

SkGlyphRunList::SkGlyphRunList(SkGlyphRun* glyphRun)
        : fOriginalPaint{&glyphRun->paint()}
        , fOriginalTextBlob{nullptr}
        , fOrigin{SkPoint::Make(0, 0)}
        , fGlyphRuns{SkSpan<SkGlyphRun>{glyphRun, 1}} {}

uint64_t SkGlyphRunList::uniqueID() const {
    return fOriginalTextBlob != nullptr ? fOriginalTextBlob->uniqueID()
                                        : SK_InvalidUniqueID;
}

bool SkGlyphRunList::anyRunsLCD() const {
    for (const auto& r : fGlyphRuns) {
        if (r.paint().isLCDRenderText()) {
            return true;
        }
    }
    return false;
}

void SkGlyphRunList::temporaryShuntBlobNotifyAddedToCache(uint32_t cacheID) const {
    SkASSERT(fOriginalTextBlob != nullptr);
    fOriginalTextBlob->notifyAddedToCache(cacheID);
}

// -- SkGlyphRunListIterator -----------------------------------------------------------------------
constexpr SkPoint SkGlyphRunListIterator::fZero;

// -- SkGlyphIDSet ---------------------------------------------------------------------------------
// A faster set implementation that does not need any initialization, and reading the set items
// is order the number of items, and not the size of the universe.
// This implementation is based on the paper by Briggs and Torczon, "An Efficient Representation
// for Sparse Sets"
//
// This implementation assumes that the unique glyphs added are appended to a vector that may
// already have unique glyph from a previous computation. This allows the packing of multiple
// UniqueID sequences in a single vector.
SkSpan<const SkGlyphID> SkGlyphIDSet::uniquifyGlyphIDs(
        uint32_t universeSize,
        SkSpan<const SkGlyphID> glyphIDs,
        SkGlyphID* uniqueGlyphIDs,
        uint16_t* denseIndices) {
    static constexpr SkGlyphID  kUndefGlyph{0};

    if (universeSize > fUniverseToUniqueSize) {
        fUniverseToUnique.reset(universeSize);
        fUniverseToUniqueSize = universeSize;
        // If the following bzero becomes a performance problem, the memory can be marked as
        // initialized for valgrind and msan.
        // valgrind = VALGRIND_MAKE_MEM_DEFINED(fUniverseToUnique, universeSize * sizeof(SkGlyphID))
        // msan = sk_msan_mark_initialized(fUniverseToUnique, universeSize * sizeof(SkGlyphID))
        sk_bzero(fUniverseToUnique, universeSize * sizeof(SkGlyphID));
    }

    // No need to clear fUniverseToUnique here... the set insertion algorithm is designed to work
    // correctly even when the fUniverseToUnique buffer is uninitialized!

    size_t uniqueSize = 0;
    size_t denseIndicesCursor = 0;
    for (auto glyphID : glyphIDs) {

        // If the glyphID is not in range then it is the undefined glyph.
        if (glyphID >= universeSize) {
            glyphID = kUndefGlyph;
        }

        // The index into the unique ID vector.
        auto uniqueIndex = fUniverseToUnique[glyphID];

        if (uniqueIndex >= uniqueSize || uniqueGlyphIDs[uniqueIndex] != glyphID) {
            uniqueIndex = SkTo<uint16_t>(uniqueSize);
            uniqueGlyphIDs[uniqueSize] = glyphID;
            fUniverseToUnique[glyphID] = uniqueIndex;
            uniqueSize += 1;
        }

        denseIndices[denseIndicesCursor++] = uniqueIndex;
    }

    // If we're hanging onto these arrays for a long time, we don't want their size to drift
    // endlessly upwards. It's unusual to see a typeface with more than 4096 possible glyphs.
    if (fUniverseToUniqueSize > 4096) {
        fUniverseToUnique.reset(4096);
        sk_bzero(fUniverseToUnique, 4096 * sizeof(SkGlyphID));
        fUniverseToUniqueSize = 4096;
    }

    return SkSpan<const SkGlyphID>(uniqueGlyphIDs, uniqueSize);
}

// -- SkGlyphRunBuilder ----------------------------------------------------------------------------
void SkGlyphRunBuilder::drawTextAtOrigin(
        const SkPaint& paint, const void* bytes, size_t byteLength) {
    auto glyphIDs = textToGlyphIDs(paint, bytes, byteLength);
    if (!glyphIDs.empty()) {
        this->initialize(glyphIDs.size());
    }

    auto positions = SkSpan<const SkPoint>{fPositions, glyphIDs.size()};

    // Every glyph is at the origin.
    sk_bzero((void *)positions.data(), positions.size_bytes());

    this->makeGlyphRun(
            paint,
            glyphIDs,
            positions,
            SkSpan<const uint16_t>{},  // no dense indices for now.,
            SkSpan<const SkGlyphID>{},
            SkSpan<const char>{},
            SkSpan<const uint32_t>{});
    this->makeGlyphRunList(paint, nullptr, SkPoint::Make(0, 0));
}

void SkGlyphRunBuilder::drawText(
        const SkPaint& paint, const void* bytes, size_t byteLength, SkPoint origin) {
    auto glyphIDs = textToGlyphIDs(paint, bytes, byteLength);
    if (!glyphIDs.empty()) {
        this->initialize(glyphIDs.size());
        this->simplifyDrawText(
                paint, glyphIDs, origin, fUniqueGlyphIDIndices, fUniqueGlyphIDs, fPositions);
    }

    this->makeGlyphRunList(paint, nullptr, SkPoint::Make(0, 0));
}

void SkGlyphRunBuilder::drawPosTextH(const SkPaint& paint, const void* bytes,
                                     size_t byteLength, const SkScalar* xpos,
                                     SkScalar constY) {
    auto glyphIDs = textToGlyphIDs(paint, bytes, byteLength);
    if (!glyphIDs.empty()) {
        this->initialize(glyphIDs.size());
        this->simplifyDrawPosTextH(
                paint, glyphIDs, xpos, constY, fUniqueGlyphIDIndices, fUniqueGlyphIDs, fPositions);
    }

    this->makeGlyphRunList(paint, nullptr, SkPoint::Make(0, 0));
}

void SkGlyphRunBuilder::drawPosText(const SkPaint& paint, const void* bytes,
                                    size_t byteLength, const SkPoint* pos) {
    auto glyphIDs = textToGlyphIDs(paint, bytes, byteLength);
    if (!glyphIDs.empty()) {
        this->initialize(glyphIDs.size());
        this->simplifyDrawPosText(paint, glyphIDs, pos,
                fUniqueGlyphIDIndices, fUniqueGlyphIDs);
    }

    this->makeGlyphRunList(paint, nullptr, SkPoint::Make(0, 0));
}

void SkGlyphRunBuilder::drawTextBlob(const SkPaint& paint, const SkTextBlob& blob, SkPoint origin) {
    SkPaint runPaint = paint;

    // Figure out all the storage needed to pre-size everything below.
    size_t totalGlyphs = 0;
    for (SkTextBlobRunIterator it(&blob); !it.done(); it.next()) {
        totalGlyphs += it.glyphCount();
    }

    // Pre-size all the buffers so they don't move during processing.
    this->initialize(totalGlyphs);

    uint16_t* currentDenseIndices = fUniqueGlyphIDIndices;
    SkPoint* currentPositions = fPositions;
    SkGlyphID* currentUniqueGlyphIDs = fUniqueGlyphIDs;

    for (SkTextBlobRunIterator it(&blob); !it.done(); it.next()) {
        // applyFontToPaint() always overwrites the exact same attributes,
        // so it is safe to not re-seed the paint for this reason.
        it.applyFontToPaint(&runPaint);
        size_t runSize = it.glyphCount();

        // These better be glyphs
        SkASSERT(runPaint.getTextEncoding() == SkPaint::kGlyphID_TextEncoding);

        auto text = SkSpan<const char>(it.text(), it.textSize());
        auto clusters = SkSpan<const uint32_t>(it.clusters(), runSize);
        const SkPoint& offset = it.offset();
        auto glyphIDs = SkSpan<const SkGlyphID>{it.glyphs(), runSize};

        size_t uniqueGlyphIDsSize = 0;
        switch (it.positioning()) {
            case SkTextBlobRunIterator::kDefault_Positioning: {
                uniqueGlyphIDsSize = this->simplifyDrawText(
                        runPaint, glyphIDs, offset,
                        currentDenseIndices, currentUniqueGlyphIDs, currentPositions,
                        text, clusters);
            }
                break;
            case SkTextBlobRunIterator::kHorizontal_Positioning: {
                auto constY = offset.y();
                uniqueGlyphIDsSize = this->simplifyDrawPosTextH(
                        runPaint, glyphIDs, it.pos(), constY,
                        currentDenseIndices, currentUniqueGlyphIDs, currentPositions,
                        text, clusters);
            }
                break;
            case SkTextBlobRunIterator::kFull_Positioning:
                uniqueGlyphIDsSize = this->simplifyDrawPosText(
                        runPaint, glyphIDs, (const SkPoint*)it.pos(),
                        currentDenseIndices, currentUniqueGlyphIDs,
                        text, clusters);
                break;
        }

        currentDenseIndices += runSize;
        currentPositions += runSize;
        currentUniqueGlyphIDs += uniqueGlyphIDsSize;
    }

    this->makeGlyphRunList(paint, &blob, origin);
}

void SkGlyphRunBuilder::drawGlyphPos(
        const SkPaint& paint, SkSpan<const SkGlyphID> glyphIDs, const SkPoint* pos) {
    if (!glyphIDs.empty()) {
        this->initialize(glyphIDs.size());
        this->simplifyDrawPosText(paint, glyphIDs, pos,
                fUniqueGlyphIDIndices, fUniqueGlyphIDs);
        this->makeGlyphRunList(paint, nullptr, SkPoint::Make(0, 0));
    }
}

const SkGlyphRunList& SkGlyphRunBuilder::useGlyphRunList() {
    return fGlyphRunList;
}

void SkGlyphRunBuilder::initialize(size_t totalRunSize) {

    if (totalRunSize > fMaxTotalRunSize) {
        fMaxTotalRunSize = totalRunSize;
        fUniqueGlyphIDIndices.reset(fMaxTotalRunSize);
        fPositions.reset(fMaxTotalRunSize);
        fUniqueGlyphIDs.reset(fMaxTotalRunSize);
    }

    fGlyphRunListStorage.clear();
}

SkSpan<const SkGlyphID> SkGlyphRunBuilder::textToGlyphIDs(
        const SkPaint& paint, const void* bytes, size_t byteLength) {
    auto encoding = paint.getTextEncoding();
    if (encoding != SkPaint::kGlyphID_TextEncoding) {
        auto tfEncoding = convert_encoding(encoding);
        int utfSize = SkUTFN_CountUnichars(tfEncoding, bytes, byteLength);
        if (utfSize > 0) {
            size_t runSize = SkTo<size_t>(utfSize);
            fScratchGlyphIDs.resize(runSize);
            auto typeface = SkPaintPriv::GetTypefaceOrDefault(paint);
            typeface->charsToGlyphs(bytes, tfEncoding, fScratchGlyphIDs.data(), runSize);
            return SkSpan<const SkGlyphID>{fScratchGlyphIDs};
        } else {
            return SkSpan<const SkGlyphID>();
        }
    } else {
        return SkSpan<const SkGlyphID>((const SkGlyphID*)bytes, byteLength / 2);
    }
}

SkSpan<const SkGlyphID> SkGlyphRunBuilder::addDenseAndUnique(
        const SkPaint& paint,
        SkSpan<const SkGlyphID> glyphIDs,
        uint16_t* uniqueGlyphIDIndices,
        SkGlyphID* uniqueGlyphIDs) {
    SkSpan<const SkGlyphID> uniquifiedGlyphIDs;
    if (!glyphIDs.empty()) {
        auto typeface = SkPaintPriv::GetTypefaceOrDefault(paint);
        auto glyphUniverseSize = typeface->countGlyphs();

        // There better be glyphs in the font if we want to uniqify.
        if (glyphUniverseSize > 0) {
            uniquifiedGlyphIDs = fGlyphIDSet.uniquifyGlyphIDs(
                    glyphUniverseSize, glyphIDs, uniqueGlyphIDs, uniqueGlyphIDIndices);
        }
    }

    return uniquifiedGlyphIDs;
}

void SkGlyphRunBuilder::makeGlyphRun(
        const SkPaint& runPaint,
        SkSpan<const SkGlyphID> glyphIDs,
        SkSpan<const SkPoint> positions,
        SkSpan<const uint16_t> uniqueGlyphIDIndices,
        SkSpan<const SkGlyphID> uniqueGlyphIDs,
        SkSpan<const char> text,
        SkSpan<const uint32_t> clusters) {

    // Ignore empty runs.
    if (!glyphIDs.empty()) {
        SkPaint glyphRunPaint{runPaint};
        glyphRunPaint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
        glyphRunPaint.setTextAlign(SkPaint::kLeft_Align);

        fGlyphRunListStorage.emplace_back(
                std::move(glyphRunPaint),
                uniqueGlyphIDIndices,
                positions,
                glyphIDs,
                uniqueGlyphIDs,
                text,
                clusters);
    }
}

void SkGlyphRunBuilder::makeGlyphRunList(
        const SkPaint& paint, const SkTextBlob* blob, SkPoint origin) {

    fGlyphRunList.~SkGlyphRunList();
    new (&fGlyphRunList) SkGlyphRunList{
        paint, blob, origin, SkSpan<SkGlyphRun>{fGlyphRunListStorage}};
}

size_t SkGlyphRunBuilder::simplifyDrawText(
        const SkPaint& paint, SkSpan<const SkGlyphID> glyphIDs, SkPoint origin,
        uint16_t* uniqueGlyphIDIndicesBuffer, SkGlyphID* uniqueGlyphIDsBuffer, SkPoint* positions,
        SkSpan<const char> text, SkSpan<const uint32_t> clusters) {
    SkASSERT(!glyphIDs.empty());

    auto runSize = glyphIDs.size();

    auto unqiueGlyphIDs = this->addDenseAndUnique(
            paint, glyphIDs, uniqueGlyphIDIndicesBuffer, uniqueGlyphIDsBuffer);

    if (!unqiueGlyphIDs.empty()) {
        fScratchAdvances.resize(runSize);
        {
            auto cache = SkStrikeCache::FindOrCreateStrikeExclusive(paint);
            cache->getAdvances(unqiueGlyphIDs, fScratchAdvances.data());
        }

        SkPoint endOfLastGlyph = origin;

        for (size_t i = 0; i < runSize; i++) {
            positions[i] = endOfLastGlyph;
            endOfLastGlyph += fScratchAdvances[uniqueGlyphIDIndicesBuffer[i]];
        }

        if (paint.getTextAlign() != SkPaint::kLeft_Align) {
            SkVector len = endOfLastGlyph - origin;
            if (paint.getTextAlign() == SkPaint::kCenter_Align) {
                len.scale(SK_ScalarHalf);
            }
            for (auto& pt : SkSpan<SkPoint>{positions, runSize}) {
                pt -= len;
            }

        }

        this->makeGlyphRun(
                paint,
                glyphIDs,
                SkSpan<const SkPoint>{positions, runSize},
                SkSpan<const uint16_t>{uniqueGlyphIDIndicesBuffer, runSize},
                unqiueGlyphIDs,
                text,
                clusters);
    }

    return unqiueGlyphIDs.size();
}

size_t SkGlyphRunBuilder::simplifyDrawPosTextH(
        const SkPaint& paint, SkSpan<const SkGlyphID> glyphIDs,
        const SkScalar* xpos, SkScalar constY,
        uint16_t* uniqueGlyphIDIndicesBuffer, SkGlyphID* uniqueGlyphIDsBuffer, SkPoint* positions,
        SkSpan<const char> text, SkSpan<const uint32_t> clusters) {

    auto posCursor = positions;
    for (auto x : SkSpan<const SkScalar>{xpos, glyphIDs.size()}) {
        *posCursor++ = SkPoint::Make(x, constY);
    }

    return simplifyDrawPosText(paint, glyphIDs, positions,
            uniqueGlyphIDIndicesBuffer, uniqueGlyphIDsBuffer,
            text, clusters);
}

size_t SkGlyphRunBuilder::simplifyDrawPosText(
        const SkPaint& paint, SkSpan<const SkGlyphID> glyphIDs, const SkPoint* pos,
        uint16_t* uniqueGlyphIDIndicesBuffer, SkGlyphID* uniqueGlyphIDsBuffer,
        SkSpan<const char> text, SkSpan<const uint32_t> clusters) {
    auto runSize = glyphIDs.size();

    // The dense indices are not used by the rest of the stack yet.
    SkSpan<const SkGlyphID> uniqueGlyphIDs;
    #ifdef SK_DEBUG
        uniqueGlyphIDs = this->addDenseAndUnique(
                paint, glyphIDs, uniqueGlyphIDIndicesBuffer, uniqueGlyphIDsBuffer);
    #endif

    // TODO: when using the unique glyph system have a guard that there are actually glyphs like
    // drawText above.
    this->makeGlyphRun(
            paint,
            glyphIDs,
            SkSpan<const SkPoint>{pos, runSize},
            SkSpan<const SkGlyphID>{uniqueGlyphIDIndicesBuffer, runSize},
            uniqueGlyphIDs,
            text,
            clusters);
    return uniqueGlyphIDs.size();
}


