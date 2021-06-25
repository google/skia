/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkSharedMutex.h"

#include "include/core/SkTypes.h"
#include "include/private/SkSemaphore.h"

#if !defined(__has_feature)
    #define __has_feature(x) 0
#endif

#if __has_feature(thread_sanitizer)

    /* Report that a lock has been created at address "lock". */
    #define ANNOTATE_RWLOCK_CREATE(lock) \
        AnnotateRWLockCreate(__FILE__, __LINE__, lock)

    /* Report that the lock at address "lock" is about to be destroyed. */
    #define ANNOTATE_RWLOCK_DESTROY(lock) \
        AnnotateRWLockDestroy(__FILE__, __LINE__, lock)

    /* Report that the lock at address "lock" has been acquired.
       is_w=1 for writer lock, is_w=0 for reader lock. */
    #define ANNOTATE_RWLOCK_ACQUIRED(lock, is_w) \
        AnnotateRWLockAcquired(__FILE__, __LINE__, lock, is_w)

    /* Report that the lock at address "lock" is about to be released. */
    #define ANNOTATE_RWLOCK_RELEASED(lock, is_w) \
      AnnotateRWLockReleased(__FILE__, __LINE__, lock, is_w)

    #if defined(DYNAMIC_ANNOTATIONS_WANT_ATTRIBUTE_WEAK)
        #if defined(__GNUC__)
            #define DYNAMIC_ANNOTATIONS_ATTRIBUTE_WEAK __attribute__((weak))
        #else
            /* TODO(glider): for Windows support we may want to change this macro in order
               to prepend __declspec(selectany) to the annotations' declarations. */
            #error weak annotations are not supported for your compiler
        #endif
    #else
        #define DYNAMIC_ANNOTATIONS_ATTRIBUTE_WEAK
    #endif

    extern "C" {
    void AnnotateRWLockCreate(
        const char *file, int line,
        const volatile void *lock) DYNAMIC_ANNOTATIONS_ATTRIBUTE_WEAK;
    void AnnotateRWLockDestroy(
        const char *file, int line,
        const volatile void *lock) DYNAMIC_ANNOTATIONS_ATTRIBUTE_WEAK;
    void AnnotateRWLockAcquired(
        const char *file, int line,
        const volatile void *lock, long is_w) DYNAMIC_ANNOTATIONS_ATTRIBUTE_WEAK;
    void AnnotateRWLockReleased(
        const char *file, int line,
        const volatile void *lock, long is_w) DYNAMIC_ANNOTATIONS_ATTRIBUTE_WEAK;
    }

#else

    #define ANNOTATE_RWLOCK_CREATE(lock)
    #define ANNOTATE_RWLOCK_DESTROY(lock)
    #define ANNOTATE_RWLOCK_ACQUIRED(lock, is_w)
    #define ANNOTATE_RWLOCK_RELEASED(lock, is_w)

#endif

