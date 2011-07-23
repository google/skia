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

#ifndef SkBitSet_DEFINED
#define SkBitSet_DEFINED

#include "SkTypes.h"

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
    bool isBitSet(int index);

    /** Or bits from source.  false is returned if this doesn't have the same
     *  bit count as source.
     */
    bool orBits(SkBitSet& source);

private:
    SkAutoFree fBitData;
    // Dword (32-bit) count of the bitset.
    size_t fDwordCount;
    size_t fBitCount;

    uint32_t* internalGet(int index) {
        SkASSERT((size_t)index < fBitCount);
        size_t internalIndex = index / 32;
        SkASSERT(internalIndex < fDwordCount);
        return (uint32_t*)fBitData.get() + internalIndex;
    }
};


#endif
