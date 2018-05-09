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

// A faster set implementation that does not need any initialization, and reading the set items
// is order the number of items, and not the size of the universe.
// This implementation is based on the paper by Briggs and Torczon, "An Efficient Representation
// for Sparse Sets"
class GlyphSet {
public:
    GlyphSet(uint32_t glyphUniverseSize)
    : fUniverseSize{glyphUniverseSize}
    , fIndexes{skstd::make_unique_default<uint16_t[]>(2 * glyphUniverseSize)}
    , fUniqueGlyphIDs{&fIndexes[glyphUniverseSize]} {
        SkASSERT(glyphUniverseSize <= (1 << 16));
        sk_msan_mark_initialized(fIndexes.get(), &fIndexes[glyphUniverseSize], "works with uninited");
    }

    uint16_t add(SkGlyphID glyphID) {
        if (glyphID >= fUniverseSize) {
            glyphID = kUndefGlyph;
        }
        auto index = fIndexes[glyphID];
        if (index < fUniqueCount && fUniqueGlyphIDs[index] == glyphID) {
            return index;
        }

        fUniqueGlyphIDs[fUniqueCount] = glyphID;
        fIndexes[glyphID] = fUniqueCount;
        fUniqueCount += 1;
        return fUniqueCount - 1;
    }

    std::tuple<uint16_t, std::unique_ptr<SkGlyphID[]>> uniqueGlyphIDs() const {
        auto uniqueGlyphs = skstd::make_unique_default<SkGlyphID[]>(fUniqueCount);
        memcpy(uniqueGlyphs.get(), fUniqueGlyphIDs, fUniqueCount * sizeof(SkGlyphID));
        return std::make_tuple(fUniqueCount, std::move(uniqueGlyphs));
    }

private:
    static constexpr SkGlyphID  kUndefGlyph{0};
    const uint32_t              fUniverseSize;
    uint16_t                    fUniqueCount{0};
    std::unique_ptr<uint16_t[]> fIndexes;
    SkGlyphID*                  fUniqueGlyphIDs;
 };

template<typename T>
bool is_aligned(const void* ptr) {
    uintptr_t bits = reinterpret_cast<uintptr_t>(ptr);
    return (bits & (alignof(T) - 1)) == 0;
}

template<typename T>
bool is_aligned_size(size_t size) {
    return size % sizeof(T) == 0;
}

SkTypeface::Encoding convert_encoding(SkPaint::TextEncoding encoding) {
    switch (encoding) {
        case  SkPaint::kUTF8_TextEncoding: return SkTypeface::kUTF8_Encoding;
        case SkPaint::kUTF16_TextEncoding: return SkTypeface::kUTF16_Encoding;
        case SkPaint::kUTF32_TextEncoding: return SkTypeface::kUTF32_Encoding;
        default: return SkTypeface::kUTF32_Encoding;
    }
}

using Core = std::tuple<size_t,   std::unique_ptr<uint16_t[]>,
                        uint16_t, std::unique_ptr<SkGlyphID[]>>;

Core make_from_glyphids(size_t glyphCount, const SkGlyphID* glyphs, SkGlyphID maxGlyphID) {
    if (glyphCount == 0) { return Core(0, nullptr, 0, nullptr); }

    GlyphSet glyphSet{maxGlyphID};

    auto denseIndex = skstd::make_unique_default<uint16_t[]>(glyphCount);
    for (size_t i = 0; i < glyphCount; i++) {
        denseIndex[i] = glyphSet.add(glyphs[i]);
    }

    std::unique_ptr<SkGlyphID[]> uniqueGlyphIDs;
    uint16_t uniqueCount;
    std::tie(uniqueCount, uniqueGlyphIDs) = glyphSet.uniqueGlyphIDs();

    return Core(glyphCount, std::move(denseIndex), uniqueCount, std::move(uniqueGlyphIDs));
}

