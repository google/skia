/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_INLINEDVECTOR
#define SKSL_INLINEDVECTOR

#include <algorithm>

#include "include/private/SkMalloc.h"

namespace SkSL {
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
 * SkSL::InlinedVector<N, T> implements a simple std::vector-like array with a small buffer inside
 * of the structure. The internal buffer will be used until there is more data than will fit. This
 * will save on malloc/free traffic for structures which are often small.
 *
 * This is adapted from SkSTArray, but attempts to hew more closely to the std::vector interface and
 * is a bit more efficient with its internal storage. Like vector (but unlike SkTArray), it does not
 * attempt to shrink automatically.
 */

template <int N, typename T> class InlinedVector {
public:
    /** Creates an empty vector with no initial storage. */
    InlinedVector() {
        this->initWithPreallocatedStorage(0);
    }

    /** Creates an empty vector containing `initialSize` elements. */
    explicit InlinedVector(int initialSize) {
        this->initWithPreallocatedStorage(0);
        this->push_back_n(initialSize);
    }

    /** Copies another vector into this one. */
    InlinedVector(const InlinedVector& that) {
        this->initWithPreallocatedStorage(that.fCount);
        this->copy(that.itemArray());
    }

    /** Moves another vector into this one. */
    InlinedVector(InlinedVector&& that) {
        if (that.fDidAllocate) {
            fCount = that.fCount;
            fDidAllocate = true;
            fAlloc.fItemArray = that.fAlloc.fItemArray;
            fAlloc.fCapacity = that.fAlloc.fCapacity;

            that.fCount = 0;
            that.fDidAllocate = false;
        } else {
            this->initWithPreallocatedStorage(that.fCount);
            that.move(this->itemArray());
            that.fCount = 0;
        }
    }

    /** Creates a vector containing a range of elements. */
    template <class InputIt>
    InlinedVector(InputIt rangeBegin, InputIt rangeEnd) {
        this->initWithPreallocatedStorage(rangeEnd - rangeBegin);
        this->copy(rangeBegin);
    }

    /** Copies another vector into this one. */
    InlinedVector& operator=(const InlinedVector& that) {
        if (this == &that) {
            return *this;
        }
        this->clear();
        this->checkRealloc(that.size());
        fCount = that.size();
        this->copy(that.itemArray());
        return *this;
    }

    /** Moves another vector into this one. */
    InlinedVector& operator=(InlinedVector&& that) {
        if (this == &that) {
            return *this;
        }
        this->clear();
        this->checkRealloc(that.size());
        fCount = that.size();
        that.move(this->itemArray());
        that.fCount = 0;
        return *this;
    }

    ~InlinedVector() {
        this->clear();
        if (fDidAllocate) {
            sk_free(fAlloc.fItemArray);
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
            this->checkRealloc(n - fCount);
        }
    }

    /** Are there zero items in the array? */
    bool empty() const { return !fCount; }

    /**
     * Adds one new value-initialized T value and returns it by reference.
     * The reference only remains valid until the next call that adds or removes elements.
     */
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
            this->push_back_n(newSize - fCount);
        } else if (newSize < fCount) {
            this->pop_back_n(fCount - newSize);
        }
    }

    /** Swaps the contents of this array with that array. Swaps pointers if possible,
        otherwise copies the T values. */
    void swap(InlinedVector& that) {
        using std::swap;
        if (this == &that) {
            return;
        }
        if (fDidAllocate && that.fDidAllocate) {
            swap(fAlloc.fItemArray, that.fAlloc.fItemArray);
            swap(fAlloc.fCapacity, that.fAlloc.fCapacity);
            // std::swap doesn't support bitfields, so we swap fCount by hand.
            int thatCount = that.fCount;
            that.fCount = fCount;
            fCount = thatCount;
        } else {
            InlinedVector temp(std::move(that));
            that = std::move(*this);
            *this = std::move(temp);
        }
    }

    /** Accesses the first element of the vector. */
    T* begin() {
        return this->itemArray();
    }
    const T* begin() const {
        return this->itemArray();
    }
    T* data() {
        return this->itemArray();
    }
    const T* data() const {
        return this->itemArray();
    }
    /** Accesses the last element of the vector. */
    T* end() {
        return this->itemArray() + fCount;
    }
    const T* end() const {
        return this->itemArray() + fCount;
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
        return this->itemArray()[i];
    }
    const T& operator[](int i) const {
        SkASSERT(i < fCount);
        SkASSERT(i >= 0);
        return this->itemArray()[i];
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
    bool operator==(const InlinedVector<N, T>& that) const {
        int thisSize = this->size();
        if (thisSize != that.size()) {
            return false;
        }

        T* thisItemArray = this->itemArray();
        T* thatItemArray = that.itemArray();
        for (int index = 0; index < thisSize; ++index) {
            if (thisItemArray[index] != thatItemArray[index]) {
                return false;
            }
        }
        return true;
    }

    /** Compare elements for inequality. */
    bool operator!=(const InlinedVector<N, T>& that) const {
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
            fAlloc.fItemArray = (T*)sk_malloc_throw(fAlloc.fCapacity, sizeof(T));
        }
    }

    void copy(const T* src) {
        // Copy items from the `src` array into this vector.
        // The destination item array is assumed to start as uninitialized raw storage.
        T* thisItemArray = this->itemArray();
        for (int i = 0; i < fCount; ++i) {
            new (&thisItemArray[i]) T(src[i]);
        }
    }

    void move(T* dst) {
        // Move all of our items out of this vector and into the pre-sized array in `dst`.
        // The source item array is destroyed, leaving behind uninitialized raw storage.
        // `dst` is assumed to start as uninitialized raw storage.
        T* thisItemArray = this->itemArray();
        for (int i = 0; i < fCount; ++i) {
            new (&dst[i]) T(std::move(thisItemArray[i]));
            thisItemArray[i].~T();
        }
    }

    static constexpr int kMinHeapAllocCount = 8;

    /**
     * Allocates n more value-initialized T values, and returns the address of
     * the start of that new range. This address is only valid until the next API
     * call made on the array that might add or remove elements.
     */
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
        T* thisItemArray = this->itemArray();
        for (int i = 0; i < n; ++i) {
            thisItemArray[fCount + i].~T();
        }
    }

    /** Helper function that makes space for n objects, adjusts the count, but does not initialize
        the new objects. */
    void* push_back_raw(int n) {
        this->checkRealloc(n);
        T* ptr = this->itemArray() + fCount;
        fCount += n;
        return ptr;
    }

    /** Ensures that we have sufficient capacity for `delta` new elements. */
    void checkRealloc(int delta) {
        SkASSERT(fCount >= 0);
        SkASSERT(this->capacity() >= 0);
        SkASSERT(-delta <= fCount);

        // Move into 64bit math temporarily, to avoid local overflows
        int64_t newCount = fCount + delta;
        if (this->capacity() >= newCount) {
            return;
        }

        // We leave at least 50% extra space over the requested delta for future growth.
        // TODO: calls to `reserve` or `resize` should honor the user's requested amount.
        int64_t newAllocCount = newCount + ((newCount + 1) >> 1);
        // Align the new allocation count to kMinHeapAllocCount.
        static_assert(internal::is_pow2(kMinHeapAllocCount), "min alloc count not power of two.");
        newAllocCount = (newAllocCount + (kMinHeapAllocCount - 1)) & ~(kMinHeapAllocCount - 1);
        // At small sizes the old and new alloc count can both be kMinHeapAllocCount.
        if (newAllocCount == this->capacity()) {
            return;
        }

        newAllocCount = internal::pin_s64_to_s32(newAllocCount);
        SkASSERT(newAllocCount >= newCount);
        T* newItemArray = (T*)sk_malloc_throw((size_t)newAllocCount, sizeof(T));
        this->move(newItemArray);
        if (fDidAllocate) {
            sk_free(fAlloc.fItemArray);
        }
        fAlloc.fItemArray = newItemArray;
        fAlloc.fCapacity = (int)newAllocCount;
        fDidAllocate = true;
    }

    T* itemArray() {
        return fDidAllocate ? fAlloc.fItemArray : reinterpret_cast<T*>(fStorage.get());
    }

    const T* itemArray() const {
        return fDidAllocate ? fAlloc.fItemArray : reinterpret_cast<const T*>(fStorage.get());
    }

    // The number of items in this vector. (The size, not the capacity.)
    int fCount : 31;

    // We steal a bit from our item count to indicate how our internal storage is used.
    int fDidAllocate : 1;

    // When `fDidAllocate` is false, `fStorage` is used to hold the contents of our vector.
    // When `fDidAllocate` is true, `fAlloc` is used to point to our malloced items.
    // `checkRealloc` is responsible for migrating data when the vector grows.
    struct Allocated {
        int fCapacity;
        T* fItemArray;
    };
    union {
        internal::AlignedStorage<N, T> fStorage;
        Allocated fAlloc;
    };
};

template <int N, typename T>
static inline void swap(InlinedVector<N, T>& a, InlinedVector<N, T>& b) {
    a.swap(b);
}

template <int N, typename T> constexpr int InlinedVector<N, T>::kMinHeapAllocCount;

}  // namespace SkSL

#endif
