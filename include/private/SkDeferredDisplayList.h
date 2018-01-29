/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDeferredDisplayList_DEFINED
#define SkDeferredDisplayList_DEFINED

#include "SkSurfaceCharacterization.h"

#if SK_SUPPORT_GPU
#include "GrOpList.h"
#endif

#ifdef SK_RASTER_RECORDER_IMPLEMENTATION
class SkImage; // DDL TODO: rm this since it is just for the temporary placeholder implementation
#endif

class SkSurface;

/*
 * This class contains pre-processed gpu operations that can be replayed into
 * an SkSurface via draw(SkDeferredDisplayList*).
 *
 * TODO: we probably need to expose this class so users can query it for memory usage.
 */
class SkDeferredDisplayList {
public:

#ifdef SK_RASTER_RECORDER_IMPLEMENTATION
    SkDeferredDisplayList(const SkSurfaceCharacterization& characterization, sk_sp<SkImage> image)
            : fCharacterization(characterization)
            , fImage(std::move(image)) {
    }
#else
    SkDeferredDisplayList(const SkSurfaceCharacterization& characterization,
                          SkTArray<sk_sp<GrOpList>>&& opLists)
            : fCharacterization(characterization)
            , fOpLists(std::move(opLists)) {
    }
#endif

    const SkSurfaceCharacterization& characterization() const {
        return fCharacterization;
    }

    // DDL TODO: remove this. It is just scaffolding to get something up & running
    bool draw(SkSurface*);

private:
    friend class GrDrawingManager; // for access to 'fOpLists'

    const SkSurfaceCharacterization fCharacterization;

#ifdef SK_RASTER_RECORDER_IMPLEMENTATION
    // DDL TODO: actually store the GPU opLists
    sk_sp<SkImage>            fImage;
#else
    const SkTArray<sk_sp<GrOpList>> fOpLists;
#endif
};

#endif
