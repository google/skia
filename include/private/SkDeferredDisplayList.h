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

    // DDL TODO: remove this. It is just scaffolding to get something up & running
    bool draw(SkSurface*) const;
#endif

#if SK_SUPPORT_GPU
    // This object is the source from which the lazy proxy backing the DDL will pull its backing
    // texture when the DDL is replayed. It has to be separately ref counted bc the lazy proxy
    // can outlive the DDL.
    class LazyProxyData : public SkRefCnt {
    public:
        // Upon being replayed - this field will be filled in (by the DrawingManager) with the proxy
        // backing the destination SkSurface. Note that, since there is no good place to clear it
        // it can become a dangling pointer.
        GrRenderTargetProxy*     fReplayDest = nullptr;
    };
#else
    class LazyProxyData : public SkRefCnt {};
#endif

    SkDeferredDisplayList(const SkSurfaceCharacterization& characterization,
                          sk_sp<LazyProxyData>);

    const SkSurfaceCharacterization& characterization() const {
        return fCharacterization;
    }

private:
    friend class GrDrawingManager; // for access to 'fOpLists' and 'fLazyProxyData'
    friend class SkDeferredDisplayListRecorder; // for access to 'fLazyProxyData'

    const SkSurfaceCharacterization fCharacterization;

#ifdef SK_RASTER_RECORDER_IMPLEMENTATION
    sk_sp<SkImage>               fImage;
#else

#if SK_SUPPORT_GPU
    SkTArray<sk_sp<GrOpList>>    fOpLists;
#endif
    sk_sp<LazyProxyData>         fLazyProxyData;

#endif
};

#endif
