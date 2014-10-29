/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTaskGroup_DEFINED
#define SkTaskGroup_DEFINED

#include "SkTypes.h"

struct SkRunnable;

class SkTaskGroup : SkNoncopyable {
public:
    // Create one of these in main() to enable SkTaskGroups globally.
    struct Enabler : SkNoncopyable {
        explicit Enabler(int threads = -1);  // Default is system-reported core count.
        ~Enabler();
    };

    SkTaskGroup();
    ~SkTaskGroup() { this->wait(); }

    // Add a task to this SkTaskGroup.  It will likely run on another thread.
    // Neither add() method takes owership of any of its parameters.
    void add(SkRunnable*);

    template <typename T>
    void add(void (*fn)(T*), T* arg) { this->add((void_fn)fn, (void*)arg); }

    // Add a batch of N tasks, all calling fn with different arguments.
    // Equivalent to a loop over add(fn, arg), but with perhaps less synchronization overhead.
    template <typename T>
    void batch(void (*fn)(T*), T* args, int N) { this->batch((void_fn)fn, args, N, sizeof(T)); }

    // Block until all Tasks previously add()ed to this SkTaskGroup have run.
    // You may safely reuse this SkTaskGroup after wait() returns.
    void wait();

private:
    typedef void(*void_fn)(void*);

    void add  (void_fn, void* arg);
    void batch(void_fn, void* args, int N, size_t stride);

    /*atomic*/ int32_t fPending;
};

#endif//SkTaskGroup_DEFINED
