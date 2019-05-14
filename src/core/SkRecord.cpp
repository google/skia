/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkImage.h"
#include "src/core/SkRecord.h"

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
    return fApproxBytesAllocated + sizeof(SkRecord);
}
