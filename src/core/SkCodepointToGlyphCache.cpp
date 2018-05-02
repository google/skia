/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCodepointToGlyphCache.h"

#include <algorithm>
#include <vector>

#include "SkMakeUnique.h"
#include "SkMutex.h"
#include "SkUtils.h"

SkCodepointToGlyphCache::SkCodepointToGlyphCache(SkTypeface* typeface)
    : fTypeface{typeface} { }

size_t SkCodepointToGlyphCache::utfNToGlyphs(
    const void *text, size_t bytes, SkTypeface::Encoding encoding, uint16_t glyphs[])
{
    // Just no.
    if (bytes > std::numeric_limits<uint32_t>::max()) {
        return 0;
    }

    size_t glyphCount = 0;

    struct Missing {
        uint32_t codepoint;
        uint32_t position;
    };

    struct MissingLess {
        bool operator()(const Missing &a, const Missing &b) {
            return a.codepoint < b.codepoint;
        }
    };

    std::vector<Missing> missing;

    fMu.acquireShared();
    if (fMap.count() > 0) {
        auto convert = [&glyphs, &missing, this](size_t i, uint32_t c) {
            if (auto glyph = fMap.find(c)) {
                glyphs[i] = *glyph;
            } else {
                missing.push_back(Missing{c, SkTo<uint32_t>(i)});
            }
        };
        glyphCount = SkParseUnicode(text, bytes, encoding, convert);
        fMu.releaseShared();
    } else {
        // Can release the lock early for first time.
        fMu.releaseShared();
        auto convert = [&missing](size_t i, uint32_t c) {
            missing.emplace_back(Missing{c, SkTo<uint32_t>(i)});
        };
        glyphCount = SkParseUnicode(text, bytes, encoding, convert);
    }

    if (missing.empty()) {
        return glyphCount;
    } else {
        // Sort to get the codepoints in order.
        std::sort(missing.begin(), missing.end(), MissingLess());

        // Find all the missing pairs, and fill in the glyphs using positions.
        uint32_t lastCodepoint = std::numeric_limits<uint32_t>::max();
        uint16_t glyph = 0;
        for (auto& m : missing) {
            if (m.codepoint != lastCodepoint) {
                glyph = fTypeface->codepointToGlyph(m.codepoint);
                SkAutoMutexAcquire l(fMu);
                fMap.set(m.codepoint, glyph);
            }
            glyphs[m.position] = glyph;
        }
    }

    return glyphCount;
}
