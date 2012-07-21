//
//  SkTRefArray.h
//  core
//
//  Created by Mike Reed on 7/17/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef SkTRefArray_DEFINED
#define SkTRefArray_DEFINED

#include "SkThread.h"
#include <new>

/**
 *  Wrapper to manage thread-safe sharing of an array of T objects. The array
 *  cannot be grown or shrunk.
 */
template <typename T> class SkTRefArray {
public:
    static SkTRefArray<T>* Create(int count) {
        size_t size = sizeof(SkTRefArray<T>) + count * sizeof(T);
        SkTRefArray<T>* obj = (SkTRefArray<T>*)sk_malloc_throw(size);
        
        obj->fCount = count;
        obj->fRefCnt = 1;

        T* array = const_cast<T*>(obj->begin());
        for (int i = 0; i < count; ++i) {
            new (&array[i]) T;
        }
        return obj;
    }

    int count() const { return fCount; }
    const T* begin() const { return (const T*)(this + 1); }
    const T* end() const { return (const T*)(this + 1) + fCount; }
    const T& operator[](int index) const {
        SkASSERT((unsigned)index < (unsigned)fCount);
        return this->begin()[index];
    }

    // We mimic SkRefCnt in API, but we don't inherit as we want to control
    // the allocation/deallocation so we can keep the array in the same
    // block of memory

    int32_t getRefCnt() const { return fRefCnt; }

    void ref() const {
        SkASSERT(fRefCnt > 0);
        sk_atomic_inc(&fRefCnt);
    }
    
    void unref() const {
        SkASSERT(fRefCnt > 0);
        if (sk_atomic_dec(&fRefCnt) == 1) {
            sk_membar_aquire__after_atomic_dec();
            this->deleteAll();
            sk_free((void*)this);
        }
    }

private:
    int             fCount;
    mutable int32_t fRefCnt;

    void deleteAll() const {
        T* array = const_cast<T*>(this->begin());
        int n = fCount;

        for (int i = 0; i < n; ++i) {
            array->~T();
            array += 1;
        }
    }
};

#endif
