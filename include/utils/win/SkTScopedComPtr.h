
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkSkTScopedPtr_DEFINED
#define SkSkTScopedPtr_DEFINED

#include "SkTemplates.h"

template<typename T>
class SkTScopedComPtr : SkNoncopyable {
private:
    T *fPtr;

public:
    explicit SkTScopedComPtr(T *ptr = NULL) : fPtr(ptr) { }
    ~SkTScopedComPtr() {
        if (NULL != fPtr) {
            fPtr->Release();
            fPtr = NULL;
        }
    }
    T &operator*() const { return *fPtr; }
    T *operator->() const { return fPtr; }
    /**
     * Returns the address of the underlying pointer.
     * This is dangerous -- it breaks encapsulation and the reference escapes.
     * Must only be used on instances currently pointing to NULL,
     * and only to initialize the instance.
     */
    T **operator&() { SkASSERT(fPtr == NULL); return &fPtr; }
    T *get() const { return fPtr; }
};

#endif
