/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDeferredDisplayList_DEFINED
#define SkDeferredDisplayList_DEFINED

#include "SkSurfaceCharacterization.h"

class SkImage; // TODO: rm this since it is just for the temporary placeholder implementation
class SkSurface;

/*
 * This class contains pre-processed gpu operations that can be replayed into
 * an SkSurface via draw(SkDeferredDisplayList*).
 *
 * TODO: we probably need to expose this class so users can query it for memory usage.
 */
class SkDeferredDisplayList {
public:
    SkDeferredDisplayList(const SkSurfaceCharacterization& characterization,
                          sk_sp<SkImage> image)  // TODO rm this parameter
            : fCharacterization(characterization)
            , fImage(std::move(image)) {
    }

    const SkSurfaceCharacterization& characterization() const {
        return fCharacterization;
    }

    // TODO: remove this. It is just scaffolding to get something up & running
    bool draw(SkSurface*);

private:
    const SkSurfaceCharacterization fCharacterization;

    // TODO: actually store the GPU opLists
    sk_sp<SkImage> fImage;
};

#endif
