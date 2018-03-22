/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkTDArray_DEFINED
#define SkTDArray_DEFINED

#include "SkTypes.h"
#include <new>
#include <vector>

// SkTDArray is now a thin wrapper over std::vector.
// Please feel free to use either.

template <typename T>
class SkTDArray {
public:
    SkTDArray() {}
    SkTDArray(const T* src, int n) : fVec(src,src+n) {}

    ~SkTDArray()                           = default;
    SkTDArray(const SkTDArray&)            = default;
    SkTDArray(SkTDArray&&)                 = default;
    SkTDArray& operator=(const SkTDArray&) = default;
    SkTDArray& operator=(SkTDArray&&)      = default;

    friend bool operator==(const SkTDArray& a, const SkTDArray& b) { return a.fVec == b.fVec; }
    friend bool operator!=(const SkTDArray& a, const SkTDArray& b) { return a.fVec != b.fVec; }

    void swap(SkTDArray& that) { std::swap(fVec, that.fVec); }

    bool isEmpty() const { return fVec.empty(); }
    int    count() const { return SkToInt(fVec.size()); }
    int reserved() const { return SkToInt(fVec.capacity()); }
    size_t bytes() const { return sizeof(T) * fVec.size(); }

    const T* begin() const { return fVec.data(); }
          T* begin()       { return fVec.data(); }
    const T*   end() const { return this->begin() + fVec.size(); }
          T*   end()       { return this->begin() + fVec.size(); }

    const T& operator[](int k) const { return fVec[k]; }
          T& operator[](int k)       { return fVec[k]; }

    const T& getAt(int k) const { return fVec[k]; }
          T& getAt(int k)       { return fVec[k]; }

    void reset() { this->~SkTDArray(); new (this) SkTDArray(); }
    void rewind()          { fVec.clear(); }
    void setCount  (int n) { fVec.resize(n); }
    void setReserve(int n) { fVec.reserve(n); }
    void shrinkToFit()     { fVec.shrink_to_fit(); }

    T* prepend() { return &*fVec.insert(fVec.begin(), T{}); }

    T* append() { return this->append(1); }
    T* append(int n, const T* src = nullptr) {
        return this->insert(this->count(), n, src);
    }
    T* appendClear() {  // For most T, you can just call append() for the same effect now.
        T* result = this->append();
        *result = 0;
        return result;
    }

    T* insert(int ix) { return this->insert(ix, 1); }
    T* insert(int ix, int n, const T* src = nullptr) {
        if (src) {
            return &*fVec.insert(fVec.begin() + ix, src, src+n);
        }
        return &*fVec.insert(fVec.begin() + ix, n, T{});
    }

    void remove(int ix, int n = 1) {
        fVec.erase(fVec.begin() + ix, fVec.begin() + ix + n);
    }

    void removeShuffle(int ix) {
        std::swap(fVec[ix], fVec.back());
        fVec.pop_back();
    }

    template <typename Fn>
    int select(Fn&& fn) const {
        for (int i = 0; i < this->count(); i++) {
            if (fn(fVec[i])) {
                return i;
            }
        }
        return -1;
    }
    int find(const T& value) const {
        return this->select([&](const T& v) { return v == value; });
    }
    bool contains(const T& value) const {
        return this->find(value) >= 0;
    }

    int rfind(const T& value) const {
        for (int i = this->count() - 1; i >= 0; i--) {
            if (fVec[i] == value) {
                return i;
            }
        }
        return -1;
    }

    int copyRange(T* dst, int ix, int max) const {
        int n = std::min(max, this->count() - ix);
        if (n > 0) {
            for (int i = 0; i < n; i++) {
                *dst++ = fVec[ix+i];
            }
            return n;
        }
        return 0;
    }
    void copy(T* dst) const {
        this->copyRange(dst, 0, this->count());
    }

    void     push(const T& value)  { fVec.push_back(value); }
    T*       push()                { fVec.push_back(T{}); return &fVec.back(); }
    const T& top() const           { return fVec.back(); }
          T& top()                 { return fVec.back(); }
    void     pop(T* value=nullptr) { if (value) { *value = fVec.back(); } fVec.pop_back(); }

    void    deleteAll() { for (T ptr : fVec) { delete ptr;       } this->reset(); }
    void      freeAll() { for (T ptr : fVec) { sk_free(ptr);     } this->reset(); }
    void     unrefAll() { for (T ptr : fVec) { ptr->unref();     } this->reset(); }
    void safeUnrefAll() { for (T ptr : fVec) { SkSafeUnref(ptr); } this->reset(); }

    void visitAll(void fn(T&)) {
        for (T ptr : fVec) {
            if (ptr) { fn(ptr); }
        }
    }

#if defined(SK_DEBUG)
    void validate() const {}
#endif

private:
    std::vector<T> fVec;
};

// Avoid using std::vector<bool> (annoying weird bitvector specialization).

struct SkTDArray_bool {
    bool value;

    SkTDArray_bool() = default;
    SkTDArray_bool(bool v) : value(v) {}
    operator bool() const { return value; }
};

template <>
class SkTDArray<bool> : public SkTDArray<SkTDArray_bool> {};

#endif
