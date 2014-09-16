/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrProgramResource_DEFINED
#define GrProgramResource_DEFINED

#include "SkRefCnt.h"

class GrGpuResource;

/**
 * Class that wraps a resource referenced by a GrProgramElement or GrDrawState. It manages
 * converting refs to pending io operations. Like SkAutoTUnref, its constructor and setter adopt
 * a ref from their caller. This class is intended only for internal use in core Gr code.
 */
class GrProgramResource : SkNoncopyable {
public:
    SK_DECLARE_INST_COUNT_ROOT(GrProgramResource);

    enum IOType {
        kRead_IOType,
        kWrite_IOType,
        kRW_IOType,

        kNone_IOType, // For internal use only, don't specify to constructor or setResource().
    };

    ~GrProgramResource();

    GrGpuResource* getResource() const { return fResource; }

    /** Does this object own a pending read or write on the resource it is wrapping. */
    bool ownsPendingIO() const { return fPendingIO; }

    /** Shortcut for calling setResource() with NULL. It cannot be called after markingPendingIO
        is called. */
    void reset();

protected:
    GrProgramResource();

    /** Adopts a ref from the caller. ioType expresses what type of IO operations will be marked as
        pending on the resource when markPendingIO is called. */
    GrProgramResource(GrGpuResource*, IOType);

    /** Adopts a ref from the caller. ioType expresses what type of IO operations will be marked as
        pending on the resource when markPendingIO is called. */
    void setResource(GrGpuResource*, IOType);

private:
    /** Called by owning GrProgramElement when the program element is first scheduled for
        execution. */
    void markPendingIO() const;

    /** Called when the program element/draw state is no longer owned by GrDrawTarget-client code.
        This lets the cache know that the drawing code will no longer schedule additional reads or
        writes to the resource using the program element or draw state. */
    void removeRef() const;

    /** Called to indicate that the previous pending IO is complete. Useful when the owning object
        still has refs, so it is not about to destroy this GrProgramResource, but its previously
        pending executions have been complete.
     */
    void pendingIOComplete() const;

    friend class GrRODrawState;
    friend class GrProgramElement;

    GrGpuResource*      fResource;
    mutable bool        fOwnRef;
    mutable bool        fPendingIO;
    IOType              fIOType;

    typedef SkNoncopyable INHERITED;
};

template <typename T> class GrProgramTResource : public GrProgramResource {
public:
    GrProgramTResource() {}

    /** Adopts a ref from the caller. ioType expresses what type of IO operations will be marked as
        pending on the resource when markPendingIO is called. */
    GrProgramTResource(T* resource, IOType ioType) : GrProgramResource(resource, ioType) {}

    T* get() const { return static_cast<T*>(this->getResource()); }

    /** Adopts a ref from the caller. ioType expresses what type of IO operations will be marked as
        pending on the resource when markPendingIO is called. */
    void set(T* resource, IOType ioType) { this->setResource(resource, ioType); }
};


#endif
