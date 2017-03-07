/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkOpTAllocator_DEFINED
#define SkOpTAllocator_DEFINED

#include "SkChunkAlloc.h"

// T is SkOpAngle2, SkOpSpan2, or SkOpSegment2
template<typename T>
class SkOpTAllocator {
public:
    static T* Allocate(SkChunkAlloc* allocator) {
        void* ptr = allocator->allocThrow(sizeof(T));
        T* record = (T*) ptr;
        return record;
    }

    static T* AllocateArray(SkChunkAlloc* allocator, int count) {
        void* ptr = allocator->allocThrow(sizeof(T) * count);
        T* record = (T*) ptr;
        return record;
    }

    static T* New(SkChunkAlloc* allocator) {
        return new (Allocate(allocator)) T();
    }
};

#endif
