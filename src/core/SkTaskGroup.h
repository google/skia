/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTaskGroup_DEFINED
#define SkTaskGroup_DEFINED

#include "include/core/SkExecutor.h"
#include "include/core/SkTypes.h"
#include "include/private/SkNoncopyable.h"
#include "src/core/SkExecutors.h"

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>

// This class simply wraps an SkExecutor and keeps a count of the extant work.
// It adds a wait() method which blocks until all the added work is complete.
class SkTaskGroup : SkNoncopyable {
public:
    // Tasks added to this SkTaskGroup will run on its executor.
    explicit SkTaskGroup(SkExecutor& executor = SkExecutors::GetDefault());
    ~SkTaskGroup() { this->wait(); }

    // Add a task to this SkTaskGroup.
    void add(std::function<void()> fn, int workList = 0);

    // Returns true if all Tasks previously add()ed to this SkTaskGroup have run.
    // It is safe to reuse this SkTaskGroup once done().
    bool done() const;

    // Block until done().
    void wait();

private:
    std::atomic<int32_t> fPending;
    SkExecutor&          fExecutor;
};

#endif//SkTaskGroup_DEFINED
