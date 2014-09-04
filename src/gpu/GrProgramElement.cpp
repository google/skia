/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrProgramElement.h"
#include "GrProgramResource.h"

void GrProgramElement::convertRefToPendingExecution() const {
    // This function makes it so that all the GrProgramResources own a single ref to their
    // underlying GrGpuResource if there are any refs to the GrProgramElement and a single
    // pending read/write if there are any pending executions of the GrProgramElement. The
    // GrProgramResource will give up its single ref and/or pending read/write in its destructor.
    SkASSERT(fRefCnt > 0);
    if (0 == fPendingExecutions) {
        for (int i = 0; i < fProgramResources.count(); ++i) {
            fProgramResources[i]->markPendingIO();
        }
    }
    ++fPendingExecutions;
    this->unref();
    if (0 == fRefCnt) {
        for (int i = 0; i < fProgramResources.count(); ++i) {
            fProgramResources[i]->removeRef();
        }
    }
}
