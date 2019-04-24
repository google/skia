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

/**
 * Helper for owning a ref on a GrSurfaceProxy.
 */
template <typename T> class GrProxyRef {
public:
    GrProxyRef() = default;
    GrProxyRef(const GrProxyRef&) = delete;
    GrProxyRef& operator=(const GrProxyRef&) = delete;

    /** ioType expresses what type of IO operations will be marked as pending on the proxy when
        markPendingIO is called. */
    GrProxyRef(sk_sp<T> proxy) { this->setProxy(std::move(proxy)); }

    ~GrProxyRef() { this->reset(); }

    void setProxy(sk_sp<T> proxy) {
        SkASSERT(!fPendingIO);
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

    /** Does this object own a pending read or write on the resource it is wrapping. */
    bool ownsPendingIO() const { return fPendingIO; }

    /** Shortcut for calling setProxy() with NULL. It cannot be called after markingPendingIO
        is called. */
    void reset() {
        if (fPendingIO) {
            SkASSERT(fProxy);
            fPendingIO = false;
        }
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
    void markPendingIO() const {
        // This should only be called when the owning GrProgramElement gets its first
        // pendingExecution ref.
        SkASSERT(!fPendingIO);
        SkASSERT(fProxy);
        fPendingIO = true;
    }

    /** Called when the program element/draw state is no longer owned by GrOpList-client code.
        This lets the cache know that the drawing code will no longer schedule additional reads or
        writes to the resource using the program element or draw state. It can only be called once.
      */
    void removeRef() const {
        SkASSERT(fOwnRef);
        SkASSERT(fPendingIO);
        SkASSERT(fProxy);
        fProxy->unref();
        fOwnRef = false;
    }

    /** Called to indicate that the previous pending IO is complete. Useful when the owning object
        still has refs, so it is not about to destroy this GrGpuResourceRef, but its previously
        pending executions have been complete. Can only be called if removeRef() was not previously
        called. */
    void pendingIOComplete() const {
        SkASSERT(fOwnRef);
        SkASSERT(fPendingIO);
        fPendingIO = false;
    }

private:
    T*              fProxy = nullptr;
    mutable bool    fOwnRef = false;
    mutable bool    fPendingIO = false;
};

using GrSurfaceProxyRef = GrProxyRef<GrSurfaceProxy>;
using GrTextureProxyRef = GrProxyRef<GrTextureProxy>;

#endif
