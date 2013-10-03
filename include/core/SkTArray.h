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
        SkNEW_PLACEMENT_ARGS(self->fItemArray + i, T, (array[i]));
    }
}
template<typename T>
inline void copyAndDelete(SkTArray<T, false>* self, char* newMemArray) {
    for (int i = 0; i < self->fCount; ++i) {
        SkNEW_PLACEMENT_ARGS(newMemArray + sizeof(T) * i, T, (self->fItemArray[i]));
        self->fItemArray[i].~T();
    }
}

}

template <typename T, bool MEM_COPY> void* operator new(size_t, SkTArray<T, MEM_COPY>*, int);

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
        this->checkRealloc((int)array.count());
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
     * Resets to count() = n newly constructed T objects.
     */
    void reset(int n) {
        SkASSERT(n >= 0);
        for (int i = 0; i < fCount; ++i) {
            fItemArray[i].~T();
        }
        // set fCount to 0 before calling checkRealloc so that no copy cons. are called.
        fCount = 0;
        this->checkRealloc(n);
        fCount = n;
        for (int i = 0; i < fCount; ++i) {
            SkNEW_PLACEMENT(fItemArray + i, T);
        }
    }

    /**
     * Resets to a copy of a C array.
     */
    void reset(const T* array, int count) {
        for (int i = 0; i < fCount; ++i) {
            fItemArray[i].~T();
        }
        int delta = count - fCount;
        this->checkRealloc(delta);
        fCount = count;
        for (int i = 0; i < count; ++i) {
            SkTArrayExt::copy(this, array);
        }
    }

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
        T* newT = reinterpret_cast<T*>(this->push_back_raw(1));
        SkNEW_PLACEMENT(newT, T);
        return *newT;
    }

    /**
     * Version of above that uses a copy constructor to initialize the new item
     */
    T& push_back(const T& t) {
        T* newT = reinterpret_cast<T*>(this->push_back_raw(1));
        SkNEW_PLACEMENT_ARGS(newT, T, (t));
        return *newT;
    }

    /**
     * Allocates n more default T values, and returns the address of the start
     * of that new range. Note: this address is only valid until the next API
     * call made on the array that might add or remove elements.
     */
    T* push_back_n(int n) {
        SkASSERT(n >= 0);
        T* newTs = reinterpret_cast<T*>(this->push_back_raw(n));
        for (int i = 0; i < n; ++i) {
            SkNEW_PLACEMENT(newTs + i, T);
        }
        return newTs;
    }

    /**
     * Version of above that uses a copy constructor to initialize all n items
     * to the same T.
     */
    T* push_back_n(int n, const T& t) {
        SkASSERT(n >= 0);
        T* newTs = reinterpret_cast<T*>(this->push_back_raw(n));
        for (int i = 0; i < n; ++i) {
            SkNEW_PLACEMENT_ARGS(newTs[i], T, (t));
        }
        return newTs;
    }

    /**
     * Version of above that uses a copy constructor to initialize the n items
     * to separate T values.
     */
    T* push_back_n(int n, const T t[]) {
        SkASSERT(n >= 0);
        this->checkRealloc(n);
        for (int i = 0; i < n; ++i) {
            SkNEW_PLACEMENT_ARGS(fItemArray + fCount + i, T, (t[i]));
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
        this->checkRealloc(0);
    }

    /**
     * Removes the last n elements. Not safe to call when count() < n.
     */
    void pop_back_n(int n) {
        SkASSERT(n >= 0);
        SkASSERT(fCount >= n);
        fCount -= n;
        for (int i = 0; i < n; ++i) {
            fItemArray[fCount + i].~T();
        }
        this->checkRealloc(0);
    }

    /**
     * Pushes or pops from the back to resize. Pushes will be default
     * initialized.
     */
    void resize_back(int newCount) {
        SkASSERT(newCount >= 0);

        if (newCount > fCount) {
            this->push_back_n(newCount - fCount);
        } else if (newCount < fCount) {
            this->pop_back_n(fCount - newCount);
        }
    }

    T* begin() {
        return fItemArray;
    }
    const T* begin() const {
        return fItemArray;
    }
    T* end() {
        return fItemArray ? fItemArray + fCount : NULL;
    }
    const T* end() const {
        return fItemArray ? fItemArray + fCount : NULL;;
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

    bool operator==(const SkTArray<T, MEM_COPY>& right) const {
        int leftCount = this->count();
        if (leftCount != right.count()) {
            return false;
        }
        for (int index = 0; index < leftCount; ++index) {
            if (fItemArray[index] != right.fItemArray[index]) {
                return false;
            }
        }
        return true;
    }

    bool operator!=(const SkTArray<T, MEM_COPY>& right) const {
        return !(*this == right);
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
        SkASSERT(count >= 0);
        SkASSERT(preAllocOrReserveCount >= 0);
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
            fAllocCount = SkMax32(fCount, fReserveCount);
            fMemArray = sk_malloc_throw(fAllocCount * sizeof(T));
        }

        SkTArrayExt::copy(this, array);
    }

private:

    static const int gMIN_ALLOC_COUNT = 8;

    // Helper function that makes space for n objects, adjusts the count, but does not initialize
    // the new objects.
    void* push_back_raw(int n) {
        this->checkRealloc(n);
        void* ptr = fItemArray + fCount;
        fCount += n;
        return ptr;
    }

    inline void checkRealloc(int delta) {
        SkASSERT(fCount >= 0);
        SkASSERT(fAllocCount >= 0);

        SkASSERT(-delta <= fCount);

        int newCount = fCount + delta;
        int newAllocCount = fAllocCount;

        if (newCount > fAllocCount || newCount < (fAllocCount / 3)) {
            // whether we're growing or shrinking, we leave at least 50% extra space for future
            // growth (clamped to the reserve count).
            newAllocCount = SkMax32(newCount + ((newCount + 1) >> 1), fReserveCount);
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

    friend void* operator new<T>(size_t, SkTArray*, int);

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

// Use the below macro (SkNEW_APPEND_TO_TARRAY) rather than calling this directly
template <typename T, bool MEM_COPY>
void* operator new(size_t, SkTArray<T, MEM_COPY>* array, int atIndex) {
    // Currently, we only support adding to the end of the array. When the array class itself
    // supports random insertion then this should be updated.
    // SkASSERT(atIndex >= 0 && atIndex <= array->count());
    SkASSERT(atIndex == array->count());
    return array->push_back_raw(1);
}

// Skia doesn't use C++ exceptions but it may be compiled with them enabled. Having an op delete
// to match the op new silences warnings about missing op delete when a constructor throws an
// exception.
template <typename T, bool MEM_COPY>
void operator delete(void*, SkTArray<T, MEM_COPY>* array, int atIndex) {
    SK_CRASH();
}

// Constructs a new object as the last element of an SkTArray.
#define SkNEW_APPEND_TO_TARRAY(array_ptr, type_name, args)  \
    (new ((array_ptr), (array_ptr)->count()) type_name args)


/**
 * Subclass of SkTArray that contains a preallocated memory block for the array.
 */
template <int N, typename T, bool MEM_COPY = false>
class SkSTArray : public SkTArray<T, MEM_COPY> {
private:
    typedef SkTArray<T, MEM_COPY> INHERITED;

public:
    SkSTArray() : INHERITED(&fStorage) {
    }

    SkSTArray(const SkSTArray& array)
        : INHERITED(array, &fStorage) {
    }

    explicit SkSTArray(const INHERITED& array)
        : INHERITED(array, &fStorage) {
    }

    explicit SkSTArray(int reserveCount)
        : INHERITED(reserveCount) {
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
