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
    int workCol = 0;
    int initCol = 0;

    while (true) {
        SkASSERT(workCol <= fWidth);
        if (this->isFinishing() && workCol >= fWidth) {
            return;
        }

        // Note that row = threadId
        if (workCol < fWidth && fWork(threadId, workCol, threadId)) {
            workCol++;
        } else {
            // Initialize something if we can't work
            this->initAnUninitializedColumn(initCol, threadId);
        }
    }
}

void SkFlexibleTaskGroup2D::work(int threadId) {
    int failCnt = 0;
    int initCol = 0;
    int row     = threadId;

    // This loop looks for work to do as long as
    // either 1. isFinishing is false
    // or     2. isFinishing is true but some rows still have unfinished tasks
    while (true) {
        RowData& rowData = fRowData[row];
        bool processed = false;

        // The Android roller somehow gets a false-positive compile warning/error about the try-lock
        // and unlock process. Hence we disable -Wthread-safety-analysis to bypass it.
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wthread-safety-analysis"
#endif
        if (rowData.fMutex.try_lock()) {
            if (rowData.fNextColumn < fWidth) {
                if (fWork(row, rowData.fNextColumn, threadId)) {
                    rowData.fNextColumn++;
                    processed = true;
                }
                failCnt = 0;
            } else {
                // isFinishing can never go from true to false. Once it's true, we count how many
                // CONSECUTIVE rows are out of work. If that count reaches fHeight, then we're out
                // of work for the whole group.
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
            row = (row + 1) % fHeight;

            // Initialize something if we can't work
            this->initAnUninitializedColumn(initCol, threadId);
        }
    }
}
