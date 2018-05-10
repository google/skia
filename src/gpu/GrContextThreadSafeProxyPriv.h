/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrContextThreadSafeProxyPriv_DEFINED
#define GrContextThreadSafeProxyPriv_DEFINED

#include "GrContext.h"

/**
 * Class that adds methods to GrContextThreadSafeProxy that are only intended for use internal to
 * Skia. This class is purely a privileged window into GrContextThreadSafeProxy. It should never
 * have additional data members or virtual methods.
 */
class GrContextThreadSafeProxyPriv {
public:
    const GrContextOptions& contextOptions() { return fProxy->fOptions; }

    const GrCaps* caps() const { return fProxy->fCaps.get(); }
    sk_sp<const GrCaps> refCaps() const { return fProxy->fCaps; }
    uint32_t contextUniqueID() const { return fProxy->fContextUniqueID; }
    GrBackend backend() const { return fProxy->fBackend; }

private:
    explicit GrContextThreadSafeProxyPriv(GrContextThreadSafeProxy* proxy) : fProxy(proxy) {}
    GrContextThreadSafeProxyPriv(const GrContextThreadSafeProxy&) = delete;
    GrContextThreadSafeProxyPriv& operator=(const GrContextThreadSafeProxyPriv&) = delete;

    // No taking addresses of this type.
    const GrContextThreadSafeProxyPriv* operator&() const = delete;
    GrContextThreadSafeProxyPriv* operator&() = delete;

    GrContextThreadSafeProxy* fProxy;

    friend class GrContextThreadSafeProxy;  // to construct/copy this type.
};

inline GrContextThreadSafeProxyPriv GrContextThreadSafeProxy::priv() {
    return GrContextThreadSafeProxyPriv(this);
}

inline const GrContextThreadSafeProxyPriv GrContextThreadSafeProxy::priv() const {
    return GrContextThreadSafeProxyPriv(const_cast<GrContextThreadSafeProxy*>(this));
}

#endif
