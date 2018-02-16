/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAtlasGlyphCache.h"

GrAtlasGlyphCache::GrAtlasGlyphCache(SkScalar glyphSizeLimit)
        : fPreserveStrike(nullptr)
        , fGlyphSizeLimit(glyphSizeLimit) {
}

GrAtlasGlyphCache::~GrAtlasGlyphCache() {
    StrikeHash::Iter iter(&fCache);
    while (!iter.done()) {
        (*iter).fIsAbandoned = true;
        (*iter).unref();
        ++iter;
    }
}

void GrAtlasGlyphCache::freeAll() {
    StrikeHash::Iter iter(&fCache);
    while (!iter.done()) {
        (*iter).fIsAbandoned = true;
        (*iter).unref();
        ++iter;
    }
    fCache.rewind();
}

void GrAtlasGlyphCache::HandleEviction(GrDrawOpAtlas::AtlasID id, void* ptr) {
    GrAtlasGlyphCache* fontCache = reinterpret_cast<GrAtlasGlyphCache*>(ptr);

    StrikeHash::Iter iter(&fontCache->fCache);
    for (; !iter.done(); ++iter) {
        GrAtlasTextStrike* strike = &*iter;
        strike->removeID(id);

        // clear out any empty strikes.  We will preserve the strike whose call to addToAtlas
        // triggered the eviction
        if (strike != fontCache->fPreserveStrike && 0 == strike->fAtlasedGlyphs) {
            fontCache->fCache.remove(GrAtlasTextStrike::GetKey(*strike));
            strike->fIsAbandoned = true;
            strike->unref();
        }
    }
}

