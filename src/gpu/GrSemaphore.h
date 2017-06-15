/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSemaphore_DEFINED
#define GrSemaphore_DEFINED

#include "SkRefCnt.h"

class GrGpu;

class GrSemaphore : public SkRefCnt {
private:
    // This function should only be used in the case of exporting and importing a GrSemaphore object
    // from one GrContext to another. When exporting, the GrSemaphore should be set to a null GrGpu,
    // and when importing it should be set to the GrGpu of the current context. Once exported, a
    // GrSemaphore should not be used with its old context.
    void resetGpu(const GrGpu* gpu) { fGpu = gpu; }

protected:
    explicit GrSemaphore(const GrGpu* gpu) : fGpu(gpu) {}

    friend class GrResourceProvider; // resetGpu

    const GrGpu* fGpu;
};

#endif