Core make_from_utfn(size_t byteLength, const void* utfN, const SkTypeface& typeface,
                    SkTypeface::Encoding encoding) {
    auto count = SkUTFN_CountUnichars(encoding, utfN, byteLength);

    if (count <= 0) {
        return Core(0, nullptr, 0, nullptr);
    }

    auto glyphs = skstd::make_unique_default<SkGlyphID[]>(count);

    // TODO: move to using cached version.
    typeface.charsToGlyphs(utfN, encoding, glyphs.get(), count);

    return make_from_glyphids(count, glyphs.get(), typeface.countGlyphs());
}

Core make_core(const SkPaint& paint, const void* bytes, size_t byteLength) {
    auto encoding = paint.getTextEncoding();
    auto typeface = SkPaintPriv::GetTypefaceOrDefault(paint);
    if (encoding == SkPaint::kGlyphID_TextEncoding) {
        return make_from_glyphids(
                byteLength / 2, reinterpret_cast<const SkGlyphID*>(bytes), typeface->countGlyphs());
    } else {
        return make_from_utfn(byteLength, bytes, *typeface, convert_encoding(encoding));
    }
}

}  // namespace

SkGlyphRun SkGlyphRun::MakeFromDrawText(
        const SkPaint& paint, const void* bytes, size_t byteLength,
        const SkPoint origin) {
    size_t runSize;
    std::unique_ptr<uint16_t[]> denseIndex;
    uint16_t uniqueSize;
    std::unique_ptr<SkGlyphID[]> uniqueGlyphIDs;
    std::tie(runSize, denseIndex, uniqueSize, uniqueGlyphIDs) = make_core(paint, bytes, byteLength);

    if (runSize == 0) { return SkGlyphRun{}; }

    auto advances = skstd::make_unique_default<SkPoint[]>(uniqueSize);

    {
        auto cache = SkStrikeCache::FindOrCreateStrikeExclusive(paint);
        cache->getAdvances(SkSpan<SkGlyphID>{uniqueGlyphIDs.get(), uniqueSize}, advances.get());
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
        runSize, std::move(denseIndex), std::move(positions), uniqueSize, std::move(uniqueGlyphIDs)};
}

SkGlyphRun SkGlyphRun::MakeFromDrawPosTextH(
        const SkPaint& paint, const void* bytes, size_t byteLength,
        const SkScalar xpos[], SkScalar constY) {
    size_t runSize;
    std::unique_ptr<uint16_t[]> denseIndex;
    uint16_t uniqueSize;
    std::unique_ptr<SkGlyphID[]> uniqueGlyphIDs;
    std::tie(runSize, denseIndex, uniqueSize, uniqueGlyphIDs) = make_core(paint, bytes, byteLength);

    if (runSize == 0) { return SkGlyphRun{}; }

    auto positions = skstd::make_unique_default<SkPoint[]>(runSize);

    for (size_t i = 0; i < runSize; i++) {
        positions[i] = SkPoint::Make(xpos[i], constY);
    }

    return SkGlyphRun{
        runSize, std::move(denseIndex), std::move(positions), uniqueSize, std::move(uniqueGlyphIDs)};
}

SkGlyphRun SkGlyphRun::MakeFromDrawPosText(
        const SkPaint& paint, const void* bytes, size_t byteLength,
        const SkPoint pos[]) {
    size_t runSize;
    std::unique_ptr<uint16_t[]> denseIndex;
    uint16_t uniqueSize;
    std::unique_ptr<SkGlyphID[]> uniqueGlyphIDs;
    std::tie(runSize, denseIndex, uniqueSize, uniqueGlyphIDs) = make_core(paint, bytes, byteLength);

    if (runSize == 0) { return SkGlyphRun{}; }

    auto positions = skstd::make_unique_default<SkPoint[]>(runSize);

    memcpy(positions.get(), pos, sizeof(SkPoint) * runSize);

    return SkGlyphRun{
        runSize, std::move(denseIndex), std::move(positions), uniqueSize, std::move(uniqueGlyphIDs)};
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
                       uint16_t uniqueSize,
                       std::unique_ptr<SkGlyphID[]>&& uniqueGlyphIDs)
    : fDenseIndex{std::move(denseIndex)}
    , fPositions{std::move(positions)}
    , fUniqueGlyphs{std::move(uniqueGlyphIDs)}
    , fRunSize{runSize}
    , fUniqueSize{uniqueSize} { }
