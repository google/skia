// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef stringslice_DEFINED
#define stringslice_DEFINED

#include "experimental/editor/stringview.h"

#include <cstddef>
#include <memory>
#include <type_traits>

namespace editor {
class SliceImpl {
public:
    SliceImpl() = default;
    SliceImpl(SliceImpl&&);
    SliceImpl(const SliceImpl& that) { this->insert(0, that.data(), that.size()); }
    ~SliceImpl() = default;
    SliceImpl& operator=(SliceImpl&&);
    SliceImpl& operator=(const SliceImpl&);

    void* data() { return fPtr.get(); }
    const void* data() const { return fPtr.get(); }
    std::size_t size() const { return fLength; }

    void insert(std::size_t offset, const void* src, std::size_t length);
    void remove(std::size_t offset, std::size_t length);
    void realloc(std::size_t newCapacity);

private:
    struct D { void operator()(void*); };
    std::unique_ptr<void, D> fPtr;
    std::size_t fLength = 0;
    std::size_t fCapacity = 0;
};

template <typename T>
class Slice {
public:
    Slice() = default;
    Slice(const T* s, std::size_t l) { this->insert(0, s, l); }
    Slice(Slice&&) = default;
    Slice(const Slice& that) = default;
    ~Slice() = default;
    Slice& operator=(Slice&&) = default;
    Slice& operator=(const Slice&) = default;

    // access:
    const T* begin() const { return static_cast<const T*>(fImpl.data()); }
    const T* end() const {
        return static_cast<const T*>(static_cast<const char*>(fImpl.data()) + fImpl.size());
    }
    std::size_t size() const { return fImpl.size() / sizeof(T); }
    editor::Span<const T> view() const { return {this->begin(), this->size()}; }

    // mutation:
    T* data() { return static_cast<T*>(fImpl.data()); }
    void insert(std::size_t offset, const T* src, std::size_t length) {
        fImpl.insert(offset * sizeof(T), src, length * sizeof(T));
    }
    void remove(std::size_t offset, std::size_t length) {
        fImpl.remove(offset * sizeof(T), length * sizeof(T));
    }

    // modify capacity only:
    void reserve(std::size_t size) { fImpl.realloc(size * sizeof(T)); }
    void shrink() { fImpl.realloc(fImpl.size()); }

private:
    SliceImpl fImpl;
    static_assert(std::is_pod<T>::value, "");
};

// A lightweight modifiable string class.
using StringSlice = Slice<char>;

}  // namespace editor;
#endif  // stringslice_DEFINED
