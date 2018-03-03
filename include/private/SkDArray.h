/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDArray_DEFINED
#define SkDArray_DEFINED

#include "SkTypes.h"
#include "SkMalloc.h"
#include "SkSafeMath.h"

class SkDArray {
public:
    SkDArray(size_t elemSize) : fArray(nullptr), fElemSize(elemSize), fReserve(0), fCount(0) {
        SkASSERT(elemSize);
    }

    SkDArray(const void* src, size_t elemSize, int count) : fElemSize(elemSize) {
        SkASSERT(elemSize);
        SkASSERT(src || count == 0);

        fReserve = fCount = 0;
        fArray = nullptr;
        if (count) {
            this->setCount(count);
            memcpy(fArray, src, this->countBytes());
        }
    }

    SkDArray(const SkDArray& src)
        : fArray(nullptr), fElemSize(src.fElemSize), fReserve(0), fCount(0) {
        SkDArray tmp(src.fArray, src.fElemSize, src.fCount);
        this->swap(tmp); // use move
    }
    SkDArray(SkDArray&& src)
        : fArray(nullptr), fElemSize(src.fElemSize), fReserve(0), fCount(0) {
        this->swap(src);
    }
    ~SkDArray() {
        sk_free(fArray);
    }

    SkDArray& operator=(const SkDArray& src) {
        if (this != &src) {
            if (src.countBytes() > this->reserveBytes()) {
                SkDArray tmp(src.fArray, src.fElemSize, src.fCount);
                this->swap(tmp);
            } else {
                sk_careful_memcpy(fArray, src.fArray, src.countBytes());
                fElemSize = src.fElemSize;
                fCount = src.fCount;
            }
        }
        return *this;
    }
    SkDArray& operator=(SkDArray&& src) {
        if (this != &src) {
            this->swap(src);
            src.reset();
        }
        return *this;
    }

    friend bool operator==(const SkDArray& a, const SkDArray& b) {
        return  a.fElemSize == b.fElemSize &&
                a.fCount == b.fCount &&
                (a.fCount == 0 ||
                 !memcmp(a.fArray, b.fArray, a.countBytes()));
    }
    friend bool operator!=(const SkDArray& a, const SkDArray& b) {
        return !(a == b);
    }

    void swap(SkDArray& other) {
        SkTSwap(fArray, other.fArray);
        SkTSwap(fReserve, other.fReserve);
        SkTSwap(fElemSize, other.fElemSize);
        SkTSwap(fCount, other.fCount);
    }

    /** Return a ptr to the array of data, to be freed with sk_free. This also
        resets the SkDArray to be empty.
     */
    void* release() {
        void* array = fArray;
        fArray = nullptr;
        fReserve = fCount = 0;
        return array;
    }

    bool isEmpty() const { return fCount == 0; }

    /**
     *  Return the number of elements in the array
     */
    int count() const { return fCount; }

    size_t elemSize() const { return fElemSize; }

    /**
     *  Return the total number of elements allocated.
     *  reserved() - count() gives you the number of elements you can add
     *  without causing an allocation.
     */
    int reserved() const { return fReserve; }

    /**
     *  return the number of bytes in the array: count * sizeof(T)
     */
    size_t countBytes() const {
#ifdef SK_DEBUG
        size_t safeSize = SkSafeMath::Mul(fCount, fElemSize);
        SkASSERT(safeSize == fCount * fElemSize);
#endif
        return fCount * fElemSize;
    }

    void* begin() { return fArray; }
    const void* begin() const { return fArray; }
    void* end() { return fArray ? (char*)fArray + this->countBytes() : nullptr; }
    const void* end() const { return fArray ? (const char*)fArray + this->countBytes() : nullptr; }

    void* getAt(int index)  {
        return (char*)fArray + index * fElemSize;
    }
    const void* getAt(int index) const {
        return (const char*)fArray + index * fElemSize;
    }

    void* operator[](int index) {
        SkASSERT(index < fCount);
        return this->getAt(index);
    }
    const void* operator[](int index) const {
        SkASSERT(index < fCount);
        return this->getAt(index);
    }

    void reset() {
        if (fArray) {
            sk_free(fArray);
            fArray = nullptr;
            fReserve = fCount = 0;
        } else {
            SkASSERT(fReserve == 0 && fCount == 0);
        }
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
        if (count > fReserve) {
            this->resizeStorageToAtLeast(count);
        }
        fCount = count;
    }

