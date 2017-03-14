/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSurfaceProxyPriv_DEFINED
#define GrSurfaceProxyPriv_DEFINED

#include "GrSurfaceProxy.h"

/** Class that adds methods to GrSurfaceProxy that are only intended for use internal to Skia.
    This class is purely a privileged window into GrSurfaceProxy. It should never have additional
    data members or virtual methods. */
class GrSurfaceProxyPriv {
public:
    // Beware! This call is only guaranteed to tell you if the proxy in question has
    // any pending IO in its current state. It won't tell you about the IO state in the
    // future when the proxy is actually used/instantiated.
    bool hasPendingIO() const { return fProxy->hasPendingIO(); }

    // Don't abuse these two!!!!!!!
    bool isExact() const { return SkBackingFit::kExact == fProxy->fFit; }

    // These next two are very specialized and wacky - don't use them!

    // In the case where an unbudgeted, deferred SkSurface_Gpu has snapped a budgeted, deferred
    // SkImage_Gpu, this serves to propagate the budgeting forward in time. For now, and
    // presumably forever, this will not change any flushing decisions but may make Ganesh
    // appear to have gone over budget. In the case of non-deferred proxies this will immediately
    // propagate the budget decision to the resource, which in itself is dubious.
    void makeBudgeted();
    // In the case where a budgeted, deferred SkSurface_Gpu has snapped an unbudgeted, deferred
    // SkImage_Gpu, this serves to propagate the lack of budgeting forward in time.
    void makeUnbudgeted();

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
