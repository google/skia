/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkClusterator_DEFINED
#define SkClusterator_DEFINED

#include <vector>

#include "SkTypes.h"
#include "SkPaint.h"

/** Given the m-to-n glyph-to-character mapping data (as returned by
    harfbuzz), iterate over the clusters. */
class SkClusterator {
public:
    SkClusterator(const void* sourceText,
                  size_t sourceByteCount,
                  const SkPaint& paint,
                  const uint32_t* clusters,
                  uint32_t utf8TextByteLength,
                  const char* utf8Text);
    const SkGlyphID* glyphs() const { return fGlyphs; }
    uint32_t glyphCount() const { return fGlyphCount; }
    bool reversedChars() const { return fReversedChars; }
    struct Cluster {
        const char* fUtf8Text;
        uint32_t fTextByteLength;
        uint32_t fGlyphIndex;
        uint32_t fGlyphCount;
        explicit operator bool() const { return fGlyphCount != 0; }
        bool operator==(const SkClusterator::Cluster& o) {
            return fUtf8Text       == o.fUtf8Text
                && fTextByteLength == o.fTextByteLength
                && fGlyphIndex     == o.fGlyphIndex
                && fGlyphCount     == o.fGlyphCount;
        }

    };
    Cluster next();

private:
    std::vector<SkGlyphID> fGlyphStorage;
    std::vector<char> fUtf8textStorage;
    std::vector<uint32_t> fClusterStorage;
    const SkGlyphID* fGlyphs;
    const uint32_t* fClusters;
    const char* fUtf8Text;
    uint32_t fGlyphCount;
    uint32_t fTextByteLength;
    uint32_t fCurrentGlyphIndex = 0;
    bool fReversedChars = false;
};


#endif  // SkClusterator_DEFINED
