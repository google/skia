/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextBlobCache.h"

static const int kVerticesPerGlyph = 4;

GrTextBlobCache::~GrTextBlobCache() {
    this->freeAll();
}

GrAtlasTextContext::BitmapTextBlob* GrTextBlobCache::createBlob(int glyphCount, int runCount,
                                                                size_t maxVASize) {
    // We allocate size for the BitmapTextBlob itself, plus size for the vertices array,
    // and size for the glyphIds array.
    size_t verticesCount = glyphCount * kVerticesPerGlyph * maxVASize;
    size_t size = sizeof(BitmapTextBlob) +
                  verticesCount +
                  glyphCount * sizeof(GrGlyph**) +
                  sizeof(BitmapTextBlob::Run) * runCount;

    BitmapTextBlob* cacheBlob = SkNEW_PLACEMENT(fPool.allocate(size), BitmapTextBlob);

    // setup offsets for vertices / glyphs
    cacheBlob->fVertices = sizeof(BitmapTextBlob) + reinterpret_cast<unsigned char*>(cacheBlob);
    cacheBlob->fGlyphs = reinterpret_cast<GrGlyph**>(cacheBlob->fVertices + verticesCount);
    cacheBlob->fRuns = reinterpret_cast<BitmapTextBlob::Run*>(cacheBlob->fGlyphs + glyphCount);

    // Initialize runs
    for (int i = 0; i < runCount; i++) {
        SkNEW_PLACEMENT(&cacheBlob->fRuns[i], BitmapTextBlob::Run);
    }
    cacheBlob->fRunCount = runCount;
    cacheBlob->fPool = &fPool;
    return cacheBlob;
}

void GrTextBlobCache::freeAll() {
    SkTDynamicHash<BitmapTextBlob, BitmapTextBlob::Key>::Iter iter(&fCache);
    while (!iter.done()) {
        (&(*iter))->unref();
        ++iter;
    }
    fCache.rewind();
}
