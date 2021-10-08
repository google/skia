/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBitSet_DEFINED
#define SkBitSet_DEFINED

#include "include/private/SkMalloc.h"
#include "include/private/SkTOptional.h"
#include "include/private/SkTemplates.h"
#include "src/core/SkMathPriv.h"
#include <climits>
#include <cstring>
#include <limits>
#include <memory>

class SkBitSet {
public:
    explicit SkBitSet(size_t size)
        : fSize(size)
        // May http://wg21.link/p0593 be accepted.
        , fChunks((Chunk*)sk_calloc_throw(NumChunksFor(fSize) * sizeof(Chunk)))
    {}

    SkBitSet(const SkBitSet&) = delete;
    SkBitSet& operator=(const SkBitSet&) = delete;
    SkBitSet(SkBitSet&& that) { *this = std::move(that); }
    SkBitSet& operator=(SkBitSet&& that) {
        if (this != &that) {
            this->fSize = that.fSize;
            this->fChunks = std::move(that.fChunks);
            that.fSize = 0;
        }
        return *this;
    }
    ~SkBitSet() = default;

    /** Set the value of the index-th bit to true. */
    void set(size_t index) {
        SkASSERT(index < fSize);
        *this->chunkFor(index) |= ChunkMaskFor(index);
    }

    /** Sets every bit in the bitset to true. */
    void set() {
        Chunk* chunks = fChunks.get();
        const size_t numChunks = NumChunksFor(fSize);
        std::memset(chunks, 0xFF, sizeof(Chunk) * numChunks);
    }

    /** Set the value of the index-th bit to false. */
    void reset(size_t index) {
        SkASSERT(index < fSize);
        *this->chunkFor(index) &= ~ChunkMaskFor(index);
    }

    /** Sets every bit in the bitset to false. */
    void reset() {
        Chunk* chunks = fChunks.get();
        const size_t numChunks = NumChunksFor(fSize);
        std::memset(chunks, 0, sizeof(Chunk) * numChunks);
    }

    bool test(size_t index) const {
        SkASSERT(index < fSize);
        return SkToBool(*this->chunkFor(index) & ChunkMaskFor(index));
    }

    size_t size() const {
        return fSize;
    }

    // Calls f(size_t index) for each set index.
    template<typename FN>
    void forEachSetIndex(FN f) const {
        const Chunk* chunks = fChunks.get();
        const size_t numChunks = NumChunksFor(fSize);
        for (size_t i = 0; i < numChunks; ++i) {
            if (Chunk chunk = chunks[i]) {  // There are set bits
                const size_t index = i * kChunkBits;
                for (size_t j = 0; j < kChunkBits; ++j) {
                    if (0x1 & (chunk >> j)) {
                        f(index + j);
                    }
                }
            }
        }
    }

    using OptionalIndex = skstd::optional<size_t>;

    // If any bits are set, returns the index of the first.
    OptionalIndex findFirst() {
        const Chunk* chunks = fChunks.get();
        const size_t numChunks = NumChunksFor(fSize);
        for (size_t i = 0; i < numChunks; ++i) {
            if (Chunk chunk = chunks[i]) {  // There are set bits
                static_assert(kChunkBits <= std::numeric_limits<uint32_t>::digits, "SkCTZ");
                const size_t bitIndex = i * kChunkBits + SkCTZ(chunk);
                return OptionalIndex(bitIndex);
            }
        }
        return OptionalIndex();
    }

    // If any bits are not set, returns the index of the first.
    OptionalIndex findFirstUnset() {
        const Chunk* chunks = fChunks.get();
        const size_t numChunks = NumChunksFor(fSize);
        for (size_t i = 0; i < numChunks; ++i) {
            if (Chunk chunk = ~chunks[i]) {  // if there are unset bits ...
                static_assert(kChunkBits <= std::numeric_limits<uint32_t>::digits, "SkCTZ");
                const size_t bitIndex = i * kChunkBits + SkCTZ(chunk);
                if (bitIndex >= fSize) {
                    break;
                }
                return OptionalIndex(bitIndex);
            }
        }
        return OptionalIndex();
    }

private:
    size_t fSize;

    using Chunk = uint32_t;
    static_assert(std::numeric_limits<Chunk>::radix == 2);
    inline static constexpr size_t kChunkBits = std::numeric_limits<Chunk>::digits;
    static_assert(kChunkBits == sizeof(Chunk)*CHAR_BIT, "SkBitSet must use every bit in a Chunk");
    std::unique_ptr<Chunk, SkFunctionWrapper<void(void*), sk_free>> fChunks;

    Chunk* chunkFor(size_t index) const {
        return fChunks.get() + (index / kChunkBits);
    }

    static constexpr Chunk ChunkMaskFor(size_t index) {
        return (Chunk)1 << (index & (kChunkBits-1));
    }

    static constexpr size_t NumChunksFor(size_t size) {
        return (size + (kChunkBits-1)) / kChunkBits;
    }
};

#endif
