/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkClusterator_DEFINED
#define SkClusterator_DEFINED

#include <cstdint>

namespace sktext {
class GlyphRun;
}

/** Given the m-to-n glyph-to-character mapping data (as returned by
    harfbuzz), iterate over the clusters. */
class SkClusterator {
public:
    SkClusterator(const sktext::GlyphRun& run);
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
    uint32_t const * const fClusters;
    char const * const fUtf8Text;
    uint32_t const fGlyphCount;
    uint32_t const fTextByteLength;
    bool const fReversedChars;
    uint32_t fCurrentGlyphIndex = 0;
};
#endif  // SkClusterator_DEFINED
