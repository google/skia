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

// A 2D grid (height rows x width columns) of tasks to be executed on a given executor with
// threadCnt number of threads.
//
// The height (number of rows) is fixed. The width (number of columns) may be dynamically expanded.
//
// The task on row i and column j is abstracted as WorkFn(i, j, t). Parameter t is the thread id and
// it shouldn't affect the work to be done. It's only used to allow some thread local variables
// (e.g., thread local allocators). We guarantee that the task on the same row will be executed in
// order (i.e., WorkFn(1, 1, t) is guaranteed to finish before calling WorkFn(1, 2, t)). Tasks in
// different rows can happen in any order.
//
// There are also width number of init calls (fInit), one per column. WorkFn(i, j, t) may return
// false if column j requires initialization but it's not initialized yet. In that case, a thread t
// need to call InitFn(j, t) once to unblock all rows that depend on the initialization of column j.
// (Again, t shouldn't affect the init work to be done; it's just for some thread local variables).
// The init calls have no order requirement so we can call them in any order.
//
// Multiple therads may try to init the same column j at the same time. InitFn is expected to handle
// this gracefully (e.g., let only one thread do the init and return immediately for other threads).
class SkTaskGroup2D {
public:
    // Return false iff the column needs an unfinished initialization.
    using WorkFn = std::function<bool(int, int, int)>;

    // Return false iff no initialization is done for this column in this call (either because it's
    // already being initialized by another thread, or there's no initialization work to do.)
    using InitFn = std::function<bool(int, int)>;

    SkTaskGroup2D(WorkFn&& work, InitFn&& init, int height, SkExecutor* executor, int threadCnt)
            : fWork(work), fInit(init), fHeight(height), fThreadCnt(threadCnt), fIsFinishing(false)
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

    // initCol must be thread local
    void initSomething(int& initCol, int threadId) {
        bool didSomeInit = false;
        while (initCol < fWidth && !didSomeInit) {
            didSomeInit = fInit(initCol++, threadId);
        }
    }

    WorkFn      fWork;
    InitFn      fInit;
    const int   fHeight;
    const int   fThreadCnt;

    std::atomic<bool>   fIsFinishing;
    std::atomic<int>    fWidth;

    std::unique_ptr<SkTaskGroup> fThreadsGroup;
};

// A simple spinning task group that assumes height equals threadCnt.
class SkSpinningTaskGroup2D final : public SkTaskGroup2D {
public:
    SkSpinningTaskGroup2D(WorkFn&& w, InitFn&& i, int h, SkExecutor* x, int t)
            : SkTaskGroup2D(std::move(w), std::move(i), h, x, t) {
        SkASSERT(h == t); // height must be equal to threadCnt
    }

protected:
    void work(int threadId) override;
};

class SkFlexibleTaskGroup2D final : public SkTaskGroup2D {
public:
    SkFlexibleTaskGroup2D(WorkFn&& w, InitFn&& i, int h, SkExecutor* x, int t)
            : SkTaskGroup2D(std::move(w), std::move(i), h, x, t), fRowData(h) {}

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
