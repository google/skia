/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTaskGroup2D.h"

void SkTaskGroup2D::start() {
    fThreadsGroup->batch(fThreadCnt, [this](int threadId){
        this->work(threadId);
    });
}

void SkTaskGroup2D::addColumn() {
    SkASSERT(!fIsFinishing); // we're not supposed to add more work after the calling of finish
    fWidth++;
}

void SkTaskGroup2D::finish() {
    fIsFinishing.store(true, std::memory_order_relaxed);
    fThreadsGroup->wait();
}

void SkSpinningTaskGroup2D::work(int threadId) {
    int& nextColumn = fRowData[threadId].fNextColumn;

    while (true) {
        SkASSERT(nextColumn <= fWidth);
        if (this->isFinishing() && nextColumn >= fWidth) {
            return;
        }

        if (nextColumn < fWidth) {
            fWork(threadId, nextColumn);
            nextColumn++;
        }
    }
}

SkFlexibleTaskGroup2D::SkFlexibleTaskGroup2D(Work2D&& w, int h, SkExecutor* x, int t)
        : SkTaskGroup2D(std::move(w), h, x, t), fRowData(h), fThreadData(t) {
    for (int i = 0; i < t; ++i) {
        fThreadData[i].fRowIndex = i;
    }
}


void SkFlexibleTaskGroup2D::work(int threadId) {
    int failCnt = 0;
    int& rowIndex = fThreadData[threadId].fRowIndex;

    // This loop looks for work to do as long as
    // either 1. isFinishing is false
    // or     2. isFinishing is true but some rows still have unfinished tasks
    while (true) {
        RowData& rowData = fRowData[rowIndex];
        bool processed = false;

        // The Android roller somehow gets a false-positive compile warning/error about the try-lock
        // and unlock process. Hence we disable -Wthread-safety-analysis to bypass it.
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wthread-safety-analysis"
#endif
        if (rowData.fMutex.try_lock()) {
            if (rowData.fNextColumn < fWidth) {
                fWork(rowIndex, rowData.fNextColumn);
                rowData.fNextColumn++;
                processed = true;
            } else {
                // isFinishing can never go from true to false. Once it's true, we count how many
                // times that a row is out of work. If that count reaches fHeight, then we're out of
                // work for the whole group.
                failCnt += this->isFinishing();
            }
            rowData.fMutex.unlock();
        }
#ifdef __clang__
#pragma clang diagnostic pop
#endif

        if (!processed) {
            if (failCnt >= fHeight) {
                return;
            }
            rowIndex = (rowIndex + 1) % fHeight;
        }
    }
}
