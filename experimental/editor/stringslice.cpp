// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "experimental/editor/stringslice.h"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <cstring>

using namespace editor;

void SliceImpl::D::operator()(void* t) { std::free(t); }

SliceImpl::SliceImpl(SliceImpl&& that)
    : fPtr(std::move(that.fPtr))
    , fLength(that.fLength)
    , fCapacity(that.fCapacity)
{
    that.fLength = 0;
    that.fCapacity = 0;
}

SliceImpl& SliceImpl::operator=(SliceImpl&& that) {
    if (this != &that) {
        this->~SliceImpl();
        new (this)SliceImpl(std::move(that));
    }
    return *this;
}

SliceImpl& SliceImpl::operator=(const SliceImpl& that) {
    if (this != &that) {
        fLength = 0;
        if (that.size() > 0) {
            this->insert(0, that.data(), that.size());
        }
    }
    return *this;
}

void SliceImpl::insert(std::size_t offset, const void* text, std::size_t length) {
    if (length) {
        offset = std::min(fLength, offset);
        this->realloc(fLength + length);
        char* s = static_cast<char*>(fPtr.get());
        assert(s);
        if (offset != fLength) {
            std::memmove(s + offset + length, s + offset, fLength - offset);
        }
        if (text) {
            std::memcpy(s + offset, text, length);
        } else {
            std::memset(s + offset, 0, length);
        }
        fLength += length;
    }
}

void SliceImpl::remove(std::size_t offset, std::size_t length) {
    if (length && offset < fLength) {
        length = std::min(length, fLength - offset);
        assert(length > 0);
        assert(length + offset <= fLength);
        if (length + offset < fLength) {
            char* s = static_cast<char*>(fPtr.get());
            assert(s);
            std::memmove(s + offset, s + offset + length, fLength - (length + offset));
        }
        fLength -= length;
    }
}

void SliceImpl::realloc(std::size_t size) {
    // round up to multiple of (1 << kBits) bytes
    static constexpr unsigned kBits = 4;
    std::size_t newCapacity = std::max(fLength, size);
    newCapacity = newCapacity ? (((newCapacity - 1) >> kBits) + 1) << kBits : 0;
    if (newCapacity != fCapacity) {
        assert(newCapacity % (1u << kBits) == 0);
        assert(newCapacity >= size);
        assert(newCapacity >= fLength);
        fCapacity = newCapacity;
        fPtr.reset(fCapacity ? std::realloc(fPtr.release(), fCapacity) : nullptr);
    }
}
