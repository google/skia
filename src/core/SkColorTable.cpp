/*
 * Copyright 2009 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorTable.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

SkColorTable::SkColorTable(const SkPMColor colors[], int count) {
    SkASSERT(0 == count || colors);
    SkASSERT(count >= 0 && count <= 256);

    fCount = count;
    fColors = reinterpret_cast<SkPMColor*>(sk_malloc_throw(count * sizeof(SkPMColor)));

    memcpy(fColors, colors, count * sizeof(SkPMColor));
}

SkColorTable::~SkColorTable() {
    sk_free(fColors);
}

void SkColorTable::Skip(SkReadBuffer& buffer) {
    const int count = buffer.getArrayCount();
    if (count < 0 || count > 256) {
        buffer.validate(false);
    } else {
        buffer.skip(count * sizeof(SkPMColor));
    }
}
