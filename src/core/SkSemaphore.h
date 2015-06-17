/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSemaphore_DEFINED
#define SkSemaphore_DEFINED

#include "SkTypes.h"
#include "SkAtomics.h"

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

    // Increment the counter N times.
    // Generally it's better to call signal(N) instead of signal() N times.
    void signal(int N = 1);

    // Decrement the counter by 1,
    // then if the counter is <= 0, sleep this thread until the counter is > 0.
    void wait();

private:
    // This implementation follows the general strategy of
    //     'A Lightweight Semaphore with Partial Spinning'
    // found here
    //     http://preshing.com/20150316/semaphores-are-surprisingly-versatile/
    // That article (and entire blog) are very much worth reading.
    //
    // We wrap an OS-provided semaphore with a user-space atomic counter that
    // lets us avoid interacting with the OS semaphore unless strictly required:
    // moving the count from >0 to <=0 or vice-versa, i.e. sleeping or waking threads.
    struct OSSemaphore;

    SkAtomic<int>  fCount;
    OSSemaphore*   fOSSemaphore;
};

#endif//SkSemaphore_DEFINED
