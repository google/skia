/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextureStripAtlas.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrDynamicTextureStripAtlas.h"
#include "SkBitmap.h"

////////////////////////////////////////////////////////////////////////////////
GrTextureStripAtlasManager::~GrTextureStripAtlasManager() {
    this->deleteAllAtlases();
}

void GrTextureStripAtlasManager::deleteAllAtlases() {
    AtlasHash::Iter iter(&fAtlasCache);
    while (!iter.done()) {
        AtlasEntry* tmp = &(*iter);
        ++iter;
        delete tmp;
    }
    fAtlasCache.reset();
}

void GrTextureStripAtlasManager::abandon() {
    this->deleteAllAtlases();
}

sk_sp<GrTextureStripAtlas> GrTextureStripAtlasManager::refAtlas(
                                                          const GrTextureStripAtlas::Desc& desc) {
    AtlasEntry* entry = fAtlasCache.find(desc);
    if (!entry) {
        // TODO: Does the AtlasEntry need a copy of the Desc if the GrTextureStripAtlas has one?
        entry = new AtlasEntry(desc, sk_sp<GrTextureStripAtlas>(new GrTextureStripAtlas(desc)));

        fAtlasCache.add(entry);
    }

    return entry->fAtlas;
}
