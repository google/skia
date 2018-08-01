/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureStripAtlas_DEFINED
#define GrTextureStripAtlas_DEFINED

#include "effects/GrDynamicTextureStripAtlas.h"

#include "SkNoncopyable.h"
#include "SkOpts.h"
#include "SkRefCnt.h"
#include "SkTDynamicHash.h"

class GrContext;
class GrProxyProvider;
class GrTextureProxy;
class SkBitmap;

class GrTextureStripAtlasManager {
public:
    GrTextureStripAtlasManager() {}
    ~GrTextureStripAtlasManager();

    void abandon();

    /**
     * Try to find an atlas with the required parameters, creates a new one if necessary
     */
    sk_sp<GrTextureStripAtlas> refAtlas(const GrTextureStripAtlas::Desc&);

private:
    void deleteAllAtlases();

    // Hash table entry for atlases
    class AtlasEntry : public ::SkNoncopyable {
    public:
        AtlasEntry(const GrTextureStripAtlas::Desc& desc, sk_sp<GrTextureStripAtlas> atlas)
            : fDesc(desc)
            , fAtlas(std::move(atlas)) {
        }
        ~AtlasEntry() { }

        // for SkTDynamicHash
        static const GrTextureStripAtlas::Desc& GetKey(const AtlasEntry& entry) {
            return entry.fDesc;
        }
        static uint32_t Hash(const GrTextureStripAtlas::Desc& desc) {
            return SkOpts::hash(&desc, sizeof(GrTextureStripAtlas::Desc));
        }

        const GrTextureStripAtlas::Desc fDesc;
        sk_sp<GrTextureStripAtlas> fAtlas;
    };

    typedef SkTDynamicHash<AtlasEntry, GrTextureStripAtlas::Desc> AtlasHash;

    AtlasHash fAtlasCache;
};

#endif
