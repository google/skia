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
#include <cstddef>
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

    void reset();

    void assign(const void* src, int count, size_t sizeOfT);

    bool empty() const { return fCount == 0; }
    void clear() { fCount = 0; }
    int size() const { return fCount; }

    // Resizes the array to store exactly `newCount` elements.
    //
    // This never shrinks the allocation, and it may increase the allocation by
    // more than is strictly required, based on a private growth heuristic.
    void resize(int newCount, size_t sizeOfT);

    int decreaseCount() {
        SkASSERT(fCount > 0);
        fCount -= 1;
        return fCount;
    }

    void* push_back(size_t sizeOfT) {
        if (fCount < fReserve) {
            return fStorage + SkToSizeT(fCount++) * sizeOfT;
        } else {
            return this->append(sizeOfT);
        }
    }

    size_t size_bytes(size_t sizeOfT) const;

    int capacity() const { return fReserve; }
    void reserve(size_t newReserve, size_t sizeOfT);

    void shrinkToFit(size_t sizeOfT);
    void swap(SkTDStorage& that) {
        using std::swap;
        swap(fStorage, that.fStorage);
    }
    template <typename T> T* data() const { return reinterpret_cast<T*>(fStorage); }

    void* erase(int index, int count, size_t sizeOfT);
    // Removes the entry at 'index' and replaces it with the last array element
    void* removeShuffle(int index, size_t sizeOfT);

    void* prepend(size_t sizeOfT);
    void* append(size_t sizeOfT);
    void* append(const void* src, int count, size_t sizeOfT);

    void* insert(int index, size_t sizeOfT);
    void* insert(int index, const void* src, int count, size_t sizeOfT);

private:
    int calculateSizeDeltaOrDie(int delta) const;

    std::byte* fStorage{nullptr};
    int fReserve{0};  // size of the allocation in fArray (#elements)
    int fCount{0};    // logical number of elements (fCount <= fReserve)
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
        fStorage.assign(src, count, sizeof(T));
    }
    SkTDArray(const std::initializer_list<T>& list) : SkTDArray(list.begin(), list.size()) {}
    SkTDArray(const SkTDArray<T>& src) {
        fStorage.assign(src.data(), src.count(), sizeof(T));
    }
    SkTDArray<T>& operator=(const SkTDArray<T>& src) {
        if (this != &src) {
            fStorage.assign(src.data(), src.count(), sizeof(T));
        }
        return *this;
    }

    SkTDArray(SkTDArray<T>&& src)
        : fStorage{std::move(src.fStorage)} {}

    SkTDArray<T>& operator=(SkTDArray<T>&& src) {
        if (this != &src) {
            fStorage = std::move(src.fStorage);
        }
        return *this;
    }

    friend bool operator==(const SkTDArray<T>& a, const SkTDArray<T>& b) {
        return a.count() == b.count() &&
               (a.count() == 0 || !memcmp(a.data(), b.data(), SkToSizeT(a.size()) * sizeof(T)));
    }
    friend bool operator!=(const SkTDArray<T>& a, const SkTDArray<T>& b) { return !(a == b); }

    void swap(SkTDArray<T>& that) {
        using std::swap;
        swap(fStorage, that.fStorage);
    }

    bool empty() const { return fStorage.empty(); }

    // Return the number of elements in the array
    int count() const { return fStorage.size(); }
    int size() const { return fStorage.size(); }

     // Return the total number of elements allocated.
     // reserved() - count() gives you the number of elements you can add
     // without causing an allocation.
    int reserved() const { return fStorage.capacity(); }

    // return the number of bytes in the array: count * sizeof(T)
    size_t bytes() const { return SkToSizeT(this->size()) * sizeof(T); }

    T*       data() { return fStorage.data<T>(); }
    const T* data() const { return fStorage.data<T>(); }
    T*       begin() { return this->data(); }
    const T* begin() const { return this->data(); }
    T*       end() { return this->data() ? this->data() + this->size() : nullptr; }
    const T* end() const { return this->data() ? this->data() + this->size() : nullptr; }

    T& operator[](int index) {
        SkASSERT(index < this->size());
        return this->data()[index];
    }
    const T& operator[](int index) const {
        SkASSERT(index < this->size());
        return this->data()[index];
    }

    T& getAt(int index) { return (*this)[index]; }

    const T& back() const {
        SkASSERT(this->size() > 0);
        return this->data()[this->size() - 1];
    }
    T& back() {
        SkASSERT(this->size() > 0);
        return this->data()[this->size() - 1];
    }

    void reset() {
        fStorage.reset();
    }

    void rewind() {
        fStorage.clear();
    }

     // Sets the number of elements in the array.
     // If the array does not have space for count elements, it will increase
     // the storage allocated to some amount greater than that required.
     // It will never shrink the storage.
    void setCount(int count) {
        fStorage.resize(count, sizeof(T));
    }

    void reserve(size_t n) {
        fStorage.reserve(n, sizeof(T));
    }

    T* append() {
        return reinterpret_cast<T*>(fStorage.append(sizeof(T)));
    }
    T* append(int count, const T* src = nullptr) {
        return reinterpret_cast<T*>(fStorage.append(src, count, sizeof(T)));
    }

    T* insert(int index) {
        return reinterpret_cast<T*>(fStorage.insert(index, sizeof(T)));
    }
    T* insert(int index, int count, const T* src = nullptr) {
        return reinterpret_cast<T*>(fStorage.insert(index, src, count, sizeof(T)));
    }

    void remove(int index, int count = 1) {
        fStorage.erase(index, count, sizeof(T));
    }

    void removeShuffle(int index) {
        fStorage.removeShuffle(index, sizeof(T));
    }

    int find(const T& elem) const {
        const T* iter = this->begin();
        const T* stop = this->end();

        for (; iter < stop; iter++) {
            if (*iter == elem) {
                return SkToInt(iter - this->begin());
            }
        }
        return -1;
    }

    // routines to treat the array like a stack
    void     push_back(const T& v) { *reinterpret_cast<T*>(fStorage.push_back(sizeof(T))) = v; }

    void     pop(T* elem) {
        SkASSERT(this->size() > 0);
        if (elem) {
            *elem = (*this)[this->size() - 1];
        }
        fStorage.decreaseCount();
    }
    void pop() {
        fStorage.decreaseCount();
    }

    void deleteAll() {
        for (T p : *this) {
            delete p;
        }
        this->reset();
    }

    void freeAll() {
        for (T p : *this) {
            sk_free(p);
        }

        this->reset();
    }

    void unrefAll() {
        for (T p : *this) {
            p->unref();
        }
        this->reset();
    }

    void shrinkToFit() {
        fStorage.shrinkToFit(sizeof(T));
    }

private:
    SkTDStorage fStorage;
};

template <typename T> static inline void swap(SkTDArray<T>& a, SkTDArray<T>& b) { a.swap(b); }

#endif
