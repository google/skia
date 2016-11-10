/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFixedAlloc.h"

SkFixedAlloc::SkFixedAlloc(void* ptr, size_t len) : fBytes((char*)ptr), fUsed(0), fLimit(len) {
    if (size_t tail = (uintptr_t)fBytes & 15) {
        size_t fixup = (16 - tail);
        if (fLimit < fixup) {
            fLimit = 0;
            return;
        }
        fBytes += fixup;
        fLimit -= fixup;
    }
}

void SkFixedAlloc::undo() {
    SkASSERT(fUsed >= 16);

    void (*dtor)(void*);
    size_t len;
    this->readFooter(&dtor, &len);

    dtor(fBytes + fUsed - 16 - len);
    fUsed -= 16 + len;
}

void SkFixedAlloc::reset() {
    while (fUsed) {
        this->undo();
    }
}

void SkFixedAlloc::writeFooter(void (*dtor)(void*), size_t len) {
    static_assert(sizeof(dtor) + sizeof(len) <= 16, "");
    memcpy(fBytes + fUsed               , &dtor, sizeof(dtor));
    memcpy(fBytes + fUsed + sizeof(dtor), &len,  sizeof(len));
    fUsed += 16;
}

void SkFixedAlloc::readFooter(void (**dtor)(void*), size_t* len) const {
    memcpy(dtor, fBytes + fUsed - 16                , sizeof(*dtor));
    memcpy(len,  fBytes + fUsed - 16 + sizeof(*dtor), sizeof(*len));
}
