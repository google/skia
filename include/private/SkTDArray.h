/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkTDArray_DEFINED
#define SkTDArray_DEFINED

#include "SkTypes.h"
#include <vector>

// SkTDArray is now a thin wrapper over std::vector.
// Please feel free to use either.

// Implementation notes:
//   1) We take care to use SkToInt(size_t) and SkToSizeT(int) to do conversions that
//      assert the value fits either direction.  The SkToInt(size_t) should be obvious,
//      but even SkToSizeT(int) has caught negative values passed in where you'd expect
//      >= 0.
//
//   2) We try to add yet-undefined items to the array with default initialization.
//      This can look funny, as the only really good way to get that is to write
//      "T v;", a T default-intialized on the stack, often meaning uninitialized.
//      This avoids the slightly more expensive value initialization (like, floats
//      to 0.0f) that std::vector normally provides.
//
//   3) See the end of the file for how we avoid std::vector<bool>.

template <typename T>
class SkTDArray {
public:
    SkTDArray()                            = default;
    ~SkTDArray()                           = default;
    SkTDArray(const SkTDArray&)            = default;
    SkTDArray(SkTDArray&&)                 = default;
    SkTDArray& operator=(const SkTDArray&) = default;
    SkTDArray& operator=(SkTDArray&&)      = default;

    SkTDArray(const T* src, int n) : fVec(src, src+SkToSizeT(n)) {}

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

    const T& operator[](int k) const { return fVec[SkToSizeT(k)]; }
          T& operator[](int k)       { return fVec[SkToSizeT(k)]; }
    const T&      getAt(int k) const { return fVec[SkToSizeT(k)]; }
          T&      getAt(int k)       { return fVec[SkToSizeT(k)]; }

    void reset() { this->~SkTDArray(); new (this) SkTDArray(); }
    void rewind()          { fVec.clear(); }
    void setCount  (int n) { T v; fVec.resize(SkToSizeT(n), v); }
    void setReserve(int n) { fVec.reserve(SkToSizeT(n)); }
    void shrinkToFit()     { fVec.shrink_to_fit(); }

    T* append(int n = 1, const T* src = nullptr) {
        return this->insert(this->count(), n, src);
    }
    T* insert(int ix, int n = 1, const T* src = nullptr) {
        if (src) {
            return &*fVec.insert(fVec.begin() + SkToSizeT(ix), src, src+SkToSizeT(n));
        }
        T v;
        return &*fVec.insert(fVec.begin() + SkToSizeT(ix), SkToSizeT(n), v);
    }

    void remove(int ix, int n = 1) {
        fVec.erase(fVec.begin() + SkToSizeT(ix),
                   fVec.begin() + SkToSizeT(ix) + SkToSizeT(n));
    }
    void removeShuffle(int ix) {
        std::swap(fVec[SkToSizeT(ix)], fVec.back());
        fVec.pop_back();
    }

    int find(const T& value) const {
        for (int i = 0; i < this->count(); i++) {
            if (fVec[i] == value) {
                return i;
            }
        }
        return -1;
    }
    bool contains(const T& value) const {
        return this->find(value) >= 0;
    }

    void     push(const T& value)    { fVec.push_back(value); }
    T*       push()                  { T v; fVec.push_back(v); return &fVec.back(); }
    void     pop(T* value = nullptr) { if (value) { *value = fVec.back(); } fVec.pop_back(); }
    const T& top() const             { return fVec.back(); }
          T& top()                   { return fVec.back(); }

    void    deleteAll() { for (T ptr : fVec) { delete ptr;       } this->reset(); }
    void      freeAll() { for (T ptr : fVec) { sk_free(ptr);     } this->reset(); }
    void     unrefAll() { for (T ptr : fVec) { ptr->unref();     } this->reset(); }
    void safeUnrefAll() { for (T ptr : fVec) { SkSafeUnref(ptr); } this->reset(); }

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
