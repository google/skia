/*
 * Copyright (C) 2011 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <string.h>
#include "SkBitSet.h"

SkBitSet::SkBitSet(int numberOfBits)
    : fBitData(NULL), fDwordCount(0), fBitCount(numberOfBits) {
    SkASSERT(numberOfBits > 0);
    // Round up size to 32-bit boundary.
    fDwordCount = (numberOfBits + 31) / 32;
    fBitData.set(malloc(fDwordCount * sizeof(uint32_t)));
    clearAll();
}

SkBitSet::SkBitSet(const SkBitSet& source)
    : fBitData(NULL), fDwordCount(0), fBitCount(0) {
    *this = source;
}

const SkBitSet& SkBitSet::operator=(const SkBitSet& rhs) {
    if (this == (SkBitSet*)&rhs) {
        return *this;
    }
    fBitCount = rhs.fBitCount;
    fBitData.free();
    fDwordCount = rhs.fDwordCount;
    fBitData.set(malloc(fDwordCount * sizeof(uint32_t)));
    memcpy(fBitData.get(), rhs.fBitData.get(), fDwordCount * sizeof(uint32_t));
    return *this;
}

bool SkBitSet::operator==(const SkBitSet& rhs) {
    if (fBitCount == rhs.fBitCount) {
        if (fBitData.get() != NULL) {
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
    if (fBitData.get() != NULL) {
        sk_bzero(fBitData.get(), fDwordCount * sizeof(uint32_t));
    }
}

void SkBitSet::setBit(int index, bool value) {
    uint32_t mask = 1 << (index % 32);
    if (value) {
        *(internalGet(index)) |= mask;
    } else {
        *(internalGet(index)) &= ~mask;
    }
}

bool SkBitSet::isBitSet(int index) const {
    uint32_t mask = 1 << (index % 32);
    return (*internalGet(index) & mask);
}

bool SkBitSet::orBits(const SkBitSet& source) {
    if (fBitCount != source.fBitCount) {
        return false;
    }
    uint32_t* targetBitmap = internalGet(0);
    uint32_t* sourceBitmap = source.internalGet(0);
    for (size_t i = 0; i < fDwordCount; ++i) {
        targetBitmap[i] |= sourceBitmap[i];
    }
    return true;
}
