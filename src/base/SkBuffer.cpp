/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/base/SkBuffer.h"

#include "include/private/base/SkAlign.h"
#include "include/private/base/SkMalloc.h"

#include <cstdint>

///////////////////////////////////////////////////////////////////////////////////////////////////

const void* SkRBuffer::skip(size_t size) {
    if (fValid && size <= this->available()) {
        const void* pos = fPos;
        fPos += size;
        return pos;
    }
    fValid = false;
    return nullptr;
}

bool SkRBuffer::read(void* buffer, size_t size) {
    if (const void* src = this->skip(size)) {
        sk_careful_memcpy(buffer, src, size);
        return true;
    }
    return false;
}

bool SkRBuffer::skipToAlign4() {
    intptr_t pos = reinterpret_cast<intptr_t>(fPos);
    size_t n = SkAlign4(pos) - pos;
    if (fValid && n <= this->available()) {
        fPos += n;
        return true;
    } else {
        fValid = false;
        return false;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void* SkWBuffer::skip(size_t size) {
    void* result = fPos;
    writeNoSizeCheck(nullptr, size);
    return fData == nullptr ? nullptr : result;
}

void SkWBuffer::writeNoSizeCheck(const void* buffer, size_t size) {
    SkASSERT(fData == nullptr || fStop == nullptr || fPos + size <= fStop);
    if (fData && buffer) {
        sk_careful_memcpy(fPos, buffer, size);
    }
    fPos += size;
}

size_t SkWBuffer::padToAlign4() {
    size_t pos = this->pos();
    size_t n = SkAlign4(pos) - pos;

    if (n && fData)
    {
        char* p = fPos;
        char* stop = p + n;
        do {
            *p++ = 0;
        } while (p < stop);
    }
    fPos += n;
    return n;
}

#if 0
#ifdef SK_DEBUG
    static void AssertBuffer32(const void* buffer)
    {
        SkASSERT(buffer);
        SkASSERT(((size_t)buffer & 3) == 0);
    }
#else
    #define AssertBuffer32(buffer)
#endif

#endif
