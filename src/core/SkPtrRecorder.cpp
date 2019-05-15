/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/core/SkPtrRecorder.h"
#include "src/core/SkTSearch.h"

void SkPtrSet::reset() {
    Pair* p = fList.begin();
    Pair* stop = fList.end();
    while (p < stop) {
        this->decPtr(p->fPtr);
        p += 1;
    }
    fList.reset();
}

bool SkPtrSet::Less(const Pair& a, const Pair& b) {
    return (char*)a.fPtr < (char*)b.fPtr;
}

uint32_t SkPtrSet::find(void* ptr) const {
    if (nullptr == ptr) {
        return 0;
    }

    int count = fList.count();
    Pair pair;
    pair.fPtr = ptr;

    int index = SkTSearch<Pair, Less>(fList.begin(), count, pair, sizeof(pair));
    if (index < 0) {
        return 0;
    }
    return fList[index].fIndex;
}

uint32_t SkPtrSet::add(void* ptr) {
    if (nullptr == ptr) {
        return 0;
    }

    int count = fList.count();
    Pair pair;
    pair.fPtr = ptr;

    int index = SkTSearch<Pair, Less>(fList.begin(), count, pair, sizeof(pair));
    if (index < 0) {
        index = ~index; // turn it back into an index for insertion
        this->incPtr(ptr);
        pair.fIndex = count + 1;
        *fList.insert(index) = pair;
        return count + 1;
    } else {
        return fList[index].fIndex;
    }
}

void SkPtrSet::copyToArray(void* array[]) const {
    int count = fList.count();
    if (count > 0) {
        SkASSERT(array);
        const Pair* p = fList.begin();
        // p->fIndex is base-1, so we need to subtract to find its slot
        for (int i = 0; i < count; i++) {
            int index = p[i].fIndex - 1;
            SkASSERT((unsigned)index < (unsigned)count);
            array[index] = p[i].fPtr;
        }
    }
}
