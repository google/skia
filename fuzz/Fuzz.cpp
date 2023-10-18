/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "fuzz/Fuzz.h"
#include "fuzz/FuzzCommon.h"

// UBSAN reminds us that bool can only legally hold 0 or 1.
void Fuzz::next(bool* b) {
    uint8_t n;
    this->next(&n);
    *b = (n & 1) == 1;
}

void Fuzz::nextBytes(void* n, size_t size) {
    if ((fNextByte + size) > fSize) {
        sk_bzero(n, size);
        memcpy(n, fData + fNextByte, fSize - fNextByte);
        fNextByte = fSize;
        return;
    }
    memcpy(n, fData + fNextByte, size);
    fNextByte += size;
}

void Fuzz::next(SkRegion* region) {
    // See FuzzCommon.h
    FuzzNiceRegion(this, region, 10);
}

void Fuzz::nextRange(float* f, float min, float max) {
    this->next(f);
    if (!std::isnormal(*f) && *f != 0.0f) {
        // Don't deal with infinity or other strange floats.
        *f = max;
    }
    *f = min + std::fmod(std::abs(*f), (max - min + 1));
}
