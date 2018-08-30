/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSemaphore_DEFINED
#define GrSemaphore_DEFINED

#include "GrBackendSemaphore.h"
#include "SkRefCnt.h"

class GrGpu;

class GrSemaphore : public SkRefCnt {
public:
    // The derived class can return its GrBackendSemaphore. This is used when flushing with signal
    // semaphores so we can set the client's GrBackendSemaphore object after we've created the
    // internal semaphore.
    virtual GrBackendSemaphore backendSemaphore() const = 0;

protected:
    explicit GrSemaphore(const GrGpu* gpu) : fGpu(gpu) {}

    const GrGpu* fGpu;
};

#endif
