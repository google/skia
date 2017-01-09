/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrProgramElement.h"
#include "GrGpuResourceRef.h"
#include "SkAtomics.h"

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
