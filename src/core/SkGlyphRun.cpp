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
#include "SkPaint.h"
#include "SkPaintPriv.h"
#include "SkStrikeCache.h"
#include "SkUtils.h"

namespace {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wuninitialized"
struct GlyphIndex {
    GlyphIndex() : glyph(glyph), index(index) {}
    GlyphIndex(SkGlyphID glyph_, uint16_t index_) : glyph(glyph_), index(index_) {}
    SkGlyphID glyph;
    uint16_t index;
    constexpr bool operator < (const GlyphIndex& rhs) const { return glyph < rhs.glyph; }
};

struct UnicharIndex {
    UnicharIndex() : unichar(unichar), index(index) {}
    UnicharIndex(SkUnichar unichar_, uint16_t index_) : unichar(unichar_), index(index_) {}
    SkUnichar unichar;
    uint16_t index;
    constexpr bool operator < (const UnicharIndex& rhs) const { return unichar < rhs.unichar; }
};

struct DenseIndex {
    DenseIndex() : index(index) {}
    DenseIndex(uint16_t index_) : index(index_) {}
    operator uint16_t () const { return index; }
    uint16_t index;
};
#pragma clang diagnostic pop

template<typename T>
bool is_aligned(const void* ptr) {
    uintptr_t bits = reinterpret_cast<uintptr_t>(ptr);
    return (bits & (alignof(T) - 1)) == 0;
}

template<typename T>
bool is_aligned_size(size_t size) {
    return size % sizeof(T) == 0;
}

size_t for_each_unichar(const void* utfn,
                        size_t byteLength,
                        SkTypeface::Encoding encoding,
                        std::function<void(size_t, SkUnichar)> eachUnichar) {
    size_t unicodeSize = 0;
    switch(encoding) {
        case SkTypeface::kUTF8_Encoding:{
            auto cursor = static_cast<const char*>(utfn);
            auto end = SkTAddOffset<const char>(utfn, byteLength);
            while (cursor < end) {
                SkUnichar c = SkUTF8_NextUnicharWithError(&cursor, end);
                if (c < 0) { return 0; }
                eachUnichar(unicodeSize, c);
                unicodeSize += 1;
            }
            break;
        }
        case SkTypeface::kUTF16_Encoding: {
            if (!is_aligned<uint16_t>(utfn) || !is_aligned_size<uint16_t>(byteLength)) { return 0; }
            auto cursor = static_cast<const uint16_t*>(utfn);
            auto end = SkTAddOffset<const uint16_t>(utfn, byteLength);
            while (cursor < end) {
                // TODO: add the error version when written.
                eachUnichar(unicodeSize, SkUTF16_NextUnichar(&cursor));
                unicodeSize += 1;
            }
            break;
        }
        case SkTypeface::kUTF32_Encoding: {
            if (!is_aligned<uint32_t>(utfn) || !is_aligned_size<uint32_t>(byteLength)) { return 0; }
            auto cursor = static_cast<const uint32_t*>(utfn);
            auto end = SkTAddOffset<const uint32_t>(utfn, byteLength);
            while (cursor < end) {
                eachUnichar(unicodeSize, (SkUnichar)*cursor);
                cursor += 1;
                unicodeSize += 1;
            }
            break;
        }
    }
    return unicodeSize;
}


SkTypeface::Encoding convert_encoding(SkPaint::TextEncoding encoding) {
    switch (encoding) {
        case  SkPaint::kUTF8_TextEncoding: return SkTypeface::kUTF8_Encoding;
        case SkPaint::kUTF16_TextEncoding: return SkTypeface::kUTF16_Encoding;
        case SkPaint::kUTF32_TextEncoding: return SkTypeface::kUTF32_Encoding;
        default: return SkTypeface::kUTF32_Encoding;
    }
}

using Core = std::tuple<uint16_t, std::unique_ptr<uint16_t[]>, std::vector<SkGlyphID>>;


Core make_from_glyphids(uint16_t count, const SkGlyphID* glyphs) {
    if (count == 0) { return Core(0, nullptr, std::vector<SkGlyphID>()); }

    auto glyphIndex = skstd::make_unique_default<GlyphIndex[]>(count);

    for (uint16_t i = 0; i < count; i++) {
        glyphIndex[i] = GlyphIndex{glyphs[i], i};
    }

    std::sort(&glyphIndex[0], &glyphIndex[count]);

    auto denseIndex = skstd::make_unique_default<uint16_t[]>(count);
    std::vector<SkGlyphID> unique;

    SkGlyphID last = std::numeric_limits<SkGlyphID>::max();
    uint16_t uniqueIndex = 0;
    for (uint16_t i = 0; i < count; i++) {
        auto& gi = glyphIndex[i];
        if (last != gi.glyph) {
            unique.emplace_back(gi.glyph);
            last = gi.glyph;
            uniqueIndex = SkTo<uint16_t>(unique.size() - 1);
        }
        denseIndex[gi.index] = uniqueIndex;
    }

    return Core(count, std::move(denseIndex), std::move(unique));
}

Core make_from_utfn(size_t byteLength, const void* utfN,
                    const SkTypeface& typeface, SkTypeface::Encoding encoding) {
    auto count = SkUTFN_CountUnichars(encoding, utfN, byteLength);

    if (count <= 0) {
        return Core(0, nullptr, std::vector<SkGlyphID>());
    }

    SkASSERT(count < std::numeric_limits<uint16_t>::max());

    std::vector<UnicharIndex> unicharIndex(count);

    auto add = [&unicharIndex](size_t index, SkUnichar c) {
        unicharIndex[index] = UnicharIndex{c, SkTo<uint16_t>(index)};
    };

    for_each_unichar(utfN, byteLength, encoding, add);

    std::sort(unicharIndex.begin(), unicharIndex.end());

    auto denseIndex = skstd::make_unique_default<uint16_t[]>(count);
    std::vector<SkUnichar> uniqueUnichar;

    SkUnichar last = std::numeric_limits<SkUnichar>::max();
    uint16_t uniqueIndex = 0;
    for (auto& ui : unicharIndex) {
        if (last != ui.unichar) {
            uniqueUnichar.emplace_back(ui.unichar);
            last = ui.unichar;
            uniqueIndex = SkTo<uint16_t>(uniqueUnichar.size() - 1);
        }
        denseIndex[ui.index] = uniqueIndex;
    }

    // TODO: convert to fast resize type.
    std::vector<SkGlyphID> glyphs(uniqueUnichar.size());

    // TODO: move to using cached version.
    typeface.charsToGlyphs(
            uniqueUnichar.data(), SkTypeface::kUTF32_Encoding,
            glyphs.data(), uniqueUnichar.size());

    return Core(SkTo<uint16_t>(count), std::move(denseIndex), std::move(glyphs));
}

Core make_core(const SkPaint& paint, const void* bytes, size_t byteLength) {
    auto encoding = paint.getTextEncoding();
    if (encoding == SkPaint::kGlyphID_TextEncoding) {
        return make_from_glyphids(byteLength / 2, reinterpret_cast<const SkGlyphID*>(bytes));
    } else {
        auto typeface = SkPaintPriv::GetTypefaceOrDefault(paint);
        return make_from_utfn(byteLength, bytes, *typeface, convert_encoding(encoding));
    }
}

}  // namespace