#ifdef SK_DEBUG

    #include "include/private/SkTDArray.h"
    #include "include/private/SkThreadID.h"

    class SkSharedMutex::ThreadIDSet {
    public:
        // Returns true if threadID is in the set.
        bool find(SkThreadID threadID) const {
            for (auto& t : fThreadIDs) {
                if (t == threadID) return true;
            }
            return false;
        }

        // Returns true if did not already exist.
        bool tryAdd(SkThreadID threadID) {
            for (auto& t : fThreadIDs) {
                if (t == threadID) return false;
            }
            fThreadIDs.append(1, &threadID);
            return true;
        }
        // Returns true if already exists in Set.
        bool tryRemove(SkThreadID threadID) {
            for (int i = 0; i < fThreadIDs.count(); ++i) {
                if (fThreadIDs[i] == threadID) {
                    fThreadIDs.remove(i);
                    return true;
                }
            }
            return false;
        }

        void swap(ThreadIDSet& other) {
            fThreadIDs.swap(other.fThreadIDs);
        }

        int count() const {
            return fThreadIDs.count();
        }

    private:
        SkTDArray<SkThreadID> fThreadIDs;
    };

    SkSharedMutex::SkSharedMutex()
        : fCurrentShared(new ThreadIDSet)
        , fWaitingExclusive(new ThreadIDSet)
        , fWaitingShared(new ThreadIDSet){
        ANNOTATE_RWLOCK_CREATE(this);
    }

    SkSharedMutex::~SkSharedMutex() {  ANNOTATE_RWLOCK_DESTROY(this); }

    void SkSharedMutex::acquire() {
        SkThreadID threadID(SkGetThreadID());
        int currentSharedCount;
        int waitingExclusiveCount;
        {
            SkAutoMutexExclusive l(fMu);

            SkASSERTF(!fCurrentShared->find(threadID),
                      "Thread %lx already has an shared lock\n", threadID);

            if (!fWaitingExclusive->tryAdd(threadID)) {
                SkDEBUGFAILF("Thread %lx already has an exclusive lock\n", threadID);
            }

            currentSharedCount = fCurrentShared->count();
            waitingExclusiveCount = fWaitingExclusive->count();
        }

        if (currentSharedCount > 0 || waitingExclusiveCount > 1) {
            fExclusiveQueue.wait();
        }

        ANNOTATE_RWLOCK_ACQUIRED(this, 1);
    }

    // Implementation Detail:
    // The shared threads need two separate queues to keep the threads that were added after the
    // exclusive lock separate from the threads added before.
    void SkSharedMutex::release() {
        ANNOTATE_RWLOCK_RELEASED(this, 1);
        SkThreadID threadID(SkGetThreadID());
        int sharedWaitingCount;
        int exclusiveWaitingCount;
        int sharedQueueSelect;
        {
            SkAutoMutexExclusive l(fMu);
            SkASSERT(0 == fCurrentShared->count());
            if (!fWaitingExclusive->tryRemove(threadID)) {
                SkDEBUGFAILF("Thread %lx did not have the lock held.\n", threadID);
            }
            exclusiveWaitingCount = fWaitingExclusive->count();
            sharedWaitingCount = fWaitingShared->count();
            fWaitingShared.swap(fCurrentShared);
            sharedQueueSelect = fSharedQueueSelect;
            if (sharedWaitingCount > 0) {
                fSharedQueueSelect = 1 - fSharedQueueSelect;
            }
        }

        if (sharedWaitingCount > 0) {
            fSharedQueue[sharedQueueSelect].signal(sharedWaitingCount);
        } else if (exclusiveWaitingCount > 0) {
            fExclusiveQueue.signal();
        }
    }

    void SkSharedMutex::assertHeld() const {
        SkThreadID threadID(SkGetThreadID());
        SkAutoMutexExclusive l(fMu);
        SkASSERT(0 == fCurrentShared->count());
        SkASSERT(fWaitingExclusive->find(threadID));
    }

    void SkSharedMutex::acquireShared() {
        SkThreadID threadID(SkGetThreadID());
        int exclusiveWaitingCount;
        int sharedQueueSelect;
        {
            SkAutoMutexExclusive l(fMu);
            exclusiveWaitingCount = fWaitingExclusive->count();
            if (exclusiveWaitingCount > 0) {
                if (!fWaitingShared->tryAdd(threadID)) {
                    SkDEBUGFAILF("Thread %lx was already waiting!\n", threadID);
                }
            } else {
                if (!fCurrentShared->tryAdd(threadID)) {
                    SkDEBUGFAILF("Thread %lx already holds a shared lock!\n", threadID);
                }
            }
            sharedQueueSelect = fSharedQueueSelect;
        }

        if (exclusiveWaitingCount > 0) {
            fSharedQueue[sharedQueueSelect].wait();
        }

        ANNOTATE_RWLOCK_ACQUIRED(this, 0);
    }

    void SkSharedMutex::releaseShared() {
        ANNOTATE_RWLOCK_RELEASED(this, 0);
        SkThreadID threadID(SkGetThreadID());

        int currentSharedCount;
        int waitingExclusiveCount;
        {
            SkAutoMutexExclusive l(fMu);
            if (!fCurrentShared->tryRemove(threadID)) {
                SkDEBUGFAILF("Thread %lx does not hold a shared lock.\n", threadID);
            }
            currentSharedCount = fCurrentShared->count();
            waitingExclusiveCount = fWaitingExclusive->count();
        }

        if (0 == currentSharedCount && waitingExclusiveCount > 0) {
            fExclusiveQueue.signal();
        }
    }

    void SkSharedMutex::assertHeldShared() const {
        SkThreadID threadID(SkGetThreadID());
        SkAutoMutexExclusive l(fMu);
        SkASSERT(fCurrentShared->find(threadID));
    }

