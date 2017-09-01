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

class SkExecutor {
public:
    virtual ~SkExecutor();

    // Create a thread pool SkExecutor with a fixed thread count, by default the number of cores.
    static std::unique_ptr<SkExecutor> MakeFIFOThreadPool(int threads = 0);
    static std::unique_ptr<SkExecutor> MakeLIFOThreadPool(int threads = 0);

    // There is always a default SkExecutor available by calling SkExecutor::GetDefault().
    static SkExecutor& GetDefault();
    static void SetDefault(SkExecutor*);  // Does not take ownership.  Not thread safe.

    // Add work to execute.
    virtual void add(std::function<void(void)>) = 0;

    // If it makes sense for this executor, use this thread to execute work for a little while.
    virtual void borrow() {}
};

#endif//SkExecutor_DEFINED