SkGlyphRun SkGlyphRun::MakeFromDrawText(
        const SkPaint& paint, const void* bytes, size_t byteLength,
        const SkPoint origin) {
    uint16_t runSize;
    std::unique_ptr<uint16_t[]> denseIndex;
    std::vector<SkGlyphID> uniqueGlyphIDs;
    std::tie(runSize, denseIndex, uniqueGlyphIDs) = make_core(paint, bytes, byteLength);

    if (runSize == 0) { return SkGlyphRun{}; }

    auto advances = skstd::make_unique_default<SkPoint[]>(uniqueGlyphIDs.size());

    {
        auto cache = SkStrikeCache::FindOrCreateStrikeExclusive(paint);
        cache->getAdvances(SkSpan<SkGlyphID>{uniqueGlyphIDs}, advances.get());
    }

    auto positions = skstd::make_unique_default<SkPoint[]>(runSize);

    SkPoint endOfLastGlyph = origin;

    for (int i = 0; i < runSize; i++) {
        positions[i] = endOfLastGlyph;
        endOfLastGlyph += advances[denseIndex[i]];
    }

    if (paint.getTextAlign() != SkPaint::kLeft_Align) {
        SkVector len = endOfLastGlyph - origin;
        if (paint.getTextAlign() == SkPaint::kCenter_Align) {
            len.scale(SK_ScalarHalf);
        }
        for (int i = 0; i < runSize; i++) {
            positions[i] -= len;
        }
    }

    return SkGlyphRun{
        runSize, std::move(denseIndex), std::move(uniqueGlyphIDs), std::move(positions)};
}

