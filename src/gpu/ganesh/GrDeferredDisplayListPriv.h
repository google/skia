/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDeferredDisplayListPriv_DEFINED
#define GrDeferredDisplayListPriv_DEFINED

#include "include/private/chromium/GrDeferredDisplayList.h"

/*************************************************************************************************/
/** Class that adds methods to GrDeferredDisplayList that are only intended for use internal to
    Skia. This class is purely a privileged window into GrDeferredDisplayList. It should never have
    additional data members or virtual methods. */
class GrDeferredDisplayListPriv {
public:
    int numRenderTasks() const {
        return fDDL->fRenderTasks.size();
    }

    GrRenderTargetProxy* targetProxy() const {
        return fDDL->fTargetProxy.get();
    }

    const GrDeferredDisplayList::LazyProxyData* lazyProxyData() const {
        return fDDL->fLazyProxyData.get();
    }

    const skia_private::TArray<GrRecordingContext::ProgramData>& programData() const {
        return fDDL->programData();
    }

    const skia_private::TArray<sk_sp<GrRenderTask>>& renderTasks() const {
        return fDDL->fRenderTasks;
    }

private:
    explicit GrDeferredDisplayListPriv(GrDeferredDisplayList* ddl) : fDDL(ddl) {}
    GrDeferredDisplayListPriv& operator=(const GrDeferredDisplayListPriv&) = delete;

    // No taking addresses of this type.
    const GrDeferredDisplayListPriv* operator&() const;
    GrDeferredDisplayListPriv* operator&();

    GrDeferredDisplayList* fDDL;

    friend class GrDeferredDisplayList; // to construct/copy this type.
};

inline GrDeferredDisplayListPriv GrDeferredDisplayList::priv() {
    return GrDeferredDisplayListPriv(this);
}

inline const GrDeferredDisplayListPriv GrDeferredDisplayList::priv () const {  // NOLINT(readability-const-return-type)
    return GrDeferredDisplayListPriv(const_cast<GrDeferredDisplayList*>(this));
}

#endif
