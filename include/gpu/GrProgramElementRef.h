/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrProgramElementRef_DEFINED
#define GrProgramElementRef_DEFINED

#include "SkRefCnt.h"
#include "GrTypes.h"

/**
 * Helper for owning a GrProgramElement subclass and being able to convert a ref to pending
 * execution. It is like an SkAutoTUnref for program elements whose execution can be deferred. Once
 * in the pending execution state it is illegal to change the object that is owned by the
 * GrProgramElementRef. Its destructor will either unref the GrProgramElement or signal that
 * the pending execution has completed, depending on whether convertToPendingExec() was called.
 */
template <typename T> class GrProgramElementRef : SkNoncopyable {
public:
    GrProgramElementRef() : fOwnPendingExec(false), fObj(NULL) {};

    // Adopts a ref from the caller.
    explicit GrProgramElementRef(T* obj) : fOwnPendingExec(false), fObj(obj)  {}

    // Adopts a ref from the caller. Do not call after convertToPendingExec.
    void reset(T* obj) {
        SkASSERT(!fOwnPendingExec);
        SkSafeUnref(fObj);
        fObj = obj;
    }

    void convertToPendingExec() {
        SkASSERT(!fOwnPendingExec);
        fObj->convertRefToPendingExecution();
        fOwnPendingExec = true;
    }

    // In the short term we need to support copying a GrProcessorStage and making the copy own
    // the same type of ref as the source. This function exists to support this. TODO: Once
    // GrDrawState and GrOptDrawState no longer share a base class they won't have to share
    // GrProcessorStage and we can have GrOptDrawState always own pending executions rather than
    // refs on GrProgramElements. At that point we should be able to delete this function.
    // This function makes assumptions that are valid in the GrProcessorStage use case and should
    // not be used elsewhere.
    void initAndRef(const GrProgramElementRef& that) {
        SkASSERT(!fObj);
        SkASSERT(that.fObj);
        if (that.fOwnPendingExec) {
            SkASSERT(that.fObj->fPendingExecutions > 0);
            that.fObj->fPendingExecutions++;
        } else {
            that.fObj->ref();
        }
        this->fOwnPendingExec = that.fOwnPendingExec;
        this->fObj = that.fObj;
    }

    T* get() const { return fObj; }
    operator T*() { return fObj; }

    /** If T is const, the type returned from operator-> will also be const. */
    typedef typename SkTConstType<typename SkAutoTUnref<T>::template BlockRef<T>,
                                  SkTIsConst<T>::value>::type BlockRefType;

    /**
     * GrProgramElementRef assumes ownership of the ref and manages converting the ref to a
     * pending execution. As a result, it is an error for the user to ref or unref through
     * GrProgramElementRef. Therefore operator-> returns BlockRef<T>*.
     */
    BlockRefType *operator->() const {
        return static_cast<BlockRefType*>(fObj);
    }

    ~GrProgramElementRef() {
        if (fObj) {
            if (fOwnPendingExec) {
                fObj->completedExecution();
            } else {
                fObj->unref();
            }
        }
    }

private:
    bool fOwnPendingExec;
    T*   fObj;

    typedef SkNoncopyable INHERITED;
};
#endif
