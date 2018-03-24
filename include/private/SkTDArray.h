
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkTDArray_DEFINED
#define SkTDArray_DEFINED

#include "../private/SkDArray.h"
#include "SkTypes.h"
#include "SkMalloc.h"

template <typename T> class SkTDArray {
public:
    SkTDArray() : fArray(sizeof(T)) {}
    SkTDArray(const T src[], int count) : fArray(src, sizeof(T), count) {}
    SkTDArray(const SkTDArray<T>& src) : fArray(src.fArray) {}
    SkTDArray(SkTDArray<T>&& src) : fArray(src.fArray) {}

    SkTDArray<T>& operator=(const SkTDArray<T>& src) {
        fArray = src.fArray;
        return *this;
    }
    SkTDArray<T>& operator=(SkTDArray<T>&& src) {
        fArray = std::move(src.fArray);
        return *this;
    }

    friend bool operator==(const SkTDArray<T>& a, const SkTDArray<T>& b) {
        return a.fArray == b.fArray;
    }
    friend bool operator!=(const SkTDArray<T>& a, const SkTDArray<T>& b) {
        return a.fArray != b.fArray;
    }

    void swap(SkTDArray<T>& other) {
        SkTSwap(fArray, other.fArray);
    }

    // The deleter that ought to be used for a std:: smart pointer that takes ownership from
    // release().
    struct Deleter {
        void operator()(const void* p) { sk_free((void*)p); }
    };

    /** Return a ptr to the array of data, to be freed with sk_free. This also
        resets the SkTDArray to be empty.
     */
    T* release() { return (T*)fArray.release(); }

    bool isEmpty() const { return fArray.isEmpty(); }

    /**
     *  Return the number of elements in the array
     */
    int count() const { return fArray.count(); }

    /**
     *  Return the total number of elements allocated.
     *  reserved() - count() gives you the number of elements you can add
     *  without causing an allocation.
     */
    int reserved() const { return fArray.reserved(); }

    /**
     *  return the number of bytes in the array: count * sizeof(T)
     */
    size_t bytes() const { return fArray.countBytes(); }

    T* begin() { return (T*)fArray.begin(); }
    const T* begin() const { return (const T*)fArray.begin(); }
    T* end() { return (T*)fArray.end(); }
    const T* end() const { return (const T*)fArray.end(); }

    T& operator[](int index) { return *(T*)fArray[index]; }
    const T& operator[](int index) const { return *(const T*)fArray[index]; }

    T& getAt(int index) { return *(T*)fArray.getAt(index); }
    const T& getAt(int index) const { return *(const T*)fArray.getAt(index); }

    void reset() { fArray.reset(); }
    void rewind() { fArray.rewind(); }

    /**
     *  Sets the number of elements in the array.
     *  If the array does not have space for count elements, it will increase
     *  the storage allocated to some amount greater than that required.
     *  It will never shrink the storage.
     */
    void setCount(int count) { fArray.setCount(count); }

    void setReserve(int reserve) { fArray.setReserve(reserve); }

    T* prepend() { return (T*)fArray.prepend(); }
    T* append() { return (T*)fArray.append(); }
    T* append(int count, const T* src = nullptr) { return (T*)fArray.append(count, src); }
    T* appendClear() { return (T*)fArray.appendClear(); }

    T* insert(int index) { return (T*)fArray.insert(index); }
    T* insert(int index, int count, const T* src = nullptr) {
        return (T*)fArray.insert(index, count, src);
    }

    void remove(int index, int count = 1) { fArray.remove(index, count); }
    void removeShuffle(int index, int count = 1) { fArray.removeShuffle(index, count); }

    template <typename S> int select(S&& selector) const {
        const T* iter = this->begin();
        const T* stop = this->end();

        for (; iter < stop; iter++) {
            if (selector(*iter)) {
                return SkToInt(iter - this->begin());
            }
        }
        return -1;
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
    int copyRange(T* dst, int index, int max) const { return fArray.copyRange(dst, index, max); }
    void copy(T* dst) const { this->copy(dst); }

    // routines to treat the array like a stack
    T*       push() { return this->append(); }
    void     push(const T& elem) { *this->append() = elem; }
    const T& top() const { return (*this)[this->count() - 1]; }
    T&       top() { return (*this)[this->count() - 1]; }
    void     pop(T* elem) {
        SkASSERT(this->count() > 0);
        if (elem) {
            *elem = (*this)[this->count() - 1];
            fArray.pop();
        }
    }
    void     pop() { fArray.pop(); }

    void deleteAll() {
        T*  iter = this->begin();
        T*  stop = this->end();
        while (iter < stop) {
            delete *iter;
            iter += 1;
        }
        this->reset();
    }

    void freeAll() {
        SkASSERT(sizeof(T) == sizeof(void*));
        fArray.freeAll();
    }

    void unrefAll() {
        T*  iter = this->begin();
        T*  stop = this->end();
        while (iter < stop) {
            (*iter)->unref();
            iter += 1;
        }
        this->reset();
    }

    void safeUnrefAll() {
        T*  iter = this->begin();
        T*  stop = this->end();
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
        SkASSERT(sizeof(T) == fArray.elemSize());
        fArray.validate();
    }
#endif

    void shrinkToFit() { fArray.shrinkToFit(); }

private:
    SkDArray  fArray;
};

#endif
