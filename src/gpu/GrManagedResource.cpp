/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrManagedResource.h"

#include "src/gpu/GrGpuResourcePriv.h"
#include "src/gpu/GrTexture.h"


#ifdef SK_TRACE_MANAGED_RESOURCES
std::atomic<uint32_t> GrManagedResource::fKeyCounter{0};
#endif

void GrTextureResource::addIdleProc(GrTexture* owningTexture,
                                    sk_sp<GrRefCntedCallback> idleProc) const {
    SkASSERT(!fOwningTexture || fOwningTexture == owningTexture);
    fOwningTexture = owningTexture;
    fIdleProcs.push_back(std::move(idleProc));
}

int GrTextureResource::idleProcCnt() const { return fIdleProcs.count(); }

sk_sp<GrRefCntedCallback> GrTextureResource::idleProc(int i) const { return fIdleProcs[i]; }

void GrTextureResource::resetIdleProcs() const { fIdleProcs.reset(); }

void GrTextureResource::removeOwningTexture() const { fOwningTexture = nullptr; }

void GrTextureResource::notifyQueuedForWorkOnGpu() const { ++fNumOwners; }

void GrTextureResource::notifyFinishedWithWorkOnGpu() const {
    SkASSERT(fNumOwners);
    if (--fNumOwners || !fIdleProcs.count()) {
        return;
    }
    if (fOwningTexture) {
        if (fOwningTexture->resourcePriv().hasRef()) {
            // Wait for the texture to become idle in the cache to call the procs.
            return;
        }
        fOwningTexture->callIdleProcsOnBehalfOfResource();
    } else {
        fIdleProcs.reset();
    }
}
