/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTArrayDebug_DEFINED
#define SkTArrayDebug_DEFINED

#include <iterator>
#include <type_traits>
#include <vector>

#include "include/core/SkTypes.h"  // for SkASSERT

/** The debug implementation of SkTArray<T> wraps a std::vector. This allows debuggers to visualize
 *  the contents of the array. This class does not attempt to replicate the exact performance, size
 *  or safety characteristics of a SkTArray since it is intended for debug use. (It should never be
 *  *less* safe than a SkTArray, but may err on the side of being more safe.)
 *
 *  Notable differences in the debug implementation:
 *  - SkSTArray does not actually have inline storage, because std::vector<> doesn't support it.
 *  - MEM_MOVE is not honored. std::vector always copies and moves objects using their constructors.
 *    This can expose edge cases where SkTArray is used to wrap a type that doesn't actually
 *    support std::move (!). Generally, those cases should just be fixed.
 *  - SkTArray<bool> does not work properly because std::vector<bool> is not a conforming container.
 *    These can generally be replaced with SkTArray<uint8_t>.
 */
template <typename T, bool MEM_MOVE = false> class SkTArray {
public:
    SkTArray() {}
    explicit SkTArray(int reserveCount)       { fVector.reserve(reserveCount); }
    SkTArray(const SkTArray& that)            { fVector = that.fVector; }
    SkTArray(SkTArray&& that)                 { fVector = std::move(that.fVector); }
    SkTArray& operator=(const SkTArray& that) { fVector = that.fVector; return *this; }
    SkTArray& operator=(SkTArray&& that)      { fVector = std::move(that.fVector); return *this; }
    SkTArray(const T* array, int count)       { this->reset(array, count); }

    void reset()                          { fVector.clear(); }
    void reset(int n)                     { fVector.clear(); fVector.resize(n); }
    void reset(const T* array, int count) { fVector = std::vector<T>(array, array + count); }

    void reserve(int n) { fVector.reserve(fVector.size() + n); }

    void removeShuffle(int n) {
        if (n != this->count() - 1) {
            fVector[n] = std::move(fVector.back());
        }
        fVector.pop_back();
    }

    int count() const   { return fVector.size(); }
    size_t size() const { return fVector.size(); }
    bool empty() const  { return fVector.empty(); }

    T& push_back()           { return fVector.emplace_back(); }
    T& push_back(const T& t) { return fVector.emplace_back(t); }
    T& push_back(T&& t)      { return fVector.emplace_back(std::move(t)); }

    template <class... Args> T& emplace_back(Args&&... args) {
        return fVector.emplace_back(std::forward<Args>(args)...);
    }

    T* push_back_n(int n) {
        SkASSERT(n >= 0);
        fVector.resize(fVector.size() + n);
        return &*(fVector.end() - n);
    }

    T* push_back_n(int n, const T& t) {
        SkASSERT(n >= 0);
        fVector.insert(fVector.end(), n, t);
        return &*(fVector.end() - n);
    }

    T* push_back_n(int n, const T t[]) {
        SkASSERT(n >= 0);
        fVector.insert(fVector.end(), t, t + n);
        return &*(fVector.end() - n);
    }

    T* move_back_n(int n, T t[]) {
        SkASSERT(n >= 0);
        fVector.insert(fVector.end(), std::make_move_iterator(t), std::make_move_iterator(t + n));
        return &*(fVector.end() - n);
    }

    void pop_back()                { fVector.pop_back(); }
    void pop_back_n(int n)         { fVector.erase(fVector.end() - n, fVector.end()); }
    void resize(size_t newCount)   { fVector.resize(newCount); }
    void resize_back(int newCount) { fVector.resize(newCount); }
    void swap(SkTArray& that)      { fVector.swap(that.fVector); }

    T* begin()             { return empty() ? nullptr : &*fVector.begin(); }
    const T* begin() const { return empty() ? nullptr : &*fVector.begin(); }
    T* end()               { return empty() ? nullptr : &*fVector.end(); }
    const T* end() const   { return empty() ? nullptr : &*fVector.end(); }
    T* data()              { return empty() ? nullptr : fVector.data(); }
    const T* data() const  { return empty() ? nullptr : fVector.data(); }

    T& operator[] (int i)             { return fVector[i]; }
    const T& operator[] (int i) const { return fVector[i]; }
    T& at(int i)                      { return fVector.at(i); }
    const T& at(int i) const          { return fVector.at(i); }

    T& front()             { return fVector.front(); }
    const T& front() const { return fVector.front(); }
    T& back()              { return fVector.back(); }
    const T& back() const  { return fVector.back(); }

    T& fromBack(int i)             { return fVector.rbegin()[i]; }
    const T& fromBack(int i) const { return fVector.rbegin()[i]; }

    bool operator==(const SkTArray<T, MEM_MOVE>& right) const { return fVector == right.fVector; }
    bool operator!=(const SkTArray<T, MEM_MOVE>& right) const { return fVector != right.fVector; }

protected:
    std::vector<T> fVector;
};

template <typename T, bool M> static inline void swap(SkTArray<T, M>& a, SkTArray<T, M>& b) {
    a.swap(b);
}

/**
 *  SkSTArray is an alias for SkTArray. Inline storage optimization is not supported.
 */
template <int N, typename T, bool MEM_MOVE = false>
class SkSTArray : public SkTArray<T, MEM_MOVE> {
public:
    // Inherit constructors.
    using SkTArray<T, MEM_MOVE>::SkTArray;

    // Support copying/moving from the base class type.
    SkSTArray(const SkTArray<T, MEM_MOVE>& that) {
        this->fVector = that.fVector;
    }
    SkSTArray(SkTArray<T, MEM_MOVE>&& that) {
        this->fVector = std::move(that.fVector);
    }
    SkSTArray& operator=(const SkTArray<T, MEM_MOVE>& that) {
        this->fVector = that.fVector;
        return *this;
    }
    SkSTArray& operator=(SkTArray<T, MEM_MOVE>&& that) {
        this->fVector = std::move(that.fVector);
        return *this;
    }
};

#endif
