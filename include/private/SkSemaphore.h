/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSemaphore_DEFINED
#define SkSemaphore_DEFINED

#include "../private/SkOnce.h"
#include "SkTypes.h"
#include <atomic>

class SkBaseSemaphore {
public:
    constexpr SkBaseSemaphore(int count = 0)
        : fCount(count), fOSSemaphore(nullptr) {}

    // Increment the counter n times.
    // Generally it's better to call signal(n) instead of signal() n times.
    void signal(int n = 1);

    // Decrement the counter by 1,
    // then if the counter is < 0, sleep this thread until the counter is >= 0.
    void wait();

    // If the counter is positive, decrement it by 1 and return true, otherwise return false.
    bool try_wait();

    // SkBaseSemaphore has no destructor.  Call this to clean it up.
    void cleanup();

private:
    // This implementation follows the general strategy of
    //     'A Lightweight Semaphore with Partial Spinning'
    // found here
    //     http://preshing.com/20150316/semaphores-are-surprisingly-versatile/
    // That article (and entire blog) are very much worth reading.
    //
    // We wrap an OS-provided semaphore with a user-space atomic counter that
    // lets us avoid interacting with the OS semaphore unless strictly required:
    // moving the count from >=0 to <0 or vice-versa, i.e. sleeping or waking threads.
    struct OSSemaphore;

    void osSignal(int n);
    void osWait();

    std::atomic<int> fCount;
    SkOnce           fOSSemaphoreOnce;
    OSSemaphore*     fOSSemaphore;
};

class SkSemaphore : public SkBaseSemaphore {
public:
    using SkBaseSemaphore::SkBaseSemaphore;
    ~SkSemaphore() { this->cleanup(); }
};

inline void SkBaseSemaphore::signal(int n) {
    int prev = fCount.fetch_add(n, std::memory_order_release);

    // We only want to call the OS semaphore when our logical count crosses
    // from <0 to >=0 (when we need to wake sleeping threads).
    //
    // This is easiest to think about with specific examples of prev and n.
    // If n == 5 and prev == -3, there are 3 threads sleeping and we signal
    // SkTMin(-(-3), 5) == 3 times on the OS semaphore, leaving the count at 2.
    //
    // If prev >= 0, no threads are waiting, SkTMin(-prev, n) is always <= 0,
    // so we don't call the OS semaphore, leaving the count at (prev + n).
    int toSignal = SkTMin(-prev, n);
    if (toSignal > 0) {
        this->osSignal(toSignal);
    }
}

inline void SkBaseSemaphore::wait() {
    // Since this fetches the value before the subtract, zero and below means that there are no
    // resources left, so the thread needs to wait.
    if (fCount.fetch_sub(1, std::memory_order_acquire) <= 0) {
        this->osWait();
    }
}

#endif//SkSemaphore_DEFINED
