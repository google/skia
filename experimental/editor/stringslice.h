// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef stringslice_DEFINED
#define stringslice_DEFINED

#include <memory>
#include <cstddef>

namespace editor {
// A lightweight modifiable string class.
class StringSlice {
public:
    StringSlice() = default;
    StringSlice(const char* s, std::size_t l) { this->insert(0, s, l); }
    ~StringSlice() = default;
    StringSlice(StringSlice&&);
    StringSlice& operator=(StringSlice&&);

    // access:
    // Does not have a c_str method; is *not* NUL-terminated.
    const char* begin() const { return fPtr.get(); }
    const char* end() const { return fPtr ? fPtr.get() + fLength : nullptr; }
    std::size_t size() const { return fLength; }

    // mutation:
    void insert(std::size_t offset, const char* text, std::size_t length);
    void remove(std::size_t offset, std::size_t length);
    void reserve(std::size_t);
    void shrink();

private:
    struct FreeWrapper { void operator()(void*); };
    std::unique_ptr<char, FreeWrapper> fPtr;
    std::size_t fLength = 0;
    std::size_t fCapacity = 0;

    StringSlice(const StringSlice&) = delete;
    StringSlice& operator=(const StringSlice&) = delete;
};
}  // namespace editor;
#endif  // stringslice_DEFINED
