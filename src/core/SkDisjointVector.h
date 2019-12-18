/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDisjointVector_DEFINED
#define SkDisjointVector_DEFINED

#include "include/private/SkTArray.h"
#include "src/core/SkMathPriv.h"

#include <memory>
#include <tuple>

class SkBlockHandlerPower2 {
public:
    std::tuple<int, int> toBlockAndOffset(int index) const {
        int i = index + 1;
        int block = 31 - SkCLZ(i);
        // This is the size of the block, and the sum of all previous blocks.
        int blockSize = 1 << block;
        int blockOffset = i - blockSize;
        return {block, blockOffset};
    }

    int blockSize(int block) const {
        return 1 << block;
    }

    // The sum of block sizes including this block..
    int blockSum(int block) const {
        return (1 << (block + 1)) - 1;
    }
};

template <typename T>
class SkDisjointVector {
    class Iterator {
    public:
        using value_type = T;
        using difference_type = ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type;
        using iterator_category = std::input_iterator_tag;
        constexpr Iterator(SkDisjointVector* vector, int blockIndex, T* cursor, T* end)
                : fVector{vector}
                , fBlockIndex{blockIndex}
                , fCursor{cursor}
                , fBlockEnd{end} { }
        constexpr Iterator(const Iterator& that)
            : fVector{that.fVector}
            , fBlockIndex{that.fBlockIndex}
            , fCursor{that.fCursor}
            , fBlockEnd{that.fBlockEnd} {}
        constexpr Iterator& operator++() {
            fCursor++;
            if (fCursor == fBlockEnd) {
                fBlockIndex++;
                if(fBlockIndex < fVector->fBlocks.size()) {
                    fCursor = fVector->blockStart(fBlockIndex);
                    fBlockEnd = fVector->blockEnd(fBlockIndex);
                }
            }
            return *this;
        }
        constexpr Iterator operator++(int) { Iterator tmp(*this); operator++(); return tmp; }
        constexpr bool operator==(const Iterator& rhs) const { return fCursor == rhs.fCursor; }
        constexpr bool operator!=(const Iterator& rhs) const { return fCursor != rhs.fCursor; }
        constexpr reference operator*() { return *fCursor; }

    private:
        const SkDisjointVector* const fVector{nullptr};
        int fBlockIndex;
        T* fCursor;
        T* fBlockEnd;
    };

public:
    // Destruct elements in reverse order.
    ~SkDisjointVector() {
        auto destructBlock = [](T* cursor, T* blockStart) {
            while (cursor > blockStart) {
                --cursor;
                cursor->~T();
            }
        };

        // Do last block;
        destructBlock(fBlockCursor, this->blockStart(fBlocks.size() - 1));

        for(int block = fBlocks.size() - 1; block > 0;) {
            --block;
            destructBlock(this->blockEnd(block), fBlocks[block].get());
        }
    }

    template<typename... Args>
    void emplace(Args&&... args) {
        if (fBlockCursor == fBlockEnd) {
            this->addBlockAdjustCursorAndEnd();
        }
        new (fBlockCursor++) T{std::forward<Args>(args)...};
    }

    T& operator[](int32_t index) {
        SkASSERT(!this->empty());
        auto [block, offset] = fBH.toBlockAndOffset(index);
        return this->blockStart(block)[offset];
    }

    [[nodiscard]] bool empty() const {
        return fBlocks.empty();
    }

    [[nodiscard]] size_t size() const {
        if (fBlocks.empty()) { return 0; }
        // Sum of all the blocks.
        size_t blockSum  = fBH.blockSum(fBlocks.size() - 1);
        // Reduce by the unused portion of the last block.
        return blockSum - (fBlockEnd - fBlockCursor);
    }

    Iterator begin() {
        return Iterator{this, 0, this->blockStart(0), this->blockEnd(0)};
    }
    Iterator end() {
        return Iterator{this, (int)fBlocks.size() - 1, fBlockCursor, fBlockEnd};
    }

private:
    using Elem = std::aligned_storage_t<sizeof(T), alignof(T)>;

    void addBlockAdjustCursorAndEnd() {
        int blockSize = fBH.blockSize(fBlocks.size());
        T* block = (T*)new Elem[blockSize];
        fBlockCursor = block;
        fBlockEnd = block + blockSize;
        fBlocks.push_back(std::unique_ptr<T>(block));
    }

    T* blockStart(int blockIndex) const {
        if (fBlocks.empty()) { return nullptr; }
        return fBlocks[blockIndex].get();
    }

    T* blockEnd(int blockIndex) const {
        if (fBlocks.empty()) { return nullptr; }
        return this->blockStart(blockIndex) + fBH.blockSize(blockIndex);
    }

    SkBlockHandlerPower2 fBH;
    SkSTArray<4, std::unique_ptr<T>> fBlocks;
    T* fBlockEnd{nullptr};
    T* fBlockCursor{nullptr};
};

#endif  // SkDisjointVector_DEFINED
