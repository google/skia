/*
 * Copyright 2018 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkGlyphRun.h"

#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkTextBlob.h"
#include "include/private/SkTo.h"
#include "src/core/SkDevice.h"
#include "src/core/SkFontPriv.h"
#include "src/core/SkStrike.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkTextBlobPriv.h"
#include "src/core/SkUtils.h"

// -- SkGlyphRun -----------------------------------------------------------------------------------
SkGlyphRun::SkGlyphRun(const SkFont& font,
                       SkSpan<const SkPoint> positions,
                       SkSpan<const SkGlyphID> glyphIDs,
                       SkSpan<const char> text,
                       SkSpan<const uint32_t> clusters)
        : fPositions{positions}
        , fGlyphIDs{glyphIDs}
        , fText{text}
        , fClusters{clusters}
        , fFont{font} {}

SkGlyphRun::SkGlyphRun(const SkGlyphRun& that, const SkFont& font)
    : fPositions{that.fPositions}
    , fGlyphIDs{that.fGlyphIDs}
    , fText{that.fText}
    , fClusters{that.fClusters}
    , fFont{font} {}

void SkGlyphRun::filloutGlyphsAndPositions(SkGlyphID* glyphIDs, SkPoint* positions) {
    memcpy(glyphIDs, fGlyphIDs.data(), fGlyphIDs.size_bytes());
    memcpy(positions, fPositions.data(), fPositions.size_bytes());
}

// -- SkGlyphRunList -------------------------------------------------------------------------------
SkGlyphRunList::SkGlyphRunList() = default;
SkGlyphRunList::SkGlyphRunList(
        const SkPaint& paint,
        const SkTextBlob* blob,
        SkPoint origin,
        SkSpan<const SkGlyphRun> glyphRunList)
        : fOriginalPaint{&paint}
        , fOriginalTextBlob{blob}
        , fOrigin{origin}
        , fGlyphRuns{glyphRunList} { }

SkGlyphRunList::SkGlyphRunList(const SkGlyphRun& glyphRun, const SkPaint& paint)
        : fOriginalPaint{&paint}
        , fOriginalTextBlob{nullptr}
        , fOrigin{SkPoint::Make(0, 0)}
        , fGlyphRuns{SkSpan<const SkGlyphRun>{&glyphRun, 1}} {}

uint64_t SkGlyphRunList::uniqueID() const {
    return fOriginalTextBlob != nullptr ? fOriginalTextBlob->uniqueID()
                                        : SK_InvalidUniqueID;
}

bool SkGlyphRunList::anyRunsLCD() const {
    for (const auto& r : fGlyphRuns) {
        if (r.font().getEdging() == SkFont::Edging::kSubpixelAntiAlias) {
            return true;
        }
    }
    return false;
}

bool SkGlyphRunList::anyRunsSubpixelPositioned() const {
    for (const auto& r : fGlyphRuns) {
        if (r.font().isSubpixel()) {
            return true;
        }
    }
    return false;
}

bool SkGlyphRunList::allFontsFinite() const {
    for (const auto& r : fGlyphRuns) {
        if (!SkFontPriv::IsFinite(r.font())) {
            return false;
        }
    }
    return true;
}

void SkGlyphRunList::temporaryShuntBlobNotifyAddedToCache(uint32_t cacheID) const {
    SkASSERT(fOriginalTextBlob != nullptr);
    fOriginalTextBlob->notifyAddedToCache(cacheID);
}

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
void SkGlyphRunBuilder::drawTextUTF8(const SkPaint& paint, const SkFont& font, const void* bytes,
                                     size_t byteLength, SkPoint origin) {
    auto glyphIDs = textToGlyphIDs(font, bytes, byteLength, SkTextEncoding::kUTF8);
    if (!glyphIDs.empty()) {
        this->initialize(glyphIDs.size());
        this->simplifyDrawText(font, glyphIDs, origin, fPositions);
    }

    this->makeGlyphRunList(paint, nullptr, SkPoint::Make(0, 0));
}

void SkGlyphRunBuilder::drawTextBlob(const SkPaint& paint, const SkTextBlob& blob, SkPoint origin,
                                     SkBaseDevice* device) {
    // Figure out all the storage needed to pre-size everything below.
    size_t totalGlyphs = 0;
    for (SkTextBlobRunIterator it(&blob); !it.done(); it.next()) {
        totalGlyphs += it.glyphCount();
    }

    // Pre-size all the buffers so they don't move during processing.
    this->initialize(totalGlyphs);

    SkPoint* positions = fPositions;

    for (SkTextBlobRunIterator it(&blob); !it.done(); it.next()) {
        // applyFontToPaint() always overwrites the exact same attributes,
        // so it is safe to not re-seed the paint for this reason.
        size_t runSize = it.glyphCount();

        auto text = SkSpan<const char>(it.text(), it.textSize());
        auto clusters = SkSpan<const uint32_t>(it.clusters(), runSize);
        const SkPoint& offset = it.offset();
        auto glyphIDs = SkSpan<const SkGlyphID>{it.glyphs(), runSize};

        switch (it.positioning()) {
            case SkTextBlobRunIterator::kDefault_Positioning: {
                this->simplifyDrawText(
                        it.font(), glyphIDs, offset, positions, text, clusters);
            }
                break;
            case SkTextBlobRunIterator::kHorizontal_Positioning: {
                auto constY = offset.y();
                this->simplifyDrawPosTextH(
                        it.font(), glyphIDs, it.pos(), constY, positions, text, clusters);
            }
                break;
            case SkTextBlobRunIterator::kFull_Positioning:
                this->simplifyDrawPosText(
                        it.font(), glyphIDs, (const SkPoint*)it.pos(), text, clusters);
                break;
            case SkTextBlobRunIterator::kRSXform_Positioning: {
                if (!this->empty()) {
                    this->makeGlyphRunList(paint, &blob, origin);
                    device->drawGlyphRunList(this->useGlyphRunList());
                }

                device->drawGlyphRunRSXform(it.font(), it.glyphs(), (const SkRSXform*)it.pos(),
                                            runSize, origin, paint);

                // re-init in case we keep looping and need the builder again
                this->initialize(totalGlyphs);
            } break;
        }

        positions += runSize;
    }

    if (!this->empty()) {
        this->makeGlyphRunList(paint, &blob, origin);
        device->drawGlyphRunList(this->useGlyphRunList());
    }
}

void SkGlyphRunBuilder::drawGlyphsWithPositions(const SkPaint& paint, const SkFont& font,
                                            SkSpan<const SkGlyphID> glyphIDs, const SkPoint* pos) {
    if (!glyphIDs.empty()) {
        this->initialize(glyphIDs.size());
        this->simplifyDrawPosText(font, glyphIDs, pos);
        this->makeGlyphRunList(paint, nullptr, SkPoint::Make(0, 0));
    }
}

const SkGlyphRunList& SkGlyphRunBuilder::useGlyphRunList() {
    return fGlyphRunList;
}

void SkGlyphRunBuilder::initialize(size_t totalRunSize) {

    if (totalRunSize > fMaxTotalRunSize) {
        fMaxTotalRunSize = totalRunSize;
        fPositions.reset(fMaxTotalRunSize);
    }

    fGlyphRunListStorage.clear();
}

SkSpan<const SkGlyphID> SkGlyphRunBuilder::textToGlyphIDs(
        const SkFont& font, const void* bytes, size_t byteLength, SkTextEncoding encoding) {
    if (encoding != SkTextEncoding::kGlyphID) {
        int count = font.countText(bytes, byteLength, encoding);
        if (count > 0) {
            fScratchGlyphIDs.resize(count);
            font.textToGlyphs(bytes, byteLength, encoding, fScratchGlyphIDs.data(), count);
            return SkSpan<const SkGlyphID>{fScratchGlyphIDs};
        } else {
            return SkSpan<const SkGlyphID>();
        }
    } else {
        return SkSpan<const SkGlyphID>((const SkGlyphID*)bytes, byteLength / 2);
    }
}

void SkGlyphRunBuilder::makeGlyphRun(
        const SkFont& font,
        SkSpan<const SkGlyphID> glyphIDs,
        SkSpan<const SkPoint> positions,
        SkSpan<const char> text,
        SkSpan<const uint32_t> clusters) {

    // Ignore empty runs.
    if (!glyphIDs.empty()) {
        fGlyphRunListStorage.emplace_back(
                font,
                positions,
                glyphIDs,
                text,
                clusters);
    }
}

void SkGlyphRunBuilder::makeGlyphRunList(
        const SkPaint& paint, const SkTextBlob* blob, SkPoint origin) {

    fGlyphRunList.~SkGlyphRunList();
    new (&fGlyphRunList) SkGlyphRunList{
        paint, blob, origin, SkSpan<const SkGlyphRun>{fGlyphRunListStorage}};
}

void SkGlyphRunBuilder::simplifyDrawText(
        const SkFont& font, SkSpan<const SkGlyphID> glyphIDs,
        SkPoint origin, SkPoint* positions,
        SkSpan<const char> text, SkSpan<const uint32_t> clusters) {
    SkASSERT(!glyphIDs.empty());

    auto runSize = glyphIDs.size();

    if (!glyphIDs.empty()) {
        fScratchAdvances.resize(runSize);
        {
            auto cache = SkStrikeCache::FindOrCreateStrikeWithNoDeviceExclusive(font);
            cache->getAdvances(glyphIDs, fScratchAdvances.data());
        }

        SkPoint endOfLastGlyph = origin;

        for (size_t i = 0; i < runSize; i++) {
            positions[i] = endOfLastGlyph;
            endOfLastGlyph += fScratchAdvances[i];
        }

        this->makeGlyphRun(
                font,
                glyphIDs,
                SkSpan<const SkPoint>{positions, runSize},
                text,
                clusters);
    }
}

void SkGlyphRunBuilder::simplifyDrawPosTextH(
        const SkFont& font, SkSpan<const SkGlyphID> glyphIDs,
        const SkScalar* xpos, SkScalar constY, SkPoint* positions,
        SkSpan<const char> text, SkSpan<const uint32_t> clusters) {

    auto posCursor = positions;
    for (auto x : SkSpan<const SkScalar>{xpos, glyphIDs.size()}) {
        *posCursor++ = SkPoint::Make(x, constY);
    }

    simplifyDrawPosText(font, glyphIDs, positions, text, clusters);
}

void SkGlyphRunBuilder::simplifyDrawPosText(
        const SkFont& font, SkSpan<const SkGlyphID> glyphIDs,
        const SkPoint* pos,
        SkSpan<const char> text, SkSpan<const uint32_t> clusters) {
    auto runSize = glyphIDs.size();

    this->makeGlyphRun(
            font,
            glyphIDs,
            SkSpan<const SkPoint>{pos, runSize},
            text,
            clusters);
}
