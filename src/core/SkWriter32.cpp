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

    // [ 4 byte len ] [ str ... ] [1 - 4 \0s]
    uint32_t* ptr = this->reservePad(sizeof(uint32_t) + len + 1);
    *ptr = len;
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
    const bool wasExternal = (fExternal != NULL) && (fData == fExternal);

    fCapacity = 4096 + SkTMax(size, fCapacity + (fCapacity / 2));
    fInternal.realloc(fCapacity);
    fData = fInternal.get();

    if (wasExternal) {
        // we were external, so copy in the data
        memcpy(fData, fExternal, fUsed);
    }
    // Invalidate the snapshot, we know it is no longer useful.
    fSnapshot.reset(NULL);
}

SkData* SkWriter32::snapshotAsData() const {
    // get a non const version of this, we are only conceptually const
    SkWriter32& mutable_this = *const_cast<SkWriter32*>(this);
    // we use size change detection to invalidate the cached data
    if ((fSnapshot.get() != NULL) && (fSnapshot->size() != fUsed)) {
        mutable_this.fSnapshot.reset(NULL);
    }
    if (fSnapshot.get() == NULL) {
        uint8_t* buffer = NULL;
        if ((fExternal != NULL) && (fData == fExternal)) {
            // We need to copy to an allocated buffer before returning.
            buffer = (uint8_t*)sk_malloc_throw(fUsed);
            memcpy(buffer, fData, fUsed);
        } else {
            buffer = mutable_this.fInternal.detach();
            // prepare us to do copy on write, by pretending the data buffer
            // is external and size limited
            mutable_this.fData = buffer;
            mutable_this.fCapacity = fUsed;
            mutable_this.fExternal = buffer;
        }
        mutable_this.fSnapshot.reset(SkData::NewFromMalloc(buffer, fUsed));
    }
    return SkRef(fSnapshot.get()); // Take an extra ref for the caller.
}
