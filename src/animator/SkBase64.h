
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkBase64_DEFINED
#define SkBase64_DEFINED

#include "SkTypes.h"

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
    static size_t Encode(const void* src, size_t length, void* dest);

#ifdef SK_SUPPORT_UNITTEST
    static void UnitTest();
#endif
private:
    Error decode(const void* srcPtr, size_t length, bool writeDestination);

    size_t fLength;
    char* fData;
    friend class SkImage;
};

#endif // SkBase64_DEFINED
