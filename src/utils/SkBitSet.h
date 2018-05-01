/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBitSet_DEFINED
#define SkBitSet_DEFINED

#include "SkTDArray.h"
#include "SkTemplates.h"

class SkBitSet {
public:
    explicit SkBitSet(int numberOfBits) {
        SkASSERT(numberOfBits >= 0);
        fDwordCount = (numberOfBits + 31) / 32;  // Round up size to 32-bit boundary.
        if (fDwordCount > 0) {
            fBitData.reset((uint32_t*)sk_calloc_throw(fDwordCount * sizeof(uint32_t)));
        }
    }

    SkBitSet(const SkBitSet&) = delete;
    SkBitSet& operator=(const SkBitSet&) = delete;

    /** Set the value of the index-th bit to true.  */
    void set(int index) {
        uint32_t mask = 1 << (index & 31);
        uint32_t* chunk = this->internalGet(index);
        SkASSERT(chunk);
        *chunk |= mask;
    }

    template<typename T>
    void setAll(T* array, int len) {
        static_assert(std::is_integral<T>::value, "T is integral");
        for (int i = 0; i < len; ++i) {
            this->set(static_cast<int>(array[i]));
        }
    }

    bool has(int index) const {
        const uint32_t* chunk = this->internalGet(index);
        uint32_t mask = 1 << (index & 31);
        return chunk && SkToBool(*chunk & mask);
    }

    /** Export indices of set bits to T array. */
    template<typename T>
    void exportTo(SkTDArray<T>* array) const {
        static_assert(std::is_integral<T>::value, "T is integral");
        SkASSERT(array);
        uint32_t* data = reinterpret_cast<uint32_t*>(fBitData.get());
        for (unsigned int i = 0; i < fDwordCount; ++i) {
            uint32_t value = data[i];
            if (value) {  // There are set bits
                unsigned int index = i * 32;
                for (unsigned int j = 0; j < 32; ++j) {
                    if (0x1 & (value >> j)) {
                        array->push(index + j);
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
