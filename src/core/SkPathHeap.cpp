
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkPathHeap.h"
#include "SkPath.h"
#include "SkStream.h"
#include "SkFlattenableBuffers.h"
#include <new>

SK_DEFINE_INST_COUNT(SkPathHeap)

#define kPathCount  64

SkPathHeap::SkPathHeap() : fHeap(kPathCount * sizeof(SkPath)) {
}

SkPathHeap::SkPathHeap(SkFlattenableReadBuffer& buffer)
            : fHeap(kPathCount * sizeof(SkPath)) {
    const int count = buffer.readInt();

    fPaths.setCount(count);
    SkPath** ptr = fPaths.begin();
    SkPath* p = (SkPath*)fHeap.allocThrow(count * sizeof(SkPath));

    for (int i = 0; i < count; i++) {
        new (p) SkPath;
        buffer.readPath(p);
        *ptr++ = p; // record the pointer
        p++;        // move to the next storage location
    }
}

SkPathHeap::~SkPathHeap() {
    SkPath** iter = fPaths.begin();
    SkPath** stop = fPaths.end();
    while (iter < stop) {
        (*iter)->~SkPath();
        iter++;
    }
}

int SkPathHeap::append(const SkPath& path) {
    SkPath* p = (SkPath*)fHeap.allocThrow(sizeof(SkPath));
    new (p) SkPath(path);
    *fPaths.append() = p;
    return fPaths.count();
}

void SkPathHeap::flatten(SkFlattenableWriteBuffer& buffer) const {
    int count = fPaths.count();

    buffer.writeInt(count);
    SkPath* const* iter = fPaths.begin();
    SkPath* const* stop = fPaths.end();
    while (iter < stop) {
        buffer.writePath(**iter);
        iter++;
    }
}
