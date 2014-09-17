/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrProgramElement.h"
#include "GrGpuResourceRef.h"

uint32_t GrProgramElement::CreateUniqueID() {
    static int32_t gUniqueID = SK_InvalidUniqueID;
    uint32_t id;
    do {
        id = static_cast<uint32_t>(sk_atomic_inc(&gUniqueID) + 1);
    } while (id == SK_InvalidUniqueID);
    return id;
}

void GrProgramElement::convertRefToPendingExecution() const {
    // This function makes it so that all the GrGpuResourceRefs own a single ref to their
    // underlying GrGpuResource if there are any refs to the GrProgramElement and a single
    // pending read/write if there are any pending executions of the GrProgramElement. The
    // GrGpuResourceRef will give up its single ref and/or pending read/write in its destructor.
    SkASSERT(fRefCnt > 0);
    if (0 == fPendingExecutions) {
        for (int i = 0; i < fGpuResources.count(); ++i) {
            fGpuResources[i]->markPendingIO();
        }
    }
    ++fPendingExecutions;
    this->unref();
    if (0 == fRefCnt) {
        for (int i = 0; i < fGpuResources.count(); ++i) {
            fGpuResources[i]->removeRef();
        }
    }
}

void GrProgramElement::completedExecution() const {
    this->validate();
    --fPendingExecutions;
    if (0 == fPendingExecutions) {
        if (0 == fRefCnt) {
            SkDELETE(this);
        } else {
            // Now our pending executions have ocurred and we still have refs. Convert
            // ownership of our resources back to regular refs.
            for (int i = 0; i < fGpuResources.count(); ++i) {
                fGpuResources[i]->pendingIOComplete();
            }

        }
    }
}
