/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkReader32.h"
#include "SkString.h"
#include "SkWriter32.h"

/*
 *  Strings are stored as: length[4-bytes] + string_data + '\0' + pad_to_mul_4
 */

const char* SkReader32::readString(size_t* outLen) {
    size_t len = this->readInt();
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
    if (NULL == str) {
        str = "";
        len = 0;
    }
    if ((long)len < 0) {
        len = strlen(str);
    }
    this->write32(len);
    // add 1 since we also write a terminating 0
    size_t alignedLen = SkAlign4(len + 1);
    char* ptr = (char*)this->reserve(alignedLen);
    {
        // Write the terminating 0 and fill in the rest with zeroes
        uint32_t* padding = (uint32_t*)(ptr + (alignedLen - 4));
        *padding = 0;
    }
    // Copy the string itself.
    memcpy(ptr, str, len);
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
    bool wasExternal = (fExternal != NULL) && (fData == fExternal);
    // cause the buffer to grow
    fInternal.setCount(size);
    fData = fInternal.begin();
    if (wasExternal) {
        // we were external, so copy in the data
        memcpy(fData, fExternal, fUsed);
    }
    // Find out the size the buffer grew to, it may be more than we asked for.
    fCapacity = fInternal.reserved();
    // Expand the array so all reserved space is "used", we maintain the
    // amount we have written manually outside the array
    fInternal.setCount(fCapacity);
    SkASSERT(fInternal.count() == (int)fCapacity);
    SkASSERT(fInternal.reserved() == (int)fCapacity);
}
