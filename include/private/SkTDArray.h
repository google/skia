/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkTDArray_DEFINED
#define SkTDArray_DEFINED

#include "SkMalloc.h"
#include "SkTo.h"
#include "SkTypes.h"

#include <cstring>
#include <initializer_list>
#include <memory>
#include <utility>

class SkMemoryBlock {
public:
    SkMemoryBlock();

    void* growAndInsert(size_t elementSize, size_t maxCapacity,
                        size_t n, size_t insertion);

    void shrinkToFit(size_t elementSize, size_t n);

    void* data() { return fData; }

    // An array view onto the memory block with Tsize elements.
    template <size_t TSize>
    class SizedView;

    template <typename T>
    SizedView<sizeof(T)> view() {
        return SizedView<sizeof(T)>(this);
    }

private:
    // fSize and fCapacity are in element counts.
    size_t fSize{0};
    size_t fCapacity{0};
    void*  fData{nullptr};
};

template <size_t TSize>
class SkMemoryBlock::SizedView {
    explicit SizedView(SkMemoryBlock* block) : fBlock{block} {}

    // The maximum number of entries with space left over to point to one after the last one.
    static constexpr size_t MaxCapacity() {
        return std::numeric_limits<size_t>::max() / TSize - 1;
    }

    // Use alignment of 1 because all alignment is handled by malloc.
    using Indexer = typename std::aligned_storage<TSize, 1>::type*;

    Indexer index() { return reinterpret_cast<Indexer>(fBlock->fData); }

    void* ptr(size_t i) { return reinterpret_cast<void*>(&this->index()[i]); }

    void* growAndInsert(size_t n, size_t insertAt) {
        return fBlock->growAndInsert(TSize, MaxCapacity(), n, insertAt);
    }

    void* append(size_t n) {
        if (n <= fBlock->fCapacity - fBlock->fSize) {
            void* result = this->ptr(fBlock->fSize);
            fBlock->fSize += n;
            return result;
        }

        return this->growAndInsert(n, fBlock->fSize);
    }

    void* insert(size_t n, size_t insertAt) {
        return this->growAndInsert(n, insertAt);
    }

    static constexpr size_t ToBytes(size_t n) { return TSize * n; }

    void copy(void* dst, void* src, size_t n) {
        memmove(dst, src, ToBytes(n));
    }

private:
    SkMemoryBlock* const fBlock;
};

#if 0
// Maintains a block move memory of a given size, and maps indexes to address. It is not
// responsible for checking bounds.
class SkMemoryBlock {
public:
    SkMemoryBlock() : fBlock{nullptr}, fSize{0} { }
    explicit SkMemoryBlock(size_t reserveSize);
    // Make same size block, but only copy n bytes from src.
    SkMemoryBlock(const SkMemoryBlock& src, size_t n);
    SkMemoryBlock(SkMemoryBlock&& src);

    ~SkMemoryBlock();

    uint8_t* copyTo(size_t to, const void* from, size_t n);
    uint8_t* copyFrom(void* to, size_t from, size_t n);

    // Append n bytes and the end e.
    uint8_t* append(size_t e, size_t n) {
        SkASSERT(e <= fSize);
        SkASSERT(n > 0);

        // Check that there is room and no overflow.
        if (e + n <= fSize && e <= e + n) {
            return fBlock + e;
        }

        // appendHelper handles growing and overflow.
        return this->appendHelper(e, n);
    }

    // Insert n bytes at i moving the bytes [i, e).
    uint8_t* insert(size_t i, size_t e, size_t n);

    // Remove n bytes at r moving bytes [r + n, e).
    uint8_t* remove(size_t r, size_t e, size_t n);

    void* shrinkToFit(size_t n);
    void swap(SkMemoryBlock& that);

    void* ptrOf(size_t i) { return fBlock + i; }
    const void* ptrOf(size_t i) const { return fBlock + i; }

    bool contains(void* vPtr) const {
        auto ptr = (uint8_t*)vPtr;
        return (ptr != nullptr && fBlock <= ptr && ptr <= fBlock + fSize);
    }

    size_t indexOf(void *vPtr) const {
        SkASSERT(contains(vPtr));
        auto ptr = (uint8_t*)vPtr;
        return ptr - fBlock;
    }

    uint8_t* begin() const { return fBlock; }
    uint8_t* end()   const { return fBlock ? fBlock + fSize : nullptr; }
    void* data() { return fBlock; }

