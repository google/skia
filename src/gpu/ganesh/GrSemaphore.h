/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSemaphore_DEFINED
#define GrSemaphore_DEFINED

#include "include/gpu/ganesh/GrBackendSemaphore.h"

/**
 * Represents a semaphore-like GPU synchronization object.
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
