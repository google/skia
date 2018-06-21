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

#include "SkDevice.h"
#include "SkDrawFilter.h"
#include "SkGlyphCache.h"
#include "SkMSAN.h"
#include "SkMakeUnique.h"
#include "SkPaint.h"
#include "SkPaintPriv.h"
#include "SkStrikeCache.h"
#include "SkTextBlob.h"
#include "SkTextBlobRunIterator.h"
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

template <typename T>
static SkSpan<T> make_span(T* ptr, ptrdiff_t size) { return SkSpan<T>{ptr, size}; }

template <typename T>
static SkSpan<const T> make_const_span(T* ptr, ptrdiff_t size) {
    return SkSpan<const T>{ptr, size};
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
        , fTemporaryShuntGlyphIDs{glyphIDs}
        , fUniqueGlyphIDs{uniqueGlyphIDs}
        , fText{text}
        , fClusters{clusters}
        , fRunPaint{std::move(runPaint)} {}

void SkGlyphRun::temporaryShuntToDrawPosText(SkBaseDevice* device, SkPoint origin) {

    auto pos = (const SkScalar*) this->positions().data();

    device->drawPosText(
            fTemporaryShuntGlyphIDs.data(), fTemporaryShuntGlyphIDs.size() * sizeof(SkGlyphID),
            pos, 2, origin, fRunPaint);
}

void SkGlyphRun::temporaryShuntToCallback(TemporaryShuntCallback callback) {
    auto bytes = (const char *)fTemporaryShuntGlyphIDs.data();
    auto pos = (const SkScalar*) this->positions().data();
    callback(this->runSize(), bytes, pos);
}

// -- SkGlyphRunList -------------------------------------------------------------------------------
SkGlyphRunList::SkGlyphRunList(
        SkSpan<SkGlyphRun> glyphRuns, SkPoint origin, const SkTextBlob* textBlob)
        : fGlyphRuns{glyphRuns}
        , fOrigin{origin}
        , fTemporaryTextBlobShunt{textBlob} { }

uint64_t SkGlyphRunList::uniqueID() const {
    return fTemporaryTextBlobShunt != nullptr ? fTemporaryTextBlobShunt->uniqueID()
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

void SkGlyphRunList::temporaryShuntBlobnotifyAddedToCache(uint32_t cacheID) const {
    SkASSERT(fTemporaryTextBlobShunt != nullptr);
    fTemporaryTextBlobShunt->notifyAddedToCache(cacheID);
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
    }

    // No need to clear fIndices here... the set insertion algorithm is designed to work
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
        fUniverseToUniqueSize = 4096;
   }

    return make_const_span(uniqueGlyphIDs, uniqueSize);
}

// -- SkGlyphRunBuilder ----------------------------------------------------------------------------
void SkGlyphRunBuilder::prepareDrawText(
        const SkPaint& paint, const void* bytes, size_t byteLength, SkPoint origin) {

    auto glyphIDs = textToGlyphIDs(paint, bytes, byteLength);

    this->initialize(1, glyphIDs.size());

    this->drawText(
            paint, glyphIDs, origin,
            fUniqueGlyphIDIndices.get(), fUniqueGlyphIDs.get(), fPositions.get(),
            SkSpan<const char>(), SkSpan<const uint32_t>());
}

void SkGlyphRunBuilder::prepareDrawPosTextH(const SkPaint& paint, const void* bytes,
                                            size_t byteLength, const SkScalar* xpos,
                                            SkScalar constY) {

    auto glyphIDs = textToGlyphIDs(paint, bytes, byteLength);

    this->initialize(1, glyphIDs.size());

    this->drawPosTextH(
            paint, glyphIDs, xpos, constY,
            fUniqueGlyphIDIndices.get(), fUniqueGlyphIDs.get(), fPositions.get(),
            SkSpan<const char>(), SkSpan<const uint32_t>());
}

