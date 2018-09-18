/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSurfaceProxyRef_DEFINED
#define GrSurfaceProxyRef_DEFINED

#include "GrTypesPriv.h"

class GrSurfaceProxy;

class GrSurfaceProxyRef : SkNoncopyable {
public:
    virtual ~GrSurfaceProxyRef();

    GrSurfaceProxy* get() const { return fProxy; }

    /** Does this object own a pending read or write on the resource it is wrapping. */
    bool ownsPendingIO() const { return fPendingIO; }

    /** What type of IO does this represent? This is independent of whether a normal ref or a
        pending IO is currently held. */
    GrIOType ioType() const { return fIOType; }

    /** Shortcut for calling setProxy() with NULL. It cannot be called after markingPendingIO
        is called. */
    void reset();

protected:
    GrSurfaceProxyRef();

    /** ioType expresses what type of IO operations will be marked as
        pending on the resource when markPendingIO is called. */
    GrSurfaceProxyRef(sk_sp<GrSurfaceProxy>, GrIOType);

    /** ioType expresses what type of IO operations will be marked as
        pending on the resource when markPendingIO is called. */
    void setProxy(sk_sp<GrSurfaceProxy>, GrIOType);

private:
    /** Called by owning GrProgramElement when the program element is first scheduled for
        execution. It can only be called once. */
    void markPendingIO() const;

    /** Called when the program element/draw state is no longer owned by GrOpList-client code.
        This lets the cache know that the drawing code will no longer schedule additional reads or
        writes to the resource using the program element or draw state. It can only be called once.
      */
    void removeRef() const;

    /** Called to indicate that the previous pending IO is complete. Useful when the owning object
        still has refs, so it is not about to destroy this GrGpuResourceRef, but its previously
        pending executions have been complete. Can only be called if removeRef() was not previously
        called. */
    void pendingIOComplete() const;

    friend class GrResourceIOProcessor;
    friend class GrOpList;                 // for setProxy

    GrSurfaceProxy* fProxy;
    mutable bool    fOwnRef;
    mutable bool    fPendingIO;
    GrIOType        fIOType;

    typedef SkNoncopyable INHERITED;
};

#endif
