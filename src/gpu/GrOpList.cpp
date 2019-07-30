/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrOpList.h"

#include "include/gpu/GrContext.h"
#include "src/gpu/GrDeferredProxyUploader.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrRenderTargetPriv.h"
#include "src/gpu/GrStencilAttachment.h"
#include "src/gpu/GrSurfaceProxy.h"
#include "src/gpu/GrTextureProxyPriv.h"
#include <atomic>

uint32_t GrOpList::CreateUniqueID() {
    static std::atomic<uint32_t> nextID{1};
    uint32_t id;
    do {
        id = nextID++;
    } while (id == SK_InvalidUniqueID);
    return id;
}

GrOpList::GrOpList(sk_sp<GrOpMemoryPool> opMemoryPool,
                   sk_sp<GrSurfaceProxy> surfaceProxy,
                   GrAuditTrail* auditTrail)
        : fOpMemoryPool(std::move(opMemoryPool))
        , fAuditTrail(auditTrail)
        , fUniqueID(CreateUniqueID())
        , fFlags(0) {
    SkASSERT(fOpMemoryPool);
    fTarget = std::move(surfaceProxy);
}

GrOpList::~GrOpList() {
    if (fTarget && this == fTarget->getLastOpList()) {
        // Ensure the target proxy doesn't keep hold of a dangling back pointer.
        fTarget->setLastOpList(nullptr);
    }
}

void GrOpList::endFlush() {
    if (fTarget && this == fTarget->getLastOpList()) {
        fTarget->setLastOpList(nullptr);
    }

    fTarget.reset();
    fDeferredProxies.reset();
    fAuditTrail = nullptr;
}

#ifdef SK_DEBUG
bool GrOpList::deferredProxiesAreInstantiated() const {
    for (int i = 0; i < fDeferredProxies.count(); ++i) {
        if (!fDeferredProxies[i]->isInstantiated()) {
            return false;
        }
    }

    return true;
}
#endif

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
    dependedOn->addDependent(this);

    SkDEBUGCODE(this->validate());
}

// Convert from a GrSurface-based dependency to a GrOpList one
void GrOpList::addDependency(
        GrSurfaceProxy* dependedOn, GrMipMapped mipMapped,
        GrTextureResolveManager textureResolveManager, const GrCaps& caps) {
    GrOpList* dependedOnOpList = dependedOn->getLastOpList();

    GrTextureProxy* textureProxy = dependedOn->asTextureProxy();
    SkASSERT(GrMipMapped::kNo == mipMapped || textureProxy);

    // Does this have mipmaps that need to be regenerated?
    if (GrMipMapped::kYes == mipMapped && textureProxy->mipMapsAreDirty()) {
        // We only read our own target during dst reads, and we shouldn't use mipmaps in that case.
        SkASSERT(dependedOnOpList != this);

        // Create an opList that resolves the texture's mipmap data.
        GrOpList* textureResolveOpList = textureResolveManager.newTextureResolveOpList(
                sk_ref_sp(textureProxy), GrTextureResolveManager::ResolveFlags::kMipMaps, caps);

        // The GrTextureResolveOpList constructor should have called addDependency (in this
        // instance, recursively) on the textureProxy.
        SkASSERT(!dependedOnOpList || textureResolveOpList->dependsOn(dependedOnOpList));
        SkASSERT(!textureProxy->texPriv().isDeferred() ||
                 textureResolveOpList->fDeferredProxies.back() == textureProxy);

        // The GrTextureResolveOpList constructors should have also marked the mipmaps clean and set
        // the last opList on the textureProxy to textureResolveOpList.
        SkASSERT(!textureProxy->mipMapsAreDirty());
        SkASSERT(textureProxy->getLastOpList() == textureResolveOpList);

        // Fall through and add textureResolveOpList as a dependency of "this".
        dependedOnOpList = textureResolveOpList;
    } else if (textureProxy && textureProxy->texPriv().isDeferred()) {
        fDeferredProxies.push_back(textureProxy);
    }

    if (dependedOnOpList) {
        // If it is still receiving dependencies, this GrOpList shouldn't be closed
        SkASSERT(!this->isClosed());

        if (dependedOnOpList == this) {
            // self-read - presumably for dst reads. We can't make it closed in the self-read case.
        } else {
            this->addDependency(dependedOnOpList);

            // We are closing 'dependedOnOpList' here bc the current contents of it are what 'this'
            // dependedOnOpList depends on. We need a break in 'dependedOnOpList' so that the usage
            // of that state has a chance to execute.
            dependedOnOpList->makeClosed(caps);
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


void GrOpList::addDependent(GrOpList* dependent) {
    fDependents.push_back(dependent);
}

#ifdef SK_DEBUG
bool GrOpList::isDependedent(const GrOpList* dependent) const {
    for (int i = 0; i < fDependents.count(); ++i) {
        if (fDependents[i] == dependent) {
            return true;
        }
    }

    return false;
}

void GrOpList::validate() const {
    // TODO: check for loops and duplicates

    for (int i = 0; i < fDependencies.count(); ++i) {
        SkASSERT(fDependencies[i]->isDependedent(this));
    }
}
#endif

void GrOpList::closeThoseWhoDependOnMe(const GrCaps& caps) {
    for (int i = 0; i < fDependents.count(); ++i) {
        if (!fDependents[i]->isClosed()) {
            fDependents[i]->makeClosed(caps);
        }
    }
}

bool GrOpList::isInstantiated() const {
    if (!fTarget->isInstantiated()) {
        return false;
    }

    int minStencilSampleCount = (fTarget->asRenderTargetProxy())
            ? fTarget->asRenderTargetProxy()->numStencilSamples()
            : 0;

    if (minStencilSampleCount) {
        GrRenderTarget* rt = fTarget->peekRenderTarget();
        SkASSERT(rt);

        GrStencilAttachment* stencil = rt->renderTargetPriv().getStencilAttachment();
        if (!stencil) {
            return false;
        }
        SkASSERT(stencil->numSamples() >= minStencilSampleCount);
    }

    GrSurface* surface = fTarget->peekSurface();
    if (surface->wasDestroyed()) {
        return false;
    }

    return true;
}

#ifdef SK_DEBUG
static const char* op_to_name(GrLoadOp op) {
    return GrLoadOp::kLoad == op ? "load" : GrLoadOp::kClear == op ? "clear" : "discard";
}

void GrOpList::dump(bool printDependencies) const {
    SkDebugf("--------------------------------------------------------------\n");
    SkDebugf("opListID: %d - proxyID: %d - surfaceID: %d\n", fUniqueID,
             fTarget ? fTarget->uniqueID().asUInt() : -1,
             fTarget && fTarget->peekSurface()
                     ? fTarget->peekSurface()->uniqueID().asUInt()
                     : -1);
    SkDebugf("ColorLoadOp: %s %x StencilLoadOp: %s\n",
             op_to_name(fColorLoadOp),
             GrLoadOp::kClear == fColorLoadOp ? fLoadClearColor.toBytes_RGBA() : 0x0,
             op_to_name(fStencilLoadOp));

    if (printDependencies) {
        SkDebugf("I rely On (%d): ", fDependencies.count());
        for (int i = 0; i < fDependencies.count(); ++i) {
            SkDebugf("%d, ", fDependencies[i]->fUniqueID);
        }
        SkDebugf("\n");

        SkDebugf("(%d) Rely On Me: ", fDependents.count());
        for (int i = 0; i < fDependents.count(); ++i) {
            SkDebugf("%d, ", fDependents[i]->fUniqueID);
        }
        SkDebugf("\n");
    }
}
#endif
