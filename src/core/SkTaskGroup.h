/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTaskGroup_DEFINED
#define SkTaskGroup_DEFINED

#include "SkExecutor.h"
#include "SkTypes.h"
#include <atomic>
#include <functional>

class SkTaskGroup : SkNoncopyable {
public:
    // Tasks added to this SkTaskGroup will run on its executor.
    explicit SkTaskGroup(SkExecutor& executor = SkExecutor::GetDefault());
    ~SkTaskGroup() { this->wait(); }

    // Add a task to this SkTaskGroup.
    void add(std::function<void(void)> fn);

    // Add a batch of N tasks, all calling fn with different arguments.
    void batch(int N, std::function<void(int)> fn);

    // Block until all Tasks previously add()ed to this SkTaskGroup have run.
    // You may safely reuse this SkTaskGroup after wait() returns.
    void wait();

    // A convenience for testing tools.
    // Creates and owns a thread pool, and passes it to SkExecutor::SetDefault().
    struct Enabler {
        explicit Enabler(int threads = -1);  // -1 -> num_cores, 0 -> noop
        std::unique_ptr<SkExecutor> fThreadPool;
    };

private:
    std::atomic<int32_t> fPending;
    SkExecutor&          fExecutor;
};

#endif//SkTaskGroup_DEFINED
