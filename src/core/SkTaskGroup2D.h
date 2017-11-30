/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTaskGroup2D_DEFINED
#define SkTaskGroup2D_DEFINED

#include "SkTaskGroup.h"

#include <mutex>
#include <vector>

// A 2D grid (height rows x width columns) of tasks.
//
// The task on row i and column j is abstracted as Work2D(i, j). We guarantee that the task on the
// same row will be executed in order (i.e., Work2D(1, 1) is guaranteed to finish before calling
// Work2D(1, 2)). Tasks in different rows can happen in any order.
//
// The height (number of rows) is fixed. The width (number of columns) may be dynamically expanded.
//
// The tasks will eventually be executed on the executor with threadCnt number of hardware threads.
class SkTaskGroup2D {
public:
    using Work2D = std::function<void(int, int)>;

    SkTaskGroup2D(Work2D&& work, int height, SkExecutor* executor, int threadCnt)
            : fWork(work), fHeight(height), fThreadCnt(threadCnt), fIsFinishing(false), fWidth(0)
            , fThreadsGroup(new SkTaskGroup(*executor)) {}

    virtual ~SkTaskGroup2D() {}

    virtual void addColumn(); // Add a new column of tasks.

    void start(); // start threads to execute tasks
    void finish(); // wait and finish all tasks (no more tasks can be added after calling this)

    SK_ALWAYS_INLINE bool isFinishing() const {
        return fIsFinishing.load(std::memory_order_relaxed);
    }

protected:
    static constexpr int MAX_CACHE_LINE = 64;

    // Finish all tasks on the threadId and then return.
    virtual void work(int threadId) = 0;

    Work2D      fWork; // fWork(i, j) is the task to be done on row i and column j
    const int   fHeight;
    const int   fThreadCnt;

    std::atomic<bool>   fIsFinishing;
    std::atomic<int>    fWidth;

    std::unique_ptr<SkTaskGroup> fThreadsGroup;
};

// A simple spinning task group that assumes height equals threadCnt.
class SkSpinningTaskGroup2D final : public SkTaskGroup2D {
public:
    SkSpinningTaskGroup2D(Work2D&& w, int h, SkExecutor* x, int t)
            : SkTaskGroup2D(std::move(w), h, x, t), fRowData(h) {
        SkASSERT(h == t); // height must be equal to threadCnt
    }

protected:
    void work(int threadId) override;

private:
    // alignas(MAX_CACHE_LINE) to avoid false sharing by cache lines
    struct alignas(MAX_CACHE_LINE) RowData {
        RowData() : fNextColumn(0) {}

        int fNextColumn; // next column index to be executed
    };

    std::vector<RowData>  fRowData;
};

class SkFlexibleTaskGroup2D final : public SkTaskGroup2D {
public:
    SkFlexibleTaskGroup2D(Work2D&&, int, SkExecutor*, int);

protected:
    void work(int threadId) override;

private:
    // alignas(MAX_CACHE_LINE) to avoid false sharing by cache lines
    struct alignas(MAX_CACHE_LINE) RowData {
        RowData() : fNextColumn(0) {}

        int         fNextColumn; // next column index to be executed
        std::mutex  fMutex;      // the mutex for the thread to acquire
    };

    struct alignas(MAX_CACHE_LINE) ThreadData {
        ThreadData() : fRowIndex(0) {}

        int fRowIndex; // the row that the current thread is working on
    };

    std::vector<RowData>    fRowData;
    std::vector<ThreadData> fThreadData;
};

#endif//SkTaskGroup2D_DEFINED
