/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkExecutor.h"
#include "src/base/SkSpinlock.h"
#include "src/core/SkTaskGroup.h"
#include "tests/Test.h"

#include <thread>

namespace {

constexpr int kNumWorkLists = 2;
constexpr int kNumThreads = 4;
constexpr int kHighPriority = 0;
constexpr int kLowPriority = 1;

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
    int fData[kMaxCount] SK_GUARDED_BY(fSpinLock);
};

// Make sure all high priority work is started before the low priority work is begun
void priority_test(skiatest::Reporter* reporter, std::unique_ptr<SkExecutor> executor) {
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

// Test out discarding
void discard_test(skiatest::Reporter* reporter, std::unique_ptr<SkExecutor> executor) {
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
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    taskGroup.discardAllPendingWork();

    taskGroup.wait();

    // We should've shaved off some work
    REPORTER_ASSERT(reporter, collector.count() < Collector::kMaxCount);
    // But, the work that got done should still be in order
    REPORTER_ASSERT(reporter, collector.dataIsInOrder());
}

std::unique_ptr<SkExecutor> make_2x_FIFO() {
    return SkExecutor::MakeMultiListFIFOThreadPool(
        kNumWorkLists, kNumThreads, /* allowBorrowing= */ false);
}

std::unique_ptr<SkExecutor> make_2x_LIFO() {
    return SkExecutor::MakeMultiListLIFOThreadPool(
        kNumWorkLists, kNumThreads, /* allowBorrowing= */ false);
}

std::unique_ptr<SkExecutor> make_1x_FIFO() {
    return SkExecutor::MakeFIFOThreadPool(kNumThreads, /* allowBorrowing= */ false);
}

} // anonymous namespace

DEF_TEST(ExecutorTest, reporter) {
    // 1x_LIFO is incompatible w/ how this test is structured
    for (auto makeExecutor : { make_2x_FIFO, make_2x_LIFO, make_1x_FIFO }) {
        priority_test(reporter, makeExecutor());
        discard_test(reporter, makeExecutor());
    }
}
