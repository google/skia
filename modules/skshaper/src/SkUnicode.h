/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkUnicode_DEFINED
#define SkUnicode_DEFINED

#include "include/core/SkTypes.h"
#include "src/core/SkSpan.h"
#include <vector>
#include <unicode/utf.h>

namespace skia {

enum class UtfFormat {
    kUTF8,
    kUTF16
};
// Bidi
typedef size_t Position;
typedef uint8_t BidiLevel;
enum class Direction {
    kLTR,
    kRTL,
};
struct BidiRegion {
    BidiRegion(Position start, Position end, BidiLevel level)
      : start(start), end(end), level(level) { }
    Position start;
    Position end;
    BidiLevel level;
};
// LineBreaks
enum class LineBreakType {
    kSoftLineBreak,
    kHardLineBreak
};
struct LineBreakBefore {
    LineBreakBefore(Position pos, LineBreakType breakType)
      : pos(pos), breakType(breakType) { }
    Position pos;
    LineBreakType breakType;
};
// Other breaks
enum class UBreakType {
    kWords,
    kGraphemes,
    kLines
};
struct Range {
    Position start;
    Position end;
};

class SkUnicode {
    public:
        typedef uint32_t ScriptID;
        typedef uint32_t CombiningClass;
        typedef uint32_t GeneralCategory;
        virtual ~SkUnicode() {}
        // High level methods (that we actually use somewhere=SkParagraph)
        virtual bool getBidiRegions
               (const char utf8[], int utf8Units, Direction dir, std::vector<BidiRegion>* results) = 0;
        virtual bool getLineBreaks
               (const char utf8[], int utf8Units, std::vector<LineBreakBefore>* results) = 0;
        virtual bool getWords
               (const char utf8[], int utf8Units, std::vector<Position>* results) = 0;
        virtual bool getGraphemes
               (const char utf8[], int utf8Units, std::vector<Position>* results) = 0;
        virtual bool getWhitespaces
               (const char utf8[], int utf8Units, std::vector<Position>* results) = 0;

        static void ReorderVisual(const BidiLevel runLevels[], int levelsCount, int32_t logicalFromVisual[]);
};

std::unique_ptr<SkUnicode> SkUnicode_Make();

}

#endif // SkUnicode_DEFINED
