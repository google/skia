/*
* Copyright 2020 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "include/core/SkString.h"
#include "modules/skshaper/src/SkUnicode.h"

class SkUnicode_none : public SkUnicode {
public:
    ~SkUnicode_none() override { }

    std::unique_ptr<SkBidiIterator> makeBidiIterator(const uint16_t text[], int count,
                                                     SkBidiIterator::Direction dir) override {
        return nullptr;
    }
    std::unique_ptr<SkBidiIterator> makeBidiIterator(const char text[],
                                                     int count,
                                                     SkBidiIterator::Direction dir) override {
        return nullptr;
    }
    std::unique_ptr<SkBreakIterator> makeBreakIterator(const char locale[],
                                                       BreakType breakType) override {
        return nullptr;
    }
    std::unique_ptr<SkBreakIterator> makeBreakIterator(BreakType breakType) override {
        return nullptr;
    }
    std::unique_ptr<SkScriptIterator> makeScriptIterator() override {
        return nullptr;
    }

    bool isControl(SkUnichar utf8) override {
        return false;
    }

    bool isWhitespace(SkUnichar utf8) override {
        return false;
    }

    bool isSpace(SkUnichar utf8) override {
        return false;
    }

    SkString convertUtf16ToUtf8(const std::u16string& utf16) override {
        return SkString();
    }

    bool getBidiRegions(const char utf8[],
                        int utf8Units,
                        TextDirection dir,
                        std::vector<BidiRegion>* results) override {
        return false;
    }

    bool getLineBreaks(const char utf8[],
                       int utf8Units,
                       std::vector<LineBreakBefore>* results) override {
        return false;
    }

    bool getWords(const char utf8[], int utf8Units, std::vector<Position>* results) override {
        return false;
    }

    bool getGraphemes(const char utf8[], int utf8Units, std::vector<Position>* results) override {
        return false;
    }

    void reorderVisual(const BidiLevel runLevels[],
                       int levelsCount,
                       int32_t logicalFromVisual[]) override {
    }
};

std::unique_ptr<SkUnicode> SkUnicode::Make() {
    return std::make_unique<SkUnicode_none>();
}

void SkBidiIterator::ReorderVisual(const Level runLevels[], int levelsCount,
                                   int32_t logicalFromVisual[]) {
}