void SkGlyphRunBuilder::prepareDrawPosText(const SkPaint& paint, const void* bytes,
                                           size_t byteLength, const SkPoint* pos) {
    auto glyphIDs = textToGlyphIDs(paint, bytes, byteLength);

    this->initialize(1, glyphIDs.size());

    this->drawPosText(
            std::move(paint), glyphIDs, pos,
            fUniqueGlyphIDIndices, fUniqueGlyphIDs,
            SkSpan<const char>(), SkSpan<const uint32_t>());
}

void SkGlyphRunBuilder::prepareTextBlob(
        const SkPaint& paint, const SkTextBlob& blob, SkPoint origin, SkDrawFilter* drawFilter) {
    SkPaint runPaint = paint;

    // Figure out all the storage needed to pre-size everything below. The sizing code assumes
    // that the drawFilter does not change the typeface on the paint.
    size_t runListSize = 0;
    size_t totalGlyphs = 0;
    for (SkTextBlobRunIterator it(&blob); !it.done(); it.next()) {
        totalGlyphs += it.glyphCount();

        runListSize += 1;
    }

    // Pre-size all the buffers so they don't move during processing.
    this->initialize(runListSize, totalGlyphs);

    fTemporaryTextBlobShunt = &blob;
    fOrigin = origin;

    uint16_t* currentDenseIndices = fUniqueGlyphIDIndices;
    SkPoint* currentPositions = fPositions;
    SkGlyphID* currentUniqueGlyphIDs = fUniqueGlyphIDs;

    for (SkTextBlobRunIterator it(&blob); !it.done(); it.next()) {
        // applyFontToPaint() always overwrites the exact same attributes,
        // so it is safe to not re-seed the paint for this reason.
        it.applyFontToPaint(&runPaint);

        if (drawFilter != nullptr && !drawFilter->filter(&runPaint, SkDrawFilter::kText_Type)) {
            // A false return from filter() means we should abort the current draw.
            runPaint = paint;
            continue;
        }

        // These better be glyphs
        SkASSERT(runPaint.getTextEncoding() == SkPaint::kGlyphID_TextEncoding);

        auto text = SkSpan<const char>(it.text(), it.textSize());
        auto clusters = SkSpan<const uint32_t>(it.clusters(), SkTo<ptrdiff_t>(it.glyphCount()));
        const SkPoint& offset = it.offset();
        auto glyphIDs = SkSpan<const SkGlyphID>{it.glyphs(), SkTo<ptrdiff_t>(it.glyphCount())};

        ptrdiff_t uniqueGlyphIDsSize = 0;
        switch (it.positioning()) {
            case SkTextBlob::kDefault_Positioning: {
                uniqueGlyphIDsSize = this->drawText(
                        runPaint, glyphIDs, offset,
                        currentDenseIndices, currentUniqueGlyphIDs, currentPositions,
                        text, clusters);
            }
            break;
            case SkTextBlob::kHorizontal_Positioning: {
                auto constY = offset.y();
                uniqueGlyphIDsSize = this->drawPosTextH(
                        runPaint, glyphIDs, it.pos(), constY,
                        currentDenseIndices, currentUniqueGlyphIDs, currentPositions,
                        text, clusters);
            }
            break;
            case SkTextBlob::kFull_Positioning:
                uniqueGlyphIDsSize = this->drawPosText(
                        runPaint, glyphIDs, (const SkPoint*)it.pos(),
                        currentDenseIndices, currentUniqueGlyphIDs,
                        text, clusters);
            break;
            default:
                SK_ABORT("unhandled positioning mode");
        }

        if (drawFilter != nullptr) {
            // A draw filter may change the paint arbitrarily, so we must re-seed in this case.
            runPaint = paint;
        }
        currentDenseIndices += it.glyphCount();
        currentPositions += it.glyphCount();
        currentUniqueGlyphIDs += uniqueGlyphIDsSize;
    }
}

SkGlyphRun* SkGlyphRunBuilder::useGlyphRun() {
    auto glyphRunList = this->useGlyphRunList();
    SkASSERT(glyphRunList->size() == 1);
    return &(*glyphRunList)[0];
}

