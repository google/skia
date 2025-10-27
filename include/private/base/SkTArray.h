/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTArray_DEFINED
#define SkTArray_DEFINED

#include "include/private/base/SkASAN.h"  // IWYU pragma: keep
#include "include/private/base/SkAlignedStorage.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkAttributes.h"
#include "include/private/base/SkContainers.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkMalloc.h"
#include "include/private/base/SkMath.h"
#include "include/private/base/SkSpan_impl.h"
#include "include/private/base/SkTo.h"
#include "include/private/base/SkTypeTraits.h"  // IWYU pragma: keep

#include <algorithm>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <new>
#include <utility>

namespace skia_private {
/** TArray<T> implements a typical, mostly std::vector-like array.
    Each T will be default-initialized on allocation, and ~T will be called on destruction.

    MEM_MOVE controls the behavior when a T needs to be moved (e.g. when the array is resized)
      - true: T will be bit-copied via memcpy.
      - false: T will be moved via move-constructors.
*/
template <typename T, bool MEM_MOVE = sk_is_trivially_relocatable_v<T>> class TArray {
public:
    using value_type = T;

    /**
     * Creates an empty array with no initial storage
     */
    TArray() : fOwnMemory(true), fCapacity{0} {}

    /**
     * Creates an empty array that will preallocate space for reserveCount elements.
     */
    explicit TArray(int reserveCount) : TArray() { this->reserve_exact(reserveCount); }

    /**
     * Copies one array to another. The new array will be heap allocated.
     */
    TArray(const TArray& that) : TArray(that.fData, that.fSize) {}

    TArray(TArray&& that) {
        if (that.fOwnMemory) {
            this->setData(that);
            that.setData({});
        } else {
            this->initData(that.fSize);
            that.move(fData);
        }
        this->changeSize(that.fSize);
        that.changeSize(0);
    }

    /**
     * Creates a TArray by copying contents of a standard C array. The new
     * array will be heap allocated. Be careful not to use this constructor
     * when you really want the (void*, int) version.
     */
    TArray(const T* array, int count) {
        this->initData(count);
        this->copy(array);
    }

    /**
     * Creates a TArray by copying contents from an SkSpan. The new array will be heap allocated.
     */
    TArray(SkSpan<const T> data) : TArray(data.begin(), static_cast<int>(data.size())) {}

    /**
     * Creates a TArray by copying contents of an initializer list.
     */
    TArray(std::initializer_list<T> data) : TArray(data.begin(), data.size()) {}

    TArray& operator=(const TArray& that) {
        if (this == &that) {
            return *this;
        }
        this->clear();
        this->checkRealloc(that.size(), kExactFit);
        this->changeSize(that.fSize);
        this->copy(that.fData);
        return *this;
    }

    TArray& operator=(TArray&& that) {
        if (this != &that) {
            this->clear();
            this->unpoison();
            that.unpoison();
            if (that.fOwnMemory) {
                // The storage is on the heap, so move the data pointer.
                if (fOwnMemory) {
                    sk_free(fData);
                }

                fData = std::exchange(that.fData, nullptr);

                // Can't use exchange with bitfields.
                fCapacity = that.fCapacity;
                that.fCapacity = 0;

                fOwnMemory = true;

                this->changeSize(that.fSize);
            } else {
                // The data is stored inline in that, so move it element-by-element.
                this->checkRealloc(that.size(), kExactFit);
                this->changeSize(that.fSize);
                that.move(fData);
            }
            that.changeSize(0);
        }
        return *this;
    }

    ~TArray() {
        this->destroyAll();
        this->unpoison();
        if (fOwnMemory) {
            sk_free(fData);
        }
    }

    /**
     * Resets to size() = n newly constructed T objects and resets any reserve count.
     */
    void reset(int n) {
        SkASSERT(n >= 0);
        this->clear();
        this->checkRealloc(n, kExactFit);
        this->changeSize(n);
        for (int i = 0; i < this->size(); ++i) {
            new (fData + i) T;
        }
    }

    /**
     * Resets to a copy of a C array and resets any reserve count.
     */
    void reset(SkSpan<const T> src) {
        this->clear();
        this->checkRealloc(src.size(), kExactFit);
        this->changeSize(src.size());
        this->copy(src.data());
    }

    /**
     * Ensures there is enough reserved space for at least n elements. This is guaranteed at least
     * until the array size grows above n and subsequently shrinks below n, any version of reset()
     * is called, or reserve() is called again.
     */
    void reserve(int n) {
        SkASSERT(n >= 0);
        if (n > this->size()) {
            this->checkRealloc(n - this->size(), kGrowing);
        }
    }

    /**
     * Ensures there is enough reserved space for exactly n elements. The same capacity guarantees
     * as above apply.
     */
    void reserve_exact(int n) {
        SkASSERT(n >= 0);
        if (n > this->size()) {
            this->checkRealloc(n - this->size(), kExactFit);
        }
    }

    void removeShuffle(int n) {
        SkASSERT(n < this->size());
        int newCount = fSize - 1;
        fData[n].~T();
        if (n != newCount) {
            this->move(n, newCount);
        }
        this->changeSize(newCount);
    }

    // Is the array empty.
    bool empty() const { return fSize == 0; }

    /**
     * Adds one new default-initialized T value and returns it by reference. Note that the reference
     * only remains valid until the next call that adds or removes elements.
     */
    T& push_back() {
        void* newT = this->push_back_raw(1);
        return *new (newT) T;
    }

    /**
     * Adds one new T value which is copy-constructed, returning it by reference. As always,
     * the reference only remains valid until the next call that adds or removes elements.
     */
    T& push_back(const T& t) {
        this->unpoison();
        T* newT;
        if (this->capacity() > fSize) SK_LIKELY {
            // Copy over the element directly.
            newT = new (fData + fSize) T(t);
        } else {
            newT = this->growAndConstructAtEnd(t);
        }

        this->changeSize(fSize + 1);
        return *newT;
    }

    /**
     * Adds one new T value which is copy-constructed, returning it by reference.
     */
    T& push_back(T&& t) {
        this->unpoison();
        T* newT;
        if (this->capacity() > fSize) SK_LIKELY {
            // Move over the element directly.
            newT = new (fData + fSize) T(std::move(t));
        } else {
            newT = this->growAndConstructAtEnd(std::move(t));
        }

        this->changeSize(fSize + 1);
        return *newT;
    }

    /**
     *  Constructs a new T at the back of this array, returning it by reference.
     */
    template <typename... Args> T& emplace_back(Args&&... args) {
        this->unpoison();
        T* newT;
        if (this->capacity() > fSize) SK_LIKELY {
            // Emplace the new element in directly.
            newT = new (fData + fSize) T(std::forward<Args>(args)...);
        } else {
            newT = this->growAndConstructAtEnd(std::forward<Args>(args)...);
        }

        this->changeSize(fSize + 1);
        return *newT;
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
        T* end = this->end();
        this->changeSize(fSize + n);
        for (int i = 0; i < n; ++i) {
            new (end + i) T(t[i]);
        }
        return end;
    }

    /**
     * Version of above that uses the move constructor to set n items.
     */
    T* move_back_n(int n, T* t) {
        SkASSERT(n >= 0);
        this->checkRealloc(n, kGrowing);
        T* end = this->end();
        this->changeSize(fSize + n);
        for (int i = 0; i < n; ++i) {
            new (end + i) T(std::move(t[i]));
        }
        return end;
    }

    /**
     * Removes the last element. Not safe to call when size() == 0.
     */
    void pop_back() {
        sk_collection_not_empty(this->empty());
        fData[fSize - 1].~T();
        this->changeSize(fSize - 1);
    }

    /**
     * Removes the last n elements. Not safe to call when size() < n.
     */
    void pop_back_n(int n) {
        SkASSERT(n >= 0);
        SkASSERT(this->size() >= n);
        int i = fSize;
        while (i-- > fSize - n) {
            (*this)[i].~T();
        }
        this->changeSize(fSize - n);
    }

    /**
     * Pushes or pops from the back to resize. Pushes will be default initialized.
     */
    void resize_back(int newCount) {
        SkASSERT(newCount >= 0);
        if (newCount > this->size()) {
            if (this->empty()) {
                // When the container is completely empty, grow to exactly the requested size.
                this->checkRealloc(newCount, kExactFit);
            }
            this->push_back_n(newCount - fSize);
        } else if (newCount < this->size()) {
            this->pop_back_n(fSize - newCount);
        }
    }

    /** Swaps the contents of this array with that array. Does a pointer swap if possible,
        otherwise copies the T values. */
    void swap(TArray& that) {
        using std::swap;
        if (this == &that) {
            return;
        }
        if (fOwnMemory && that.fOwnMemory) {
            swap(fData, that.fData);
            swap(fSize, that.fSize);

            // Can't use swap because fCapacity is a bit field.
            auto allocCount = fCapacity;
            fCapacity = that.fCapacity;
            that.fCapacity = allocCount;
        } else {
            // This could be more optimal...
            TArray copy(std::move(that));
            that = std::move(*this);
            *this = std::move(copy);
        }
    }

    /**
     * Moves all elements of `that` to the end of this array, leaving `that` empty.
     * This is a no-op if `that` is empty or equal to this array.
     */
    void move_back(TArray& that) {
        if (that.empty() || &that == this) {
            return;
        }
        void* dst = this->push_back_raw(that.size());
        // After move() returns, the contents of `dst` will have either been in-place initialized
        // using a the move constructor (per-item from `that`'s elements), or will have been
        // mem-copied into when MEM_MOVE is true (now valid objects).
        that.move(dst);
        // All items in `that` have either been destroyed (when MEM_MOVE is false) or should be
        // considered invalid (when MEM_MOVE is true). Reset fSize to 0 directly to skip any further
        // per-item destruction.
        that.changeSize(0);
    }

    T* begin() {
        return fData;
    }
    const T* begin() const {
        return fData;
    }

    // It's safe to use fItemArray + fSize because if fItemArray is nullptr then adding 0 is
    // valid and returns nullptr. See [expr.add] in the C++ standard.
    T* end() {
        if (fData == nullptr) {
            SkASSERT(fSize == 0);
        }
        return fData + fSize;
    }
    const T* end() const {
        if (fData == nullptr) {
            SkASSERT(fSize == 0);
        }
        return fData + fSize;
    }
    T* data() { return fData; }
    const T* data() const { return fData; }
    int size() const { return fSize; }
    size_t size_bytes() const { return Bytes(fSize); }
    void resize(size_t count) { this->resize_back((int)count); }

    void clear() {
        this->destroyAll();
        this->changeSize(0);
    }

    void shrink_to_fit() {
        if (!fOwnMemory || fSize == fCapacity) {
            return;
        }
        this->unpoison();
        if (fSize == 0) {
            sk_free(fData);
            fData = nullptr;
            fCapacity = 0;
        } else {
            SkSpan<std::byte> allocation = Allocate(fSize);
            this->move(TCast(allocation.data()));
            if (fOwnMemory) {
                sk_free(fData);
            }
            // Poison is applied in `setDataFromBytes`.
            this->setDataFromBytes(allocation);
        }
    }

    /**
     * Get the i^th element.
     */
    T& operator[] (int i) {
        return fData[sk_collection_check_bounds(i, this->size())];
    }

    const T& operator[] (int i) const {
        return fData[sk_collection_check_bounds(i, this->size())];
    }

    T& at(int i) { return (*this)[i]; }
    const T& at(int i) const { return (*this)[i]; }

    /**
     * equivalent to operator[](0)
     */
    T& front() {
        sk_collection_not_empty(this->empty());
        return fData[0];
    }

    const T& front() const {
        sk_collection_not_empty(this->empty());
        return fData[0];
    }

    /**
     * equivalent to operator[](size() - 1)
     */
    T& back() {
        sk_collection_not_empty(this->empty());
        return fData[fSize - 1];
    }

    const T& back() const {
        sk_collection_not_empty(this->empty());
        return fData[fSize - 1];
    }

    /**
     * equivalent to operator[](size()-1-i)
     */
    T& fromBack(int i) {
        return (*this)[fSize - i - 1];
    }

    const T& fromBack(int i) const {
        return (*this)[fSize - i - 1];
    }

    bool operator==(const TArray<T, MEM_MOVE>& right) const {
        int leftCount = this->size();
        if (leftCount != right.size()) {
            return false;
        }
        for (int index = 0; index < leftCount; ++index) {
            if (fData[index] != right.fData[index]) {
                return false;
            }
        }
        return true;
    }

    bool operator!=(const TArray<T, MEM_MOVE>& right) const {
        return !(*this == right);
    }

    int capacity() const {
        return fCapacity;
    }

protected:
    // Creates an empty array that will use the passed storage block until it is insufficiently
    // large to hold the entire array.
    template <int InitialCapacity>
    TArray(SkAlignedSTStorage<InitialCapacity, T>* storage, int size = 0) {
        static_assert(InitialCapacity >= 0);
        SkASSERT(size >= 0);
        SkASSERT(storage->get() != nullptr);
        if (size > InitialCapacity) {
            this->initData(size);
        } else {
            this->setDataFromBytes(*storage);
            this->changeSize(size);

            // setDataFromBytes always sets fOwnMemory to true, but we are actually using static
            // storage here, which shouldn't ever be freed.
            fOwnMemory = false;
        }
    }

    // Copy a C array, using pre-allocated storage if preAllocCount >= count. Otherwise, storage
    // will only be used when array shrinks to fit.
    template <int InitialCapacity>
    TArray(const T* array, int size, SkAlignedSTStorage<InitialCapacity, T>* storage)
            : TArray{storage, size} {
        this->copy(array);
    }
    template <int InitialCapacity>
    TArray(SkSpan<const T> data, SkAlignedSTStorage<InitialCapacity, T>* storage)
            : TArray{storage, static_cast<int>(data.size())} {
        this->copy(data.begin());
    }

private:
    // Growth factors for checkRealloc.
    static constexpr double kExactFit = 1.0;
    static constexpr double kGrowing = 1.5;

    static constexpr int kMinHeapAllocCount = 8;
    static_assert(SkIsPow2(kMinHeapAllocCount), "min alloc count not power of two.");

    // Note for 32-bit machines kMaxCapacity will be <= SIZE_MAX. For 64-bit machines it will
    // just be INT_MAX if the sizeof(T) < 2^32.
    static constexpr int kMaxCapacity = SkToInt(std::min(SIZE_MAX / sizeof(T), (size_t)INT_MAX));

    void setDataFromBytes(SkSpan<std::byte> allocation) {
        T* data = TCast(allocation.data());
        // We have gotten extra bytes back from the allocation limit, pin to kMaxCapacity. It
        // would seem like the SkContainerAllocator should handle the divide, but it would have
        // to a full divide instruction. If done here the size is known at compile, and usually
        // can be implemented by a right shift. The full divide takes ~50X longer than the shift.
        size_t size = std::min(allocation.size() / sizeof(T), SkToSizeT(kMaxCapacity));
        this->setData(SkSpan<T>(data, size));
    }

    void setData(SkSpan<T> array) {
        this->unpoison();

        fData = array.data();
        fCapacity = SkToU32(array.size());
        fOwnMemory = true;

        this->poison();
    }

    void unpoison() {
#ifdef SK_SANITIZE_ADDRESS
        if (fData && fPoisoned) {
            // SkDebugf("UNPOISONING %p : 0 -> %zu\n", fData, Bytes(fCapacity));
            sk_asan_unpoison_memory_region(this->begin(), Bytes(fCapacity));
            fPoisoned = false;
        }
#endif
    }

    void poison() {
#ifdef SK_SANITIZE_ADDRESS
        if (fData && fCapacity > fSize) {
            // SkDebugf("  POISONING %p : %zu -> %zu\n", fData, Bytes(fSize), Bytes(fCapacity));
            sk_asan_poison_memory_region(this->end(), Bytes(fCapacity - fSize));
            fPoisoned = true;
        }
#endif
    }

    void changeSize(int n) {
        this->unpoison();
        fSize = n;
        this->poison();
    }

    // We disable Control-Flow Integrity sanitization (go/cfi) when casting item-array buffers.
    // CFI flags this code as dangerous because we are casting `buffer` to a T* while the buffer's
    // contents might still be uninitialized memory. When T has a vtable, this is especially risky
    // because we could hypothetically access a virtual method on fItemArray and jump to an
    // unpredictable location in memory. Of course, TArray won't actually use fItemArray in this
    // way, and we don't want to construct a T before the user requests one. There's no real risk
    // here, so disable CFI when doing these casts.
    SK_NO_SANITIZE_CFI
    static T* TCast(void* buffer) {
        return (T*)buffer;
    }

    static size_t Bytes(int n) {
        SkASSERT(n <= kMaxCapacity);
        return SkToSizeT(n) * sizeof(T);
    }

    static SkSpan<std::byte> Allocate(int capacity, double growthFactor = 1.0) {
        return SkContainerAllocator{sizeof(T), kMaxCapacity}.allocate(capacity, growthFactor);
    }

    void initData(int count) {
        this->setDataFromBytes(Allocate(count));
        this->changeSize(count);
    }

    void destroyAll() {
        if (!this->empty()) {
            T* cursor = this->begin();
            T* const end = this->end();
            do {
                cursor->~T();
                cursor++;
            } while (cursor < end);
        }
    }

    /** In the following move and copy methods, 'dst' is assumed to be uninitialized raw storage.
     *  In the following move methods, 'src' is destroyed leaving behind uninitialized raw storage.
     */
    void copy(const T* src) {
        if constexpr (std::is_trivially_copyable_v<T>) {
            if (!this->empty() && src != nullptr) {
                sk_careful_memcpy(fData, src, this->size_bytes());
            }
        } else {
            for (int i = 0; i < this->size(); ++i) {
                new (fData + i) T(src[i]);
            }
        }
    }

    void move(int dst, int src) {
        if constexpr (MEM_MOVE) {
            memcpy(static_cast<void*>(&fData[dst]),
                   static_cast<const void*>(&fData[src]),
                   sizeof(T));
        } else {
            new (&fData[dst]) T(std::move(fData[src]));
            fData[src].~T();
        }
    }

    void move(void* dst) {
        if constexpr (MEM_MOVE) {
            sk_careful_memcpy(dst, fData, Bytes(fSize));
        } else {
            for (int i = 0; i < this->size(); ++i) {
                new (static_cast<char*>(dst) + Bytes(i)) T(std::move(fData[i]));
                fData[i].~T();
            }
        }
    }

    // Helper function that makes space for n objects, adjusts the count, but does not initialize
    // the new objects.
    void* push_back_raw(int n) {
        this->checkRealloc(n, kGrowing);
        void* ptr = fData + fSize;
        this->changeSize(fSize + n);
        return ptr;
    }

    template <typename... Args>
    SK_ALWAYS_INLINE T* growAndConstructAtEnd(Args&&... args) {
        SkSpan<std::byte> buffer = this->preallocateNewData(/*delta=*/1, kGrowing);
        T* newT = new (TCast(buffer.data()) + fSize) T(std::forward<Args>(args)...);
        this->installDataAndUpdateCapacity(buffer);

        return newT;
    }

    void checkRealloc(int delta, double growthFactor) {
        SkASSERT(delta >= 0);
        SkASSERT(fSize >= 0);
        SkASSERT(fCapacity >= 0);

        // Check if there are enough remaining allocated elements to satisfy the request.
        if (this->capacity() - fSize < delta) {
            // Looks like we need to reallocate.
            this->installDataAndUpdateCapacity(this->preallocateNewData(delta, growthFactor));
        }
    }

    SkSpan<std::byte> preallocateNewData(int delta, double growthFactor) {
        SkASSERT(delta >= 0);
        SkASSERT(fSize >= 0);
        SkASSERT(fCapacity >= 0);

        // Don't overflow fSize or size_t later in the memory allocation. Overflowing memory
        // allocation really only applies to fSizes on 32-bit machines; on 64-bit machines this
        // will probably never produce a check. Since kMaxCapacity is bounded above by INT_MAX,
        // this also checks the bounds of fSize.
        if (delta > kMaxCapacity - fSize) {
            sk_report_container_overflow_and_die();
        }
        const int newCount = fSize + delta;

        return Allocate(newCount, growthFactor);
    }

    void installDataAndUpdateCapacity(SkSpan<std::byte> allocation) {
        this->move(TCast(allocation.data()));
        if (fOwnMemory) {
            sk_free(fData);
        }
        this->setDataFromBytes(allocation);
        SkASSERT(fData != nullptr);
    }

    T* fData{nullptr};
    int fSize{0};
    uint32_t fOwnMemory : 1;
    uint32_t fCapacity : 31;
#ifdef SK_SANITIZE_ADDRESS
    bool fPoisoned = false;
#endif
};

template <typename T, bool M> static inline void swap(TArray<T, M>& a, TArray<T, M>& b) {
    a.swap(b);
}

// Subclass of TArray that contains a pre-allocated memory block for the array.
template <int Nreq, typename T, bool MEM_MOVE = sk_is_trivially_relocatable_v<T>>
class STArray : private SkAlignedSTStorage<SkContainerAllocator::RoundUp<T>(Nreq), T>,
                public TArray<T, MEM_MOVE> {
    // We round up the requested array size to the next capacity multiple.
    // This space would likely otherwise go to waste.
    static constexpr int N = SkContainerAllocator::RoundUp<T>(Nreq);
    static_assert(Nreq > 0);
    static_assert(N >= Nreq);

    using Storage = SkAlignedSTStorage<N,T>;

public:
    STArray()
        : Storage{}
        , TArray<T, MEM_MOVE>(this) {}  // Must use () to avoid confusion with initializer_list
                                        // when T=bool because * are convertable to bool.

    STArray(const T* array, int count)
        : Storage{}
        , TArray<T, MEM_MOVE>{array, count, this} {}

    STArray(SkSpan<const T> data)
        : Storage{}
        , TArray<T, MEM_MOVE>{data, this} {}

    STArray(std::initializer_list<T> data)
        : STArray{data.begin(), SkToInt(data.size())} {}

    explicit STArray(int reserveCount)
        : STArray() { this->reserve_exact(reserveCount); }

    STArray(const STArray& that)
        : STArray() { *this = that; }

    explicit STArray(const TArray<T, MEM_MOVE>& that)
        : STArray() { *this = that; }

    STArray(STArray&& that)
        : STArray() { *this = std::move(that); }

    explicit STArray(TArray<T, MEM_MOVE>&& that)
        : STArray() { *this = std::move(that); }

    STArray& operator=(const STArray& that) {
        TArray<T, MEM_MOVE>::operator=(that);
        return *this;
    }

    STArray& operator=(const TArray<T, MEM_MOVE>& that) {
        TArray<T, MEM_MOVE>::operator=(that);
        return *this;
    }

    STArray& operator=(STArray&& that) {
        TArray<T, MEM_MOVE>::operator=(std::move(that));
        return *this;
    }

    STArray& operator=(TArray<T, MEM_MOVE>&& that) {
        TArray<T, MEM_MOVE>::operator=(std::move(that));
        return *this;
    }

    // Force the use of TArray for data() and size().
    using TArray<T, MEM_MOVE>::data;
    using TArray<T, MEM_MOVE>::size;
};
}  // namespace skia_private
#endif  // SkTArray_DEFINED
