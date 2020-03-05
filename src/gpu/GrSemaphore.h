/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSemaphore_DEFINED
#define GrSemaphore_DEFINED

#include "include/gpu/GrBackendSemaphore.h"
#include "src/gpu/GrGpuResource.h"

/**
 * Represents a semaphore-like GPU synchronization object. This is a slightly odd fit for
 * GrGpuResource because we don't care about budgeting, recycling, or read/write references for
 * these. However, making it a GrGpuResource makes it simpler to handle releasing/abandoning these
 * along with other resources. If more cases like this arise we could consider moving some of the
 * unused functionality off of GrGpuResource.
 */
class GrSemaphore {
public:
    virtual ~GrSemaphore() {}

    // The derived class can return its GrBackendSemaphore. This is used when flushing with signal
    // semaphores so we can set the client's GrBackendSemaphore object after we've created the
    // internal semaphore.
    virtual GrBackendSemaphore backendSemaphore() const = 0;

private:
    friend class GrGpu; // for setIsOwned
    // This is only used in GrGpu to handle the case where we created a semaphore that was meant to
    // be borrowed, but we failed to submit it. So we must go back and switch the semaphore to owned
    // so that it gets deleted.
    virtual void setIsOwned() = 0;
};

#endif
