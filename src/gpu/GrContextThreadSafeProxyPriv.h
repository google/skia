/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrContextThreadSafeProxyPriv_DEFINED
#define GrContextThreadSafeProxyPriv_DEFINED

#include "include/gpu/GrContextThreadSafeProxy.h"

/**
 * Class that adds methods to GrContextThreadSafeProxy that are only intended for use internal to
 * Skia. This class is purely a privileged window into GrContextThreadSafeProxy. It should never
 * have additional data members or virtual methods.
 */
class GrContextThreadSafeProxyPriv {
public:
    // from GrContext_Base
    uint32_t contextID() const { return fProxy->contextID(); }

    bool matches(GrContext_Base* candidate) const { return fProxy->matches(candidate); }

    const GrContextOptions& options() const { return fProxy->options(); }

    const GrCaps* caps() const { return fProxy->caps(); }
    sk_sp<const GrCaps> refCaps() const { return fProxy->refCaps(); }

    sk_sp<GrSkSLFPFactoryCache> fpFactoryCache();

    // GrContextThreadSafeProxyPriv
    static sk_sp<GrContextThreadSafeProxy> Make(GrBackendApi,
                                                const GrContextOptions&,
                                                uint32_t contextID,
                                                sk_sp<const GrCaps>,
                                                sk_sp<GrSkSLFPFactoryCache>);

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
