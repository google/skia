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
    SkMemoryBlock() { }
    SkMemoryBlock(size_t Tsize, size_t n);
    ~SkMemoryBlock();
    template <typename T>
    T* append(size_t i, size_t n) {
        if (this->sizeOf<T>(i + n) <= fSize) {
            return this->addrOf<T>(i);
        }

        return (T*)this->moveTail(sizeof(T), i, i + n, 0);
    }

    template <typename T>
    T* insert(size_t i, size_t e, size_t n) {
        SkASSERT(i <= e);

        if (this->sizeOf<T>(e + n) < fSize) {
            if (i < e) {
                this->copyTo(i + n, this->addrOf<T>(i), e - i);
            }
            return this->addrOf<T>(i);
        }

        return (T*)this->moveTail(sizeof(T), i, i + n, e - i);
    }

    template <typename T>
    T* copyTo(size_t index, T* src, size_t count) {
        return (T*)memmove((void*)this->addrOf<T>(index), (void*)src, this->sizeOf<T>(count));
    }

    template <typename T>
    int addressToIndex(T* addr) const {
        SkASSERT(this->containsAddr((void*)addr));

        return SkTo<int>((T*)addr - (T*)fBlock);
    }

    uint8_t& operator[](size_t index) {
        SkASSERT(index <= fSize);
        return fBlock[index];
    }

    const uint8_t&  operator[](size_t index) const {
        SkASSERT(index <= fSize);
        return fBlock[index];
    }

    uint8_t* begin()              { return fBlock; }
    const uint8_t* begin() const  { return fBlock; }
    const uint8_t* cbegin() const { return fBlock; }

    bool empty() const { return fSize == 0; }

    void* data() const { return fBlock; }

    void* shrinkToFit(size_t Tsize, size_t n);
    void swap(SkMemoryBlock& that);
    size_t capacity(size_t Tsize) const ;
    bool containsAddr(void * addr) const;

private:
    template <typename T>
    T* addrOf(size_t i) { return (T*)(fBlock + this->sizeOf<T>(i)); }
    template <typename T>
    size_t sizeOf(size_t i) { return sizeof(T) * i; }
    void* moveTail(size_t Tsize, size_t from, size_t to, size_t n);

    uint8_t* fBlock{nullptr};
    size_t   fSize{0};
};

template <typename T> class SkTDArray {
public:
    SkTDArray() { }
    SkTDArray(const T src[], int count)
            : fBlock{sizeof(T), SkTo<size_t>(count)}
            , fCount {count} {
        SkASSERT(src || count == 0);

        if (fCount) {
            fBlock.copyTo(0, src, fCount);
        }
    }

    SkTDArray(const std::initializer_list<T>& list) : SkTDArray(list.begin(), list.size()) {}

    SkTDArray(const SkTDArray<T>& src) {
        SkTDArray<T> tmp{src.addrOf(0), src.fCount};
        this->swap(tmp);
    }

    SkTDArray(SkTDArray<T>&& src) {
        this->swap(src);
    }

    ~SkTDArray() = default;

    SkTDArray<T>& operator=(const SkTDArray<T>& src) {
        if (this != &src) {
            if (SkTo<size_t>(src.fCount) > fBlock.capacity(sizeof(T))) {
                SkTDArray<T> tmp(src.addrOf(0), src.fCount);
                this->swap(tmp);
            } else if (!src.empty()) {
                fBlock.copyTo(0, src.addrOf(0), src.fCount);
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
                 !memcmp(a.addrOf(0), b.addrOf(0), a.fCount * sizeof(T)));
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
    int reserved() const { return fBlock.capacity(sizeof(T)); }

    /**
     *  return the number of bytes in the array: count * sizeof(T)
     */
    size_t bytes() const { return fCount * sizeof(T); }

    T*        begin()       { return (T*)fBlock.begin(); }
    const T*  begin() const { return this->cbegin(); }
    const T* cbegin() const { return (const T*)fBlock.cbegin(); }
    T*        end()       { return !fBlock.empty() ? this->addrOf(fCount) : nullptr; }
    const T*  end() const { return this->cend(); }
    const T* cend() const { return !fBlock.empty() ? this->addrOf(fCount) : nullptr; }

    T&  operator[](int index) {
        SkASSERT(index < fCount);
        return *addrOf(index);
    }

    const T&  operator[](int index) const {
        SkASSERT(index < fCount);
        return *addrOf(index);
    }

    T&  getAt(int index)  {
        return (*this)[index];
    }


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
            fBlock.append<T>(fCount, count - fCount);
        }
        fCount = count;
    }

    void reserve(size_t reserve) {
        SkASSERT_RELEASE(SkTFitsIn<int>(reserve));
        SkASSERT(reserve >= 0);
        auto capacity = fBlock.capacity(sizeof(T));
        if (reserve > capacity) {
            fBlock.append<T>(fCount, reserve - capacity);
        }
    }
    void setReserve(int n) {
        this->reserve(SkTo<size_t>(n));
    }

    T* prepend() {
        auto answer = fBlock.insert(0, fCount, 1);
        fBlock.copyTo(1, 0, fCount);
        this->incCount(1);
        return addrOf(0);
    }

    T* append() {
        return this->append(1, nullptr);
    }
    T* append(int count, const T* src = nullptr) {
        SkASSERT(0 <= count);
        int oldCount = fCount;
        if (count)  {
            SkASSERT(src == nullptr || fBlock.empty() || !fBlock.containsAddr((void*)src));
            auto space = fBlock.append<T>(fCount, count);
            this->incCount(count);
            if (src) {
                memcpy(space, src, sizeof(T) * count);
            }
        }

        return addrOf(oldCount);
    }

    T* insert(int index) {
        return this->insert(index, 1, nullptr);
    }

    T* insert(int index, int count, const T* src = nullptr) {
        SkASSERT(count);
        SkASSERT(index <= fCount);
        auto result = fBlock.insert<T>(index, fCount, count);
        this->incCount(count);
        if (src) {
            fBlock.copyTo(index, src, count);
        }
        return result;
    }

    void remove(int index, int count = 1) {
        SkASSERT(index + count <= fCount);
        fBlock.copyTo(index, addrOf(index + count), fCount - index);
        fCount = fCount - count;
    }

    void removeShuffle(int index) {
        SkASSERT(index < fCount);

        fCount--;
        // Only move if index was not the last element.
        if (index != fCount) {
            fBlock.copyTo(index, this->addrOf(fCount), 1);
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

        memcpy(dst, this->addrOf(index), sizeof(T) * count);
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
                return fBlock.addressToIndex(&t);
            }
        }
        return -1;
    }

    int rfind(const T& elem) const {
        const T* iter = this->addrOf(fCount);
        const T* stop = this->addrOf(0);

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
        fBlock.shrinkToFit(sizeof(T), fCount);
    }

private:
    T* addrOf(int index) const {
        return (T*)&fBlock[sizeof(T) * index];
    }

    int incCount(int delta) {
        SkASSERT(delta > 0);
        // We take care to avoid overflow here.
        // The sum of fCount and delta is at most 4294967294, which fits fine in uint32_t.
        uint32_t count = (uint32_t)fCount + (uint32_t)delta;
        SkASSERT_RELEASE( SkTFitsIn<int>(count) );
        fCount = count;
        return fCount;
    }

    SkMemoryBlock fBlock;
    int fCount {0};
};

template <typename T> static inline void swap(SkTDArray<T>& a, SkTDArray<T>& b) {
    a.swap(b);
}

#endif
