
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrResourceCache2.h"
#include "GrGpuResource.h"  
#include "SkRefCnt.h"

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
        fScratchMap.insert(resource->getScratchKey(), resource);
    }
}

void GrResourceCache2::removeResource(GrGpuResource* resource) {
    SkASSERT(this->isInCache(resource));
    fResources.remove(resource);    
    if (!resource->getScratchKey().isNullScratch()) {
        fScratchMap.remove(resource->getScratchKey(), resource);
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
