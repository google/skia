/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSemaphore_DEFINED
#define SkSemaphore_DEFINED

#include "SkTypes.h"
#include "../private/SkAtomics.h"
#include "../private/SkOncePtr.h"

struct SkBaseSemaphore {

    // Increment the counter by 1.
    // This is a specialization for supporting SkMutex.
    void signal() {
        // Since this fetches the value before the add, 0 indicates that this thread is running and
        // no threads are waiting, -1 and below means that threads are waiting, but only signal 1
        // thread to run.
        if (sk_atomic_fetch_add(&fCount, 1, sk_memory_order_release) < 0) {
           this->osSignal(1);
        }
    }

    // Increment the counter N times.
    // Generally it's better to call signal(N) instead of signal() N times.
    void signal(int N);

    // Decrement the counter by 1,
    // then if the counter is <= 0, sleep this thread until the counter is > 0.
    void wait() {
        // Since this fetches the value before the subtract, zero and below means that there are no
        // resources left, so the thread needs to wait.
        if (sk_atomic_fetch_sub(&fCount, 1, sk_memory_order_acquire) <= 0) {
            this->osWait();
        }
    }

    struct OSSemaphore;

    void osSignal(int n);
    void osWait();
    void deleteSemaphore();

    // This implementation follows the general strategy of
    //     'A Lightweight Semaphore with Partial Spinning'
    // found here
    //     http://preshing.com/20150316/semaphores-are-surprisingly-versatile/
    // That article (and entire blog) are very much worth reading.
    //
    // We wrap an OS-provided semaphore with a user-space atomic counter that
    // lets us avoid interacting with the OS semaphore unless strictly required:
    // moving the count from >0 to <=0 or vice-versa, i.e. sleeping or waking threads.
    int                        fCount;
    SkBaseOncePtr<OSSemaphore> fOSSemaphore;
};

/**
 * SkSemaphore is a fast mostly-user-space semaphore.
 *
 * A semaphore is logically an atomic integer with a few special properties:
 *   - The integer always starts at 0.
 *   - You can only increment or decrement it, never read or write it.
 *   - Increment is spelled 'signal()'; decrement is spelled 'wait()'.
 *   - If a call to wait() decrements the counter to <= 0,
 *     the calling thread sleeps until another thread signal()s it back above 0.
 */
class SkSemaphore : SkNoncopyable {
public:
    // Initializes the counter to 0.
    // (Though all current implementations could start from an arbitrary value.)
    SkSemaphore();
    ~SkSemaphore();

    void wait();

    void signal(int n = 1);

private:
    SkBaseSemaphore fBaseSemaphore;
};

#endif//SkSemaphore_DEFINED
