/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_INLINEDVECTOR
#define SKSL_INLINEDVECTOR

#include <algorithm>

#ifndef SKSL_STANDALONE
    #include "include/private/SkMalloc.h"
#else
    #include <stdlib.h>
    #define sk_malloc_throw(a, b)  malloc(a * b)
    #define sk_free(p)             free(p)
#endif

namespace internal {

/**
 * Adapted from SkAlignedSTStorage.
 */
template <int N, typename T> class AlignedStorage {
public:
    AlignedStorage() = default;

    AlignedStorage(AlignedStorage&&) = delete;
    AlignedStorage(const AlignedStorage&) = delete;
    AlignedStorage& operator=(AlignedStorage&&) = delete;
    AlignedStorage& operator=(const AlignedStorage&) = delete;

    /**
     * Returns void* because this object does not initialize the
     * memory. Use placement new for types that require a cons.
     */
    size_t size() const { return N; }
    void* get() { return fData; }
    const void* get() const { return fData; }

private:
    union {
        void*   fPtr;
        double  fDouble;
        char    fData[sizeof(T) * N];
    };
};

static constexpr int32_t pin_s64_to_s32(int64_t x) {
    return x < INT32_MIN ? INT32_MIN : (x > INT32_MAX ? INT32_MAX : (int32_t)x);
}

template <typename T> static constexpr inline bool is_pow2(T value) {
    return (value & (value - 1)) == 0;
}

}  // namespace internal

/**
 * SkInlinedVector<N, T> implements a simple std::vector-like array with a small buffer inside of
 * the structure. The internal buffer will be used until there is more data than will fit. This will
 * save on malloc/free traffic for vectors which generally contain only a few elements.
 *
 * This is adapted from SkSTArray, but attempts to hew more closely to the std::vector interface and
 * is a bit more efficient with its internal storage. Like vector (but unlike SkTArray), it does not
 * attempt to shrink automatically.
 */

