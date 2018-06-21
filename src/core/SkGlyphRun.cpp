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
}  // namespace

// -- SkGlyphSet ----------------------------------------------------------------------------------
uint32_t SkGlyphSet::uniqueSize() {
    // The size is how big the vector is grown since being passed into reuse.
    return fUniqueGlyphIDs->size() - fStartOfUniqueIDs;
}

uint16_t SkGlyphSet::add(SkGlyphID glyphID) {
    static constexpr SkGlyphID  kUndefGlyph{0};

    if (glyphID >= fUniverseSize) {
        glyphID = kUndefGlyph;
    }

    if (glyphID >= fIndices.size()) {
        fIndices.resize(glyphID + 1);
    }

    auto index = fIndices[glyphID];

    // Remember we start at the end of what ever was passed in.
    if (index < this->uniqueSize() && (*fUniqueGlyphIDs)[fStartOfUniqueIDs + index] == glyphID) {
        return index;
    }

    uint16_t newIndex = SkTo<uint16_t>(this->uniqueSize());
    fUniqueGlyphIDs->push_back(glyphID);
    fIndices[glyphID] = newIndex;
    return newIndex;
}



void SkGlyphSet::reuse(uint32_t glyphUniverseSize, std::vector<SkGlyphID>* uniqueGlyphIDs) {
    SkASSERT(glyphUniverseSize <= (1 << 16));
    fUniverseSize = glyphUniverseSize;
    fUniqueGlyphIDs = uniqueGlyphIDs;

    // Capture the vector end to act as the start of a new unique id vector.
    fStartOfUniqueIDs = uniqueGlyphIDs->size();

    // If we're hanging onto these arrays for a long time, we don't want their size to drift
    // endlessly upwards. It's unusual to see a typeface with more than 4096 possible glyphs.
    if (glyphUniverseSize < 4096 && fIndices.size() > 4096) {
        fIndices.resize(4096);
        fIndices.shrink_to_fit();
    }

    // No need to clear fIndices here... SkGlyphSet's set insertion algorithm is designed to work
    // correctly even when the fIndexes buffer is uninitialized!
}

__attribute__((noinline)) static size_t process_unique_glyphs(
        uint32_t universeSize, SkSpan<const SkGlyphID> glyphIDs,
        SkGlyphID* uniqueGlyphIDs, uint16_t* denseIndicies, uint16_t* scratch) {
    static constexpr SkGlyphID  kUndefGlyph{0};
    size_t uniqueSize = 0;
    size_t denseIndiciesCursor = 0;
    for (auto glyphID : glyphIDs) {
        if (glyphID >= universeSize) {
            glyphID = kUndefGlyph;
        }

        auto index = scratch[glyphID];

        // Remember we start at the end of what ever was passed in.
        if (index >= uniqueSize || uniqueGlyphIDs[index] != glyphID) {
            index = SkTo<uint16_t>(uniqueSize);
            uniqueGlyphIDs[uniqueSize++] = glyphID;
            scratch[glyphID] = index;
        }

        denseIndicies[denseIndiciesCursor++] = index;
    }

    return uniqueSize;
}

// -- SkGlyphRun -----------------------------------------------------------------------------------
SkGlyphRun::SkGlyphRun(const SkIndexedRunInfo& runInfo,
                       size_t denseOffset, size_t denseSize,
                       size_t uniqueOffset, uint16_t uniqueSize,
                       SkSpan<SkGlyphID>  scratchGlyphs,
                       SkSpan<const char> text,
                       SkSpan<const uint32_t> clusters,
                       SkPaint&&          runPaint)
        : fRunInfo{runInfo}
        , fDenseOffset{denseOffset}, fDenseSize{denseSize}
        , fUniqueOffset{uniqueOffset}, fUniqueSize{uniqueSize}
        , fTemporaryShuntGlyphIDs{scratchGlyphs}
        , fText{text}
        , fClusters{clusters}
        , fRunPaint{std::move(runPaint)} {}

