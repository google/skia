/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTDArray_DEFINED
#define SkTDArray_DEFINED

#include "include/core/SkTypes.h"
#include "include/private/SkMalloc.h"
#include "include/private/SkTo.h"

#include <algorithm>
#include <climits>
#include <initializer_list>
#include <tuple>
#include <utility>

class SK_SPI SkTDStorage {
public:
    SkTDStorage() = default;
    SkTDStorage(const SkTDStorage& that) = delete;
    SkTDStorage& operator= (const SkTDStorage& that) = delete;
    SkTDStorage(SkTDStorage&& that);
    SkTDStorage& operator= (SkTDStorage&& that);
    ~SkTDStorage();

    int assign(const void* src, int count, size_t sizeOfT);

    int resizeStorageToAtLeast(int count, size_t sizeOfT);
    int shrinkToFit(int count, size_t sizeOfT);
    void swap(SkTDStorage& that) {
        using std::swap;
        swap(fStorage, that.fStorage);
    }
    template <typename T>
    T* data() const { return static_cast<T*>(fStorage); }

    struct StateUpdate {int count, reserve;};
    StateUpdate append(
            const void* src, int count, size_t sizeOfT, int reserve, int oldCount);
private:
    void* fStorage{nullptr};
};

template <typename T> static inline void swap(SkTDStorage& a, SkTDStorage& b) { a.swap(b); }

