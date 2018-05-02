/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCodepointToGlyphCache_DEFINED
#define SkCodepointToGlyphCache_DEFINED

#include "SkSharedMutex.h"
#include "SkTHash.h"
#include "SkTypeface.h"

class SkCodepointToGlyphCache {
public:
    SkCodepointToGlyphCache(SkTypeface*);

    size_t utfNToGlyphs(
        const void *text,
        size_t bytes,
        SkTypeface::Encoding encoding,
        uint16_t glyphs[]);

private:
    SkTypeface* const fTypeface;
    SkSharedMutex fMu;
    SkTHashMap<uint32_t, uint16_t> fMap;
};

#endif  // SkCodepointToGlyphCache_DEFINED
