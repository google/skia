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
    fIsFinishing = true;
    fThreadsGroup->wait();
}

void SkSpinningTaskGroup2D::work(int threadId) {
    int workCol = 0;
    int initCol = 0;

    while (true) {
        SkASSERT(workCol <= fWidth);
        if (this->isFinishing() && workCol >= fWidth) {
            return;
        }

        // Note that row = threadId
        if (workCol < fWidth && fKernel->work2D(threadId, workCol, threadId)) {
            workCol++;
        } else {
            // Initialize something if we can't work
            this->initAnUninitializedColumn(initCol, threadId);
        }
    }
}

void SkFlexibleTaskGroup2D::work(int threadId) {
    int row = threadId;
    int initCol = 0;
    int numRowsCompleted = 0;
    std::vector<bool> completedRows(fHeight, false);

    // Only keep fHeight - numRowsCompleted number of threads looping. When rows are about to
    // complete, this strategy keeps the contention low.
    while (threadId < fHeight - numRowsCompleted) {
        RowData& rowData = fRowData[row];

        // The Android roller somehow gets a false-positive compile warning/error about the try-lock
        // and unlock process. Hence we disable -Wthread-safety-analysis to bypass it.
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wthread-safety-analysis"
#endif
        if (rowData.fMutex.try_lock()) {
            while (rowData.fNextColumn < fWidth &&
                    fKernel->work2D(row, rowData.fNextColumn, threadId)) {
                rowData.fNextColumn++;
            }
            // isFinishing can never go from true to false. Once it's true, we count how many rows
            // are completed (out of work). If that count reaches fHeight, then we're out of work
            // for the whole group and we can stop.
            if (rowData.fNextColumn == fWidth && this->isFinishing()) {
                numRowsCompleted += (completedRows[row] == false);
                completedRows[row] = true; // so we won't count this row twice
            }
            rowData.fMutex.unlock();
        }
#ifdef __clang__
#pragma clang diagnostic pop
#endif

        // By reaching here, we're either unable to acquire the row, or out of work, or blocked by
        // initialization
        row = (row + 1) % fHeight; // Move to the next row
        this->initAnUninitializedColumn(initCol, threadId); // Initialize something
    }
}
