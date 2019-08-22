/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDeferredDisplayList_DEFINED
#define SkDeferredDisplayList_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSurfaceCharacterization.h"
#include "include/core/SkTypes.h"

class SkDeferredDisplayListPriv;

#if SK_SUPPORT_GPU
#include "include/private/SkTArray.h"
#include <map>
class GrRenderTask;
class GrRenderTargetProxy;
struct GrCCPerOpListPaths;
#endif

/*
 * This class contains pre-processed gpu operations that can be replayed into
 * an SkSurface via draw(SkDeferredDisplayList*).
 *
 * TODO: we probably need to expose this class so users can query it for memory usage.
 */
class SkDeferredDisplayList {
public:

#if SK_SUPPORT_GPU
    // This object is the source from which the lazy proxy backing the DDL will pull its backing
    // texture when the DDL is replayed. It has to be separately ref counted bc the lazy proxy
    // can outlive the DDL.
    class SK_API LazyProxyData : public SkRefCnt {
    public:
        // Upon being replayed - this field will be filled in (by the DrawingManager) with the proxy
        // backing the destination SkSurface. Note that, since there is no good place to clear it
        // it can become a dangling pointer.
        GrRenderTargetProxy*     fReplayDest = nullptr;
    };
#else
    class SK_API LazyProxyData : public SkRefCnt {};
#endif

    SK_API SkDeferredDisplayList(const SkSurfaceCharacterization& characterization,
                                 sk_sp<LazyProxyData>);
    SK_API ~SkDeferredDisplayList();

    SK_API const SkSurfaceCharacterization& characterization() const {
        return fCharacterization;
    }

    // Provides access to functions that aren't part of the public API.
    SkDeferredDisplayListPriv priv();
    const SkDeferredDisplayListPriv priv() const;

private:
    friend class GrDrawingManager; // for access to 'fRenderTasks' and 'fLazyProxyData'
    friend class SkDeferredDisplayListRecorder; // for access to 'fLazyProxyData'
    friend class SkDeferredDisplayListPriv;

    const SkSurfaceCharacterization fCharacterization;

#if SK_SUPPORT_GPU
    // This needs to match the same type in GrCoverageCountingPathRenderer.h
    using PendingPathsMap = std::map<uint32_t, sk_sp<GrCCPerOpListPaths>>;

    SkTArray<sk_sp<GrRenderTask>>   fRenderTasks;
    PendingPathsMap                 fPendingPaths;  // This is the path data from CCPR.
#endif
    sk_sp<LazyProxyData>            fLazyProxyData;
};

#endif
