/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDeferredDisplayListPriv_DEFINED
#define SkDeferredDisplayListPriv_DEFINED

#include "include/core/SkDeferredDisplayList.h"
class GrRenderTargetProxy;

/*************************************************************************************************/

// This object is the source from which the lazy proxy backing the DDL will pull its backing
// texture when the DDL is replayed. It has to be separately ref counted bc the lazy proxy
// can outlive the DDL.
class SkDeferredDisplayList::LazyProxyData : public SkRefCnt {
#if SK_SUPPORT_GPU
public:
    // Upon being replayed - this field will be filled in (by the DrawingManager) with the
    // proxy backing the destination SkSurface. Note that, since there is no good place to
    // clear it, it can become a dangling pointer.
    GrRenderTargetProxy* fReplayDest = nullptr;
#endif
};

/*************************************************************************************************/
/** Class that adds methods to SkDeferredDisplayList that are only intended for use internal to Skia.
    This class is purely a privileged window into SkDeferredDisplayList. It should never have
    additional data members or virtual methods. */
class SkDeferredDisplayListPriv {
public:

#if SK_SUPPORT_GPU
    int numRenderTasks() const {
        return fDDL->fRenderTasks.count();
    }

    const SkDeferredDisplayList::LazyProxyData* lazyProxyData() const {
        return fDDL->fLazyProxyData.get();
    }

    const SkTArray<GrRecordingContext::ProgramData>& programData() const {
        return fDDL->programData();
    }
#endif

private:
    explicit SkDeferredDisplayListPriv(SkDeferredDisplayList* ddl) : fDDL(ddl) {}
    SkDeferredDisplayListPriv(const SkDeferredDisplayListPriv&);            // unimpl
    SkDeferredDisplayListPriv& operator=(const SkDeferredDisplayListPriv&); // unimpl

    // No taking addresses of this type.
    const SkDeferredDisplayListPriv* operator&() const;
    SkDeferredDisplayListPriv* operator&();

    SkDeferredDisplayList* fDDL;

    friend class SkDeferredDisplayList; // to construct/copy this type.
};

inline SkDeferredDisplayListPriv SkDeferredDisplayList::priv() {
    return SkDeferredDisplayListPriv(this);
}

inline const SkDeferredDisplayListPriv SkDeferredDisplayList::priv () const {
    return SkDeferredDisplayListPriv(const_cast<SkDeferredDisplayList*>(this));
}

#endif
