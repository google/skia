//
//  SkTRefArray.h
//  core
//
//  Created by Mike Reed on 7/17/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef SkTRefArray_DEFINED
#define SkTRefArray_DEFINED

#include "SkRefCnt.h"
#include <new>

/**
 *  Wrapper to manage thread-safe sharing of an array of T objects. The array
 *  cannot be grown or shrunk.
 */
template <typename T> class SkTRefArray : public SkRefCnt {
    /*
     *  Shared factory to allocate the space needed for our instance plus N
     *  T entries at the end. We call our constructor, but not the constructors
     *  for the elements. Those are called by the proper Create method.
     */
    static SkTRefArray<T>* Alloc(int count) {
        // space for us, and our [count] elements
        size_t size = sizeof(SkTRefArray<T>) + count * sizeof(T);
        SkTRefArray<T>* obj = (SkTRefArray<T>*)sk_malloc_throw(size);

        SkNEW_PLACEMENT(obj, SkTRefArray<T>);
        obj->fCount = count;
        return obj;
    }

public:
    /**
     *  Return a new array with 'count' elements, initialized to their default
     *  value. To change them to some other value, use writableBegin/End or
     *  writableAt(), but do that before this array is given to another thread.
     */
    static SkTRefArray<T>* Create(int count) {
        SkTRefArray<T>* obj = Alloc(count);
        T* array = const_cast<T*>(obj->begin());
        for (int i = 0; i < count; ++i) {
            SkNEW_PLACEMENT(&array[i], T);
        }
        return obj;
    }

    /**
     *  Return a new array with 'count' elements, initialized from the provided
     *  src array. To change them to some other value, use writableBegin/End or
     *  writableAt(), but do that before this array is given to another thread.
     */
    static SkTRefArray<T>* Create(const T src[], int count) {
        SkTRefArray<T>* obj = Alloc(count);
        T* array = const_cast<T*>(obj->begin());
        for (int i = 0; i < count; ++i) {
            SkNEW_PLACEMENT_ARGS(&array[i], T, (src[i]));
        }
        return obj;
    }

    int count() const { return fCount; }
    const T* begin() const { return (const T*)(this + 1); }
    const T* end() const { return this->begin() + fCount; }
    const T& at(int index) const {
        SkASSERT((unsigned)index < (unsigned)fCount);
        return this->begin()[index];
    }
    const T& operator[](int index) const { return this->at(index); }

    // For the writable methods, we assert that we are the only owner if we
    // call these, since other owners are not informed if we change an element.

    T* writableBegin() {
        SkASSERT(1 == this->getRefCnt());
        return (T*)(this + 1);
    }
    T* writableEnd() {
        return this->writableBegin() + fCount;
    }
    T& writableAt(int index) {
        SkASSERT((unsigned)index < (unsigned)fCount);
        return this->writableBegin()[index];
    }

protected:
    virtual void internal_dispose() const SK_OVERRIDE {
        T* array = const_cast<T*>(this->begin());
        int n = fCount;

        for (int i = 0; i < n; ++i) {
            array->~T();
            array += 1;
        }

        this->internal_dispose_restore_refcnt_to_1();
        this->~SkTRefArray<T>();
        sk_free((void*)this);
    }

private:
    int fCount;

    // hide this
    virtual ~SkTRefArray() {}

    typedef SkRefCnt INHERITED;
};

#endif
