
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrTemplates_DEFINED
#define GrTemplates_DEFINED

#include "GrNoncopyable.h"

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
 * saves value of T* in and restores in destructor
 * e.g.:
 * {
 *      GrAutoTPtrValueRestore<int*> autoCountRestore;
 *      if (useExtra) {
 *          autoCountRestore.save(&fCount);
 *          fCount += fExtraCount;
 *      }
 *      ...
 * }  // fCount is restored
 */
template <typename T> class GrAutoTPtrValueRestore : public GrNoncopyable {
public:
    GrAutoTPtrValueRestore() : fPtr(NULL), fVal() {}
    
    GrAutoTPtrValueRestore(T* ptr) {
        fPtr = ptr;
        if (NULL != ptr) {
            fVal = *ptr;
        }
    }
    
    ~GrAutoTPtrValueRestore() {
        if (NULL != fPtr) {
            *fPtr = fVal;
        }
    }
    
    // restores previously saved value (if any) and saves value for passed T*
    void save(T* ptr) {
        if (NULL != fPtr) {
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
