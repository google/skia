/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkICUInterface_DEFINED
#define SkICUInterface_DEFINED

#include "include/core/SkTypes.h"

class SkBIDIIterator {
public:
    typedef int Position;
    typedef uint8_t Level;

    virtual Position getLength() = 0;
    virtual Level getLevelAt(Position) = 0;

    // could be static, but we keep on the object
    virtual void reorderVisual(const int32_t runLevels[], int levelsCount,
                               int32_t logicalFromVisual[]) = 0;

    virtual ErrorCode setPara(const SkUTF16 text[], int length, Level paraLevel,
                              Level *embeddingLevels) = 0;
};

class SkUBreakIterator {
public:
    virtual ~SkUBreakIterator() {}

    size_t first() = 0;
    size_t next() = 0;
    size_t preceding(size_t offset) = 0;
    size_t following(size_t offset) = 0;
};

class SkICUInterface {
public:
    virtual ~SkICUInterface() {}

    virtual ScriptID unicharToScriptID(SkUnichar) = 0;
    virtual CombiningClass unicharToCombiningClass(SkUnichar) = 0;
    virtual GeneralCategory unicharToGeneralCategory(SkUnichar) = 0;
    virtual SkUnichar mirrorUnichar(SkUnichar) = 0;
    virtual bool composeUnichars(SkUnichar a, SkUnichar b, SkUnichar* ab) = 0;
    virtual bool decomposeUnichar(SkUnichar ab, SkUnichar* a, SkUnichar* b) = 0;

    virtual std::unique_ptr<SkBIDIIterator> makeBidiIterator() = 0;
    virtual std::unique_ptr<SkUBreakIterator> makeUBreakIterator() = 0;
};

#endif
