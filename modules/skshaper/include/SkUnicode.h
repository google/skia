
/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkUnicode_DEFINED
#define SkUnicode_DEFINED
#include "include/core/SkTypes.h"

class SkBidiIterator {
public:
    typedef int Position;
    typedef uint8_t Level;
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
    virtual ~SkUBreakIterator() {}
    virtual size_t first() = 0;
    virtual size_t next() = 0;
    virtual size_t preceding(size_t offset) = 0;
    virtual size_t following(size_t offset) = 0;
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
    virtual std::unique_ptr<SkBidiIterator>
        makeBidiIterator(const uint16_t text[], int count, SkBidiIterator::Direction) = 0;
    virtual std::unique_ptr<SkUBreakIterator> makeUBreakIterator() = 0;
};
std::unique_ptr<SkUnicode> SkUnicode_Make();
#endif
