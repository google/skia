/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBase64_DEFINED
#define SkBase64_DEFINED

#include "include/core/SkTypes.h"

struct SkBase64 {
public:
    enum Error {
        kNoError,
        kPadError,
        kBadCharError
    };

    SkBase64();
    Error decode(const char* src, size_t length);
    char* getData() { return fData; }
    size_t getDataSize() { return fLength; }
    /**
       Base64 encodes src into dst. encode is a pointer to at least 65 chars.
       encode[64] will be used as the pad character. Encodings other than the
       default encoding cannot be decoded.
    */
    static size_t Encode(const void* src, size_t length, void* dest, const char* encode = nullptr);

private:
    Error decode(const void* srcPtr, size_t length, bool writeDestination);

    size_t fLength;
    char* fData;
    friend class SkImageBaseBitmap;
};

#endif // SkBase64_DEFINED
