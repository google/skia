/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkData.h"
#include "SkDataUtils.h"
#include "SkStream.h"

/*static*/ SkData* SkDataUtils::ReadIntoSkData(SkStream &stream, size_t maxBytes) {
    if (0 == maxBytes) {
        return SkData::NewEmpty();
    }
    char* bufStart = reinterpret_cast<char *>(sk_malloc_throw(maxBytes));
    char* bufPtr = bufStart;
    size_t bytesRemaining = maxBytes;
    while (bytesRemaining > 0) {
        size_t bytesReadThisTime = stream.read(bufPtr, bytesRemaining);
        if (0 == bytesReadThisTime) {
            break;
        }
        bytesRemaining -= bytesReadThisTime;
        bufPtr += bytesReadThisTime;
    }
    return SkData::NewFromMalloc(bufStart, maxBytes - bytesRemaining);
}
