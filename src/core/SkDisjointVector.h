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

template <typename T>
class SkDisjointVector {
    class Iterator {
    public:
        using value_type = T;
        using difference_type = ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type;
        using iterator_category = std::input_iterator_tag;
        constexpr Iterator(SkDisjointVector* vector, size_t index)
                : fVector{vector}
                , fBlock{vector->blockIndex(index + 1)}
                , fCursor{std::get<1>(vector->blockAccess(index))}
                , fBlockEnd{std::get<2>(vector->blockAccess(index))}{ }
        constexpr Iterator(const Iterator& that)
            : fVector{that.fVector}
            , fBlock{that.fBlock}
            , fCursor{that.fCursor}
            , fBlockEnd{that.fBlockEnd} {}
        constexpr Iterator& operator++() {
            ++fCursor;
            if (fCursor == fBlockEnd) {
                fBlock++;
                fCursor = nullptr;
                fBlockEnd = nullptr;
                if (fBlock < fVector->fBlocks.size()) {
                    fCursor = fVector->fBlocks[fBlock];
                    fBlockEnd = fCursor + (1 << fBlock);
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
        int32_t fBlock;
        T* fCursor;
        T* fBlockEnd;
    };
public:

    template<typename... Args>
    void emplace(Args&&... args) {
        if (fBlockCursor == fBlockEnd) {
            this->addBlock();
        }
        new (fBlockCursor++) T{std::forward<Args>(args)...};
    }

    T& operator[](int32_t i) {
        SkASSERT(!this->empty());
        auto t = this->blockAccess(i);
        T* cursor = std::get<1>(t);
        return *cursor;
    }

    [[nodiscard]] bool empty() const {
        return fBlocks.empty();
    }

    [[nodiscard]] size_t size() const {
        if (fBlocks.empty()) { return 0; }
        // Sum of all the blocks.
        size_t blockSum  = (1 << fBlocks.size()) - 1;
        // Reduce by the unused portion of the last block.
        return blockSum - (fBlockEnd - fBlockCursor);
    }

    Iterator begin() { return Iterator{this, 0}; }
    Iterator end() { return Iterator{this, this->size()}; }

private:
    using Elem = std::aligned_storage_t<sizeof(T), alignof(T)>;
    using BI = std::tuple<int32_t, int32_t>;

    int32_t blockIndex(int32_t i) const {
        return 31 - SkCLZ(i + 1);
    }

    std::tuple<T*, T*, T*> blockAccess(int32_t index) {
        int32_t blockIndex = this->blockIndex(index);
        if (blockIndex == fBlocks.size()) {return {nullptr, nullptr, nullptr};}
        int32_t blockSize = 1 << blockIndex;
        int32_t blockCursor = (index + 1) - blockSize;
        T* block = fBlocks[blockIndex];
        return {block, block + blockCursor, block + blockSize};
    }

    void addBlock() {
        size_t newBlockSize = 1 << fBlocks.size();
        T* block = (T*)new Elem[newBlockSize];
        fBlocks.push_back(block);
        std::tie(std::ignore, fBlockCursor, fBlockEnd) = blockAccess(newBlockSize - 1);
    }

    SkSTArray<4, T*> fBlocks;
    T* fBlockEnd{nullptr};
    T* fBlockCursor{nullptr};
};

#endif  // SkDisjointVector_DEFINED
