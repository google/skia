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

    T* get() const { return fObj; }
    operator T*() { return fObj; }

    /** If T is const, the type returned from operator-> will also be const. */
    typedef typename SkTConstType<typename SkAutoTUnref<T>::BlockRef<T>,
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
        if (NULL != fObj) {
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
