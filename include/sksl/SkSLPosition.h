/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_POSITION
#define SKSL_POSITION

#include "include/core/SkTypes.h"
#include <string_view>

#ifndef __has_builtin
    #define __has_builtin(x) 0
#endif

namespace SkSL {

class Position {
public:
    Position()
        : fStartOffset(-1)
        , fEndOffset(-1) {}

    static Position Range(int startOffset, int endOffset) {
        SkASSERT(startOffset <= endOffset);
        Position result;
        result.fStartOffset = startOffset;
        result.fEndOffset = endOffset;
        return result;
    }

    bool valid() const {
        return fStartOffset != -1;
    }

    int line(std::string_view source) const;

    int startOffset() const {
        SkASSERT(this->valid());
        return fStartOffset;
    }

    int endOffset() const {
        SkASSERT(this->valid());
        return fEndOffset;
    }

    // Returns the position from this through, and including the entirety of, end.
    Position rangeThrough(Position end) const {
        if (fEndOffset == -1 || end.fEndOffset == -1) {
            return *this;
        }
        SkASSERTF(this->startOffset() <= end.startOffset() && this->endOffset() <= end.endOffset(),
                "Invalid range: (%d-%d) - (%d-%d)\n", this->startOffset(), this->endOffset(),
                end.startOffset(), end.endOffset());
        return Range(this->startOffset(), end.endOffset());
    }

    // Returns a position representing the character immediately after this position
    Position after() const {
        return Range(fEndOffset, fEndOffset + 1);
    }

    bool operator==(const Position& other) const {
        return fStartOffset == other.fStartOffset && fEndOffset == other.fEndOffset;
    }

    bool operator!=(const Position& other) const {
        return !(*this == other);
    }

    bool operator>(const Position& other) const {
        return fStartOffset > other.fStartOffset;
    }

    bool operator>=(const Position& other) const {
        return fStartOffset >= other.fStartOffset;
    }

    bool operator<(const Position& other) const {
        return fStartOffset < other.fStartOffset;
    }

    bool operator<=(const Position& other) const {
        return fStartOffset <= other.fStartOffset;
    }

private:
    int32_t fStartOffset;
    int32_t fEndOffset;
};

} // namespace SkSL

#endif
