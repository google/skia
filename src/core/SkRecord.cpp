/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkImage.h"
#include "src/core/SkRecord.h"
#include <algorithm>

SkRecord::~SkRecord() {
    Destroyer destroyer;
    for (int i = 0; i < this->count(); i++) {
        this->mutate(i, destroyer);
    }
}

void SkRecord::grow() {
    SkASSERT(fCount == fReserved);
    fReserved = fReserved ? fReserved * 2 : 4;
    fRecords.realloc(fReserved);
}

size_t SkRecord::bytesUsed() const {
    size_t bytes = fApproxBytesAllocated + sizeof(SkRecord);
    return bytes;
}

void SkRecord::defrag() {
    // Remove all the NoOps, preserving the order of other ops, e.g.
    //      Save, ClipRect, NoOp, DrawRect, NoOp, NoOp, Restore
    //  ->  Save, ClipRect, DrawRect, Restore
    Record* noops = std::remove_if(fRecords.get(), fRecords.get() + fCount,
                                   [](Record op) { return op.type() == SkRecords::NoOp_Type; });
    fCount = noops - fRecords.get();
}
