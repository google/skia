/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkTDArray_DEFINED
#define SkTDArray_DEFINED

#include "SkMalloc.h"
#include "SkSafeMath.h"
#include "SkTo.h"
#include "SkTypes.h"

#include <cstring>
#include <initializer_list>
#include <memory>
#include <utility>

class SkMemoryBlock {
public:
    SkMemoryBlock() = default;
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

    void* addrOf(size_t i) { return fBlock + i; }
    const void* addrOf(size_t i) const { return fBlock + i; }

    uint8_t* begin() const { return fBlock; }
    uint8_t* end()   const { return fBlock ? fBlock + fSize : nullptr; }
    void* data() { return fBlock; }

    bool empty() const { return fSize == 0; }
    size_t size() const { return fSize; }
    const void* data() const { return fBlock; }


    void* shrinkToFit(size_t n);
    void swap(SkMemoryBlock& that);

private:
    uint8_t* appendHelper(size_t e, size_t n);
    uint8_t* moveTail(size_t from, size_t to, size_t n);

    uint8_t* fBlock{nullptr};
    size_t   fSize{0};
};

static inline void swap(SkMemoryBlock& a, SkMemoryBlock& b) {
    a.swap(b);
}

// size_t newSize = SkSafeMath::Mul(growthCount, Tsize);

template <typename T>
class SkMemoryBlockAs {
public:
    SkMemoryBlockAs() : fMemoryBlock{} {}
    explicit SkMemoryBlockAs(size_t n) : fMemoryBlock{SS(n)} {}
    SkMemoryBlockAs(const SkMemoryBlockAs& src, size_t n)
        : fMemoryBlock{src.fMemoryBlock, n} {}
    SkMemoryBlockAs(SkMemoryBlockAs&& src)
        : fMemoryBlock{std::move(src.fMemoryBlock)} { }
    T* append(size_t e, size_t n) { return (T*)fMemoryBlock.append(S(e), SS(n)); }
    T* insert(size_t i, size_t e, size_t n) { return (T*)fMemoryBlock.insert(S(i), S(e), SS(n)); }
    T* remove(size_t r, size_t e, size_t n) { return (T*)fMemoryBlock.remove(S(r), S(e), S(n)); }
    T& operator[](size_t index) { return *(T*)fMemoryBlock.addrOf(S(index)); }
    const T& operator[](size_t index) const { return *(const T*)fMemoryBlock.addrOf(S(index)); }
    T* begin() { return (T*)fMemoryBlock.begin(); }
    T* end() { return (T*)fMemoryBlock.end(); }
    T* data() { return (T*)fMemoryBlock.data(); }
    const T* cbegin() const { return (const T*)fMemoryBlock.begin(); }
    const T* cend() const { return (const T*)fMemoryBlock.end(); }
    const T* data() const { return (const T*)fMemoryBlock.data(); }
    T* copyTo(size_t to, const void* from, size_t n) {
        return (T*)fMemoryBlock.copyTo(S(to), from, S(n));
    }
    T* copyFrom(void* to, size_t from, size_t n) {
        return (T*)fMemoryBlock.copyFrom(to, S(from), S(n));
    }
    void shrinkToFit(size_t n) { fMemoryBlock.shrinkToFit(S(n)); }
    size_t capacity() const { return this->cend() - this->cbegin(); }
    bool empty() const { return fMemoryBlock.empty(); }
    bool containsPtr(const T* ptr) const { return this->cbegin() <= ptr && ptr <= this->cend(); }
    size_t addressToIndex(const T* ptr) const {
        SkASSERT(this->containsPtr(ptr));
        return ptr - this->cbegin();
    }
    void swap(SkMemoryBlockAs& that) {
        using std::swap;
        swap(fMemoryBlock, that.fMemoryBlock);
    }

private:
    static constexpr size_t S(size_t s) { return s * sizeof(T); }
    static size_t SS(size_t s) { return SkSafeMath::Mul(s, sizeof(T)); }

    SkMemoryBlock fMemoryBlock;
};

template <typename T> static inline void swap(SkMemoryBlockAs<T>& a, SkMemoryBlockAs<T>& b) {
    a.swap(b);
}

