/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSurfaceProxyPriv_DEFINED
#define GrSurfaceProxyPriv_DEFINED

#include "GrSurfaceProxy.h"

#include "GrResourceProvider.h"

/** Class that adds methods to GrSurfaceProxy that are only intended for use internal to Skia.
    This class is purely a privileged window into GrSurfaceProxy. It should never have additional
    data members or virtual methods. */
class GrSurfaceProxyPriv {
public:
    // Beware! Woe betide anyone whosoever calls this method.
    // The refs on proxies and their backing GrSurfaces shift around based on whether the proxy
    // is instantiated or not. Additionally, the lifetime of a proxy (and a GrSurface) also
    // depends on the read and write refs (So this method can validly return 0).
    int32_t getProxyRefCnt() const { return fProxy->getProxyRefCnt(); }

    void computeScratchKey(GrScratchKey* key) const { return fProxy->computeScratchKey(key); }

    // Create a GrSurface-derived class that meets the requirements (i.e, desc, renderability)
    // of the GrSurfaceProxy.
    sk_sp<GrSurface> createSurface(GrResourceProvider* resourceProvider) const {
        return fProxy->createSurface(resourceProvider);
    }

    // Assign this proxy the provided GrSurface as its backing surface
    void assign(sk_sp<GrSurface> surface) { fProxy->assign(std::move(surface)); }

    bool requiresNoPendingIO() const {
        return fProxy->fSurfaceFlags & GrInternalSurfaceFlags::kNoPendingIO;
    }

    // Don't abuse this call!!!!!!!
    bool isExact() const { return SkBackingFit::kExact == fProxy->fFit; }

    // Don't. Just don't.
    void exactify();

    bool doLazyInstantiation(GrResourceProvider*);

    GrSurfaceProxy::LazyInstantiationType lazyInstantiationType() const {
        return fProxy->fLazyInstantiationType;
    }

    bool isSafeToDeinstantiate() const {
        return SkToBool(fProxy->fTarget) && SkToBool(fProxy->fLazyInstantiateCallback) &&
               GrSurfaceProxy::LazyInstantiationType::kDeinstantiate == lazyInstantiationType();
    }

    static bool SK_WARN_UNUSED_RESULT AttachStencilIfNeeded(GrResourceProvider*, GrSurface*,
                                                            bool needsStencil);

private:
    explicit GrSurfaceProxyPriv(GrSurfaceProxy* proxy) : fProxy(proxy) {}
    GrSurfaceProxyPriv(const GrSurfaceProxyPriv&) {} // unimpl
    GrSurfaceProxyPriv& operator=(const GrSurfaceProxyPriv&); // unimpl

    // No taking addresses of this type.
    const GrSurfaceProxyPriv* operator&() const;
    GrSurfaceProxyPriv* operator&();

    GrSurfaceProxy* fProxy;

    friend class GrSurfaceProxy; // to construct/copy this type.
};

inline GrSurfaceProxyPriv GrSurfaceProxy::priv() { return GrSurfaceProxyPriv(this); }

inline const GrSurfaceProxyPriv GrSurfaceProxy::priv () const {
    return GrSurfaceProxyPriv(const_cast<GrSurfaceProxy*>(this));
}

#endif
