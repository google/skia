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
    return fAlloc.approxBytesAllocated() +
           (fReserved - kInlineRecords) * sizeof(Record) +
           sizeof(SkRecord);
}
