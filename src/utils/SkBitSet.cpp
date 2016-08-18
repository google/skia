/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkBitSet.h"

SkBitSet::SkBitSet(int numberOfBits)
    : fBitData(nullptr), fDwordCount(0), fBitCount(numberOfBits) {
    SkASSERT(numberOfBits > 0);
    // Round up size to 32-bit boundary.
    fDwordCount = (numberOfBits + 31) / 32;
    fBitData.set(sk_calloc_throw(fDwordCount * sizeof(uint32_t)));
}

SkBitSet::SkBitSet(SkBitSet&& source)
    : fBitData(source.fBitData.release())
    , fDwordCount(source.fDwordCount)
    , fBitCount(source.fBitCount) {
    source.fDwordCount = 0;
    source.fBitCount = 0;
}

SkBitSet& SkBitSet::operator=(SkBitSet&& rhs) {
    if (this != &rhs) {
        fBitCount = rhs.fBitCount;
        fDwordCount = rhs.fDwordCount;
        fBitData.reset();  // Free old pointer.
        fBitData.set(rhs.fBitData.release());
        rhs.fBitCount = 0;
        rhs.fDwordCount = 0;
    }
    return *this;
}

bool SkBitSet::operator==(const SkBitSet& rhs) {
    if (fBitCount == rhs.fBitCount) {
        if (fBitData.get() != nullptr) {
            return (memcmp(fBitData.get(), rhs.fBitData.get(),
                           fDwordCount * sizeof(uint32_t)) == 0);
        }
        return true;
    }
    return false;
}

bool SkBitSet::operator!=(const SkBitSet& rhs) {
    return !(*this == rhs);
}

void SkBitSet::clearAll() {
    if (fBitData.get() != nullptr) {
        sk_bzero(fBitData.get(), fDwordCount * sizeof(uint32_t));
    }
}

bool SkBitSet::orBits(const SkBitSet& source) {
    if (fBitCount != source.fBitCount) {
        return false;
    }
    uint32_t* targetBitmap = this->internalGet(0);
    uint32_t* sourceBitmap = source.internalGet(0);
    for (size_t i = 0; i < fDwordCount; ++i) {
        targetBitmap[i] |= sourceBitmap[i];
    }
    return true;
}
