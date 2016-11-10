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

GrOpList::GrOpList(GrSurfaceProxy* surfaceProxy, GrAuditTrail* auditTrail) 
    : fFlags(0)
    , fTarget(surfaceProxy)
    , fAuditTrail(auditTrail) {

    surfaceProxy->setLastOpList(this);

#ifdef SK_DEBUG
    static int debugID = 0;
    fDebugID = debugID++;
#endif
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
    SkDebugf("node: %d -> RT: %d\n", fDebugID, fTarget ? fTarget->uniqueID().asUInt() : -1);
    SkDebugf("relies On (%d): ", fDependencies.count());
    for (int i = 0; i < fDependencies.count(); ++i) {
        SkDebugf("%d, ", fDependencies[i]->fDebugID);
    }
    SkDebugf("\n");
}
#endif
