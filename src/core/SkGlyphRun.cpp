/*
 * Copyright 2018 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGlyphRun.h"

#include "SkGlyphCache.h"
#include "SkPaint.h"
#include "SkPaintPriv.h"
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
SkGlyphRun::SkGlyphRun(const SkPaint& basePaint,
                       const SkRunFont& runFont,
                       SkSpan<const SkPoint> positions,
                       SkSpan<const SkGlyphID> glyphIDs,
                       SkSpan<const char> text,
                       SkSpan<const uint32_t> clusters)
        : fPositions{positions}
        , fGlyphIDs{glyphIDs}
        , fText{text}
        , fClusters{clusters}
        , fRunPaint{basePaint, runFont} {}

SkGlyphRun::SkGlyphRun(const SkGlyphRun& that, const SkPaint& paint)
    : fPositions{that.fPositions}
    , fGlyphIDs{that.fGlyphIDs}
    , fText{that.fText}
    , fClusters{that.fClusters}
    , fRunPaint{paint} {}

void SkGlyphRun::eachGlyphToGlyphRun(SkGlyphRun::PerGlyph perGlyph) {
    SkPoint point;
    SkGlyphID glyphID;
    SkGlyphRun run{
        fRunPaint,
        SkRunFont{fRunPaint},
        SkSpan<const SkPoint>{&point, 1},
        SkSpan<const SkGlyphID>{&glyphID, 1},
        SkSpan<const char>{},
        SkSpan<const uint32_t>{}
    };

    auto runSize = fGlyphIDs.size();
    for (size_t i = 0; i < runSize; i++) {
        glyphID = fGlyphIDs[i];
        point = fPositions[i];
        perGlyph(run);
    }
}

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

SkGlyphRunList::SkGlyphRunList(const SkGlyphRun& glyphRun)
        : fOriginalPaint{&glyphRun.paint()}
        , fOriginalTextBlob{nullptr}
        , fOrigin{SkPoint::Make(0, 0)}
        , fGlyphRuns{SkSpan<const SkGlyphRun>{&glyphRun, 1}} {}

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

bool SkGlyphRunList::anyRunsSubpixelPositioned() const {
    for (const auto& r : fGlyphRuns) {
        if (r.paint().isSubpixelText()) {
            return true;
        }
    }
    return false;
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
            SkRunFont{paint},
            glyphIDs,
            positions,
            SkSpan<const char>{},
            SkSpan<const uint32_t>{});
    this->makeGlyphRunList(paint, nullptr, SkPoint::Make(0, 0));
}

void SkGlyphRunBuilder::drawText(
        const SkPaint& paint, const void* bytes, size_t byteLength, SkPoint origin) {
    auto glyphIDs = textToGlyphIDs(paint, bytes, byteLength);
    if (!glyphIDs.empty()) {
        this->initialize(glyphIDs.size());
        this->simplifyDrawText(paint, SkRunFont{paint}, glyphIDs, origin, fPositions);
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
                paint, SkRunFont{paint}, glyphIDs, xpos, constY, fPositions);
    }

    this->makeGlyphRunList(paint, nullptr, SkPoint::Make(0, 0));
}

void SkGlyphRunBuilder::drawPosText(const SkPaint& paint, const void* bytes,
                                    size_t byteLength, const SkPoint* pos) {
    auto glyphIDs = textToGlyphIDs(paint, bytes, byteLength);
    if (!glyphIDs.empty()) {
        this->initialize(glyphIDs.size());
        this->simplifyDrawPosText(paint, SkRunFont{paint}, glyphIDs, pos);
    }

    this->makeGlyphRunList(paint, nullptr, SkPoint::Make(0, 0));
}

void SkGlyphRunBuilder::drawTextBlob(const SkPaint& paint, const SkTextBlob& blob, SkPoint origin) {
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
                        paint, it.runFont(), glyphIDs, offset, positions, text, clusters);
            }
                break;
            case SkTextBlobRunIterator::kHorizontal_Positioning: {
                auto constY = offset.y();
                this->simplifyDrawPosTextH(
                        paint, it.runFont(), glyphIDs, it.pos(), constY, positions, text, clusters);
            }
                break;
            case SkTextBlobRunIterator::kFull_Positioning:
                this->simplifyDrawPosText(
                        paint, it.runFont(), glyphIDs, (const SkPoint*)it.pos(), text, clusters);
                break;
        }

        positions += runSize;
    }

    this->makeGlyphRunList(paint, &blob, origin);
}

void SkGlyphRunBuilder::drawGlyphPos(
        const SkPaint& paint, SkSpan<const SkGlyphID> glyphIDs, const SkPoint* pos) {
    if (!glyphIDs.empty()) {
        this->initialize(glyphIDs.size());
        this->simplifyDrawPosText(paint, SkRunFont{paint}, glyphIDs, pos);
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

void SkGlyphRunBuilder::makeGlyphRun(
        const SkPaint& basePaint,
        const SkRunFont& runFont,
        SkSpan<const SkGlyphID> glyphIDs,
        SkSpan<const SkPoint> positions,
        SkSpan<const char> text,
        SkSpan<const uint32_t> clusters) {

    // Ignore empty runs.
    if (!glyphIDs.empty()) {
        fGlyphRunListStorage.emplace_back(
                basePaint,
                runFont,
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
        const SkPaint& paint, const SkRunFont& runFont, SkSpan<const SkGlyphID> glyphIDs,
        SkPoint origin, SkPoint* positions,
        SkSpan<const char> text, SkSpan<const uint32_t> clusters) {
    SkASSERT(!glyphIDs.empty());

    auto runSize = glyphIDs.size();

    SkPaint runPaint{paint, runFont};

    if (!glyphIDs.empty()) {
        fScratchAdvances.resize(runSize);
        {
            const SkFont font = SkFont::LEGACY_ExtractFromPaint(runPaint);
            auto cache = SkStrikeCache::FindOrCreateStrikeWithNoDeviceExclusive(font, runPaint);
            cache->getAdvances(glyphIDs, fScratchAdvances.data());
        }

        SkPoint endOfLastGlyph = origin;

        for (size_t i = 0; i < runSize; i++) {
            positions[i] = endOfLastGlyph;
            endOfLastGlyph += fScratchAdvances[i];
        }

        this->makeGlyphRun(
                paint,
                runFont,
                glyphIDs,
                SkSpan<const SkPoint>{positions, runSize},
                text,
                clusters);
    }
}

void SkGlyphRunBuilder::simplifyDrawPosTextH(
        const SkPaint& paint, const SkRunFont& runFont, SkSpan<const SkGlyphID> glyphIDs,
        const SkScalar* xpos, SkScalar constY, SkPoint* positions,
        SkSpan<const char> text, SkSpan<const uint32_t> clusters) {

    auto posCursor = positions;
    for (auto x : SkSpan<const SkScalar>{xpos, glyphIDs.size()}) {
        *posCursor++ = SkPoint::Make(x, constY);
    }

    simplifyDrawPosText(paint, runFont, glyphIDs, positions, text, clusters);
}

void SkGlyphRunBuilder::simplifyDrawPosText(
        const SkPaint& paint, const SkRunFont& runFont, SkSpan<const SkGlyphID> glyphIDs,
        const SkPoint* pos,
        SkSpan<const char> text, SkSpan<const uint32_t> clusters) {
    auto runSize = glyphIDs.size();

    this->makeGlyphRun(
            paint,
            runFont,
            glyphIDs,
            SkSpan<const SkPoint>{pos, runSize},
            text,
            clusters);
}
