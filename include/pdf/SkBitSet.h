
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

    /** Export set bits to unsigned int array. (used in font subsetting)
     */
    void exportTo(SkTDArray<uint32_t>* array) const;

private:
    SkAutoFree fBitData;
    // Dword (32-bit) count of the bitset.
    size_t fDwordCount;
    size_t fBitCount;

    uint32_t* internalGet(int index) const {
        SkASSERT((size_t)index < fBitCount);
        size_t internalIndex = index / 32;
        SkASSERT(internalIndex < fDwordCount);
        return (uint32_t*)fBitData.get() + internalIndex;
    }
};


#endif
