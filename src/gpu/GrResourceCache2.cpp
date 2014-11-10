
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrResourceCache2.h"
#include "GrGpuResource.h"  

GrResourceKey& GrResourceKey::NullScratchKey() {
    static const GrCacheID::Key kBogusKey = { { {0} } };
    static GrCacheID kBogusID(ScratchDomain(), kBogusKey);
    static GrResourceKey kNullScratchKey(kBogusID, NoneResourceType(), 0);
    return kNullScratchKey;
}

GrResourceKey::ResourceType GrResourceKey::NoneResourceType() {
    static const ResourceType gNoneResourceType = GenerateResourceType();
    return gNoneResourceType;
}

GrCacheID::Domain GrResourceKey::ScratchDomain() {
    static const GrCacheID::Domain gDomain = GrCacheID::GenerateDomain();
    return gDomain;
}

//////////////////////////////////////////////////////////////////////////////

GrResourceCache2::~GrResourceCache2() {
    this->releaseAll();
}

void GrResourceCache2::insertResource(GrGpuResource* resource) {
    SkASSERT(resource);
    SkASSERT(!resource->wasDestroyed());
    SkASSERT(!this->isInCache(resource));
    fResources.addToHead(resource);
    ++fCount;
    if (!resource->getScratchKey().isNullScratch()) {
        // TODO(bsalomon): Make this assertion possible.
        // SkASSERT(!resource->isWrapped());
        fScratchMap.insert(resource->getScratchKey(), resource);
    }
}

void GrResourceCache2::removeResource(GrGpuResource* resource) {
    SkASSERT(this->isInCache(resource));
    fResources.remove(resource);    
    if (!resource->getScratchKey().isNullScratch()) {
        fScratchMap.remove(resource->getScratchKey(), resource);
    }
    if (const GrResourceKey* contentKey = resource->getContentKey()) {
        fContentHash.remove(*contentKey);
    }
    --fCount;
}

void GrResourceCache2::abandonAll() {
    while (GrGpuResource* head = fResources.head()) {
        SkASSERT(!head->wasDestroyed());
        head->abandon();
        // abandon should have already removed this from the list.
        SkASSERT(head != fResources.head());
    }
    SkASSERT(!fScratchMap.count());
    SkASSERT(!fContentHash.count());
    SkASSERT(!fCount);
}

void GrResourceCache2::releaseAll() {
    while (GrGpuResource* head = fResources.head()) {
        SkASSERT(!head->wasDestroyed());
        head->release();
        // release should have already removed this from the list.
        SkASSERT(head != fResources.head());
    }
    SkASSERT(!fScratchMap.count());
    SkASSERT(!fCount);
}

class GrResourceCache2::AvailableForScratchUse {
public:
    AvailableForScratchUse(bool rejectPendingIO) : fRejectPendingIO(rejectPendingIO) { }

    bool operator()(const GrGpuResource* resource) const {
        if (!resource->reffedOnlyByCache() || !resource->isScratch()) {
            return false;
        }

        return !fRejectPendingIO || !resource->internalHasPendingIO();
    }

private:
    bool fRejectPendingIO;
};

GrGpuResource* GrResourceCache2::findAndRefScratchResource(const GrResourceKey& scratchKey,
                                                           uint32_t flags) {
    SkASSERT(scratchKey.isScratch());

    if (flags & (kPreferNoPendingIO_ScratchFlag | kRequireNoPendingIO_ScratchFlag)) {
        GrGpuResource* resource = fScratchMap.find(scratchKey, AvailableForScratchUse(true));
        if (resource) {
            return SkRef(resource);
        } else if (flags & kRequireNoPendingIO_ScratchFlag) {
            return NULL;
        }
        // TODO: fail here when kPrefer is specified, we didn't find a resource without pending io,
        // but there is still space in our budget for the resource.
    }
    return SkSafeRef(fScratchMap.find(scratchKey, AvailableForScratchUse(false)));
}

void GrResourceCache2::willRemoveContentKey(const GrGpuResource* resource) {
    SkASSERT(resource);
    SkASSERT(resource->getContentKey());
    SkDEBUGCODE(GrGpuResource* res = fContentHash.find(*resource->getContentKey()));
    SkASSERT(res == resource);

    fContentHash.remove(*resource->getContentKey());
}

bool GrResourceCache2::didAddContentKey(GrGpuResource* resource) {
    SkASSERT(resource);
    SkASSERT(resource->getContentKey());

    GrGpuResource* res = fContentHash.find(*resource->getContentKey());
    if (NULL != res) {
        return false;
    }

    fContentHash.add(resource);
    return true;
}
