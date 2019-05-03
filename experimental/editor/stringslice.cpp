// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "stringslice.h"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <cstring>

using namespace editor;

void StringSlice::FreeWrapper::operator()(void* t) { free(t); }

StringSlice::StringSlice(StringSlice&& that)
    : fPtr(std::move(that.fPtr))
    , fLength(that.fLength)
    , fCapacity(that.fCapacity)
{
    that.fLength = 0;
    that.fCapacity = 0;
}

StringSlice& StringSlice::operator=(StringSlice&& that) {
    if (this != &that) {
        this->~StringSlice();
        new (this)StringSlice(std::move(that));
    }
    return *this;
}

void StringSlice::insert(size_t offset, const char* text, size_t length) {
    if (length) {
        offset = std::min(fLength, offset);
        this->reserve(fLength + length); // TODO: reserve extra???
        char* s = fPtr.get();
        if (offset != fLength) {
            memmove(s + offset + length, s + offset, fLength - offset);
        }
        if (text) {
            memcpy(s + offset, text, length);
        } else {
            memset(s + offset, 0, length);
        }
        fLength += length;
    }
}

void StringSlice::remove(size_t offset, size_t length) {
    if (length && offset < fLength) {
        length = std::min(length, fLength - offset);
        assert(length > 0);
        assert(length + offset <= fLength);
        if (length + offset < fLength) {
            char* s = fPtr.get();
            memmove(s + offset, s + offset + length, fLength - (length + offset));
        }
        fLength -= length;
    }
}

void StringSlice::reserve(size_t length) {
    if (length && length > fCapacity) {
        fPtr.reset((char*)realloc(fPtr.release(), length));
        fCapacity = length;
    }
}

void StringSlice::shrink() {
    fPtr.reset((char*)realloc(fPtr.release(), fLength));
    fCapacity = fLength;
}
