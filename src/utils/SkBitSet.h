/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBitSet_DEFINED
#define SkBitSet_DEFINED

#include "include/private/SkMalloc.h"
#include "include/private/SkTemplates.h"
#include <memory>

class SkBitSet {
public:
    explicit SkBitSet(size_t size)
        : fSize(size)
        , fChunks((Chunk*)sk_calloc_throw(numChunksFor(fSize) * sizeof(Chunk)))
    {}

    SkBitSet(const SkBitSet&) = delete;
    SkBitSet& operator=(const SkBitSet&) = delete;
    SkBitSet(SkBitSet&& that) : fSize(that.fSize), fChunks(std::move(that.fChunks)) {
        that.fSize = 0;
    }
    SkBitSet& operator=(SkBitSet&& that) {
        this->fSize = that.fSize;
        this->fChunks = std::move(that.fChunks);
        that.fSize = 0;
        return *this;
    }
    ~SkBitSet() = default;

    /** Set the value of the index-th bit to true. */
    void set(size_t index) {
        SkASSERT(index < fSize);
        *this->chunkFor(index) |= chunkMaskFor(index);
    }

    bool has(size_t index) const {
        return (index < fSize) && SkToBool(*this->chunkFor(index) & chunkMaskFor(index));
    }

    size_t size() const {
        return fSize;
    }

    // Calls f(unsigned) for each set value.
    template<typename FN>
    void getSetValues(FN f) const {
        const Chunk* chunks = fChunks.get();
        const size_t numChunks = numChunksFor(fSize);
        for (size_t i = 0; i < numChunks; ++i) {
            if (Chunk chunk = chunks[i]) {  // There are set bits
                size_t index = i * ChunkBits;
                for (unsigned j = 0; j < ChunkBits; ++j) {
                    if (0x1 & (chunk >> j)) {
                        f(index | j);
                    }
                }
            }
        }
    }

private:
    size_t fSize;

    using Chunk = unsigned char; // unsigned char has no trap, may alias, has radix 2
    static constexpr size_t ChunkBits = sizeof(Chunk) * CHAR_BIT;
    static_assert(ChunkBits && !(ChunkBits & (ChunkBits - 1)), "ChunkBits must be power of 2.");
    std::unique_ptr<Chunk, SkFunctionWrapper<void(void*), sk_free>> fChunks;

    Chunk* chunkFor(size_t index) const {
        return fChunks.get() + (index / ChunkBits);
    }

    static constexpr Chunk chunkMaskFor(size_t index) {
        return 1 << (index & (ChunkBits-1));
    }

    static constexpr size_t numChunksFor(size_t size) {
        return (size + (ChunkBits-1)) / ChunkBits;
    }
};


#endif
