
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

// DATA_TYPE indicates that T has a trivial cons, destructor
// and can be shallow-copied
template <typename T, bool DATA_TYPE = false> class SkTArray {
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
        SkASSERT(reserveCount >= 0);
        fCount = 0;
        fReserveCount = reserveCount > gMIN_ALLOC_COUNT ? reserveCount :
                                                          gMIN_ALLOC_COUNT;
        fAllocCount = fReserveCount;
        fMemArray = sk_malloc_throw(sizeof(T) * fReserveCount);
        fPreAllocMemArray = NULL;
    }

    /**
     * Creates an empty array that will use the passed storage block until it
     * is insufficiently large to hold the entire array.
     */
    template <int N>
    SkTArray(SkAlignedSTStorage<N,T>* storage) {
        SkASSERT(N > 0);
        fCount              = 0;
        fReserveCount       = N;
        fAllocCount         = N;
        fMemArray           = storage->get();
        fPreAllocMemArray   = storage->get();
    }

    /**
     * Creates an empty array that will use the passed memory block until the
     * count exceeds preAllocCount. Be careful not to use this constructor
     * when you really want the (T*, int) version.
     */
    SkTArray(void* preAllocStorage, int preAllocCount) {
        SkASSERT(preAllocCount >= 0);
        // we allow NULL,0 args and revert to the default cons. behavior
        // this makes it possible for a owner-object to use same constructor
        // to get either prealloc or nonprealloc behavior based using same line
        SkASSERT((NULL == preAllocStorage) == !preAllocCount);

        fCount              = 0;
        fReserveCount       = preAllocCount > 0 ? preAllocCount :
                                                  gMIN_ALLOC_COUNT;
        fAllocCount         = preAllocCount;
        fMemArray           = preAllocStorage;
        fPreAllocMemArray   = preAllocStorage;
    }

    /**
     * Copies one array to another. The new array will be heap allocated.
     */
    explicit SkTArray(const SkTArray& array) {
        fCount              = array.count();
        fReserveCount       = gMIN_ALLOC_COUNT;
        fAllocCount         = SkMax32(fReserveCount, fCount);
        fMemArray           = sk_malloc_throw(sizeof(T) * fAllocCount);
        fPreAllocMemArray   = NULL;

        if (DATA_TYPE) {
            memcpy(fMemArray, array.fMemArray, sizeof(T) * fCount);
        } else {
            for (int i = 0; i < fCount; ++i) {
                new (fItemArray + i) T(array[i]);
            }
        }
    }

    /**
     * Creates a SkTArray by copying contents of a standard C array. The new 
     * array will be heap allocated. Be careful not to use this constructor
     * when you really want the (void*, int) version.
     */
    SkTArray(const T* array, int count) {
        SkASSERT(count >= 0);
        fCount              = count;
        fReserveCount       = gMIN_ALLOC_COUNT;
        fAllocCount         = SkMax32(fReserveCount, fCount);
        fMemArray           = sk_malloc_throw(sizeof(T) * fAllocCount);
        fPreAllocMemArray   = NULL;
        if (DATA_TYPE) {
            memcpy(fMemArray, array, sizeof(T) * fCount);
        } else {
            for (int i = 0; i < fCount; ++i) {
                new (fItemArray + i) T(array[i]);
            }
        }
    }

    /**
     * Copy another array, using preallocated storage if preAllocCount >=
     * array.count(). Otherwise preAllocStorage is only used if the array
     * shrinks to fit.
     */
    SkTArray(const SkTArray& array,
             void* preAllocStorage, int preAllocCount) {

        SkASSERT(preAllocCount >= 0);

        // for same reason as non-copying cons we allow NULL, 0 for prealloc
        SkASSERT((NULL == preAllocStorage) == !preAllocCount);

        fCount              = array.count();
        fReserveCount       = preAllocCount > 0 ? preAllocCount :
                                                  gMIN_ALLOC_COUNT;
        fPreAllocMemArray   = preAllocStorage;

        if (fReserveCount >= fCount && preAllocCount) {
            fAllocCount = fReserveCount;
            fMemArray = preAllocStorage;
        } else {
            fAllocCount = SkMax32(fCount, fReserveCount);
            fMemArray = sk_malloc_throw(fAllocCount * sizeof(T));
        }

        if (DATA_TYPE) {
            memcpy(fMemArray, array.fMemArray, sizeof(T) * fCount);
        } else {
            for (int i = 0; i < fCount; ++i) {
                new (fItemArray + i) T(array[i]);
            }
        }
    }

    /**
     * Copy C array to SkTArray, using preallocated storage if preAllocCount >=
     * preAllocCount. Otherwise preAllocStorage is only used if the array
     * shrinks to fit.
     */
    SkTArray(const T* array, int count,
             void* preAllocStorage, int preAllocCount) {

        SkASSERT(count >= 0);
        SkASSERT(preAllocCount >= 0);

        // for same reason as non-copying cons we allow NULL, 0 for prealloc
        SkASSERT((NULL == preAllocStorage) == !preAllocCount);

        fCount              = count;
        fReserveCount       = (preAllocCount > 0) ? preAllocCount :
                                                    gMIN_ALLOC_COUNT;
        fPreAllocMemArray   = preAllocStorage;

        if (fReserveCount >= fCount && preAllocCount) {
            fAllocCount = fReserveCount;
            fMemArray = preAllocStorage;
        } else {
            fAllocCount = SkMax32(fCount, fReserveCount);
            fMemArray = sk_malloc_throw(fAllocCount * sizeof(T));
        }

        if (DATA_TYPE) {
            memcpy(fMemArray, array, sizeof(T) * fCount);
        } else {
            for (int i = 0; i < fCount; ++i) {
                new (fItemArray + i) T(array[i]);
            }
        }
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
        if (DATA_TYPE) {
            memcpy(fMemArray, array.fMemArray, sizeof(T) * fCount);
        } else {
            for (int i = 0; i < fCount; ++i) {
                new (fItemArray + i) T(array[i]);
            }
        }
        return *this;
    }

    ~SkTArray() {
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

private:

    static const int gMIN_ALLOC_COUNT = 8;

    inline void checkRealloc(int delta) {
        SkASSERT(fCount >= 0);
        SkASSERT(fAllocCount >= 0);

        SkASSERT(-delta <= fCount);

        int newCount = fCount + delta;
        int fNewAllocCount = fAllocCount;

        if (newCount > fAllocCount) {
            fNewAllocCount = SkMax32(newCount + ((newCount + 1) >> 1),
                                   fReserveCount);
        } else if (newCount < fAllocCount / 3) {
            fNewAllocCount = SkMax32(fAllocCount / 2, fReserveCount);
        }

        if (fNewAllocCount != fAllocCount) {

            fAllocCount = fNewAllocCount;
            char* fNewMemArray;

            if (fAllocCount == fReserveCount && NULL != fPreAllocMemArray) {
                fNewMemArray = (char*) fPreAllocMemArray;
            } else {
                fNewMemArray = (char*) sk_malloc_throw(fAllocCount*sizeof(T));
            }

            if (DATA_TYPE) {
                memcpy(fNewMemArray, fMemArray, fCount * sizeof(T));
            } else {
                for (int i = 0; i < fCount; ++i) {
                    new (fNewMemArray + sizeof(T) * i) T(fItemArray[i]);
                    fItemArray[i].~T();
                }
            }

            if (fMemArray != fPreAllocMemArray) {
                sk_free(fMemArray);
            }
            fMemArray = fNewMemArray;
        }
    }

    int fReserveCount;
    int fCount;
    int fAllocCount;
    void*    fPreAllocMemArray;
    union {
        T*       fItemArray;
        void*    fMemArray;
    };
};

#endif