    void setReserve(int reserve) {
        if (reserve > fReserve) {
            this->resizeStorageToAtLeast(reserve);
        }
    }

    void* prepend() {
        this->adjustCount(1);
        memmove((char*)fArray + fElemSize, fArray, (fCount - 1) * fElemSize);
        return fArray;
    }

    void* append() {
        return this->append(1, nullptr);
    }
    void* append(int count, const void* src = nullptr) {
        int oldCount = fCount;
        if (count)  {
            SkASSERT(src == nullptr || fArray == nullptr ||
                     (const char*)src + count * fElemSize <= (const char*)fArray ||
                     (const char*)fArray + oldCount * fElemSize <= (const char*)src);

            this->adjustCount(count);
            if (src) {
                memcpy((char*)fArray + oldCount * fElemSize, src, fElemSize * count);
            }
        }
        return (char*)fArray + oldCount * fElemSize;
    }

    void* appendClear() {
        void* result = this->append();
        sk_bzero(result, fElemSize);
        return result;
    }

    void* insert(int index) {
        return this->insert(index, 1, nullptr);
    }
    void* insert(int index, int count, const void* src = nullptr) {
        SkASSERT(count);
        SkASSERT(index <= fCount);
        size_t oldCount = fCount;
        this->adjustCount(count);
        char* dst = (char*)fArray + index * fElemSize;
        memmove(dst + count * fElemSize, dst, fElemSize * (oldCount - index));
        if (src) {
            memcpy(dst, src, fElemSize * count);
        }
        return dst;
    }

    void remove(int index, int count = 1) {
        SkASSERT(index + count <= fCount);
        fCount = fCount - count;
        memmove((char*)fArray + index * fElemSize,
                (const char*)fArray + (index + count) * fElemSize,
                fElemSize * (fCount - index));
    }

    void removeShuffle(int index, int count = 1) {
        SkASSERT(index < fCount);
        int newCount = fCount - count;
        fCount = newCount;
        if (index != newCount) {
            memcpy((char*)fArray + index * fElemSize, (const char*)fArray + newCount * fElemSize,
                   fElemSize);
        }
    }

    void pop() {
        SkASSERT(fCount > 0);
        --fCount;
    }

    /**
     * Copies up to max elements into dst. The number of items copied is
     * capped by count - index. The actual number copied is returned.
     */
    int copyRange(void* dst, int index, int max) const {
        SkASSERT(max >= 0);
        SkASSERT(!max || dst);
        if (index >= fCount) {
            return 0;
        }
        int count = SkMin32(max, fCount - index);
        memcpy(dst, (const char*)fArray + fElemSize * index, fElemSize * count);
        return count;
    }

    void copy(void* dst) const {
        this->copyRange(dst, 0, fCount);
    }

    void freeAll() {
        void** iter = (void**)fArray;
        void** stop = (void**)fArray + fCount;
        while (iter < stop) {
            sk_free(*iter);
            iter += 1;
        }
        this->reset();
    }

#ifdef SK_DEBUG
    void validate() const {
        SkASSERT((fReserve == 0 && fArray == nullptr) ||
                 (fReserve > 0 && fArray != nullptr));
        SkASSERT(fCount <= fReserve);
        size_t size = SkSafeMath::Mul(fReserve, fElemSize);
        SkASSERT(size == fReserve * fElemSize);
    }
#endif

    void shrinkToFit() {
        fReserve = fCount;
        fArray = sk_realloc_throw(fArray, fReserve * fElemSize);
    }

private:
    void*   fArray;
    size_t  fElemSize;
    int     fReserve;
    int     fCount;

    size_t reserveBytes() const { return fReserve * fElemSize; }

    /**
     *  Adjusts the number of elements in the array.
     *  This is the same as calling setCount(count() + delta).
     */
    void adjustCount(int delta) {
        this->setCount(fCount + delta);
    }

    /**
     *  Increase the storage allocation such that it can hold (fCount + extra)
     *  elements.
     *  It never shrinks the allocation, and it may increase the allocation by
     *  more than is strictly required, based on a private growth heuristic.
     *
     *  note: does NOT modify fCount
     */
    void resizeStorageToAtLeast(int count) {
        SkASSERT(count > fReserve);
        fReserve = count + 4;
        fReserve += fReserve / 4;
        fArray = sk_realloc_throw(fArray, fReserve, fElemSize);
    }
};

#endif
