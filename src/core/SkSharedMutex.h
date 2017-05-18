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

#ifdef SK_DEBUG
    #include "SkMutex.h"
    #include <memory>
#endif  // SK_DEBUG

// There are two shared lock implementations one debug the other is high performance. They implement
// an interface similar to pthread's rwlocks.
// This is a shared lock implementation similar to pthreads rwlocks. The high performance
// implementation is cribbed from Preshing's article:
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
    void assertHeld() const;

    // Acquire lock for shared use.
    void acquireShared();

    // Release lock for shared use.
    void releaseShared();

    // Fail if shared lock not held.
    void assertHeldShared() const;

private:
#ifdef SK_DEBUG
    class ThreadIDSet;
    std::unique_ptr<ThreadIDSet> fCurrentShared;
    std::unique_ptr<ThreadIDSet> fWaitingExclusive;
    std::unique_ptr<ThreadIDSet> fWaitingShared;
    int fSharedQueueSelect{0};
    mutable SkMutex fMu;
    SkSemaphore fSharedQueue[2];
    SkSemaphore fExclusiveQueue;
#else
    SkAtomic<int32_t> fQueueCounts;
    SkSemaphore       fSharedQueue;
    SkSemaphore       fExclusiveQueue;
#endif  // SK_DEBUG
};

#ifndef SK_DEBUG
inline void SkSharedMutex::assertHeld() const {};
inline void SkSharedMutex::assertHeldShared() const {};
#endif  // SK_DEBUG

class SkAutoSharedMutexShared {
public:
    SkAutoSharedMutexShared(SkSharedMutex& lock) : fLock(lock) { lock.acquireShared(); }
    ~SkAutoSharedMutexShared() { fLock.releaseShared(); }
private:
    SkSharedMutex& fLock;
};

#define SkAutoSharedMutexShared(...) SK_REQUIRE_LOCAL_VAR(SkAutoSharedMutexShared)

#endif // SkSharedLock_DEFINED
