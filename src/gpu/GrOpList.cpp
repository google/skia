/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrOpList.h"

#include "GrContext.h"
#include "GrDeferredProxyUploader.h"
#include "GrMemoryPool.h"
#include "GrSurfaceProxy.h"
#include "GrTextureProxyPriv.h"

#include "SkAtomics.h"

uint32_t GrOpList::CreateUniqueID() {
    static int32_t gUniqueID = SK_InvalidUniqueID;
    uint32_t id;
    // Loop in case our global wraps around, as we never want to return a 0.
    do {
        id = static_cast<uint32_t>(sk_atomic_inc(&gUniqueID) + 1);
    } while (id == SK_InvalidUniqueID);
    return id;
}

GrOpList::GrOpList(GrResourceProvider* resourceProvider, sk_sp<GrOpMemoryPool> opMemoryPool,
                   GrSurfaceProxy* surfaceProxy, GrAuditTrail* auditTrail)
        : fOpMemoryPool(std::move(opMemoryPool))
        , fAuditTrail(auditTrail)
        , fUniqueID(CreateUniqueID())
        , fFlags(0) {
    SkASSERT(fOpMemoryPool);
    fTarget.setProxy(sk_ref_sp(surfaceProxy), kWrite_GrIOType);
    fTarget.get()->setLastOpList(this);

    if (resourceProvider && !resourceProvider->explicitlyAllocateGPUResources()) {
        // MDB TODO: remove this! We are currently moving to having all the ops that target
        // the RT as a dest (e.g., clear, etc.) rely on the opList's 'fTarget' pointer
        // for the IO Ref. This works well but until they are all swapped over (and none
        // are pre-emptively instantiating proxies themselves) we need to instantiate
        // here so that the GrSurfaces are created in an order that preserves the GrSurface
        // re-use assumptions.
        fTarget.get()->instantiate(resourceProvider);
    }

    fTarget.markPendingIO();
}

GrOpList::~GrOpList() {
    if (fTarget.get() && this == fTarget.get()->getLastOpList()) {
        // Ensure the target proxy doesn't keep hold of a dangling back pointer.
        fTarget.get()->setLastOpList(nullptr);
    }
}

bool GrOpList::instantiate(GrResourceProvider* resourceProvider) {
    return SkToBool(fTarget.get()->instantiate(resourceProvider));
}

void GrOpList::endFlush() {
    if (fTarget.get() && this == fTarget.get()->getLastOpList()) {
        fTarget.get()->setLastOpList(nullptr);
    }

    fTarget.reset();
    fDeferredProxies.reset();
    fAuditTrail = nullptr;
}

void GrOpList::instantiateDeferredProxies(GrResourceProvider* resourceProvider) {
    for (int i = 0; i < fDeferredProxies.count(); ++i) {
        if (resourceProvider->explicitlyAllocateGPUResources()) {
            SkASSERT(fDeferredProxies[i]->priv().isInstantiated());
        } else {
            fDeferredProxies[i]->instantiate(resourceProvider);
        }
    }
}

void GrOpList::prepare(GrOpFlushState* flushState) {
    for (int i = 0; i < fDeferredProxies.count(); ++i) {
        fDeferredProxies[i]->texPriv().scheduleUpload(flushState);
    }

    this->onPrepare(flushState);
}

// Add a GrOpList-based dependency
void GrOpList::addDependency(GrOpList* dependedOn) {
    SkASSERT(!dependedOn->dependsOn(this));  // loops are bad

    if (this->dependsOn(dependedOn)) {
        return;  // don't add duplicate dependencies
    }

    fDependencies.push_back(dependedOn);
}

// Convert from a GrSurface-based dependency to a GrOpList one
void GrOpList::addDependency(GrSurfaceProxy* dependedOn, const GrCaps& caps) {
    if (dependedOn->getLastOpList()) {
        // If it is still receiving dependencies, this GrOpList shouldn't be closed
        SkASSERT(!this->isClosed());

        GrOpList* opList = dependedOn->getLastOpList();
        if (opList == this) {
            // self-read - presumably for dst reads. We can't make it closed in the self-read case.
        } else {
            this->addDependency(opList);

            // We are closing 'opList' here bc the current contents of it are what 'this' opList
            // depends on. We need a break in 'opList' so that the usage of that state has a
            // chance to execute.
            opList->makeClosed(caps);
        }
    }

    if (GrTextureProxy* textureProxy = dependedOn->asTextureProxy()) {
        if (textureProxy->texPriv().isDeferred()) {
            fDeferredProxies.push_back(textureProxy);
        }
    }
}

bool GrOpList::dependsOn(const GrOpList* dependedOn) const {
    for (int i = 0; i < fDependencies.count(); ++i) {
        if (fDependencies[i] == dependedOn) {
            return true;
        }
    }

    return false;
}

bool GrOpList::isInstantiated() const {
    return fTarget.get()->priv().isInstantiated();
}

#ifdef SK_DEBUG
static const char* op_to_name(GrLoadOp op) {
    return GrLoadOp::kLoad == op ? "load" : GrLoadOp::kClear == op ? "clear" : "discard";
}

void GrOpList::dump(bool printDependencies) const {
    SkDebugf("--------------------------------------------------------------\n");
    SkDebugf("opListID: %d -> proxyID: %d\n", fUniqueID,
             fTarget.get() ? fTarget.get()->uniqueID().asUInt() : -1);
    SkDebugf("ColorLoadOp: %s %x StencilLoadOp: %s\n",
             op_to_name(fColorLoadOp),
             GrLoadOp::kClear == fColorLoadOp ? fLoadClearColor : 0x0,
             op_to_name(fStencilLoadOp));

    if (printDependencies) {
        SkDebugf("I rely On (%d): ", fDependencies.count());
        for (int i = 0; i < fDependencies.count(); ++i) {
            SkDebugf("%d, ", fDependencies[i]->fUniqueID);
        }
        SkDebugf("\n");
    }
}
#endif
