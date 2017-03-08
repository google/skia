/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrOpList.h"

#include "GrRenderTargetOpList.h"
#include "GrSurface.h"
#include "GrSurfaceProxy.h"

uint32_t GrOpList::CreateUniqueID() {
    static int32_t gUniqueID = SK_InvalidUniqueID;
    uint32_t id;
    // Loop in case our global wraps around, as we never want to return a 0.
    do {
        id = static_cast<uint32_t>(sk_atomic_inc(&gUniqueID) + 1);
    } while (id == SK_InvalidUniqueID);
    return id;
}

GrOpList::GrOpList(GrSurfaceProxy* surfaceProxy, GrAuditTrail* auditTrail)
    : fUniqueID(CreateUniqueID())
    , fFlags(0)
    , fTarget(surfaceProxy)
    , fAuditTrail(auditTrail) {

    surfaceProxy->setLastOpList(this);
}

GrOpList::~GrOpList() {
    if (fTarget && this == fTarget->getLastOpList()) {
        fTarget->setLastOpList(nullptr);
    }
}

// Add a GrOpList-based dependency
void GrOpList::addDependency(GrOpList* dependedOn) {
    SkASSERT(!dependedOn->dependsOn(this));  // loops are bad

    if (this->dependsOn(dependedOn)) {
        return;  // don't add duplicate dependencies
    }

    *fDependencies.push() = dependedOn;
}

// Convert from a GrSurface-based dependency to a GrOpList one
void GrOpList::addDependency(GrSurface* dependedOn) {
    if (dependedOn->getLastOpList()) {
        // If it is still receiving dependencies, this GrOpList shouldn't be closed
        SkASSERT(!this->isClosed());

        GrOpList* opList = dependedOn->getLastOpList();
        if (opList == this) {
            // self-read - presumably for dst reads
        } else {
            this->addDependency(opList);

            // Can't make it closed in the self-read case
            opList->makeClosed();
        }
    }
}

#ifdef SK_DEBUG
void GrOpList::dump() const {
    SkDebugf("--------------------------------------------------------------\n");
    SkDebugf("node: %d -> RT: %d\n", fUniqueID, fTarget ? fTarget->uniqueID().asUInt() : -1);
    SkDebugf("relies On (%d): ", fDependencies.count());
    for (int i = 0; i < fDependencies.count(); ++i) {
        SkDebugf("%d, ", fDependencies[i]->fUniqueID);
    }
    SkDebugf("\n");
}
#endif
