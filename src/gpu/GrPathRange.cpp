/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrPathRange.h"
#include "SkPath.h"

GrPathRange::GrPathRange(GrGpu* gpu,
                         PathGenerator* pathGenerator)
    : INHERITED(gpu, kCached_LifeCycle),
      fPathGenerator(SkRef(pathGenerator)),
      fNumPaths(fPathGenerator->getNumPaths()) {
    const int numGroups = (fNumPaths + kPathsPerGroup - 1) / kPathsPerGroup;
    fGeneratedPaths.reset((numGroups + 7) / 8); // 1 bit per path group.
    memset(&fGeneratedPaths.front(), 0, fGeneratedPaths.count());
}

GrPathRange::GrPathRange(GrGpu* gpu,
                         int numPaths)
    : INHERITED(gpu, kCached_LifeCycle),
      fNumPaths(numPaths) {
}

void GrPathRange::loadPathsIfNeeded(const void* indices, PathIndexType indexType, int count) const {
    switch (indexType) {
        case kU8_PathIndexType:
            return this->loadPathsIfNeeded(reinterpret_cast<const uint8_t*>(indices), count);
        case kU16_PathIndexType:
            return this->loadPathsIfNeeded(reinterpret_cast<const uint16_t*>(indices), count);
        case kU32_PathIndexType:
            return this->loadPathsIfNeeded(reinterpret_cast<const uint32_t*>(indices), count);
        default:
            SkFAIL("Unknown path index type");
    }
}

#ifdef SK_DEBUG

void GrPathRange::assertPathsLoaded(const void* indices, PathIndexType indexType, int count) const {
    switch (indexType) {
        case kU8_PathIndexType:
            return this->assertPathsLoaded(reinterpret_cast<const uint8_t*>(indices), count);
        case kU16_PathIndexType:
            return this->assertPathsLoaded(reinterpret_cast<const uint16_t*>(indices), count);
        case kU32_PathIndexType:
            return this->assertPathsLoaded(reinterpret_cast<const uint32_t*>(indices), count);
        default:
            SkFAIL("Unknown path index type");
    }
}

#endif
