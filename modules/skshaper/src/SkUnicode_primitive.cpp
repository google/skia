/*
* Copyright 2020 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "modules/skshaper/src/SkUnicode.h"

class SkUnicode_primitive : public SkUnicode {
public:
    ~SkUnicode_primitive() override { }
    std::unique_ptr<SkBidiIterator> makeBidiIterator(const uint16_t text[], int count,
                                                     SkBidiIterator::Direction dir) override = 0;
    std::unique_ptr<SkBidiIterator> makeBidiIterator(const char text[],
                                                     int count,
                                                     SkBidiIterator::Direction dir) override = 0;
    std::unique_ptr<SkBreakIterator> makeBreakIterator(const char locale[],
                                                       BreakType breakType) override = 0;
    std::unique_ptr<SkScriptIterator> makeScriptIterator() override = 0;

    // TODO: Use ICU data file to detect controls and whitespaces
    bool isControl(SkUnichar utf8) override = 0;

    bool isWhitespace(SkUnichar utf8) override = 0;

    SkString convertUtf16ToUtf8(const std::u16string& utf16) override = 0;

    bool getBidiRegions(const char utf8[],
                        int utf8Units,
                        TextDirection dir,
                        std::vector<BidiRegion>* results) override = 0;
    bool getLineBreaks(const char utf8[],
                       int utf8Units,
                       std::vector<LineBreakBefore>* results) override = 0;
    bool getWords(const char utf8[], int utf8Units, std::vector<Position>* results) override = 0;

    bool getGraphemes(const char utf8[], int utf8Units, std::vector<Position>* results) override = 0;

    bool getWhitespaces(const char utf8[], int utf8Units, std::vector<Position>* results) override = 0;
    void reorderVisual(const BidiLevel runLevels[],
                       int levelsCount,
                       int32_t logicalFromVisual[]) override = 0;
};

std::unique_ptr<SkUnicode> SkUnicode::MakePrimitiveUnicode() {
    // TODO: Implement
    return nullptr;
}
