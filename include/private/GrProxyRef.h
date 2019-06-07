/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrProxyRef_DEFINED
#define GrProxyRef_DEFINED

#include "include/private/GrSurfaceProxy.h"
#include "include/private/GrTextureProxy.h"
#include "include/private/GrTypesPriv.h"

/**
 * Helper for owning a ref on a GrSurfaceProxy.
 */
template <typename T> class GrProxyRef {
public:
    GrProxyRef() = default;
    GrProxyRef(const GrProxyRef&) = delete;
    GrProxyRef& operator=(const GrProxyRef&) = delete;

    GrProxyRef(sk_sp<T> proxy) { this->setProxy(std::move(proxy)); }

    ~GrProxyRef() { this->reset(); }

    void setProxy(sk_sp<T> proxy) {
        SkASSERT(SkToBool(fProxy) == fOwnRef);
        SkSafeUnref(fProxy);
        if (!proxy) {
            fProxy = nullptr;
            fOwnRef = false;
        } else {
            fProxy = proxy.release();  // due to the semantics of this class we unpack from sk_sp
            fOwnRef = true;
        }
    }

    T* get() const { return fProxy; }

    // Shortcut for calling setProxy() with NULL.
    void reset() {
        if (fOwnRef) {
            SkASSERT(fProxy);
            fProxy->unref();
            fOwnRef = false;
        }
        fProxy = nullptr;
    }

private:
    T*              fProxy = nullptr;
    mutable bool    fOwnRef = false;
};

using GrSurfaceProxyRef = GrProxyRef<GrSurfaceProxy>;
using GrTextureProxyRef = GrProxyRef<GrTextureProxy>;

#endif
