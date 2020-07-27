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
#include "src/core/SkMathPriv.h"
#include <climits>
#include <limits>
#include <memory>

class SkBitSet {
public:
    explicit SkBitSet(size_t size)
        : fSize(size)
        // May http://wg21.link/p0593 be accepted.
        , fChunks((Chunk*)sk_calloc_throw(numChunksFor(fSize) * sizeof(Chunk)))
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
        *this->chunkFor(index) |= chunkMaskFor(index);
    }

    /** Set the value of the index-th bit to false.  */
    void reset(size_t index) {
        SkASSERT(index < fSize);
        *this->chunkFor(index) &= ~chunkMaskFor(index);
    }

    bool test(size_t index) const {
        SkASSERT(index < fSize);
        return SkToBool(*this->chunkFor(index) & chunkMaskFor(index));
    }

    size_t size() const {
        return fSize;
    }

    // Calls f(size_t index) for each set index.
    template<typename FN>
    void forEachSetIndex(FN f) const {
        const Chunk* chunks = fChunks.get();
        const size_t numChunks = numChunksFor(fSize);
        for (size_t i = 0; i < numChunks; ++i) {
            if (Chunk chunk = chunks[i]) {  // There are set bits
                const size_t index = i * ChunkBits;
                for (size_t j = 0; j < ChunkBits; ++j) {
                    if (0x1 & (chunk >> j)) {
                        f(index + j);
                    }
                }
            }
        }
    }

    // Use std::optional<size_t> when possible.
    class OptionalIndex {
        bool fHasValue;
        size_t fValue;
    public:
        OptionalIndex() : fHasValue(false) {}
        constexpr OptionalIndex(size_t index) : fHasValue(true), fValue(index) {}

        constexpr size_t* operator->() { return &fValue; }
        constexpr const size_t* operator->() const { return &fValue; }
        constexpr size_t& operator*() & { return fValue; }
        constexpr const size_t& operator*() const& { return fValue; }
        constexpr size_t&& operator*() && { return std::move(fValue); }
        constexpr const size_t&& operator*() const&& { return std::move(fValue); }

        constexpr explicit operator bool() const noexcept { return fHasValue; }
        constexpr bool has_value() const noexcept { return fHasValue; }

        constexpr size_t& value() & { return fValue; }
        constexpr const size_t& value() const & { return fValue; }
        constexpr size_t&& value() && { return std::move(fValue); }
        constexpr const size_t&& value() const && { return std::move(fValue); }

        template<typename U> constexpr size_t value_or(U&& defaultValue) const& {
            return bool(*this) ? **this
                               : static_cast<size_t>(std::forward<U>(defaultValue));
        }
        template<typename U> constexpr size_t value_or(U&& defaultValue) && {
            return bool(*this) ? std::move(**this)
                               : static_cast<size_t>(std::forward<U>(defaultValue));
        }
    };
    // If any bits are set returns the index of the first.
    OptionalIndex findFirst() {
        const Chunk* chunks = fChunks.get();
        const size_t numChunks = numChunksFor(fSize);
        for (size_t i = 0; i < numChunks; ++i) {
            if (Chunk chunk = chunks[i]) {  // There are set bits
                static_assert(ChunkBits <= std::numeric_limits<uint32_t>::digits, "SkCTZ");
                return OptionalIndex(i * ChunkBits + SkCTZ(chunk));
            }
        }
        return OptionalIndex();
    }

private:
    size_t fSize;

    using Chunk = uint32_t;
    static_assert(std::numeric_limits<Chunk>::radix == 2);
    static constexpr size_t ChunkBits = std::numeric_limits<Chunk>::digits;
    static_assert(ChunkBits == sizeof(Chunk)*CHAR_BIT, "It would work, but don't waste bits.");
    std::unique_ptr<Chunk, SkFunctionWrapper<void(void*), sk_free>> fChunks;

    Chunk* chunkFor(size_t index) const {
        return fChunks.get() + (index / ChunkBits);
    }

    static constexpr Chunk chunkMaskFor(size_t index) {
        return (Chunk)1 << (index & (ChunkBits-1));
    }

    static constexpr size_t numChunksFor(size_t size) {
        return (size + (ChunkBits-1)) / ChunkBits;
    }
};


#endif
