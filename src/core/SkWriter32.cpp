/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkString.h"
#include "include/private/SkTo.h"
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkReader32.h"

#include "src/core/SkWriter32.h"

void SkWriter32::writeMatrix(const SkMatrix& matrix) {
    size_t size = SkMatrixPriv::WriteToMemory(matrix, nullptr);
    SkASSERT(SkAlign4(size) == size);
    SkMatrixPriv::WriteToMemory(matrix, this->reserve(size));
}

/*
 *  Strings are stored as: length[4-bytes] + string_data + '\0' + pad_to_mul_4
 */

const char* SkReader32::readString(size_t* outLen) {
    size_t len = this->readU32();
    const void* ptr = this->peek();

    // skip over the string + '\0' and then pad to a multiple of 4
    size_t alignedSize = SkAlign4(len + 1);
    this->skip(alignedSize);

    if (outLen) {
        *outLen = len;
    }
    return (const char*)ptr;
}

size_t SkReader32::readIntoString(SkString* copy) {
    size_t len;
    const char* ptr = this->readString(&len);
    if (copy) {
        copy->set(ptr, len);
    }
    return len;
}

void SkWriter32::writeString(const char str[], size_t len) {
    if (nullptr == str) {
        str = "";
        len = 0;
    }
    if ((long)len < 0) {
        len = strlen(str);
    }

    // [ 4 byte len ] [ str ... ] [1 - 4 \0s]
    uint32_t* ptr = this->reservePad(sizeof(uint32_t) + len + 1);
    *ptr = SkToU32(len);
    char* chars = (char*)(ptr + 1);
    memcpy(chars, str, len);
    chars[len] = '\0';
}

size_t SkWriter32::WriteStringSize(const char* str, size_t len) {
    if ((long)len < 0) {
        SkASSERT(str);
        len = strlen(str);
    }
    const size_t lenBytes = 4;    // we use 4 bytes to record the length
    // add 1 since we also write a terminating 0
    return SkAlign4(lenBytes + len + 1);
}

void SkWriter32::growToAtLeast(size_t size) {
    const bool wasExternal = (fExternal != nullptr) && (fData == fExternal);

    fCapacity = 4096 + std::max(size, fCapacity + (fCapacity / 2));
    fInternal.realloc(fCapacity);
    fData = fInternal.get();

    if (wasExternal) {
        // we were external, so copy in the data
        memcpy(fData, fExternal, fUsed);
    }
}

sk_sp<SkData> SkWriter32::snapshotAsData() const {
    return SkData::MakeWithCopy(fData, fUsed);
}
