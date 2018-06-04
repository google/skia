/*
 * Copyright 2018 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGlyphRun.h"

#include <algorithm>
#include <tuple>

#include "SkDraw.h"
#include "SkGlyphCache.h"
#include "SkMakeUnique.h"
#include "SkMSAN.h"
#include "SkPaint.h"
#include "SkPaintPriv.h"
#include "SkStrikeCache.h"
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

using Core = std::tuple<size_t,   std::unique_ptr<uint16_t[]>, std::vector<SkGlyphID>>;

Core make_from_glyphids(
        size_t glyphCount, const SkGlyphID* glyphs, SkGlyphID maxGlyphID, SkGlyphSet* glyphSet) {
    if (glyphCount == 0) { return Core(0, nullptr, std::vector<SkGlyphID>()); }

    glyphSet->reuse(maxGlyphID);

    auto denseIndex = skstd::make_unique_default<uint16_t[]>(glyphCount);
    for (size_t i = 0; i < glyphCount; i++) {
        denseIndex[i] = glyphSet->add(glyphs[i]);
    }

    return Core(glyphCount, std::move(denseIndex), glyphSet->uniqueGlyphIDs());
}

Core make_from_utfn(size_t byteLength, const void* utfN, const SkTypeface& typeface,
                    SkTypeface::Encoding encoding, SkGlyphSet* glyphSet) {
    auto count = SkUTFN_CountUnichars(encoding, utfN, byteLength);

    if (count <= 0) {
        return Core(0, nullptr, std::vector<SkGlyphID>());
    }

    auto glyphs = skstd::make_unique_default<SkGlyphID[]>(count);

    // TODO: move to using cached version.
    typeface.charsToGlyphs(utfN, encoding, glyphs.get(), count);

    return make_from_glyphids(count, glyphs.get(), typeface.countGlyphs(), glyphSet);
}

Core make_core(const SkPaint& paint, const void* bytes, size_t byteLength, SkGlyphSet* glyphSet) {
    auto encoding = paint.getTextEncoding();
    auto typeface = SkPaintPriv::GetTypefaceOrDefault(paint);
    if (encoding == SkPaint::kGlyphID_TextEncoding) {
        return make_from_glyphids(
                byteLength / 2, reinterpret_cast<const SkGlyphID*>(bytes),
                typeface->countGlyphs(), glyphSet);
    } else {
        return make_from_utfn(byteLength, bytes, *typeface, convert_encoding(encoding), glyphSet);
    }
}

}  // namespace

SkGlyphRun SkGlyphRun::MakeFromDrawText(
        const SkPaint& paint, const void* bytes, size_t byteLength,
        const SkPoint origin, SkGlyphSet* glyphSet) {
    size_t runSize;
    std::unique_ptr<uint16_t[]> denseIndex;
    std::vector<SkGlyphID> uniqueGlyphIDs;
    std::tie(runSize, denseIndex, uniqueGlyphIDs) = make_core(paint, bytes, byteLength, glyphSet);

    if (runSize == 0) { return SkGlyphRun{}; }

    auto advances = skstd::make_unique_default<SkPoint[]>(uniqueGlyphIDs.size());

    {
        auto cache = SkStrikeCache::FindOrCreateStrikeExclusive(paint);
        cache->getAdvances(SkSpan<SkGlyphID>{uniqueGlyphIDs.data(),
                                             uniqueGlyphIDs.size()}, advances.get());
    }

    auto positions = skstd::make_unique_default<SkPoint[]>(runSize);

    SkPoint endOfLastGlyph = origin;

    for (size_t i = 0; i < runSize; i++) {
        positions[i] = endOfLastGlyph;
        endOfLastGlyph += advances[denseIndex[i]];
    }

    if (paint.getTextAlign() != SkPaint::kLeft_Align) {
        SkVector len = endOfLastGlyph - origin;
        if (paint.getTextAlign() == SkPaint::kCenter_Align) {
            len.scale(SK_ScalarHalf);
        }
        for (size_t i = 0; i < runSize; i++) {
            positions[i] -= len;
        }
    }

    return SkGlyphRun{
        runSize, std::move(denseIndex), std::move(positions), std::move(uniqueGlyphIDs)};
}

SkGlyphRun SkGlyphRun::MakeFromDrawPosTextH(
        const SkPaint& paint, const void* bytes, size_t byteLength,
        const SkScalar xpos[], SkScalar constY, SkGlyphSet* glyphSet) {
    size_t runSize;
    std::unique_ptr<uint16_t[]> denseIndex;
    std::vector<SkGlyphID> uniqueGlyphIDs;
    std::tie(runSize, denseIndex, uniqueGlyphIDs) = make_core(paint, bytes, byteLength, glyphSet);

    if (runSize == 0) { return SkGlyphRun{}; }

    auto positions = skstd::make_unique_default<SkPoint[]>(runSize);

    for (size_t i = 0; i < runSize; i++) {
        positions[i] = SkPoint::Make(xpos[i], constY);
    }

    return SkGlyphRun{
        runSize, std::move(denseIndex), std::move(positions), std::move(uniqueGlyphIDs)};
}

SkGlyphRun SkGlyphRun::MakeFromDrawPosText(
        const SkPaint& paint, const void* bytes, size_t byteLength,
        const SkPoint pos[], SkGlyphSet* glyphSet) {
    size_t runSize;
    std::unique_ptr<uint16_t[]> denseIndex;
    std::vector<SkGlyphID> uniqueGlyphIDs;
    std::tie(runSize, denseIndex, uniqueGlyphIDs) = make_core(paint, bytes, byteLength, glyphSet);

    if (runSize == 0) { return SkGlyphRun{}; }

    auto positions = skstd::make_unique_default<SkPoint[]>(runSize);

    memcpy(positions.get(), pos, sizeof(SkPoint) * runSize);

    return SkGlyphRun{
        runSize, std::move(denseIndex), std::move(positions), std::move(uniqueGlyphIDs)};
}

std::unique_ptr<SkGlyphID[]> SkGlyphRun::copyGlyphIDs() const {
    auto glyphs = skstd::make_unique_default<SkGlyphID[]>(fRunSize);

    for (size_t i = 0; i < fRunSize; i++) {
        glyphs[i] = fUniqueGlyphs[fDenseIndex[i]];
    }

    return glyphs;
}

SkGlyphRun::SkGlyphRun(size_t runSize,
                       std::unique_ptr<uint16_t[]>&& denseIndex,
                       std::unique_ptr<SkPoint[]>&& positions,
                       std::vector<SkGlyphID>&& uniqueGlyphIDs)
    : fDenseIndex{std::move(denseIndex)}
    , fPositions{std::move(positions)}
    , fUniqueGlyphs{std::move(uniqueGlyphIDs)}
    , fRunSize{runSize} { }

uint16_t SkGlyphSet::add(SkGlyphID glyphID) {
    static constexpr SkGlyphID  kUndefGlyph{0};

    if (glyphID >= fUniverseSize) {
        glyphID = kUndefGlyph;
    }

    if (glyphID >= fIndices.size()) {
        fIndices.resize(glyphID + 1);
    }

    auto index = fIndices[glyphID];
    if (index < fUniqueGlyphIDs.size() && fUniqueGlyphIDs[index] == glyphID) {
        return index;
    }

    uint16_t newIndex = SkTo<uint16_t>(fUniqueGlyphIDs.size());
    fUniqueGlyphIDs.push_back(glyphID);
    fIndices[glyphID] = newIndex;
    return newIndex;
}

std::vector<SkGlyphID> SkGlyphSet::uniqueGlyphIDs() {
    return fUniqueGlyphIDs;
}

void SkGlyphSet::reuse(uint32_t glyphUniverseSize) {
    SkASSERT(glyphUniverseSize <= (1 << 16));
    fUniverseSize = glyphUniverseSize;
    // If we're hanging onto these arrays for a long time, we don't want their size to drift
    // endlessly upwards. It's unusual to see more than 256 unique glyphs used in a run,
    // or a typeface with more than 4096 possible glyphs.
    if (fUniqueGlyphIDs.size() > 256) {
        fUniqueGlyphIDs.resize(256);
        fUniqueGlyphIDs.shrink_to_fit();
    }
    fUniqueGlyphIDs.clear();

    if (glyphUniverseSize < 4096 && fIndices.size() > 4096) {
        fIndices.resize(4096);
        fIndices.shrink_to_fit();
    }

    // No need to clear fIndices here... SkGlyphSet's set insertion algorithm is designed to work
    // correctly even when the fIndexes buffer is uninitialized!
}