#else

    // The fQueueCounts fields holds many counts in an int32_t in order to make managing them atomic.
    // These three counts must be the same size, so each gets 10 bits. The 10 bits represent
    // the log of the count which is 1024.
    //
    // The three counts held in fQueueCounts are:
    // * Shared - the number of shared lock holders currently running.
    // * WaitingExclusive - the number of threads waiting for an exclusive lock.
    // * WaitingShared - the number of threads waiting to run while waiting for an exclusive thread
    //   to finish.
    static const int kLogThreadCount = 10;

    enum {
        kSharedOffset          = (0 * kLogThreadCount),
        kWaitingExlusiveOffset = (1 * kLogThreadCount),
        kWaitingSharedOffset   = (2 * kLogThreadCount),
        kSharedMask            = ((1 << kLogThreadCount) - 1) << kSharedOffset,
        kWaitingExclusiveMask  = ((1 << kLogThreadCount) - 1) << kWaitingExlusiveOffset,
        kWaitingSharedMask     = ((1 << kLogThreadCount) - 1) << kWaitingSharedOffset,
    };

    SkSharedMutex::SkSharedMutex() : fQueueCounts(0) { ANNOTATE_RWLOCK_CREATE(this); }
    SkSharedMutex::~SkSharedMutex() {  ANNOTATE_RWLOCK_DESTROY(this); }
    void SkSharedMutex::acquire() {
        // Increment the count of exclusive queue waiters.
        int32_t oldQueueCounts = fQueueCounts.fetch_add(1 << kWaitingExlusiveOffset,
                                                        std::memory_order_acquire);

        // If there are no other exclusive waiters and no shared threads are running then run
        // else wait.
        if ((oldQueueCounts & kWaitingExclusiveMask) > 0 || (oldQueueCounts & kSharedMask) > 0) {
            fExclusiveQueue.wait();
        }
        ANNOTATE_RWLOCK_ACQUIRED(this, 1);
    }

    void SkSharedMutex::release() {
        ANNOTATE_RWLOCK_RELEASED(this, 1);

        int32_t oldQueueCounts = fQueueCounts.load(std::memory_order_relaxed);
        int32_t waitingShared;
        int32_t newQueueCounts;
        do {
            newQueueCounts = oldQueueCounts;

            // Decrement exclusive waiters.
            newQueueCounts -= 1 << kWaitingExlusiveOffset;

            // The number of threads waiting to acquire a shared lock.
            waitingShared = (oldQueueCounts & kWaitingSharedMask) >> kWaitingSharedOffset;

            // If there are any move the counts of all the shared waiters to actual shared. They are
            // going to run next.
            if (waitingShared > 0) {

                // Set waiting shared to zero.
                newQueueCounts &= ~kWaitingSharedMask;

                // Because this is the exclusive release, then there are zero readers. So, the bits
                // for shared locks should be zero. Since those bits are zero, we can just |= in the
                // waitingShared count instead of clearing with an &= and then |= the count.
                newQueueCounts |= waitingShared << kSharedOffset;
            }

        } while (!fQueueCounts.compare_exchange_strong(oldQueueCounts, newQueueCounts,
                                                       std::memory_order_release,
                                                       std::memory_order_relaxed));

        if (waitingShared > 0) {
            // Run all the shared.
            fSharedQueue.signal(waitingShared);
        } else if ((newQueueCounts & kWaitingExclusiveMask) > 0) {
            // Run a single exclusive waiter.
            fExclusiveQueue.signal();
        }
    }

    void SkSharedMutex::acquireShared() {
        int32_t oldQueueCounts = fQueueCounts.load(std::memory_order_relaxed);
        int32_t newQueueCounts;
        do {
            newQueueCounts = oldQueueCounts;
            // If there are waiting exclusives then this shared lock waits else it runs.
            if ((newQueueCounts & kWaitingExclusiveMask) > 0) {
                newQueueCounts += 1 << kWaitingSharedOffset;
            } else {
                newQueueCounts += 1 << kSharedOffset;
            }
        } while (!fQueueCounts.compare_exchange_strong(oldQueueCounts, newQueueCounts,
                                                       std::memory_order_acquire,
                                                       std::memory_order_relaxed));

        // If there are waiting exclusives, then this shared waits until after it runs.
        if ((newQueueCounts & kWaitingExclusiveMask) > 0) {
            fSharedQueue.wait();
        }
        ANNOTATE_RWLOCK_ACQUIRED(this, 0);

    }

    void SkSharedMutex::releaseShared() {
        ANNOTATE_RWLOCK_RELEASED(this, 0);

        // Decrement the shared count.
        int32_t oldQueueCounts = fQueueCounts.fetch_sub(1 << kSharedOffset,
                                                        std::memory_order_release);

        // If shared count is going to zero (because the old count == 1) and there are exclusive
        // waiters, then run a single exclusive waiter.
        if (((oldQueueCounts & kSharedMask) >> kSharedOffset) == 1
            && (oldQueueCounts & kWaitingExclusiveMask) > 0) {
            fExclusiveQueue.signal();
        }
    }

#endif
