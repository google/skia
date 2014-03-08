
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkPathHeap.h"
#include "SkPath.h"
#include "SkStream.h"
#include "SkReadBuffer.h"
#include "SkTSearch.h"
#include "SkWriteBuffer.h"
#include <new>

#define kPathCount  64

SkPathHeap::SkPathHeap() : fHeap(kPathCount * sizeof(SkPath)) {
}

SkPathHeap::SkPathHeap(SkReadBuffer& buffer)
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

SkPathHeap::LookupEntry::LookupEntry(const SkPath& path)
    : fGenerationID(path.getGenerationID()), fStorageSlot(0) {
}

SkPathHeap::LookupEntry* SkPathHeap::addIfNotPresent(const SkPath& path) {
    LookupEntry searchKey(path);
    int index = SkTSearch<const LookupEntry, LookupEntry::Less>(
                                    fLookupTable.begin(),
                                    fLookupTable.count(),
                                    searchKey,
                                    sizeof(LookupEntry));
    if (index < 0) {
        index = ~index;
        *fLookupTable.insert(index) = LookupEntry(path);
    }

    return &fLookupTable[index];;
}

int SkPathHeap::insert(const SkPath& path) {
    SkPathHeap::LookupEntry* entry = this->addIfNotPresent(path);

    if (entry->storageSlot() > 0) {
        return entry->storageSlot();
    }

    int newSlot = this->append(path);
    SkASSERT(newSlot > 0);
    entry->setStorageSlot(newSlot);
    return newSlot;
}

void SkPathHeap::flatten(SkWriteBuffer& buffer) const {
    int count = fPaths.count();

    buffer.writeInt(count);
    SkPath* const* iter = fPaths.begin();
    SkPath* const* stop = fPaths.end();
    while (iter < stop) {
        buffer.writePath(**iter);
        iter++;
    }
}
