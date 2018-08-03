/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextureStripAtlas.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrDDLTextureStripAtlas.h"
#include "GrDynamicTextureStripAtlas.h"
#include "SkBitmap.h"

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

void GrTextureStripAtlasManager::finish(GrProxyProvider* proxyProvider) {
    for (AtlasHash::Iter iter(&fAtlasCache); !iter.done(); ++iter) {
        AtlasEntry* tmp = &(*iter);
        tmp->fAtlas->finish(proxyProvider);
    }
}


sk_sp<GrTextureStripAtlas> GrTextureStripAtlasManager::addStrip(
                                                          GrContext* context,
                                                          const GrTextureStripAtlas::Desc& desc,
                                                          const SkBitmap& bitmap,
                                                          int* row) {
    SkASSERT(kPremul_SkAlphaType == bitmap.alphaType());

    AtlasEntry* entry = fAtlasCache.find(desc);
    if (!entry) {
        sk_sp<GrTextureStripAtlas> atlas;

        if (!context->contextPriv().resourceProvider()) {
            atlas.reset(new GrDDLTextureStripAtlas(desc));
        } else {
            atlas.reset(new GrDynamicTextureStripAtlas(desc));
        }

        entry = new AtlasEntry(sk_sp<GrTextureStripAtlas>(std::move(atlas)));
        fAtlasCache.add(entry);
    }

    *row = entry->fAtlas->addStrip(context, bitmap);
    if (*row < 0) {
        return nullptr;
    }

    return entry->fAtlas;
}

