/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGpuResourceRef_DEFINED
#define GrGpuResourceRef_DEFINED

#include "GrGpuResource.h"
#include "SkRefCnt.h"

/**
 * This class is intended only for internal use in core Gr code.
 *
 * Class that wraps a resource referenced by a GrProgramElement or GrDrawState. It manages
 * converting refs to pending IO operations. It allows a resource ownership to be in three
 * states:
 *          1. Owns a single ref
 *          2. Owns a single ref and a pending IO operation (read, write, or read-write)
 *          3. Owns a single pending IO operation.
 *
 * It is legal to destroy the GrGpuResourceRef in any of these states. It starts in state
 * 1. Calling markPendingIO() converts it from state 1 to state 2. Calling removeRef() goes from
 * state 2 to state 3. Calling pendingIOComplete() moves from state 2 to state 1. There is no
 * valid way of going from state 3 back to 2 or 1.
 *
 * Like sk_sp, its constructor and setter adopt a ref from their caller.
 *
 * TODO: Once GrDODrawState no longer exists and therefore GrDrawState and GrOptDrawState no
 * longer share an instance of this class, attempt to make the resource owned by GrGpuResourceRef
 * only settable via the constructor.
 */
class GrGpuResourceRef : SkNoncopyable {
public:
    ~GrGpuResourceRef();

    GrGpuResource* getResource() const { return fResource; }

    /** Does this object own a pending read or write on the resource it is wrapping. */
    bool ownsPendingIO() const { return fPendingIO; }

    /** What type of IO does this represent? This is independent of whether a normal ref or a
        pending IO is currently held. */
    GrIOType ioType() const { return fIOType; }

    /** Shortcut for calling setResource() with NULL. It cannot be called after markingPendingIO
        is called. */
    void reset();

protected:
    GrGpuResourceRef();

    /** Adopts a ref from the caller. ioType expresses what type of IO operations will be marked as
        pending on the resource when markPendingIO is called. */
    GrGpuResourceRef(GrGpuResource*, GrIOType);

    /** Adopts a ref from the caller. ioType expresses what type of IO operations will be marked as
        pending on the resource when markPendingIO is called. */
    void setResource(GrGpuResource*, GrIOType);

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

    GrGpuResource*  fResource;
    mutable bool    fOwnRef;
    mutable bool    fPendingIO;
    GrIOType        fIOType;

    typedef SkNoncopyable INHERITED;
};

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


/**
 * Templated version of GrGpuResourceRef to enforce type safety.
 */
template <typename T> class GrTGpuResourceRef : public GrGpuResourceRef {
public:
    GrTGpuResourceRef() {}

    /** Adopts a ref from the caller. ioType expresses what type of IO operations will be marked as
        pending on the resource when markPendingIO is called. */
    GrTGpuResourceRef(T* resource, GrIOType ioType) : INHERITED(resource, ioType) { }

    T* get() const { return static_cast<T*>(this->getResource()); }

    /** Adopts a ref from the caller. ioType expresses what type of IO operations will be marked as
        pending on the resource when markPendingIO is called. */
    void set(T* resource, GrIOType ioType) { this->setResource(resource, ioType); }

private:
    typedef GrGpuResourceRef INHERITED;
};

/**
 * This is similar to GrTGpuResourceRef but can only be in the pending IO state. It never owns a
 * ref.
 */
template <typename T, GrIOType IO_TYPE> class GrPendingIOResource : SkNoncopyable {
public:
    GrPendingIOResource(T* resource = NULL) : fResource(NULL) {
        this->reset(resource);
    }

    GrPendingIOResource(const GrPendingIOResource& that)
        : GrPendingIOResource(that.get()) {
    }

    void reset(T* resource = nullptr) {
        if (resource) {
            switch (IO_TYPE) {
                case kRead_GrIOType:
                    resource->addPendingRead();
                    break;
                case kWrite_GrIOType:
                    resource->addPendingWrite();
                    break;
                case kRW_GrIOType:
                    resource->addPendingRead();
                    resource->addPendingWrite();
                    break;
            }
        }
        this->release();
        fResource = resource;
    }

    ~GrPendingIOResource() {
        this->release();
    }

    explicit operator bool() const { return SkToBool(fResource); }

    bool operator==(const GrPendingIOResource& other) const {
        return fResource == other.fResource;
    }

    T* get() const { return fResource; }

private:
    void release() {
        if (fResource) {
            switch (IO_TYPE) {
                case kRead_GrIOType:
                    fResource->completedRead();
                    break;
                case kWrite_GrIOType:
                    fResource->completedWrite();
                    break;
                case kRW_GrIOType:
                    fResource->completedRead();
                    fResource->completedWrite();
                    break;
            }
        }
    }

    T* fResource;
};
#endif
