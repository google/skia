
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkBitSet_DEFINED
#define SkBitSet_DEFINED

#include "SkTypes.h"
#include "SkTDArray.h"

class SkBitSet {
public:
    /** NumberOfBits must be greater than zero.
     */
    explicit SkBitSet(int numberOfBits);
    explicit SkBitSet(const SkBitSet& source);

    const SkBitSet& operator=(const SkBitSet& rhs);
    bool operator==(const SkBitSet& rhs);
    bool operator!=(const SkBitSet& rhs);

    /** Clear all data.
     */
    void clearAll();

    /** Set the value of the index-th bit.
     */
    void setBit(int index, bool value);

    /** Test if bit index is set.
     */
    bool isBitSet(int index) const;

    /** Or bits from source.  false is returned if this doesn't have the same
     *  bit count as source.
     */
    bool orBits(const SkBitSet& source);

    /** Export indices of set bits to T array.
     */
    template<typename T>
    void exportTo(SkTDArray<T>* array) const {
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
    SkAutoFree fBitData;
    // Dword (32-bit) count of the bitset.
    size_t fDwordCount;
    size_t fBitCount;

    uint32_t* internalGet(int index) const {
        SkASSERT((size_t)index < fBitCount);
        size_t internalIndex = index / 32;
        SkASSERT(internalIndex < fDwordCount);
        return reinterpret_cast<uint32_t*>(fBitData.get()) + internalIndex;
    }
};


#endif
