
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrResourceCache2_DEFINED
#define GrResourceCache2_DEFINED

#include "GrGpuResource.h"
#include "GrGpuResourceCacheAccess.h"
#include "GrResourceKey.h"
#include "SkRefCnt.h"
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

    void insertResource(GrGpuResource*);

    void removeResource(GrGpuResource*);

    // This currently returns a bool and fails when an existing resource has a key that collides
    // with the new content key. In the future it will null out the content key for the existing
    // resource. The failure is a temporary measure taken because duties are split between two
    // cache objects currently.
    bool didSetContentKey(GrGpuResource*);

    void abandonAll();

    void releaseAll();

    enum {
        /** Preferentially returns scratch resources with no pending IO. */
        kPreferNoPendingIO_ScratchFlag = 0x1,
        /** Will not return any resources that match but have pending IO. */
        kRequireNoPendingIO_ScratchFlag = 0x2,
    };
    GrGpuResource* findAndRefScratchResource(const GrResourceKey& scratchKey, uint32_t flags = 0);
    
#ifdef SK_DEBUG
    // This is not particularly fast and only used for validation, so debug only.
    int countScratchEntriesForKey(const GrResourceKey& scratchKey) const {
        SkASSERT(scratchKey.isScratch());
        return fScratchMap.countForKey(scratchKey);
    }
#endif

    GrGpuResource* findAndRefContentResource(const GrResourceKey& contentKey) {
        SkASSERT(!contentKey.isScratch());
        return SkSafeRef(fContentHash.find(contentKey));
    }

    bool hasContentKey(const GrResourceKey& contentKey) const {
        SkASSERT(!contentKey.isScratch());
        return SkToBool(fContentHash.find(contentKey));
    }

private:
#ifdef SK_DEBUG
    bool isInCache(const GrGpuResource* r) const { return fResources.isInList(r); }
#endif

    class AvailableForScratchUse;

    struct ScratchMapTraits {
        static const GrResourceKey& GetKey(const GrGpuResource& r) {
            return r.cacheAccess().getScratchKey();
        }

        static uint32_t Hash(const GrResourceKey& key) { return key.getHash(); }
    };
    typedef SkTMultiMap<GrGpuResource, GrResourceKey, ScratchMapTraits> ScratchMap;

    struct ContentHashTraits {
        static const GrResourceKey& GetKey(const GrGpuResource& r) {
            return *r.cacheAccess().getContentKey();
        }

        static uint32_t Hash(const GrResourceKey& key) { return key.getHash(); }
    };
    typedef SkTDynamicHash<GrGpuResource, GrResourceKey, ContentHashTraits> ContentHash;

    int                                 fCount;
    SkTInternalLList<GrGpuResource>     fResources;
    // This map holds all resources that can be used as scratch resources.
    ScratchMap                          fScratchMap;
    // This holds all resources that have content keys.
    ContentHash                         fContentHash;
};

#endif
