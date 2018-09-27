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
#include "GrCCPerOpListPaths.h"
#include "GrOpList.h"

#include <map>
#endif

class SkDeferredDisplayListPriv;
class SkSurface;
/*
 * This class contains pre-processed gpu operations that can be replayed into
 * an SkSurface via draw(SkDeferredDisplayList*).
 *
 * TODO: we probably need to expose this class so users can query it for memory usage.
 */
class SK_API SkDeferredDisplayList {
public:

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
    ~SkDeferredDisplayList();

    const SkSurfaceCharacterization& characterization() const {
        return fCharacterization;
    }

    // Provides access to functions that aren't part of the public API.
    SkDeferredDisplayListPriv priv();
    const SkDeferredDisplayListPriv priv() const;

private:
    friend class GrDrawingManager; // for access to 'fOpLists' and 'fLazyProxyData'
    friend class SkDeferredDisplayListRecorder; // for access to 'fLazyProxyData'
    friend class SkDeferredDisplayListPriv;

    const SkSurfaceCharacterization fCharacterization;

#if SK_SUPPORT_GPU
    // This needs to match the same type in GrCoverageCountingPathRenderer.h
    using PendingPathsMap = std::map<uint32_t, sk_sp<GrCCPerOpListPaths>>;

    SkTArray<sk_sp<GrOpList>>    fOpLists;
    PendingPathsMap              fPendingPaths;  // This is the path data from CCPR.
#endif
    sk_sp<LazyProxyData>         fLazyProxyData;
};

#endif
