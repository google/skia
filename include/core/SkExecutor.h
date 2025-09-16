/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkExecutor_DEFINED
#define SkExecutor_DEFINED

#include <functional>
#include <memory>
#include "include/core/SkTypes.h"

class SK_API SkExecutor {
public:
    virtual ~SkExecutor();

    // Create a thread pool SkExecutor with a fixed thread count, by default the number of cores.
    static std::unique_ptr<SkExecutor> MakeFIFOThreadPool(int threads = 0,
                                                          bool allowBorrowing = true);
    static std::unique_ptr<SkExecutor> MakeLIFOThreadPool(int threads = 0,
                                                          bool allowBorrowing = true);

    // A work list is the queue or stack to which work is added and removed. The above two
    // factory functions create an executor with only one list while the following two factories
    // can create executors with multiple work lists. Having multiple work lists allows for
    // prioritization with work being pulled from the lower indexed work lists first - with
    // work list '0' being the highest priority.
    static std::unique_ptr<SkExecutor> MakeMultiListFIFOThreadPool(int numWorkLists,
                                                                   int threads = 0,
                                                                   bool allowBorrowing = true);
    static std::unique_ptr<SkExecutor> MakeMultiListLIFOThreadPool(int numWorkLists,
                                                                   int threads = 0,
                                                                   bool allowBorrowing = true);

    // There is always a default SkExecutor available by calling SkExecutor::GetDefault().
    static SkExecutor& GetDefault();
    static void SetDefault(SkExecutor*);  // Does not take ownership.  Not thread safe.

    // Add work to execute.
    virtual void add(std::function<void(void)> fn, int /* workList */) { this->add(std::move(fn)); }

    // deprecated
    virtual void add(std::function<void(void)>) = 0;

    // Returns the number of discarded work units
    virtual int discardAllPendingWork() { return 0; }

    // If it makes sense for this executor, use this thread to execute work for a little while.
    virtual void borrow() {}

protected:
    SkExecutor() = default;
    SkExecutor(const SkExecutor&) = delete;
    SkExecutor& operator=(const SkExecutor&) = delete;
};

#endif//SkExecutor_DEFINED
