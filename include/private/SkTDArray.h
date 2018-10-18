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
        if (this->addrOf<T>(i + n) <= fSize) {
            return this->addrOf<T>(i);
        }

        return this->moveTail(sizeof(T), i, i + n, 0);
    }

    template <typename T>
    T* insert(size_t i, size_t e, size_t n) {
        SkASSERT(i <= e);

        if (sizeof(T) * (e + n) < fSize) {
            if (i < e) {
                this->copyTo(i + n, this->addrOf<T>(i), e - i);
            }
            return this->addrOf<T>(i);
        }

        return this->moveTail(sizeof(T), i, i + n, e - i);
    }

    template <typename T>
    T* copyTo(size_t index, T* src, size_t count) {
        return (T*)memmove(this->addrOf<T>(index), src, count * sizeof(T));
    }

    template <typename T>
    int addressToIndex(void* addr) const {
        SkASSERT(this->containsAddr(addr));

        return SkTo<int>((T*)addr - (T*)fBlock);
    }

    uint8_t& operator[](size_t index) {
        SkASSERT(index < fSize);
        return fBlock[index];
    }

    const uint8_t&  operator[](size_t index) const {
        SkASSERT(index < fSize);
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
    void* addrOf(size_t i) { return &fBlock[sizeof(T) * i]; }
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
        SkTDArray<T> tmp{src.fArray, src.fCount};
        this->swap(tmp);
    }

    SkTDArray(SkTDArray<T>&& src) {
        this->swap(src);
    }

    ~SkTDArray() = default;

    SkTDArray<T>& operator=(const SkTDArray<T>& src) {
        if (this != &src) {
            if (src.fCount > fBlock.capacity(sizeof(T))) {
                SkTDArray<T> tmp(src.fArray, src.fCount);
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
    T*        end()       { return fBlock.empty() ? this->addrOf(fCount) : nullptr; }
    const T*  end() const { return this->cend(); }
    const T* cend() const { return fBlock.empty() ? this->addrOf(fCount) : nullptr; }

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
    const T&  getAt(int index) const {
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
        SkASSERT(reserve >= 0);
        if (reserve > fBlock.capacity(sizeof(T))) {
            fBlock.append<T>(reserve);
        }
    }
    void setReserve(int n) {
        SkASSERT_RELEASE(SkTFitsIn<int>(n));
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
            SkASSERT(src == nullptr || fBlock.empty() || !fBlock.containsAddr(src));
            auto space = fBlock.append<T>(fCount, count);
            this->incCount(count);
            if (src) {
                memcpy(space, src, sizeof(T) * count);
            }
        }
        return addrOf(oldCount);
    }

    T* appendClear() {
        T* result = this->append();
        *result = 0;
        return result;
    }

    T* insert(int index) {
        return this->insert(index, 1, nullptr);
    }
    T* insert(int index, int count, const T* src = nullptr) {
        SkASSERT(count);
        SkASSERT(index <= fCount);
        size_t oldCount = fCount;
        this->adjustCount(count);
        T* dst = fBlock.begin<T>() + index;
        memmove(dst + count, dst, sizeof(T) * (oldCount - index));
        if (src) {
            memcpy(dst, src, sizeof(T) * count);
        }
        return dst;
    }

    void remove(int index, int count = 1) {
        SkASSERT(index + count <= fCount);
        fBlock.copyTo(index, addrOf(index + count), fCount - index);
        fCount = fCount - count;
    }

    void removeShuffle(int index) {
        SkASSERT(index < fCount);
        int newCount = fCount - 1;
        fCount = newCount;
        if (index != newCount) {
            memcpy(fArray + index, fArray + newCount, sizeof(T));
        }
    }

    int find(const T& elem) const {

        for (const T& t : *this) {
            if (t == elem) {
                return fBlock.addressToIndex(&t);
            }
        }
        return -1;
    }

    int rfind(const T& elem) const {
        const T* iter = fArray + fCount;
        const T* stop = fArray;

        while (iter > stop) {
            if (*--iter == elem) {
                return SkToInt(iter - stop);
            }
        }
        return -1;
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
        memcpy(dst, fArray + index, sizeof(T) * count);
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

    void deleteAll() {
        T*  iter = fArray;
        T*  stop = fArray + fCount;
        while (iter < stop) {
            delete *iter;
            iter += 1;
        }
        this->reset();
    }

    void freeAll() {
        T*  iter = fArray;
        T*  stop = fArray + fCount;
        while (iter < stop) {
            sk_free(*iter);
            iter += 1;
        }
        this->reset();
    }

    void unrefAll() {
        T*  iter = fArray;
        T*  stop = fArray + fCount;
        while (iter < stop) {
            (*iter)->unref();
            iter += 1;
        }
        this->reset();
    }

    void safeUnrefAll() {
        T*  iter = fArray;
        T*  stop = fArray + fCount;
        while (iter < stop) {
            SkSafeUnref(*iter);
            iter += 1;
        }
        this->reset();
    }

    void visitAll(void visitor(T&)) {
        T* stop = this->end();
        for (T* curr = this->begin(); curr < stop; curr++) {
            if (*curr) {
                visitor(*curr);
            }
        }
    }

#ifdef SK_DEBUG
    void validate() const {
        SkASSERT((fReserve == 0 && fArray == nullptr) ||
                 (fReserve > 0 && fArray != nullptr));
        SkASSERT(fCount <= fReserve);
    }
#endif

    void shrinkToFit() {
        fBlock.shrinkToFit(sizeof(T), fCount);
    }

private:
    T* addrOf(int index) {
        return &fBlock[sizeof(T) * index];
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

    /**
     *  Adjusts the number of elements in the array.
     *  This is the same as calling setCount(count() + delta).
     */
    void adjustCount(int delta) {
        this->setCount(incCount(delta));
    }

    SkMemoryBlock fBlock;
    int fCount {0};
};

template <typename T> static inline void swap(SkTDArray<T>& a, SkTDArray<T>& b) {
    a.swap(b);
}

#endif