SkGlyphRunList* SkGlyphRunBuilder::useGlyphRunList() {
    new ((void*)&fScratchGlyphRunList)
        SkGlyphRunList{SkSpan<SkGlyphRun>(
                fGlyphRuns.get(), fGlyphRunListSize), fOrigin, fTemporaryTextBlobShunt};
    return &fScratchGlyphRunList;
}

void SkGlyphRunBuilder::draw(SkBaseDevice* device) {
    auto glyphRunList = this->useGlyphRunList();
    device->drawGlyphRunList(glyphRunList);
}

void SkGlyphRunBuilder::initialize(
        size_t runListSize, size_t totalGlyphs) {
    // Delete all the old runs
    for (auto& run : fScratchGlyphRunList) {
        run.~SkGlyphRun();
    }

    fTemporaryTextBlobShunt = nullptr;
    fOrigin = {0, 0};
    fGlyphRunListSize = 0;

    if (totalGlyphs > fMaxRunSize) {
        fUniqueGlyphIDIndices.reset(totalGlyphs);
        fPositions.reset(totalGlyphs);
        fUniqueGlyphIDs.reset(totalGlyphs);
        fMaxRunSize = totalGlyphs;
    }

    if (runListSize > fMaxRunListSize) {
        fGlyphRuns.reset(runListSize);
        fMaxRunListSize = runListSize;
    }
}

SkSpan<const SkGlyphID> SkGlyphRunBuilder::textToGlyphIDs(
        const SkPaint& paint, const void* bytes, size_t byteLength) {
    ptrdiff_t runSize = 0;
    SkGlyphID* glyphIDs = nullptr;
    auto encoding = paint.getTextEncoding();
    auto typeface = SkPaintPriv::GetTypefaceOrDefault(paint);
    if (encoding != SkPaint::kGlyphID_TextEncoding) {
        auto tfEncoding = convert_encoding(encoding);
        int utfSize = SkUTFN_CountUnichars(tfEncoding, bytes, byteLength);
        if (utfSize > 0) {
            runSize = SkTo<size_t>(utfSize);
            fScratchGlyphIDs.resize(runSize);
            typeface->charsToGlyphs(bytes, tfEncoding, fScratchGlyphIDs.data(), runSize);
            glyphIDs = fScratchGlyphIDs.data();
        }
    } else {
        runSize = byteLength / 2;
        glyphIDs = (SkGlyphID*)bytes;
    }
    return SkSpan<const SkGlyphID>{glyphIDs, runSize};
}

SkSpan<const SkGlyphID> SkGlyphRunBuilder::addDenseAndUnique(
        const SkPaint& paint,
        SkSpan<const SkGlyphID> glyphIDs,
        uint16_t* denseIndices,
        SkGlyphID* uniqueGlyphIDs) {

    SkSpan<const SkGlyphID> uniquifiedGlyphIDs;
    if (!glyphIDs.empty()) {
        auto typeface = SkPaintPriv::GetTypefaceOrDefault(paint);
        auto glyphUniverseSize = typeface->countGlyphs();
        uniquifiedGlyphIDs = fGlyphIDSet.uniquifyGlyphIDs(
                glyphUniverseSize, glyphIDs, uniqueGlyphIDs, denseIndices);
    }

    return uniquifiedGlyphIDs;
}

void SkGlyphRunBuilder::addGlyphRunToList(
        const SkPaint& runPaint,
        SkSpan<const uint16_t> denseIndices,
        SkSpan<const SkPoint> positions,
        SkSpan<const SkGlyphID> glyphIDs,
        SkSpan<const SkGlyphID> uniqueGlyphIDs,
        SkSpan<const char> text,
        SkSpan<const uint32_t> clusters) {

    // Ignore empty runs.
    if (!denseIndices.empty()) {
        SkPaint modifiedRunPaint{runPaint};

        // TODO: remove these once the text stack has all the encoding and align code removed.
        modifiedRunPaint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
        modifiedRunPaint.setTextAlign(SkPaint::kLeft_Align);

        new (&fGlyphRuns[fGlyphRunListSize++]) SkGlyphRun {
            std::move(modifiedRunPaint),
            denseIndices,
            positions,
            glyphIDs,
            uniqueGlyphIDs,
            text,
            clusters
        };
    }
}


