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

    ~GrProxyRef() { this->reset1(); }

    void setProxy(sk_sp<T> proxy) {
//        SkASSERT(!fPendingIO);
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

    /** Shortcut for calling setProxy() with NULL. It cannot be called after markingPendingIO
        is called. */
    void reset1() {
#if 0
        if (fPendingIO) {
            SkASSERT(fProxy);
            fPendingIO = false;
        }
#endif
        if (fOwnRef) {
            SkASSERT(fProxy);
            fProxy->unref();
            fOwnRef = false;
        }
        fProxy = nullptr;
    }

    /**
     * Called when transferring into an op list and therefore scheduled for an IO operation. It can
     * only be called once.
     */
    void markPendingIO1() const {
#if 0
        // This should only be called when the owning GrProgramElement gets its first
        // pendingExecution ref.
        SkASSERT(!fPendingIO);
        SkASSERT(fProxy);
        fPendingIO = true;
#endif
    }

    /** Called when the program element/draw state is no longer owned by GrOpList-client code.
        This lets the cache know that the drawing code will no longer schedule additional reads or
        writes to the resource using the program element or draw state. It can only be called once.
      */
    void removeRef1() const {
#if 0
        SkASSERT(fOwnRef);
        SkASSERT(fPendingIO);
        SkASSERT(fProxy);
        fProxy->unref();
        fOwnRef = false;
#endif
    }

private:
    T*              fProxy = nullptr;
    mutable bool    fOwnRef = false;
//    mutable bool    fPendingIO = false;
};

using GrSurfaceProxyRef = GrProxyRef<GrSurfaceProxy>;
using GrTextureProxyRef = GrProxyRef<GrTextureProxy>;

#endif
