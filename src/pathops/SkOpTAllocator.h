/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkOpTAllocator_DEFINED
#define SkOpTAllocator_DEFINED

#include "SkArenaAlloc.h"

// T is SkOpAngle2, SkOpSpan2, or SkOpSegment2
template<typename T>
class SkOpTAllocator {
public:
    static T* Allocate(SkArenaAlloc* allocator) {
        return allocator->make<T>();
    }

    static T* AllocateArray(SkArenaAlloc* allocator, int count) {
        return allocator->makeArrayDefault<T>(count);
    }

    static T* New(SkArenaAlloc* allocator) {
        return allocator->make<T>();
    }
};

#endif
