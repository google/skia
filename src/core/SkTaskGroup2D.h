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

// The interface for doing work on a 2D grid with possible initialization on columns.
class SkWorkKernel2D {
public:
    // Return false iff the column needs initialization and such initialization is not finished yet.
    virtual bool work2D(int row, int column, int thread) = 0;

    // Return false if no initialization is done for this colum (e.g., it's already initialized; or
    // maybe some other thread is initializing the column).
    virtual bool initColumn(int column, int thread) = 0;

    virtual ~SkWorkKernel2D() {}
};

// A 2D grid (height rows x width columns) of tasks to be executed on a given executor with
// threadCnt number of threads.
//
// The height (number of rows) is fixed. The width (number of columns) may be dynamically expanded.
//
// The task on row i and column j is abstracted as work2D(i, j, t). Parameter t is the thread id and
// it shouldn't affect the work to be done. It's only used to allow some variables that are not
// thread safe and should be used exclusively by one thread (e.g., thread allocators). We guarantee
// that the task on the same row will be executed in order (i.e., work2D(1, 1, t) is guaranteed to
// finish before calling work2D(1, 2, t)). Tasks in different rows can happen in any order.
//
// There are also width number of init calls, one per column. work2D(i, j, t) may return false if
// column j requires initialization but it's not initialized yet. In that case, a thread t needs to
// call initColumn(j, t) once to unblock all rows that depend on the initialization of column j.
// (Again, t shouldn't affect the init work to be done; it's just for some non-thread-safe
// variables). The init calls have no order requirement so we can call them in any order.
//
// Multiple therads may try to init the same column j at the same time. InitFn is expected to handle
// this gracefully (e.g., let only one thread do the init and return immediately for other threads).
class SkTaskGroup2D {
public:
    SkTaskGroup2D(SkWorkKernel2D* kernel, int height, SkExecutor* executor, int threadCnt)
            : fKernel(kernel), fHeight(height), fThreadCnt(threadCnt), fIsFinishing(false)
            , fWidth(0), fThreadsGroup(new SkTaskGroup(*executor)) {}

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

    // Initialize a column that needs to be initialized. The parameter initCol is not thread safe
    // and should only be exclusively accessed by the working thread which will modify it to the
    // column that may need to be initialized next.
    void initAnUninitializedColumn(int& initCol, int threadId) {
        bool didSomeInit = false;
        while (initCol < fWidth && !didSomeInit) {
            didSomeInit = fKernel->initColumn(initCol++, threadId);
        }
    }

    SkWorkKernel2D*     fKernel;
    const int           fHeight;
    const int           fThreadCnt;

    std::atomic<bool>   fIsFinishing;
    std::atomic<int>    fWidth;

    std::unique_ptr<SkTaskGroup> fThreadsGroup;
};

// A simple spinning task group that assumes height equals threadCnt.
class SkSpinningTaskGroup2D final : public SkTaskGroup2D {
public:
    SkSpinningTaskGroup2D(SkWorkKernel2D* kernel, int h, SkExecutor* x, int t)
            : SkTaskGroup2D(kernel, h, x, t) {
        SkASSERT(h == t); // height must be equal to threadCnt
    }

protected:
    void work(int threadId) override;
};

class SkFlexibleTaskGroup2D final : public SkTaskGroup2D {
public:
    SkFlexibleTaskGroup2D(SkWorkKernel2D* kernel, int h, SkExecutor* x, int t)
            : SkTaskGroup2D(kernel, h, x, t), fRowData(h) {}

protected:
    void work(int threadId) override;

private:
    // alignas(MAX_CACHE_LINE) to avoid false sharing by cache lines
    struct alignas(MAX_CACHE_LINE) RowData {
        RowData() : fNextColumn(0) {}

        int         fNextColumn; // next column index to work
        std::mutex  fMutex;      // the mutex for the thread to acquire
    };

    std::vector<RowData>    fRowData;
};

#endif//SkTaskGroup2D_DEFINED
