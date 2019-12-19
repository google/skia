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
#include "include/private/GrRecordingContext.h"
#include "include/private/SkTArray.h"
#include <map>
class GrOpMemoryPool;
class GrRenderTask;
class GrRenderTargetProxy;
struct GrCCPerOpsTaskPaths;
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
#if SK_SUPPORT_GPU
    // TODO: we should probably also store the GrProgramDescs - since we already have
    // them
    SK_API const SkTDArray<const GrProgramInfo*>& programInfos() const {
        return fProgramInfos;
    }
#endif

    friend class GrDrawingManager; // for access to 'fRenderTasks', 'fLazyProxyData', 'fArenas'
    friend class SkDeferredDisplayListRecorder; // for access to 'fLazyProxyData'
    friend class SkDeferredDisplayListPriv;

    const SkSurfaceCharacterization fCharacterization;

#if SK_SUPPORT_GPU
    // This needs to match the same type in GrCoverageCountingPathRenderer.h
    using PendingPathsMap = std::map<uint32_t, sk_sp<GrCCPerOpsTaskPaths>>;

    // These are ordered such that the destructor cleans op tasks up first (which may refer back
    // to the arena and memory pool in their destructors).
    GrRecordingContext::OwnedArenas fArenas;
    PendingPathsMap                 fPendingPaths;  // This is the path data from CCPR.
    SkTArray<sk_sp<GrRenderTask>>   fRenderTasks;

    // The program infos should be stored in 'fRecordTimeData' so do not need to be ref counted
    // or deleted in the destructor.
    SkTDArray<const GrProgramInfo*> fProgramInfos;
#endif
    sk_sp<LazyProxyData>            fLazyProxyData;
};

#endif
