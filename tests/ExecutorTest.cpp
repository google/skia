/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkExecutor.h"
#include "src/core/SkSpinlock.h"
#include "src/core/SkTaskGroup.h"
#include "tests/Test.h"

#include <array>
#include <atomic>
#include <chrono>
#include <thread>

namespace {

constexpr int kNumWorkLists = 2;
constexpr int kNumThreads = 4;

enum Priority {
    kHighPriority = 0,
    kLowPriority = 1,
};

// This utility just collects a set kMaxCount ints in a thread-safe manner. It has a
// predicate to check if the results match expectations.
class Collector {
public:
    static constexpr int kMaxCount = 200;

    void insert(int i) SK_EXCLUDES(fSpinLock) {
        SkAutoSpinlock lock{fSpinLock};

        if (fCount >= kMaxCount) {
            return;
        }

        fData[fCount++] = i;
    }

    int count() const SK_EXCLUDES(fSpinLock) {
        SkAutoSpinlock lock{fSpinLock};
        return fCount;
    }

#if defined(SK_DEBUG)
    void dump() const SK_EXCLUDES(fSpinLock) {
        SkAutoSpinlock lock{fSpinLock};

        for (int i = 0; i < fCount; ++i) {
            SkDebugf("%d ", fData[i]);
        }
        SkDebugf("\n");
    }
#endif

    // For this following tests we expect that all the high priority work is completed
    // before the low priority work. The length of the two task types is set up so,
    // even if low priority work is picked up while the high priority tasks are finishing,
    // all the high priority task should still finish before the low priority ones.
    bool dataIsInOrder() const SK_EXCLUDES(fSpinLock) {
        SkAutoSpinlock lock{fSpinLock};

        for (int i = 0; i < fCount; ++i) {
            if (i < 100) {
                if (fData[i] >= 100) { return false; }
            } else {
                if (fData[i] < 100) { return false; }
            }
        }
        return true;
    }

private:
    mutable SkSpinlock fSpinLock;

    int fCount SK_GUARDED_BY(fSpinLock) = 0;
    std::array<int, kMaxCount> fData SK_GUARDED_BY(fSpinLock);
};

// Make sure all high priority work is started before the low priority work is begun
template <typename ExecutorType>
void priority_test(skiatest::Reporter* reporter, std::unique_ptr<ExecutorType> executor);

template <>
void priority_test<SkFunctionalExecutor>(skiatest::Reporter* reporter,
                                         std::unique_ptr<SkFunctionalExecutor> executor) {
    SkTaskGroup taskGroup(*executor);
    Collector collector;

    for (int i = 0; i < 100; ++i) {
        taskGroup.add([&collector, i]() {
            collector.insert(i);
            std::this_thread::sleep_for(std::chrono::microseconds(50));
        }, kHighPriority);
    }
    for (int i = 100; i < 200; ++i) {
        taskGroup.add([&collector, i]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            collector.insert(i);
        }, kLowPriority);
    }

    taskGroup.wait();

    REPORTER_ASSERT(reporter, collector.count() == Collector::kMaxCount);
    REPORTER_ASSERT(reporter, collector.dataIsInOrder());
}

struct CallbackContext {
    Collector* fCollector;
    std::atomic<int32_t>* fPending;
    int fCount;
    Priority fPriority;
};

void callback(void* contextIn) {
    CallbackContext* context = static_cast<CallbackContext*>(contextIn);

    if (context->fPriority == Priority::kLowPriority) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    context->fCollector->insert(context->fCount);
    if (context->fPriority == Priority::kHighPriority) {
        std::this_thread::sleep_for(std::chrono::microseconds(50));
    }
    context->fPending->fetch_add(-1, std::memory_order_release);
}

template <>
void priority_test<SkCallbackExecutor>(skiatest::Reporter* reporter,
                                       std::unique_ptr<SkCallbackExecutor> executor) {
    Collector collector;
    std::atomic<int32_t> pending{0};

    std::array<CallbackContext, 200> contexts;

    for (int i = 0; i < 100; ++i) {
        contexts[i].fCollector = &collector;
        contexts[i].fPending = &pending;
        contexts[i].fCount = i;
        contexts[i].fPriority = Priority::kHighPriority;

        pending.fetch_add(+1, std::memory_order_relaxed);
        executor->add({ callback, &contexts[i] }, Priority::kHighPriority);
    }
    for (int i = 100; i < 200; ++i) {
        contexts[i].fCollector = &collector;
        contexts[i].fPending = &pending;
        contexts[i].fCount = i;
        contexts[i].fPriority = Priority::kLowPriority;

        pending.fetch_add(+1, std::memory_order_relaxed);
        executor->add({ callback, &contexts[i] }, Priority::kLowPriority);
    }

    while (pending.load(std::memory_order_acquire) > 0) {
        executor->borrow();
    }
    executor.reset();

    REPORTER_ASSERT(reporter, collector.count() == Collector::kMaxCount);
    REPORTER_ASSERT(reporter, collector.dataIsInOrder());
}

template <typename ExecutorType>
std::unique_ptr<ExecutorType> make_2x_FIFO() {
    return SkExecutors<ExecutorType>::MakeMultiListFIFOThreadPool(
        kNumWorkLists, kNumThreads, /* allowBorrowing= */ false);
}

template <typename ExecutorType>
std::unique_ptr<ExecutorType> make_2x_LIFO() {
    return SkExecutors<ExecutorType>::MakeMultiListLIFOThreadPool(
        kNumWorkLists, kNumThreads, /* allowBorrowing= */ false);
}

template <typename ExecutorType>
std::unique_ptr<ExecutorType> make_1x_FIFO() {
    return SkExecutors<ExecutorType>::MakeFIFOThreadPool(kNumThreads, /* allowBorrowing= */ false);
}

template <typename ExecutorType>
void run_test(skiatest::Reporter* reporter) {
    using FactoryFunc = std::unique_ptr<ExecutorType> (*)();

    FactoryFunc factories[] = { make_2x_FIFO<ExecutorType>,
                                make_2x_LIFO<ExecutorType>,
                                make_1x_FIFO<ExecutorType> };

    // 1x_LIFO is incompatible w/ how this test is structured
    for (FactoryFunc makeExecutor : factories) {
        priority_test<ExecutorType>(reporter, makeExecutor());
    }
}

} // anonymous namespace

DEF_TEST(ExecutorTest_StdFunction, reporter) {
    run_test<SkFunctionalExecutor>(reporter);
}

DEF_TEST(ExecutorTest_Callback, reporter) {
    run_test<SkCallbackExecutor>(reporter);
}
