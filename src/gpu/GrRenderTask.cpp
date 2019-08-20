/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrRenderTask.h"

#include "src/gpu/GrRenderTargetPriv.h"
#include "src/gpu/GrStencilAttachment.h"
#include "src/gpu/GrTextureProxyPriv.h"

uint32_t GrRenderTask::CreateUniqueID() {
    static std::atomic<uint32_t> nextID{1};
    uint32_t id;
    do {
        id = nextID++;
    } while (id == SK_InvalidUniqueID);
    return id;
}

GrRenderTask::GrRenderTask(sk_sp<GrSurfaceProxy> target)
        : fTarget(std::move(target))
        , fUniqueID(CreateUniqueID())
        , fFlags(0) {
}

GrRenderTask::~GrRenderTask() {
    if (fTarget && this == fTarget->getLastRenderTask()) {
        // Ensure the target proxy doesn't keep hold of a dangling back pointer.
        fTarget->setLastRenderTask(nullptr);
    }
}

#ifdef SK_DEBUG
bool GrRenderTask::deferredProxiesAreInstantiated() const {
    for (int i = 0; i < fDeferredProxies.count(); ++i) {
        if (!fDeferredProxies[i]->isInstantiated()) {
            return false;
        }
    }

    return true;
}
#endif

void GrRenderTask::prepare(GrOpFlushState* flushState) {
    for (int i = 0; i < fDeferredProxies.count(); ++i) {
        fDeferredProxies[i]->texPriv().scheduleUpload(flushState);
    }

    this->onPrepare(flushState);
}

// Add a GrRenderTask-based dependency
void GrRenderTask::addDependency(GrRenderTask* dependedOn) {
    SkASSERT(!dependedOn->dependsOn(this));  // loops are bad

    if (this->dependsOn(dependedOn)) {
        return;  // don't add duplicate dependencies
    }

    fDependencies.push_back(dependedOn);
    dependedOn->addDependent(this);

    SkDEBUGCODE(this->validate());
}

// Convert from a GrSurface-based dependency to a GrRenderTask one
void GrRenderTask::addDependency(GrSurfaceProxy* dependedOn, GrMipMapped mipMapped,
                                 GrTextureResolveManager textureResolveManager,
                                 const GrCaps& caps) {
    GrRenderTask* dependedOnTask = dependedOn->getLastRenderTask();
    GrTextureProxy* textureProxy = dependedOn->asTextureProxy();

    if (GrMipMapped::kYes == mipMapped) {
        SkASSERT(textureProxy);
        if (GrMipMapped::kYes != textureProxy->mipMapped()) {
            // There are some cases where we might be given a non-mipmapped texture with a mipmap
            // filter. See skbug.com/7094.
            mipMapped = GrMipMapped::kNo;
        }
    }

    // Does this proxy have mipmaps that need to be regenerated?
    if (GrMipMapped::kYes == mipMapped && textureProxy->mipMapsAreDirty()) {
        // We only read our own target during dst reads, and we shouldn't use mipmaps in that case.
        SkASSERT(dependedOnTask != this);

        // Create an opList that resolves the texture's mipmap data.
        GrRenderTask* textureResolveTask = textureResolveManager.newTextureResolveRenderTask(
                sk_ref_sp(textureProxy), GrTextureResolveFlags::kMipMaps, caps);

        // The GrTextureResolveRenderTask factory should have called addDependency (in this
        // instance, recursively) on the textureProxy.
        SkASSERT(!dependedOnTask || textureResolveTask->dependsOn(dependedOnTask));
        SkASSERT(!textureProxy->texPriv().isDeferred() ||
                 textureResolveTask->fDeferredProxies.back() == textureProxy);

        // The GrTextureResolveRenderTask factory should have also marked the mipmaps clean and set
        // the last opList on the textureProxy to textureResolveTask.
        SkASSERT(!textureProxy->mipMapsAreDirty());
        SkASSERT(textureProxy->getLastRenderTask() == textureResolveTask);

        // Fall through and add textureResolveTask as a dependency of "this".
        dependedOnTask = textureResolveTask;
    } else if (textureProxy && textureProxy->texPriv().isDeferred()) {
        fDeferredProxies.push_back(textureProxy);
    }

    if (dependedOnTask) {
        // If it is still receiving dependencies, this GrRenderTask shouldn't be closed
        SkASSERT(!this->isClosed());

        if (dependedOnTask == this) {
            // self-read - presumably for dst reads. We can't make it closed in the self-read case.
        } else {
            this->addDependency(dependedOnTask);

            // We are closing 'dependedOnTask' here bc the current contents of it are what 'this'
            // dependedOnTask depends on. We need a break in 'dependedOnTask' so that the usage of
            // that state has a chance to execute.
            dependedOnTask->makeClosed(caps);
        }
    }
}

bool GrRenderTask::dependsOn(const GrRenderTask* dependedOn) const {
    for (int i = 0; i < fDependencies.count(); ++i) {
        if (fDependencies[i] == dependedOn) {
            return true;
        }
    }

    return false;
}


void GrRenderTask::addDependent(GrRenderTask* dependent) {
    fDependents.push_back(dependent);
}

#ifdef SK_DEBUG
bool GrRenderTask::isDependedent(const GrRenderTask* dependent) const {
    for (int i = 0; i < fDependents.count(); ++i) {
        if (fDependents[i] == dependent) {
            return true;
        }
    }

    return false;
}

void GrRenderTask::validate() const {
    // TODO: check for loops and duplicates

    for (int i = 0; i < fDependencies.count(); ++i) {
        SkASSERT(fDependencies[i]->isDependedent(this));
    }
}
#endif

void GrRenderTask::closeThoseWhoDependOnMe(const GrCaps& caps) {
    for (int i = 0; i < fDependents.count(); ++i) {
        if (!fDependents[i]->isClosed()) {
            fDependents[i]->makeClosed(caps);
        }
    }
}

bool GrRenderTask::isInstantiated() const {
    // Some renderTasks (e.g. GrTransferFromRenderTask) don't have a target.
    if (!fTarget) {
        return true;
    }

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
void GrRenderTask::dump(bool printDependencies) const {
    SkDebugf("--------------------------------------------------------------\n");
    SkDebugf("renderTaskID: %d - proxyID: %d - surfaceID: %d\n", fUniqueID,
             fTarget ? fTarget->uniqueID().asUInt() : -1,
             fTarget && fTarget->peekSurface()
                     ? fTarget->peekSurface()->uniqueID().asUInt()
                     : -1);

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
