/*
 * Copyright 2017 Google LLC
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

    // Add work to execute.
    virtual void add(std::function<void(void)> fn, int /* workList */) { this->add(std::move(fn)); }

    // deprecated
    virtual void add(std::function<void(void)>) = 0;

    // If it makes sense for this executor, use this thread to execute work for a little while.
    virtual void borrow() {}

protected:
    SkExecutor() = default;
    SkExecutor(const SkExecutor&) = delete;
    SkExecutor& operator=(const SkExecutor&) = delete;
};

#endif // SkExecutor_DEFINED
