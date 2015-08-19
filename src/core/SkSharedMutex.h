/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSharedLock_DEFINED
#define SkSharedLock_DEFINED

#include "SkAtomics.h"
#include "SkSemaphore.h"
#include "SkTypes.h"
    
// This is a shared lock implementation similar to pthreads rwlocks. This implementation is
// cribbed from Preshing's article:
// http://preshing.com/20150316/semaphores-are-surprisingly-versatile/
//
// This lock does not obey strict queue ordering. It will always alternate between readers and
// a single writer.
class SkSharedMutex {
public:
    SkSharedMutex();
    ~SkSharedMutex();
    // Acquire lock for exclusive use.
    void acquire();

    // Release lock for exclusive use.
    void release();

    // Fail if exclusive is not held.
#ifdef SK_DEBUG
    void assertHeld() const;
#else
    void assertHeld() const {}
#endif

    // Acquire lock for shared use.
    void acquireShared();

    // Release lock for shared use.
    void releaseShared();

    // Fail if shared lock not held.
#ifdef SK_DEBUG
    void assertHeldShared() const;
#else
    void assertHeldShared() const {}
#endif

private:
    SkAtomic<int32_t> fQueueCounts;
    SkSemaphore fSharedQueue;
    SkSemaphore fExclusiveQueue;
};


#endif // SkSharedLock_DEFINED