SkGlyphRun SkGlyphRun::MakeFromDrawPosTextH(
        const SkPaint& paint, const void* bytes, size_t byteLength,
        const SkScalar xpos[], SkScalar constY) {
    uint16_t runSize;
    std::unique_ptr<uint16_t[]> denseIndex;
    std::vector<SkGlyphID> uniqueGlyphIDs;
    std::tie(runSize, denseIndex, uniqueGlyphIDs) = make_core(paint, bytes, byteLength);

    if (runSize == 0) { return SkGlyphRun{}; }

    auto positions = skstd::make_unique_default<SkPoint[]>(runSize);

    for (int i = 0; i < runSize; i++) {
        positions[i] = SkPoint::Make(xpos[i], constY);
    }

    return SkGlyphRun{
            runSize, std::move(denseIndex), std::move(uniqueGlyphIDs), std::move(positions)};
}

SkGlyphRun SkGlyphRun::MakeFromDrawPosText(
        const SkPaint& paint, const void* bytes, size_t byteLength,
        const SkPoint pos[]) {
    uint16_t runSize;
    std::unique_ptr<uint16_t[]> denseIndex;
    std::vector<SkGlyphID> uniqueGlyphIDs;
    std::tie(runSize, denseIndex, uniqueGlyphIDs) = make_core(paint, bytes, byteLength);

    if (runSize == 0) { return SkGlyphRun{}; }

    auto positions = skstd::make_unique_default<SkPoint[]>(runSize);

    memcpy(positions.get(), pos, sizeof(SkPoint) * runSize);

    return SkGlyphRun{
            runSize, std::move(denseIndex), std::move(uniqueGlyphIDs), std::move(positions)};
}

std::unique_ptr<SkGlyphID[]> SkGlyphRun::copyGlyphIDs() const {
    auto glyphs = skstd::make_unique_default<SkGlyphID[]>(fRunSize);

    for (int i = 0; i < fRunSize; i++) {
        glyphs[i] = fUniqueGlyphs[fDenseIndex[i]];
    }

    return glyphs;
}

SkGlyphRun::SkGlyphRun(uint16_t runSize,
                       std::unique_ptr<uint16_t[]>&& denseIndex,
                       std::vector<SkGlyphID>&& uniqueGlyphs,
                       std::unique_ptr<SkPoint[]>&& positions)
    : fDenseIndex{std::move(denseIndex)}
    , fUniqueGlyphs{std::move(uniqueGlyphs)}
    , fPositions{std::move(positions)}
    , fRunSize{runSize} { }

void SkGlyphRunList::add(SkGlyphRun&& run) {
    fRunList.push_back(std::move(run));
}