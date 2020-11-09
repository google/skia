/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDeferredDisplayListPriv_DEFINED
#define SkDeferredDisplayListPriv_DEFINED

#include "include/core/SkDeferredDisplayList.h"

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

    GrRenderTargetProxy* targetProxy() const {
        return fDDL->fTargetProxy.get();
    }

    const SkDeferredDisplayList::LazyProxyData* lazyProxyData() const {
        return fDDL->fLazyProxyData.get();
    }

    const SkTArray<GrRecordingContext::ProgramData>& programData() const {
        return fDDL->programData();
    }

    const SkTArray<sk_sp<GrRenderTask>>& renderTasks() const {
        return fDDL->fRenderTasks;
    }
#endif

private:
    explicit SkDeferredDisplayListPriv(SkDeferredDisplayList* ddl) : fDDL(ddl) {}
    SkDeferredDisplayListPriv(const SkDeferredDisplayListPriv&) = delete;
    SkDeferredDisplayListPriv& operator=(const SkDeferredDisplayListPriv&) = delete;

    // No taking addresses of this type.
    const SkDeferredDisplayListPriv* operator&() const;
    SkDeferredDisplayListPriv* operator&();

    SkDeferredDisplayList* fDDL;

    friend class SkDeferredDisplayList; // to construct/copy this type.
};

inline SkDeferredDisplayListPriv SkDeferredDisplayList::priv() {
    return SkDeferredDisplayListPriv(this);
}

inline const SkDeferredDisplayListPriv SkDeferredDisplayList::priv () const {  // NOLINT(readability-const-return-type)
    return SkDeferredDisplayListPriv(const_cast<SkDeferredDisplayList*>(this));
}

#endif
