/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrProgramElement.h"
#include "GrGpuResourceRef.h"
#include "SkAtomics.h"

uint32_t GrProgramElement::CreateUniqueID() {
    static int32_t gUniqueID = SK_InvalidUniqueID;
    uint32_t id;
    do {
        id = static_cast<uint32_t>(sk_atomic_inc(&gUniqueID) + 1);
    } while (id == SK_InvalidUniqueID);
    return id;
}

void GrProgramElement::addPendingIOs() const {
    for (int i = 0; i < fGpuResources.count(); ++i) {
        fGpuResources[i]->markPendingIO();
    }
}

void GrProgramElement::removeRefs() const {
    for (int i = 0; i < fGpuResources.count(); ++i) {
        fGpuResources[i]->removeRef();
    }
}

void GrProgramElement::pendingIOComplete() const {
    for (int i = 0; i < fGpuResources.count(); ++i) {
        fGpuResources[i]->pendingIOComplete();
    }
}