template <typename T> class SkTDArray {
public:
    SkTDArray() { }
    SkTDArray(const T src[], int count)
            : fBlock{SkTo<size_t>(count)}
            , fCount {count} {
        SkASSERT(src || count == 0);

        if (fCount) {
            fBlock.copyTo(0, src, fCount);
        }
    }

    SkTDArray(const std::initializer_list<T>& list) : SkTDArray(list.begin(), list.size()) {}

    SkTDArray(const SkTDArray<T>& src)
        : fBlock{src.fBlock, SkTo<size_t>(src.fCount)}
        , fCount{src.fCount} {}

    SkTDArray(SkTDArray<T>&& src)
        : fBlock{std::move(src.fBlock)}
        , fCount{src.fCount} { }

    ~SkTDArray() = default;

    SkTDArray<T>& operator=(const SkTDArray<T>& src) {
        if (this != &src) {
            if (SkTo<size_t>(src.fCount) > fBlock.capacity()) {
                SkTDArray<T> tmp(src.data(), src.fCount);
                this->swap(tmp);
            } else if (!src.empty()) {
                fBlock.copyTo(0, src.data(), src.fCount);
                fCount = src.fCount;
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
        return  a.fCount == b.fCount &&
                (a.fCount == 0 ||
                 !memcmp(a.begin(), b.cbegin(), a.fCount * sizeof(T)));
    }
    friend bool operator!=(const SkTDArray<T>& a, const SkTDArray<T>& b) {
        return !(a == b);
    }

    void swap(SkTDArray<T>& that) {
        using std::swap;
        swap(fBlock, that.fBlock);
        swap(fCount, that.fCount);
    }

    bool empty() const { return fCount == 0; }
    bool isEmpty() const { return this->empty(); }

    /**
     *  Return the number of elements in the array
     */
    int count() const { return fCount; }
    size_t size() const { return fCount; }

    /**
     *  Return the total number of elements allocated.
     *  reserved() - count() gives you the number of elements you can add
     *  without causing an allocation.
     */
    int reserved() const { return SkTo<int>(fBlock.capacity()); }

    /**
     *  return the number of bytes in the array: count * sizeof(T)
     */
    size_t bytes() const { return fCount * sizeof(T); }

    T*        begin()       { return fBlock.begin(); }
    const T*  begin() const { return this->cbegin(); }
    const T* cbegin() const { return fBlock.cbegin(); }
    T*        end()       { return !fBlock.empty() ? &fBlock[fCount] : nullptr; }
    const T*  end() const { return this->cend(); }
    const T* cend() const { return !fBlock.empty() ? &fBlock[fCount] : nullptr; }

    T&  operator[](int index) {
        SkASSERT(index < fCount);
        return fBlock[index];
    }

    const T&  operator[](int index) const {
        SkASSERT(index < fCount);
        return fBlock[index];
    }

    T&  getAt(int index)  {
        return (*this)[index];
    }

    T* data() { return fBlock.data(); }
    const T* data() const { return fBlock.data(); }

    void reset() {
        this->~SkTDArray();
        new (this) SkTDArray{};
    }

    void rewind() {
        // same as setCount(0)
        fCount = 0;
    }

    /**
     *  Sets the number of elements in the array.
     *  If the array does not have space for count elements, it will increase
     *  the storage allocated to some amount greater than that required.
     *  It will never shrink the storage.
     */
    void setCount(int count) {
        SkASSERT(count >= 0);
        if (count > fCount) {
            fBlock.append(fCount, count - fCount);
        }
        fCount = count;
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
        auto answer = fBlock.insert(0, fCount, 1);
        fBlock.copyTo(1, 0, fCount);
        this->incCount(1);
        return &fBlock[0];
    }

    T* append() {
        return this->append(1, nullptr);
    }

    T* append(int count, const T* src = nullptr) {
        SkASSERT(0 <= count);
        int oldCount = fCount;
        if (count)  {
            SkASSERT(src == nullptr || fBlock.empty() || !fBlock.containsPtr(src));
            fBlock.append(fCount, count);
            this->incCount(count);
            if (src) {
                fBlock.copyTo(oldCount, src, count);
            }
        }

        return &fBlock[oldCount];
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
    T*      push() { return this->append(); }
    const T& top() const { return (*this)[fCount - 1]; }
    T&       top() { return (*this)[fCount - 1]; }
    void     pop(T* elem) { SkASSERT(fCount > 0); if (elem) *elem = (*this)[fCount - 1]; --fCount; }
    void     pop() { SkASSERT(fCount > 0); --fCount; }

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
        uint32_t count = (uint32_t)fCount + (uint32_t)delta;
        SkASSERT_RELEASE( SkTFitsIn<int>(count) );
        fCount = count;
        return fCount;
    }

    SkMemoryBlockAs<T> fBlock;
    int fCount {0};
};

template <typename T> static inline void swap(SkTDArray<T>& a, SkTDArray<T>& b) {
    a.swap(b);
}

#endif
