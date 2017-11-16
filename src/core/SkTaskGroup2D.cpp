/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTaskGroup2D.h"

void SkTaskGroup2D::start() {
    fThreadsGroup->batch(fThreadCnt, [this](int threadId){
        while (this->next(threadId)) {}
    });
}

void SkTaskGroup2D::addColumn() {
    SkASSERT(!fIsFinishing);
    fWidth++;
}

void SkTaskGroup2D::finish() {
    fIsFinishing.store(true, std::memory_order_relaxed);
    fThreadsGroup->wait();
}

bool SkSpinningTaskGroup2D::next(int threadId) {
    int& nextColumn = fRowData[threadId].fNextColumn;

    while (true) {
        SkASSERT(nextColumn <= fWidth);
        bool isFinishing = fIsFinishing.load(std::memory_order_relaxed);
        if (isFinishing && nextColumn >= fWidth) {
            return false;
        }

        if (nextColumn < fWidth) {
            fWork(threadId, nextColumn);
            nextColumn++;
            return true;
        }
    }
}

SkFlexibleTaskGroup2D::SkFlexibleTaskGroup2D(Work2D&& w, int h, SkExecutor* x, int t)
        : SkTaskGroup2D(std::move(w), h, x, t), fRowData(h), fThreadData(t) {
    for (int i = 0; i < t; ++i) {
        fThreadData[i].fRowIndex = i;
    }
}


bool SkFlexibleTaskGroup2D::next(int threadId) {
    int failCnt = 0;
    int& rowIndex = fThreadData[threadId].fRowIndex;
    while (true) {
        RowData& rowData = fRowData[rowIndex];
        bool locked = rowData.fMutex.try_lock();
        bool processed = false;

        if (locked) {
            if (rowData.fNextColumn < fWidth) {
                fWork(rowIndex, rowData.fNextColumn);
                rowData.fNextColumn++;
                processed = true;
            } else {
                failCnt += fIsFinishing.load(std::memory_order_relaxed);
            }
            rowData.fMutex.unlock();
        }

        if (processed) {
            return true;
        } else {
            if (failCnt >= fHeight) {
                return false;
            }
            rowIndex = (rowIndex + 1) % fHeight;
        }
    }
}
