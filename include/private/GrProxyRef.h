/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrProxyRef_DEFINED
#define GrProxyRef_DEFINED

#include "GrSurfaceProxy.h"
#include "GrTextureProxy.h"
#include "GrTypesPriv.h"

template <typename T> class GrProxyRef {
public:
    GrProxyRef() = default;
    GrProxyRef(const GrProxyRef&) = delete;
    GrProxyRef& operator=(const GrProxyRef&) = delete;

    GrProxyRef(sk_sp<T> proxy) { this->setProxy(std::move(proxy)); }

    ~GrProxyRef() { this->reset(); }

    void setProxy(sk_sp<T> proxy) { fProxy = std::move(proxy); }
    sk_sp<T> proxy() const { return fProxy; }

    T* get() const { return fProxy.get(); }

    void reset() { fProxy.reset(); }

    void removeRef() {
        SkASSERT(fProxy);
        fProxy.reset();
    }

private:
    sk_sp<T> fProxy;
};

using GrSurfaceProxyRef = GrProxyRef<GrSurfaceProxy>;
using GrTextureProxyRef = GrProxyRef<GrTextureProxy>;

#endif