template <int N, typename T> class SkInlinedVector {
public:
    /** Creates an empty vector with no initial storage. */
    SkInlinedVector() {
        this->initWithPreallocatedStorage(0);
    }

    /** Creates an empty vector containing `initialSize` elements. */
    explicit SkInlinedVector(int initialSize) {
        this->initWithPreallocatedStorage(0);
        this->push_back_n(initialSize);
    }

    /** Copies another vector into this one. */
    SkInlinedVector(const SkInlinedVector& that) {
        this->initWithPreallocatedStorage(that.fCount);
        this->copy(that.data());
    }

    /** Moves another vector into this one. */
    SkInlinedVector(SkInlinedVector&& that) {
        if (that.fDidAllocate) {
            fCount = that.fCount;
            fDidAllocate = true;
            fAlloc.fData = that.fAlloc.fData;
            fAlloc.fCapacity = that.fAlloc.fCapacity;

            that.fCount = 0;
            that.fDidAllocate = false;
        } else {
            this->initWithPreallocatedStorage(that.fCount);
            that.move(this->data());
            that.fCount = 0;
        }
    }

    /** Creates a vector containing a range of elements. */
    template <class InputIt>
    SkInlinedVector(InputIt rangeBegin, InputIt rangeEnd) {
        this->initWithPreallocatedStorage(rangeEnd - rangeBegin);
        this->copy(rangeBegin);
    }

    /** Copies another vector into this one. */
    SkInlinedVector& operator=(const SkInlinedVector& that) {
        if (this == &that) {
            return *this;
        }
        this->clear();
        this->checkReallocExact(that.size());
        fCount = that.size();
        this->copy(that.data());
        return *this;
    }

    /** Moves another vector into this one. */
    SkInlinedVector& operator=(SkInlinedVector&& that) {
        if (this == &that) {
            return *this;
        }
        this->clear();
        this->checkReallocExact(that.size());
        fCount = that.size();
        that.move(this->data());
        that.fCount = 0;
        return *this;
    }

    ~SkInlinedVector() {
        this->clear();
        if (fDidAllocate) {
            sk_free(fAlloc.fData);
        }
    }

    /** Removes all items from the vector, leaving capacity as-is. */
    void clear() {
        this->pop_back_n(fCount);
    }

    /** Ensures there is enough capacity to hold n elements total. */
    void reserve(int n) {
        SkASSERT(n >= 0);
        if (n > fCount) {
            this->checkReallocExact(n - fCount);
        }
    }

    /** Are there no items in the array? */
    bool empty() const { return !fCount; }

    /** Adds one new value-initialized T value and returns it by reference.
        The reference only remains valid until the next call that adds or removes elements. */
    T& push_back() {
        void* newT = this->push_back_raw(1);
        return *new (newT) T;
    }

    /** Adds one new copy-constructed T and returns it by reference. */
    T& push_back(const T& t) {
        void* newT = this->push_back_raw(1);
        return *new (newT) T(t);
    }

    /** Adds one new move-constructed T and returns it by reference. */
    T& push_back(T&& t) {
        void* newT = this->push_back_raw(1);
        return *new (newT) T(std::move(t));
    }

    /** Constructs a new T at the back of this array. */
    template <class... Args> T& emplace_back(Args&&... args) {
        void* newT = this->push_back_raw(1);
        return *new (newT) T(std::forward<Args>(args)...);
    }

    /** Removes the last element. Unsafe to call when empty(). */
    void pop_back() {
        this->pop_back_n(1);
    }

    /** Grows or shrinks the vector to the requested size. Pushes will be value-initialized. */
    void resize(int newSize) {
        SkASSERT(newSize >= 0);

        if (newSize > fCount) {
            int elemsToAdd = newSize - fCount;
            this->checkReallocExact(elemsToAdd);
            this->push_back_n(elemsToAdd);
        } else if (newSize < fCount) {
            this->pop_back_n(fCount - newSize);
        }
    }

    /** Swaps the contents of this array with that array. Swaps pointers if possible,
        otherwise copies the T values. */
    void swap(SkInlinedVector& that) {
        using std::swap;
        if (this == &that) {
            return;
        }
        if (fDidAllocate && that.fDidAllocate) {
            swap(fAlloc.fData, that.fAlloc.fData);
            swap(fAlloc.fCapacity, that.fAlloc.fCapacity);
            // std::swap doesn't support bitfields, so we swap fCount by hand.
            int thatCount = that.fCount;
            that.fCount = fCount;
            fCount = thatCount;
        } else {
            SkInlinedVector temp(std::move(that));
            that = std::move(*this);
            *this = std::move(temp);
        }
    }

    /** Accesses the first element of the vector. */
    T* data() {
        return fDidAllocate ? fAlloc.fData : reinterpret_cast<T*>(fStorage.get());
    }
    const T* data() const {
        return fDidAllocate ? fAlloc.fData : reinterpret_cast<const T*>(fStorage.get());
    }
    T* begin() {
        return this->data();
    }
    const T* begin() const {
        return this->data();
    }
    /** Accesses the last element of the vector. */
    T* end() {
        return this->data() + fCount;
    }
    const T* end() const {
        return this->data() + fCount;
    }
    /** Returns the number of elements in the vector. */
    int size() const {
        return fCount;
    }
    /** Returns the number of elements the vector is currently capable of holding. */
    int capacity() const {
        return fDidAllocate ? fAlloc.fCapacity : N;
    }
    /** Get the i'th element. */
    T& operator[](int i) {
        SkASSERT(i < fCount);
        SkASSERT(i >= 0);
        return this->data()[i];
    }
    const T& operator[](int i) const {
        SkASSERT(i < fCount);
        SkASSERT(i >= 0);
        return this->data()[i];
    }
    T& at(int i) {
        return (*this)[i];
    }
    const T& at(int i) const {
        return (*this)[i];
    }
    /** Equivalent to operator[](0). */
    T& front() {
        return this->operator[](0);
    }
    const T& front() const {
        return this->operator[](0);
    }
    /** Equivalent to operator[](size() - 1). */
    T& back() {
        return this->operator[](fCount - 1);
    }
    const T& back() const {
        return this->operator[](fCount - 1);
    }

    /** Compare all elements for equality. */
    bool operator==(const SkInlinedVector<N, T>& that) const {
        int thisSize = this->size();
        if (thisSize != that.size()) {
            return false;
        }

        T* thisData = this->data();
        T* thatData = that.data();
        for (int index = 0; index < thisSize; ++index) {
            if (thisData[index] != thatData[index]) {
                return false;
            }
        }
        return true;
    }

    /** Compare elements for inequality. */
    bool operator!=(const SkInlinedVector<N, T>& that) const {
        return !(*this == that);
    }

private:
    void initWithPreallocatedStorage(int count) {
        SkASSERT(count >= 0);
        fCount = count;
        if (count <= N) {
            fDidAllocate = false;
        } else {
            fDidAllocate = true;
            fAlloc.fCapacity = std::max(count, kMinHeapAllocCount);
            fAlloc.fData = (T*)sk_malloc_throw(fAlloc.fCapacity, sizeof(T));
        }
    }

    /** Copy items from the `src` array into this vector.
        The destination item array is assumed to start as uninitialized raw storage. */
    void copy(const T* src) {
        T* thisData = this->data();
        for (int i = 0; i < fCount; ++i) {
            new (&thisData[i]) T(src[i]);
        }
    }

    /** Move all of our items out of this vector and into the pre-sized array in `dst`.
        The source item array is destroyed, leaving behind uninitialized raw storage.
        `dst` is assumed to start as uninitialized raw storage. */
    void move(T* dst) {
        T* thisData = this->data();
        for (int i = 0; i < fCount; ++i) {
            new (&dst[i]) T(std::move(thisData[i]));
            thisData[i].~T();
        }
    }

    /** Allocates n more value-initialized T values, and returns the address of
        the start of that new range. This address is only valid until the next API
        call made on the array that might add or remove elements. */
    T* push_back_n(int n) {
        SkASSERT(n >= 0);
        T* newTs = static_cast<T*>(this->push_back_raw(n));
        for (int i = 0; i < n; ++i) {
            new (&newTs[i]) T();
        }
        return newTs;
    }

    /** Removes the last n elements. Not safe to call when size() < n. */
    void pop_back_n(int n) {
        SkASSERT(n >= 0);
        SkASSERT(fCount >= n);
        fCount -= n;
        T* thisData = this->data();
        for (int i = 0; i < n; ++i) {
            thisData[fCount + i].~T();
        }
    }

    /** Helper function that makes space for n objects, adjusts the count, but does not initialize
        the new objects. */
    void* push_back_raw(int n) {
        this->checkReallocGeometric(n);
        T* ptr = this->data() + fCount;
        fCount += n;
        return ptr;
    }

    /** Ensures that we have sufficient capacity for `delta` new elements, plus a ~50% buffer. */
    void checkReallocGeometric(int delta) {
        SkASSERT(fCount >= 0);
        SkASSERT(this->capacity() >= 0);
        SkASSERT(-delta <= fCount);

        // Move into 64bit math temporarily, to avoid local overflows
        int64_t newCount = fCount + delta;
        if (this->capacity() >= newCount) {
            return;
        }

        // We leave ~50% extra space over the requested delta for future growth.
        int64_t newAllocCount = newCount + ((newCount + 1) >> 1);

        // Align the new allocation count to kMinHeapAllocCount.
        static_assert(internal::is_pow2(kMinHeapAllocCount), "min alloc count not power of two.");
        newAllocCount = (newAllocCount + (kMinHeapAllocCount - 1)) & ~(kMinHeapAllocCount - 1);

        // At small sizes the old and new alloc count can both be kMinHeapAllocCount.
        if (newAllocCount <= this->capacity()) {
            return;
        }

        newAllocCount = internal::pin_s64_to_s32(newAllocCount);
        SkASSERT(newAllocCount >= newCount);
        this->doRealloc(newAllocCount);
    }

    /** Ensures that we have sufficient capacity for `delta` new elements exactly. */
    void checkReallocExact(int delta) {
        SkASSERT(fCount >= 0);
        SkASSERT(this->capacity() >= 0);
        SkASSERT(-delta <= fCount);

        int64_t newCount = fCount + delta;
        if (this->capacity() >= newCount) {
            return;
        }

        this->doRealloc(newCount);
    }

    /** Reallocs our storage to the exact requested amount. */
    void doRealloc(int64_t newAllocCount) {
        T* newItemArray = (T*)sk_malloc_throw((size_t)newAllocCount, sizeof(T));
        this->move(newItemArray);
        if (fDidAllocate) {
            sk_free(fAlloc.fData);
        }
        fAlloc.fData = newItemArray;
        fAlloc.fCapacity = (int)newAllocCount;
        fDidAllocate = true;
    }

    // No point in allocating fewer than 8 bytes on the heap.
    static constexpr int kMinHeapAllocCount = 8;

    // `fStorage` is our built-in storage, used when the data in inline.
    // `fAlloc` points to our out-of-line allocated storage.
    // (`doRealloc` is responsible for migrating data out of `fStorage` when the vector grows.)
    struct Allocated {
        int fCapacity;
        T* fData;
    };
    union {
        internal::AlignedStorage<N, T> fStorage;
        Allocated fAlloc;
    };

    // The number of items in this vector. (Specifically the `size()`, not the `capacity()`.)
    int fCount : 31;

    // This bit indicates whether `fAlloc` or `fStorage` is currently active. (True = `fAlloc`.)
    int fDidAllocate : 1;
};

template <int N, typename T>
static inline void swap(SkInlinedVector<N, T>& a, SkInlinedVector<N, T>& b) {
    a.swap(b);
}

#endif
