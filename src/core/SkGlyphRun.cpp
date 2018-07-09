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
#include "SkDraw.h"
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

void SkGlyphRun::temporaryShuntToDrawPosText(SkBaseDevice* device) {

    auto pos = (const SkScalar*) this->positions().data();
    auto origin = SkPoint::Make(0, 0);

    if (!fTemporaryShuntGlyphIDs.empty()) {
        device->drawPosText(
                fTemporaryShuntGlyphIDs.data(), fTemporaryShuntGlyphIDs.size() * sizeof(SkGlyphID),
                pos, 2, origin, fRunPaint);
    }
}

void SkGlyphRun::temporaryShuntToCallback(TemporaryShuntCallback callback) {
    auto bytes = (const char *)fTemporaryShuntGlyphIDs.data();
    auto pos = (const SkScalar*) this->positions().data();
    callback(fTemporaryShuntGlyphIDs.size(), bytes, pos);
}

// -- SkGlyphRunBuilder ----------------------------------------------------------------------------
void SkGlyphRunBuilder::prepareDrawText(
        const SkPaint& paint, const void* bytes, size_t byteLength, SkPoint origin) {
    this->initialize();
    SkSpan<const char> originalText((const char*)bytes, byteLength);
    if (paint.getTextEncoding() != SkPaint::kUTF8_TextEncoding) {
        originalText = SkSpan<const char>();
    }
    this->drawText(paint, bytes, byteLength, origin, originalText, SkSpan<const uint32_t>());
}

void SkGlyphRunBuilder::prepareDrawPosTextH(const SkPaint& paint, const void* bytes,
                                            size_t byteLength, const SkScalar* xpos,
                                            SkScalar constY) {
    this->initialize();
    this->drawPosTextH(
            paint, bytes, byteLength, xpos, constY, SkSpan<const char>(), SkSpan<const uint32_t>());
}

void SkGlyphRunBuilder::prepareDrawPosText(const SkPaint& paint, const void* bytes,
                                           size_t byteLength, const SkPoint* pos) {
    this->initialize();
    this->drawPosText(paint, bytes, byteLength, pos,
            SkSpan<const char>(), SkSpan<const uint32_t>());
}

SkGlyphRun* SkGlyphRunBuilder::useGlyphRun() {
    return &fScratchGlyphRun;
}

void SkGlyphRunBuilder::initialize() {
    fUniqueID = 0;
    fDenseIndex.clear();
    fPositions.clear();
    fUniqueGlyphIDs.clear();

    // Be sure to clean up the last run before we reuse it.
    fScratchGlyphRun.~SkGlyphRun();
}

void SkGlyphRunBuilder::addDenseAndUnique(
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

    SkASSERT(glyphIDs != nullptr);

    if (runSize > 0) {
        fGlyphSet.reuse(typeface->countGlyphs(), &fUniqueGlyphIDs);
        for (size_t i = 0; i < runSize; i++) {
            fDenseIndex.push_back(fGlyphSet.add(glyphIDs[i]));
        }
    }
}

void SkGlyphRunBuilder::makeGlyphRun(
        const SkPaint& runPaint,
        SkSpan<const char> text, SkSpan<const uint32_t> clusters) {

    // Ignore empty runs.
    if (!fDenseIndex.empty()) {
        SkPaint glyphRunPaint{runPaint};
        glyphRunPaint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
        glyphRunPaint.setTextAlign(SkPaint::kLeft_Align);

        new ((void*)&fScratchGlyphRun) SkGlyphRun{
                std::move(glyphRunPaint),
                SkSpan<const uint16_t>{fDenseIndex},
                SkSpan<const SkPoint>{fPositions},
                SkSpan<const SkGlyphID>{fScratchGlyphIDs},
                SkSpan<const SkGlyphID>{fUniqueGlyphIDs},
                text,
                clusters
        };
    }
}

void SkGlyphRunBuilder::drawText(
        const SkPaint& paint, const void* bytes, size_t byteLength, SkPoint origin,
        SkSpan<const char> text, SkSpan<const uint32_t> clusters) {

    this->addDenseAndUnique(paint, bytes, byteLength);

    fScratchAdvances.resize(fUniqueGlyphIDs.size());
    {
        auto cache = SkStrikeCache::FindOrCreateStrikeExclusive(paint);
        cache->getAdvances(SkSpan<const SkGlyphID>{fUniqueGlyphIDs}, fScratchAdvances.data());
    }

    SkPoint endOfLastGlyph = origin;

    for (size_t i = 0; i < fDenseIndex.size(); i++) {
        fPositions.push_back(endOfLastGlyph);
        endOfLastGlyph += fScratchAdvances[fDenseIndex[i]];
    }

    if (paint.getTextAlign() != SkPaint::kLeft_Align) {
        SkVector len = endOfLastGlyph - origin;
        if (paint.getTextAlign() == SkPaint::kCenter_Align) {
            len.scale(SK_ScalarHalf);
        }
        for (size_t i = 0; i < fDenseIndex.size(); i++) {
            fPositions[i] -= len;
        }
    }

    this->makeGlyphRun(paint, text, clusters);
}

void SkGlyphRunBuilder::drawPosTextH(const SkPaint& paint, const void* bytes,
                                     size_t byteLength, const SkScalar* xpos,
                                     SkScalar constY,
                                     SkSpan<const char> text, SkSpan<const uint32_t> clusters) {

    this->addDenseAndUnique(paint, bytes, byteLength);

    for (size_t i = 0; i < fDenseIndex.size(); i++) {
        fPositions.push_back(SkPoint::Make(xpos[i], constY));
    }

    this->makeGlyphRun(paint, text, clusters);
}

void SkGlyphRunBuilder::drawPosText(const SkPaint& paint, const void* bytes,
                                    size_t byteLength, const SkPoint* pos,
                                    SkSpan<const char> text, SkSpan<const uint32_t> clusters) {
    this->addDenseAndUnique(paint, bytes, byteLength);

    for (size_t i = 0; i < fDenseIndex.size(); i++) {
        fPositions.push_back(pos[i]);
    }

    this->makeGlyphRun(paint, text, clusters);
}


