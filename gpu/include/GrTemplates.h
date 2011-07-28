
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
 * Reserves memory that is aligned on double and pointer boundaries.
 * Hopefully this is sufficient for all practical purposes.
 */
template <size_t N> class GrAlignedSStorage : GrNoncopyable {
public:
    void* get() { return fData; }
private:
    union {
        void*   fPtr;
        double  fDouble;
        char    fData[N];
    };
};

/**
 * Reserves memory that is aligned on double and pointer boundaries.
 * Hopefully this is sufficient for all practical purposes. Otherwise,
 * we have to do some arcane trickery to determine alignment of non-POD
 * types. Lifetime of the memory is the lifetime of the object.
 */
template <int N, typename T> class GrAlignedSTStorage : GrNoncopyable {
public:
    /**
     * Returns void* because this object does not initialize the
     * memory. Use placement new for types that require a cons.
     */
    void* get() { return fStorage.get(); }
private:
    GrAlignedSStorage<sizeof(T)*N> fStorage;
};

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
