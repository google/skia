/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrHack_DEFINED
#define GrHack_DEFINED

#include "include/gpu/GrTypes.h"
#include "include/private/GrTypesPriv.h"

class GrScratchKey;

// Captures information about a single GrSurface in the GrResourceCache
struct GrTextureCacheInfo {
    GrPixelConfig fConfig;
    int fWidth;
    int fHeight;
    GrRenderable fRenderable;
    int fSampleCount;
    GrMipMapped fMipMapped;
    bool fHasScratchKey;

    void dump() const;
    void computeScratchKey(GrScratchKey* dst) const;
};

// Captures the state of Ganesh's GrResourceCache
class GrCacheState {
public:
    GrCacheState(const SkString& name, int numNonPurgeable, int numPurgeable)
            : fName(name)
            , fNumNonPurgeable(numNonPurgeable)
            , fNumPurgeable(numPurgeable) {
        fNonPurgeableTextures.reset(new GrTextureCacheInfo[fNumNonPurgeable]);
        fPurgeableTextures.reset(new GrTextureCacheInfo[fNumPurgeable]);
    }

    void dump() const;

private:
    friend class GrResourceCache; // for non-const nonPurgeableResource & purgeableResource

    GrTextureCacheInfo* nonPurgeableResource(int index) {
        return &fNonPurgeableTextures[index];
    }

    GrTextureCacheInfo* purgeableResource(int index) {
        return &fPurgeableTextures[index];
    }

    const GrTextureCacheInfo* nonPurgeableResource(int index) const {
        return &fNonPurgeableTextures[index];
    }

    const GrTextureCacheInfo* purgeableResource(int index) const {
        return &fPurgeableTextures[index];
    }

    const SkString fName;

    int            fNumNonPurgeable;
    std::unique_ptr<GrTextureCacheInfo[]> fNonPurgeableTextures;

    int            fNumPurgeable;
    std::unique_ptr<GrTextureCacheInfo[]> fPurgeableTextures;
};


#endif
