/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/core/SkTaskGroup.h"

#include "include/core/SkExecutor.h"

#include <type_traits>
#include <utility>

SkTaskGroup::SkTaskGroup(SkExecutor& executor) : fPending(0), fExecutor(executor) {}

void SkTaskGroup::add(std::function<void()> fn, int workList) {
    fPending.fetch_add(+1, std::memory_order_relaxed);
    fExecutor.add([this, fn{std::move(fn)}] {
                      fn();
                      fPending.fetch_add(-1, std::memory_order_release);
                  },
                  workList);
}

bool SkTaskGroup::done() const {
    return fPending.load(std::memory_order_acquire) == 0;
}

void SkTaskGroup::wait() {
    // Actively help the executor do work until our task group is done.
    // This lets SkTaskGroups nest arbitrarily deep on a single SkExecutor:
    // no thread ever blocks waiting for others to do its work.
    // (We may end up doing work that's not part of our task group.  That's fine.)
    while (!this->done()) {
        fExecutor.borrow();
    }
}

SkTaskGroup::Enabler::Enabler(int threads) {
    if (threads) {
        fThreadPool = SkExecutor::MakeLIFOThreadPool(threads);
        SkExecutor::SetDefault(fThreadPool.get());
    }
}
