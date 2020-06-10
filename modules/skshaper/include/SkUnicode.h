
/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkUnicode_DEFINED
#define SkUnicode_DEFINED
#include "include/core/SkTypes.h"
#include <vector>

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
        (const uint16_t text[], int count, SkBidiIterator::Direction) = 0;
    virtual std::unique_ptr<SkBidiIterator> makeBidiIterator
        (const char text[], int count, SkBidiIterator::Direction) = 0;
    virtual std::vector<SkBidiIterator::Region> getBidiRegions
        (const char utf8[], int utf8Units, SkBidiIterator::Direction dir) = 0;
    virtual std::unique_ptr<SkUBreakIterator> makeUBreakIterator
        (const char text[], int count, SkUBreakIterator::UBreakType type, SkUBreakIterator::Encoding encoding) = 0;
};
std::unique_ptr<SkUnicode> SkUnicode_Make();
#endif
