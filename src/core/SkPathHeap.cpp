#include "SkPathHeap.h"
#include "SkPath.h"
#include "SkStream.h"
#include "SkFlattenable.h"
#include <new>

#define kPathCount  64

SkPathHeap::SkPathHeap() : fHeap(kPathCount * sizeof(SkPath)) {
}

SkPathHeap::SkPathHeap(SkFlattenableReadBuffer& buffer)
            : fHeap(kPathCount * sizeof(SkPath)) {
    int count = buffer.readS32();

    fPaths.setCount(count);
    SkPath** ptr = fPaths.begin();
    SkPath* p = (SkPath*)fHeap.allocThrow(count * sizeof(SkPath));

    for (int i = 0; i < count; i++) {
        new (p) SkPath;
        p->unflatten(buffer);
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
    
    buffer.write32(count);
    SkPath** iter = fPaths.begin();
    SkPath** stop = fPaths.end();
    while (iter < stop) {
        (*iter)->flatten(buffer);
        iter++;
    }
}

