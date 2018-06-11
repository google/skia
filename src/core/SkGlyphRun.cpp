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
#include "SkMakeUnique.h"
#include "SkMSAN.h"
#include "SkPaint.h"
#include "SkPaintPriv.h"
#include "SkStrikeCache.h"
#include "SkUtils.h"

static SkTypeface::Encoding convert_encoding(SkPaint::TextEncoding encoding) {
    switch (encoding) {
        case  SkPaint::kUTF8_TextEncoding: return SkTypeface::kUTF8_Encoding;
        case SkPaint::kUTF16_TextEncoding: return SkTypeface::kUTF16_Encoding;
        case SkPaint::kUTF32_TextEncoding: return SkTypeface::kUTF32_Encoding;
        default: return SkTypeface::kUTF32_Encoding;
    }
}

// -- SkGlyphSet ----------------------------------------------------------------------------------
uint16_t SkGlyphSet::add(SkGlyphID glyphID) {
    static constexpr SkGlyphID  kUndefGlyph{0};

    if (glyphID >= fUniverseSize) {
        glyphID = kUndefGlyph;
    }

    if (glyphID >= fIndices.size()) {
        fIndices.resize(glyphID + 1);
    }

    auto index = fIndices[glyphID];
    if (index < fUniqueGlyphIDs->size() && (*fUniqueGlyphIDs)[index] == glyphID) {
        return index;
    }

    uint16_t newIndex = SkTo<uint16_t>(fUniqueGlyphIDs->size());
    fUniqueGlyphIDs->push_back(glyphID);
    fIndices[glyphID] = newIndex;
    return newIndex;
}

void SkGlyphSet::reuse(uint32_t glyphUniverseSize, std::vector<SkGlyphID>* uniqueGlyphIDs) {
    SkASSERT(glyphUniverseSize <= (1 << 16));
    fUniverseSize = glyphUniverseSize;
    fUniqueGlyphIDs = uniqueGlyphIDs;
    // If we're hanging onto these arrays for a long time, we don't want their size to drift
    // endlessly upwards. It's unusual to see more than 256 unique glyphs used in a run,
    // or a typeface with more than 4096 possible glyphs.
    if (fUniqueGlyphIDs->size() > 256) {
        fUniqueGlyphIDs->resize(256);
        fUniqueGlyphIDs->shrink_to_fit();
    }
    fUniqueGlyphIDs->clear();

    if (glyphUniverseSize < 4096 && fIndices.size() > 4096) {
        fIndices.resize(4096);
        fIndices.shrink_to_fit();
    }

    // No need to clear fIndices here... SkGlyphSet's set insertion algorithm is designed to work
    // correctly even when the fIndexes buffer is uninitialized!
}

// -- SkGlyphRunBuilder ----------------------------------------------------------------------------
void SkGlyphRunBuilder::prepareDrawText(
        const SkPaint& paint, const void* bytes, size_t byteLength, SkPoint origin) {

    this->initializeDenseAndUnique(paint, bytes, byteLength);

    fScratchAdvances.resize(this->uniqueSize());
    {
        auto cache = SkStrikeCache::FindOrCreateStrikeExclusive(paint);
        cache->getAdvances(SkSpan<SkGlyphID>{fUniqueGlyphs}, fScratchAdvances.data());
    }

    SkPoint endOfLastGlyph = origin;

    for (size_t i = 0; i < this->runSize(); i++) {
        fPositions.push_back(endOfLastGlyph);
        endOfLastGlyph += fScratchAdvances[fDenseIndex[i]];
    }

    if (paint.getTextAlign() != SkPaint::kLeft_Align) {
        SkVector len = endOfLastGlyph - origin;
        if (paint.getTextAlign() == SkPaint::kCenter_Align) {
            len.scale(SK_ScalarHalf);
        }
        for (size_t i = 0; i < this->runSize(); i++) {
            fPositions[i] -= len;
        }
    }

}

void SkGlyphRunBuilder::prepareDrawPosTextH(const SkPaint& paint, const void* bytes,
                                            size_t byteLength, const SkScalar* xpos,
                                            SkScalar constY) {

    this->initializeDenseAndUnique(paint, bytes, byteLength);

    for (size_t i = 0; i < runSize(); i++) {
        fPositions.push_back(SkPoint::Make(xpos[i], constY));
    }
}

void SkGlyphRunBuilder::prepareDrawPosText(const SkPaint& paint, const void* bytes,
                                           size_t byteLength, const SkPoint* pos) {
    this->initializeDenseAndUnique(paint, bytes, byteLength);

    for (size_t i = 0; i < runSize(); i++) {
        fPositions.push_back(pos[i]);
    }
}

const SkGlyphRun& SkGlyphRunBuilder::useGlyphRun() const {
    new ((void*)&fScratchGlyphRun) SkGlyphRun{SkSpan<uint16_t>(fDenseIndex),
                                       SkSpan<SkPoint>(fPositions),
                                       SkSpan<SkGlyphID>(fUniqueGlyphs)};
    return fScratchGlyphRun;
}

void SkGlyphRunBuilder::temporaryShuntToDrawPosText(const SkPaint& paint, SkBaseDevice* device) {

    auto pos = (const SkScalar*) fPositions.data();

    device->drawPosText(
            fTemporaryShuntGlyphIDs, fDenseIndex.size() * 2,
            pos, 2, SkPoint::Make(0, 0), paint);
}

void SkGlyphRunBuilder::temporaryShuntToCallback(TemporaryShuntCallback callback) {
    auto bytes = (const char *)fTemporaryShuntGlyphIDs;
    auto pos = (const SkScalar*)fPositions.data();
    callback(this->runSize(), bytes, pos);
}

void SkGlyphRunBuilder::initializeDenseAndUnique(
        const SkPaint& paint, const void* bytes, size_t byteLength) {

    fDenseIndex.clear();
    fPositions.clear();
    fUniqueGlyphs.clear();
    fTemporaryShuntGlyphIDs = nullptr;

    size_t runSize = 0;
    const SkGlyphID* glyphIDs = nullptr;
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
        glyphIDs = (const SkGlyphID*)bytes;
    }

    if (runSize == 0) { return; }
    fTemporaryShuntGlyphIDs = glyphIDs;

    fGlyphSet.reuse(typeface->countGlyphs(), &fUniqueGlyphs);
    for (size_t i = 0; i < runSize; i++) {
        fDenseIndex.push_back(fGlyphSet.add(glyphIDs[i]));
    }
}


