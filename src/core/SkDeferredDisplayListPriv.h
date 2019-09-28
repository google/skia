/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDeferredDisplayListPriv_DEFINED
#define SkDeferredDisplayListPriv_DEFINED

#include "include/private/SkDeferredDisplayList.h"

/** Class that adds methods to SkDeferredDisplayList that are only intended for use internal to Skia.
    This class is purely a privileged window into SkDeferredDisplayList. It should never have
    additional data members or virtual methods. */
class SkDeferredDisplayListPriv {
public:
    int numRenderTasks() const {
#if SK_SUPPORT_GPU
        return fDDL->fRenderTasks.count();
#else
        return 0;
#endif
    }

    const SkDeferredDisplayList::LazyProxyData* lazyProxyData() const {
#if SK_SUPPORT_GPU
        return fDDL->fLazyProxyData.get();
#else
        return nullptr;
#endif
    }

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