    bool empty() const { return fSize == 0; }
    size_t size() const { return fSize; }
    const void* data() const { return fBlock; }

private:
    uint8_t* appendHelper(size_t e, size_t n);
    uint8_t* moveTail(size_t from, size_t to, size_t n);

    uint8_t* fBlock{nullptr};
    size_t   fSize{0};
};

static inline void swap(SkMemoryBlock& a, SkMemoryBlock& b) {
    a.swap(b);
}

// -- SkMemoryBlockAs --------------------------------------------------------------------------
// Handles the count for things of a given size. This class makes sure that a wrapped index is
// never passed to SkMemoryBlock. This means that fSize is never larger than MaxSize();
template<size_t TSize>
class SkMemoryBlockAs {
public:
    SkMemoryBlockAs() : fMemoryBlock{} {}

    SkMemoryBlockAs(size_t n, const void* data)
            : fMemoryBlock{SS(n), data}, fSize{n} {}

    SkMemoryBlockAs(const SkMemoryBlockAs& src, size_t n)
            : fMemoryBlock{src.fMemoryBlock, n} {}

    SkMemoryBlockAs(SkMemoryBlockAs&& src)
            : fMemoryBlock{std::move(src.fMemoryBlock)}, fSize{src.fSize} {
        new(&src) SkMemoryBlockAs{};
    }

    void* append(size_t n) {
        if ()
        if (fCapacity - fSize < n) {
            fMemoryBlock.grow()
            if (MaxSize() - fCapacity < n) {
                SK_ABORT("Too large");
            }
            this->grow(0, fSize, fSize);
        }

        size_t result = fSize;
        fSize += n;
        return this->ptrOf(result);
    }

    void* insert(size_t insertAt, size_t n) {
        if (insertAt > fSize) {
            SK_ABORT("Insertion point is too large");
        }
        if (fCapacity - fSize < n) {
            if (MaxSize() - fCapacity) {
                SK_ABORT("Too large");
            }
            this->grow(0, insertAt, fSize);
        }
        fSize += n;
        return this->ptrOf(insertAt);
    }

    void* remove(size_t r, size_t n) {
        SkASSERT(r < fSize);
        if (fSize - r >= n) {
            size_t end = fSize;
            fSize -= n;
            return (void*) fMemoryBlock.remove(S(r), S(end), S(n));
        }
        SK_ABORT("removed too much");
        return nullptr;
    }

    void* ptrOf(size_t index) {
        SkASSERT(index <= fSize);
        return fMemoryBlock.ptrOf(S(index));
    }

    const void* ptrOf(size_t index) const {
        SkASSERT(index <= fSize);
        return fMemoryBlock.ptrOf(S(index));
    }

    void clear() { fSize = 0; }

    bool empty() const { return fSize == 0; }

    size_t size() const { return fSize; }

    size_t capacity() const { return fCapacity; }

    void resize(size_t n) {
        if (fCapacity < n) {
            if (MaxSize() < n) {
                SK_ABORT("resize is too big");
            }
            this->growTo(n, fSize, fSize);
        }

        fSize = n;
    }

    void* begin() { return this->ptrOf(0); }

    void* end() { return this->ptrOf(fSize); }

    void* data() { return fMemoryBlock.data(); }

    const void* data() const { return fMemoryBlock.data(); }

    const void* cbegin() const { return this->ptrOf(0); }

    const void* cend() const { return this->ptrOf(fSize); }

    void* copyTo(size_t to, const void* from, size_t n) {
        return (void*) fMemoryBlock.copyTo(S(to), from, S(n));
    }

    void* copyFrom(void* to, size_t from, size_t n) {
        return (void*) fMemoryBlock.copyFrom(to, S(from), S(n));
    }

    void shrinkToFit(size_t n) { fMemoryBlock.shrinkToFit(S(n)); }

    bool containsPtr(const void* ptr) const { return this->cbegin() <= ptr && ptr <= this->cend(); }

    size_t addressToIndex(const void* ptr) const {
        return (ptr - fData) / TSize;
    }

    void swap(SkMemoryBlockAs& that) {
        using std::swap;
        swap(fSize, that.fSize);
        swap(fCapacity, that.fCapacity);
        swap(fMemoryBlock, that.fMemoryBlock);
    }

private:

