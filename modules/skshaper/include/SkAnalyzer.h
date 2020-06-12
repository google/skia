
/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkAnalyzer_DEFINED
#define SkAnalyzer_DEFINED
#include "include/core/SkTypes.h"
#include <vector>

namespace skia {
namespace unicode {

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
    Position start;
    Position end;
    BidiLevel level;
};

// LineBreaks
enum class BreakType {
    kSoftLineBreak,
    kHardLineBreak
};
struct LineBreak {
    Position pos;
    BreakType breakType;
};

struct Range {
    Position start;
    Position end;
};

class SkUnicodeInput {
  public:
      SkSpan<const char> getText() = 0;
      // ICU iterator running on text in this format (converted if required UTF8 -> UTF16)
      // Bidi iterator only runs on UTF16
      UtfFormat getUtfFormat() = 0;
};

class SkUnicodeOutput {
    public:
    // Positions value are given (converted if necessary) into this format
    UtfFormat getUtfFormat() = 0;
    Direction getTextDirection() = 0;

    void setBidiRegions(Position start, Position end, BidiLevel level) = 0;
    void setLineBreaks(Position pos, BreakType breakType) = 0;
    void setWords(Position start, Position end) = 0;
    void setGraphemes(Position start, Position end) = 0;
};

class SkUnicodeAnalyzer {
    public:
        bool analyzeBidi(const SkUnicodeInput& input, SkUnicodeOutput& output);
        bool analyzeLineBreaks(const SkUnicodeInput& input, SkUnicodeOutput& output);
        bool analyzeWords(const SkUnicodeInput& input, SkUnicodeOutput& output);
        bool analyzeGraphemes(const SkUnicodeInput& input, SkUnicodeOutput& output);
};
}
}

class SkBidiIterator {
public:
    typedef int32_t Position;
    typedef uint8_t Level;
    struct Region {
        Region(Position start, Position end, Level level)
            : start(start), end(end), level(level) { }
        Position start;
        Position end;
        Level level;
    };
    enum Direction {
        kLTR,
        kRTL,
    };
    virtual ~SkBidiIterator() {}
    virtual Position getLength() = 0;
    virtual Level getLevelAt(Position) = 0;
    static void ReorderVisual(const Level runLevels[], int levelsCount, int32_t logicalFromVisual[]);
};

class SkUBreakIterator {
public:
    enum UBreakType {
        kGrapheme,
        kWord,
        kLine
    };
    // SkParagraph iterates text by lines and graphemes as utf8 (SkShaper requires it)
    // Flutter requires results for words iterator to be in utf16
    // It's possible that other clients will have different needs
    enum Encoding {
        kUtf8,
        kUtf16
    };
    typedef int32_t Position;
    typedef int32_t Status;
    virtual ~SkUBreakIterator() {}
    virtual Position first() = 0;
    virtual Position next() = 0;
    virtual Position preceding(Position offset) = 0;
    virtual Position following(Position offset) = 0;
    virtual Status status() = 0;
};

class SkUnicode {
public:
    typedef uint32_t ScriptID;
    typedef uint32_t CombiningClass;
    typedef uint32_t GeneralCategory;
    virtual ~SkUnicode() {}
    virtual ScriptID unicharToScriptID(SkUnichar) = 0;
    virtual CombiningClass unicharToCombiningClass(SkUnichar) = 0;
    virtual GeneralCategory unicharToGeneralCategory(SkUnichar) = 0;
    virtual SkUnichar mirrorUnichar(SkUnichar) = 0;
    virtual bool composeUnichars(SkUnichar a, SkUnichar b, SkUnichar* ab) = 0;
    virtual bool decomposeUnichar(SkUnichar ab, SkUnichar* a, SkUnichar* b) = 0;
    virtual std::unique_ptr<SkBidiIterator> makeBidiIterator
        (const uint16_t utf16[], int utf16Units, SkBidiIterator::Direction) = 0;
    virtual std::unique_ptr<SkBidiIterator> makeBidiIterator
        (const char utf8[], int utf8Units, SkBidiIterator::Direction) = 0;
    virtual bool getBidiRegions
           (const char utf8[], int utf8Units, SkBidiIterator::Direction dir,
            std::vector<SkBidiIterator::Region>& results) = 0;
    virtual std::unique_ptr<SkUBreakIterator> makeUBreakIterator
           (const char utf8[], int utf8Units, SkUBreakIterator::UBreakType type, SkUBreakIterator::Encoding encoding) = 0;
    virtual bool getBreaks
           (const char utf8[], int utf8Units, SkUBreakIterator::UBreakType type, SkUBreakIterator::Encoding encoding,
            std::vector<size_t> & results) = 0;
};
std::unique_ptr<SkUnicode> SkUnicode_Make();
#endif // SkAnalyzer_DEFINED
