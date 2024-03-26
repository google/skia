/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkBidiSubsetFactory_DEFINED
#define SkBidiSubsetFactory_DEFINED

#include "modules/skunicode/include/SkUnicode.h"
#include "modules/skunicode/src/SkUnicode_icu_bidi.h"

#include <unicode/ubidi.h>
#include <unicode/umachine.h>
#include <unicode/utypes.h>

#include <cstdint>

class SkBidiSubsetFactory : public SkBidiFactory {
public:
    const char* errorName(UErrorCode status) const override;
    SkBidiFactory::BidiCloseCallback bidi_close_callback() const override;
    UBiDiDirection bidi_getDirection(const UBiDi* bidi) const override;
    SkBidiIterator::Position bidi_getLength(const UBiDi* bidi) const override;
    SkBidiIterator::Level bidi_getLevelAt(const UBiDi* bidi, int pos) const override;
    UBiDi* bidi_openSized(int32_t maxLength,
                          int32_t maxRunCount,
                          UErrorCode* pErrorCode) const override;
    void bidi_setPara(UBiDi* bidi,
                      const UChar* text,
                      int32_t length,
                      UBiDiLevel paraLevel,
                      UBiDiLevel* embeddingLevels,
                      UErrorCode* status) const override;
    void bidi_reorderVisual(const SkUnicode::BidiLevel runLevels[],
                            int levelsCount,
                            int32_t logicalFromVisual[]) const override;
};

#endif
