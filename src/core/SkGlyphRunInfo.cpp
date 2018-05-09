/*
 * Copyright 2018 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include <algorithm>

#include "SkGlyphRunInfo.h"

#include "SkDraw.h"
#include "SkPaint.h"
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
    SkTypeface::Encoding typefaceEncoding;
    switch (encoding) {
        case  SkPaint::kUTF8_SkTextEncoding: return SkTypeface::kUTF8_Encoding;
        case SkPaint::kUTF16_SkTextEncoding: return SkTypeface::kUTF16_Encoding;
        case SkPaint::kUTF32_SkTextEncoding: return SkTypeface::kUTF32_Encoding;
    }
}

}  // namespace

SkGlyphRunInfo Make(const SkPaint& paint, const void* bytes, size_t byteLength) {
    SkGlyphRunInfo glyphRunInfo;
    auto encoding = paint.getTextEncoding();
    if (encoding == SkPaint::kGlyphID_TextEncoding) {
        return SkGlyphRunInfo::Make(
            byteLength / 2, reinterpret_cast<const SkGlyphID*>(bytes));
    } else {
        return SkGlyphRunInfo::Make(
            byteLength, bytes, *paint.getTypeface(), convert_encoding(encoding));
    }
}

SkGlyphRunInfo SkGlyphRunInfo::Make(uint16_t count, const SkGlyphID* glyphs) {
    std::vector<GlyphIndex> glyphIndex(count);

    for (uint16_t i = 0; i < count; i++) {
        glyphIndex[i] = GlyphIndex{glyphs[i], i};
    }

    std::sort(glyphIndex.begin(), glyphIndex.end());

    std::vector<DenseIndex> denseIndex(count);
    std::vector<DrawableGlyph> unique;

    SkGlyphID last = 0;
    uint16_t uniqueIndex = 0;
    for (auto& gi : glyphIndex) {
        if (last != gi.glyph) {
            unique.emplace_back(gi.glyph);
            last = gi.glyph;
            uniqueIndex = SkTo<uint16_t>(unique.size() - 1);
        }
        denseIndex[gi.index] = uniqueIndex;
    }

    return SkGlyphRunInfo{std::move(denseIndex), std::move(unique)};
}

SkGlyphRunInfo SkGlyphRunInfo::Make(size_t byteLength, const void* utfN,
                                    const SkTypeface& typeface, SkTypeface::Encoding encoding) {
    SkGlyphRunInfo info;
    auto count = SkUTFN_CountUnichars(encoding, utfN, byteLength);
    if (count > 0) {
        SkASSERT(count < std::numeric_limits<uint16_t>::max());

        std::vector<UnicharIndex> unicharIndex(count);

        auto add = [&unicharIndex](size_t index, SkUnichar c) {
            unicharIndex[index] = UnicharIndex{c, SkTo<uint16_t>(index)};
        };

        for_each_unichar(utfN, byteLength, encoding, add);

        std::sort(unicharIndex.begin(), unicharIndex.end());

        std::vector<DenseIndex> denseIndex(count);
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
            unicharIndex.data(), SkTypeface::kUTF32_Encoding, glyphs.data(), uniqueUnichar.size());

        std::vector<DrawableGlyph> uniqueGlyphData;

        for (auto glyph : glyphs) {
            uniqueGlyphData.emplace_back(glyph);
        }

        info = SkGlyphRunInfo{std::move(denseIndex), std::move(uniqueGlyphData)};
    }

    return info;
}

void SkGlyphRunInfo::preparePositions(
    const SkPoint* pos, const SkPaint& paint, const SkMatrix& ctm, const SkSurfaceProps* props,
    SkScalerContextFlags scFlags) {
    SkMatrix adjustedCtm;

    SkPaint drawPaint(paint);
    if (SkDraw::ShouldDrawTextAsPaths(paint, ctm)) {

    }

}

SkGlyphRunInfo::SkGlyphRunInfo(
    std::vector<SkGlyphRunInfo::DenseIndex>&& denseIndex,
    std::vector<DrawableGlyph>&& unique)
    : fDenseIndex{std::move(denseIndex)}, fUnique{std::move(unique)} { }