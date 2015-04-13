/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRecord.h"

SkRecord::~SkRecord() {
    Destroyer destroyer;
    for (unsigned i = 0; i < this->count(); i++) {
        this->mutate<void>(i, destroyer);
    }
}

void SkRecord::grow() {
    SkASSERT(fCount == fReserved);
    SkASSERT(fReserved > 0);
    fReserved *= 2;
    fRecords.realloc(fReserved);
}

size_t SkRecord::bytesUsed() const {
    size_t bytes = fAlloc.approxBytesAllocated() + sizeof(SkRecord);
    // If fReserved <= kInlineRecords, we've already accounted for fRecords with sizeof(SkRecord).
    // When we go over that limit, they're allocated on the heap (and the inline space is wasted).
    if (fReserved > kInlineRecords) {
        bytes += fReserved * sizeof(Record);
    }
    return bytes;
}
