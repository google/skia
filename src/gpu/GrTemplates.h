/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTemplates_DEFINED
#define GrTemplates_DEFINED

#include "SkTypes.h"

/**
 *  Use to cast a ptr to a different type, and maintain strict-aliasing
 */
template <typename Dst, typename Src> Dst GrTCast(Src src) {
    union {
        Src src;
        Dst dst;
    } data;
    data.src = src;
    return data.dst;
}

/**
 * takes a T*, saves the value it points to,  in and restores the value in the
 * destructor
 * e.g.:
 * {
 *      GrAutoTRestore<int*> autoCountRestore;
 *      if (useExtra) {
 *          autoCountRestore.reset(&fCount);
 *          fCount += fExtraCount;
 *      }
 *      ...
 * }  // fCount is restored
 */
template <typename T> class GrAutoTRestore : SkNoncopyable {
public:
    GrAutoTRestore() : fPtr(NULL), fVal() {}

    GrAutoTRestore(T* ptr) {
        fPtr = ptr;
        if (ptr) {
            fVal = *ptr;
        }
    }

    ~GrAutoTRestore() {
        if (fPtr) {
            *fPtr = fVal;
        }
    }

    // restores previously saved value (if any) and saves value for passed T*
    void reset(T* ptr) {
        if (fPtr) {
            *fPtr = fVal;
        }
        fPtr = ptr;
        fVal = *ptr;
    }
private:
    T* fPtr;
    T  fVal;
};

#endif
