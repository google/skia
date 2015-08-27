/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPendingProgramElement_DEFINED
#define GrPendingProgramElement_DEFINED

#include "SkRefCnt.h"
#include "GrTypes.h"

/**
 * Helper for owning a pending execution on a GrProgramElement. Using this rather than ref allows
 * resources that are owned by the program element to be correctly tracked as having pending reads
 * and writes rather than refs.
 */
template <typename T> class GrPendingProgramElement : SkNoncopyable {
public:
    GrPendingProgramElement() : fObj(nullptr) { };

    // Adds a pending execution on obj.
    explicit GrPendingProgramElement(T* obj) : fObj(obj)  {
        if (obj) {
            obj->addPendingExecution();
        }
    }

    void reset(T* obj) {
        if (obj) {
            obj->addPendingExecution();
        }
        if (fObj) {
            fObj->completedExecution();
        }
        fObj = obj;
    }

    T* get() const { return fObj; }
    operator T*() { return fObj; }

    T *operator->() const { return fObj; }

    ~GrPendingProgramElement() {
        if (fObj) {
            fObj->completedExecution();
        }
    }

private:
    T*   fObj;

    typedef SkNoncopyable INHERITED;
};
#endif