    static constexpr size_t MaxSize() {
        // The maximum number of entries with space left over to point to one after the last one.
        return std::numeric_limits<size_t>::max() / TSize - 1;
    }

    static constexpr size_t S(size_t s) { return s * TSize; }

    static size_t SS(size_t s) {
        if (s > MaxSize()) {
            SK_ABORT("Size out of range.");
        }
        return S(s);
    }

    SkMemoryBlock::Extent
    SkMemoryBlock fMemoryBlock;

};

template<typename T>
static inline void swap(
        SkMemoryBlockAs<sizeof(T)>& a, SkMemoryBlockAs<sizeof(T)>& b) {
    a.swap(b);
}
#endif

// -- SkTDArray ------------------------------------------------------------------------------------
template<typename T>
class SkTDArray {
public:
    SkTDArray() {}

    SkTDArray(const T src[], int count)
            : fBlock{SkTo<size_t>(count)} {
        SkASSERT(src || count == 0);

        if (count > 0) {
            fBlock.copyTo(0, src, count);
        }
    }

    SkTDArray(const std::initializer_list<T>& list)
            : SkTDArray(list.begin(), list.size()) {}

    SkTDArray(const SkTDArray<T>& src)
            : fBlock{src.fBlock} {}

    SkTDArray(SkTDArray<T>&& src)
            : fBlock{std::move(src.fBlock)} {}

    ~SkTDArray() = default;

    SkTDArray<T>& operator=(const SkTDArray<T>& src) {
        if (this != &src) {
            if (src.fBlock.size() > fBlock.capacity()) {
                SkTDArray<T> tmp(src.data(), src.fCount);
                this->swap(tmp);
            } else if (!src.empty()) {
                fBlock.clear();
                fBlock.copyTo(0, src.data(), src.fCount);
            }
        }
        return *this;
    }

    SkTDArray<T>& operator=(SkTDArray<T>&& src) {
        if (this != &src) {
            this->swap(src);
            src.reset();
        }
        return *this;
    }

    friend bool operator==(const SkTDArray<T>& a, const SkTDArray<T>& b) {
        return a.size() == b.size() &&
               (a.size() == 0 ||
                !memcmp(a.cbegin(), b.cbegin(), a.fCount * sizeof(T)));
    }

    friend bool operator!=(const SkTDArray<T>& a, const SkTDArray<T>& b) {
        return !(a == b);
    }

    void swap(SkTDArray<T>& that) {
        using std::swap;
        swap(fBlock, that.fBlock);
    }

    bool empty() const { return fBlock.empty(); }

    bool isEmpty() const { return this->empty(); }

    /**
     *  Return the number of elements in the array
     */
    int count() const { return this->size(); }

    size_t size() const { return fBlock.size(); }

    /**
     *  Return the total number of elements allocated.
     *  reserved() - count() gives you the number of elements you can add
     *  without causing an allocation.
     */
    int reserved() const { return SkTo<int>(fBlock.capacity()); }

    /**
     *  return the number of bytes in the array: count * sizeof(T)
     */
    // N.B. this is always safe because fBlock never lets count so large that it overflows size_t.
    size_t bytes() const { return fBlock.size() * sizeof(T); }

    T* begin() { return fBlock.begin(); }

    const T* begin() const { return this->cbegin(); }

    const T* cbegin() const { return fBlock.cbegin(); }

    T* end() { return !fBlock.empty() ? &fBlock.end() : nullptr; }

    const T* end() const { return this->cend(); }

    const T* cend() const { return !fBlock.empty() ? &fBlock.end() : nullptr; }

    T& operator[](int index) {
        return *fBlock.ptrOf(index);
    }

    const T& operator[](int index) const {
        return *fBlock.ptrOf(index);
    }

    T& getAt(int index) {
        return (*this)[index];
    }

    T* data() { return fBlock.data(); }

    const T* data() const { return fBlock.data(); }

    void reset() {
        this->~SkTDArray();
        new(this) SkTDArray{};
    }

    void rewind() {
        fBlock.clear();
    }

    /**
     *  Sets the number of elements in the array.
     *  If the array does not have space for count elements, it will increase
     *  the storage allocated to some amount greater than that required.
     *  It will never shrink the storage.
     */
    void setCount(int count) {
        SkASSERT(count >= 0);

        fBlock.resize(count);
    }