// SkTDArray<T> implements a std::vector-like array for raw data-only objects that do not require
// construction or destruction. The constructor and destructor for T will not be called; T objects
// will always be moved via raw memcpy. Newly created T objects will contain uninitialized memory.
//
// In most cases, std::vector<T> can provide a similar level of performance for POD objects when
// used with appropriate care. In new code, consider std::vector<T> instead.
template <typename T> class SkTDArray {
public:
    SkTDArray() = default;
    SkTDArray(const T src[], int count) {
        SkASSERT(src || count == 0);
        fReserve = fStorage.assign(src, count, sizeof(T));
        fCount = count;
    }
    SkTDArray(const std::initializer_list<T>& list) : SkTDArray(list.begin(), list.size()) {}
    SkTDArray(const SkTDArray<T>& src) {
        fReserve = fStorage.assign(src.array(), src.fCount, sizeof(T));
        fCount = src.fCount;
    }
    SkTDArray<T>& operator=(const SkTDArray<T>& src) {
        if (this != &src) {
            if (src.fCount > fReserve) {
                fReserve = fStorage.assign(src.array(), src.fCount, sizeof(T));
            } else {
                sk_careful_memcpy(this->array(), src.array(), sizeof(T) * SkToSizeT(src.fCount));
            }
            fCount = src.fCount;
        }
        return *this;
    }

    SkTDArray(SkTDArray<T>&& src)
        : fStorage{std::move(src.fStorage)}
        , fReserve{src.fReserve}
        , fCount{src.fCount} {}

    SkTDArray<T>& operator=(SkTDArray<T>&& src) {
        if (this != &src) {
            fStorage = std::move(src.fStorage);
            fReserve = std::exchange(src.fReserve, 0);
            fCount = std::exchange(src.fCount, 0);
        }
        return *this;
    }

    friend bool operator==(const SkTDArray<T>& a, const SkTDArray<T>& b) {
        return a.fCount == b.fCount &&
               (a.fCount == 0 || !memcmp(a.array(), b.array(), SkToSizeT(a.fCount) * sizeof(T)));
    }
    friend bool operator!=(const SkTDArray<T>& a, const SkTDArray<T>& b) { return !(a == b); }

    void swap(SkTDArray<T>& that) {
        using std::swap;
        swap(fStorage, that.fStorage);
        swap(fReserve, that.fReserve);
        swap(fCount, that.fCount);
    }

    bool isEmpty() const { return fCount == 0; }
    bool empty() const { return this->isEmpty(); }

    // Return the number of elements in the array
    int    count() const { return fCount; }
    size_t size() const { return fCount; }

     // Return the total number of elements allocated.
     // reserved() - count() gives you the number of elements you can add
     // without causing an allocation.
    int reserved() const { return fReserve; }

    // return the number of bytes in the array: count * sizeof(T)
    size_t bytes() const { return fCount * sizeof(T); }

    T*       data() { return this->array(); }
    const T* data() const { return this->array(); }
    T*       begin() { return this->array(); }
    const T* begin() const { return this->array(); }
    T*       end() { return this->array() ? this->array() + fCount : nullptr; }
    const T* end() const { return this->array() ? this->array() + fCount : nullptr; }

    T& operator[](int index) {
        SkASSERT(index < fCount);
        return this->array()[index];
    }
    const T& operator[](int index) const {
        SkASSERT(index < fCount);
        return this->array()[index];
    }

    T& getAt(int index) { return (*this)[index]; }

    const T& back() const {
        SkASSERT(fCount > 0);
        return this->array()[fCount - 1];
    }
    T& back() {
        SkASSERT(fCount > 0);
        return this->array()[fCount - 1];
    }

    void reset() {
        this->~SkTDArray();
        new (this) SkTDArray{};
    }

    void rewind() {
        // same as setCount(0)
        fCount = 0;
    }

     // Sets the number of elements in the array.
     // If the array does not have space for count elements, it will increase
     // the storage allocated to some amount greater than that required.
     // It will never shrink the storage.
    void setCount(int count) {
        SkASSERT(count >= 0);
        if (count > fReserve) {
            this->resizeStorageToAtLeast(count);
        }
        fCount = count;
    }

    void setReserve(int reserve) {
        SkASSERT(reserve >= 0);
        if (reserve > fReserve) {
            this->resizeStorageToAtLeast(reserve);
        }
    }
    void reserve(size_t n) {
        SkASSERT_RELEASE(SkTFitsIn<int>(n));
        this->setReserve(SkToInt(n));
    }

    T* prepend() {
        this->adjustCount(1);
        memmove(this->array() + 1, this->array(), (fCount - 1) * sizeof(T));
        return this->array();
    }

    T* append() { return this->append(1, nullptr); }
    T* append(int count, const T* src = nullptr) {
        int oldCount = fCount;
        auto [newCount, newReserve] = fStorage.append(src, count, sizeof(T), fReserve, fCount);
        fCount = newCount;
        fReserve = newReserve;
        return this->array() + oldCount;
    }

    T* insert(int index) { return this->insert(index, 1, nullptr); }
    T* insert(int index, int count, const T* src = nullptr) {
        SkASSERT(count);
        SkASSERT(index <= fCount);
        size_t oldCount = fCount;
        this->adjustCount(count);
        T* dst = this->array() + index;
        memmove(dst + count, dst, sizeof(T) * (oldCount - index));
        if (src) {
            memcpy(dst, src, sizeof(T) * count);
        }
        return dst;
    }

    void remove(int index, int count = 1) {
        SkASSERT(index + count <= fCount);
        fCount = fCount - count;
        memmove(this->array() + index, this->array() + index + count, sizeof(T) * (fCount - index));
    }

    void removeShuffle(int index) {
        SkASSERT(index < fCount);
        int newCount = fCount - 1;
        fCount = newCount;
        if (index != newCount) {
            memcpy(this->array() + index, this->array() + newCount, sizeof(T));
        }
    }

    int find(const T& elem) const {
        const T* iter = this->array();
        const T* stop = this->array() + fCount;

        for (; iter < stop; iter++) {
            if (*iter == elem) {
                return SkToInt(iter - this->array());
            }
        }
        return -1;
    }

    int rfind(const T& elem) const {
        const T* iter = this->array() + fCount;
        const T* stop = this->array();

        while (iter > stop) {
            if (*--iter == elem) {
                return SkToInt(iter - stop);
            }
        }
        return -1;
    }

    // Returns true iff the array contains this element.
    bool contains(const T& elem) const { return (this->find(elem) >= 0); }

    // Copies up to max elements into dst. The number of items copied is
    // capped by count - index. The actual number copied is returned.
    int copyRange(T* dst, int index, int max) const {
        SkASSERT(max >= 0);
        SkASSERT(!max || dst);
        if (index >= fCount) {
            return 0;
        }
        int count = std::min(max, fCount - index);
        memcpy(dst, this->array() + index, sizeof(T) * count);
        return count;
    }

    void copy(T* dst) const { this->copyRange(dst, 0, fCount); }

    // routines to treat the array like a stack
    void     push_back(const T& v) { *this->append() = v; }
    T*       push() { return this->append(); }
    const T& top() const { return (*this)[fCount - 1]; }
    T&       top() { return (*this)[fCount - 1]; }
    void     pop(T* elem) {
        SkASSERT(fCount > 0);
        if (elem) *elem = (*this)[fCount - 1];
        --fCount;
    }
    void pop() {
        SkASSERT(fCount > 0);
        --fCount;
    }

    void deleteAll() {
        T* iter = this->array();
        T* stop = this->array() + fCount;
        while (iter < stop) {
            delete *iter;
            iter += 1;
        }
        this->reset();
    }

    void freeAll() {
        T* iter = this->array();
        T* stop = this->array() + fCount;
        while (iter < stop) {
            sk_free(*iter);
            iter += 1;
        }
        this->reset();
    }

    void unrefAll() {
        T* iter = this->array();
        T* stop = this->array() + fCount;
        while (iter < stop) {
            (*iter)->unref();
            iter += 1;
        }
        this->reset();
    }

    void safeUnrefAll() {
        T* iter = this->array();
        T* stop = this->array() + fCount;
        while (iter < stop) {
            SkSafeUnref(*iter);
            iter += 1;
        }
        this->reset();
    }

#ifdef SK_DEBUG
    void validate() const {
        SkASSERT((fReserve == 0 && this->array() == nullptr) ||
                 (fReserve > 0 && this->array() != nullptr));
        SkASSERT(fCount <= fReserve);
    }
#endif

    void shrinkToFit() {
        if (fReserve > fCount) {
            fReserve = fStorage.shrinkToFit(fCount, sizeof(T));
        }
    }

private:
    T* array() { return fStorage.data<T>(); }
    const T* array() const { return fStorage.data<T>(); }

    // Adjusts the number of elements in the array.
    // This is the same as calling setCount(count() + delta).
    void adjustCount(int delta) {
        SkASSERT(delta > 0);

        // We take care to avoid overflow here.
        // The sum of fCount and delta is at most 4294967294, which fits fine in uint32_t.
        uint32_t count = (uint32_t)fCount + (uint32_t)delta;
        SkASSERT_RELEASE(SkTFitsIn<int>(count));

        this->setCount(SkTo<int>(count));
    }

    // Increase the storage allocation such that it can hold (fCount + extra)
    // elements.
    // It never shrinks the allocation, and it may increase the allocation by
    //  more than is strictly required, based on a private growth heuristic.
    //
    //  note: this does NOT modify fCount
    void resizeStorageToAtLeast(int count) {
        SkASSERT(count > fReserve);
        fReserve = fStorage.resizeStorageToAtLeast(count, sizeof(T));
    }

    SkTDStorage fStorage;
    int fReserve = 0;  // size of the allocation in fArray (#elements)
    int fCount = 0;    // logical number of elements (fCount <= fReserve)
};

template <typename T> static inline void swap(SkTDArray<T>& a, SkTDArray<T>& b) { a.swap(b); }

#endif
