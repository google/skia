/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSharedLock_DEFINED
#define SkSharedLock_DEFINED

#include "include/private/base/SkDebug.h"
#include "include/private/base/SkSemaphore.h"
#include "include/private/base/SkThreadAnnotations.h"

#ifdef SK_DEBUG
    #include "include/private/base/SkMutex.h"
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
//
// This allows us to have a mutex without needing the one in
// the C++ std library which does not work with all clients.
// go/cstyle#Disallowed_Stdlib
class SK_CAPABILITY("mutex") SkSharedMutex {
public:
    SkSharedMutex();
    ~SkSharedMutex();
    // Acquire lock for exclusive use.
    void acquire() SK_ACQUIRE();

    // Release lock for exclusive use.
    void release() SK_RELEASE_CAPABILITY();

    // Fail if exclusive is not held.
    void assertHeld() const SK_ASSERT_CAPABILITY(this);

    // Acquire lock for shared use.
    void acquireShared() SK_ACQUIRE_SHARED();

    // Release lock for shared use.
    void releaseShared() SK_RELEASE_SHARED_CAPABILITY();

    // Fail if shared lock not held.
    void assertHeldShared() const SK_ASSERT_SHARED_CAPABILITY(this);

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
    std::atomic<int32_t> fQueueCounts;
    SkSemaphore          fSharedQueue;
    SkSemaphore          fExclusiveQueue;
#endif  // SK_DEBUG
};

#ifndef SK_DEBUG
inline void SkSharedMutex::assertHeld() const {}
inline void SkSharedMutex::assertHeldShared() const {}
#endif  // SK_DEBUG

class SK_SCOPED_CAPABILITY SkAutoSharedMutexExclusive {
public:
    explicit SkAutoSharedMutexExclusive(SkSharedMutex& lock) SK_ACQUIRE(lock)
            : fLock(lock) {
        lock.acquire();
    }
    ~SkAutoSharedMutexExclusive() SK_RELEASE_CAPABILITY() { fLock.release(); }

private:
    SkSharedMutex& fLock;
};

class SK_SCOPED_CAPABILITY SkAutoSharedMutexShared {
public:
    explicit SkAutoSharedMutexShared(SkSharedMutex& lock) SK_ACQUIRE_SHARED(lock)
            : fLock(lock)  {
        lock.acquireShared();
    }

    // You would think this should be SK_RELEASE_SHARED_CAPABILITY, but SK_SCOPED_CAPABILITY
    // doesn't fully understand the difference between shared and exclusive.
    // Please review https://reviews.llvm.org/D52578 for more information.
    ~SkAutoSharedMutexShared() SK_RELEASE_CAPABILITY() { fLock.releaseShared(); }

private:
    SkSharedMutex& fLock;
};

#endif // SkSharedLock_DEFINED