    void reserve(size_t reserve) {
        SkASSERT_RELEASE(SkTFitsIn<int>(reserve));
        SkASSERT(reserve >= 0);
        auto capacity = fBlock.capacity();
        if (reserve > capacity) {
            fBlock.append(fCount, reserve - capacity);
        }
    }

    void setReserve(int n) {
        this->reserve(SkTo<size_t>(n));
    }

    T* prepend() {
        auto view = fBlock.view();
        return view.insert(1, 0);
    }

    T* append() {
        return this->append(1, nullptr);
    }

    T* append(int count, const T* src = nullptr) {
        SkASSERT(0 <= count);
        auto view = fBlock.view<T>();
        T* result = view.append(count);
        if (src != nullptr) {
            view.copy(result, src, count);
        }
        return result;
    }

    T* insert(int index) {
        return this->insert(index, 1, nullptr);
    }

    T* insert(int index, int count, const T* src = nullptr) {
        SkASSERT(count);
        SkASSERT(index <= fCount);
        auto result = fBlock.insert(index, fCount, count);
        this->incCount(count);
        if (src) {
            fBlock.copyTo(index, src, count);
        }
        return result;
    }

    void remove(int index, int count = 1) {
        SkASSERT(index + count <= fCount);
        fBlock.remove(index, fCount, count);
        fCount = fCount - count;
    }

    void removeShuffle(int index) {
        SkASSERT(index < fCount);

        fCount--;
        // Only move if index was not the last element.
        if (index != fCount) {
            fBlock.copyTo(index, &fBlock[fCount], 1);
        }
    }

    /**
     * Returns true iff the array contains this element.
     */
    bool contains(const T& elem) const {
        return (this->find(elem) >= 0);
    }

    /**
     * Copies up to max elements into dst. The number of items copied is
     * capped by count - index. The actual number copied is returned.
     */
    int copyRange(T* dst, int index, int max) const {
        SkASSERT(max >= 0);
        SkASSERT(!max || dst);
        if (index >= fCount) {
            return 0;
        }
        int count = SkMin32(max, fCount - index);

        fBlock.copyFrom(dst, index, count);
        return count;
    }

    void copy(T* dst) const {
        this->copyRange(dst, 0, fCount);
    }

    // routines to treat the array like a stack
    void push_back(const T& v) { *this->append() = v; }

    T* push() { return this->append(); }

    const T& top() const { return (*this)[fCount - 1]; }

    T& top() { return (*this)[fCount - 1]; }

    void pop(T* elem) {
        SkASSERT(fCount > 0);
        if (elem) *elem = (*this)[fCount - 1];
        --fCount;
    }

    void pop() {
        SkASSERT(fCount > 0);
        --fCount;
    }

    int find(const T& elem) const {

        for (const T& t : *this) {
            if (t == elem) {
                return SkTo<int>(fBlock.addressToIndex(&t));
            }
        }
        return -1;
    }

    int rfind(const T& elem) const {
        const T* iter = this->end();
        const T* stop = this->begin();

        while (iter > stop) {
            if (*--iter == elem) {
                return SkToInt(iter - stop);
            }
        }
        return -1;
    }

    // deleteAll, freeAll, unrefAll, safeUnrefAll all assume the elemets for SkTDArray are pointers,
    // so T is a Foo*, and T* is a Foo**.
    void deleteAll() {

        for (T ptr : *this) {
            delete ptr;
        }
        this->reset();
    }

    void freeAll() {
        for (T ptr : *this) {
            sk_free(ptr);
        }
        this->reset();
    }

    void unrefAll() {
        for (T ptr : *this) {
            ptr->unref();
        }
        this->reset();
    }

    void safeUnrefAll() {
        for (T ptr : *this) {
            SkSafeUnref(ptr);
        }
        this->reset();
    }

    void shrinkToFit() {
        fBlock.shrinkToFit(fCount);
    }

private:
    int incCount(int delta) {
        SkASSERT(delta > 0);
        // We take care to avoid overflow here.
        // The sum of fCount and delta is at most 4294967294, which fits fine in uint32_t.
        uint32_t count = (uint32_t) fCount + (uint32_t) delta;
        SkASSERT_RELEASE(SkTFitsIn<int>(count));
        fCount = count;
        return fCount;
    }

    SkMemoryBlock fBlock;
};

template<typename T>
static inline void swap(SkTDArray<T>& a, SkTDArray<T>& b) {
    a.swap(b);
}

#endif
