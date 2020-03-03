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
class GrRenderTask;
class GrRenderTargetProxy;
struct GrCCPerOpsTaskPaths;
#endif

/*
 * This class contains pre-processed gpu operations that can be replayed into
 * an SkSurface via SkSurface::draw(SkDeferredDisplayList*).
 */
class SkDeferredDisplayList {
public:
    SK_API ~SkDeferredDisplayList();

    SK_API const SkSurfaceCharacterization& characterization() const {
        return fCharacterization;
    }

#if SK_SUPPORT_GPU
    /**
     * Iterate through the programs required by the DDL.
     */
    class SK_API ProgramIterator {
    public:
        ProgramIterator(GrContext*, SkDeferredDisplayList*);
        ~ProgramIterator();

        void compile();
        bool done() const;
        void next();

    private:
        GrContext*                                       fContext;
        const SkTArray<GrRecordingContext::ProgramData>& fProgramData;
        int                                              fIndex;
    };
#endif

    // Provides access to functions that aren't part of the public API.
    SkDeferredDisplayListPriv priv();
    const SkDeferredDisplayListPriv priv() const;

private:
    friend class GrDrawingManager; // for access to 'fRenderTasks', 'fLazyProxyData', 'fArenas'
    friend class SkDeferredDisplayListRecorder; // for access to 'fLazyProxyData'
    friend class SkDeferredDisplayListPriv;

    // This object is the source from which the lazy proxy backing the DDL will pull its backing
    // texture when the DDL is replayed. It has to be separately ref counted bc the lazy proxy
    // can outlive the DDL.
    class LazyProxyData : public SkRefCnt {
#if SK_SUPPORT_GPU
    public:
        // Upon being replayed - this field will be filled in (by the DrawingManager) with the
        // proxy backing the destination SkSurface. Note that, since there is no good place to
        // clear it, it can become a dangling pointer.
        GrRenderTargetProxy* fReplayDest = nullptr;
#endif
    };

    SK_API SkDeferredDisplayList(const SkSurfaceCharacterization& characterization,
                                 sk_sp<LazyProxyData>);

#if SK_SUPPORT_GPU
    const SkTArray<GrRecordingContext::ProgramData>& programData() const {
        return fProgramData;
    }
#endif

    const SkSurfaceCharacterization fCharacterization;

#if SK_SUPPORT_GPU
    // This needs to match the same type in GrCoverageCountingPathRenderer.h
    using PendingPathsMap = std::map<uint32_t, sk_sp<GrCCPerOpsTaskPaths>>;

    // When programs are stored in 'fProgramData' their memory is actually allocated in
    // 'fArenas.fRecordTimeAllocator'. In that case that arena must be freed before
    // 'fPendingPaths' which relies on uniquely holding the atlas proxies used by the
    // GrCCClipPaths.
    PendingPathsMap                 fPendingPaths;  // This is the path data from CCPR.
    // These are ordered such that the destructor cleans op tasks up first (which may refer back
    // to the arena and memory pool in their destructors).
    GrRecordingContext::OwnedArenas fArenas;
    SkTArray<sk_sp<GrRenderTask>>   fRenderTasks;

    SkTArray<GrRecordingContext::ProgramData> fProgramData;
    sk_sp<LazyProxyData>            fLazyProxyData;
#endif
};

#endif
