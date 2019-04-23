/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBitSet_DEFINED
#define SkBitSet_DEFINED

#include "include/private/SkTemplates.h"

class SkBitSet {
public:
    explicit SkBitSet(int numberOfBits) {
        SkASSERT(numberOfBits >= 0);
        fDwordCount = (numberOfBits + 31) / 32;  // Round up size to 32-bit boundary.
        if (fDwordCount > 0) {
            fBitData.reset((uint32_t*)sk_calloc_throw(fDwordCount * sizeof(uint32_t)));
        }
    }

    /** Set the value of the index-th bit to true.  */
    void set(int index) {
        uint32_t mask = 1 << (index & 31);
        uint32_t* chunk = this->internalGet(index);
        SkASSERT(chunk);
        *chunk |= mask;
    }

    bool has(int index) const {
        const uint32_t* chunk = this->internalGet(index);
        uint32_t mask = 1 << (index & 31);
        return chunk && SkToBool(*chunk & mask);
    }

    // Calls f(unsigned) for each set value.
    template<typename FN>
    void getSetValues(FN f) const {
        const uint32_t* data = fBitData.get();
        for (unsigned i = 0; i < fDwordCount; ++i) {
            if (uint32_t value = data[i]) {  // There are set bits
                unsigned index = i * 32;
                for (unsigned j = 0; j < 32; ++j) {
                    if (0x1 & (value >> j)) {
                        f(index | j);
                    }
                }
            }
        }
    }

private:
    std::unique_ptr<uint32_t, SkFunctionWrapper<void, void, sk_free>> fBitData;
    size_t fDwordCount;  // Dword (32-bit) count of the bitset.

    uint32_t* internalGet(int index) const {
        size_t internalIndex = index / 32;
        if (internalIndex >= fDwordCount) {
            return nullptr;
        }
        return fBitData.get() + internalIndex;
    }
};


#endif