void SkGlyphRun::temporaryShuntToDrawPosText(SkBaseDevice* device, SkPoint origin) {

    auto pos = (const SkScalar*) this->positions().data();

    device->drawPosText(
            fTemporaryShuntGlyphIDs.data(), fDenseSize * sizeof(SkGlyphID),
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

// -- SkGlyphRunBuilder ----------------------------------------------------------------------------
void SkGlyphRunBuilder::prepareDrawText(
        const SkPaint& paint, const void* bytes, size_t byteLength, SkPoint origin) {
    this->initialize();
    auto glyphs = textToGlyphIDs(paint, bytes, byteLength);
    this->drawText(
            paint, bytes, byteLength, origin, SkSpan<const char>(), SkSpan<const uint32_t>());
}

void SkGlyphRunBuilder::prepareDrawPosTextH(const SkPaint& paint, const void* bytes,
                                            size_t byteLength, const SkScalar* xpos,
                                            SkScalar constY) {
    this->initialize();
    auto glyphs = textToGlyphIDs(paint, bytes, byteLength);
    this->drawPosTextH(
            paint, bytes, byteLength, xpos, constY,
            SkSpan<const char>(), SkSpan<const uint32_t>());
}

void SkGlyphRunBuilder::prepareDrawPosText(const SkPaint& paint, const void* bytes,
                                           size_t byteLength, const SkPoint* pos) {
    this->initialize();
    auto glyphs = textToGlyphIDs(paint, bytes, byteLength);
    this->drawPosText(paint, bytes, byteLength, pos,
            SkSpan<const char>(), SkSpan<const uint32_t>());
}

void SkGlyphRunBuilder::prepareTextBlob(
        const SkPaint& paint, const SkTextBlob& blob, SkPoint origin, SkDrawFilter* drawFilter) {
    this->initialize();
    fTemporaryTextBlobShunt = &blob;
    fOrigin = origin;

    SkPaint runPaint = paint;

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
        auto clusters = SkSpan<const uint32_t>(it.clusters(), it.glyphCount());
        size_t glyphLen = it.glyphCount() * sizeof(SkGlyphID);
        const SkPoint& offset = it.offset();

        switch (it.positioning()) {
            case SkTextBlob::kDefault_Positioning: {
                this->drawText(runPaint, it.glyphs(), glyphLen, offset, text, clusters);
            }
            break;
            case SkTextBlob::kHorizontal_Positioning: {
                auto constY = offset.y();
                this->drawPosTextH(
                        runPaint, it.glyphs(), glyphLen, it.pos(), constY, text, clusters);
            }
            break;
            case SkTextBlob::kFull_Positioning:
                this->drawPosText(
                        runPaint, it.glyphs(), glyphLen, (const SkPoint*)it.pos(), text,
                        clusters);
            break;
            default:
                SK_ABORT("unhandled positioning mode");
        }

        if (drawFilter != nullptr) {
            // A draw filter may change the paint arbitrarily, so we must re-seed in this case.
            runPaint = paint;
        }
    }
}

SkGlyphRun* SkGlyphRunBuilder::useGlyphRun() {
    auto glyphRunList = this->useGlyphRunList();
    SkASSERT(glyphRunList->size() == 1);
    return &(*glyphRunList)[0];
}

SkGlyphRunList* SkGlyphRunBuilder::useGlyphRunList() {
    new ((void*)&fScratchGlyphRunList)
        SkGlyphRunList{SkSpan<SkGlyphRun>(fGlyphRuns), fOrigin, fTemporaryTextBlobShunt};
    return &fScratchGlyphRunList;
}

void SkGlyphRunBuilder::draw(SkBaseDevice* device) {
    auto glyphRunList = this->useGlyphRunList();
    device->drawGlyphRunList(glyphRunList);
}

size_t SkGlyphRunBuilder::runSize() const { return fDenseIndex.size() - fLastDenseIndex; }

size_t SkGlyphRunBuilder::uniqueSize() const { return fUniqueGlyphIDs.size() - fLastUniqueIndex; }

void SkGlyphRunBuilder::initialize() {
    fTemporaryTextBlobShunt = nullptr;
    fDenseIndex.clear();
    fPositions.clear();
    fUniqueGlyphIDs.clear();
    fGlyphRuns.clear();
    fLastDenseIndex = 0;
    fLastUniqueIndex = 0;
    fOrigin = {0, 0};
}

SkSpan<const SkGlyphID> SkGlyphRunBuilder::textToGlyphIDs(
        const SkPaint& paint, const void* bytes, size_t byteLength) {
    size_t runSize = 0;
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

SkGlyphID* SkGlyphRunBuilder::addDenseAndUnique(
        const SkPaint& paint, SkSpan<const SkGlyphID> glyphIDs) {

    auto glyphUniverseSize = paint.getTypeface()->countGlyphs();

    if (glyphIDs.size() > 0) {
        process_unique_glyphs(glyphUniverseSize, glyphIDs,
                &fDenseIndex, &fUniqueGlyphIDs, &fGlyphUniverse);
    }

    return glyphIDs;
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
    if (denseIndices.size() > 0) {
        SkPaint modifiedRunPaint{runPaint};

        // TODO: remove these once the text stack has all the encoding and align code removed.
        modifiedRunPaint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
        modifiedRunPaint.setTextAlign(SkPaint::kLeft_Align);

        new (&fGlyphRuns[fGlyphRunListCursor++]) SkGlyphRun {
            modifiedRunPaint,
            denseIndices,
            positions,
            glyphIDs,
            uniqueGlyphIDs,
            text,
            clusters
        };
    }
}

void SkGlyphRunBuilder::sizeBuffers(
        size_t runListSize, size_t totalGlyphs, uint16_t glyphUniverseSize) {
    if (totalGlyphs > fMaxGlyphSize) {
        fUniqueGlyphIDIndicies.reset(totalGlyphs);
        fPositions.reset(totalGlyphs);
        fUniqueGlyphIDs.reset(totalGlyphs);
        fMaxGlyphSize = totalGlyphs;
    }

    if (glyphUniverseSize > fMaxGlyphUniverseSize) {
        fGlyphUniverse.reset(glyphUniverseSize);
        fMaxGlyphUniverseSize = glyphUniverseSize;
    }

    if (runListSize > fMaxRunListSize) {
        fUniqueGlyphIDsSizes.reset(runListSize);
        fGlyphRuns.reset(runListSize);
        fMaxRunListSize = runListSize;
    }
}

void SkGlyphRunBuilder::drawText(
        const SkPaint& paint, SkSpan<const SkGlyphID> glyphIDs, SkPoint origin,
        uint16_t* denseIndicies, SkGlyphID* uniqueGlyphIDs, SkPoint* positions,
        SkSpan<const char> text, SkSpan<const uint32_t> clusters) {

    auto uniqueGlyphIDSize = this->addDenseAndUnique(paint, glyphIDs);

    fScratchAdvances.resize(this->uniqueSize());
    {
        auto cache = SkStrikeCache::FindOrCreateStrikeExclusive(paint);
        cache->getAdvances(
                fIndexed.uniqueGlyphIDs(fLastUniqueIndex, uniqueSize()), fScratchAdvances.data());
    }

    SkPoint endOfLastGlyph = origin;

    for (size_t i = 0; i < this->runSize(); i++) {
        fPositions.push_back(endOfLastGlyph);
        endOfLastGlyph += fScratchAdvances[fDenseIndex[fLastDenseIndex + i]];
    }

    if (paint.getTextAlign() != SkPaint::kLeft_Align) {
        SkVector runWidth = endOfLastGlyph - origin;
        if (paint.getTextAlign() == SkPaint::kCenter_Align) {
            runWidth.scale(SK_ScalarHalf);
        }
        for (size_t i = fLastDenseIndex; i < this->runSize(); i++) {
            fPositions[i] -= runWidth;
        }
    }

    this->addGlyphRunToList(paint, temporaryShuntGlyphIDs, text, clusters);
}

void SkGlyphRunBuilder::drawPosTextH(
        const SkPaint& paint, SkSpan<SkGlyphID> glyphIDs,
        const SkScalar* xpos, SkScalar constY,
        SkSpan<const char> text, SkSpan<const uint32_t> clusters) {

    SkGlyphID* temporaryShuntGlyphIDs = this->addDenseAndUnique(paint, bytes, byteLength);

    for (size_t i = 0; i < runSize(); i++) {
        fPositions.push_back(SkPoint::Make(xpos[i], constY));
    }

    this->addGlyphRunToList(paint, temporaryShuntGlyphIDs, text, clusters);
}

void SkGlyphRunBuilder::drawPosText(
        const SkPaint& paint, SkSpan<const SkGlyphID> glyphIDs, const SkPoint* pos,
        uint16_t* denseIndicies, SkGlyphID* uniqueGlyphIDs, SkPoint* positions,
        SkSpan<const char> text, SkSpan<const uint32_t> clusters) {
    auto uniqueGlyphIDSize = this->addDenseAndUnique(paint, glyphIDs);

    this->addGlyphRunToList(paint, temporaryShuntGlyphIDs, text, clusters);
}


