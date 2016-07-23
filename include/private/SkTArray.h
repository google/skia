/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTArray_DEFINED
#define SkTArray_DEFINED

#include "../private/SkTLogic.h"
#include "../private/SkTemplates.h"
#include "SkTypes.h"

#include <new>
#include <utility>

/** When MEM_COPY is true T will be bit copied when moved.
    When MEM_COPY is false, T will be copy constructed / destructed.
    In all cases T will be default-initialized on allocation,
    and its destructor will be called from this object's destructor.
*/
template <typename T, bool MEM_COPY = false> class SkTArray {
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
        this->init(0, NULL, reserveCount);
    }

    /**
     * Copies one array to another. The new array will be heap allocated.
     */
    explicit SkTArray(const SkTArray& that) {
        this->init(that.fCount, NULL, 0);
        this->copy(that.fItemArray);
    }
    explicit SkTArray(SkTArray&& that) {
        this->init(that.fCount, NULL, 0);
        that.move(fMemArray);
        that.fCount = 0;
    }

    /**
     * Creates a SkTArray by copying contents of a standard C array. The new
     * array will be heap allocated. Be careful not to use this constructor
     * when you really want the (void*, int) version.
     */
    SkTArray(const T* array, int count) {
        this->init(count, NULL, 0);
        this->copy(array);
    }

    /**
     * assign copy of array to this
     */
    SkTArray& operator =(const SkTArray& that) {
        for (int i = 0; i < fCount; ++i) {
            fItemArray[i].~T();
        }
        fCount = 0;
        this->checkRealloc(that.count());
        fCount = that.count();
        this->copy(that.fItemArray);
        return *this;
    }
    SkTArray& operator =(SkTArray&& that) {
        for (int i = 0; i < fCount; ++i) {
            fItemArray[i].~T();
        }
        fCount = 0;
        this->checkRealloc(that.count());
        fCount = that.count();
        that.move(fMemArray);
        that.fCount = 0;
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
     * Resets to count() = n newly constructed T objects.
     */
    void reset(int n) {
        SkASSERT(n >= 0);
        for (int i = 0; i < fCount; ++i) {
            fItemArray[i].~T();
        }
        // Set fCount to 0 before calling checkRealloc so that no elements are moved.
        fCount = 0;
        this->checkRealloc(n);
        fCount = n;
        for (int i = 0; i < fCount; ++i) {
            new (fItemArray + i) T;
        }
    }

    /**
     * Ensures there is enough reserved space for n elements.
     */
    void reserve(int n) {
        if (fCount < n) {
            this->checkRealloc(n - fCount);
        }
    }

    /**
     * Resets to a copy of a C array.
     */
    void reset(const T* array, int count) {
        for (int i = 0; i < fCount; ++i) {
            fItemArray[i].~T();
        }
        fCount = 0;
        this->checkRealloc(count);
        fCount = count;
        this->copy(array);
    }

    void removeShuffle(int n) {
        SkASSERT(n < fCount);
        int newCount = fCount - 1;
        fCount = newCount;
        fItemArray[n].~T();
        if (n != newCount) {
            this->move(n, newCount);
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
     * Adds 1 new default-initialized T value and returns it by reference. Note
     * the reference only remains valid until the next call that adds or removes
     * elements.
     */
    T& push_back() {
        void* newT = this->push_back_raw(1);
        return *new (newT) T;
    }

    /**
     * Version of above that uses a copy constructor to initialize the new item
     */
    T& push_back(const T& t) {
        void* newT = this->push_back_raw(1);
        return *new (newT) T(t);
    }

    /**
     * Version of above that uses a move constructor to initialize the new item
     */
    T& push_back(T&& t) {
        void* newT = this->push_back_raw(1);
        return *new (newT) T(std::move(t));
    }

    /**
     *  Construct a new T at the back of this array.
     */
    template<class... Args> T& emplace_back(Args&&... args) {
        void* newT = this->push_back_raw(1);
        return *new (newT) T(std::forward<Args>(args)...);
    }

    /**
     * Allocates n more default-initialized T values, and returns the address of
     * the start of that new range. Note: this address is only valid until the
     * next API call made on the array that might add or remove elements.
     */
    T* push_back_n(int n) {
        SkASSERT(n >= 0);
        void* newTs = this->push_back_raw(n);
        for (int i = 0; i < n; ++i) {
            new (static_cast<char*>(newTs) + i * sizeof(T)) T;
        }
        return static_cast<T*>(newTs);
    }

    /**
     * Version of above that uses a copy constructor to initialize all n items
     * to the same T.
     */
    T* push_back_n(int n, const T& t) {
        SkASSERT(n >= 0);
        void* newTs = this->push_back_raw(n);
        for (int i = 0; i < n; ++i) {
            new (static_cast<char*>(newTs) + i * sizeof(T)) T(t);
        }
        return static_cast<T*>(newTs);
    }

    /**
     * Version of above that uses a copy constructor to initialize the n items
     * to separate T values.
     */
    T* push_back_n(int n, const T t[]) {
        SkASSERT(n >= 0);
        this->checkRealloc(n);
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

    /** Swaps the contents of this array with that array. Does a pointer swap if possible,
        otherwise copies the T values. */
    void swap(SkTArray* that) {
        if (this == that) {
            return;
        }
        if (this->fPreAllocMemArray != this->fItemArray &&
            that->fPreAllocMemArray != that->fItemArray) {
            // If neither is using a preallocated array then just swap.
            SkTSwap(fItemArray, that->fItemArray);
            SkTSwap(fCount, that->fCount);
            SkTSwap(fAllocCount, that->fAllocCount);
        } else {
            // This could be more optimal...
            SkTArray copy(std::move(*that));
            *that = std::move(*this);
            *this = std::move(copy);
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
        return fItemArray ? fItemArray + fCount : NULL;
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
        this->init(0, storage->get(), N);
    }

    /**
     * Copy another array, using preallocated storage if preAllocCount >=
     * array.count(). Otherwise storage will only be used when array shrinks
     * to fit.
     */
    template <int N>
    SkTArray(const SkTArray& array, SkAlignedSTStorage<N,T>* storage) {
        this->init(array.fCount, storage->get(), N);
        this->copy(array.fItemArray);
    }

    /**
     * Copy a C array, using preallocated storage if preAllocCount >=
     * count. Otherwise storage will only be used when array shrinks
     * to fit.
     */
    template <int N>
    SkTArray(const T* array, int count, SkAlignedSTStorage<N,T>* storage) {
        this->init(count, storage->get(), N);
        this->copy(array);
    }

    void init(int count, void* preAllocStorage, int preAllocOrReserveCount) {
        SkASSERT(count >= 0);
        SkASSERT(preAllocOrReserveCount >= 0);
        fCount              = count;
        fReserveCount       = (preAllocOrReserveCount > 0) ?
                                    preAllocOrReserveCount :
                                    gMIN_ALLOC_COUNT;
        fPreAllocMemArray   = preAllocStorage;
        if (fReserveCount >= fCount &&
            preAllocStorage) {
            fAllocCount = fReserveCount;
            fMemArray = preAllocStorage;
        } else {
            fAllocCount = SkMax32(fCount, fReserveCount);
            fMemArray = sk_malloc_throw(fAllocCount * sizeof(T));
        }
    }

private:
    /** In the following move and copy methods, 'dst' is assumed to be uninitialized raw storage.
     *  In the following move methods, 'src' is destroyed leaving behind uninitialized raw storage.
     */
    template <bool E = MEM_COPY> SK_WHEN(E, void) copy(const T* src) {
        sk_careful_memcpy(fMemArray, src, fCount * sizeof(T));
    }
    template <bool E = MEM_COPY> SK_WHEN(E, void) move(int dst, int src) {
        memcpy(&fItemArray[dst], &fItemArray[src], sizeof(T));
    }
    template <bool E = MEM_COPY> SK_WHEN(E, void) move(void* dst) {
        sk_careful_memcpy(dst, fMemArray, fCount * sizeof(T));
    }

    template <bool E = MEM_COPY> SK_WHEN(!E, void) copy(const T* src) {
        for (int i = 0; i < fCount; ++i) {
            new (fItemArray + i) T(src[i]);
        }
    }
    template <bool E = MEM_COPY> SK_WHEN(!E, void) move(int dst, int src) {
        new (&fItemArray[dst]) T(std::move(fItemArray[src]));
        fItemArray[src].~T();
    }
    template <bool E = MEM_COPY> SK_WHEN(!E, void) move(void* dst) {
        for (int i = 0; i < fCount; ++i) {
            new (static_cast<char*>(dst) + sizeof(T) * i) T(std::move(fItemArray[i]));
            fItemArray[i].~T();
        }
    }

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
            void* newMemArray;

            if (fAllocCount == fReserveCount && fPreAllocMemArray) {
                newMemArray = fPreAllocMemArray;
            } else {
                newMemArray = sk_malloc_throw(fAllocCount*sizeof(T));
            }

            this->move(newMemArray);

            if (fMemArray != fPreAllocMemArray) {
                sk_free(fMemArray);
            }
            fMemArray = newMemArray;
        }
    }

    int     fReserveCount;
    int     fCount;
    int     fAllocCount;
    void*   fPreAllocMemArray;
    union {
        T*       fItemArray;
        void*    fMemArray;
    };
};

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
