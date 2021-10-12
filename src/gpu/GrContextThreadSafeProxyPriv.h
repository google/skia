/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrContextThreadSafeProxyPriv_DEFINED
#define GrContextThreadSafeProxyPriv_DEFINED

#include "include/gpu/GrContextThreadSafeProxy.h"
#include "include/private/GrContext_Base.h"

#include "src/gpu/GrCaps.h"
#include "src/gpu/text/GrTextBlobCache.h"

/**
 * Class that adds methods to GrContextThreadSafeProxy that are only intended for use internal to
 * Skia. This class is purely a privileged window into GrContextThreadSafeProxy. It should never
 * have additional data members or virtual methods.
 */
class GrContextThreadSafeProxyPriv {
public:
    void init(sk_sp<const GrCaps>, sk_sp<GrThreadSafePipelineBuilder>) const;

    bool matches(GrContext_Base* candidate) const {
        return fProxy == candidate->threadSafeProxy().get();
    }

    GrBackend backend() const { return fProxy->fBackend; }
    const GrContextOptions& options() const { return fProxy->fOptions; }
    uint32_t contextID() const { return fProxy->fContextID; }

    const GrCaps* caps() const { return fProxy->fCaps.get(); }
    sk_sp<const GrCaps> refCaps() const { return fProxy->fCaps; }

    GrTextBlobCache* getTextBlobCache() { return fProxy->fTextBlobCache.get(); }
    const GrTextBlobCache* getTextBlobCache() const { return fProxy->fTextBlobCache.get(); }

    GrThreadSafeCache* threadSafeCache() { return fProxy->fThreadSafeCache.get(); }
    const GrThreadSafeCache* threadSafeCache() const { return fProxy->fThreadSafeCache.get(); }

    void abandonContext() { fProxy->abandonContext(); }
    bool abandoned() const { return fProxy->abandoned(); }

    // GrContextThreadSafeProxyPriv
    static sk_sp<GrContextThreadSafeProxy> Make(GrBackendApi, const GrContextOptions&);

private:
    explicit GrContextThreadSafeProxyPriv(GrContextThreadSafeProxy* proxy) : fProxy(proxy) {}
    // Required until C++17 copy elision
    GrContextThreadSafeProxyPriv(const GrContextThreadSafeProxyPriv&) = default;
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

inline const GrContextThreadSafeProxyPriv GrContextThreadSafeProxy::priv() const {  // NOLINT(readability-const-return-type)
    return GrContextThreadSafeProxyPriv(const_cast<GrContextThreadSafeProxy*>(this));
}

#endif
