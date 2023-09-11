/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkUnicode_icu_bidi_DEFINED
#define SkUnicode_icu_bidi_DEFINED

#include "modules/skunicode/include/SkUnicode.h"
#include <unicode/ubidi.h>
#include <unicode/umachine.h>
#include <unicode/utypes.h>
#include <cstdint>
#include <memory>
#include <vector>

class SkUnicode_IcuBidi {
public:
    static std::unique_ptr<SkBidiIterator> MakeIterator(const uint16_t utf16[], int utf16Units,
                                                        SkBidiIterator::Direction dir);
    static std::unique_ptr<SkBidiIterator> MakeIterator(const char utf8[],
                                                        int utf8Units,
                                                        SkBidiIterator::Direction dir);
    static bool ExtractBidi(const char utf8[], int utf8Units, SkUnicode::TextDirection dir,
                            std::vector<SkUnicode::BidiRegion>* bidiRegions);

    static const char* errorName(UErrorCode status);
    static void bidi_close(UBiDi* bidi);
    static UBiDiDirection bidi_getDirection(const UBiDi* bidi);
    static SkBidiIterator::Position bidi_getLength(const UBiDi* bidi);
    static SkBidiIterator::Level bidi_getLevelAt(const UBiDi* bidi, int pos);
    static UBiDi* bidi_openSized(int32_t maxLength, int32_t maxRunCount, UErrorCode* pErrorCode);
    static void bidi_setPara(UBiDi* bidi,
                             const UChar* text,
                             int32_t length,
                             UBiDiLevel paraLevel,
                             UBiDiLevel* embeddingLevels,
                             UErrorCode* status);
    static void bidi_reorderVisual(const SkUnicode::BidiLevel runLevels[],
                                   int levelsCount,
                                   int32_t logicalFromVisual[]);
};

#endif // SkUnicode_icu_bidi_DEFINED
