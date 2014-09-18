/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrPathRange.h"
#include "SkPath.h"

enum {
    kPathsPerGroup = 16 // Paths get tracked in groups of 16 for lazy loading.
};

GrPathRange::GrPathRange(GrGpu* gpu,
                         PathGenerator* pathGenerator,
                         const SkStrokeRec& stroke)
    : INHERITED(gpu, kIsWrapped),
      fPathGenerator(SkRef(pathGenerator)),
      fNumPaths(fPathGenerator->getNumPaths()),
      fStroke(stroke) {
    const int numGroups = (fNumPaths + kPathsPerGroup - 1) / kPathsPerGroup;
    fGeneratedPaths.reset((numGroups + 7) / 8); // 1 bit per path group.
    memset(&fGeneratedPaths.front(), 0, fGeneratedPaths.count());
}

GrPathRange::GrPathRange(GrGpu* gpu,
                         int numPaths,
                         const SkStrokeRec& stroke)
    : INHERITED(gpu, kIsWrapped),
      fNumPaths(numPaths),
      fStroke(stroke) {
}

void GrPathRange::willDrawPaths(const uint32_t indices[], int count) const {
    if (NULL == fPathGenerator.get()) {
        return;
    }

    bool didLoadPaths = false;

    for (int i = 0; i < count; ++i) {
        SkASSERT(indices[i] < static_cast<uint32_t>(fNumPaths));

        const int groupIndex = indices[i] / kPathsPerGroup;
        const int groupByte = groupIndex / 8;
        const uint8_t groupBit = 1 << (groupIndex % 8);

        const bool hasPath = SkToBool(fGeneratedPaths[groupByte] & groupBit);
        if (!hasPath) {
            // We track which paths are loaded in groups of kPathsPerGroup. To
            // mark a path as loaded we need to load the entire group.
            const int groupFirstPath = groupIndex * kPathsPerGroup;
            const int groupLastPath = SkTMin(groupFirstPath + kPathsPerGroup, fNumPaths) - 1;

            SkPath path;
            for (int pathIdx = groupFirstPath; pathIdx <= groupLastPath; ++pathIdx) {
                fPathGenerator->generatePath(pathIdx, &path);
                this->onInitPath(pathIdx, path);
            }

            fGeneratedPaths[groupByte] |= groupBit;
            didLoadPaths = true;
        }
    }

    if (didLoadPaths) {
        this->didChangeGpuMemorySize();
    }
}
