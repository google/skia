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
        : fStartOffsetOrLine(-1)
        , fEndOffset(-1) {}

    static Position Line(int line) {
        Position result;
        result.fStartOffsetOrLine = line;
        result.fEndOffset = -1;
        return result;
    }

    static Position Range(int startOffset, int endOffset) {
        SkASSERT(startOffset <= endOffset);
        Position result;
        result.fStartOffsetOrLine = startOffset;
        result.fEndOffset = endOffset;
        return result;
    }

#if __has_builtin(__builtin_FILE) && __has_builtin(__builtin_LINE)
    static Position Capture(const char* file = __builtin_FILE(), int line = __builtin_LINE());
#else
    static Position Capture() { return Position(); }
#endif // __has_builtin(__builtin_FILE) && __has_builtin(__builtin_LINE)

    bool valid() const {
        return fStartOffsetOrLine != -1;
    }

    int line(std::string_view source = std::string_view()) const;

    int startOffset() const {
        SkASSERT(this->valid());
        SkASSERTF(fEndOffset != -1, "checking offsets of line position (%d)", fStartOffsetOrLine);
        return fStartOffsetOrLine;
    }

    int endOffset() const {
        SkASSERT(this->valid());
        SkASSERTF(fEndOffset != -1, "checking offsets of line position (%d)", fStartOffsetOrLine);
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

    bool operator==(const Position& other) const {
        return fStartOffsetOrLine == other.fStartOffsetOrLine &&
                fEndOffset == other.fEndOffset;
    }

    bool operator!=(const Position& other) const {
        return !(*this == other);
    }

    bool operator>(const Position& other) const {
        return fStartOffsetOrLine > other.fStartOffsetOrLine;
    }

    bool operator>=(const Position& other) const {
        return fStartOffsetOrLine >= other.fStartOffsetOrLine;
    }

    bool operator<(const Position& other) const {
        return fStartOffsetOrLine < other.fStartOffsetOrLine;
    }

    bool operator<=(const Position& other) const {
        return fStartOffsetOrLine <= other.fStartOffsetOrLine;
    }

private:
    // Contains either a start offset (if fEndOffset != -1) or a line number (if fEndOffset == -1)
    int32_t fStartOffsetOrLine;
    int32_t fEndOffset;
};

} // namespace SkSL

#endif
