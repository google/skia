/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/private/base/SkAssert.h"
#include "modules/skunicode/src/SkBidiFactory_icu_full.h"
#include "modules/skunicode/src/SkUnicode_icupriv.h"

#include <unicode/ubidi.h>
#include <unicode/umachine.h>
#include <unicode/utypes.h>

const char* SkBidiICUFactory::errorName(UErrorCode status) const {
    return SkGetICULib()->f_u_errorName(status);
}

SkBidiFactory::BidiCloseCallback SkBidiICUFactory::bidi_close_callback() const {
    return SkGetICULib()->f_ubidi_close;
}

UBiDiDirection SkBidiICUFactory::bidi_getDirection(const UBiDi* bidi) const {
    return SkGetICULib()->f_ubidi_getDirection(bidi);
}

SkBidiIterator::Position SkBidiICUFactory::bidi_getLength(const UBiDi* bidi) const {
    return SkGetICULib()->f_ubidi_getLength(bidi);
}

SkBidiIterator::Level SkBidiICUFactory::bidi_getLevelAt(const UBiDi* bidi, int pos) const {
    return SkGetICULib()->f_ubidi_getLevelAt(bidi, pos);
}

UBiDi* SkBidiICUFactory::bidi_openSized(int32_t maxLength,
                                        int32_t maxRunCount,
                                        UErrorCode* pErrorCode) const {
    return SkGetICULib()->f_ubidi_openSized(maxLength, maxRunCount, pErrorCode);
}

void SkBidiICUFactory::bidi_setPara(UBiDi* bidi,
                                    const UChar* text,
                                    int32_t length,
                                    UBiDiLevel paraLevel,
                                    UBiDiLevel* embeddingLevels,
                                    UErrorCode* status) const {
    return SkGetICULib()->f_ubidi_setPara(bidi, text, length, paraLevel, embeddingLevels, status);
}

void SkBidiICUFactory::bidi_reorderVisual(const SkUnicode::BidiLevel runLevels[],
                                          int levelsCount,
                                          int32_t logicalFromVisual[]) const {
    if (levelsCount == 0) {
        // To avoid an assert in unicode
        return;
    }
    SkASSERT(runLevels != nullptr);
    SkGetICULib()->f_ubidi_reorderVisual(runLevels, levelsCount, logicalFromVisual);
}
