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

void SkTaskGroup::add(std::function<void(void)> fn) {
    fPending.fetch_add(+1, std::memory_order_relaxed);
    fExecutor.add([this, fn{std::move(fn)}] {
        fn();
        fPending.fetch_add(-1, std::memory_order_release);
    });
}

void SkTaskGroup::batch(int N, std::function<void(int)> fn) {
    // TODO: I really thought we had some sort of more clever chunking logic.
    fPending.fetch_add(+N, std::memory_order_relaxed);
    for (int i = 0; i < N; i++) {
        fExecutor.add([=] {
            fn(i);
            fPending.fetch_add(-1, std::memory_order_release);
        });
    }
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
