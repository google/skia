/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTArray_DEFINED
#define SkTArray_DEFINED

#include <new>
#include "SkTypes.h"
#include "SkTemplates.h"

template <typename T, bool MEM_COPY = false> class SkTArray;

namespace SkTArrayExt {

template<typename T>
inline void copy(SkTArray<T, true>* self, const T* array) {
    memcpy(self->fMemArray, array, self->fCount * sizeof(T));
}
template<typename T>
inline void copyAndDelete(SkTArray<T, true>* self, char* newMemArray) {
    memcpy(newMemArray, self->fMemArray, self->fCount * sizeof(T));
}

template<typename T>
inline void copy(SkTArray<T, false>* self, const T* array) {
    for (int i = 0; i < self->fCount; ++i) {
        new (self->fItemArray + i) T(array[i]);
    }
}
template<typename T>
inline void copyAndDelete(SkTArray<T, false>* self, char* newMemArray) {
    for (int i = 0; i < self->fCount; ++i) {
        new (newMemArray + sizeof(T) * i) T(self->fItemArray[i]);
        self->fItemArray[i].~T();
    }
}

}

/** When MEM_COPY is true T will be bit copied when moved.
    When MEM_COPY is false, T will be copy constructed / destructed.
    In all cases T's constructor will be called on allocation,
    and its destructor will be called from this object's destructor.
*/
template <typename T, bool MEM_COPY> class SkTArray {
public:
    /**
     * Creates an empty array with no initial storage
     */
    SkTArray() {
        fCount = 0;
        fReserveCount = gMIN_ALLOC_COUNT;
        fAllocCount = 0;
        fMemArray = NULL;
        fPreAllocMemArray = NULL;
    }

    /**
     * Creates an empty array that will preallocate space for reserveCount 
     * elements.
     */
    explicit SkTArray(int reserveCount) {
        this->init(NULL, 0, NULL, reserveCount);
    }
  
    /**
     * Copies one array to another. The new array will be heap allocated.
     */
    explicit SkTArray(const SkTArray& array) {
        this->init(array.fItemArray, array.fCount, NULL, 0);
    }

    /**
     * Creates a SkTArray by copying contents of a standard C array. The new 
     * array will be heap allocated. Be careful not to use this constructor
     * when you really want the (void*, int) version.
     */
    SkTArray(const T* array, int count) {
        this->init(array, count, NULL, 0);
    }

    /**
     * assign copy of array to this
     */
    SkTArray& operator =(const SkTArray& array) {
        for (int i = 0; i < fCount; ++i) {
            fItemArray[i].~T();
        }
        fCount = 0;
        checkRealloc((int)array.count());
        fCount = array.count();
        SkTArrayExt::copy(this, static_cast<const T*>(array.fMemArray));
        return *this;
    }

    virtual ~SkTArray() {
        for (int i = 0; i < fCount; ++i) {
            fItemArray[i].~T();
        }
        if (fMemArray != fPreAllocMemArray) {
            sk_free(fMemArray);
        }
    }

    /**
     * Resets to count() == 0
     */
    void reset() { this->pop_back_n(fCount); }

    /**
     * Number of elements in the array.
     */
    int count() const { return fCount; }

    /**
     * Is the array empty.
     */
    bool empty() const { return !fCount; }

    /**
     * Adds 1 new default-constructed T value and returns in by reference. Note
     * the reference only remains valid until the next call that adds or removes
     * elements.
     */
    T& push_back() {
        checkRealloc(1);
        new ((char*)fMemArray+sizeof(T)*fCount) T;
        ++fCount;
        return fItemArray[fCount-1];
    }

    /**
     * Version of above that uses a copy constructor to initialize the new item
     */
    T& push_back(const T& t) {
        checkRealloc(1);
        new ((char*)fMemArray+sizeof(T)*fCount) T(t);
        ++fCount;
        return fItemArray[fCount-1];
    }

    /**
     * Allocates n more default T values, and returns the address of the start
     * of that new range. Note: this address is only valid until the next API
     * call made on the array that might add or remove elements.
     */
    T* push_back_n(int n) {
        SkASSERT(n >= 0);
        checkRealloc(n);
        for (int i = 0; i < n; ++i) {
            new (fItemArray + fCount + i) T;
        }
        fCount += n;
        return fItemArray + fCount - n;
    }

    /**
     * Version of above that uses a copy constructor to initialize all n items
     * to the same T.
     */
    T* push_back_n(int n, const T& t) {
        SkASSERT(n >= 0);
        checkRealloc(n);
        for (int i = 0; i < n; ++i) {
            new (fItemArray + fCount + i) T(t);
        }
        fCount += n;
        return fItemArray + fCount - n;
    }

    /**
     * Version of above that uses a copy constructor to initialize the n items
     * to separate T values.
     */
    T* push_back_n(int n, const T t[]) {
        SkASSERT(n >= 0);
        checkRealloc(n);
        for (int i = 0; i < n; ++i) {
            new (fItemArray + fCount + i) T(t[i]);
        }
        fCount += n;
        return fItemArray + fCount - n;
    }

    /**
     * Removes the last element. Not safe to call when count() == 0.
     */
    void pop_back() {
        SkASSERT(fCount > 0);
        --fCount;
        fItemArray[fCount].~T();
        checkRealloc(0);
    }

    /**
     * Removes the last n elements. Not safe to call when count() < n.
     */
    void pop_back_n(int n) {
        SkASSERT(n >= 0);
        SkASSERT(fCount >= n);
        fCount -= n;
        for (int i = 0; i < n; ++i) {
            fItemArray[i].~T();
        }
        checkRealloc(0);
    }

    /**
     * Pushes or pops from the back to resize. Pushes will be default
     * initialized.
     */
    void resize_back(int newCount) {
        SkASSERT(newCount >= 0);

        if (newCount > fCount) {
            push_back_n(newCount - fCount);
        } else if (newCount < fCount) {
            pop_back_n(fCount - newCount);
        }
    }

    /**
     * Get the i^th element.
     */
    T& operator[] (int i) {
        SkASSERT(i < fCount);
        SkASSERT(i >= 0);
        return fItemArray[i];
    }

    const T& operator[] (int i) const {
        SkASSERT(i < fCount);
        SkASSERT(i >= 0);
        return fItemArray[i];
    }

    /**
     * equivalent to operator[](0)
     */
    T& front() { SkASSERT(fCount > 0); return fItemArray[0];}

    const T& front() const { SkASSERT(fCount > 0); return fItemArray[0];}

    /**
     * equivalent to operator[](count() - 1)
     */
    T& back() { SkASSERT(fCount); return fItemArray[fCount - 1];}

    const T& back() const { SkASSERT(fCount > 0); return fItemArray[fCount - 1];}

    /**
     * equivalent to operator[](count()-1-i)
     */
    T& fromBack(int i) {
        SkASSERT(i >= 0);
        SkASSERT(i < fCount);
        return fItemArray[fCount - i - 1];
    }

    const T& fromBack(int i) const {
        SkASSERT(i >= 0);
        SkASSERT(i < fCount);
        return fItemArray[fCount - i - 1];
    }

protected:
    /**
     * Creates an empty array that will use the passed storage block until it
     * is insufficiently large to hold the entire array.
     */
    template <int N>
    SkTArray(SkAlignedSTStorage<N,T>* storage) {
        this->init(NULL, 0, storage->get(), N);
    }

    /**
     * Copy another array, using preallocated storage if preAllocCount >=
     * array.count(). Otherwise storage will only be used when array shrinks
     * to fit.
     */
    template <int N>
    SkTArray(const SkTArray& array, SkAlignedSTStorage<N,T>* storage) {
        this->init(array.fItemArray, array.fCount, storage->get(), N);
    }

    /**
     * Copy a C array, using preallocated storage if preAllocCount >=
     * count. Otherwise storage will only be used when array shrinks
     * to fit.
     */
    template <int N>
    SkTArray(const T* array, int count, SkAlignedSTStorage<N,T>* storage) {
        this->init(array, count, storage->get(), N);
    }

    void init(const T* array, int count,
               void* preAllocStorage, int preAllocOrReserveCount) {
        GrAssert(count >= 0);
        GrAssert(preAllocOrReserveCount >= 0);
        fCount              = count;
        fReserveCount       = (preAllocOrReserveCount > 0) ?
                                    preAllocOrReserveCount :
                                    gMIN_ALLOC_COUNT;
        fPreAllocMemArray   = preAllocStorage;
        if (fReserveCount >= fCount &&
            NULL != preAllocStorage) {
            fAllocCount = fReserveCount;
            fMemArray = preAllocStorage;
        } else {
            fAllocCount = GrMax(fCount, fReserveCount);
            fMemArray = GrMalloc(fAllocCount * sizeof(T));
        }

        SkTArrayExt::copy(this, array);
    }

private:

    static const int gMIN_ALLOC_COUNT = 8;

    inline void checkRealloc(int delta) {
        SkASSERT(fCount >= 0);
        SkASSERT(fAllocCount >= 0);

        SkASSERT(-delta <= fCount);

        int newCount = fCount + delta;
        int newAllocCount = fAllocCount;

        if (newCount > fAllocCount) {
            newAllocCount = SkMax32(newCount + ((newCount + 1) >> 1),
                                   fReserveCount);
        } else if (newCount < fAllocCount / 3) {
            newAllocCount = SkMax32(fAllocCount / 2, fReserveCount);
        }

        if (newAllocCount != fAllocCount) {

            fAllocCount = newAllocCount;
            char* newMemArray;

            if (fAllocCount == fReserveCount && NULL != fPreAllocMemArray) {
                newMemArray = (char*) fPreAllocMemArray;
            } else {
                newMemArray = (char*) sk_malloc_throw(fAllocCount*sizeof(T));
            }

            SkTArrayExt::copyAndDelete<T>(this, newMemArray);

            if (fMemArray != fPreAllocMemArray) {
                sk_free(fMemArray);
            }
            fMemArray = newMemArray;
        }
    }

    template<typename X> friend void SkTArrayExt::copy(SkTArray<X, true>* that, const X*);
    template<typename X> friend void SkTArrayExt::copyAndDelete(SkTArray<X, true>* that, char*);

    template<typename X> friend void SkTArrayExt::copy(SkTArray<X, false>* that, const X*);
    template<typename X> friend void SkTArrayExt::copyAndDelete(SkTArray<X, false>* that, char*);

    int fReserveCount;
    int fCount;
    int fAllocCount;
    void*    fPreAllocMemArray;
    union {
        T*       fItemArray;
        void*    fMemArray;
    };
};

/**
 * Subclass of SkTArray that contains a preallocated memory block for the array.
 */
template <int N, typename T, bool DATA_TYPE = false>
class SkSTArray : public SkTArray<T, DATA_TYPE> {
private:
    typedef SkTArray<T, DATA_TYPE> INHERITED;

public:
    SkSTArray() : INHERITED(&fStorage) {
    }

    SkSTArray(const SkSTArray& array)
        : INHERITED(array, &fStorage) {
    }

    explicit SkSTArray(const INHERITED& array)
        : INHERITED(array, &fStorage) {
    }

    SkSTArray(const T* array, int count)
        : INHERITED(array, count, &fStorage) {
    }

    SkSTArray& operator= (const SkSTArray& array) {
        return *this = *(const INHERITED*)&array;
    }

    SkSTArray& operator= (const INHERITED& array) {
        INHERITED::operator=(array);
        return *this;
    }

private:
    SkAlignedSTStorage<N,T> fStorage;
};

#endif