ptrdiff_t SkGlyphRunBuilder::drawText(
        const SkPaint& paint, SkSpan<const SkGlyphID> glyphIDs, SkPoint origin,
        uint16_t* denseIndices, SkGlyphID* uniqueGlyphIDs, SkPoint* positions,
        SkSpan<const char> text, SkSpan<const uint32_t> clusters) {

    auto unqiueGlyphIDs = this->addDenseAndUnique(paint, glyphIDs, denseIndices, uniqueGlyphIDs);

    fScratchAdvances.resize(unqiueGlyphIDs.size());
    {
        auto cache = SkStrikeCache::FindOrCreateStrikeExclusive(paint);
        cache->getAdvances(unqiueGlyphIDs, fScratchAdvances.data());
    }

    SkPoint endOfLastGlyph = origin;

    auto runSize = glyphIDs.size();
    for (ptrdiff_t i = 0; i < runSize; i++) {
        positions[i] = endOfLastGlyph;
        endOfLastGlyph += fScratchAdvances[denseIndices[i]];
    }

    if (paint.getTextAlign() != SkPaint::kLeft_Align) {
        SkVector runWidth = endOfLastGlyph - origin;
        if (paint.getTextAlign() == SkPaint::kCenter_Align) {
            runWidth.scale(SK_ScalarHalf);
        }
        for (ptrdiff_t i = 0; i < runSize; i++) {
            positions[i] -= runWidth;
        }
    }

    this->addGlyphRunToList(
            paint,
            SkSpan<const uint16_t>{denseIndices, runSize},
            SkSpan<const SkPoint>{positions, runSize},
            glyphIDs,
            unqiueGlyphIDs,
            text, clusters);

    return unqiueGlyphIDs.size();
}

ptrdiff_t SkGlyphRunBuilder::drawPosTextH(
        const SkPaint& paint, SkSpan<const SkGlyphID> glyphIDs,
        const SkScalar* xpos, SkScalar constY,
        uint16_t* denseIndices, SkGlyphID* uniqueGlyphIDs, SkPoint* positions,
        SkSpan<const char> text, SkSpan<const uint32_t> clusters) {

    auto unqiueGlyphIDs = this->addDenseAndUnique(paint, glyphIDs, denseIndices, uniqueGlyphIDs);

    auto runSize = glyphIDs.size();
    for (ptrdiff_t i = 0; i < runSize; i++) {
        positions[i] = SkPoint::Make(xpos[i], constY);
    }

    this->addGlyphRunToList(
            paint,
            SkSpan<const uint16_t>{denseIndices, runSize},
            SkSpan<const SkPoint>{positions, runSize},
            glyphIDs,
            unqiueGlyphIDs,
            text, clusters);

    return unqiueGlyphIDs.size();}

ptrdiff_t SkGlyphRunBuilder::drawPosText(
        const SkPaint& paint, SkSpan<const SkGlyphID> glyphIDs, const SkPoint* pos,
        uint16_t* denseIndices, SkGlyphID* uniqueGlyphIDs,
        SkSpan<const char> text, SkSpan<const uint32_t> clusters) {
    auto unqiueGlyphIDs = this->addDenseAndUnique(paint, glyphIDs, denseIndices, uniqueGlyphIDs);

    auto runSize = glyphIDs.size();
    this->addGlyphRunToList(
            paint,
            SkSpan<const uint16_t>{denseIndices, runSize},
            SkSpan<const SkPoint>{pos, runSize},
            glyphIDs,
            unqiueGlyphIDs,
            text, clusters);

    return unqiueGlyphIDs.size();
}


