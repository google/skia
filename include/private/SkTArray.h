/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTArray_DEFINED
#define SkTArray_DEFINED

#include "include/core/SkMath.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/SkContainers.h"
#include "include/private/SkMalloc.h"
#include "include/private/SkSafe32.h"
#include "include/private/SkTLogic.h"
#include "include/private/SkTemplates.h"
#include "include/private/SkTo.h"

#include <algorithm>
#include <climits>
#include <string.h>
#include <initializer_list>
#include <memory>
#include <new>
#include <utility>

/** SkTArray<T> implements a typical, mostly std::vector-like array.
    Each T will be default-initialized on allocation, and ~T will be called on destruction.

    MEM_MOVE controls the behavior when a T needs to be moved (e.g. when the array is resized)
      - true: T will be bit-copied via memcpy.
      - false: T will be moved via move-constructors.

    Modern implementations of std::vector<T> will generally provide similar performance
    characteristics when used with appropriate care. Consider using std::vector<T> in new code.
*/
template <typename T, bool MEM_MOVE = sk_is_trivially_relocatable_v<T>> class SkTArray {
private:
    enum ReallocType { kExactFit, kGrowing };

public:
    using value_type = T;

    /**
     * Creates an empty array with no initial storage
     */
    SkTArray() { this->init(0); }

    /**
     * Creates an empty array that will preallocate space for reserveCount
     * elements.
     */
    explicit SkTArray(int reserveCount) : SkTArray() { this->reserve_back(reserveCount); }

    /**
     * Copies one array to another. The new array will be heap allocated.
     */
    SkTArray(const SkTArray& that)
        : SkTArray(that.fItemArray, that.fCount) {}

    SkTArray(SkTArray&& that) {
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
            this->init(that.fCount);
            that.move(fItemArray);
            that.fCount = 0;
        }
    }

    /**
     * Creates a SkTArray by copying contents of a standard C array. The new
     * array will be heap allocated. Be careful not to use this constructor
     * when you really want the (void*, int) version.
     */
    SkTArray(const T* array, int count) {
        this->init(count);
        this->copy(array);
    }
    /**
     * Creates a SkTArray by copying contents of an initializer list.
     */
    SkTArray(std::initializer_list<T> data)
        : SkTArray(data.begin(), data.size()) {}

    SkTArray& operator=(const SkTArray& that) {
        if (this == &that) {
            return *this;
        }
        this->clear();
        this->checkRealloc(that.count(), kExactFit);
        fCount = that.fCount;
        this->copy(that.fItemArray);
        return *this;
    }
    SkTArray& operator=(SkTArray&& that) {
        if (this == &that) {
            return *this;
        }
        for (int i = 0; i < this->count(); ++i) {
            fItemArray[i].~T();
        }
        fCount = 0;
        this->checkRealloc(that.count(), kExactFit);
        fCount = that.fCount;
        that.move(fItemArray);
        that.fCount = 0;
        return *this;
    }

    ~SkTArray() {
        this->clear();
        if (fOwnMemory) {
            sk_free(fItemArray);
        }
    }

    /**
     * Resets to count() == 0 and resets any reserve count.
     */
    void reset() {
        this->pop_back_n(fCount);
    }

    /**
     * Resets to count() = n newly constructed T objects and resets any reserve count.
     */
    void reset(int n) {
        SkASSERT(n >= 0);
        this->clear();
        this->checkRealloc(n, kExactFit);
        fCount = n;
        for (int i = 0; i < this->count(); ++i) {
            new (fItemArray + i) T;
        }
    }

    /**
     * Resets to a copy of a C array and resets any reserve count.
     */
    void reset(const T* array, int count) {
        SkASSERT(count >= 0);
        this->clear();
        this->checkRealloc(count, kExactFit);
        fCount = count;
        this->copy(array);
    }

    /**
     * Ensures there is enough reserved space for n elements.
     */
    void reserve(int n) {
        SkASSERT(n >= 0);
        if (n < count()) {
          return;
        }
        reserve_back(n - count());
    }

    /**
     * Ensures there is enough reserved space for n additional elements. The is guaranteed at least
     * until the array size grows above n and subsequently shrinks below n, any version of reset()
     * is called, or reserve_back() is called again.
     */
    void reserve_back(int n) {
        SkASSERT(n >= 0);
        if (n > 0) {
            this->checkRealloc(n, kExactFit);
        }
    }

    void removeShuffle(int n) {
        SkASSERT(n < this->count());
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
        T* newTs = TCast(this->push_back_raw(n));
        for (int i = 0; i < n; ++i) {
            new (&newTs[i]) T;
        }
        return newTs;
    }

    /**
     * Version of above that uses a copy constructor to initialize all n items
     * to the same T.
     */
    T* push_back_n(int n, const T& t) {
        SkASSERT(n >= 0);
        T* newTs = TCast(this->push_back_raw(n));
        for (int i = 0; i < n; ++i) {
            new (&newTs[i]) T(t);
        }
        return static_cast<T*>(newTs);
    }

    /**
     * Version of above that uses a copy constructor to initialize the n items
     * to separate T values.
     */
    T* push_back_n(int n, const T t[]) {
        SkASSERT(n >= 0);
        this->checkRealloc(n, kGrowing);
        for (int i = 0; i < n; ++i) {
            new (fItemArray + fCount + i) T(t[i]);
        }
        fCount += n;
        return fItemArray + fCount - n;
    }

    /**
     * Version of above that uses the move constructor to set n items.
     */
    T* move_back_n(int n, T* t) {
        SkASSERT(n >= 0);
        this->checkRealloc(n, kGrowing);
        for (int i = 0; i < n; ++i) {
            new (fItemArray + fCount + i) T(std::move(t[i]));
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
    }

    /**
     * Removes the last n elements. Not safe to call when count() < n.
     */
    void pop_back_n(int n) {
        SkASSERT(n >= 0);
        SkASSERT(this->count() >= n);
        fCount -= n;
        for (int i = 0; i < n; ++i) {
            fItemArray[fCount + i].~T();
        }
    }

    /**
     * Pushes or pops from the back to resize. Pushes will be default
     * initialized.
     */
    void resize_back(int newCount) {
        SkASSERT(newCount >= 0);

        if (newCount > this->count()) {
            this->push_back_n(newCount - fCount);
        } else if (newCount < this->count()) {
            this->pop_back_n(fCount - newCount);
        }
    }

    /** Swaps the contents of this array with that array. Does a pointer swap if possible,
        otherwise copies the T values. */
    void swap(SkTArray& that) {
        using std::swap;
        if (this == &that) {
            return;
        }
        if (fOwnMemory && that.fOwnMemory) {
            swap(fItemArray, that.fItemArray);

            auto count = fCount;
            fCount = that.fCount;
            that.fCount = count;

            auto allocCount = fAllocCount;
            fAllocCount = that.fAllocCount;
            that.fAllocCount = allocCount;
        } else {
            // This could be more optimal...
            SkTArray copy(std::move(that));
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

    // It's safe to use fItemArray + fCount because if fItemArray is nullptr then adding 0 is
    // valid and returns nullptr. See [expr.add] in the C++ standard.
    T* end() {
        if (fItemArray == nullptr) {
            SkASSERT(fCount == 0);
        }
        return fItemArray + fCount;
    }
    const T* end() const {
        if (fItemArray == nullptr) {
            SkASSERT(fCount == 0);
        }
        return fItemArray + fCount;
    }
    T* data() { return fItemArray; }
    const T* data() const { return fItemArray; }
    int size() const { return fCount; }
    size_t size_bytes() const { return this->bytes(fCount); }
    void resize(size_t count) { this->resize_back((int)count); }

    void clear() { this->pop_back_n(fCount); }

    void shrink_to_fit() {
        if (!fOwnMemory || fCount == fAllocCount) {
            return;
        }
        if (fCount == 0) {
            sk_free(fItemArray);
            fItemArray = nullptr;
            fAllocCount = 0;
        } else {
            SkSpan<std::byte> allocation = Allocate(fCount);
            this->move(TCast(allocation.data()));
            if (fOwnMemory) {
                sk_free(fItemArray);
            }
            this->setItemArray(allocation);
        }
    }

    /**
     * Get the i^th element.
     */
    T& operator[] (int i) {
        SkASSERT(i < this->count());
        SkASSERT(i >= 0);
        return fItemArray[i];
    }

    const T& operator[] (int i) const {
        SkASSERT(i < this->count());
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

    /**
     * equivalent to operator[](count()-1-i)
     */
    T& fromBack(int i) {
        SkASSERT(i >= 0);
        SkASSERT(i < this->count());
        return fItemArray[fCount - i - 1];
    }

    const T& fromBack(int i) const {
        SkASSERT(i >= 0);
        SkASSERT(i < this->count());
        return fItemArray[fCount - i - 1];
    }

    bool operator==(const SkTArray<T, MEM_MOVE>& right) const {
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

    bool operator!=(const SkTArray<T, MEM_MOVE>& right) const {
        return !(*this == right);
    }

    int capacity() const {
        return fAllocCount;
    }

protected:
    /**
     * Creates an empty array that will use the passed storage block until it
     * is insufficiently large to hold the entire array.
     */
    template <int N>
    SkTArray(SkAlignedSTStorage<N,T>* storage) {
        this->initWithPreallocatedStorage(0, storage->get(), N);
    }

    /**
     * Copy a C array, using preallocated storage if preAllocCount >=
     * count. Otherwise storage will only be used when array shrinks
     * to fit.
     */
    template <int N>
    SkTArray(const T* array, int count, SkAlignedSTStorage<N,T>* storage) {
        this->initWithPreallocatedStorage(count, storage->get(), N);
        this->copy(array);
    }

private:
    static constexpr int kMinHeapAllocCount = 8;
    static_assert(SkIsPow2(kMinHeapAllocCount), "min alloc count not power of two.");
    // Note for 32-bit machines kMaxCapacity will be <= SIZE_MAX. For 64-bit machines it will
    // just be INT_MAX if the sizeof(T) < 2^32.
    static constexpr uint32_t kMaxCapacity = std::min(SIZE_MAX / sizeof(T), (size_t)INT_MAX);

    void setItemArray(SkSpan<std::byte> allocation) {
        fItemArray = TCast(allocation.data());
        // We have gotten extra bytes back from the allocation limit, pin to kMaxCapacity. It
        // would seem like the SkContainerAllocator should handle the divide, but it would have
        // to a full divide instruction. If done here the size is known at compile, and usually
        // can be implemented by a right shift. The full divide takes ~50X longer than the shift.
        fAllocCount =
                SkToU32(std::min(allocation.size() / sizeof(T), SkToSizeT(kMaxCapacity)));
        fOwnMemory = true;
    }

    // We disable Control-Flow Integrity sanitization (go/cfi) when casting item-array buffers.
    // CFI flags this code as dangerous because we are casting `buffer` to a T* while the buffer's
    // contents might still be uninitialized memory. When T has a vtable, this is especially risky
    // because we could hypothetically access a virtual method on fItemArray and jump to an
    // unpredictable location in memory. Of course, SkTArray won't actually use fItemArray in this
    // way, and we don't want to construct a T before the user requests one. There's no real risk
    // here, so disable CFI when doing these casts.
    SK_ATTRIBUTE(no_sanitize("cfi"))
    static T* TCast(void* buffer) {
        return (T*)buffer;
    }

    size_t bytes(int n) const {
        SkASSERT(n <= SkToInt(kMaxCapacity));
        return SkToSizeT(n) * sizeof(T);
    }

    static SkSpan<std::byte> Allocate(int capacity, double growthFactor = 1.0) {
        return SkContainerAllocator{sizeof(T), kMaxCapacity}.allocate(capacity, growthFactor);
    }

    void init(int count) {
        fCount = count;
        if (!count) {
            fAllocCount = 0;
            fItemArray = nullptr;
        } else {
            fAllocCount = SkToU32(std::max(count, kMinHeapAllocCount));
            this->setItemArray(Allocate(fAllocCount));
        }
        fOwnMemory = true;
    }

    void initWithPreallocatedStorage(int count, void* preallocStorage, int preallocCount) {
        SkASSERT(count >= 0);
        SkASSERT(preallocCount > 0);
        SkASSERT(preallocStorage);
        fCount = count;
        fItemArray = nullptr;
        if (count > preallocCount) {
            fAllocCount = SkToU32(std::max(count, kMinHeapAllocCount));
            SkSpan<std::byte> allocation = Allocate(fAllocCount);
            this->setItemArray(allocation);
        } else {
            fAllocCount = SkToU32(preallocCount);
            fItemArray = TCast(preallocStorage);
            fOwnMemory = false;
        }
    }

    /** In the following move and copy methods, 'dst' is assumed to be uninitialized raw storage.
     *  In the following move methods, 'src' is destroyed leaving behind uninitialized raw storage.
     */
    void copy(const T* src) {
        if constexpr (std::is_trivially_copyable_v<T>) {
            if (!this->empty() && src != nullptr) {
                sk_careful_memcpy(fItemArray, src, this->size_bytes());
            }
        } else {
            for (int i = 0; i < this->count(); ++i) {
                new (fItemArray + i) T(src[i]);
            }
        }
    }

    void move(int dst, int src) {
        if constexpr (MEM_MOVE) {
            memcpy(static_cast<void*>(&fItemArray[dst]),
                   static_cast<void*>(&fItemArray[src]),
                   sizeof(T));
        } else {
            new (&fItemArray[dst]) T(std::move(fItemArray[src]));
            fItemArray[src].~T();
        }
    }

    void move(void* dst) {
        if constexpr (MEM_MOVE) {
            sk_careful_memcpy(dst, fItemArray, this->bytes(fCount));
        } else {
            for (int i = 0; i < this->count(); ++i) {
                new (static_cast<char*>(dst) + this->bytes(i)) T(std::move(fItemArray[i]));
                fItemArray[i].~T();
            }
        }
    }

    // Helper function that makes space for n objects, adjusts the count, but does not initialize
    // the new objects.
    void* push_back_raw(int n) {
        this->checkRealloc(n, kGrowing);
        void* ptr = fItemArray + fCount;
        fCount += n;
        return ptr;
    }

    void checkRealloc(int delta, ReallocType reallocType) {
        // This constant needs to be declared in the function where it is used to work around
        // MSVC's persnickety nature about template definitions.
        SkASSERT(delta >= 0);
        SkASSERT(fCount >= 0);
        SkASSERT(fAllocCount >= 0);

        // Return if there are enough remaining allocated elements to satisfy the request.
        if (SkToInt(fAllocCount) - fCount >= delta) { return; }

        // Don't overflow fCount or size_t later in the memory allocation. Overflowing memory
        // allocation really only applies to fCounts on 32-bit machines; on 64-bit machines this
        // will probably never produce a check. Since kMaxCapacity is bounded above by INT_MAX,
        // this also checks the bounds of fCount.
        SkASSERT_RELEASE(delta <= SkToInt(kMaxCapacity) - fCount);
        const int newCount = fCount + delta;

        double growthFactor = reallocType == kExactFit ? 1.0 : 1.5;
        SkSpan<std::byte> allocation = Allocate(newCount, growthFactor);

        this->move(TCast(allocation.data()));
        if (fOwnMemory) {
            sk_free(fItemArray);
        }
        this->setItemArray(allocation);
        SkASSERT(SkToInt(fAllocCount) >= newCount);
        SkASSERT(fItemArray != nullptr);
    }

    T* fItemArray;
    int fCount;
    uint32_t fAllocCount : 31;
    uint32_t fOwnMemory  :  1;
};

template <typename T, bool M> static inline void swap(SkTArray<T, M>& a, SkTArray<T, M>& b) {
    a.swap(b);
}

template<typename T, bool MEM_MOVE> constexpr int SkTArray<T, MEM_MOVE>::kMinHeapAllocCount;

/**
 * Subclass of SkTArray that contains a preallocated memory block for the array.
 */
template <int N, typename T, bool MEM_MOVE = sk_is_trivially_relocatable_v<T>>
class SkSTArray : private SkAlignedSTStorage<N,T>, public SkTArray<T, MEM_MOVE> {
private:
    using STORAGE   = SkAlignedSTStorage<N,T>;
    using INHERITED = SkTArray<T, MEM_MOVE>;

public:
    SkSTArray()
        : STORAGE{}, INHERITED(static_cast<STORAGE*>(this)) {}

    SkSTArray(const T* array, int count)
        : STORAGE{}, INHERITED(array, count, static_cast<STORAGE*>(this)) {}

    SkSTArray(std::initializer_list<T> data)
        : SkSTArray(data.begin(), SkToInt(data.size())) {}

    explicit SkSTArray(int reserveCount)
        : SkSTArray() {
        this->reserve_back(reserveCount);
    }

    SkSTArray         (const SkSTArray&  that) : SkSTArray() { *this = that; }
    explicit SkSTArray(const INHERITED&  that) : SkSTArray() { *this = that; }
    SkSTArray         (      SkSTArray&& that) : SkSTArray() { *this = std::move(that); }
    explicit SkSTArray(      INHERITED&& that) : SkSTArray() { *this = std::move(that); }

    SkSTArray& operator=(const SkSTArray& that) {
        INHERITED::operator=(that);
        return *this;
    }
    SkSTArray& operator=(const INHERITED& that) {
        INHERITED::operator=(that);
        return *this;
    }

    SkSTArray& operator=(SkSTArray&& that) {
        INHERITED::operator=(std::move(that));
        return *this;
    }
    SkSTArray& operator=(INHERITED&& that) {
        INHERITED::operator=(std::move(that));
        return *this;
    }
};

#endif
