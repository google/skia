/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextBlobCache.h"

GrTextBlobCache::~GrTextBlobCache() {
    this->freeAll();
}

void GrTextBlobCache::freeAll() {
    SkTDynamicHash<GrAtlasTextBlob, GrAtlasTextBlob::Key>::Iter iter(&fCache);
    while (!iter.done()) {
        GrAtlasTextBlob* blob = &(*iter);
        fBlobList.remove(blob);
        blob->unref();
        ++iter;
    }
    fCache.rewind();

    // There should be no allocations in the memory pool at this point
    SkASSERT(fPool.isEmpty());
}
