/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkExecutors_DEFINED
#define SkExecutors_DEFINED

#include "include/core/SkExecutor.h"
#include "include/core/SkTypes.h"

#include <memory>

namespace SkExecutors {
    // Create a thread pool SkExecutor with a fixed thread count, by default the number of cores.
    std::unique_ptr<SkExecutor> MakeFIFOThreadPool(int threads = 0,
                                                   bool allowBorrowing = true);
    std::unique_ptr<SkExecutor> MakeLIFOThreadPool(int threads = 0,
                                                   bool allowBorrowing = true);

    // A work list is the queue or stack to which work is added and removed. The above two
    // factory functions create an executor with only one list while the following two factories
    // can create executors with multiple work lists. Having multiple work lists allows for
    // prioritization with work being pulled from the lower indexed work lists first - with
    // work list '0' being the highest priority.
    std::unique_ptr<SkExecutor> MakeMultiListFIFOThreadPool(int numWorkLists,
                                                            int threads = 0,
                                                            bool allowBorrowing = true);
    std::unique_ptr<SkExecutor> MakeMultiListLIFOThreadPool(int numWorkLists,
                                                            int threads = 0,
                                                            bool allowBorrowing = true);

    // There is always a default SkExecutor available by calling SkExecutor::GetDefault().
    SkExecutor& GetDefault();
    void SetDefault(SkExecutor*);  // Does not take ownership.  Not thread safe.

    // A convenience for testing tools.
    // Creates and owns a thread pool, and passes it to SkExecutor::SetDefault().
    struct Enabler {
        explicit Enabler(int threads = -1);  // -1 -> num_cores, 0 -> noop
        ~Enabler();
        std::unique_ptr<SkExecutor> fThreadPool;
    };
}

#endif // SkExecutors_DEFINED
