/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_INLINEDVECTOR
#define SKSL_INLINEDVECTOR

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
    /**
     * Creates an empty vector with no initial storage
     */
    InlinedVector() {
        this->initWithPreallocatedStorage(0, fStorage.get(), N);
    }

    /**
     * Creates an empty vector containing initialSize elements.
     */
    explicit InlinedVector(int initialSize) {
        this->initWithPreallocatedStorage(0, fStorage.get(), N);
        this->push_back_n(initialSize);
    }

    /**
     * Copies another vector into this one.
     */
    InlinedVector(const InlinedVector& that) {
        this->initWithPreallocatedStorage(that.fCount, fStorage.get(), N);
        this->copy(that.fItemArray);
    }

    /**
     * Moves another vector into this one.
     */
    InlinedVector(InlinedVector&& that) {
        if (that.fOwnMemory) {
            fItemArray = that.fItemArray;
            fCount = that.fCount;
            fAllocCount = that.fAllocCount;
            fOwnMemory = true;

            that.fItemArray = nullptr;
            that.fCount = 0;
            that.fAllocCount = 0;
            that.fOwnMemory = true;
        } else {
            this->initWithPreallocatedStorage(that.fCount, fStorage.get(), N);
            that.move(fItemArray);
            that.fCount = 0;
        }
    }

    /**
     * Copy from a C array.
     */
    template <class InputIt>
    InlinedVector(InputIt array, InputIt endArray) {
        this->initWithPreallocatedStorage(endArray - array, fStorage.get(), N);
        this->copy(array);
    }

    InlinedVector& operator=(const InlinedVector& that) {
        if (this == &that) {
            return *this;
        }
        for (int i = 0; i < fCount; ++i) {
            fItemArray[i].~T();
        }
        fCount = 0;
        this->checkRealloc(that.size());
        fCount = that.size();
        this->copy(that.fItemArray);
        return *this;
    }

    InlinedVector& operator=(InlinedVector&& that) {
        if (this == &that) {
            return *this;
        }
        for (int i = 0; i < fCount; ++i) {
            fItemArray[i].~T();
        }
        fCount = 0;
        this->checkRealloc(that.size());
        fCount = that.size();
        that.move(fItemArray);
        that.fCount = 0;
        return *this;
    }

    ~InlinedVector() {
        for (int i = 0; i < fCount; ++i) {
            fItemArray[i].~T();
        }
        if (fOwnMemory) {
            sk_free(fItemArray);
        }
    }

    /**
     * Resets to count() == 0 and resets any reserve count.
     */
    void clear() {
        this->pop_back_n(fCount);
    }

    /**
     * Ensures there is enough reserved space to hold n elements total.
     */
    void reserve(int n) {
        SkASSERT(n >= 0);
        if (n > 0) {
            this->checkRealloc(n - fCount);
        }
    }

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
     * Removes the last element. Not safe to call when count() == 0.
     */
    void pop_back() {
        SkASSERT(fCount > 0);
        --fCount;
        fItemArray[fCount].~T();
    }

    /**
     * Pushes or pops from the back to resize. Pushes will be default
     * initialized.
     */
    void resize(int newCount) {
        SkASSERT(newCount >= 0);

        if (newCount > fCount) {
            this->push_back_n(newCount - fCount);
        } else if (newCount < fCount) {
            this->pop_back_n(fCount - newCount);
        }
    }

    /** Swaps the contents of this array with that array. Does a pointer swap if possible,
        otherwise copies the T values. */
    void swap(InlinedVector& that) {
        using std::swap;
        if (this == &that) {
            return;
        }
        if (fOwnMemory && that.fOwnMemory) {
            swap(fItemArray, that.fItemArray);
            // std::swap doesn't support bitfields.
            int thatCount = that.fCount;
            that.fCount = fCount;
            fCount = thatCount;
            int thatAllocCount = that.fAllocCount;
            that.fAllocCount = fAllocCount;
            fAllocCount = thatAllocCount;
        } else {
            // This could be more optimal...
            InlinedVector copy(std::move(that));
            that = std::move(*this);
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
        return fItemArray ? fItemArray + fCount : nullptr;
    }
    const T* end() const {
        return fItemArray ? fItemArray + fCount : nullptr;
    }
    T* data() { return fItemArray; }
    const T* data() const { return fItemArray; }
    int size() const { return fCount; }
    int capacity() const { return fAllocCount; }

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

    T& at(int i) { return (*this)[i]; }
    const T& at(int i) const { return (*this)[i]; }

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

    bool operator==(const InlinedVector<N, T>& right) const {
        int leftCount = this->size();
        if (leftCount != right.size()) {
            return false;
        }
        for (int index = 0; index < leftCount; ++index) {
            if (fItemArray[index] != right.fItemArray[index]) {
                return false;
            }
        }
        return true;
    }

    bool operator!=(const InlinedVector<N, T>& right) const {
        return !(*this == right);
    }

private:
    void init(int count = 0, int reserveCount = 0) {
        SkASSERT(count >= 0);
        SkASSERT(reserveCount >= 0);
        fCount = count;
        fOwnMemory = true;
        if (!count && !reserveCount) {
            fAllocCount = 0;
            fItemArray = nullptr;
        } else {
            fAllocCount = std::max(count, std::max(kMinHeapAllocCount, reserveCount));
            fItemArray = (T*)sk_malloc_throw((size_t)fAllocCount, sizeof(T));
        }
    }

    void initWithPreallocatedStorage(int count, void* preallocStorage, int preallocCount) {
        SkASSERT(count >= 0);
        SkASSERT(preallocCount > 0);
        SkASSERT(preallocStorage);
        fCount = count;
        fItemArray = nullptr;
        if (count > preallocCount) {
            fAllocCount = std::max(count, kMinHeapAllocCount);
            fItemArray = (T*)sk_malloc_throw(fAllocCount, sizeof(T));
            fOwnMemory = true;
        } else {
            fAllocCount = preallocCount;
            fItemArray = (T*)preallocStorage;
            fOwnMemory = false;
        }
    }

    /** In the following move and copy methods, 'dst' is assumed to be uninitialized raw storage.
     *  In the following move methods, 'src' is destroyed leaving behind uninitialized raw storage.
     */
    void copy(const T* src) {
        // Some types may be trivially copyable, in which case we *could* use memcopy; but
        // MEM_MOVE == true implies that the type is trivially movable, and not necessarily
        // trivially copyable (think sk_sp<>).  So short of adding another template arg, we
        // must be conservative and use copy construction.
        for (int i = 0; i < fCount; ++i) {
            new (fItemArray + i) T(src[i]);
        }
    }

    void move(int dst, int src) {
        new (&fItemArray[dst]) T(std::move(fItemArray[src]));
        fItemArray[src].~T();
    }
    void move(void* dst) {
        for (int i = 0; i < fCount; ++i) {
            new (static_cast<char*>(dst) + sizeof(T) * (size_t)i) T(std::move(fItemArray[i]));
            fItemArray[i].~T();
        }
    }

    static constexpr int kMinHeapAllocCount = 8;

    /**
     * Allocates n more value-initialized T values, and returns the address of
     * the start of that new range. Note: this address is only valid until the
     * next API call made on the array that might add or remove elements.
     */
    T* push_back_n(int n) {
        SkASSERT(n >= 0);
        void* newTs = this->push_back_raw(n);
        for (int i = 0; i < n; ++i) {
            new (static_cast<char*>(newTs) + i * sizeof(T)) T();
        }
        return static_cast<T*>(newTs);
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
    }

    // Helper function that makes space for n objects, adjusts the count, but does not initialize
    // the new objects.
    void* push_back_raw(int n) {
        this->checkRealloc(n);
        void* ptr = fItemArray + fCount;
        fCount += n;
        return ptr;
    }

    void checkRealloc(int delta) {
        SkASSERT(fCount >= 0);
        SkASSERT(fAllocCount >= 0);
        SkASSERT(-delta <= fCount);

        // Move into 64bit math temporarily, to avoid local overflows
        int64_t newCount = fCount + delta;

        // We allow fAllocCount to be in the range [newCount, 3*newCount]. We never shrink.
        bool mustGrow = newCount > fAllocCount;
        if (!mustGrow) {
            return;
        }

        // Whether we're growing or shrinking, we leave at least 50% extra space for future growth.
        int64_t newAllocCount = newCount + ((newCount + 1) >> 1);
        // Align the new allocation count to kMinHeapAllocCount.
        static_assert(internal::is_pow2(kMinHeapAllocCount), "min alloc count not power of two.");
        newAllocCount = (newAllocCount + (kMinHeapAllocCount - 1)) & ~(kMinHeapAllocCount - 1);
        // At small sizes the old and new alloc count can both be kMinHeapAllocCount.
        if (newAllocCount == fAllocCount) {
            return;
        }

        fAllocCount = internal::pin_s64_to_s32(newAllocCount);
        SkASSERT(fAllocCount >= newCount);
        T* newItemArray = (T*)sk_malloc_throw((size_t)fAllocCount, sizeof(T));
        this->move(newItemArray);
        if (fOwnMemory) {
            sk_free(fItemArray);

        }
        fItemArray = newItemArray;
        fOwnMemory = true;
    }

    int fCount : 31;
    int fOwnMemory : 1;
    // TODO: these 96 bits of storage are free real estate when "fOwnMemory" is true.
    int fAllocCount;
    T* fItemArray;
    internal::AlignedStorage<N, T> fStorage;
};

template <int N, typename T>
static inline void swap(InlinedVector<N, T>& a, InlinedVector<N, T>& b) {
    a.swap(b);
}

template <int N, typename T> constexpr int InlinedVector<N, T>::kMinHeapAllocCount;

}  // namespace SkSL

#endif
