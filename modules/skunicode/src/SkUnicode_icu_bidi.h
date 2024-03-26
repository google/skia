/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkUnicode_icu_bidi_DEFINED
#define SkUnicode_icu_bidi_DEFINED

#include "include/core/SkRefCnt.h"
#include "modules/skunicode/include/SkUnicode.h"

#include <unicode/ubidi.h>
#include <unicode/umachine.h>
#include <unicode/utypes.h>
#include <cstdint>
#include <memory>
#include <vector>

// Some versions of SkUnicode need a small subset of ICU to do bidi things. This
// allows us to have the same API for the subset as well as the full ICU.
class SkBidiFactory : public SkRefCnt {
public:
    std::unique_ptr<SkBidiIterator> MakeIterator(const uint16_t utf16[],
                                                 int utf16Units,
                                                 SkBidiIterator::Direction dir) const;
    std::unique_ptr<SkBidiIterator> MakeIterator(const char utf8[],
                                                 int utf8Units,
                                                 SkBidiIterator::Direction dir) const;
    bool ExtractBidi(const char utf8[],
                     int utf8Units,
                     SkUnicode::TextDirection dir,
                     std::vector<SkUnicode::BidiRegion>* bidiRegions) const;

    virtual const char* errorName(UErrorCode status) const = 0;

using BidiCloseCallback = void(*)(UBiDi* bidi);
    virtual BidiCloseCallback bidi_close_callback() const = 0;
    virtual UBiDiDirection bidi_getDirection(const UBiDi* bidi) const = 0;
    virtual SkBidiIterator::Position bidi_getLength(const UBiDi* bidi) const = 0;
    virtual SkBidiIterator::Level bidi_getLevelAt(const UBiDi* bidi, int pos) const = 0;
    virtual UBiDi* bidi_openSized(int32_t maxLength,
                                  int32_t maxRunCount,
                                  UErrorCode* pErrorCode) const = 0;
    virtual void bidi_setPara(UBiDi* bidi,
                              const UChar* text,
                              int32_t length,
                              UBiDiLevel paraLevel,
                              UBiDiLevel* embeddingLevels,
                              UErrorCode* status) const = 0;
    virtual void bidi_reorderVisual(const SkUnicode::BidiLevel runLevels[],
                                    int levelsCount,
                                    int32_t logicalFromVisual[]) const = 0;
};

#endif // SkUnicode_icu_bidi_DEFINED
