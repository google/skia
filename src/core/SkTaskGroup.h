/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTaskGroup_DEFINED
#define SkTaskGroup_DEFINED

#include <functional>

#include "SkTypes.h"
#include "SkAtomics.h"
#include "SkTemplates.h"

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
    void add(std::function<void(void)> fn);

    // Add a batch of N tasks, all calling fn with different arguments.
    void batch(int N, std::function<void(int)> fn);

    // Block until all Tasks previously add()ed to this SkTaskGroup have run.
    // You may safely reuse this SkTaskGroup after wait() returns.
    void wait();

private:
    SkAtomic<int32_t> fPending;
};

#endif//SkTaskGroup_DEFINED
