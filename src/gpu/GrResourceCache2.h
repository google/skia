
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrResourceCache2_DEFINED
#define GrResourceCache2_DEFINED

#include "GrGpuResource.h"
#include "GrResourceKey.h"
#include "SkTInternalLList.h"
#include "SkTMultiMap.h"

/**
 *  Eventual replacement for GrResourceCache. Currently it simply holds a list
 *  of all GrGpuResource objects for a GrContext. It is used to invalidate all
 *  the resources when necessary.
 */
class GrResourceCache2 {
public:
    GrResourceCache2() : fCount(0) {};
    ~GrResourceCache2();

    void insertResource(GrGpuResource* resource);

    void removeResource(GrGpuResource* resource);

    void abandonAll();

    void releaseAll();

private:
#ifdef SK_DEBUG
    bool isInCache(const GrGpuResource* r) const {
        return fResources.isInList(r);
    }
#endif


    void removeScratch(const GrGpuResource* resource);
    struct ScratchMapTraits {
        static const GrResourceKey& GetKey(const GrGpuResource& r) {
            return r.getScratchKey();
        }

        static uint32_t Hash(const GrResourceKey& key) { return key.getHash(); }
    };
    typedef SkTMultiMap<GrGpuResource, GrResourceKey, ScratchMapTraits> ScratchMap;

    int                                 fCount;
    SkTInternalLList<GrGpuResource>     fResources;
    // This map holds all resources that can be used as scratch resources.
    ScratchMap                          fScratchMap; 
};

#endif
