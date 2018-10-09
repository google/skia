// Copyright 2018 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef SkVector_DEFINED
#define SkVector_DEFINED

#include "SkTArray.h"
// sk::Vector is a drop-in replacement for std::vector.
namespace sk {
template <typename T>
class Vector : public SkTArray<T> {
public:
    Vector() {}
    explicit Vector(size_t s) { this->push_back_n((int)s); }
    Vector(const T* begin, const T* end) : SkTArray<T>(begin, end - begin) {}
    Vector(Vector&& that) = default;
    Vector(const Vector& that) = default;
    Vector& operator=(Vector&& that) = default;
    Vector& operator=(const Vector& that) = default;

    T* data() { return this->begin(); }
    const T* data() const { return this->begin(); }
    size_t size() const { return (size_t)this->count(); }
    void resize(size_t count) { this->resize_back((int)count); }
};
}
#endif  // SkVector_DEFINED
