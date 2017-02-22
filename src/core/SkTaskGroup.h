/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTaskGroup_DEFINED
#define SkTaskGroup_DEFINED

#include "SkAtomics.h"
#include "SkExecutor.h"
#include "SkTypes.h"
#include <functional>

class SkTaskGroup : SkNoncopyable {
public:
    // Tasks added to this SkTaskGroup will run on this executor.
    explicit SkTaskGroup(SkExecutor& executor = SkExecutor::GetDefault());
    ~SkTaskGroup() { this->wait(); }

    // Add a task to this SkTaskGroup.
    void add(std::function<void(void)> fn);

    // Add a batch of N tasks, all calling fn with different arguments.
    void batch(int N, std::function<void(int)> fn);

    // Block until all Tasks previously add()ed to this SkTaskGroup have run.
    // You may safely reuse this SkTaskGroup after wait() returns.
    void wait();

    // Creates a thread pool and sets it to be the default executor.
    struct Enabler {
        Enabler(int threads);
        std::unique_ptr<SkExecutor> fExecutor;
    };

private:
    SkAtomic<int32_t> fPending;
    SkExecutor&       fExecutor;
};

#endif//SkTaskGroup_DEFINED
